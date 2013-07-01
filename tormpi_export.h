#ifndef TORMPI_EXPORT_H
#define TORMPI_EXPORT_H

/*
  It is THE ONLY file that should be included directly in the
  application code.  Therefore it includes declarations which are
  necessary BOTH in libtormpi AND in the application.  These are put
  in tormpi.h, which is included here.
*/

#include "tormpi.h"

/*
  On the other side, this file should be included ONLY in the
  application and therefore it should contain directly what cannot be
  in libtormpi.  This includes in particular the redefinition of MPI_
  stuff, which is below.
*/

/* vector of Nearest neihbors directions, available to the application*/
extern int * tormpi_neigh[2];

#define MPI_Init tormpi_Init
#define MPI_Finalize tormpi_Finalize

/* basic p2p */
//#define MPI_Send tormpi_Send
//#define MPI_Bsend tormpi_Bsend
//#define MPI_Ssend tormpi_Ssend
//#define MPI_Rsend tormpi_Rsend
//#define MPI_Recv tormpi_Recv
#define MPI_Sendrecv tormpi_Sendrecv
#define MPI_Sendrecv_replace tormpi_Sendrecv_replace

/* non-blocking p2p */
#define MPI_Isend tormpi_Isend
#define MPI_Ibsend tormpi_Ibsend
#define MPI_Issend tormpi_Issend
#define MPI_Irsend tormpi_Irsend
#define MPI_Irecv tormpi_Irecv
#define MPI_Isendrecv tormpi_Isendrecv

#define MPI_Wait tormpi_Wait
#define MPI_Waitall tormpi_Waitall
#define MPI_Waitany tormpi_Waitany
#define MPI_Waitsome tormpi_Waitsome
#define MPI_Test tormpi_Test
#define MPI_Testany tormpi_Testany
#define MPI_Testall tormpi_Testall
#define MPI_Testsome tormpi_Testsome

/* persistent p2p */
#define MPI_Send_init tormpi_Send_init
#define MPI_Bsend_init tormpi_Bsend_init
#define MPI_Ssend_init tormpi_Ssend_init
#define MPI_Rsend_init tormpi_Rsend_init
#define MPI_Recv_init tormpi_Recv_init
#define MPI_Start tormpi_Start
#define MPI_Startall tormpi_Startall

/* MPI_Request */
#define MPI_Request tormpi_Request

/* other functions using MPI_Request */
#define MPI_Request_free tormpi_Request_free
#define MPI_Request_get_status tormpi_Request_get_status
#define MPI_Cancel tormpi_Cancel
#define MPI_Grequest_start tormpi_Grequest_start
#define MPI_Grequest_complete tormpi_Grequest_complete
#define MPI_File_iread_at tormpi_File_iread_at
#define MPI_File_iwrite_at tormpi_File_iwrite_at
#define MPI_File_iread tormpi_File_iread
#define MPI_File_iwrite tormpi_File_iwrite
#define MPI_File_iread_shared tormpi_File_iread_shared
#define MPI_File_iwrite_shared tormpi_File_iwrite_shared
#define MPI_Request_f2c tormpi_Request_f2c
#define MPI_Request_c2f tormpi_Request_c2f

#endif
