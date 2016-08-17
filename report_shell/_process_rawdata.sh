#!/bin/bash

if [ -z $1 ]; then

   echo " Please enter into the name of your bench"

   exit 0
else

   echo $1
   
fi

bench=$1"_"
event_csvreport_path="$bench""csvreport/event_csvreport"
part_csvreport_path="$bench""csvreport/part_csvreport"


rm -rf ${event_csvreport_path}
rm -rf ${part_csvreport_path}

rm -f vtune_define*
rm -f hw_events*

threads_num=(61 122 183 244)

code_type=(openmp)

event_list1=(Clockticks Instructions-Retired CPI-Rate Vectorization-Intensity L1-Compute-to-Data-Access-Ratio L2-Compute-to-Data-Access-Ratio L1-TLB-Miss-Ratio L2-TLB-Miss-Ratio CPU_CLK_UNHALTED INSTRUCTIONS_EXECUTED DATA_READ_OR_WRITE DATA_READ_MISS_OR_WRITE_MISS DATA_PAGE_WALK LONG_DATA_PAGE_WALK BRANCHES BRANCHES_MISPREDICTED FE_STALLED VPU_DATA_READ VPU_DATA_WRITE VPU_DATA_READ_MISS VPU_DATA_WRITE_MISS VPU_INSTRUCTIONS_EXECUTED VPU_ELEMENTS_ACTIVE) 

#event_list2=(Clockticks Instructions-Retired CPI-Rate L1-Misses L1-Hit-Ratio Estimated-Latency-Impact L1-Compute-to-Data-Access-Ratio L2-Compute-to-Data-Access-Ratio CODE_READ CPU_CLK_UNHALTED INSTRUCTIONS_EXECUTED DATA_READ_OR_WRITE DATA_READ_MISS_OR_WRITE_MISS EXEC_STAGE_CYCLES L2_DATA_READ_MISS_CACHE_FILL L2_DATA_WRITE_MISS_CACHE_FILL L2_DATA_READ_MISS_MEM_FILL L2_DATA_WRITE_MISS_MEM_FILL VPU_ELEMENTS_ACTIVE L1_DATA_HIT_INFLIGHT_PF1 L2_VICTIM_REQ_WITH_DATA HWP_L2MISS SNP_HITM_L2)
event_list2=(L1-Misses L1-Hit-Ratio Estimated-Latency-Impact CODE_READ CPU_CLK_UNHALTED INSTRUCTIONS_EXECUTED DATA_READ_OR_WRITE DATA_READ_MISS_OR_WRITE_MISS EXEC_STAGE_CYCLES L2_DATA_READ_MISS_CACHE_FILL L2_DATA_WRITE_MISS_CACHE_FILL L2_DATA_READ_MISS_MEM_FILL L2_DATA_WRITE_MISS_MEM_FILL VPU_ELEMENTS_ACTIVE L1_DATA_HIT_INFLIGHT_PF1 L2_VICTIM_REQ_WITH_DATA HWP_L2MISS SNP_HITM_L2)

if [ ! -d "${event_csvreport_path}" ]; then
    mkdir ${event_csvreport_path}
fi

if [ ! -d "${part_csvreport_path}" ]; then
    mkdir ${part_csvreport_path}
fi

part_list=(part1 part2)

for part in ${part_list[@]}
do
   if [ $part = "part1" ]; then

      echo "Now is proceeding the 1nd part of the raw data"
      event_list=("${event_list1[@]}")
    
   else

      echo "Now is proceeding the 2nd part of the raw data"
      event_list=("${event_list2[@]}")

   fi   

   for i in ${code_type[@]}
   do

      for k in ${event_list[@]}
      do

         csv_file=grep_summary_${part}_${i}_"${k}".csv
         tmp_var=${k}
         var="${tmp_var//-/ }"
         echo "${var}"

         if [ -f "${event_csvreport_path}/tmp_${csv_file}" ]; then

            rm -f tmp_${csv_file}	

         fi

#      echo "tmp_${csv_file}"

         for j in ${threads_num[@]}
         do
      
            grep -m 1 "\<${var},[^:]" "$bench"csvreport/vtune_mic_summary_${part}_${i}_${j}.csv >> ${event_csvreport_path}/tmp_${csv_file}

         done

         cat ${event_csvreport_path}/tmp_${csv_file} | \
# need to be changed if there are more than 2 columns
         awk -F"," '{print $2}' | 
         awk 'NR==1{row=NF;for (i=1;i<=row;i++) r[i]=$i;v=r[1];}NR>1{c[NR-1]=$1;for (i=1;i<=row;i++) r[i]=r[i]","$(i+1)}\
            END{printf v;for (i=1;i<=NR-1;i++) printf ","c[i];print "";}' | \
#         sed "s/^/$1,${i},${var},&/g" > ${event_csvreport_path}/${csv_file}
         sed "s/^/${var},${i},$1,&/g" > ${event_csvreport_path}/${csv_file}
   
         rm -f ${event_csvreport_path}/tmp_${csv_file}


         if [[ "${k}" =~ "-" ]]; then

            echo " ${k} is integerated into>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> vtune_define"
            if [[ ! -f ${part_csvreport_path}/vtune_define_$1_${part}.csv ]]; then
                echo "Event,Code Model,Bench,61,122,183,244" > ${part_csvreport_path}/vtune_define_$1_${part}.csv
            fi
            cat ${event_csvreport_path}/${csv_file} >> ${part_csvreport_path}/vtune_define_$1_${part}.csv

         else

            echo " ${k} is integerated into>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> hw_events"
            if [[ ! -f ${part_csvreport_path}/hw_events_$1_${part}.csv ]]; then
                echo "Event,Code Model,Bench,61,122,183,244" > ${part_csvreport_path}/hw_events_$1_${part}.csv
            fi
            cat ${event_csvreport_path}/${csv_file} >> ${part_csvreport_path}/hw_events_$1_${part}.csv

         fi

      done
   done
done



#         END{printf v;for (i=1;i<=NR-1;i++) printf ","c[i];print "";}' | \

#            sed 's/^/${i}&/g'

         
#amplxe-cl -report summary -r result/"$bench"vtune_mic_${i}_${x}_${j}/ -report-output=csvreport/vtune_mic_summary_${x}_${j}_${i}.csv -format=csv -csv-delimiter=,
