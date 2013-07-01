#ifndef TORMPI_INTERN_H
#define TORMPI_INTERN_H

/* The parameters below define different possible ways to compile the
 tormpi library They correspond to the _proxy and _dbg suffixes */

#undef WITHPROXY
#undef WITHPROXYBIP
#define DEBUGLEVEL 5

/*
  It is THE ONLY tormpi header file that should be included directly
  in the libtormpi files.  Therefore, it includes declarations and
  definitions which are necessary BOTH in libtormpi AND in the
  application.  These are put in tormpi.h, which is included here.
*/

#include "tormpi.h"
#ifdef WITHPROXY
#include <atnlib_proxy.h>
#endif


/*
  On the other side, this file should be included ONLY in the
  libtormpi files and therefore it should contain directly what cannot
  be seen by the application codes.  This includes in particular the
  definition of tormpi structures in term of MPI_ ones, which is
  below.
*/

#define TORMPI_HWDIRS 6      /* 3 max HW dimensions x 2 directions */
#define TORMPI_MAXBUF128 1024   /* 128k Bytes in units of 128 Bytes */
#define TORMPI_NCRED 16         /* Maximal number of credits available per virtual channel per link */
#define TORMPI_BUFALIGN 16
#define TORMPI_MINPACK  128     /* Minimal Size in Bytes of a Packet */
#define TORMPI_MINPACKF 128.0   /* the same in float */
#ifdef WITHPROXY
# define ATNSEND proxy_atnSend
#else
# define ATNSEND atnSend
#endif

#if defined MAIN_PROGRAM
#  define EXTERN
#else
#  define EXTERN extern
#endif

EXTERN int * tormpi_neigh[2];
EXTERN int * tormpi_lut;
EXTERN int  tormpi_neigh_comm_only;
EXTERN int  tormpi_vc[TORMPI_HWDIRS+1];
EXTERN int  tormpi_notifies[TORMPI_HWDIRS];
EXTERN int  tormpi_maxbuff, tormpi_maxnid;
EXTERN int  tormpi_neigh_rk[TORMPI_HWDIRS];
EXTERN int  tormpi_preventlastnid;
EXTERN MPI_Comm tormpi_comm_cart,tormpi_local_comm;
EXTERN int  tormpi_ndims, *tormpi_dims,tormpi_hwdims[TORMPI_HWDIRS/2],tormpi_mycore;
EXTERN MPI_Group tormpi_local_group;
EXTERN int tormpi_bip __attribute__ ((aligned (TORMPI_BUFALIGN)));

#endif
