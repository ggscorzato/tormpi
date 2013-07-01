#include <stdlib.h>
#include <mpi.h>
#include "tormpi_intern.h"

int tormpi_Test(tormpi_Request *request, int *flag, MPI_Status *status){
  int tflag;

  tflag=(*request).flag;
  switch(tflag) {
  case 1: case 3: 
    *flag=0;
    break;
  case 2:
    *flag=1;
    (*request).mpir=MPI_REQUEST_NULL;
    break;
  case 0: 
    MPI_Test(&((*request).mpir),flag,status); 
    break;
  }
  return(MPI_SUCCESS);
}

int tormpi_Testall(int count, tormpi_Request *array_of_requests, int *flag, MPI_Status *array_of_statuses){
  int i;
  int * flags;
  *flag=1;
  flags=malloc(count*sizeof(int));
  for(i=0; i<count;i++){
    tormpi_Test(&array_of_requests[i], &flags[i], &array_of_statuses[i]);
    (*flag) *= flags[i];
  }
  return(MPI_SUCCESS);
}

  /* The following two routines deal only with the MPI part of the
   request, which is undefined for torus requests and should appear as
   inactive I do not know how this function can be useful. If anybody
   explains me that, I could maybe code a different behaviour */
int tormpi_Testany(int count, tormpi_Request *array_of_requests, int *index, int *flag, MPI_Status *status){
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
  res=MPI_Testany(count,array_of_mpi_req,index,flag,status);
  return(res);
}

int tormpi_Testsome(int incount, tormpi_Request *array_of_requests, int *outcount, int *array_of_indices, MPI_Status *array_of_statuses){
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
  res=MPI_Testsome(incount,array_of_mpi_req,outcount,array_of_indices,array_of_statuses);
  return(res);
}
