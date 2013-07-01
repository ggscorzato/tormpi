#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "tormpi_intern.h"

int tormpi_Recv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status){
  MPI_Aint position=0,size;
  int recvres, esize;
  void *pbuf, *atn_buf;
  unsigned int linkId,chId,offset,sz,nid,errc,errp;

  /* Use the lut only for non optimised nearest-neighbors-only communications */
  if( tormpi_neigh_comm_only==0) {
    source=tormpi_lut[source];
  }
  
  /* rank of source is non-neg when source is not reachabe via atn */
  if (source >= 0){

    recvres=MPI_Recv(buf, count, datatype, source, tag, comm, status);
    return(recvres);

  }else {

    chId=tormpi_vc[-source];
    linkId=-source-1;
    nid = tormpi_notifies[linkId]; 
    
    /* compute the size (in 128Bytes units) */
    MPI_Pack_external_size("external32",count,datatype,&size); 
    sz=(unsigned int)ceil(size/TORMPI_MINPACKF);
    esize=sz*TORMPI_MINPACK;
    if(posix_memalign(&atn_buf,(size_t)TORMPI_BUFALIGN,(size_t)esize) !=0) return(-701);
    
    /* I use - in a loop - the available space.  Note that global
       offset and nid remain unchanged at the end of this routine,
       since each Credit is followed by its Poll.  It is not so in
       Irecv and Start. */
    offset=nid*tormpi_maxbuff;
    pbuf=atn_buf;

    /* cycle of the buffer, if the message is too long */
    while(tormpi_maxbuff < sz){
      atnCredit(linkId,chId,offset,tormpi_maxbuff,nid);
#ifdef WITHPROXYBIP
      atnSend(linkId,chId,&tormpi_bip,0,1);
#endif
      atnPoll(linkId,chId,offset,tormpi_maxbuff,pbuf,nid);
      sz-=tormpi_maxbuff;
      pbuf+=tormpi_maxbuff*TORMPI_MINPACK;
    }
    atnCredit(linkId,chId,offset,sz,nid);
#ifdef WITHPROXYBIP
    atnSend(linkId,chId,&tormpi_bip,0,1);
#endif
    atnPoll(linkId,chId,offset,sz,pbuf,nid);

    /* Finally unpack the message in the application format */
    MPI_Unpack_external("external32",atn_buf,size,&position,buf,count,datatype);

    free(atn_buf);
    return(MPI_SUCCESS);
  }
}
