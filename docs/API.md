**This Chapter guides you to use our API for accessing gStore. We provide HTTP api(suggested) and socket api, corresponding to ghttp and gserver respectively.**

# HTTP API

Compired with socket API, HTTP API is more stable and more standard, and can maintain connection. Socket API can not guaratee correct transmission, so the network transmission is faster.

## Easy Examples

We provide C++, java, python, php and nodejs API for ghttp now. Please refer to example codes in `api/http/cpp`, `api/http/java`, `api/http/python`, `api/http/php` and `api/http/nodejs`. To use these examples, please make sure that executables have already been generated.

Next, **start up ghttp service by using \texttt{bin/ghttp} command.** It is ok if you know a running usable ghttp server and try to connect to it. (you do not need to change anything if using examples, just by default). Then, for C++ and java code, you need to compile the example codes in the directory `api/http/cpp/example` and `api/http/java/example`. 

Finally, go to the example directory and run the corresponding executables. All these four executables will connect to a specified ghttp server and do some load or query operations. Be sure that you see the query results in the terminal where you run the examples, otherwise please go to [Frequently Asked Questions](FAQ.md) for help or report it to us.(the report approach is described in [README](../README.md))

You are advised to read the example code carefully, as well as the corresponding Makefile. This will help you to understand the API, specially if you want to write your own programs based on the API interface.

- - -

## API structure

The HTTP API of gStore is placed in api/http directory in the root directory of gStore, whose contents are listed below:

- gStore/api/http/
	
	- cpp/ (the C++ API)
		
		- client.cpp (source code of C++ API)

		- client.h

		- example/ (small example program to show the basic idea of using the C++ API)
			- GET-example.cpp

			- Benchmark.cpp

			- POST-example.cpp

			- Makefile

		- Makefile (compile and build lib)
		
	- java/ (the Java API)

		- client.java

		- lib/

		- src/

			- Makefile

			- jgsc/

				- GstoreConnector.java

		- example/

			- Benckmark.java

            - GETexample.java

			- POSTexample.java

			- Makefile

    - python/ (the Python API)

        - example/

            - Benchmark.py

            - GET-example.py

            - POST-example.py

        - src/ 

            - GstoreConnector.py

    - nodejs/ (the Nodejs API)

        - GstoreConnector.js (source code of Nodejs API)

        - LICENSE

        - package.json

        - README.md

        - example/ (small example to show the basic idea of using the Nodejs API)		

            - POST-example.js

            - GET-example.js		
			
    - php/ (the Php API)

        - example/

            - Benchmark.php

            - POST-example.php

            - GET-example.php

        - src/
            
            - GstoreConnector.php
- - -

## C++ API

#### Interface

To use the C++ API, please place the phrase `#include "client.h"` in your cpp code. Functions in client.h should be called like below:

```
// initialize 
GstoreConnector gc("127.0.0.1", 9000, "root", "123456");

// build a new database by a RDF file.
// note that the relative path is related to the server
gc.build("lubm", "data/lubm/lubm.nt");

// load the database that you built.
gc.load("lubm");

// to add, delete a user or modify the privilege of a user, operation must be done by the root user
gc.user("add_user", "user1", "111111");

// show all users
gc.showUser();

// query
std::string res = gc.query("lubm", "json", sparql);
std::cout << res << std::endl;

// query and save the result in a file called "ans.txt"
gc.fquery("lubm", "json", sparql, "ans.txt");

// save the database if you have changed the database
gc.checkpoint("lubm");

// show information of the database
gc.monitor("lubm");

// show all databases
gc.show();

// unload this database.
gc.unload("lubm");

// also, you can load some exist database directly and then query.
gc.load("lubm");
res = gc.query("lubm", "json", sparql);
std::cout << res << std::endl;
gc.unload("lubm");

// drop the database directly
gc.drop("lubm", false);

// drop the database and leave a backup
gc.drop("lubm", true);

// get CoreVersion and APIVersion
gc.getCoreVersion();
gc.getAPIVersion();

```
The original declaration of these functions are as below:

