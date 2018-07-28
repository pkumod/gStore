"""
# Filename: Benchmark.py
# Author: yangchaofan
# Last Modified: 2018-7-18 15:13
# Description: a simple example of multi-thread query
"""
# before you run this example, make sure that you have started up ghttp service (using bin/ghttp db_name port)

import threading
import sys
sys.path.append('../src')
import GstoreConnector

# variables definition
tnum = 3000
correctness = True

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
def Mythread(rnum, sparql, filename):
    global correctness
    
    # query
    gc = GstoreConnector.GstoreConnector("172.31.222.78", 3305)
    #gc.build("test", "data/lubm/lubm.nt", "root", "123456")
    #gc.load("test", "root", "123456")
    gc.fquery("root", "123456", "test", sparql, filename)
    #res = gc.query("root", "123456", "test", sparql)

    # read the file to a str
    with open(filename, "r") as f:
        res = f.read()

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

#create the threads
for i in range(tnum):
    filename = "result/res" + str(i) + ".txt"
    t = threading.Thread(target=Mythread, args=(result[i%6],sparql[i%6],filename,))
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

print("Main thread exit")
