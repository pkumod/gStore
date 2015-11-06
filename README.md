#gStore

## gStore2.0

### Overview
gStore is a graph-based RDF data management system (or what is commonly called a “triple store”) that maintains the graph structure of the original [RDF](http://www.w3.org/TR/rdf11-concepts/) data. Its data model is a labeled, directed multiedge graph, where each vertex corresponds to a subject or an object. We also represent a given [SPARQL](http://www.w3.org/TR/sparql11-overview/) query by a query graph Q. Query processing involves finding subgraph matches of Q over the RDF graph G. gStore incorporates an index over the RDF graph (called VS-tree) to speed up query processing. VS-tree is a heightbalanced tree with a number of associated pruning techniques to speed up subgraph matching.

### Install Steps
System Requirement: 64-bit linux server with GCC, make, readline installed.
*We have tested on linux server with CentOS 6.2 x86_64 and CentOS 6.6 x86_64. The version of GCC should be 4.4.7 or later.*

You can install gStore2.0 in one command. Just run

`# make` 

to compile the gStore code and build executable "gload", "gquery", "gserver", "gclient".

### Usage
gStore2.0 currently includes four executables and others.

####1. gload
gload is used to build a new database from a RDF triple format file.

`# ./gload db_name rdf_triple_file_name`

For example, we build a database from LUBM_10.n3 which can be found in example folder.

    [root@master Gstore]# ./gload db_LUBM10 ./example/LUBM_10.n3 
    2015年05月21日 星期四 20时58分21秒  -0.484698 seconds
    gload...
    argc: 3 DB_store:db_LUBM10      RDF_data: ./example/LUBM_10.n3  
    begin encode RDF from : ./example/LUBM_10.n3 ...

####2. gquery
gquery is used to query an exsisting database with SPARQL files.

`./gquery db_name`  (use `./gquery --help` for detail)

The program shows a command prompt("gsql>"): you can type in a command here, use `help` to see information of all commands.

For `sparql` command, input a file name which can be interpreted as a single SPARQL query. 

When the program finish answering the query, it shows the command prompt again. 

*gStore2.0 only support simple “select” queries now.*

We also take LUBM_10.n3 as an example.

    [root@master Gstore]# ./gquery db_LUBM10/
    gquery...
    argc: 2 DB_store:db_LUBM10/
    loadTree...
    LRUCache initial...
    LRUCache initial finish
    finish loadCache
    finish loadEntityID2FileLineMap
    open KVstore
    finish load
    finish loading
    Type `help` for information of all commands
    gsql>sparql ./example/LUBM_q0.txt
    ... ...
    Total time used: 4ms.
    final result is : 
    <http://www.Department0.University0.edu/FullProfessor0>
    <http://www.Department1.University0.edu/FullProfessor0>
    <http://www.Department2.University0.edu/FullProfessor0>
    <http://www.Department3.University0.edu/FullProfessor0>
    <http://www.Department4.University0.edu/FullProfessor0>
    <http://www.Department5.University0.edu/FullProfessor0>
    <http://www.Department6.University0.edu/FullProfessor0>
    <http://www.Department7.University0.edu/FullProfessor0>
    <http://www.Department8.University0.edu/FullProfessor0>
    <http://www.Department9.University0.edu/FullProfessor0>
    <http://www.Department10.University0.edu/FullProfessor0>
    <http://www.Department11.University0.edu/FullProfessor0>
    <http://www.Department12.University0.edu/FullProfessor0>
    <http://www.Department13.University0.edu/FullProfessor0>
    <http://www.Department14.University0.edu/FullProfessor0>

Notice: 

"[empty result]" will be printed if no answer, and there is an empty line after all results.

Readline lib is used, so you can use <UP> arrow key to see command history, and use <LEFT> arrow key to move and modify your entire command.

What is more, path completion is supported for utility. (not built-in command completion)

####3. gserver
gserver is a daemon. It should be launched first when accessing gStore by gclient or API. It communicates with client through socket. 

    [root@master Gstore]# ./gserver 
    port=3305
    Wait for input...

You can also assign a custom port for listening.

    [root@master Gstore]# ./gserver 3307
    port=3307
    Wait for input...

####4. gclient
gclient is designed as a client to send commands and receive feedbacks.

    [root@master Gstore]# ./gclient 
    ip=127.0.0.1 port=3305
    ->

You can also assign gserver's ip and port.

    [root@centos74 Gstore]# ./gclient 172.31.19.15 3307
    ip=172.31.19.15 port=3307
    ->


We can use these following commands now:

`->import db_name rdf_triple_file_name;`
build a database from RDF triple file.

`->load db_name;`
load an exsisting database.

`->unload db_name;`
unload database, but will not delete it on disk, you can load it next time.

`->query SPARQL;`
query the current database with a SPARQL query.

`->show databases;`
show the current database's name.

`->insert db_name rdf_triple_file_name;`
insert a handful of new RDF triples to the orginal database.

Note that each command ends with ";".

####5. API
We provide JAVA and C++ API for accessing gStore now. Please refer to example codes in `api/cpp/example` and `api/java/example`.

####6. gtest
We provide a test tool to generate structural logs for datasets. Please type `./gtest --help` in the working directory for detail.

(change paths in the main/gtest.cpp if needed)

You should place the datasets and queries this way: 

DIR/WatDiv/database/*.nt 

DIR/WatDiv/query/*.sql 

And the output will be sorted into there logs: load.log/(for database loading time and size), time.log/(for query time) and result.log/(for all query results).

All logs produced by this program are in TSV format, you can load them into Calc/Excel directly.

### System Update Log

####Nov 06, 2015.

We merge several classes(like Bstr) and adjust the project structure, as well as the debug system. 

In addition, most warnnings are removed, except for warnnings in Parser module, which is due to the use of ANTLR.

What is more, we change RangeValue module to Stream, and add Stream for ResultSet. We also better the gquery console, so now you can redirect query results to a specified file in the gsql console.

Unable to add Stream for IDlist due to complex operations, but this is not necessary. Realpath is used to supported soft links in the gquery console, but it not works in Gstore.(though works if not in Gtsore)

####Oct 20, 2015.

We add a gtest tool for utility, you can use it to query several datasets with their own queries.

In addition, gquery console is improved. Readline lib is used for input instead of fgets, and the gquery console can support commands history, modifying command and commands completion now.

What is more, we found and fix a bug in Database/(a pointer for debugging log is not set to NULL after fclose operation, so if you close one databse and open another, the system will fail entirely because the system think that the debugging log is still open)

####Sep 25, 2015. 

We implement the version of B+Tree, and replace the old one.

After testing on DBpedia, LUBM, and WatDiv benchmark, we conclude that the new BTree performs more efficient than
the old version. For the same triple file, the new version spends shorter time on executing gload command.

Besides, the new version can handle the long literal objects ocasion efficiently, while triples whose object's length exceeds 4096 bytes result in frequent inefficent split operations on the old version BTree. 

####Feb 2, 2015

We modify the RDF parser and SPARQL parser.

Under the new RDF parser, we also redesign the encode strategy, which reduces RDF file scanning times.

Now we can parse the standard SPARQL v1.1 grammar correctly, and can support basic graph pattern(BGP) SPARQL queries written by this standard grammar.

####Dec 11, 2014.

We add API for C/CPP and JAVA.

####Nov 20, 2014.

We share our gStore2.0 code as an open-source project under BSD license on github.
