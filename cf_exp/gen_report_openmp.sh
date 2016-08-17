#!/bin/sh
threads_num=(244 183 122 61 32 16)

for i in ${threads_num[@]}
do
  if [ ! -d "csvreport" ]; then
    mkdir csvreport
  fi
  amplxe-cl -report summary -r result/vtune_mic_${i}_part1_openmp/ -report-output=csvreport/vtune_mic_${i}_part1_summary_openmp.csv -format=csv -csv-delimiter=,
  amplxe-cl -report hw-events -r result/vtune_mic_${i}_part1_openmp/ -report-output=csvreport/vtune_mic_${i}_part1_hwevents_openmp.csv -format=csv -csv-delimiter=,
  amplxe-cl -report hw-events -group-by thread -r result/vtune_mic_${i}_part1_openmp/ -report-output=csvreport/vtune_mic_${i}_part1_hwevents_thread_openmp.csv -format=csv -csv-delimiter=,
  amplxe-cl -report hw-events -group-by module -r result/vtune_mic_${i}_part1_openmp/ -report-output=csvreport/vtune_mic_${i}_part1_hwevents_module_openmp.csv -format=csv -csv-delimiter=,
  amplxe-cl -report hw-events -group-by function -r result/vtune_mic_${i}_part1_openmp/ -report-output=csvreport/vtune_mic_${i}_part1_hwevents_function_openmp.csv -format=csv -csv-delimiter=,
  amplxe-cl -report summary -r result/vtune_mic_${i}_part2_openmp/ -report-output=csvreport/vtune_mic_${i}_part2_summary_openmp.csv -format=csv -csv-delimiter=,
  amplxe-cl -report hw-events -r result/vtune_mic_${i}_part2_openmp/ -report-output=csvreport/vtune_mic_${i}_part2_hwevents_openmp.csv -format=csv -csv-delimiter=,
  amplxe-cl -report hw-events -group-by thread -r result/vtune_mic_${i}_part2_openmp/ -report-output=csvreport/vtune_mic_${i}_part2_hwevents_thread_openmp.csv -format=csv -csv-delimiter=,
  amplxe-cl -report hw-events -group-by module -r result/vtune_mic_${i}_part2_openmp/ -report-output=csvreport/vtune_mic_${i}_part2_hwevents_module_openmp.csv -format=csv -csv-delimiter=,
  amplxe-cl -report hw-events -group-by function -r result/vtune_mic_${i}_part2_openmp/ -report-output=csvreport/vtune_mic_${i}_part2_hwevents_function_openmp.csv -format=csv -csv-delimiter=,
done
