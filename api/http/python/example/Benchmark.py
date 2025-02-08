"""
# Filename: Benchmark.py
# Author: wangjian
# Last Modified: 2025-01-09 15:08
# Description: Functional and performance testing based on the specified benchmark datasets
"""
import time
import json
import sys
sys.path.append('./src')
import GstoreConnector

# before you run this benchmark testing, make sure that you have started up api service (using bin/gserver -b)
# default db_name: lubm(must be built in advance)

IP = "127.0.0.1"
Port = 9000
username = "root"
password = "123456"
load_num = 10
query_num = 10
# base_path
#   ├── lubm10m
#   │   ├── 1k.nt
#   │   ├── 10k.nt
#   │   ├── 100k.nt
#   │   ├── lubm_q1.rq - lubm_q14.rq (14 queries)
base_path = "path_to_test_dir"
db_name = "lubm10m"
# query files, 14 queries in total
sparqls = [
    "lubm_q1.rq", 
    "lubm_q2.rq", 
    "lubm_q3.rq", 
    "lubm_q4.rq", 
    "lubm_q5.rq", 
    "lubm_q6.rq", 
    "lubm_q7.rq", 
    "lubm_q8.rq", 
    "lubm_q9.rq", 
    "lubm_q10.rq", 
    "lubm_q11.rq", 
    "lubm_q12.rq", 
    "lubm_q13.rq", 
    "lubm_q14.rq"
    ]
# batch update files, 3 data files in total
batch_updates = ["1k.nt", "10k.nt", "100k.nt"]

# build
def build(gc):
    # drop the database
    res = gc.drop(db_name)
    # build the database
    startTime = time.perf_counter()
    res = gc.build(db_name, base_path + db_name + "/" + db_name + ".nt")
    endTime = time.perf_counter()
     # convert time to ms and print
    costTime = round((endTime - startTime) * 1000)
    print("build cost time(ms): " + str(costTime))
    res = gc.monitor(db_name)
    json_obj = json.loads(res)
    triple_num = int(json_obj['tripleNum'])
    build_perf = round(triple_num / (endTime - startTime))
    print("build number of triples: " + json_obj['tripleNum'])
    print("build performance(triples/s): " + str(build_perf))

def load(gc):
    # load the database
    startTime = time.perf_counter()
    res = gc.load(db_name)
    endTime = time.perf_counter()
     # convert time to ms and print
    costTime = round((endTime - startTime) * 1000)
    return costTime

def unload(gc):
    # unload the database
    gc.unload(db_name)

def query(gc, sparql):
    costTimes = [None] * query_num
    for i in range(query_num):
        res = gc.query(db_name, sparql)
        json_obj = json.loads(res)
        costTimes[i] = int(json_obj['QueryTime'])
    average = sum(costTimes) / len(costTimes)
    return average


def testLoad(gc):
    costTimes = [None] * load_num
    for i in range(load_num):
        unload(gc)
        costTimes[i] = load(gc)
        time.sleep(1)
    averageTime = sum(costTimes) / len(costTimes)
    print("load average time(ms): " + str(averageTime))
    res = gc.monitor(db_name)
    json_obj = json.loads(res)
    triple_num = int(json_obj['tripleNum'])
    build_perf = round(triple_num / averageTime) * 1000
    print("load performance(triples/s): " + str(build_perf))

def testQuery(gc):
    for sparql in sparqls:
        file_path = base_path + db_name + "/" + sparql
        sparql_str = open(file_path, 'r').read()
        average = query(gc, sparql_str)
        print("["+sparql+"] average time(ms): " + str(average))
        time.sleep(1)

def testBatchInsertAndBatchRemove(gc):
    res = gc.backup(db_name)
    json_obj = json.loads(res)
    backup_file_path = json_obj['backupfilepath']
    for file in batch_updates:
        startTime = time.perf_counter()
        res = gc.batchRemove(db_name, base_path + db_name + "/" + file)
        endTime = time.perf_counter()
        costTime = (endTime - startTime) * 1000
        print("batch remove [" + file +"] cost time(ms): " + str(costTime))
        json_obj = json.loads(res)
        remove_num = json_obj['success_num']
        remove_perf = round(remove_num / (endTime - startTime))
        print("batch remove [" + file +"] performance(triples/s): " + str(remove_perf))

    for file in batch_updates:
        startTime = time.perf_counter()
        res = gc.batchInsert(db_name, base_path + db_name + "/" + file)
        endTime = time.perf_counter()
        costTime = round((endTime - startTime) * 1000)
        print("batch insert [" + file +"] cost time(ms): " + str(costTime))
        json_obj = json.loads(res)
        insert_num = json_obj['success_num']
        insert_perf = round(insert_num / (endTime - startTime))
        print("batch insert [" + file +"] performance(triples/s): " + str(insert_perf))

# connect to gstore
gc = GstoreConnector.GstoreConnector(IP, Port, username, password)

build(gc)
testLoad(gc)
testQuery(gc)
testBatchInsertAndBatchRemove(gc)