```
If request_type is "GET", the last parameter can be omitted:

GstoreConnector(std::string serverIP, int serverPort, std::string username, std::string password);

std::string build(std::string db_name, std::string rdf_file_path, std::string request_type);

std::string load(std::string db_name, std::string request_type);

std::string unload(std::string db_name, std::string request_type);

std::string user(std::string type, std::string username2, std::string addition, std::string request_type);

std::string showUser(std::string request_type);

std::string query(std::string db_name, std::string format, std::string sparql, std::string request_type);

std::string drop(std::string db_name, bool is_backup, std::string request_type);

std::string monitor(std::string db_name, std::string request_type);

std::string checkpoint(std::string db_name, std::string request_type);

std::string show(std::string request_type);

std::string getCoreVersion(std::string request_type);

std::string getAPIVersion(std::string request_type);

void fquery(std::string db_name, std::string format, std::string sparql, std::string filename, std::string request_type);

```

- - -

## Java API

#### Interface

To use the Java API, please see gStore/api/http/java/src/jgsc/GstoreConnector.java. Functions should be called like below:

```
// initialize
GstoreConnector gc = new GstoreConnector("127.0.0.1", 9000, "root", "123456");

// build a new database by a RDF file.
// note that the relative path is related to the server
gc.build("lubm", "data/lubm/lubm.nt");

// load the database that you built.
gc.load("lubm");

// to add, delete a user or modify the privilege of a user, operation must be done by the root user
gc.user("add_user", "user1", "111111");

// show all users
gc.showUser();

// query
String res = gc.query("lubm", "json", sparql);
System.out.println(res);

// query and save the result in a file called "ans.txt"
gc.fquery("lubm", "json", sparql, "ans.txt");

// save the database if you have changed the database
gc.checkpoint("lubm");

// show information of the database
gc.monitor("lubm");

// show all databases
gc.show();

// unload this database.
gc.unload("lubm");

// also, you can load some exist database directly and then query.
gc.load("lubm");
res = gc.query("lubm", "json", sparql);
System.out.println(res);
gc.unload("lubm");

// drop the database directly
gc.drop("lubm", false);

// drop the database and leave a backup
gc.drop("lubm", true);

// get CoreVersion and APIVersion
gc.getCoreVersion();
gc.getAPIVersion();

```


The original declaration of these functions are as below:

```
If request_type is "GET", the last parameter can be omitted:

public class GstoreConnector(String serverIP, int serverPort, String username, String password);

public String build(String db_name, String rdf_file_path, String request_type);

public String load(String db_name, String request_type);

public String unload(String db_name, String request_type);

public String user(String type, String username2, String addition, String request_type);

public String showUser(String request_type);

public String query(String db_name, String format, String sparql, String request_type);

public void fquery(String db_name, String format, String sparql, String filename, String request_type);

public String drop(String db_name, boolean is_backup, String request_type);

public String monitor(String db_name, String request_type);

public String checkpoint(String db_name, String request_type);

public String show(String request_type);

public String getCoreVersion(String request_type);

public String getAPIVersion(String request_type);

```

- - -

## Python API

#### Interface

To use Python API, please see gStore/api/http/python/src/GstoreConnector.py. Functions should be called like following:

```
# start a gc with given IP, Port, username and password
gc =  GstoreConnector.GstoreConnector("127.0.0.1", 9000, "root", "123456")

# build a database with a RDF graph
res = gc.build("lubm", "data/lubm/lubm.nt")

# load the database 
res = gc.load("lubm")

# to add, delete a user or modify the privilege of a user, operation must be done by the root user
res = gc.user("add_user", "user1", "111111")

# show all users
res = gc.showUser()

# query
res = gc.query("lubm", "json", sparql)
print(res)

# query and save the result in a file called "ans.txt"
gc.fquery("lubm", "json", sparql, "ans.txt")

# save the database if you have changed the database
res = gc.checkpoint("lubm")

# show information of the database
res = gc.monitor("lubm")

# show all databases
res = gc.show()

# unload the database
res = gc.unload("lubm")

# drop the database directly
res = gc.drop("lubm", False)

# drop the database and leave a backup
res = gc.drop("lubm", True)

# get CoreVersion and APIVersion
res = gc.getCoreVersion();
res = gc.getAPIVersion();

```
The original declaration of these functions are as below:

```
If request_type is "GET", the last parameter can be omitted:

public class GstoreConnector(self, serverIP, serverPort, username, password):

def build(self, db_name, rdf_file_path, request_type):

def load(self, db_name, request_type):

def unload(self, db_name, request_type):

def user(self, type, username2, addition, request_type):

def showUser(self, request_type):

def query(self, db_name, format, sparql, request_type):

def fquery(self, db_name, format, sparql, filename, request_type):

def drop(self, db_name, is_backup, request_type):

def monitor(self, db_name, request_type):    

def checkpoint(self, db_name, request_type):

def show(self, request_type):

def getCoreVersion(self, request_type):

def getAPIVersion(self, request_type):

```

