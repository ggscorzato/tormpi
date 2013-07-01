#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "tormpi_intern.h"

int tormpi_Finalize(void){
  int err;

  err=MPI_Finalize();
  atnFinalize();
#ifdef WITHPROXY
  proxy_atnFinalize();  
#endif
  return err;
}
