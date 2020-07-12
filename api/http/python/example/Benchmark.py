"""
# Filename: Benchmark.py
# Author: suxunbin
# Last Modified: 2019-5-15 20:11
# Description: a simple example of multi-thread query
"""
import GstoreConnector
import threading
import sys
sys.path.append('../src')

# before you run this example, make sure that you have started up ghttp service (using bin/ghttp db_name port)
# default db_name: lubm(must be built in advance)

IP = "127.0.0.1"
Port = 9000
username = "root"
password = "123456"
tnum = 1000
correctness = True
RequestType = "POST"

threads = []
result = [15, 0, 828, 27, 27, 5916]
sparql = []
sparql0 = "select ?x where\
            {\
             ?x   <ub:name> <FullProfessor0> .\
            }"
sparql1 = "select distinct ?x where\
            {\
            ?x <rdf:type>  <ub:GraduateStudent>.\
            ?y <rdf:type>  <ub:GraduateStudent>.\
            ?z <rdf:type>  <ub:GraduateStudent>.\
            ?x <ub:memberOf>  ?z.\
            ?z <ub:subOrganizationOf> ?y.\
            ?x <ub:undergaduateDegreeFrom> ?y.\
            }"
sparql2 = "select distinct ?x where\
            {\
            ?x   <rdf:type>  <ub:Course>.\
            ?x   <ub:name>   ?y.\
            }"
sparql3 = "select ?x where\
            {\
            ?x   <rdf:type>  <ub:UndergraduateStudent>.\
            ?y   <ub:name>  <Course1>.\
            ?x   <ub:takesCourse>  ?y.\
            ?z   <ub:teacherOf>  ?y.\
            ?z   <ub:name>  <FullProfessor1>.\
            ?z   <ub:worksFor>   ?w.\
            ?w   <ub:name>    <Department0>.\
            }"
sparql4 = "select distinct ?x where\
            {\
            ?x   <rdf:type>  <ub:UndergraduateStudent>.\
            ?y   <ub:name>   <Course1>.\
            ?x   <ub:takesCourse>  ?y.\
            ?z   <ub:teacherOf>  ?y.\
            ?z   <ub:name>  <FullProfessor1>.\
            ?z   <ub:worksFor>  ?w.\
            ?w   <ub:name>  <Department0>.\
            }"
sparql5 = "select distinct ?x where\
            {\
            ?x    <rdf:type>    <ub:UndergraduateStudent>.\
            }"

# thread function


def Mythread(rnum, sparql, filename, RequestType):
    global correctness

    # query
    gc = GstoreConnector.GstoreConnector(IP, Port, username, password)
    res = gc.query("lubm", "json", sparql, RequestType)

    # fquery
    #gc = GstoreConnector.GstoreConnector(IP, Port, username, password)
    #gc.fquery("lubm", "json", sparql, filename, RequestType)
    # with open(filename, "r") as f:
    #    res = f.read()

    # count the nums
    m = 0
    for i in range(len(sparql)):
        if (sparql[i] == "?"):
            m = m + 1
        if (sparql[i] == "{"):
            break
    n = 0
    for i in range(len(res)):
        if (res[i] == "{"):
            n = n + 1
    Num = (n-3)/(m+1)

    # compare the result
    if (rnum != Num):
        correctness = False
        print("sparql: "+sparql)
        print("Num: "+str(Num))


# create sparql
sparql.append(sparql0)
sparql.append(sparql1)
sparql.append(sparql2)
sparql.append(sparql3)
sparql.append(sparql4)
sparql.append(sparql5)

# create the threads
for i in range(tnum):
    filename = "result/res" + str(i) + ".txt"
    t = threading.Thread(target=Mythread, args=(
        result[i % 6], sparql[i % 6], filename, RequestType,))
    threads.append(t)

# start threads
for i in threads:
    i.start()

# wait for the threads
for i in threads:
    i.join()

if (correctness == True):
    print("The answers are correct!")
else:
    print("The answers exist errors!")
