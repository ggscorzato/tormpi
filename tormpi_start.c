#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>
#include "tormpi_intern.h"

int tormpi_Start(tormpi_Request *request){
  MPI_Aint size,position=0;
  int flag,nnid,count,err,errp,errc,res;
  unsigned int linkId, chId, sz,offset,nid;
  void *atn_buf,*pbuf,*buf;
  MPI_Datatype datatype;

  flag=(*request).flag;
  switch(flag) {

  case 3: /* atnCredit as initiated by tormpi_Recv_init */
    linkId=(*request).lid;
    chId=(*request).chid;
    sz=(*request).len;
    atn_buf=(*request).buf;

    nid = tormpi_notifies[linkId]; 
    offset=nid*tormpi_maxbuff;
    pbuf=atn_buf;

    if(nid < tormpi_maxnid - tormpi_preventlastnid ) {
      atnCredit(linkId,chId,offset,sz,nid);
#ifdef WITHPROXYBIP
      atnSend(linkId,chId,&tormpi_bip,0,1);
#endif
      tormpi_notifies[linkId]=nid + 1;
      (*request).nid=nid;
    } else {
      fprintf(stderr,"tormpi_start ERROR: last notify index(%d>= %d - %d). Please use _sendrecv_init.\n",
	      nid,tormpi_maxnid,tormpi_preventlastnid);
      exit(-754);
    }
    break;

  case 4: /* ATNSEND as initiated by tormpi_Send_init */
    linkId=(*request).lid;
    chId=(*request).chid;
    sz=(*request).len;
    atn_buf=(*request).buf;
    buf = (*request).MPIbuf;

    MPI_Pack_external("external32",buf,count,datatype,atn_buf,size,&position);

    pbuf=atn_buf;
#ifdef WITHPROXYBIP
    atnCredit(linkId,chId,0,1,0);
    atnPoll(linkId,chId,0,1,&tormpi_bip,0);
#endif
    ATNSEND(linkId,chId,pbuf,0,sz);
    return(MPI_SUCCESS);
    break;

  case 5:
    // atnCredit and atnSend, as initiated by tormpi_sendrecv_init (to be done)
  case 6:
    // atnCredit, atnSend (and possibly atnPoll), as initiated by tormpi_sendrecv_init (to be done)

  case 0: /* MPI */
    res=MPI_Start(&((*request).mpir));
    return(res);
  }
}

int tormpi_Startall(int count, tormpi_Request *array_of_requests){
  int i;
  for(i=0; i<count;i++){
    tormpi_Start(&array_of_requests[i]);
  }
  return(MPI_SUCCESS);
}

