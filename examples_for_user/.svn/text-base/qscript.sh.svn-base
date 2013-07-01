#!/bin/bash

cd $PBS_O_WORKDIR
PROGRAM="invert"
mpirun  -np $AURORA_NP  --mca btl openib,self,sm  --hostfile $AURORA_HOSTFILE -rf $AURORA_RANKFILE   ./$PROGRAM >> out



