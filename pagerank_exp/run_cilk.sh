#!/bin/sh
export CILK_NWORKERS="$1"
echo CILK_NWORKERS: "$1"

./pagerank.cilk large_dataset.txt
