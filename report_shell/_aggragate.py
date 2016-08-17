import sys
import os
import shutil


path= "./Events/"

if os.path.exists(path):
    shutil.rmtree(path)
os.mkdir(path) 

bench_list = ("kmeans", "cf", "naivebayes", "pagerank", "pca", "nlpca", "svm", "sort")

file_list = ("self","vtune")

write_file_name = " "


write_writer = ""

for file_name in file_list:

    insert_flag = 0

    for bench_index in range(len(bench_list)):
        bench_name = bench_list[bench_index]
        read_file_name = "final_report/"+file_name+"_define_"+bench_name+".csv"
    

        file_reader = open(read_file_name, "r")
        lines = file.readlines(file_reader)

        array_raw = [i.strip().split(',') for i in lines]

        insert_head = range(len(array_raw))

        if insert_flag == 0:
            for i in range(len(array_raw)):
                write_file_name = path+str(array_raw[i][0])+".csv"
                file_writer = open(write_file_name, "a")
                file_writer.write(lines[0])
            insert_flag =1

        for i in range(1,len(array_raw)):
            write_file_name = path +str(array_raw[i][0])+".csv"
            file_writer = open(write_file_name, "a")
            print i
        
            print lines[i]
            print write_file_name
            file_writer.write(lines[i])
    
