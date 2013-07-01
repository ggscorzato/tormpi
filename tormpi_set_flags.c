#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "tormpi_intern.h"

/* This fnction sets the maximal number of nofity inices available and
   the maximal buffer (in units of 128 Bytes) correspondingly. The
   input parameter newmaxnid must be less than TORMPI_NCRED.  The
   function must be called in a point when all communications are
   completed, otherwise the result is unpredictable. */
void tormpi_set_maxnidandbuf(int newmaxnid){
  if(newmaxnid > TORMPI_NCRED){
    newmaxnid=TORMPI_NCRED;
  }
  tormpi_maxnid=newmaxnid;
  tormpi_maxbuff=TORMPI_MAXBUF128/tormpi_maxnid;
}

/* This function decides whether the last notify index can be used.
   This can be safely done if there ar no blocking communications
   calle in between non-blocking ones.  */
void tormpi_set_preventlastnid(int flag) {
  tormpi_preventlastnid=flag;
}


/* This function decides whether the communications are addressed by
   rank (flag=0, default) or by directions (flag=1). Once flag is set
   to 1 once, we cannot go back.  */
void tormpi_set_neigh_comm_only(int flag) {
  if ( flag==0 && tormpi_neigh_comm_only==1 ) {
    fprintf(stdout,
     "tormpi error. You cannot go back to tormpi_neigh_comm_only=0, once the tormpi_lut has been removed.\n");
  } else {
    tormpi_neigh_comm_only=flag;
    if (flag==1) free(tormpi_lut);
  }
}