- - -

## Nodejs API

#### Interface

Before using Nodejs API, type `npm install request` and `npm install request-promise` under the nodejs folder to add the required module.

To use Nodejs API, please see gStore/api/http/nodejs/GstoreConnector.js. Functions should be called like following:

```
// start a gc with given IP, Port, username and password
gc =  new GstoreConnector("127.0.0.1", 9000, "root", "123456");

// build a database with a RDF graph
res = gc.build("lubm", "data/lubm/lubm.nt");

// load the database 
res = gc.load("lubm");

// to add, delete a user or modify the privilege of a user, operation must be done by the root user
res = gc.user("add_user", "user1", "111111");

// show all users
res = gc.showUser();

// query
res = gc.query("lubm", "json", sparql);
console.log(JSON.stringify(res,","));

// save the database if you have changed the database
res = gc.checkpoint("lubm");

// show information of the database
res = gc.monitor("lubm");

// show all databases
res = gc.show();

// unload the database
res = gc.unload("lubm");

// drop the database directly
res = gc.drop("lubm", false);

// drop the database and leave a backup
res = gc.drop("lubm", true);

// get CoreVersion and APIVersion
res = gc.getCoreVersion();
res = gc.getAPIVersion();

```
The original declaration of these functions are as below:

```
If request_type is "GET", the last parameter can be omitted:

class GstoreConnector(ip = '', port, username = '', password = '');

async build(db_name = '', rdf_file_path = '', request_type);

async load(db_name = '', request_type);

async unload(db_name = '', request_type);

async user(type = '', username2 = '' , addition = '' , request_type);

async showUser(request_type);

async query(db_name = '', format = '' , sparql = '' , request_type);

async drop(db_name = '', is_backup , request_type);

async monitor(db_name = '', request_type);    

async checkpoint(db_name = '', request_type);

async show(request_type);

async getCoreVersion(request_type);

async getAPIVersion(request_type);

```

- - -

## Php API

#### Interface

To use Php API, please see gStore/api/http/php/src/GstoreConnector.php. Functions should be called like following:

```
// start a gc with given IP, Port, username and password
$gc = new GstoreConnector("127.0.0.1", 9000, "root", "123456");

// build a database with a RDF graph
$res = $gc->build("lubm", "data/lubm/lubm.nt");
echo $res . PHP_EOL;

// load rdf
$ret = $gc->load("test", $username, $password);
echo $ret . PHP_EOL;

// to add, delete a user or modify the privilege of a user, operation must be done by the root user
//$res = $gc->user("add_user", "user1", "111111");
//echo $res . PHP_EOL;

// show all users
$res = $gc->showUser();
echo $res. PHP_EOL;

// query
$res = $gc->query("lubm", "json", $sparql);
echo $res. PHP_EOL;

// query and save the result in a file called "ans.txt"
$gc->fquery("lubm", "json", $sparql, "ans.txt");

// save the database if you have changed the database
$res = $gc->checkpoint("lubm");
echo $res. PHP_EOL;

// show information of the database
$res = $gc->monitor("lubm");
echo $res. PHP_EOL;

// show all databases
$res = $gc->show();
echo $res. PHP_EOL;

// unload the database
$res = $gc->unload("lubm");
echo $res. PHP_EOL;

// drop the database directly
$res = $gc->drop("lubm", false);
echo $res. PHP_EOL;

// drop the database and leave a backup
$res = $gc->drop("lubm", true);
echo $res. PHP_EOL;

// get CoreVersion and APIVersion
$res = $gc->getCoreVersion();
echo $res. PHP_EOL;
$res = $gc->getAPIVersion();
echo $res. PHP_EOL;

```

The original declaration of these functions are as below:

```
If request_type is "GET", the last parameter can be omitted:

class GstoreConnector($ip, $port, $username, $password)

function build($db_name, $rdf_file_path, $request_type)

function load($db_name, $request_type)

function unload($db_name, $request_type)

function user($type, $username2, $addition, $request_type)

function showUser($request_type)

function query($db_name, $format, $sparql, $request_type)

function fquery($db_name, $format, $sparql, $filename, $request_type)

function drop($db_name, $is_backup, $request_type)

function monitor($db_name, $request_type)

function checkpoint($db_name, $request_type)

function show($request_type)

function getCoreVersion($request_type)

function getAPIVersion($request_type)

```

