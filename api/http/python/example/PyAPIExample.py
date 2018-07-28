"""
# Filename: PyAPIExample.py
# Author: yangchaofan
# Last Modified: 2018-7-18 15:10
# Description: a simple example of python API
"""
import sys
sys.path.append('../src')
import GstoreConnector

# before you run this example, make sure that you have started up ghttp service (using bin/ghttp db_name port)
username = "root"
password = "123456"
sparql = "select ?x where \
                 { \
                     ?x  <rdf:type> <ub:UndergraduateStudent>. \
                     ?y    <ub:name> <Course1>. \
                     ?x    <ub:takesCourse>  ?y. \
                     ?z   <ub:teacherOf>    ?y. \
                     ?z    <ub:name> <FullProfessor1>. \
                     ?z    <ub:worksFor>    ?w. \
                     ?w    <ub:name>    <Department0>. \
                 }"
filename = "res.txt"

# start a gc with given IP and Port
gc =  GstoreConnector.GstoreConnector("172.31.222.78", 3305)

# unload the database
#ret = gc.unload("test", username, password)

# build database with a RDF graph
ret = gc.build("test", "data/lubm/lubm.nt", username, password)

# load the database 
ret = gc.load("test", username, password)

# query
print (gc.query(username, password, "test", sparql))

# query and save the result in a file
gc.fquery(username, password, "test", sparql, filename)


