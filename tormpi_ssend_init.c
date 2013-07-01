#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "tormpi_intern.h"

int tormpi_Ssend_init(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, tormpi_Request *request){
  MPI_Aint position=0,size;
  int sendres, esize;
  void *atn_buf;
  unsigned int linkId,chId,offset=0,sz,err;

  /* Use the lut only for non optimised nearest-neighbors-only communications */
  if( tormpi_neigh_comm_only==0) {
    dest=tormpi_lut[dest];
  }
  
 if (dest >= 0){
    (*request).flag=0;
    sendres=MPI_Ssend_init(buf, count, datatype, dest, tag, comm, &((*request).mpir));
    return(sendres);

  }else {
    (*request).flag=4;
    chId=tormpi_vc[-dest];
    linkId=-dest-1;
    
    MPI_Pack_external_size("external32",count,datatype,&size);
    sz=(unsigned int)ceil(size/TORMPI_MINPACKF);
    esize=sz*TORMPI_MINPACK;
    if(posix_memalign(&atn_buf,(size_t)TORMPI_BUFALIGN,(size_t)esize) !=0) return(-700);
    
    (*request).lid=linkId;
    (*request).chid=chId;
    (*request).len=sz;
    (*request).buf=atn_buf;
    (*request).MPIbuf=buf;

    if(sz > tormpi_maxbuff) {
      fprintf(stderr,"tormpi_send_init ERROR: packet too large (%d>%d). Please use _sendrecv_init.\n",
	      sz,tormpi_maxbuff);
      exit(-753);
    }

    return(MPI_SUCCESS);
  }
}

