import sys

threads_num = ( 61,122,183,244 )
#threads_num = []
#threads_num.append(244)
#array_part_name = ("part1","part2")
array_part_name = ("part1","part2")

array_events_name1 = ("Branch Miss Ratio","FE_STALLED Per K-Cycles","VPU Instructions Ratio","VPU Data Read Or Write Ratio","VPU Data Read Or Write Miss Ratio")
array_events_name2 = ("Cache Miss Per K-Instructions","Memory Bandwidth","L2 Mem Fill Ratio","L2 Cache Fill Ratio","L2 Cache Hit Ratio","Bandwidth")

#oc_offset1 = (0,16)
oc_offset1 = []
oc_offset1.append(0)
#oc_offset2 = (0,15)
oc_offset2 = []
oc_offset2.append(0)

def write_to_file(_file_writer,_array):
    for i in range(len(_array)):
        for j in range(len(_array[0])):
            _file_writer.write(_array[i][j])
            if j != (len(_array[0])-1):
                _file_writer.write(',')
        _file_writer.write('\n')
    return

def write_one_dem_to_file(_file_writer,_array):
    for i in range(len(_array)):
        _array_tmp = _array[i]
        for j in range(len(_array_tmp)):
            _file_writer.write(str(_array_tmp[j]))
            if j!= (len(_array_tmp)-1):
                _file_writer.write(',')
        _file_writer.write('\n')
    return

bench_name = sys.argv[1]
if len(bench_name) == 0:
   print " bench name is null"
   exit()

read_file_prefix = bench_name+"_csvreport/part_csvreport/hw_events_" + bench_name + "_"
write_file_prefix = bench_name+"_csvreport/part_csvreport/self_define_" + bench_name + "_"
total_file_name = "self_define_" + bench_name + ".csv"

total_file_writer = open(total_file_name,"w")

#array_total = [[] for i in range(1+2*(len(array_events_name1)+len(array_events_name2)))]
array_total = []
array_total_tmp = []
array_total_tmp.append('Event')
array_total_tmp.append('Code Model')
array_total_tmp.append('Bench ')
for i in range(len(threads_num)):
    array_total_tmp.append(threads_num[i])
array_total.append(array_total_tmp)

for part in range(len(array_part_name)):
    if len(array_part_name[part]) != 0:
       file_name = read_file_prefix + array_part_name[part] + ".csv"
    else:
       exit()

    file_reader = open(file_name,"r")
    file_tmp_writer = open(write_file_prefix+array_part_name[part] +"_tmp.csv","w")

    lines = file.readlines(file_reader)

    array_raw = [i.strip().split(',') for i in lines]
    array_process = [[] for i in range(len(threads_num)+3)]

#    print array_raw

    for i in range(len(array_raw)):
        for j in range(len(array_raw[0])):
            array_process[j].append(array_raw[i][j])
            
#    print array_process 

    write_to_file(file_tmp_writer,array_process)
    
    if part == 0:
        array_events_name = array_events_name1
    elif part == 1:
        array_events_name = array_events_name2
    array_bench_name = []
    array_bench_name.append("Bench")
#    for i in range(len(array_events_name)*2):
    for i in range(len(array_events_name)):
        array_bench_name.append(bench_name)
   
    array_code_model = []
    array_code_model.append("Code Model")
#    for i in range(len(array_events_name)):
#        array_code_model.append("cilk")
    for i in range(len(array_events_name)):
        array_code_model.append("openmp")

    array_title = []
    array_title.append("Events")
#    for i in range(len(array_events_name)):
#        array_title.append(array_events_name[i])
    for i in range(len(array_events_name)):
        array_title.append(array_events_name[i])

    array_events_threads =[]
    array_events_threads.append(array_title)
    array_events_threads.append(array_code_model)
    array_events_threads.append(array_bench_name)
   
    for i in range(3,len(array_process)):
#	print len(array_process)
        hw_events = array_process[i]
#	print hw_events
        array_events = []
        array_events.append(threads_num[i-3])
#	print i
        if part == 0:
            for j in oc_offset1:
#		print array_events
#		print j
                array_events.append(float(hw_events[j+9]) / float(hw_events[j+8]))               # Branch Miss Ratio
                array_events.append((float(hw_events[j+10])/ float(hw_events[j+2])) *1000)       # FE_STALLED Per K-Cycles
                array_events.append(float(hw_events[j+15])/ float(hw_events[j+3]))               # VPU Instructions Ratio
                array_events.append((float(hw_events[j+11]) + float(hw_events[j+12]))/float(hw_events[j+4]))  # VPU Data Read Or Write Ratio
                array_events.append((float(hw_events[j+13]) + float(hw_events[j+14]))/float(hw_events[j+5]))  # VPU Data Read Or Write Miss Ratio
        if part == 1:
            for j in oc_offset2:
                #Cache Miss Per K-Instructions
                array_events.append(((float(hw_events[j+5])+float(hw_events[j+12]))/float(hw_events[j+3])) * 1000)
                #Memory Bandwidth
                array_events.append(((float(hw_events[j+9])+float(hw_events[j+10])+float(hw_events[j+13])+float(hw_events[j+14])+float(hw_events[j+15]))*64/float(hw_events[j+2]))*1090000000)
                #L2 Mem Fill Ratio
                array_events.append((float(hw_events[j+9])+float(hw_events[j+10]))/(float(hw_events[j+7])+float(hw_events[j+8])+float(hw_events[j+9])+float(hw_events[j+10])))
                #L2 Cache Fill Ratio
                array_events.append((float(hw_events[j+7])+float(hw_events[j+8]))/(float(hw_events[j+7])+float(hw_events[j+8])+float(hw_events[j+9])+float(hw_events[j+10])))
                #L2 Cache Hit Ratio
                array_events.append((float(hw_events[j+5])+float(hw_events[j+12]))/(float(hw_events[j+7])+float(hw_events[j+8])+float(hw_events[j+9])+float(hw_events[j+10])))
                #Bandwidth
                array_events.append((float(hw_events[j+9])+float(hw_events[j+10])+float(hw_events[j+14])+float(hw_events[j+13])+float(hw_events[j+15]))*64/float(hw_events[j+2])*1.1)


        array_events_threads.append(array_events)
#	print array_events_threads
#    print part
#    print array_events_threads 
#    array_events_threads_tmp = [[] for i in range(2*(len(array_events_name)))]
    array_events_threads_tmp = [[] for i in range((len(array_events_name)))]
#    print len(array_events_name)
    print ("--------------")
    print array_events_threads
    for i in range(len(array_events_threads)):
        array_temp = array_events_threads[i]
#        for j in range(1,len(array_temp)):
#        for j in range(1,2*(len(array_events_name))+1):
        for j in range(1,len(array_events_name)+1):
            array_events_threads_tmp[j-1].append(array_temp[j])
    print("tmptmptmptmptmptmptmp=========================")
    print array_events_threads_tmp
    array_events_threads_tmp2 = []
    for i in range(len(array_events_threads_tmp)):
        array_events_threads_tmp2.append(array_events_threads_tmp[i])
#        array_events_threads_tmp2.append(array_events_threads_tmp[i+len(array_events_name)])
    print("tmp222222222222222222222222222222222222222222222222222222")
    print array_events_threads_tmp2    
    for i in range(len(array_events_threads_tmp2)):
        array_total.append(array_events_threads_tmp2[i])

print ("8**************************")
print array_total
write_one_dem_to_file(total_file_writer,array_total)















