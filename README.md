#gStoreD

## gStoreD 1.2

### Overview
gStoreD is a distributed RDF data management system (or what is commonly called a "triple store") that is based on "partial evaluation" and maintains the graph structure of the original [RDF](http://www.w3.org/TR/rdf11-concepts/) data. Its data model is a labeled, directed multiedge graph, where each vertex corresponds to a subject or an object. We also represent a given [SPARQL](http://www.w3.org/TR/sparql11-overview/) query by a query graph Q. Query processing involves finding subgraph matches of Q over the RDF graph G. 

### Install Steps
System Requirement: 64-bit linux server with GCC, mpich-3.0.4, make, readline installed.
*We have tested on linux server with CentOS 6.2 x86_64 and CentOS 6.6 x86_64. The version of GCC should be 4.4.7 or later.*

You can install gStoreD 1.2 in one command. Just run

`# make` 

to compile the gStore code and build executable "gloadD", "gqueryD".

### Usage
gStoreD 1.2 currently includes four executables and others.

####1. gloadD
gloadD is used to build a new database from a RDF triple format file.

`# mpiexec -f host_file_name -n host_number + 1 ./gloadD db_name rdf_triple_file_name internal_vertices_file_name`

The host file is used to idenitfy the hosts. "host_number + 1" means that there should be one more process as the master site.

Each line in the file of the internal vertices consists of two parts: the first part is the URI of an resouce, and the second part is the identifier of the partition that the resouce belongs to. The tab "\t" is used as the separator.

For example, we build a database from dbpedia_example_distgStore.n3 which can be found in example folder.

    [root@master Gstore]# mpiexec -f hosts.txt -n 5 ./gloadD db_dbpedia_example_distgStore ./example/dbpedia_example_distgStore.n3 ./example/dbpedia_example_distgStore_internal.TXT

####2. gqueryD
gqueryD is used to query an exsisting database with SPARQL files.

`mpiexec -f host_file_name -n host_number + 1 ./gqueryD db_name query_file_name`

When the program finish answering the query, the SPARQL matches are written in the file named `finalRes.txt`.

*gStoreD 1.1 support simple "select" queries and "ask" queries now.*

We also take dbpedia_example_distgStore.n3 as an example.

    [root@master Gstore]# mpiexec -f hosts.txt -n 5 ./gqueryD db_dbpedia_example_distgStore ./example/query.txt
   
Notice: 

All results are output into finalRes.txt.

If you encounter any problems, please send emails to me (email address: pku09pp@pku.edu.cn).