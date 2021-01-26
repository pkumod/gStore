## gStore currently includes thirteen executables and others.

**All the commands of gStore should be used in the root directory of gStore like bin/gconsole, because executables are placed in bin/, and they may use some files whose paths are indicated in the code, not absolute paths. We will ensure that all paths are absolute later by asking users to give the absolute path in their own systems to really install/configure the gStore. However, you must do as we told now to avoid errors.**

#### 0. Format of data
The RDF data should be given in N-Triple format(XML is not supported by now) and queries must be given in SPARQL 1.1 syntax.
Not all syntax in SPARQL 1.1 are parsered and answered in gStore, for example, property path is beyond the ability of gStore system.
Tabs, '<' and '>' are not allowed to appear in entity, literal or predicates of the data and queries.

- - -

#### 1. gbuild

As long as you download and compile the code of gStore system, a database named `system`(the real directory name is `system.db`) will be created automatically.
This is the database that manages the information of system statistics, including all users and all databases.
You can query this database using `gquery` command, but you are forbidded to modify it using editors.

gbuild is used to build a new database from a RDF triple format file.

`# bin/gbuild db_name rdf_triple_file_path`

For example, we build a database from lubm.nt which can be found in data folder.

    [bookug@localhost gStore]$ bin/gbuild lubm ./data/lubm/lubm.nt 
    gbuild...
    argc: 3 DB_store:lubm      RDF_data: ./data/lubm/lubm.nt  
    begin encode RDF from : ./data/lubm/lubm.nt ...

Notice: 

- You should not build a empty database because this will cause problems.

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

- To ouput the result to disk files, use `sparql ${YOUR_QUERY} > ${YOUR_FILE}` in the console.

- - -

#### 3. ghttp

ghttp runs gStore like HTTP server with port 9000(You need to open this port in your environment, `iptables` tool is suggested). Visit from browser with prescriptive url, then gStore will execute corresponding operation.

type:

`bin/ghttp db_name serverPort` or `bin/ghttp serverPort db_name` to start server with serverPort and load database named db_name initially.

Attention: the argument serverPort or db_name can be left out

if you leave out the argument serverPort in the commond, then the corresponding value will be set to default as 9000.

if you leave out the argument db_name in the commond, then the server will start with no database loaded.


operation: build, load, unload, user, showUser, query, drop, monitor, show, checkpoint, getCoreVersion, getAPIVersion

```
// initialize 
GstoreConnector gc("127.0.0.1", 9000, "root", "123456");

// build a new database by a RDF file.
gc.build("lubm", "data/lubm/lubm.nt");

// load the database that you built.
gc.load("lubm");

//add a user(with username: Jack, password: 2)
gc.user("add_user", "Jack", "2");

//add privilege to user Jack(add_query, add_load, add_unload)
gc.user("add_query", "Jack", "lubm");

//delete privilege of a user Jack(delete_query, delete_load, delete_unload)
gc.user("delete_query", "Jack", "lubm");

//delete user(with username: Jack, password: 2)
gc.user("delete_user", "Jack", "2");

// show all users
gc.showUser();

// query
res = gc.query("lubm", "json", sparql);
std::cout << res << std::endl;

// save updates of a loaded database  
gc.checkpoint("lubm");

// show statistical information of a loaded database
gc.monitor("lubm");

// show all databases already built and if they are loaded
gc.show();

// unload this database.
gc.unload("lubm");

// drop a database already built completely.
gc.drop("lubm", false);

// drop a database already built but leave a backup.
gc.drop("lubm", true);

// get CoreVersion and APIVersion
gc.getCoreVersion();
gc.getAPIVersion();

```

```
db_name: the name of database, like lubm
format: html, json, txt, csv
sparql: select ?s where { ?s ?p ?o . }
ds_path in the server: like /home/data/test.n3
operation: the type of operation: like load, unload, query ...
type: the type of operation that you execute on user, like: add_user, delete_user, add_query, add_load...
username: the username of the user that execute the operation
password: the password of the user that execute the operation
```

`ghttp` support both GET and POST request-type.
`ghttp` support concurrent read-only queries, but when queries containing updates come, the whole database will be locked.
The number of concurrent running queries is suggest to be lower than 300 on a machine with dozens of kernel threads, though we can run 13000 queries concurrently in our experiments.
To use the concurrency feature, you had better modify the system settings of 'open files' and 'maximum processes' to 65535 or larger.
Three scripts are placed in [setup](../scripts/setup/) to help you modify the settings in different Linux distributions.

**If queries containing updates are sent via `ghttp`, you'd better often send a `checkpoint` command to the `ghttp` console. Otherwise, the updates may not be synchronize to disk and will be lost if the `ghttp` server is stopped abnormally.(For example, type "Ctrl+C")**

**Attention: you had better not stop ghttp by simply type the command "Ctrl+C", because this is not safe.

** In order to stop the ghttp server, you can type `bin/shutdown serverPort`
- - -


#### 4. gserver

**This is not maintained now.**

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

**This is not maintained now.**

gclient is designed as a client to send commands and receive feedbacks.

    [bookug@localhost gStore]$ bin/gclient 
    ip=127.0.0.1 port=3305
    gsql>

You can also assign the ip and port of gserver.

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

#### 6. gconsole

**This is not maintained now.**

