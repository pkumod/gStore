## Data Format

​	`gStore` is a graph database engine based on the RDF model, and its data format follows the RDF model. RDF, the W3C standard for describing real-world resources, is a general way to describe information so that it can be read and understood by computer applications. Any entity in the real world can be represented as a resource in the RDF model, such as a book's title, author, modification date, content, and copyright information. These resources can be used to abstract concepts, entities, and events from the objective world in the knowledge graph. An attribute of each resource and its attribute value, or its relationship to other resources, is called a piece of knowledge. Properties and relationships can be represented as triples.  

​		A triad consists of three elements: Subject, Property 1, and Object. It usually describes the relationship between two resources or some Property of a resource. When a triple describes the attributes of a resource, its three elements are also called body, attribute, and Property Value. For example, the triad "Aristotle, Birthplace, Chalcis" expresses the fact that Aristotle was born in Chalcis.

​       Using these properties and relationships, a large number of resources can be linked together to form a large RDF knowledge graph dataset. Therefore, a knowledge graph can often be viewed as a collection of triples. These collections of triples, in turn, form an RDF dataset. The triplet set of knowledge graph can be stored in relational database or graph database.gStore 1.2 adds support for various RDF serialization formats such as **Turtle**, **TriG**, **RDF/XML**, **RDFa**, and **JSON-LD**. Here are some of the mainstream data formats.

### N-Triple

​      RDF data should be provided in n-triple format (XML is not currently supported), and queries must be provided in SPARQL1.1 syntax. The following is an example of the n-triple format file:        

```
@prefix foaf:  <http://xmlns.com/foaf/0.1/> .	
_:a  foaf:name   "Johnny Lee Outlaw" .
_:a  foaf:mbox   <mailto:jlow@example.com> .
_:b  foaf:name   "Peter Goodguy" .
_:b  foaf:mbox   <mailto:peter@example.org> .
_:c  foaf:mbox   <mailto:carol@example.org> .
```

Triples are typically stored in the W3C-defined NT file format and represent three RDF data, where the values wrapped in `<` and `>`are urIs of an entity, and the values wrapped in '"" are literals representing the value of an attribute of the entity, followed by'`^^ `to indicate the type of the value. The following three RDF data points represent two attributes of `John`, `gender` and `age`, with values of `male` and `28` respectively. The last one indicates that `John` and `Li` have a `friend` relationship.

```NT
<John> <gender> "male"^^<http://www.w3.org/2001/XMLSchema#String>.
<John> <age> "28"^^<http://www.w3.org/2001/XMLSchema#Int>.
<John> <friend> <Li>.
```

