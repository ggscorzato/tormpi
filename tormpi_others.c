#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "tormpi_intern.h"

/* The functions below only deal with the MPI_Request part of the
   tormpi_Request. The corresponding MPI_ functions are not supported
   when they act on requests routed via the torus */
int tormpi_Request_free(tormpi_Request *request){
  int res;
  res=MPI_Request_free(&((*request).mpir));
  return(res);
}
int tormpi_Request_get_status(tormpi_Request request, int *flag, MPI_Status *status){
  int res;
  res=MPI_Request_get_status((request).mpir,flag,status);
  return(res);
}
int tormpi_Cancel(tormpi_Request *request){
  int res;
  res=MPI_Cancel(&((*request).mpir));
  return(res);
}
int tormpi_Grequest_start(MPI_Grequest_query_function *query_fn, MPI_Grequest_free_function *free_fn, MPI_Grequest_cancel_function *cancel_fn, void *extra_state, tormpi_Request *request) {
  int res;
  (*request).flag=0;
  res=MPI_Grequest_start(query_fn, free_fn, cancel_fn, extra_state, &((*request).mpir));
  return(res);
}
int tormpi_Grequest_complete(tormpi_Request request){
  int res;
  res=MPI_Grequest_complete((request).mpir);
  return(res);
}
int tormpi_File_iread_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request){
  int res;
  (*request).flag=0;
  res=MPI_File_iread_at(fh, offset, buf, count, datatype, &((*request).mpir));
  return(res);
}
int tormpi_File_iwrite_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request){
  int res;
  (*request).flag=0;
  res=MPI_File_iwrite_at(fh, offset, buf, count, datatype, &((*request).mpir));
  return(res);
}
int tormpi_File_iread(MPI_File fh, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request){
  int res;
  (*request).flag=0;
  res=MPI_File_iread(fh, buf, count, datatype, &((*request).mpir));
  return(res);
}
int tormpi_File_iwrite(MPI_File fh, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request){
  int res;
  (*request).flag=0;
  res=MPI_File_iwrite(fh, buf, count, datatype, &((*request).mpir));
  return(res);
}
int tormpi_File_iread_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request){
  int res;
  (*request).flag=0;
  res=MPI_File_iread_shared(fh, buf, count, datatype, &((*request).mpir));
  return(res);
}
int tormpi_File_iwrite_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request){
  int res;
  (*request).flag=0;
  res=MPI_File_iwrite_shared(fh, buf, count, datatype, &((*request).mpir));
  return(res);
}
tormpi_Request tormpi_Request_f2c(MPI_Fint request){
  tormpi_Request treq;
  MPI_Request mpireq;
  mpireq=MPI_Request_f2c(request);
  treq.mpir=mpireq;
  return(treq);
}
MPI_Fint tormpi_Request_c2f(tormpi_Request request){
  MPI_Fint req;
  req=MPI_Request_c2f(request.mpir);
  return(req);
}

