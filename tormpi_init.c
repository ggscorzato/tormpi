#define MAIN_PROGRAM
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include "tormpi_intern.h"

/* linkId=JDIRW-1, where J=P,M and W=X,Y,Z*/
#define PDIRX 1
#define MDIRX 2
#define PDIRY 3
#define MDIRY 4
#define PDIRZ 5
#define MDIRZ 6
#define BOARDNAMEMAXLENGTH 30
#define DN 0
#define UP 1

int tormpi_Init (int *argc, char ***argv) {
  int *periods, reorder,nprocs,myrank,err;
  int rank_up,rank_dn,ncores,*coords;
  int mu,j,k,core,rank,ix,iy,iz,nx,ny,nz,board,*boards;
  int *plusdirs, *minusdirs,*KK,*koord,*local_list;
  FILE *rfid=NULL;
  char var[128], value[128], temp[128];
  MPI_Group full_group;
  
  /*** Initialize MPI and atn. Identify this process by rank and the total number of processes */
  atnInit();
  err=MPI_Init(argc, argv);
#ifdef WITHPROXY
  proxy_atnInit();
#endif
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);
  tormpi_comm_cart=MPI_COMM_WORLD;

  /*** Scan atnsetup.conf to get the mapping between HW directions and MPI directions */
  if((rfid = fopen("atnsetup.conf", "r")) == (FILE*)NULL) fprintf(stderr,"Error opening atnsetup.conf");
  while (fgets(temp, 127, rfid) != NULL) {
    if(temp[0] == '#')
      continue;
    sscanf(temp, "%[A-Z]=%s\n", var, value);

    if (!strcmp(var, "MPINDIMS")) tormpi_ndims = atoi(value);

    if (!strcmp(var, "NDIMX")) nx = atoi(value);
    if (!strcmp(var, "NDIMY")) ny = atoi(value);
    if (!strcmp(var, "NDIMZ")) nz = atoi(value);

    if (!strcmp(var, "IX")) {
      if (!strcmp(value,"none")) {
        ix = tormpi_ndims + 1;
      } else {
        ix = atoi(value);
      }
    }
    if (!strcmp(var, "IY")) {
      if (!strcmp(value,"none")) {
	iy = tormpi_ndims + 1;
      } else {
	iy = atoi(value);
      }
    }
    if (!strcmp(var, "IZ")) {
      if (!strcmp(value,"none")) {
	iz = tormpi_ndims + 1;
      } else {
	iz = atoi(value);
      }
    }
  }
  fclose(rfid);
  KK=(int*)calloc(tormpi_ndims,sizeof(int));
  coords=(int*)calloc(tormpi_ndims,sizeof(int));

  if((rfid = fopen("atnsetup.conf", "r")) == (FILE*)NULL) fprintf(stderr,"Error opening atnsetup.conf");
  while (fgets(temp, 127, rfid) != NULL) {
    if(temp[0] == '#')
      continue;
    sscanf(temp, "LDIM%d=%s\n", &mu, value);
    if (mu<tormpi_ndims) KK[mu]=atoi(value);
  }
  fclose(rfid);

#if ( DEBUGLEVEL > 0)
  if(myrank==0) {
    printf("atnsetup.conf scanned -- ix=%d,iy=%d,iz=%d, and KK=[",ix,iy,iz);fflush(stdout);
    for(mu=0;mu<tormpi_ndims;mu++){
      printf("%d,",KK[mu]);fflush(stdout);
    }
    printf("]\n",KK[mu]);fflush(stdout);
  }
#endif

  /*** Scan rankfile for board names, in order to check which processes sit in the same board */
  boards=(int *)calloc(nprocs,sizeof(int));
  if((rfid = fopen("rankfile_tormpi", "r")) == (FILE*)NULL) fprintf(stderr,"Error opening rankfile");
  while (fgets(temp, 127, rfid) != NULL) {
    if(temp[0] == '#')
      continue;
    sscanf(temp, "rank %d=anode%d slot=%d\n", &rank,&board,&core);
    if(rank<nprocs) {
      boards[rank]=board;
    } else {
      fprintf(stderr,"Error: rankfile contains more ranks than mpi processes");
    }
    if(rank==myrank) {
      tormpi_mycore=core;
    }
    //    printf("rank:core:board:boards[rank]= (%d:%d:%d:%d)\n",rank,core,board,boards[rank]);fflush(stdout);
  }
  fclose(rfid);