---
---

# Socket API

**This APIis not maintained now.**

## Easy Examples

We provide JAVA, C++, PHP and Python API for gStore now. Please refer to example codes in `api/socket/cpp/example`, `api/socket/java/example`, `api/socket/php` and `api/socket/python`. To use C++ and Java examples to have a try, please ensure that executables have already been generated. Otherwise, just type `make APIexample` in the root directory of gStore to compile the codes, as well as API. 

Next, **start up a gStore server by using `./gserver` command.** It is ok if you know a running usable gStore server and try to connect to it, but notice that **the server ip and port of server and client must be matched.**(you do not need to change any thing if using examples, just by default) Then, you need to compile the example codes in the directory gStore/api/socket. We provide a utility to do this, and you just need to type `make APIexample` in the root directory of gStore. Or you can compile the codes by yourself, in this case please go to gStore/api/socket/cpp/example/ and gStore/socket/api/java/example/, respectively.

Finally, go to the example directory and run the corresponding executables. For C++, just use `./example` command to run it. And for Java, use `make run` command or `java -cp ../lib/GstoreJavaAPI.jar:. JavaAPIExample` to run it. Both the two executables will connect to a specified gStore server and do some load or query operations. Be sure that you see the query results in the terminal where you run the examples, otherwise please go to [Frequently Asked Questions](FAQ.md) for help or report it to us.(the report approach is described in [README](../README.md))

You are advised to read the example code carefully, as well as the corresponding Makefile. This will help you to understand the API, specially if you want to write your own programs based on the API interface.

- - -

## API structure

The socket API of gStore is placed in api/socket directory in the root directory of gStore, whose contents are listed below:

- gStore/api/socket/
	
	- cpp/ (the C++ API)
		
		- src/ (source code of C++ API, used to build the lib/libgstoreconnector.a)

			- GstoreConnector.cpp (interfaces to interact with gStore server)

			- GstoreConnector.h

			- Makefile (compile and build lib)
		
		- lib/ (where the static lib lies in)

			- .gitignore

			- libgstoreconnector.a (only exist after compiled, you need to link this lib when you use the C++ API)

		- example/ (small example program to show the basic idea of using the C++ API)

			- CppAPIExample.cpp

			- Makefile

	- java/ (the Java API)

		- src/ (source code of Java API, used to build the lib/GstoreJavaAPI.jar)
			
			- jgsc/ (the package which you need to import when you use the Java API)
				
				- GstoreConnector.java (interfaces to interact with gStore server)

			- Makefile (compile and build lib)

		- lib/

			- .gitignore

			- GstoreJavaAPI.jar (only exist after compiled, you need to include this JAR in your class path)

		- example/ (small example program to show the basic idea of using the Java API)

			- JavaAPIExample.cpp

			- Makefile

	- php/ (the PHP API)

		- PHPAPIExxample.php (small example program to show the basic idea of using the PHP API)

		- GstoreConnector.php (source code of PHP API)

	- python/ (the python API)

		- src/ (source code of Python API)

			- GstoreConnector.py

		- lib/

		- example/ (small example program to show the basic idea of using the Python API)

			- PythonAPIExample.py


- - -

## C++ API

#### Interface

To use the C++ API, please place the phrase `#include "GstoreConnector.h"` in your cpp code. Functions in GstoreConnector.h should be called like below:

```
// initialize the Gstore server's IP address and port.
GstoreConnector gc("127.0.0.1", 3305);

// build a new database by a RDF file.
// note that the relative path is related to gserver.
gc.build("LUBM10.db", "example/LUBM_10.n3");

// then you can execute SPARQL query on this database.
std::string sparql = "select ?x where \
					 { \
					  ?x    <rdf:type>    <ub:UndergraduateStudent>. \
					  ?y    <ub:name> <Course1>. \
					  ?x    <ub:takesCourse>  ?y. \
					  ?z    <ub:teacherOf>    ?y. \
					  ?z    <ub:name> <FullProfessor1>. \
					  ?z    <ub:worksFor>    ?w. \
					  ?w    <ub:name>    <Department0>. \
					  }";
std::string answer = gc.query(sparql);

// unload this database.
gc.unload("LUBM10.db");

// also, you can load some exist database directly and then query.
gc.load("LUBM10.db");

// query a SPARQL in current database
answer = gc.query(sparql);
```

