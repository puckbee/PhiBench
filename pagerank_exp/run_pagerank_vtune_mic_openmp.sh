#!/bin/sh

threads_num=(16 32 61 183 122 244)

#export KMP_AFFINITY=scatter
for i in ${threads_num[@]}
do
#  export OMP_NUM_THREADS=$i
  echo OMP_NUM_THREADS:$i
  time amplxe-cl -target-system=mic-native:0 -collect-with runsa -knob event-config=CPU_CLK_UNHALTED,INSTRUCTIONS_EXECUTED,DATA_READ_OR_WRITE,DATA_READ_MISS_OR_WRITE_MISS,DATA_PAGE_WALK,LONG_DATA_PAGE_WALK,BRANCHES,BRANCHES_MISPREDICTED,FE_STALLED,VPU_DATA_READ,VPU_DATA_WRITE,VPU_DATA_READ_MISS,VPU_DATA_WRITE_MISS,VPU_INSTRUCTIONS_EXECUTED,VPU_ELEMENTS_ACTIVE -r result/vtune_mic_${i}_part1_openmp -- /root/run_openmp.sh $i
  time amplxe-cl -target-system=mic-native:0 -collect-with runsa -knob event-config=CODE_READ,CPU_CLK_UNHALTED,INSTRUCTIONS_EXECUTED,DATA_READ_OR_WRITE,DATA_READ_MISS_OR_WRITE_MISS,EXEC_STAGE_CYCLES,L2_DATA_READ_MISS_CACHE_FILL,L2_DATA_WRITE_MISS_CACHE_FILL,L2_DATA_READ_MISS_MEM_FILL,L2_DATA_WRITE_MISS_MEM_FILL,VPU_ELEMENTS_ACTIVE,L1_DATA_HIT_INFLIGHT_PF1,L2_VICTIM_REQ_WITH_DATA,HWP_L2MISS,SNP_HITM_L2 -r result/vtune_mic_${i}_part2_openmp -- /root/run_openmp.sh $i
done

#rm -r result/cf_cilk

#time amplxe-cl -target-system=mic-native:0 -collect-with runsa -knob event-config=CPU_CLK_UNHALTED,INSTRUCTIONS_EXECUTED,DATA_READ_OR_WRITE,DATA_READ_MISS_OR_WRITE_MISS,DATA_PAGE_WALK,LONG_DATA_PAGE_WALK,BRANCHES,BRANCHES_MISPREDICTED,FE_STALLED,VPU_DATA_READ,VPU_DATA_WRITE,VPU_DATA_READ_MISS,VPU_DATA_WRITE_MISS,VPU_INSTRUCTIONS_EXECUTED,VPU_ELEMENTS_ACTIVE -r result/cf_cilk -- /root/cf.cilk

#amplxe-cl -report hw-events -r result/cf_cilk/ -report-output=csvreport/cf_cilk.csv -format=csv -csv-delimiter=,
#amplxe-cl -report hw-events -group-by thread -r result/cf_cilk/ -report-output=csvreport/cilk_thread.csv -format=csv -csv-delimiter=,
#amplxe-cl -report summary -r result/cf_cilk/ -report-output=csvreport/cilk_thread.csv -format=csv -csv-delimiter=,
