#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "tormpi_intern.h"

int tormpi_Irecv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, tormpi_Request *request){
  MPI_Aint position=0,size;
  int res,esize;
  void *pbuf, *atn_buf;
  unsigned int linkId,chId,offset,sz,nid,errc,errp;

  /* Use the lut only for non optimised nearest-neighbors-only communications */
  if( tormpi_neigh_comm_only==0) {
    source=tormpi_lut[source];
  }
  
  /* rank of source is non-neg when source is not reachabe via atn */
  if (source >= 0){

    (*request).flag=0;
    res=MPI_Irecv(buf, count, datatype, source, tag, comm, &((*request).mpir));
    return(res);

  }else {

    chId=tormpi_vc[-source];
    linkId=-source-1;
    nid = tormpi_notifies[linkId]; 

    /* compute the size (in 128Bytes units) */
    MPI_Pack_external_size("external32",count,datatype,&size); 
    sz=(unsigned int)ceil(size/TORMPI_MINPACKF);
    esize=sz*TORMPI_MINPACK;
    if(posix_memalign(&atn_buf,(size_t)TORMPI_BUFALIGN,(size_t)esize) !=0) return(-701);
    
    offset=nid*tormpi_maxbuff;
    pbuf=atn_buf;

    if((sz<= tormpi_maxbuff) && (nid < tormpi_maxnid - tormpi_preventlastnid )){
      /* If the packet does not exceed the buffer and the # of
	 credits, then I can afford splitting Credit and Poll */
      atnCredit(linkId,chId,offset,sz,nid);
#ifdef WITHPROXYBIP
      atnSend(linkId,chId,&tormpi_bip,0,1);
#endif

      tormpi_notifies[linkId]=nid + 1; /* encrease the nid of the following call*/
      (*request).flag=1;
      (*request).lid=linkId;
      (*request).chid=chId;
      (*request).len=sz;
      (*request).nid=nid;
      (*request).buf=atn_buf;

      (*request).MPIbuf=buf;
      (*request).MPIsize=size;
      (*request).MPIcount=count;
      (*request).MPIdatatype=datatype;

      return(MPI_SUCCESS);
    } else {
      fprintf(stderr,"tormpi_irecv ERROR: packet too large (%d>%d) or last notify index (%d>= %d - %d). Please use _isendrecv.\n",
	      sz,tormpi_maxbuff,nid,tormpi_maxnid,tormpi_preventlastnid);
      exit(-750);
    }
  }
}
