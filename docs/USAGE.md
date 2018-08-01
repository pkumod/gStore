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

For example, we build a database from lubm.nt which can be found in example folder.

    [bookug@localhost gStore]$ bin/gbuild lubm ./data/lubm/lubm.nt 
    2017年11月23日 星期四 20时58分21秒  -0.484698 seconds
    gbuild...
    argc: 3 DB_store:lubm      RDF_data: ./data/lubm/lubm.nt  
    begin encode RDF from : ./data/lubm/lubm.nt ...

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

We also take lubm.nt as an example.

    [bookug@localhost gStore]$ bin/gquery lubm
    gquery...
    argc: 2 DB_store:lubm/
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
    gsql>sparql ./data/lubm/lubm_q0.sql
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

#### 3. ghttp

ghttp runs gStore like HTTP server with port 9000. Visit from browser with prescriptive url, then gStore will execute corresponding operation.

type:

`bin/ghttp db_name serverPort` or `bin/ghttp serverPort db_name` to start server with serverPort and load database named db_name initially.

(*Attention: the argument db_name and serverPort can be left out*)
Attention: the argument serverPort can be left out

if you leave out the argument serverPort in the commond, then the corresponding value will be set to default as 9000.

(*if you leave out the argument db_name in the commond, then the db_name will set to null which means not to load a database initially.*)

URL rules are listed blow:  

parameters: operation, db_name, ds_path, format, sparql, type, username, password

NOTICE: do URL encoding before sending it to database server

operation: build, load, unload, query, monitor, show, checkpoint, user
// build a new database by a RDF file.
"http://127.0.0.1:9000/?operation=build&db_name=lubm&ds_path=data/lubm/lubm.nt&username=root&password=123456"

// load databse
"http://127.0.0.1:9000/?operation=load&db_name=lumb&username=root&password=123456"

// then you can execute SPARQL query on this database.
"http://127.0.0.1:9000/?operation=query&username=root&password=123456&db_name=lubm&format=json&sparql="

// output information of a database
"http://127.0.0.1:9000/?operation=monitor&db_name=lubm"


// unload this databse
"http://127.0.0.1:9000/?operation=unload&db_name=lubm&username=root&password=123456"

//add a user(with username: Jack, passwor: 2)
"http://127.0.0.1:9000/?operation=user&type=add_user&username1=root&password1=123456&username2=Jack&addtion=2"

//add privilege to user Jack(add_query, add_load, add_unload)
"http://127.0.0.1:9000/?operation=user&type=add_query&username1=root&password1=123456&username2=Jack&addtion=lubm"

//delete privilege of a user Jack(delete_query, delete_load, delete_unload)
"http://127.0.0.1:9000/?operation=user&type=delete_query&username1=root&password1=123456&username2=Jack&addtion=lubm"

//delete user(with username: Jack, password: 2)
"http://127.0.0.1:9000/?operation=user&type=delete_user&username1=root&password1=123456&username2=Jack&addtion=2"
db_name: the name of database, like lubm
format: html, json, txt, csv
sparql: select ?s where { ?s ?p ?o . }
ds_path in the server: like /home/data/test.n3
operation: the type of operation: like load, unload, query ...
type: the type of operation that you execute on user, like: add_user, delete_user, add_query, add_load...
username: the username of the user that execute the operation
password: the password of the user that execute the operation

- - -


#### 4. gserver

gserver is a daemon. It should be launched first when accessing gStore by gclient or API. It communicates with client through socket. 

    [bookug@localhost gStore]$ bin/gserver -s
	Server started at port 3305.

You can also assign a custom port for listening.

    [bookug@localhost gStore]$ bin/gserver -s -p 3307
	Server started at port 3307.

You can stop the server by:

    [bookug@localhost gStore]$ bin/gserver -t
	Server stopped at port 3305.

Notice: Multiple threads are not supported by gserver. If you start up gclient in more than one terminal in the same time, gserver will go down.

- - -

#### 5. gclient

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



#### 6. test utilities

A series of test program are placed in the test/ folder, and we will introduce the two useful ones: gtest.cpp and full_test.sh

**gtest is used to test gStore with multiple datasets and queries.**

To use gtest utility, please type `make gtest` to compile the gtest program first. Program gtest is a test tool to generate structural logs for datasets. Please type `./gtest --help` in the working directory for details.

**Please change paths in the test/gtest.cpp if needed.**

You should place the datasets and queries in this way: 

	DIR/WatDiv/database/*.nt 

	DIR/WatDiv/query/*.sql 

Notice that DIR is the root directory where you place all datasets waiting to be used by gtest. And WatDiv is a class of datasets, as well as lubm. Inside WatDiv(or lubm, etc. please place all datasets(named with .nt) in a database/ folder, and place all queries(corresponding to datasets, named with .sql) in a query folder.

Then you can run the gtest program with specified parameters, and the output will be sorted into three logs in gStore root directory: load.log/(for database loading time and size), time.log/(for query time) and result.log/(for all query results, not the entire output strings, but the information to record the selected two database systems matched or not).

All logs produced by this program are in TSV format(separated with '\t'), you can load them into Calc/Excel/Gnumeric directly. Notice that time unit is ms, and space unit is kb.

**full_test.sh is used to compare the performance of gStore and other database systems on multiple datasets and queries.**

To use full_test.sh utility, please download the database system which you want to tats and compare, and set the exact position of database systems and datasets in this script. The name strategy should be the same as the requirements of gtest, as well as the logs strategy. 

Only gStore and Jena are tested and compared in this script, but it is easy to add other database systems, if you would like to spend some time on reading this script. You may go to [test report](pdf/gstore测试报告.pdf) or [Frequently Asked Questions](FAQ.md) for help if you encounter a problem.

- - - 

#### 7. gadd

gadd is used to insert triples in a file to an existing database.

Usage: bin/gadd db_name rdf_triple_file_path

    [bookug@localhost gStore]$ bin/gadd lubm ./data/lubm/lubm.nt
    ...
    argc: 3 DB_store:lubm   insert file:./data/lubm/lubm.nt
    get important pre ID
    ...
    insert rdf triples done.
    inserted triples num: 99550

- - -

#### 8. gsub

gsub is used to remove triples in a file from an existing database.

Usage: bin/gsub db_name rdf_triple_file_path

    [bookug@localhost gStore]$ bin/gsub lubm ./data/lubm/lubm.nt
    ...
    argc: 3 DB_store:lubm  remove file: ./data/lubm/lubm.nt
    ...
    remove rdf triples done.
    removed triples num: 99550

- - - 

#### 9. gmonitor

After starting ghttp, type `bin/gmonitor ip port db_name` to check current status of database db_name of gStore.

    [bookug@localhost gStore]$ bin/gmonitor 127.0.0.1 9000 lubm
    parameter: ?operation=monitor&db_name=lubm
    request: http://127.0.0.1:9000/%3Foperation%3Dmonitor%26db_name%3Dlubm
    null--->[HTTP/1.1 200 OK]
    Content-Length--->[127]
    database: lubm
    triple num: 99550
    entity num: 28413
    literal num: 0
    subject num: 14569
    predicate num: 17
    connection num: 7

- - -

#### 10. gshow

After starting ghttp, type `bin/gshow ip port` to check loaded database.

    [bookug@localhost gStore]$ bin/gshow 127.0.0.1 9000
    parameter: ?operation=show
    request: http://127.0.0.1:9000/%3Foperation%3Dshow
    null--->[HTTP/1.1 200 OK]
    Content-Length--->[4]
    database: lubm
    
