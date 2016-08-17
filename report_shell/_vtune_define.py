import sys

threads_num = (61,122,183,244)
#threads_num = []
#threads_num.append(244)
#array_part_name = ("part1","part2")
array_part_name = ("part1","part2")

array_oc_offset = (7,3)

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
            if j != (len(_array_tmp)):
                _file_writer.write(',')
        _file_writer.write('\n')
    return

bench_name = sys.argv[1]
if len(bench_name) == 0:
   print " bench name is null"
   exit()

read_file_prefix = bench_name+"_csvreport/part_csvreport/vtune_define_" + bench_name + "_"
write_file_prefix = bench_name+"_csvreport/part_csvreport/vtune_define_" + bench_name + "_"
total_file_name = "vtune_define_" + bench_name + ".csv"
print("---------------------============================00000000000000000000")
print(bench_name)
print(total_file_name)
print("---------------------============================00000000000000000000")
print("---------------------============================00000000000000000000")

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

    array_process = [i.strip().split(',') for i in lines]

#    write_to_file(file_tmp_writer,array_process)
    
#    print array_events_threads 
    array_events_threads_tmp2 = []
#    for i in range(1,(len(array_process)-1)/2):
    for i in range(1,array_oc_offset[part]+1):
        array_events_threads_tmp2.append(array_process[i])
#	print i
#	print part
#	print array_oc_offset[part]
#	print file_name
#	print array_process[i+array_oc_offset[part]]
#        array_events_threads_tmp2.append(array_process[i+array_oc_offset[part]])
#    print array_events_threads_tmp2    
    for i in range(len(array_events_threads_tmp2)):
        array_total.append(array_events_threads_tmp2[i])

#print array_total
write_one_dem_to_file(total_file_writer,array_total)















