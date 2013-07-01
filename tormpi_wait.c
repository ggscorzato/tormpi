#include <stdlib.h>
#include <mpi.h>
#include "tormpi_intern.h"

int tormpi_Wait(tormpi_Request *request, MPI_Status *status){
  MPI_Aint size,position=0;
  int flag,count,errp,j;
  unsigned int lid, chid,off,len,nid;
  void *buf,*atn_buf;
  MPI_Datatype datatype;

  int myrank;
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);

  flag=(*request).flag;

  switch(flag) {

  case 1: case 3: case 5:
    lid=(*request).lid;
    chid=(*request).chid;
    len=(*request).len;
    nid=(*request).nid;
    atn_buf=(*request).buf;
    buf=(*request).MPIbuf;
    size=(*request).MPIsize;
    count=(*request).MPIcount;
    datatype=(*request).MPIdatatype;

    off=nid*tormpi_maxbuff;

    atnPoll(lid,chid,off,len,atn_buf,nid);
    tormpi_notifies[lid]--;
    
    /* Unpack the message in the application format */
    MPI_Unpack_external("external32",atn_buf,size,&position,buf,count,datatype);
    /* Release the buffer, if it is the end of an _irecv. Not if it is a persistent call*/
    if (flag == 1) free(atn_buf);
    break;

  case 0: 
    MPI_Wait(&((*request).mpir),status); 
  }

  return(MPI_SUCCESS);
}

int tormpi_Waitall(int count, tormpi_Request *array_of_requests, MPI_Status *array_of_statuses){
  int i;
  for(i=0; i<count;i++){
    tormpi_Wait(&array_of_requests[i], &array_of_statuses[i]);

  }
  return(MPI_SUCCESS);
}

  /* The following two routines deal only with the MPI part of the
   request, which is undefined for torus requests and should appear as
   inactive I do not know how this function can be useful. If anybody
   explains me that, I could maybe code a different behaviour */
int tormpi_Waitany(int count, tormpi_Request *array_of_requests, int *index, MPI_Status *status){
  MPI_Request *array_of_mpi_req;
  int i,res;
  array_of_mpi_req=(MPI_Request *)malloc(count*sizeof(MPI_Request));
  for(i=0; i<count;i++) {
    if( array_of_requests[i].flag == 0) {
      array_of_mpi_req[i] = array_of_requests[i].mpir;
    } else {
      array_of_mpi_req[i] = MPI_REQUEST_NULL;
    }
  }
  res=MPI_Waitany(count,array_of_mpi_req,index,status);
  return(res);
}

int tormpi_Waitsome(int incount, tormpi_Request *array_of_requests, int *outcount, int *array_of_indices, MPI_Status *array_of_statuses){
  int i,res;
  MPI_Request *array_of_mpi_req;

  array_of_mpi_req=(MPI_Request *)malloc(incount*sizeof(MPI_Request));
  for(i=0; i<incount;i++) {
    if( array_of_requests[i].flag == 0) {
      array_of_mpi_req[i] = array_of_requests[i].mpir;
    } else {
      array_of_mpi_req[i] = MPI_REQUEST_NULL;
    }
  }
  res=MPI_Waitsome(incount,array_of_mpi_req,outcount,array_of_indices,array_of_statuses);
  return(res);
}