The original declaration of these functions are as below:

```
GstoreConnector();
GstoreConnector(string _ip, unsigned short _port);
GstoreConnector(unsigned short _port);
bool load(string _db_name);
bool unload(string _db_name);
bool build(string _db_name, string _rdf_file_path);
string query(string _sparql);
```

Notice:

1. When using GstoreConnector(), the default value for ip and port is 127.0.0.1 and 3305, respectively.

2. When using build(), the rdf_file_path(the second parameter) should be related to the position where gserver lies in.

3. Please remember to unload the database you have loaded, otherwise things may go wrong.(the errors may not be reported!)

#### Compile

You are advised to see gStore/api/socket/cpp/example/Makefile for instructions on how to compile your code with the C++ API. Generally, what you must do is compile your own code to object with header in the C++ API, and link the object with static lib in the C++ API.

Let us assume that your source code is placed in test.cpp, whose position is ${TEST}, while the gStore project position is ${GSTORE}/gStore.(if using devGstore as name instead of gStore, then the path is ${GSTORE}/devGstore) Please go to the ${TEST} directory first:

> Use `g++ -c -I${GSTORE}/gStore/api/socket/cpp/src/ test.cpp -o test.o` to compile your test.cpp into test.o, relative API header is placed in api/socket/cpp/src/.

> Use `g++ -o test test.o -L${GSTORE}/gStore/api/socket/cpp/lib/ -lgstoreconnector` to link your test.o with the libgstoreconnector.a(a static lib) in api/socket/cpplib/.

Then you can type `./test` to execute your own program, which uses our C++ API. It is also advised for you to place relative compile commands in a Makefile, as well as other commands if you like.

- - -

## Java API

#### Interface

To use the Java API, please place the phrase `import jgsc.GstoreConnector;` in your java code. Functions in GstoreConnector.java should be called like below:

```
// initialize IP address and port of the Gstore server.
GstoreConnector gc = new GstoreConnector("127.0.0.1", 3305);

// build a new database by a RDF file.
// note that the relative path is related to gserver.
gc.build("LUBM10.db", "example/LUBM_10.n3");

// then you can execute SPARQL query on this database.
String sparql = "select ?x where "
				+ "{"
				+ "?x    <rdf:type>    <ub:UndergraduateStudent>. "
				+ "?y    <ub:name> <Course1>. "
				+ "?x    <ub:takesCourse>  ?y. "
				+ "?z    <ub:teacherOf>    ?y. "
				+ "?z    <ub:name> <FullProfessor1>. "
				+ "?z    <ub:worksFor>    ?w. "
				+ "?w    <ub:name>    <Department0>. "
				+ "}";
String answer = gc.query(sparql);

// unload this database.
gc.unload("LUBM10.db");

// also, you can load some exist database directly and then query.
gc.load("LUBM10.db");

// query a SPARQL in current database
answer = gc.query(sparql);    
```

The original declaration of these functions are as below:

```
GstoreConnector();
GstoreConnector(string _ip, unsigned short _port);
GstoreConnector(unsigned short _port);
bool load(string _db_name);
bool unload(string _db_name);
bool build(string _db_name, string _rdf_file_path);
string query(string _sparql);
```

Notice:

1. When using GstoreConnector(), the default value for ip and port is 127.0.0.1 and 3305, respectively.

2. When using build(), the rdf_file_path(the second parameter) should be related to the position where gserver lies in.

3. Please remember to unload the database you have loaded, otherwise things may go wrong.(the errors may not be reported!)

#### Compile

You are advised to see gStore/api/socket/java/example/Makefile for instructions on how to compile your code with the Java API. Generally, what you must do is compile your own code to object with jar file in the Java API.

Let us assume that your source code is placed in test.java, whose position is ${TEST}, while the gStore project position is ${GSTORE}/gStore.(if using devGstore as name instead of gStore, then the path is ${GSTORE}/devGstore) Please go to the ${TEST} directory first:

> Use `javac -cp ${GSTORE}/gStore/api/socket/java/lib/GstoreJavaAPI.jar test.java` to compile your test.java into test.class with the GstoreJavaAPI.jar(a jar package used in Java) in api/java/lib/.

