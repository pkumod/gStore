## gStore currently includes five executables and others.

**All the commands of gStore should be used in the root directory of gStore like bin/gconsole, because executables are placed in bin/, and they may use some files whose paths are indicated in the code, not absolute paths. We will ensure that all paths are absolute later by asking users to give the absolute path in their own systems to really install/configure the gStore. However, you must do as we told now to avoid errors.**

#### 0. gconsole

gconsole is the main console of gStore, which integrates with all functions to operate on gStore, as well as some system commands. Completion of commands name, line editing features and access to the history list are all provided. Feel free to try it, and you may have a wonderful tour!(spaces or tabs at the beginning or end is ok, and no need to type any special characters as separators)

Just type `bin/gconsole` in the root directory of gStore to use this console, and you will find a `gstore>` prompt, which indicates that you are in native mode and can type in native commands now. There are another mode of this console, which is called remote mode. Just type `connect` in the native mode to enter the remote mode, and type `disconnect` to exit to native mode.(the console connect to a gStore server whose ip is '127.0.0.1' and port is 3305, you can specify them by type `connect gStore_server_ip gStore_server_port`)

You can use `help` or `?` either in native mode or remote mode to see the help information, or you can type `help command_name` or `? command_name` to see the information of a given command. Notice that there are some differences between the commands in native mode and commands in remote mode. For example, system commands like `ls`, `cd` and `pwd` are provided in native mode, but not in remote mode. Also take care that not all commands contained in the help page are totally achieved, and we may change some functions of the console in the future.

What we have done is enough to bring you much convenience to use gStore, just enjoy it!

- - -

#### 1. gbuild

gbuild is used to build a new database from a RDF triple format file.

`# bin/gbuild db_name rdf_triple_file_path`

For example, we build a database from LUBM_10.n3 which can be found in example folder.

    [bookug@localhost gStore]$ bin/gbuild LUBM10.db ./data/LUBM_10.n3 
    2015年05月21日 星期四 20时58分21秒  -0.484698 seconds
    gbuild...
    argc: 3 DB_store:db_LUBM10      RDF_data: ./data/LUBM_10.n3  
    begin encode RDF from : ./data/LUBM_10.n3 ...

- - -

#### 2. gquery

gquery is used to query an existing database with files containing SPARQL queries.(each file contains exact one SPARQL query)

Type `bin/gquery db_name query_file` to execute the SPARQL query retrieved from query_file in the database named db_name.

Use `bin/gquery --help` for detail information of gquery usage.

To enter the gquery console, type `bin/gquery db_name`. The program shows a command prompt("gsql>"), and you can type in a command here. Use `help` to see basic information of all commands, while `help command_t` shows details of a specified command.

Type `quit` to leave the gquery console.

For `sparql` command, input a file path which contains a single SPARQL query. (*answer redirecting to file is supported*)

When the program finish answering the query, it shows the command prompt again. 

*gStore2.0 only support simple "select" queries(not for predicates) now.*

We also take LUBM_10.n3 as an example.

    [bookug@localhost gStore]$ bin/gquery LUBM10.db
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
	Type `help command_t` for detail of command_t
    gsql>sparql ./data/LUBM_q0.sql
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

- "[empty result]" will be printed if no answer, and there is an empty line after all results.

- readline lib is used, so you can use <UP> arrow key in your keyboard to see command history, and use <LEFT> and <RIGHT> arrow key to move and modify your entire command.

- path completion is supported for utility. (not built-in command completion)

- - -

#### 3. gserver

gserver is a daemon. It should be launched first when accessing gStore by gclient or API. It communicates with client through socket. 

    [bookug@localhost gStore]$ bin/gserver 
    port=3305
    Wait for input...

You can also assign a custom port for listening.

    [bookug@localhost gStore]$ bin/gserver 3307
    port=3307
    Wait for input...

Notice: Multiple threads are not supported by gserver. If you start up gclient in more than one terminal in the same time, gserver will go down.

- - -

#### 4. gclient

gclient is designed as a client to send commands and receive feedbacks.

    [bookug@localhost gStore]$ bin/gclient 
    ip=127.0.0.1 port=3305
    gsql>

You can also assign gserver's ip and port.

    [bookug@localhost gStore]$ bin/gclient 172.31.19.15 3307
    ip=172.31.19.15 port=3307
	gsql>

We can use these following commands now:

- `help` shows the information of all commands

- `import db_name rdf_triple_file_name` build a database from RDF triple file

- `load db_name` load an existing database

- `unload db_name` unload database, but will not delete it on disk, you can load it next time

- `sparql "query_string"` query the current database with a SPARQL query string(quoted by "")

- `show` displays the name of the current loaded database

Notice:

- at most one database can be loaded in the gclient console

- you can place ' ' or '\t' between different parts of command, but not use characters like ';'

- you should not place any space or tab ahead of the start of any command

- - -

#### 5. test utilities

A series of test program are placed in the test/ folder, and we will introduce the two useful ones: gtest.cpp and full_test.sh

**gtest is used to test gStore with multiple datasets and queries.**

To use gtest utility, please type `make gtest` to compile the gtest program first. Program gtest is a test tool to generate structural logs for datasets. Please type `./gtest --help` in the working directory for details.

**Please change paths in the test/gtest.cpp if needed.**

You should place the datasets and queries in this way: 

	DIR/WatDiv/database/*.nt 

	DIR/WatDiv/query/*.sql 

Notice that DIR is the root directory where you place all datasets waiting to be used by gtest. And WatDiv is a class of datasets, as well as LUBM. Inside WatDiv(or LUBM, etc. please place all datasets(named with .nt) in a database/ folder, and place all queries(corresponding to datasets, named with .sql) in a query folder.

Then you can run the gtest program with specified parameters, and the output will be sorted into three logs in gStore root directory: load.log/(for database loading time and size), time.log/(for query time) and result.log/(for all query results, not the entire output strings, but the information to record the selected two database systems matched or not).

All logs produced by this program are in TSV format(separated with '\t'), you can load them into Calc/Excel/Gnumeric directly. Notice that time unit is ms, and space unit is kb.

**full_test.sh is used to compare the performance of gStore and other database systems on multiple datasets and queries.**

To use full_test.sh utility, please download the database system which you want to tats and compare, and set the exact position of database systems and datasets in this script. The name strategy should be the same as the requirements of gtest, as well as the logs strategy. 

Only gStore and Jena are tested and compared in this script, but it is easy to add other database systems, if you would like to spend some time on reading this script. You may go to [test report](pdf/gstore测试报告.pdf) or [Frequently Asked Questions](FAQ.md) for help if you encounter a problem.

