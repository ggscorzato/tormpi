#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "tormpi_intern.h"

int tormpi_Recv_init(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, tormpi_Request *request){
  MPI_Aint position=0,size;
  int res, esize;
  void *atn_buf;
  unsigned int linkId,chId,sz,spaceleft,nid;

  /* Use the lut only for non optimised nearest-neighbors-only communications */
  if( tormpi_neigh_comm_only==0) {
    source=tormpi_lut[source];
  }

  /* rank of source is non-neg when source is not reachabe via atn */
  if (source >= 0){

    (*request).flag=0;
    res=MPI_recv_init(buf, count, datatype, source, tag, comm, &((*request).mpir));
    return(res);

  }else {

    chId=tormpi_vc[-source];
    linkId=-source-1;

    /* compute the size (in 128Bytes units) */
    MPI_Pack_external_size("external32",count,datatype,&size); 
    sz=(unsigned int)ceil(size/TORMPI_MINPACKF);
    esize=sz*TORMPI_MINPACK;
    if(posix_memalign(&atn_buf,(size_t)TORMPI_BUFALIGN,(size_t)esize) !=0) return(-701);
    
    (*request).len=sz;
    if(sz > tormpi_maxbuff) {
      fprintf(stderr,"tormpi_recv_init ERROR: packet too large (%d>%d). Please use _sendrecv_init.\n",
	      sz,tormpi_maxbuff);
      exit(-752);
    }
    (*request).flag=3;
    (*request).lid=linkId;
    (*request).chid=chId;
    (*request).buf=atn_buf;
    (*request).MPIbuf=buf;
    (*request).MPIsize=size;
    (*request).MPIcount=count;
    (*request).MPIdatatype=datatype;

    return(MPI_SUCCESS);
  }
}
