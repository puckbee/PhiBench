#!/bin/sh

if [ -z "$1" ]; then

   echo "bench is empty"

   exit 0
fi

threads_num=(16 32 61 122 183 244)

code_type=(cilk openmp)

for i in ${code_type[@]}
do
   for j in ${threads_num[@]}
   do
       time amplxe-cl -target-system=mic-native:0 -collect-with runsa -knob event-config=CPU_CLK_UNHALTED,INSTRUCTIONS_EXECUTED,DATA_READ_OR_WRITE,DATA_READ_MISS_OR_WRITE_MISS,DATA_PAGE_WALK,LONG_DATA_PAGE_WALK,BRANCHES,BRANCHES_MISPREDICTED,FE_STALLED,VPU_DATA_READ,VPU_DATA_WRITE,VPU_DATA_READ_MISS,VPU_DATA_WRITE_MISS,VPU_INSTRUCTIONS_EXECUTED,VPU_ELEMENTS_ACTIVE -r result/"$1"_vtune_mic_${j}_part1_${i} -- /root/run_on_mic.sh "$1" ${i} ${j} >> log_vtune_mic_"$1"_${i}.txt 2>&1
       time amplxe-cl -target-system=mic-native:0 -collect-with runsa -knob event-config=CODE_READ,CPU_CLK_UNHALTED,INSTRUCTIONS_EXECUTED,DATA_READ_OR_WRITE,DATA_READ_MISS_OR_WRITE_MISS,EXEC_STAGE_CYCLES,L2_DATA_READ_MISS_CACHE_FILL,L2_DATA_WRITE_MISS_CACHE_FILL,L2_DATA_READ_MISS_MEM_FILL,L2_DATA_WRITE_MISS_MEM_FILL,VPU_ELEMENTS_ACTIVE,L1_DATA_HIT_INFLIGHT_PF1,L2_VICTIM_REQ_WITH_DATA,HWP_L2MISS,SNP_HITM_L2 -r result/"$1"_vtune_mic_${j}_part2_${i} -- /root/run_on_mic.sh "$1" ${i} ${j} >> log_vtune_mic_"$1"_${i}.txt 2>&1

   done
done
