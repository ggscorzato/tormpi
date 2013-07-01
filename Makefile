# this is the makefile for TORMPI

# system specific variables:
ATNDIR=/usr/local/atnw
MPIDIR=/usr/mpi/gcc/openmpi-1.4.1/include
AUDIR=/opt/aurora/include
CC=mpicc
AR=ar

# tormpi specific variables


CFLAGS=-I$(ATNDIR) -I$(MPIDIR) -I$(AUDIR) -g -msse3
LDFLAGS=-L$(ATNDIR)/libatn.a

SRCS = $(wildcard *.c)
OBJS = $(patsubst %.c,%.o,$(wildcard *.c))

CODEGEN = $(shell ./code_gen.sh)

all: libtormpi.a

libtormpi.a : $(OBJS)
	$(AR) cr libtormpi.a $(OBJS)

$(OBJS) : $(SRCS)

$(SRCS) : tormpi.h tormpi_intern.h

tormpi_bsend.c : tormpi_send.c
	$(CODEGEN)

tormpi_ibsend.c : tormpi_isend.c
	$(CODEGEN)

tormpi_bsend_init.c : tormpi_send_init.c
	$(CODEGEN)

clean :
	rm -f *.o *~ .*~ *.a
