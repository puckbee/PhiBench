#!/bin/sh

ssh mic0 "rm /root/*"

scp ./program/"$1".cilk mic0:/root
scp ./program/"$1".openmp mic0:/root
scp ../lib/* mic0:/lib64
scp ./dataset/vtune.txt mic0:/root
scp ./run_on_mic.sh mic0:/root