Then you can type `java -cp ${GSTORE}/gStore/api/socket/java/lib/GstoreJavaAPI.jar:. test` to execute your own program(notice that the ":." in command cannot be neglected), which uses our Java API. It is also advised for you to place relative compile commands in a Makefile, as well as other commands if you like.


- - -

## PHP API

#### Interface

To use the PHP API, please place the phrase `include('GstoreConnector,php');` in your php code. Functions in
GstoreConnector.php should be called like below:

```
// initialize IP address and port of the Gstore server.
$gc = new Connector("127.0.0.1", 3305);
// build a new database by a RDF file.
// note that the relative path is related to gserver.
$gc->build("LUBM10", "example/LUBM_10.n3");
// then you can execute SPARQL query on this database.
$sparql = "select ?x where " + "{" +
	"?x    <rdf:type>    <ub:UndergraduateStudent>. " +
	"?y    <ub:name> <Course1>. " +
	"?x    <ub:takesCourse>  ?y. " +
	"?z    <ub:teacherOf>    ?y. " +
	"?z    <ub:name> <FullProfessor1>. " +
	"?z    <ub:worksFor>    ?w. " +
	"?w    <ub:name>    <Department0>. " +
	"}";
$answer = gc->query($sparql);
//unload this database.
$gc->unload("LUBM10");
//also, you can load some exist database directly and then query.
$gc->load("LUBM10");// query a SPARQL in current database
$answer = gc->query(sparql);
```

The original declaration of these functions are as below:

```
class Connector {
	public function __construct($host, $port);
	public function send($data);
	public function recv();
	public function build($db_name, $rdf_file_path);
	public function load($db_name);
	public function unload($db_name);
	public function query($sparql);
	public function __destruct();
}

```

Notice:

1. When using Connector(), the default value for ip and port is 127.0.0.1 and 3305, respectively.

2. When using build(), the rdf_file_path(the second parameter) should be related to the position where gserver lies in.

3. Please remember to unload the database you have loaded, otherwise things may go wrong.(the errors may not be reported!)

#### Run

You can see gStore/api/socket/php/PHPAPIExample for instructions on how to use PHP API. PHP script does not need compiling. You can run PHP file directly or use it in your web project.

- - -

## Python API

#### Interface

To use the Python API, please place the phrase `from GstoreConnector import GstoreConnector` in your python code. Functions in GstoreConnector.py should be called like below:

```
// initialize IP address and port of the Gstore server.
gc = GstoreConnector('127.0.0.1', 3305)
// build a new database by a RDF file.
// note that the relative path is related to gserver.
gc.build('LUBM10', 'data/LUBM_10.n3')
// then you can execute SPARQL query on this database.
$sparql = "select ?x where " + "{" +
	"?x    <rdf:type>    <ub:UndergraduateStudent>. " +
	"?y    <ub:name> <Course1>. " +
	"?x    <ub:takesCourse>  ?y. " +
	"?z    <ub:teacherOf>    ?y. " +
	"?z    <ub:name> <FullProfessor1>. " +

	"?z    <ub:worksFor>    ?w. " +
	"?w    <ub:name>    <Department0>. " +
	"}";
answer = gc.query(sparql)
//unload this database.
gc.unload('LUBM10')
//also, you can load some exist database directly and then query.
gc.load('LUBM10')// query a SPARQL in current database
answer = gc.query(sparql)
```

The original declaration of these functions are as below:

```
class GstoreConnector {
	def _connect(self)
	def _disconnect(self)
	def _send(self, msg):
	def _recv(self)
	def _pack(self, msg):
	def _communicate(f):
	def __init__(self, ip='127.0.0.1', port=3305):
	@_communicate
	def test(self)
	@_communicate
	def load(self, db_name)
	@_communicate
	def unload(self, db_name)
	@_communicate
	def build(self, db_name, rdf_file_path)
	@_communicate
	def drop(self, db_name)
	@_communicate
	def stop(self)
	@_communicate
	def query(self, sparql)
	@_communicate
	def show(self, _type=False)
}

```

Notice:

1. When using GstoreConnector(), the default value for ip and port is 127.0.0.1 and 3305, respectively.

2. When using build(), the rdf_file_path(the second parameter) should be related to the position where gserver lies in.

3. Please remember to unload the database you have loaded, otherwise things may go wrong.(the errors may not be reported!)

#### Run

You are advised to see gStore/api/socket/python/example/PythonAPIExample for examples on how to use python API. Python file does not need compiling, and you can run it directly.

