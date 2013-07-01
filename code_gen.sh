rm -f *ssend*.c *bsend*.c *rsend*.c


sed s/MPI_Send/MPI_Bsend/g tormpi_send.c | sed s/tormpi_Send/tormpi_Bsend/g > tormpi_bsend.c
sed s/MPI_Send/MPI_Rsend/g tormpi_send.c | sed s/tormpi_Send/tormpi_Rsend/g > tormpi_rsend.c
sed s/MPI_Send/MPI_Ssend/g tormpi_send.c | sed s/tormpi_Send/tormpi_Ssend/g > tormpi_ssend.c

sed s/MPI_Send/MPI_Bsend/g tormpi_send_init.c | sed s/tormpi_Send/tormpi_Bsend/g > tormpi_bsend_init.c
sed s/MPI_Send/MPI_Rsend/g tormpi_send_init.c | sed s/tormpi_Send/tormpi_Rsend/g > tormpi_rsend_init.c
sed s/MPI_Send/MPI_Ssend/g tormpi_send_init.c | sed s/tormpi_Send/tormpi_Ssend/g > tormpi_ssend_init.c

sed s/MPI_Isend/MPI_Ibsend/g tormpi_isend.c | sed s/tormpi_Isend/tormpi_Ibsend/g > tormpi_ibsend.c
sed s/MPI_Isend/MPI_Irsend/g tormpi_isend.c | sed s/tormpi_Isend/tormpi_Irsend/g > tormpi_irsend.c
sed s/MPI_Isend/MPI_Issend/g tormpi_isend.c | sed s/tormpi_Isend/tormpi_Issend/g > tormpi_issend.c
