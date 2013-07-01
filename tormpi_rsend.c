#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "tormpi_intern.h"

int tormpi_Rsend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
  MPI_Aint position=0,size;
  int sendres,esize;
  void *pbuf, *atn_buf;
  unsigned int linkId,chId,offset=0,sz,err;

  /* Use the lut only for non optimised nearest-neighbors-only communications */
  if( tormpi_neigh_comm_only==0) {
    dest=tormpi_lut[dest];
  }
  
  /* rank of dest is non-neg when dest is not reachabe via atn */
  if (dest >= 0){
    
    sendres=MPI_Rsend(buf, count, datatype, dest, tag, comm);
    return(sendres);

  }else {

    chId=tormpi_vc[-dest];
    linkId=-dest-1;

    MPI_Pack_external_size("external32",count,datatype,&size); 
    sz=(unsigned int)ceil(size/TORMPI_MINPACKF);
    esize=sz*TORMPI_MINPACK;
    if(posix_memalign(&atn_buf,(size_t)TORMPI_BUFALIGN,(size_t)esize) !=0) return(-700);
    MPI_Pack_external("external32",buf,count,datatype,atn_buf,size,&position);

    pbuf=atn_buf;

    while(tormpi_maxbuff < sz){
#ifdef WITHPROXYBIP
      atnCredit(linkId,chId,0,1,0);
      atnPoll(linkId,chId,0,1,&tormpi_bip,0);
#endif
      ATNSEND(linkId,chId,pbuf,0,tormpi_maxbuff);
      sz-=tormpi_maxbuff;
      pbuf+=tormpi_maxbuff*TORMPI_MINPACK;
    }
#ifdef WITHPROXYBIP
    atnCredit(linkId,chId,0,1,0);
    atnPoll(linkId,chId,0,1,&tormpi_bip,0);
#endif
    ATNSEND(linkId,chId,pbuf,0,sz);

    free(atn_buf);
    return(MPI_SUCCESS);
  }
}

