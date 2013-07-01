#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include "tormpi_intern.h"

int tormpi_Isendrecv(void *sendbuf, int sendcount, MPI_Datatype sendtype, 
		    int dest, int sendtag, MPI_Comm sendcomm, tormpi_Request *sendreq,
		    void *recvbuf, int recvcount, MPI_Datatype recvtype, 
		    int source, int recvtag,  MPI_Comm recvcomm, tormpi_Request *recvreq ){
  int res,esize,j;
  MPI_Aint ssize,rsize,sposition=0,rposition=0;
  void *rbuf, *sbuf, *atns_buf, *atnr_buf;
  unsigned int linkId_d,linkId_s,chId_d,chId_s,offset,sz,spaceleft,nid;

#if ( DEBUGLEVEL > 4 )
  int myrank; // DEBUG
  MPI_Comm_rank(MPI_COMM_WORLD,&myrank);  // DEBUG
#endif

  if( tormpi_neigh_comm_only==0) {
    dest=tormpi_lut[dest];
    source=tormpi_lut[source];
  }

  MPI_Pack_external_size("external32",sendcount,sendtype,&ssize); 
  MPI_Pack_external_size("external32",recvcount,recvtype,&rsize); 
  
  /* if the sizes are not equal, sorry, this is too pervert: use MPI */
  if (rsize != ssize) {
    if (dest  < 0 ) dest  =tormpi_neigh_rk[-dest  -1];
    if (source< 0 ) source=tormpi_neigh_rk[-source-1];
  }

  if (dest < 0 && source < 0){ /******************* atn send atn recv ***************/

    chId_d=tormpi_vc[-dest];
    linkId_d=-dest-1;
    chId_s=tormpi_vc[-source];
    linkId_s=-source-1;
    nid = tormpi_notifies[linkId_s]; 
    offset=nid*tormpi_maxbuff;

    sz=(unsigned int)ceil(ssize/TORMPI_MINPACKF);
    esize=sz*TORMPI_MINPACK;
    if(posix_memalign(&atns_buf,(size_t)TORMPI_BUFALIGN,(size_t)esize) !=0) exit(-700);      
    if(posix_memalign(&atnr_buf,(size_t)TORMPI_BUFALIGN,(size_t)esize) !=0) exit(-701);      

    MPI_Pack_external("external32",sendbuf,sendcount,sendtype,atns_buf,ssize,&sposition);
    
    rbuf=atnr_buf;
    sbuf=atns_buf;

#if ( DEBUGLEVEL > 4 )
    fprintf(stderr,"isr-asar[%d]:dest=%d,source%d,sz=%d,size=%d,nid=%d\n",myrank,dest,source,sz,ssize,nid);fflush(stderr); // DEBUG
#endif

    if((sz<= tormpi_maxbuff) && (nid < tormpi_maxnid - tormpi_preventlastnid )){ /* non-blocking possible */

      atnCredit(linkId_s,chId_s,offset,sz,nid);
#ifdef WITHPROXYBIP
      atnSend(linkId_s,chId_s,&tormpi_bip,0,1);
      atnCredit(linkId_d,chId_d,0,1,0);
      atnPoll(linkId_d,chId_d,0,1,&tormpi_bip,0);
#endif
      ATNSEND(linkId_d,chId_d,atns_buf,0,sz);

      tormpi_notifies[linkId_s]=nid + 1; /* encrease the nid of the following call*/
      (*sendreq).flag=2;
      (*recvreq).flag=1;
      (*recvreq).lid=linkId_s;
      (*recvreq).chid=chId_s;
      (*recvreq).len=sz;
      (*recvreq).nid=nid;
      (*recvreq).buf=atnr_buf;

      (*recvreq).MPIbuf=recvbuf;
      (*recvreq).MPIsize=ssize;
      (*recvreq).MPIcount=recvcount;
      (*recvreq).MPIdatatype=recvtype;

#if ( DEBUGLEVEL > 4 )
      fprintf(stderr,"[%d]:isbuf=%f/%f\n",myrank,((double*)atns_buf)[0],((double*)atns_buf)[ssize/sizeof(double)-1]); fflush(stderr); // DEBUG
#endif

    } else { /* non-blocking not possible */
      
    
      (*sendreq).flag=2;
      (*recvreq).flag=2;
      while(tormpi_maxbuff < sz){
	atnCredit(linkId_s,chId_s,offset,tormpi_maxbuff,nid);
#ifdef WITHPROXYBIP
	atnSend(linkId_s,chId_s,&tormpi_bip,0,1);
	atnCredit(linkId_d,chId_d,0,1,0);
	atnPoll(linkId_d,chId_d,0,1,&tormpi_bip,0);
#endif
	ATNSEND(linkId_d,chId_d,sbuf,0,tormpi_maxbuff);
	atnPoll(linkId_s,chId_s,offset,tormpi_maxbuff,rbuf,nid);
	
	sz-=tormpi_maxbuff;
	sbuf+=tormpi_maxbuff*TORMPI_MINPACK;
	rbuf+=tormpi_maxbuff*TORMPI_MINPACK;
      }
      atnCredit(linkId_s,chId_s,offset,sz,nid);
#ifdef WITHPROXYBIP
      atnSend(linkId_s,chId_s,&tormpi_bip,0,1);
      atnCredit(linkId_d,chId_d,0,1,0);
      atnPoll(linkId_d,chId_d,0,1,&tormpi_bip,0);
#endif
      ATNSEND(linkId_d,chId_d,sbuf,0,sz);
      atnPoll(linkId_s,chId_s,offset,sz,rbuf,nid);

#if ( DEBUGLEVEL > 4 )
      fprintf(stderr,"[%d]:sbuf=%f/%f\n",myrank,((double*)atns_buf)[0],((double*)atns_buf)[ssize/sizeof(double)-1]); fflush(stderr); // DEBUG
      fprintf(stderr,"[%d]:rbuf=%f/%f\n",myrank,((double*)atnr_buf)[0],((double*)atnr_buf)[rsize/sizeof(double)-1]); fflush(stderr); // DEBUG
#endif

      MPI_Unpack_external("external32",atnr_buf,rsize,&rposition,recvbuf,recvcount,recvtype);
      
      free(atnr_buf);
      free(atns_buf);
      
    }

    return(MPI_SUCCESS);
    
  } else if( dest < 0 && source >= 0 ) { /******************* atn send MPI recv ***************/

#if ( DEBUGLEVEL > 4 )
    fprintf(stderr,"isr-asmr[%d] dest=%d,source=%d\n",myrank,dest,source);fflush(stderr); // DEBUG
#endif

    chId_d=tormpi_vc[-dest];
    linkId_d=-dest-1;

    sz=(unsigned int)ceil(ssize/TORMPI_MINPACKF);
    esize=sz*TORMPI_MINPACK;
    if(posix_memalign(&atns_buf,(size_t)TORMPI_BUFALIGN,(size_t)esize) !=0) exit(-700);      

    MPI_Pack_external("external32",sendbuf,sendcount,sendtype,atns_buf,ssize,&sposition);
    
    sbuf=atns_buf;
    
    if((sz<= tormpi_maxbuff) && (nid < tormpi_maxnid - tormpi_preventlastnid )){ /* non-blocking possible */

#ifdef WITHPROXYBIP
      atnCredit(linkId_d,chId_d,0,1,0);
      atnPoll(linkId_d,chId_d,0,1,&tormpi_bip,0);
#endif
      ATNSEND(linkId_d,chId_d,atns_buf,0,sz);
      if(res!=sz) exit(-801);

      tormpi_notifies[linkId_s]=nid + 1; /* encrease the nid of the following call*/
      (*sendreq).flag=2;
	
    } else { /* non-blocking not possible */
      
    
      (*sendreq).flag=2;
      while(tormpi_maxbuff < sz){
#ifdef WITHPROXYBIP
	atnCredit(linkId_d,chId_d,0,1,0);
	atnPoll(linkId_d,chId_d,0,1,&tormpi_bip,0);
#endif
	ATNSEND(linkId_d,chId_d,sbuf,0,tormpi_maxbuff);
	
	sz-=tormpi_maxbuff;
	sbuf+=tormpi_maxbuff*TORMPI_MINPACK;
      }
#ifdef WITHPROXYBIP
      atnCredit(linkId_d,chId_d,0,1,0);
      atnPoll(linkId_d,chId_d,0,1,&tormpi_bip,0);
#endif
      ATNSEND(linkId_d,chId_d,sbuf,0,sz);
      
      free(atns_buf);

    }

    (*recvreq).flag=0;
    res=MPI_Irecv(recvbuf, recvcount, recvtype, source, recvtag, recvcomm, &((*recvreq).mpir));

    return(MPI_SUCCESS);

  } else if( dest >= 0 && source < 0 ) { /******************* MPI send atn recv ***************/

#if ( DEBUGLEVEL > 4 )
    fprintf(stderr,"isr-msar[%d] dest=%d,source=%d\n",myrank,dest,source);fflush(stderr); // DEBUG
#endif

    chId_s=tormpi_vc[-source];
    linkId_s=-source-1;
    nid = tormpi_notifies[linkId_s]; 
    offset=nid*tormpi_maxbuff;

    sz=(unsigned int)ceil(ssize/TORMPI_MINPACKF);
    esize=sz*TORMPI_MINPACK;
    if(posix_memalign(&atnr_buf,(size_t)TORMPI_BUFALIGN,(size_t)esize) !=0) exit(-701);      

    rbuf=atnr_buf;
    
    if((sz<= tormpi_maxbuff) && (nid < tormpi_maxnid - tormpi_preventlastnid )){ /* non-blocking possible */

      atnCredit(linkId_s,chId_s,offset,sz,nid);
#ifdef WITHPROXYBIP
      atnSend(linkId_s,chId_s,&tormpi_bip,0,1);
#endif

      tormpi_notifies[linkId_s]=nid + 1; /* encrease the nid of the following call*/
      (*recvreq).flag=1;
      (*recvreq).lid=linkId_s;
      (*recvreq).chid=chId_s;
      (*recvreq).len=sz;
      (*recvreq).nid=nid;
      (*recvreq).buf=atnr_buf;

      (*recvreq).MPIbuf=recvbuf;
      (*recvreq).MPIsize=ssize;
      (*recvreq).MPIcount=recvcount;
      (*recvreq).MPIdatatype=recvtype;

    } else { /* non-blocking not possible */
      
    
      (*recvreq).flag=2;
      while(tormpi_maxbuff < sz){
	atnCredit(linkId_s,chId_s,offset,tormpi_maxbuff,nid);
#ifdef WITHPROXYBIP
	atnSend(linkId_s,chId_s,&tormpi_bip,0,1);
#endif
	atnPoll(linkId_s,chId_s,offset,tormpi_maxbuff,rbuf,nid);
	
	sz-=tormpi_maxbuff;
	rbuf+=tormpi_maxbuff*TORMPI_MINPACK;
      }
      atnCredit(linkId_s,chId_s,offset,sz,nid);
#ifdef WITHPROXYBIP
      atnSend(linkId_s,chId_s,&tormpi_bip,0,1);
#endif
      atnPoll(linkId_s,chId_s,offset,sz,rbuf,nid);
      
      MPI_Unpack_external("external32",atnr_buf,rsize,&rposition,recvbuf,recvcount,recvtype);
      
      free(atnr_buf);
    }

    (*sendreq).flag=0;
    res=MPI_Isend(sendbuf, sendcount, sendtype,   dest, sendtag, sendcomm, &((*sendreq).mpir));

    return(MPI_SUCCESS);

  } else if( dest >= 0 && source >= 0 ) { /******************* MPI send MPI recv ***************/

#if ( DEBUGLEVEL > 4 )
    fprintf(stderr,"isr-msmr[%d] dest=%d,source=%d\n",myrank,dest,source);fflush(stderr); // DEBUG
#endif

    (*sendreq).flag=0;
    (*recvreq).flag=0;
    res=MPI_Isend(sendbuf, sendcount, sendtype,   dest, sendtag, sendcomm, &((*sendreq).mpir));
    res=MPI_Irecv(recvbuf, recvcount, recvtype, source, recvtag, recvcomm, &((*recvreq).mpir));
    return(res);
  }
}



