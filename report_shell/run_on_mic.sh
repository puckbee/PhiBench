#!/bin/sh

export KMP_AFFINITY=scatter
export OMP_SCHEDULE=dynamic
#export OMP_WAIT_POLICY=PASSIVE
#export KMP_SETTINGS=1
#export KMP_BLOCKTIME=0
export OMP_NUM_THREADS="$3"
echo OMP_NUM_THREADS: "$3"
#./"$1"."$2" vtune.txt "$1".param
#./s2d -s 4 -n 40 &>stencil2d.log
./"$1" --N 32768 &>"$1"_"$3".log
