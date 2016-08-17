#!/bin/sh

scp ./program/pagerank.cilk mic0:/root
scp ./program/pagerank.openmp mic0:/root
scp ../lib/* mic0:/lib64
scp ./dataset/large_dataset.txt mic0:/root
scp run_openmp.sh mic0:/root
scp run_cilk.sh mic0:/root