​    More specific information about N-Triple please check [N-Triple](https://www.w3.org/TR/n-triples/). Not all syntax in SPARQL1.1 is parsed and answered in gStore; for example, property paths are beyond the capabilities of the gStore system.

### Turtle

Turtle is a more concise serialization format for RDF. It enhances readability and conciseness on the basis of N-triple, and adds abbreviation functions such as prefixes. The following abbreviation form can represent four RDF three-month meetings, where multiple predicates of the same subject can be separated by `,` and multiple predicates of the same subject and predicate can be separated by `,`. The example file of Turtle is as follows:

```
@prefix swp: <http://www.semanticwebprimer.org/ontology/apartments.ttl#>.
@prefix dbpedia: <http://dbpedia.org/resource>.
@prefix dbpedia-owl: <http://dbpedia.org/ontology>.
@prefix xsd: <http://www.w3.org/2001/XMLSchema#>.

swp:BaronWayAppartment swp:hasNumberOfBedrooms "3"^^xsd:integer;
                        swp:isPartOf swp:BaronWayBuilding.

swp:BaronWayBuilding dbpedia-owl:location dbpedia:Amsterdam,
                                          dbpedia:Netherlands.
```

​	Turtle is usually stored in the ttl file format defined by the W3C. For a more detailed description of the Turtle file, please refer to [Turtle](https://www.w3.org/TR/turtle/)。

### TriG

TriG is an extension of the Turtle language that allows further naming of graphs, as shown in the following example：

```
# This document contains a same data as the
previous example.

@prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> .
@prefix dc: <http://purl.org/dc/terms/> .
@prefix foaf: <http://xmlns.com/foaf/0.1/> .

# default graph - no {} used.
<http://example.org/bob> dc:publisher "Bob" .
<http://example.org/alice> dc:publisher "Alice" .

# GRAPH keyword to highlight a named graph
# Abbreviation of triples using ;
GRAPH <http://example.org/bob>
{
   [] foaf:name "Bob" ;
      foaf:mbox <mailto:bob@oldcorp.example.org> ;
      foaf:knows _:b .
}

GRAPH <http://example.org/alice>
{
    _:b foaf:name "Alice" ;
        foaf:mbox <mailto:alice@work.example.org>
}
```

### **RDF/XML**

RDF/XML is a syntax defined by the W3C for expressing (i.e., serializing) RDF graphs as XML documents. RDF/XML is sometimes misleadingly referred to as RDF because it was introduced in other W3C specifications that define RDF, and historically, it was the first W3C standard RDF serialization format. For a more detailed description of RDF/XML files, please refer to [**RDF/XML**](https://www.w3.org/TR/rdf-syntax-grammar/).The following is an example:：

```
<?xml version="1.0" encoding="utf-8"?>
<rdf:RDF xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"
            xmlns:dc="http://purl.org/dc/elements/1.1/">

  <rdf:Description rdf:about="http://www.w3.org/TR/rdf-syntax-grammar">
    <dc:title>RDF 1.1 XML Syntax</dc:title>
    <dc:title xml:lang="en">RDF 1.1 XML Syntax</dc:title>
    <dc:title xml:lang="en-US">RDF 1.1 XML Syntax</dc:title>
  </rdf:Description>

  <rdf:Description rdf:about="http://example.org/buecher/baum" xml:lang="de">
    <dc:title>Der Baum</dc:title>
    <dc:description>Das Buch ist außergewöhnlich</dc:description>
    <dc:title xml:lang="en">The Tree</dc:title>
  </rdf:Description>

</rdf:RDF>
```

### JSON-LD

JSON-LD is a lightweight linked data format. It is easy for humans to read and write. It is based on the successful JSON format and provides a method to help interoperate with JSON data. JSON-LD is an ideal data format for programming environments, REST Web services, and unstructured databases such as Apache CouchDB and MongoDB. Linked data enables people to publish and use information on the web. It is a method of creating standards-based, readable data on websites. It allows applications to start with a piece of linked data and link to other linked data hosted on different sites on the web using embedded links. For a more detailed description of JSON-LD files, please refer to [JSON-LD](https://www.w3.org/TR/json-ld/). Examples are as follows：

```
{
  "@context": {
    "name": "http://xmlns.com/foaf/0.1/name",
    "knows": "http://xmlns.com/foaf/0.1/knows"
  },
  "@id": "http://me.markus-lanthaler.com/",
  "name": "Markus Lanthaler",
  "knows": [
    {
      "name": "Dave Longley"
    }
  ]
}
```

## Initialize the system database

As soon as you download and compile the code for the gStore system, a database named System (the real directory name system.db) is automatically created. This is the database that manages system statistics, including all users and all databases. You can query this database using the gquery command, but do not modify it using the editor. 

The `system` database is the built-in system database of gStore. This database cannot be deleted. It is used to store the information related to the system, especially the information of the built database  

### Command line mode(ginit)

ginit is used to initialize the database

instruction：

```shell
bin/ginit -db [db_name1],[db_name2],[...]
```

command parameter：

```
db_name1：database name
```

If no database names are written, the reinitialized `system` database will have no other database information

example：

```shell
[root@localhost gStore]$ bin/ginit -db lubm
The database lubm is not exist, now create it.
...
system.db init successfully! Used 24 ms
```

## Create database

Creating a database is one of the most important operations in gStore. It is also the first operation that users need to perform after gStore installation. gStore provides multiple methods to create a database.  

### Command line mode (gbuild)

The gbuild command is used to create a new database from an RDF file:

```shell
bin/gbuild -db dbname -f filename  
```

Parameter definition：

	dbname：database name
	filename：filepath of files with ".nt" or ".n3" as suffix ,filepath of files with ".zip "as suffix containing files with ".nt" or ".n3" as suffix

For example, we build a database called "lubm.db" from lubm.nt, which can be found in the data folder.

```shell
[root@localhost gStore]$ bin/gbuild -db lubm -f ./data/lubm/lubm.nt 
database path: ./dbhome/lubm.db
...
Save the database info to system database....
...
Add database info success, update num : 3
Build RDF database lubm successfully! Used 2869 ms
```

Note：

- You cannot create a database from an empty RDF dataset
- Note that you cannot directly `CD` into the `bin` directory. Instead, you need to perform the gbuild operation in the gStore installation root director



### Visual tool (gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



### HTTP API (ghttp)

gStore provides GHTTP component as HTTP API service component. Users can realize relevant functions by sending HTTP requests to ghttp In ghttp, graph database is constructed by `build` request. 



### Socket API (gServer)

gStore provides gServer component as Socket API service component. Users can realize related functions by sending Socket request to gServer. gServer builds graph database through `build` request. See [Development document] - [Common API] - [gServer interface Description] for details.

## Database list

The database list function is used to obtain information about all available databases in the following formats

### Command line mode (gshow)

gshow is used to get the list of all available databases.

instruction：

```shell
bin/gshow
```

example：

```shell
[root@localhost gStore]$ bin/gshow 
--------------------------------------------
| database | creator |     built_time      |
--------------------------------------------
| system   | root    |                     |
| small    | root    | 2024-08-08 10:24:01 |
| lubm     | root    | 2024-08-08 14:00:22 |
| num      | root    | 2024-08-08 10:31:42 |
| bbug     | root    | 2024-08-08 10:24:01 |
--------------------------------------------
```



### Visual tool (gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



### HTTP API (ghttp)

gStore provides ghttp component as http API service component, users can send http request to ghttp to achieve relevant functions, ghttp through the `show` command to achieve relevant functions, details see [development document] - [common API] - [ghttp interface description].

### Socket API (gServer)

gStore provides gServer component as Socket API service component. Users can realize related functions by sending Socket request to gServer. gServer displays database list through `show` request. See [Development document] - [Common API] - [gServer interface Description] for details.

## Database status Query

The database status query function is used to obtain statistics about a specified database in the following ways.

### Command line mode (gmonitor)

gmonitor is used to get statistics about a specified database.

instruction：

```shell
bin/gmonitor -db db_name
```

parameter definition：

	db_name：database name 

example：

```shell
[root@localhost gStore]$ bin/gmonitor -db test1
---------------------------------------
|     name      |        value        |
---------------------------------------
| database      | lubm                |
| creator       | root                |
| built_time    | 2024-08-08 14:00:22 |
| triple_num    | 99550               |
| entity_num    | 28413               |
| literal_num   | 0                   |
| subject_num   | 16196               |
| predicate_num | 17                  |
| disk_used     | 21 MB               |
---------------------------------------
```



### Visual tool（gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



### HTTP API（ghttp）

gStore provides ghttp component as http API service component. Users can realize relevant functions by sending http requests to ghttp. In ghttp, database statistics can be obtained through `monitor`.

## Database query

Database query is one of the most important functions of gStore. gStore supports SPARQL 1.1 query language defined by W3C. Users can use the gStore database query function in the following ways.

### Command line mdoe（gquery)

gquery is used to query existing databases using files containing SPARQL queries. (Each file contains an exact SPARQL statement. SPARQL statements can be used not only for queries, but also for additions and deletions. For details on SPARQL statements, see Chapter 8.)  

1.To query the database named db name, enter the following command:

```shell
bin/gquery -db db_name -q query_file 
```

parameter definition： 

	db_name: database name
	query_file：The path to the SPARQL statement ending with ".sql "(other name extensions are acceptable)

For example, we execute the SPARQL statement in./data/lubm/ lubM_q0.sql to query the LUBM database.

The query result is：

```shell
[root@localhost gStore]$ bin/gquery -db lubm -q ./data/lubm/lubm_q0.sql
------------------------------------------------------------
|                            ?x                            |
------------------------------------------------------------
| <http://www.Department0.University0.edu/FullProfessor0>  |
| <http://www.Department1.University0.edu/FullProfessor0>  |
| <http://www.Department2.University0.edu/FullProfessor0>  |
| <http://www.Department3.University0.edu/FullProfessor0>  |
| <http://www.Department4.University0.edu/FullProfessor0>  |
| <http://www.Department5.University0.edu/FullProfessor0>  |
| <http://www.Department6.University0.edu/FullProfessor0>  |
| <http://www.Department7.University0.edu/FullProfessor0>  |
| <http://www.Department8.University0.edu/FullProfessor0>  |
| <http://www.Department9.University0.edu/FullProfessor0>  |
| <http://www.Department10.University0.edu/FullProfessor0> |
| <http://www.Department11.University0.edu/FullProfessor0> |
| <http://www.Department12.University0.edu/FullProfessor0> |
| <http://www.Department13.University0.edu/FullProfessor0> |
| <http://www.Department14.University0.edu/FullProfessor0> |
------------------------------------------------------------
query database successfully, Used 72 ms
```

2.To learn more about how to use gQuery, enter the following command:

```shell
bin/gquery --help
```

3.Command to enter gquery console:

```shell
bin/gquery -db dbname
```

he program displays a command prompt (" gsql > ") where you can enter command:

>Use `help` to see the basic information for all commands

>Enter `quit` to exit the gquery console

>For the `sparql` command, use `sparQL query_file` to execute the SPARQL query statement, and query_file indicates the path of the file storing the SPARQL statement. When the program finishes answering the query, it displays the command prompt again
>
>We use lubm.nt as example。

```shell
(base) [root@iz8vb0u9hafhzz1mn5xcklz gStore]# bin/gquery -db lubm

gsql>sparql ./data/lubm/lubm_q0.sql
query is:
select ?x where
{
        ?x      <ub:name>       <FullProfessor0>.
}


------------------------------------------------------------
|                            ?x                            |
------------------------------------------------------------
| <http://www.Department0.University0.edu/FullProfessor0>  |
| <http://www.Department1.University0.edu/FullProfessor0>  |
| <http://www.Department2.University0.edu/FullProfessor0>  |
| <http://www.Department3.University0.edu/FullProfessor0>  |
| <http://www.Department4.University0.edu/FullProfessor0>  |
| <http://www.Department5.University0.edu/FullProfessor0>  |
| <http://www.Department6.University0.edu/FullProfessor0>  |
| <http://www.Department7.University0.edu/FullProfessor0>  |
| <http://www.Department8.University0.edu/FullProfessor0>  |
| <http://www.Department9.University0.edu/FullProfessor0>  |
| <http://www.Department10.University0.edu/FullProfessor0> |
| <http://www.Department11.University0.edu/FullProfessor0> |
| <http://www.Department12.University0.edu/FullProfessor0> |
| <http://www.Department13.University0.edu/FullProfessor0> |
| <http://www.Department14.University0.edu/FullProfessor0> |
------------------------------------------------------------

gsql>help
help - print commands message
quit - quit the console normally
sparql - load query from the second argument

gsql>quit
```

- Note:   

  \- If there is no answer, "[Empty Result]" is printed with a blank line following all results.  

  \- Use readline lib, so you can use the arrow keys in the keyboard to view the command history, and use the and arrow keys to move and modify the entire command.  

  \- Utility supports path completion. (Not built-in command completion)  

  \- Note that you cannot directly 'CD' into the 'bin' directory, but rather perform the 'gquery' operation in the gStore installation root directory 

### Visual tool (gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].

### HTTP API（ghttp）

gStore provides ghttp component as http API service component, users can realize related functions by sending http request to ghttp, ghttp through `query` request to query graph database, including query, delete, insert, See [Development document] - [common API] - [ghttp interface Description] for details.

### Socket API（gServer）

gStore provides gServer component as Socket API service component. Users can realize related functions by sending Socket request to gServer. gServer can query graph database through `query` request, including query, delete and insert. See [Development document] - [Common API] - [gServer interface Description] for details.

## Database export

The export database function enables you to export a database as an. Nt file. There are three forms:

### Command line mode (gexport)

gexport is used to export database.

instruction：

```shell
bin/gexport -db db_name -f path 
```

parameter definition：

```
db_name：database name
path：Export to specified folder (if empty, export to gStore root by default)
```

example：

```shell
[root@localhost gStore]# bin/gexport -db lubm	
gexport...
...
start exporting the database......
...
finish loading
lubm.db exported successfully! Used 187 ms
lubm.db export path: lubm_240808141125.nt
```




### Visual tool (gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



### HTTP API（ghttp）

gStore provides ghttp component as httpAPI service component. Users can realize relevant functions by sending http requests to ghttp. The function of `export` is adopted in ghttp.

## Database deletion

Deleting a database You can delete a database in the following three ways

### Command line mode (gdrop)

gdrop is use to delete database

instruction：

```shell
bin/gdrop -db db_name
```

parameter definition：

```
db_name：database name
```

example：

```shell
[root@localhost gStore]$ bin/drop -db lubm2
Begin to drop database....
...
Database lubm2 dropped successfully! Used 20 ms
```

To delete the database, you should not just type `rm -r db_name. Db` because this will not update the built-in database named `system`. Instead, you should type `bin/ gdrop-db db_name`.



### Visual tool（gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



### HTTP API（ghttp）

gStore provides ghttp component as http API service component, users can send http request to ghttp to achieve relevant functions, ghttp through the `drop` command to achieve relevant functions, details see [development document] - [common API] - [ghttp interface description]  



### Socket API（gServer）

gStore provides gServer component as Socket API service component. Users can realize related functions by sending Socket request to gServer. gServer can delete graph database by `drop` request. See [Development document] - [Common API] - [gServer interface Description] for details.

## Additional data

Inserting RDF data is a routine gStore operation, and you can do it in one of the following ways.

### Command line mode（gadd)--file

gadd is use to insert triples from a file into an existing database.

instruction：

```shell
bin/gadd -db db_name -f rdf_triple_file_path
```

parameter definition：

	dbname：database name
	filename：filepath of files with ".nt" or ".n3" as suffix ,filepath of files with ".zip "as suffix containing files with ".nt" or ".n3" as suffix

example：

```shell
[bookug@localhost gStore]$ bin/gadd -db lubm -f ./data/lubm/lubm.nt
...
finish loading
after inserted triples num 0,failed num 0,used 2452 ms
```

**Note:**   

**1. Gadd is primarily used for data insertion into RDF files**  

**2. Do not go directly to the 'bin' directory. Instead, perform the 'gadd' operation in the gStore installation root directory**

### Command line mode（gquery)---SPARQL statement

SPARQL can be defined by `insert data` instruction to insert data. Based on this principle, users can also write SPARQL insert statements and then use gStore's `gQuery` tool to insert data. Examples of SPARQL insert statements are as follows:

```SPARQL
insert data {
    <Johnny> <gender> "male"^^<http://www.w3.org/2001/XMLSchema#String>.
    <Johnny> <age> "28"^^<http://www.w3.org/2001/XMLSchema#Int>.
    <Johnny> <friend> <Peter>.
}
```

Multiple RDF data can be contained with `{}`, taking care that each one ends with a `. `  

Since the database query function can be used to insert data, the following functions can also be used to insert data.



### Visual tool （gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



### HTTP API（ghttp）

gStore provides ghttp component as AN http API service component. Users can realize relevant functions by sending http requests to ghttp. ghttp inserts data through `query` request and batch inserts data through `batchInsert`. See [Development document] - [common API] - [ghttp interface Description] for details.



### Socket API（gServer）

gStore provides gServer component as Socket API service component. Users can implement related functions by sending Socket requests to gServer. gServer inserts data through 'query' requests. See [Development document] - [Common API] - [gServer interface Description] for details.

## Data deletion

Deleting RDF data is a routine gStore operation, and you can perform this operation in the following ways.

### Command line mode（gsub）--file deletion

gsub is used to remove triples from files in an existing database.

instruction：

```bash
bin/gsub -db db_name -f rdf_triple_file_path
```

parameter definition：

	rdf_triple_file_path：filepath of files with ".nt" or ".n3" as suffix ,filepath of files with ".zip "as suffix containing files with ".nt" or ".n3" as suffix

example：

```shell
[root@localhost gStore]$ bin/gsub -db lubm -f ./data/lubm/lubm.nt
...
finish loading
after remove, used 2580 ms
```



### Command line mode（gquery)---SPARQL statement

SPARQL can be defined by 'delete data' instruction to insert data. Based on this principle, users can also write SPARQL insert statements and then use gStore's `gQuery` tool to insert data. Examples of SPARQL insert statements are as follows:

```
delete data {
 <San Zhang> <gender> "male"^^<http://www.w3.org/2001/XMLSchema#String>.
<San Zhang> <age> "28"^^<http://www.w3.org/2001/XMLSchema#Int>.
<San Zhang> <friend> <Si Li>.
}
```

Multiple RDF data can be contained with '{}', taking care that each one ends with a `. `

SPARQL also allows you to delete data based on a subquery structure using the 'DELETE WHERE' statement, as shown below

```
delete where
{
   <San Zhang> ?x ?y.
}
```

This statement deletes all information (including attributes and relationships) about an entity

Since the database query function can be used to insert data, the following functions can also be used to insert data



### Visual tool（gWorkbench)

gWorkbench is a visual management tool for gStore. Through gWorkbench, you can connect to gStore and delete data by writing SPARQL statements through database query module. See [Development document] - [Visualization Tool Workbench] - [query function] - [Graph database query] for details.  



### HTTP API（ghttp）

gStore provides ghttp component as http API service component, users can send http request toghttp to achieve relevant functions, ghttp through `query` request to delete data and `batchRemove` to delete data in batches. See [Development document] - [common API] - [ghttp interface Description] for details



### Socket API（gServer）

gStore provides gServer component as Socket API service component. Users can realize related functions by sending Socket requests to gServer. gServer can delete data through `query` requests. See [Development document] - [Common API] - [gServer interface Description] for details.

## Console Service

gStore provides a console service with contextual information via the gconsole module, which users can use to perform all types of database management operations.

### Start gconsole

**(1) Login**

```bash
bin/gconsole [-u <usr_name>]
```

A prompt for input will appear if `usr_name` is missing.

```bash
# bin/gconsole 
Enter user name: root
Enter password: 

Gstore Console , an interactive shell based utility to communicate with gstore repositories.
Gstore version: 1.3 Source distribution
Copyright (c) 2016, 2024, pkumod and topgraph and/or its affiliates.

Welcome to the Gstore Console.
Commands end with ;. Cross line input is allowed.
Comment start with #. Redirect (> and >>) is supported.
CTRL+C to quit current command. CTRL+D to exit this console.
Type 'help;' for help. 

gstore> 
```

**(2) Help**

```bash
bin/gconsole --help
```

```bash
# bin/gconsole --help
Gstore Ver 1.3 for Linux on x86_64 (Source distribution)
Gstore Console(gconsole), an interactive shell based utility to communicate with gstore repositories.
Copyright (c) 2016, 2024, pkumod and topgraph and/or its affiliates.

Usage: bin/gconsole [OPTIONS]
  -?, --help          Display this help and exit.
  -u, --user          username. 
  
Supported command in gconsole: Type "?" or "help" in the console to see info of all commands.
```

**(3)Commands**

- Ends with `;` and can span multiple lines
- Supports using `>` and `>>` to redirect output
- Supports single-line comments starting with `#` until a new line
- Supports command completion, file name completion, line editing and history commands viewing
- Abandon the current command (end execution or abandon editing) with `ctrl+C`, exit the command line with `ctrl+D`
- To enter gconsole, you need to log in; only commands that meet the current user permissions will be executed, and only the content that meets the current user permissions will be displayed.
  - 7 types of permissions: `query`, `load`, `unload`, `update`, `backup`, `restore`, `export`
- Command keywords are case-insensitive. Command-line options, database names, user names, and passwords are case-sensitive. Command keywords as follows:
  - Database management: `sparql`, `create`, `use`, `drop`, `show`, `showdbs`, `backup`, `restore`, `export`, `pdb`
  - Identity: `flushpriv`, `pusr`, `setpswd`
  - Configuration information viewing: `settings`, `version`
  - Permissions management: `setpriv`, `showusrs`, `pusr`
  - User management: `addusr`, `delusr`, `setpswd`, `showusrs`
  - Help and miscellaneous: `quit`, `help/?`, `pwd`, `clear`

### Database management

**(1)Build database**

```bash
create <database_name> [<nt_file_path>]
```

- Read `.nt` file from nt_file_path and create a database named database_name, or create an empty database
- \<database_name>: cannot be `system`
- \<nt_file_path>：file path

```
gstore> create example friend.nt；
... (this is build database process output, omitted in this document)
Add database info success.
Build RDF database example successfully!
```

**(2)Delete database**

```
drop <database_name>
```

- Cannot delete the current database
- \<database_name>：can only be specified as the name of a database for which the current user has full permissions

```
gstore> drop <database_name>;
```

**(3)Load/Switch database**

```
use <database_name>
```

- Load/switch the current database: unload the previous current database from memory (if any) and load the specified database
- The target database needs to have been created
- \<database_name>：can only be specified as the name of a database for which the current user have load and unload permissions

```
gstore> use example;
... (this is load process output, omitted in this document)
Current database switch to example successfully.
```

**(4)Query/update database**

- Run a SPARQL query on the current database
- You need to specify the current database first by`use <database_name>`

**(5)Inputting a SPARQL query**

```bash
gstore> # show all in db
     -> SELECT ?x ?y ?z 
     -> WHERE{
     ->     ?x ?y ?z. # comment
     -> } ;
... (this is query process output, omitted in this document)
---------------------------------------
|    ?x     |     ?y      |    ?z     |
---------------------------------------
| <Alice>   | <关注>      | <Bob>     |
| <Bob>     | <关注>      | <Alice>   |
| <Carol>   | <关注>      | <Bob>     |
| <Dave>    | <关注>      | <Alice>   |
| <Dave>    | <关注>      | <Eve>     |
| <Alice>   | <喜欢>      | <Bob>     |
| <Bob>     | <喜欢>      | <Eve>     |
| <Eve>     | <喜欢>      | <Carol>   |
| <Carol>   | <喜欢>      | <Bob>     |
| <Francis> | <喜欢>      | <Carol>   |
| <Alice>   | <不喜欢>    | <Eve>     |
| <Carol>   | <不喜欢>    | <Francis> |
| <Francis> | <不喜欢>    | <Eve>     |
| <Francis> | <不喜欢>    | <Dave>    |
| <Dave>    | <不喜欢>    | <Francis> |
---------------------------------------
query database successfully, Used 127 ms

```

Example of redirection (all commands support redirecting output):

```bash
gstore> # support redirect
	-> SELECT ?x ?y ?z
	-> WHERE{
	-> ?x ?y ?z.
	-> } > my_test_data/output ;
Redirect output to file: my_test_data/output

```

**(6)sparql <sparql_file>**

```shell
sparql sparql_file
## sparql_fileis a SPARQL query file, which can be either a relative path or an absolute path.
```

- Specify the sparql file: the file contains multiple sparql statements that need to be separated by `;` and the last `;` is optional
- The file content supports single-line comments starting with #

```bash
gstore> sparql all.sparql ;
... (this is query process output, omitted in this document)
----------------------------------
|    ?s     |    ?p    |   ?o    |
----------------------------------
| <Alice>   | <关注>   | <Bob>   |
| <Bob>     | <关注>   | <Alice> |
| <Carol>   | <关注>   | <Bob>   |
| <Dave>    | <关注>   | <Alice> |
| <Dave>    | <关注>   | <Eve>   |
| <Alice>   | <喜欢>   | <Bob>   |
| <Bob>     | <喜欢>   | <Eve>   |
| <Eve>     | <喜欢>   | <Carol> |
| <Carol>   | <喜欢>   | <Bob>   |
| <Francis> | <喜欢>   | <Carol> |
----------------------------------
query database successfully, Used 121 ms
```

- An example of query.sparql content:

```sql
SELECT	?x ?y ?z
WHERE {
    ?x ?y ?z.
}
```

**(7)Display database information**

```
show [<database_name>] [-n <displayed_triple_num>]
```

- Display meta information and the first displayed_triple_num triples (the first 10 triples are displayed by default)
- \<database_name>: the database name. If database_name is not specified, the information of the current database will be displayed.
- -n <displayed_triple_num>: number of lines displayed

```
gstore> show example;
... (this is load and query process output, omitted in this document)
---------------------------------------
|     name      |        value        |
---------------------------------------
| database      | example             |
| creator       | root                |
| built_time    | 2024-08-06 17:23:47 |
| triple_num    | 15                  |
| entity_num    | 6                   |
| literal_num   | 0                   |
| subject_num   | 6                   |
| predicate_num | 3                   |
---------------------------------------
----------------------------------
|    ?s     |    ?p    |   ?o    |
----------------------------------
| <Alice>   | <关注>   | <Bob>   |
| <Bob>     | <关注>   | <Alice> |
| <Carol>   | <关注>   | <Bob>   |
| <Dave>    | <关注>   | <Alice> |
| <Dave>    | <关注>   | <Eve>   |
| <Alice>   | <喜欢>   | <Bob>   |
| <Bob>     | <喜欢>   | <Eve>   |
| <Eve>     | <喜欢>   | <Carol> |
| <Carol>   | <喜欢>   | <Bob>   |
| <Francis> | <喜欢>   | <Carol> |
----------------------------------
```

**(8)View all databases**

```
showdbs
```

- Only the databases for which the current user has query permissions can be displayed.

```bash
gstore> showdbs;
--------------------------------------
| database | creater |    status     |
--------------------------------------
| system   | root    |               |
| test1    | root    | already_built |
| test     | root    | already_built |
| test2    | root    | already_built |
| test3    | root    | already_built |
| example  | root    | already_built |
```

**(9)Display current database name**

```
pdb
```

```
gstore> pdb;
example
```

**(10)Backup current database**

```
backup [<backup_folder>]
```

- \<backup_folder> specifies the backup path.

```
gstore> backup;
... (this is backup process output, omitted in this document)
Database example backup successfully.
```

```
gstore> backup back;
... (this is backup process output, omitted in this document)
Database example backup successfully.
```

**(11)Export current database**

```
export <file_path>
```

- \<file_path> specifies the export path.

```
gstore> export example.nt;
Database example export successfully.
```

**(12)Restore database**

```
restore <database_name> <backup_path>
```

- \<database_name>: database name.
- \<backup_path>: backup file path.

```
gstore> restore example backups/example.db_220929114732/
... (this is restore process output, omitted in this document)
Database example restore successfully.
```

### User Identity

**(1)Refresh permissions**

```bash
flushpriv
```

Read db and refresh current user permissions

```bash
gstore> flushpriv;
Privilige Flushed for current user successfully.
```

**(2)Display current username and permissions**

```bash
pusr [<database_name>]
```

- `pusr` displays the current username
- `pusr <database_name>` displays the current user name and the current user's permissions on <database_name>

```bash
gstore> pusr;
usrname: yuanzhiqiu

gstore> pusr example;
usrname: yuanzhiqiu
privilege on eg: query load unload update backup restore export

```

**(3)Modify the current user's password**

```
setpswd
```

- Password verification is required.

```bash
gstore> setpswd;
Enter old password:
Enter new password:
Enter new password again:
Not Matched.
Enter new password:
Enter new password again:
Password set successfully.

```

### Configuration information viewing

**(1)View congifuration**

```
settings [<conf_name>]
```

- Show all configuration information.

```
gstore> settings;
Settings: 
thread_num      30
... (this is other settings, omitted in this document)
You can Edit configuration file to change settings: ./conf/conf.ini
```

- \<conf_name> displays the configuration information of the specified conf_name.

```bash
gstore> settings ip_deny_path;
"./conf/ipDeny.config"
You can Edit configuration file to change settings: ./conf/conf.ini
```

**(2)View version**

```bash
version
```

- Output current version information.

```bash
gstore> version;
Gstore version: 1.3 Source distribution
Copyright (c) 2016, 2024, pkumod and topgraph and/or its affiliates.
```

### Permissions management

Only system users have the permission to execute commands related to permissions management. The permissions descriptions are as follows:

- Permissions: the operations a user is permitted to carry out on a database

- 7 types of permissions: `query`, `load`, `unload`, `update`, `backup`, `restore`, `export`

  ```
  [1]query [2]load [3]unload [4]update [5]backup [6]restore [7]export
  ```
  
 Users who have all seven permissions are considered to have full permissions.
  
- System users have full permissions on all databases. (System users are defined in conf.ini and system.db.)

**(1)Set user's permissions**

```
setpriv <usrname> <database_name>
```

- A password is required to verify root identity.
- \<usrname>: the user name to be set.
- \<database_name>: database name.

```bash
gstore> setpriv zero eg;
Enter your password:
[1]query [2]load [3]unload [4]update [5]backup [6]restore [7]export [8]all
Enter privilege number to assign separated by whitespace:
1 2 3
[will set priv:]00001110
Privilege set successfully.

```

**(2)View user's permissions**

```bash
pusr <database_name> <usrname>
```

- \<usrname> : the user name.
- \<database_name>: database name.

```bash
gstore> pusr eg yuanzhiqiu;
privilege on eg: query load unload update backup restore export
```

**(3)View all users and their permissions**

```bash
showusrs
```

Show which users there are and the database permissions for each user (non-root users only; only databases with has_xxx_priv for this user are displayed.)

Output format:

```bash
usr
------------------------
privilege on databases

```

```bash
----------------------------------------------------------
| user |                    privilege                    |
----------------------------------------------------------
| root | all privilege on all db                         |
| uki  | unload: query load unload update backup export  |
----------------------------------------------------------
```

### User management

Only system users have the permission to execute user management commands.

**(1)Add users**

```bash
addusr <usrname>
```

- A password is required to verify root identity.
- \<usrname>: the new user name.

```bash
gstore> addusr uki;
Enter your password:
Enter password for new user: hello
Add usr uki successfully.
```

**(2)Delete users**

```bash
delusr <usrname>
```

- A password is required to verify root identity.
- \<usrname>: the user name to be deleted.

```bash
gstore> delusr cat;
Enter your password:
Del usr cat successfully.
```

**(3)Reset passwords**

```bash
setpswd <usrname>
```

- A password is required to verify root identity.
- \<usrname>: user name.

```bash
gstore> setpswd zero;
Enter your password:
Enter new password:
Enter new password again:
Password set successfully.
```

**(4)View all users**

```bash
showusrs
```

```bash
----------------------------------------------------------
| user |                    privilege                    |
----------------------------------------------------------
| root | all privilege on all db                         |
| uki  | unload: query load unload update backup export  |
----------------------------------------------------------
```

### Help and miscellaneous

**(1)End/Cancel the current command**

`ctrl+C`

End the command that is currently being executed:

```bash
...(executing some cmd) (ctrl+C here)
gstore>

```

Or discard the command you are currently typing:

```bash
gstore> SELECT ?x ?y ?z
-> WHERE{ (ctrl+C here)
gstore>

```

**(2)Quit**

`quit` / `ctrl+D` 

This will unmount the current database from memory if it is not yet unmounted.

```bash
gstore> quit;
[xxx@xxx xxx]#

```

```bash
gstore> (ctrl+D here)
[xxx@xxx xxx]#

```

**(3)Help**

```bash
help [edit/usage/<command>]

```

- `help` displays the help information about all commands:

```bash
gstore> help;
Gstore Console(gconsole), an interactive shell based utility to communicate with gStore repositories.

For information about gStore products and services, visit:
   http://www.gstore.cn/
For developer information, including the gStore Reference Manual, visit:
   http://www.gstore.cn/pcsite/index.html#/documentation

Commands end with ;. Cross line input is allowed.
Comment start with #.
Type 'cancel;' to quit the current input statement.

List of all gconsole commands:
sparql          Answer SPARQL query(s) in file.
create          Build a database from a dataset or create 
...(this is help msg for other commands, omitted in this document)

Other help arg:
edit    Display line editing shortcut keys supported by gconsole.
usage   Display all commands as well as their usage.

```

`help edit` displays the line editing shortcut operations of `GNU readline`:

```bash
gstore> help edit;
Frequently used GNU Readline shortcuts:
CTRL-a  move cursor to the beginning of line
CTRL-e  move cursor to the end of line
CTRL-d  delete a character
CTRL-f  move cursor forward (right arrow)
CTRL-b  move cursor backward (left arrow)
CTRL-p  previous line, previous command in history (up arrow)
CTRL-n  next line, next command in history (down arrow)
CTRL-k  kill the line after the cursor, add to clipboard
CTRL-u  kill the line before the cursor, add to clipboard
CTRL-y  paste from the clipboard
ALT-b   move cursor back one word
ALT-f   move cursor forward one word
For more about GNU Readline shortcuts, see https://en.wikipedia.org/wiki/GNU_Readline#Emacs_keyboard_shortcuts

```

`help usage` displays the description and usage of all commands:

```bash
gstore> help usage;
List of all gconsole commands:
Note that all text commands must be end with ';' but need not be in one line.
sparql          Answer SPARQL query(s) in file.
                sparql <; separated SPARQL file>;
create          Build a database from a dataset or create an empty database.
                create <database_name> [<nt_file_path>];
...(this is help msg for other commands, omitted in this document)

```

`help <command>` displays the help information about the command:

```bash
gstore> help use;
use 	Set current database.
		use <database_name>;
```

**(4)Clear screen**

```bash
gstore> clear;
```

**(5)View current working path**

```bash
gstore> pwd;
/<path_to_gstore>/gstore

```

## HTTP API service  

GStore provides two sets of http API services, namely grpc and ghttp. Users can remotely connect to and operate on gStore by sending http requests.

### Starting ghttp/gRPC service

After gStore is compiled, a ghttp service and a gRPC service is displayed in the bin directory of gStore. However, the service is not started by default. You need to manually start the service: 

```bash
nohup bin/grpc -db db_name -p serverPort -c enable &
```

```bash
nohup bin/ghttp -db db_name -p serverPort -c enable &
```

Parameters:

```
db_name: The name of the database to start HTTP (Optional; The effect of this parameter is that when starting the HTTP service, the related information of the database will be loaded into memory.)

serverPort: the HTTP listening port. You need to ensure that this port is not prohibited by the server firewall. (Optional; if this port is not specified, the default port is 9000).

enable: whether to load CSR resources. 0 indicates no and 1 indicates yes. (Optional, for built-in advanced functions or user-defined graph analysis functions. The default value is 0.)
```

`ghttp` and `grpc` both support GET and POST request types.

Concurrent read-only queries are supported, but when a query containing updates arrives, the
entire database is locked. On a computer with dozens of kernel threads, the recommended
number of queries to run concurrently is less than 300, but we were able to run 13,000 queries
simultaneously in our experiment. To use the concurrency feature, it is best to set the number of open files and the maximum number of processes to 65535 or greater in the system settings.

**It is recommended to periodically execute the `checkpoint` command  to the console if you frequently send query containing updates. Otherwise, updates may not be synced with the disk, and will be lost if the server stops abnormally (for example, because of typing "Ctrl + C") **

### Shutting down the service

For safety, please use the following command to shut down the service instead of `Ctrl + C` or `kill`.

```bash
bin/shutdown
```

### HTTP API

ghttp and gRPC provide a rich API interface so that users can remotely operate most of the functions of gStore. For details, please see [Development document] - [common API] - [ghttp interface Description]  or [gRPC interface Description].

## Socket API service

gServer is an external access interface provided by gStore and a Socket API service. Users can connect and operate gStore remotely through two-way socket communication.

### Start gServer service

After gStore is compiled, a gServer service is displayed in the bin directory of gStore. However, the gServer service is not started by default. You need to manually start the gServer service:

```shell
bin/gserver -s
```

Other optional parameters:

```
-t,--stop: shutdown gserver service;
-r,--restart：restart gserver service；
-p,--port：Modify the socket connection port configuration. The default port is 9000. After the modification, restart the GServer service
-P,--printport：pritn current socket connection port configuration
-d,--debug：start debug mode（keep gserver service running in the foreground）
-k,--kill：Forcibly stop the service. You are advised to stop the service only when the service cannot be stopped
```



### Shutdown gServer service

To stop the gServer service, use the following command to stop it. It is better not to simply enter the command 'Ctrl + C' or 'kill' to stop the gServer, because it is not safe.  

```shell
bin/gserver -t
```



### gServer related API

gServer provides rich API interfaces so that users can remotely operate most functions of gStore. See [Development document] - [common API] - [gServer interface Description] for specific interfaces.