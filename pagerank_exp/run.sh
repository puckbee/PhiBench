#!/bin/sh
time ./run_pagerank_vtune_mic_cilk.sh > log_pagerank_vtune_mic_cilk.txt 2>&1
time ./run_pagerank_vtune_mic_openmp.sh > log_pagerank_vtune_mic_openmp.txt 2>&1
