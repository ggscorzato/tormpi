#ifndef TORMPI_H
#define TORMPI_H

/*
  This file is included BOTH in the application code AND in libtormpi.
  Therefore, it must include declarations that should be available to
  both.  However, it SHOULD NOT include what should not be seen by the
  application code (e.g. definition of tormpi_ functions using MPI_
  ones).  The place for these is inside the other tormpi_*.[hc] files
  such as tormpi_intern.h.  And it SHOULD NOT include what cannot be
  seen by the libtormpi (e.g. redefinition of MPI_ functions using
  tormpi_ ones).  The place for these is inside the body of
  tormpi_export.h.
 */

#include <libatn.h>
#include <mpi.h>

/* The definition of tormpi_Request needs to be done BEFORE the redefinition of MPI_Request  */
struct _tormpi_Request { 
  MPI_Request mpir;
  int flag; /* 0: set by default it is an MPI_Request, ignore tormpi req; 
	       1: recv-req set by _Irecv (credit), completed by _Wait (poll); ignore MPI req;
	          or by _Isendrecv (credit+send), completed by _Wait (poll) ; ignore MPI req;
	       2: send-req set by _Isend (send), completed by _Wait (nothing) ; ignore all req;
	          or by _Isendrecv (credit+send+poll), completed by _Wait (nothing) ; ignore all req;
	       3: recv-req set by _Recv_init, used by _Start (credit) completed by _Wait (poll); ignore MPI req;
	       4: send-req set by _Send_init, used by _Start (send) completed by _Wait (nothing); ignore MPI req.
	       5: recv-req set by _Sendrecv_init, used by _Start(credit+send) completed by _Wait (poll); ignore MPI req;
	       6: send-req set by _Sendrecv_init, used by _Start(credit+send) completed by _Wait (poll); ignore MPI req;
	       7: recv-req set by _Sendrecv_init, used by _Start(credit+send+poll) completed by _Wait (nothing); ignore MPI req;
	       8: send-req set by _Sendrecv_init, used by _Start(credit+send+poll) completed by _Wait (nothing); ignore MPI req;
	    */
  int lid;
  int chid;
  int len;
  int nid;
  void * buf;
  void * MPIbuf;
  int MPIsize;
  int MPIcount;
  MPI_Datatype MPIdatatype;
  int lid2;
  int chid2;
};

typedef struct _tormpi_Request tormpi_Request;

/* Init and Finalize */
int tormpi_Init(int *, char ***);
int tormpi_Finalize(void);

/* basic p2p */
int tormpi_Send(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int tormpi_Recv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);

int tormpi_Bsend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int tormpi_Rsend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int tormpi_Ssend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int tormpi_Sendrecv(void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag, void *recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag, MPI_Comm comm, MPI_Status *status);
int tormpi_Sendrecv_replace(void* buf, int count, MPI_Datatype datatype, int dest, int sendtag, int source, int recvtag, MPI_Comm comm, MPI_Status *status);
int tormpi_Isendrecv(void *sendbuf, int sendcount, MPI_Datatype sendtype, int dest, int sendtag, MPI_Comm sendcomm, tormpi_Request *sendreq, void *recvbuf, int recvcount, MPI_Datatype recvtype, int source, int recvtag,  MPI_Comm recvcomm, tormpi_Request *recvreq );

/* non-blocking p2p */
int tormpi_Isend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, tormpi_Request *request);
int tormpi_Ibsend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, tormpi_Request *request);
int tormpi_Issend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, tormpi_Request *request);
int tormpi_Irsend(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, tormpi_Request *request);
int tormpi_Irecv(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, tormpi_Request *request);

int tormpi_Wait(tormpi_Request *request, MPI_Status *status); 
int tormpi_Waitall(int count, tormpi_Request *array_of_requests, MPI_Status *array_of_statuses); 
int tormpi_Waitany(int count, tormpi_Request *array_of_requests, int *index, MPI_Status *status); // ??
int tormpi_Waitsome(int incount, tormpi_Request *array_of_requests, int *outcount, int *array_of_indices, MPI_Status *array_of_statuses); // ??

int tormpi_Test(tormpi_Request *request, int *flag, MPI_Status *status); // TODO (flag=0 -> MPI_Test, otherwise ignore (maybe later))
int tormpi_Testall(int count, tormpi_Request *array_of_requests, int *flag, MPI_Status *array_of_statuses);
int tormpi_Testany(int count, tormpi_Request *array_of_requests, int *index, int *flag, MPI_Status *status);
int tormpi_Testsome(int incount, tormpi_Request *array_of_requests, int *outcount, int *array_of_indices, MPI_Status *array_of_statuses);

/* persistent p2p */
int tormpi_Send_init(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, tormpi_Request *request);
int tormpi_Bsend_init(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, tormpi_Request *request);
int tormpi_Ssend_init(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, tormpi_Request *request);
int tormpi_Rsend_init(void* buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm, tormpi_Request *request);
int tormpi_Recv_init(void* buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, tormpi_Request *request);
int tormpi_Start(tormpi_Request *request);
int tormpi_Startall(int count, tormpi_Request *array_of_requests);

/* other functions using MPI_Request */
int tormpi_Request_free(tormpi_Request *request);
int tormpi_Request_get_status(tormpi_Request request, int *flag, MPI_Status *status);
int tormpi_Cancel(tormpi_Request *request);
int tormpi_Grequest_start(MPI_Grequest_query_function *query_fn, MPI_Grequest_free_function *free_fn, MPI_Grequest_cancel_function *cancel_fn, void *extra_state, tormpi_Request *request);
int tormpi_Grequest_complete(tormpi_Request request);
int tormpi_File_iread_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request);
int tormpi_File_iwrite_at(MPI_File fh, MPI_Offset offset, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request);
int tormpi_File_iread(MPI_File fh, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request); 
int tormpi_File_iwrite(MPI_File fh, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request);
int tormpi_File_iread_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request);
int tormpi_File_iwrite_shared(MPI_File fh, void *buf, int count, MPI_Datatype datatype, tormpi_Request *request); 
tormpi_Request tormpi_Request_f2c(MPI_Fint request);
MPI_Fint tormpi_Request_c2f(tormpi_Request request);

/* tormpi functions*/

void tormpi_set_maxnidandbuf(int);
void tormpi_set_preventlastnid(int flag);
void tormpi_set_neigh_comm_only(int);

#endif
