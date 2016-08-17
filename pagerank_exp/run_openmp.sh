#!/bin/sh

export KMP_AFFINITY=scatter
export OMP_NUM_THREADS="$1"
echo OMP_NUM_THREADS: "$1"
./pagerank.openmp large_dataset.txt
