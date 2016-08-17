#!/bin/bash
threads_num=(61 122 183 244)
#threads_num=(244)


code_type=(openmp)

part=(part1 part2)

if [ -z "$1" ]; then
    bench=
    echo "bench is empty"
else
    bench="$1"_
fi

#mkdir ./"$bench"csvreport
dir="../"$1"/"

#if [ ! -d "csvreport" ]; then
if [ ! -d "./"$bench"csvreport" ]; then
    mkdir ./"$bench"csvreport
fi


for i in ${threads_num[@]}
do
   for j in ${code_type[@]}
   do
      for x in ${part[@]}
      do
         amplxe-cl -report summary -r "$dir"result/"$bench"vtune_mic_${i}_${x}_${j}/ -report-output="$bench"csvreport/vtune_mic_summary_${x}_${j}_${i}.csv -format=csv -csv-delimiter=,
         amplxe-cl -report hw-events -r "$dir"result/"$bench"vtune_mic_${i}_${x}_${j}/ -report-output="$bench"csvreport/vtune_mic_hwevents_${x}_${j}_${i}.csv -format=csv -csv-delimiter=,
         amplxe-cl -report hw-events -group-by thread -r "$dir"result/"$bench"vtune_mic_${i}_${x}_${j}/ -report-output="$bench"csvreport/vtune_mic_hwevents_thread_${x}_${j}_${i}.csv -format=csv -csv-delimiter=,
         amplxe-cl -report hw-events -group-by module -r "$dir"result/"$bench"vtune_mic_${i}_${x}_${j}/ -report-output="$bench"csvreport/vtune_mic_hwevents_module_${x}_${j}_${i}.csv -format=csv -csv-delimiter=,
         amplxe-cl -report hw-events -group-by function -r "$dir"result/"$bench"vtune_mic_${i}_${x}_${j}/ -report-output="$bench"csvreport/vtune_mic_hwevents_function_${x}_${j}_${i}.csv -format=csv -csv-delimiter=,
         amplxe-cl -report callstacks -r "$dir"result/"$bench"vtune_mic_${i}_${x}_${j}/ -report-output="$bench"csvreport/vtune_mic_callstacks_${x}_${j}_${i}.csv -format=csv -csv-delimiter=,
         amplxe-cl -report top-down -r "$dir"result/"$bench"vtune_mic_${i}_${x}_${j}/ -report-output="$bench"csvreport/vtune_mic_top-down_${x}_${j}_${i}.csv -format=csv -csv-delimiter=,

       done
   done
done
