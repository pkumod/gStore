import os
import re
import random
import subprocess
import FormatHelper

db_list = ['bbug', 'lubm', 'num', 'small']


def generateStdAnswer(db_path, out_path):
    files = os.listdir(db_path)
    nt_file = list(filter(lambda x: 'nt' in x, files))[0]
    query_files = list(filter(lambda x: '.sql' in x, files))
    db_name = 'test_data'
    os.system("bin/gdrop -db " + db_name + " > /dev/null")
    os.system("bin/gbuild -db " + db_name + " -f " + os.path.join(db_path,nt_file) + " > /dev/null")
    for query_file in query_files:
        if re.match(".*\d+.sql",query_file) is None:
            continue
        query_file_path = os.path.join(db_path,query_file)
        is_select , query_vars = FormatHelper.GetSelectVar(query_file_path)
        if not is_select:
            continue
        os.system(f"cp {os.path.join(db_path,query_file)} {os.path.join(out_path,query_file)}")
        print(".....Running " + query_file_path + ".....")
        # Run query
        child = subprocess.Popen(["bin/gquery", "-db", db_name, "-q", query_file_path ]
                                 , stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        query_out = child.stdout.read().decode()
        res_start = query_out.find("final result is")
        if res_start == -1:
            print( query_file_path + 'error!')
            continue
        query_out = query_out[query_out.find("final result is"):]
        query_out = query_out.split('\n')
        output_results , _ = FormatHelper.GetOutputResult(query_out, query_vars)
        out_file_path = os.path.join(out_path,query_file).replace('.sql','r.txt')
        FormatHelper.WriteResultToFile(out_file_path, query_vars, output_results)
    os.system("bin/gdrop -db " + db_name + " > /dev/null")

def generateLimitKAnswer(full_result_path, limit_out_path):
    files = os.listdir(full_result_path)
    query_files = list(filter(lambda x: '.sql' in x, files))
    for query_file in query_files:
        if re.match(".*\d+.sql",query_file) is None:
            continue
        query_file_path = os.path.join(full_result_path,query_file)
        limit_out_file_path = os.path.join(limit_out_path,query_file)
        is_select , query_vars = FormatHelper.GetSelectVar(query_file_path)
        if not is_select:
            continue
        result_file = query_file_path.replace('.sql','r.txt')
        if not os.path.exists(result_file):
            continue
        child = subprocess.Popen(["wc", "-l", result_file]
                                 ,stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        result_num = int(child.stdout.read().decode().split()[0]) - 1
        print(".....Running " + limit_out_file_path + ".....")
        # Run query
        with open(query_file_path) as f:
            bfs_query = f.read()
        with open(limit_out_file_path,'w') as fout:
            k = random.randint(1,result_num+10)
            dfs_query = bfs_query + " limit %d"%k
            fout.write(dfs_query)

def GenerateBFS():
    os.system('rm -r bfs_test')
    os.system('mkdir -p bfs_test')
    for db_name in db_list:
        os.system('mkdir -p bfs_test/%s'%db_name)
        generateStdAnswer('data/%s/' % db_name, 'bfs_test/%s'% db_name)

def GenerateDFS():
    os.system('rm -r dfs_test')
    os.system('mkdir -p dfs_test')
    for db_name in db_list:
        os.system('mkdir -p dfs_test/%s'%db_name)
        generateLimitKAnswer('bfs_test/%s/' % db_name, 'dfs_test/%s'% db_name)

GenerateDFS()