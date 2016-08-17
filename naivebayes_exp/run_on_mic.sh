#!/bin/sh

if [ "$2" = "cilk" ]; then
export CILK_NWORKERS="$3"
echo CILK_NWORKERS: "$3"
else 
export KMP_AFFINITY=scatter
export OMP_SCHEDULE=dynamic
#export OMP_WAIT_POLICY=PASSIVE
#export KMP_SETTINGS=1
#export KMP_BLOCKTIME=0
export OMP_NUM_THREADS="$3"
echo OMP_NUM_THREADS: "$3"
fi
./"$1"."$2" classify vtune.txt