gconsole is the main console of gStore, which integrates with all functions to operate on gStore, as well as some system commands. Completion of commands name, line editing features and access to the history list are all provided. Feel free to try it, and you may have a wonderful tour!(spaces or tabs at the beginning or end is ok, and no need to type any special characters as separators)

Just type `bin/gconsole` in the root directory of gStore to use this console, and you will find a `gstore>` prompt, which indicates that you are in native mode and can type in native commands now. There are another mode of this console, which is called remote mode. Just type `connect` in the native mode to enter the remote mode, and type `disconnect` to exit to native mode.(the console connect to a gStore server whose ip is '127.0.0.1' and port is 3305, you can specify them by type `connect gStore_server_ip gStore_server_port`)

You can use `help` or `?` either in native mode or remote mode to see the help information, or you can type `help command_name` or `? command_name` to see the information of a given command. Notice that there are some differences between the commands in native mode and commands in remote mode. For example, system commands like `ls`, `cd` and `pwd` are provided in native mode, but not in remote mode. Also take care that not all commands contained in the help page are totally achieved, and we may change some functions of the console in the future.

What we have done is enough to bring you much convenience to use gStore, just enjoy it!

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

- - -

#### 11. gdrop

In order to drop the database, you should not simply type `rm -r db_name.db` because this will not update the built-in database named `system`. Instead, you should type `bin/gdrop db_name`.

---

#### 12. shutdown

After starting ghttp, type `bin/shutdown port` to stop the server instead of simply typing the command "Ctrl+C".
    
---

#### 13. ginit

If you want to restore the initial configuration of the ghttp server, type `bin/ginit` to rebuild the system.db.
    
---

#### 14. test utilities

A series of test program are placed in the `scripts/` folder, and we will introduce the several useful ones: `full_test.sh`, `basic_test.sh`, `update_test.cpp`, `parser_test.sh`, `dataset_test.cpp` and `gtest.cpp`.

**`full_test.sh` is used to compare the performance of gStore and other database systems on multiple datasets and queries.**

To use `full_test.sh` utility, please download the database system which you want to test and compare, and set the exact position of database systems and datasets in this script. The name strategy should be the same as the requirements of gtest, as well as the logs strategy. 

Only gStore and Jena are tested and compared in this script, but it is easy to add other database systems, if you would like to spend some time on reading this script. You may go to [test report](pdf/gstore测试报告.pdf) or [Frequently Asked Questions](FAQ.md) for help if you encounter a problem.

**`basic_test.sh` is used to verify the correctness of build/query/add/sub/drop on several small datasets.**

Just run `bash scripts/basic_test.sh` to use this script.

In fact, `make test` will conduct `basic_test.sh` above, `update_test.cpp` and `parser_test.sh` below.

You are advised to finish this verification each time after you add some modifications and compile again(including the case that you update the code using `git pull`).

**`update_test.cpp` is used to verify the correctness of repeatedly insertion/deletion.**

To use this utility, you will find `update_test` executable under the `scripts/` directory after you compile the whole project with `make`.

Run `scripts/update_test > /dev/null` to finish this test, and you will see the output in the end indicating whether successful or not.

This command will test 10000 groups of insertions/deletions and 1-5 triples for each group by default. In order to change the group number and the group size you can run in the way below:

```
bin/update_test ${YOUR_GROUP_NUMBER} > /dev/null

bin/update_test ${YOUR_GROUP_NUMBER} ${YOUR_GROUP_SIZE} > /dev/null

```

**`parser_test.sh` is used to verify the correctness of parser.**

Just run `bash scripts/parser_test.sh` to use this script.

**`dataset_test.cpp` is used to verify the correctness of build/query on several big datasets.**

If you want to test the correctness of build/query on the big datasets, you can find `dataset_test` executable under the `scripts/` directory after you compile the whole project with `make`.

Run `scripts/dataset_test ${DB_NAME} ${DATASET_PATH} ${QUERY_PATH} ${ANSWER_PATH}> /dev/null` to finish this test, and you will see the output in the end indicating whether successful or not.

Notice: 

- You should place queries and answers in this way: 

	${YOUR_DATA_PATH}*.sql 

	${YOUR_DATA_PATH}*.txt

- ${QUERY_PATH} and ${ANSWER_PATH} need to be ended with '/'.

- Query and its answer need to have the same name, for example, q0.sql and q0.txt.

**gtest is used to test gStore with multiple datasets and queries.**

To use gtest utility, please type `make gtest` to compile the gtest program first. Program gtest is a test tool to generate structural logs for datasets. Please type `./gtest --help` in the working directory for details.

**Please change paths in the test/gtest.cpp if needed.**

You should place the datasets and queries in this way: 

	DIR/WatDiv/database/*.nt 

	DIR/WatDiv/query/*.sql 

Notice that DIR is the root directory where you place all datasets waiting to be used by gtest. And WatDiv is a class of datasets, as well as lubm. Inside WatDiv(or lubm, etc. please place all datasets(named with .nt) in a database/ folder, and place all queries(corresponding to datasets, named with .sql) in a query folder.

Then you can run the gtest program with specified parameters, and the output will be sorted into three logs in gStore root directory: load.log/(for database loading time and size), time.log/(for query time) and result.log/(for all query results, not the entire output strings, but the information to record the selected two database systems matched or not).

All logs produced by this program are in TSV format(separated with '\t'), you can load them into Calc/Excel/Gnumeric directly. Notice that time unit is ms, and space unit is kb.

