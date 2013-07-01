#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "tormpi_intern.h"


/* WORK IN PROGRESS !!!! It si only a draft and in fact MPI_Allreduce is not redefined in toermpi_export.h  */
int tormpi_Allreduce(void* sendbuf, void* recvbuf, int count, 
		     MPI_Datatype datatype, MPI_Op op, MPI_Comm comm) {
  int hd,j,temp,dest,source;
  MPI_Status status;

  temp=  tormpi_neigh_comm_only;
  tormpi_neigh_comm_only=1;

  MPI_Allreduce(sendbuf,recvbuf,count,datatype,op,tormpi_local_comm);
  for(j=0;j<count;j++){
    ((double *)sendbuf)[j]=((double*)recvbuf)[j]; // WRONG!! assume datatyoe = double here.
  }

  if(tormpi_mycore==0){
    for(hd=0;hd<TORMPI_HWDIRS/2;hd++){
      for(j=0;j<tormpi_dims[hd];j++){
	dest=2*hd+1;
	source=dest+1;
	tormpi_sendrecv(sendbuf,count,datatype,dest,9090,recvbuf,count,datatype,source,comm,status);
	
	if(op==MPI_SUM) {
	  for(j=0;j<count;j++){
	    ((double*)sendbuf)[j]+=((double*)recvbuf)[j]; // WRONG!! need an auxiliary variable?!? of datatype ?!?
	  }
	} else {
	  fprintf(stderr,"tormpi_reduce: operationnot supported");
	}


      }
    }
  }
  tormpi_neigh_comm_only=temp;
  return(MPI_SUCCESS);
}


/* MPI_Reduce is implemented as MPI_Allreduce, since with this
   algorithm there is no gain in sending the answer to a single
   process */

int tormpi_Reduce(void* sendbuf, void* recvbuf, int count, 
	       MPI_Datatype datatype, MPI_Op op, int root, MPI_Comm comm) {
  int res;
  res=tormpi_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
  return(res);

}

