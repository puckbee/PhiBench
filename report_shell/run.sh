#!/bin/bash

bench_list=(cf kmeans naivebayes pagerank pca nlpca svm sort)
#bench_list=(gemm mc)

if [ ! -d "final_report" ]; then
    mkdir final_report
fi


for i in ${bench_list[@]}
do
    ./gen_report.sh ${i}
    ./_process_rawdata.sh ${i}
    python _self_define.py ${i}
    mv *.csv final_report
    python _vtune_define.py ${i}
    mv *.csv final_report

done

python _aggragate.py
