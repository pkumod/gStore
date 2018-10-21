# used for demo: display the basic operations of gStore in terminal, like gbuild, gquery, gadd and gsub, [ghttp] if needed

#1. The ability of managing large datasets
#NOTICE: this is very slow, so build the database in advance
#bin/gbuild LUBM500M /home/data/LUBM/database/LUBM500M.nt >& LUBM500M.log/load.txt
#NOTICE: this is too slow due to the time cost of 'load'
#bin/gquery LUBM500M /home/data/LUBM/query/q1.sql >& LUBM500M.log/q1.txt
#
# WARN: please enter the console in advance, and run these queries several times to warmup the system
#To enter the gquery console
bin/gquery LUBM500M
gsql>sparql /home/data/LUBM/query/q1.sql > q1.txt
#to view some results of q1 in another terminal
head q1.txt
gsql>sparql /home/data/LUBM/query/q2.sql > q2.txt
#to view some results of q2 in another terminal
head q2.txt

#2. The ability of handling updates
bin/gbuild lubm data/lubm/lubm.nt
#we will see 15 lines of results
bin/gquery lubm data/lubm/lubm_q0.sql
#all triples removed
bin/gsub lubm data/lubm/lubm.nt
#we will see 0 lines of results
bin/gquery lubm data/lubm/lubm_q0.sql
#re-insert all triples of the original dataset
bin/gadd lubm data/lubm/lubm.nt
#we will see exactly 15 lines of results as before
bin/gquery lubm data/lubm/lubm_q0.sql

#3. The ability of HTTP web server, as an endpoint
bin/ghttp lubm
#visit 172.31.222.78:9000, click and query

