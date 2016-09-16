**This Chapter guides you to use our API for accessing gStore.**

## Easy Examples

We provide JAVA and C++ API for gStore now. Please refer to example codes in `api/cpp/example` and `api/java/example`. To use the two examples to have a try, please ensure that executables have already been generated. Otherwise, just type `make APIexample` in the root directory of gStore to compile the codes, as well as API. 

Next, **start up a gStore server by using `./gserver` command.** It is ok if you know a running usable gStore server and try to connect to it, but notice that **the server ip and port of server and client must be matched.**(you don't need to change any thing if using examples, just by default) Then, you need to compile the example codes in the directory gStore/api/. We provide a utility to do this, and you just need to type `make APIexample` in the root directory of gStore. Or you can compile the codes by yourself, in this case please go to gStore/api/cpp/example/ and gStore/api/java/example/, respectively.

Finally, go to the example directory and run the corresponding executables. For C++, just use `./example` command to run it. And for Java, use `make run` command or `java -cp ../lib/GstoreJavaAPI.jar:. JavaAPIExample` to run it. Both the two executables will connect to a specified gStore server and do some load or query operations. Be sure that you see the query results in the terminal where you run the examples, otherwise please go to [Frequently Asked Questions](FAQ.md) for help or report it to us.(the report approach is described in [README](../README.md))

You are advised to read the example code carefully, as well as the corresponding Makefile. This will help you to understand the API, specially if you want to write your own programs based on the API interface.

- - -

## API structure

The API of gStore is placed in api/ directory in the root directory of gStore, whose contents are listed below:

- gStore/api/
	
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

You are advised to see gStore/api/cpp/example/Makefile for instructions on how to compile your code with the C++ API. Generally, what you must do is compile your own code to object with header in the C++ API, and link the object with static lib in the C++ API.

Let us assume that your source code is placed in test.cpp, whose position is ${TEST}, while the gStore project position is ${GSTORE}/gStore.(if using devGstore as name instead of gStore, then the path is ${GSTORE}/devGstore) Please go to the ${TEST} directory first:

> Use `g++ -c -I${GSTORE}/gStore/api/cpp/src/ test.cpp -o test.o` to compile your test.cpp into test.o, relative API header is placed in api/cpp/src/.

> Use `g++ -o test test.o -L${GSTORE}/gStore/api/cpp/lib/ -lgstoreconnector` to link your test.o with the libgstoreconnector.a(a static lib) in api/cpp/lib/.

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

You are advised to see gStore/api/java/example/Makefile for instructions on how to compile your code with the Java API. Generally, what you must do is compile your own code to object with jar file in the Java API.

Let us assume that your source code is placed in test.java, whose position is ${TEST}, while the gStore project position is ${GSTORE}/gStore.(if using devGstore as name instead of gStore, then the path is ${GSTORE}/devGstore) Please go to the ${TEST} directory first:

> Use `javac -cp ${GSTORE}/gStore/api/java/lib/GstoreJavaAPI.jar test.java` to compile your test.java into test.class with the GstoreJavaAPI.jar(a jar package used in Java) in api/java/lib/.

Then you can type `java -cp ${GSTORE}/gStore/api/java/lib/GstoreJavaAPI.jar:. test` to execute your own program(notice that the ":." in command cannot be neglected), which uses our Java API. It is also advised for you to place relative compile commands in a Makefile, as well as other commands if you like.

