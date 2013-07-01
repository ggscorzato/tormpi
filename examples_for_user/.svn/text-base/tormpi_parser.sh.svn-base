#!/bin/bash

# this script is an example of parser that substitutes the variables
# used by the application to access the neighbouring nodes (here
# g_nb_t/x/y/z_up/dn as in the ETMC code) with the variable
# constructed by tormpi: tormpi_neigh

usedirs=$1
useisendrecv=$2


MPINDIMS=4 # MPI Dimensions as in atnsetup.conf
PROC[$(( 0 + 2 * 0))]=g_nb_t_dn
PROC[$(( 1 + 2 * 0))]=g_nb_t_up
PROC[$(( 0 + 2 * 1))]=g_nb_x_dn
PROC[$(( 1 + 2 * 1))]=g_nb_x_up
PROC[$(( 0 + 2 * 2))]=g_nb_y_dn
PROC[$(( 1 + 2 * 2))]=g_nb_y_up
PROC[$(( 0 + 2 * 3))]=g_nb_z_dn
PROC[$(( 1 + 2 * 3))]=g_nb_z_up



for file in  xchange*.c little_D.c polyakov_loop.c;  do

  for(( mu=0 ; mu<$MPINDIMS ; mu++ )) do 
   for(( ud=0 ; ud<2 ; ud++ )) do 
    DIR=${PROC[$(( ${ud} + 2 * ${mu} ))]}

    if [ ${usedirs} == 1 ]; then
	awk ' // {gsub(/'${DIR}'/,"tormpi_neigh['${ud}']['${mu}']"); print $0} '  ${file} > tormpitmp && mv tormpitmp ${file}
    fi

    if [ ${usedirs} == 0 ]; then
	awk ' // {gsub(/tormpi_neigh\['${ud}'\]\['${mu}'\]/,"'${DIR}'"); print $0} '  ${file} > tormpitmp && mv tormpitmp ${file}
    fi

   done
  done

  if [ ${useisendrecv} == 1 ]; then
      awk ' BEGIN { RS = "\f" } // {gsub(/MPI_Isend/,"MPI_Isendrecv");gsub(/\)\;[ \n\r\t]*MPI_Irecv\(/,", /*tormpi-isendrecv*/\n"); print $0} ' ${file} > tormpitmp && mv tormpitmp ${file}
  fi

  if [ ${useisendrecv} == 0 ]; then
      awk ' BEGIN { RS = "\f" } // {gsub(/MPI_Isendrecv/,"MPI_Isend");gsub(/\, \/\*tormpi-isendrecv\*\/\n/,");\n\t MPI_Irecv("); print $0} ' ${file} > tormpitmp && mv tormpitmp ${file}
  fi


done


