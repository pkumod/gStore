# -*- coding: UTF-8 -*-
"""
# Filename: POST-example.py
# Author: suxunbin
# Last Modified: 2019-5-15 18:26
# Description: a simple POST-example of python API
"""
import GstoreConnector
import sys
sys.path.append('../src')

# before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
IP = "127.0.0.1"
Port = 9000
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

# start a gc with given IP, Port, username and password
gc = GstoreConnector.GstoreConnector(IP, Port, username, password)

# build a database with a RDF graph
res = gc.build("lubm", "data/lubm/lubm.nt", "POST")
print(res)

# load the database
res = gc.load("lubm", "POST")
print(res)

# to add, delete a user or modify the privilege of a user, operation must be done by the root user
#res = gc.user("add_user", "user1", "111111", "POST")
# print(res);

# show all users
res = gc.showUser("POST")
print(res)

# query
res = gc.query("lubm", "json", sparql, "POST")
print(res)

# query and save the result in a file
gc.fquery("lubm", "json", sparql, filename, "POST")

# save the database if you have changed the database
res = gc.checkpoint("lubm", "POST")
print(res)

# show information of the database
res = gc.monitor("lubm", "POST")
print(res)

# show all databases
res = gc.show("POST")
print(res)

# export the database
res = gc.exportDB("lubm", "export/lubm/lubm_post.nt", "POST")
print(res)

# unload the database
res = gc.unload("lubm", "POST")
print(res)

# drop the database
res = gc.drop("lubm", False, "POST")  # delete the database directly
# res = gc.drop("lubm", True, "POST") #leave a backup
print(res)

# get CoreVersion and APIVersion
res = gc.getCoreVersion("POST")
print(res)
res = gc.getAPIVersion("POST")
print(res)
