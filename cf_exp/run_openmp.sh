#!/bin/sh

export KMP_AFFINITY=scatter
export OMP_SCHEDULE=dynamic
#export OMP_WAIT_POLICY=PASSIVE
export KMP_SETTINGS=1 
#export KMP_BLOCKTIME=0  
export OMP_NUM_THREADS="$1"
echo OMP_NUM_THREADS: "$1"
./cf.openmp large_dataset.txt