#if ( DEBUGLEVEL > 0)
  printf("rankfile scanned  -- rank:core:board= (%d:%d:%d)\n",myrank,tormpi_mycore,boards[myrank]);fflush(stdout);
#endif

  /*** Create a group with the local (i.e. those living in the same board) processes */
  ncores=1;
  for(mu=0;mu<tormpi_ndims;mu++){
    ncores*=KK[mu];
  }
  local_list = (int*)calloc(ncores,sizeof(int));
  k=0;
  for(j=0;j<nprocs;j++){
    if(boards[j]==boards[myrank]){
      local_list[k]=j;
      k++;
    }
  }
  MPI_Comm_group(MPI_COMM_WORLD,&full_group);
  MPI_Group_incl(full_group,ncores,local_list,&tormpi_local_group);
  MPI_Comm_create(MPI_COMM_WORLD,tormpi_local_group,&tormpi_local_comm);

  /*** Call mpi_cart_create, in order to use mpi_cart_shift and build the tormpi_lut */

  periods=(int*)calloc(tormpi_ndims,sizeof(int));
  tormpi_dims=(int*)calloc(tormpi_ndims,sizeof(int));
  for(mu=0;mu<tormpi_ndims;mu++){
    tormpi_dims[mu]=1; /* initialize all tormpi_dims=1 by default */
  }
  if(tormpi_ndims > ix && ix >= 0) tormpi_dims[ix]=nx; /* */
  if(tormpi_ndims > iy && iy >= 0) tormpi_dims[iy]=ny;
  if(tormpi_ndims > iz && iz >= 0) tormpi_dims[iz]=nz;
  tormpi_hwdims[0]=nx;
  tormpi_hwdims[1]=ny;
  tormpi_hwdims[2]=nz;

  for(mu=0;mu<tormpi_ndims;mu++){
    periods[mu]=1;
    tormpi_dims[mu]*=KK[mu];
  }
  reorder=0;
  MPI_Cart_create(MPI_COMM_WORLD, tormpi_ndims, tormpi_dims, periods, reorder, &tormpi_comm_cart);
  MPI_Cart_coords(tormpi_comm_cart,myrank,tormpi_ndims,coords);

  /*** Build the look-up-table tormpi_lut and the small list of neighbors
       tormpi_neigh.  A process rk which is not neighbor to myrank via the torus
       is identified with tormpi_lut[rk]= rk. A process rk inside the same board
       of myrank is identified by tormpi_lut[rk]= rk (as above) and also by
       tormpi_neigh[UP/DN][mu] = rk.  A process that should be reached by atn_
       routines in direction +/-NU is identified by tormpi_lut[rk]=-P/MDIRNU and
       tormpi_neigh[UP/DN][mu] = -P/MDIRNU.
  */

  plusdirs=(int*)calloc(tormpi_ndims,sizeof(int));
  minusdirs=(int*)calloc(tormpi_ndims,sizeof(int));

  if(tormpi_ndims > ix && ix >= 0){
    plusdirs[ix]=PDIRX;   
    minusdirs[ix]=MDIRX;
  }
  if(tormpi_ndims > iy && iy >= 0){
    plusdirs[iy]=PDIRY;   
    minusdirs[iy]=MDIRY;
  }
  if(tormpi_ndims > iz && iz >= 0){
    plusdirs[iz]=PDIRZ;
    minusdirs[iz]=MDIRZ;
  }
  tormpi_lut=(int*)calloc(nprocs,sizeof(int));
  tormpi_neigh[DN]=(int*)calloc(tormpi_ndims,sizeof(int));
  tormpi_neigh[UP]=(int*)calloc(tormpi_ndims,sizeof(int));
  for(j=0;j<nprocs;j++){
    tormpi_lut[j]=j;
  }

  for(mu=0;mu<tormpi_ndims;mu++){
    MPI_Cart_shift(tormpi_comm_cart,mu,1,&rank_dn,&rank_up);
    tormpi_neigh[DN][mu]= rank_dn;
    tormpi_neigh[UP][mu]= rank_up;
    if(boards[myrank] != boards[rank_dn]){
      tormpi_lut[rank_dn]= -minusdirs[mu];
      tormpi_neigh[DN][mu]= -minusdirs[mu];
      tormpi_neigh_rk[minusdirs[mu]-1] = rank_dn;
    }
    if(boards[myrank]!=boards[rank_up]){
      tormpi_lut[rank_up]= -plusdirs[mu];
      tormpi_neigh[UP][mu]= -plusdirs[mu];
      tormpi_neigh_rk[plusdirs[mu]-1] = rank_up;
    }
    if((boards[myrank]!=boards[rank_up]) && (rank_up == rank_dn)){
      if(coords[mu]%2==1){
	tormpi_lut[rank_up]= -minusdirs[mu]; /* if in different boards, but reachable in both directions 
						I need to choose plus for even and minus for odd. */
      }
    }
  }

  /*** Build the virtual channels tormpi_vc: they are sequential indices (0-7) generated by the local 
       coordinates inside the core in the (hyper)plane orthogonal to the direction of communication */

  /** First compute the local coordintes koord[mu] inside the board */
  j=tormpi_mycore;
  koord=(int*)calloc(tormpi_ndims,sizeof(int));
  for(mu=0;mu<tormpi_ndims;mu++){
    ncores=ncores/KK[mu];
    koord[mu] = j / ncores;
    j = j % ncores;
  }

  /** The virtual channel associated to mu=0 corresponds to non-torus direction and is not used */
  tormpi_vc[0]=99;  

  /** Compute the index associated to the koord projected on the (hyper)plane orthogonal to X */
  tormpi_vc[PDIRX]=0;  
  for(mu=0;mu<tormpi_ndims;mu++) {
    if(mu!=ix){
      tormpi_vc[PDIRX] *= KK[mu];
      tormpi_vc[PDIRX] += koord[mu];
    }
  }
  tormpi_vc[MDIRX]=tormpi_vc[PDIRX];

  tormpi_vc[PDIRY]=0;
  for(mu=0;mu<tormpi_ndims;mu++) {
    if(mu!=iy){
      tormpi_vc[PDIRY] *= KK[mu];
      tormpi_vc[PDIRY] += koord[mu];
    }
  }
  tormpi_vc[MDIRY]=tormpi_vc[PDIRY];

  tormpi_vc[PDIRZ]=0;
  for(mu=0;mu<tormpi_ndims;mu++) {
    if(mu!=iz){
      tormpi_vc[PDIRZ] *= KK[mu];
      tormpi_vc[PDIRZ] += koord[mu];
    }
  }
  tormpi_vc[MDIRY]=tormpi_vc[PDIRY];

  /*** Build the global variables storing the available notify indices */
  for(j=0;j<TORMPI_HWDIRS;j++){
    tormpi_notifies[j] = 0;
  }

  /*** Set the tormpi_max_buff and tormpi_maxnid to some initial value */
  tormpi_set_maxnidandbuf(1);

  /*** Set to 0 only if you are sure that you can safely take the last notiy index. 
       Leave the default (1) if you are not sure */
  tormpi_set_preventlastnid(1);

  /*** Set to 1 only if you want to do communications addressing by
       direction (Then only Nearest Neigh are obviously allowed).
       Otherwise leave the defult 0  */
  tormpi_set_neigh_comm_only(0);

  /* bip */
  tormpi_bip=1;

  /*** Free local pointers */
  free(boards);
  free(periods);
  free(plusdirs);
  free(minusdirs);
  free(KK);
  free(koord);

#if ( DEBUGLEVEL > 0)
  printf("tormpi_init done (%d)\n",myrank);fflush(stdout);
#endif

  /*** Return as MPI_Init does */
  return err;
}
