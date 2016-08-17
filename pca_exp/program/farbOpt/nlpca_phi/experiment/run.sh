#!/bin/sh

file_data=("large_dataset.txt" "medium_dataset.txt" "tiny_dataset.txt")
threads_num=(300 250 244 200 183 150 122 100 61 32)

export KMP_AFFINTY=scatter
for i in ${threads_num[@]}
do
  export OMP_NUM_THREADS=$i
  for ((i=0;i<3;i++))
  do
	time ./train_nlpca.mic ${file_data[$i]} nlpca.param
        echo "\n"
	echo "\n"
  done
done
