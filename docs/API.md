**This Chapter guides you to use our API for accessing gStore. We provide socket API and HTTP api, corresponding to gserver and ghttp respectively.**

# Socket API

## Easy Examples

We provide JAVA, C++, PHP and Python API for gStore now. Please refer to example codes in `api/socket/cpp/example`, `api/socket/java/example`, `api/socket/php` and `api/socket/python`. To use C++ and Java examples to have a try, please ensure that executables have already been generated. Otherwise, just type `make APIexample` in the root directory of gStore to compile the codes, as well as API. 

Next, **start up a gStore server by using `./gserver` command.** It is ok if you know a running usable gStore server and try to connect to it, but notice that **the server ip and port of server and client must be matched.**(you don't need to change any thing if using examples, just by default) Then, you need to compile the example codes in the directory gStore/api/socket. We provide a utility to do this, and you just need to type `make APIexample` in the root directory of gStore. Or you can compile the codes by yourself, in this case please go to gStore/api/socket/cpp/example/ and gStore/socket/api/java/example/, respectively.

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

	-python/ (the python API)

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
// initialize the Gstore server's IP address and port.
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
// initialize the Gstore server's IP address and port.
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

You can see gStore/api/socket/php/PHPAPIExample for instructions on how to use PHP API. PHP script doesn't need compiling. You can run PHP file directly or use it in your web project.

- - -

## Python API

#### Interface

To use the Python API, please place the phrase `from GstoreConnector import GstoreConnector` in your python code. Functions in GstoreConnector.py should be called like below:

```
// initialize the Gstore server's IP address and port.
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

You are advised to see gStore/api/socket/python/example/PythonAPIExample for examples on how to use python API. Python file doesn't need compiling, and you can run it directly.


# HTTP API

Compired with socket API, HTTP API is more stable and more standard, and can maintain connection. Socket API can not guaratee correct transmission, so the network transmission is faster.

## Easy Examples

We provide JAVA and C++ API for ghttp now. Please refer to example codes in `api/http/cpp` and `api/http/java`. To use these examples, please make sure that executables have already been generated.

Next, **start up ghttp service by using \texttt{./ghttp} command.** It is ok if you know a running usable ghttp server and try to connect to it. (you don't need to change anything if using examples, just by default). Then, for Java and C++ code, you need to compile the example codes in the directory gStore/api/http/. 

Finally, go to the example directory and run the corresponding executables. All these four executables will connect to a specified ghttp server and do some load or query operations. Be sure that you see the query results in the terminal where you run the examples, otherwise please go to [Frequently Asked Questions](FAQ.md) for help or report it to us.(the report approach is described in [README](../README.md))

You are advised to read the example code carefully, as well as the corresponding Makefile. This will help you to understand the API, specially if you want to write your own programs based on the API interface.

- - -

## API structure

The HTTP API of gStore is placed in api/http directory in the root directory of gStore, whose contents are listed below:

- gStore/api/http/
	
	- cpp/ (the C++ API)
		
		- client.cpp (source code of C++ API)

		- client.h

		- example.cpp (small example program to show the basic idea of using the C++ API)

		- Makefile (compile and build lib)
		
	- java/ (the Java API)

		- HttpRequest.java (source code of Java API, with examples in main function)

		- HttpRequest.class

- - -

## C++ API

#### Interface

To use the C++ API, please place the phrase `#include "Client.h"` in your cpp code. Functions in Client.h should be called like below:

```
CHttpClient hc;
string res; 
int ret;
// build a new database by a RDF file.
ret = hc.Get("127.0.0.1:9000/build/lumb/data/LUBM_10.n3", res);
cout&lt;&lt;res&lt;&lt;endl;
// load databse
ret = hc.Get("127.0.0.1:9000/load/lumb", res);
cout&lt;&lt;res&lt;&lt;endl;
// then you can execute SPARQL query on this database.
ret = hc.Get("127.0.0.1:9000/query/data/ex0.sql", res);
cout&lt;&lt;res&lt;&lt;endl;
// output information of current database
ret = hc.Get("127.0.0.1:9000/monitor", res);
cout&lt;&lt;res&lt;&lt;endl;
// unload this databse
ret = hc.Get("127.0.0.1:9000/unload", res);
cout&lt;&lt;res&lt;&lt;endl;
```

The original declaration of these functions are as below:

```
CHttpClient();
int Post(const std::string &amp; strUrl, const std::string &amp; strPost, std::string &amp; strResponse);
int Get(const std::string &amp; strUrl, std::string &amp; strResponse);
int Posts(const std::string &amp; strUrl, const std::string &amp; strPost, std::string &amp; strResponse, const char * pCaPath = NULL);
int Gets(const std::string &amp; strUrl, std::string &amp; strResponse, const char * pCaPath = NULL);
```

- - -

## Java API

#### Interface

To use the Java API, please see gStore/api/http/java/HttpRequest.java. Functions should be called like below:

```
// build a new database by a RDF file.
String s=HttpRequest.sendGet("http://localhost:9000/build/lubm/data/LUBM_10/n3", "");
System.out.println(s);
// load databse
String s=HttpRequest.sendGet("http://localhost:9000/load/lubm", "");
System.out.println(s);
// then you can execute SPARQL query on this database.
String s=HttpRequest.sendGet("http://localhost:9000/query/data/ex0.sql", "");
System.out.println(s);
// output information of current databse
String s=HttpRequest.sendGet("http://localhost:9000/monitor", "");
System.out.println(s);
// unload this database
String s=HttpRequest.sendGet("http://localhost:9000/unload", "");
System.out.println(s);
```

The original declaration of these functions are as below:

```
public class HttpRequest();
public static String sendGet(String url, String param);
public static String sendPost(String url, String param);
```
