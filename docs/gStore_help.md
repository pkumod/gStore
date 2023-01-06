# gStore1.0 Version User Guide

## 1.Update Log

### 1.1 Code Version Update Log

---

**gStore 1.0 Version**

- Update Time：2022-10-01

- Updated functions:

  - Support of user-defined graph analysis functions: users can manage their own graph analysis functions through the API interfaces or the visual management platform gStore-workbench. Users can obtain the number of nodes and edges of the graph and neighbors of any given node, etc. through interface functions and use them as basic units to implement their own graph analysis functions. Dynamic compilation and execution of user-defined graph analysis functions are supported.

  - The gRPC network interface service: gRPC is a high-performance network interface service based on HTTP protocol implemented based on the open source library `workflow`, which further improves the efficiency and stability of the interface service. Experiments show that gRPC achieves a great improvement in concurrent access performance compared with ghttp, the previous network interface; for example, in the case of **2000/QPS**, the rate of denied access is **0%**.
  - gConsole module: in gStore 1.0, we launched the gConsole module, which enables the long-session operation of gStore with contextual information.
  - Decoupling of the optimizer and executor: gStore 1.0 decouples the optimizer and executor, converting from the original deeply coupled greedy strategy to a query optimizer based on dynamic programming and a query executor based on breadth-first traversal.
  - Optimization of Top-K queries: We implemented a Top-K SPARQL processing framework based on the DP-B algorithm in gStore, including query segmentation and sub-result aggregation.
  - Support of ACID transactions: by introducing the multi-version management mechanism, gStore 1.0 can start ACID transactions for insert and delete operations, which users can open, commit, and roll back. Currently gStore 1.0 supports four isolation levels: read-uncommitted, read-committed, repeatable read and serializable.
  - Reconstruction of database kernel and optimization of the plan tree generation logic: in gStore 1.0, two types of join operations (worst-case-optimal joins and binary joins) are introduced to optimize query execution and further improve query efficiency.
  - Optimized logging module: based on the log4cplus library, the system logs can be output in a unified format. Users can configure the log output mode (console output or file output), output format, and output level.
  - New built-in advanced functions: gStore 1.0 supports four new advanced functions, namely triangleCounting, closenessCentrality, bfsCount and kHopEnumeratePath.
  - Extended support for BIND statements: gStore 1.0 supports assigning values to variables using algebraic or logical expressions in BIND statements.
  - Optimization of some local commands and API interfaces (e.g., the shutdown command), and fixing a series of bugs (e.g., more accurate gmonitor statistics).

**gStore 0.9.1 Version**

- Update Time：2021-11-23

- Updated functions:

    - By separating gStore kernel parsing and execution, the query performance can be further improved through join order and other technologies, and the performance can be improved by more than 40% in complex queries;

    - Rewrite the http service component ghttp of gStore, and add user permissions, heartbeat detection, batch import, batch delete and other functions, and write a standard GHTTP API document (see interface list), further enrich the functions of GHTTP, improve the robustness of GHTTP; 

    - Add Personalized PageRank (PPR) custom function, which can be used to calculate the relatedness between entities to find the node with the most influence;

    - Added support for arithmetic and logical operations in Filter statements, such as arithmetic operations (e.g.? x + ? Y = 5); Logical operations (e.g.? x + ? y = 5 && ? Y > 0); 

    - The gServer component is added to realize two-way Socket API communication. Users can access gStore remotely through GHTTP component and gServer component;

    - The local operation command format is planned, and the --help command is introduced. Users can view the detailed command format of each function. For example, bin/gbuild -h/--help can view the detailed command format of gbuild;

    - Fixed a number of bugs.

**gStore 0.9.0 Version**

- Update Time：2021-02-10

- Updated functions：
  - Upgrade the SPARQL parser generator from ANTLR V3 to the latest, well-documented, and well-maintained V4;
  - Support writing numeric literals without data type suffixes in SPARQL queries;
  - Support for arithmetic and logical operators in SELECT clauses;
  - Supports aggregations of SUM, AVG, MIN, and MAX in the SELECT clause;
  - Additional support is built into filters, and function functions including `datatype`，`contains`，`ucase`，`lcase`，`strstarts`，`now`，`year`，`month`，`day`，and`abs`;
  - Supports path-related functions as an extension of SPARQL 1.1, including loop detection, shortest paths, and K-Hop reachability;
  - Supports full and incremental database backup and recovery. Administrators can enable automatic full backup;
  - Supports Log-based rollback operations;
  - Supports transactions with three levels of isolation: committed reads, snapshot isolation, and serializable  ;
  - Expand the data structure to accommodate large-scale graphs of up to 5 billion triples.



### 1.2 Document Version Update Log   

---

**2022.10.1 gStore 1.0**

- gStore 1.0 version document is available for download
- Modified the contents about Qucik Start to match gStore 1.0 version
- Modified the contents about Installation Instructions to match gStore 1.0 version
- Modified common API content to match gStore 1.0 version
- Modified Workbench console content to match gStore 1.0 version
- Added gStore 1.0 version release in gStore Chronology
- Added update log to record the version of gStore and related document updates

Previously:

- Modified the contents about Quick Start to match gStore 0.9.1 version
- Modified common API content to match gStore 0.9.1 version
- Modified Workbench console content to match gStore 0.9.1 version
- Added update log to record the version of graph database gStore and related document updates
- Added a document download directory for users to download documents

<div STYLE="page-break-after: always;"></div>

## 2. Introduction of Knowledge Graph and gStore

### 2.1 Introduction of Knowledge Graph

---

In recent years, with the revival of the concept of artificial intelligence (AI), in addition to the hot term deep learning, **knowledge graph** (KG) is undoubtedly another “silver bullet” in the eyes of researchers, industry and investors. To put it simply, KG is a sematic network, which models the entities (including properties) and the relation between each other. Below is an example extracted from a well-known knowledge graph DBpedia. There are seveval entities in the example, including “James Watt”, “Adam Smith”, “Joseph Black” and so on. This diagram clearly shows the individual attributes and attribute values of “James Watt” (e.g., name, date of birth, etc.) as well as the relationships between them (e.g., James Watt once worked at University of Glasgow, James Watt was born in Greenock, etc.). 

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221205100147727.png" alt="image-20221205100147727" style="zoom:50%;" />



RDF (Resource Description Framework) is proposed by W3C and is the de-facto data model for knowledge graphs. The proliferation of knowledge graphs has generated many RDF data management problems. Generally, RDF is a collection of triples, denoted as <subject, predicate, object>, describing relationships between different entities or the attribute values of entities. To retrieve and manipulate RDF graphs, an RDF query language, SPARQL, is proposed. More details of RDF and SPARQL can be found in [1] and [2]. Following is an example of a RDF dataset, describing facts about the scientist James Watt and his relevant entities. 

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221205100314289.png" alt="image-20221205100314289" style="zoom:50%;" />

<center>Figure 1-1 RDF data example<center>



Like SQL, SPARQL is also a descriptive structured query language. That is, users only need to describe the information they want to query according to the syntax rules defined by SPARQL, without specifying the steps of the computer to perform the query. SPARQL became an official W3C standard in January 2008. The WHERE clause in SPARQL defines the query criteria, which are also represented by triples. The following figure illustrates a SPARQL example to retrieve the workplace of James Watt. 

<center>
<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221205100601426.png" alt="image-20221205100601426" style="width:30%;" />

<center>Figure 1-2 SPARQL Query example<center>


A fundamental problem of knowledge graph data management is how to store RDF datasets and answer SPARQL queries efficiently. In general, there are two completely different technology roadmaps. One is resorting to relational database systems, which stores RDF data in carefully-designed relational tables and answers SPARQL queries by converting them into SQL queries. The other is to directly develop a native graph database to store RDF triples. RDF can be represented by a directed graph, where each subject or object is a vertex and each triple is a directed edge to connecting the corresponding subject and object. The following figure gives the corresponding query graph of the example SPARQL query.



<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221205100745603.png" alt="image-20221205100745603" style="zoom: 25%;" />



From the graph perspective, a SPARQL query can be regarded as a query graph and can be answered by subgraph matching techniques. The following figure shows how the example SPARQL query graph matches the example RDF graph. This is the core idea of our gStore system and the first work was published at VLDB 2011 [**]. Furthermore, we design a series of optimization techniques in gStore, such as structure-aware indexes, subgraph query optimization and distributed processing. 

![image-20221205100937067](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221205100937067.png)

<center>Figure 1-3 Subgraph matching of the example SPARQL query over the example RDF graph<center>



References

[1] RDF - Semantic Web Standards, 2022-12-4, https://www.w3.org/RDF/

[2] SPARQL 1.1 Query Language, 2022-12-4, https://www.w3.org/TR/sparql11-query/


### 2.2 Introduction of gStore

---

gStore is an open-source native graph database for RDF data. The first publication of gStore began with the collaboration between Prof. Lei Zou (Peking University), Prof. Tamer Özsu (University of Waterloo) and Prof. Lei Chen (Hong Kong University of Science and Technology) in the VLDB 2011 paper “Lei Zou, Jinghui Mo, Lei Chen, M. Tamer Ozsu, Dongyan Zhao, gStore: Answering SPARQL Queries Via Subgraph Matching, Proc. VLDB 4(8): 482-493, 2011”. The paper proposed a query execution scheme using subgraph matching to answer Basic Graph Pattern (BGP) statements in SPARQL. Since the publication of VLDB 2011, the PKUMOD group (led by Prof. Lei Zou) in Peking University has been continuously engaged in the open-sourcing, maintenance and optimization of gStore system.

gStore is an open-source project on both Github and Gitee, following BSD 3-clause, which is widely used in the research and open-source community. We would like to warmly invite all researchers and developers interested in gStore to join and contribute to this open-source project via Github or Gitee, and build gStore-related knowledge graph and graph database technique ecology together. According to the BSD 3-clause, we allow users to modify and redistribute codes freely on the premise of fully respecting the copyright of code authors, and also allow users to develop, distribute and sell commercial software freely on the basis of gStore codes. However, the above conditions must meet the relevant legal provisions stipulated in Chapter 10 “Legal Provisions” according to the BSD 3-clause open-source agreement. We strictly require users to mark “powered by gStore” and the gStore logo (see gStore Logo for details) on the software they distribute based on the gStore code. We strongly recommend that users refer to the “Open Source and Legal Provisions” before using gStore.

<div STYLE="page-break-after: always;"></div>

## 3. Installation Instructions

### 3.1 System Requirements

---

| Project          | Requirement                                                  |
| :--------------- | :----------------------------------------------------------- |
| Operating System | Linux, such as CentOS, Ubuntu etc.                           |
| Framework        | x86_64                                                       |
| Disk Size        | Depends on the size of the data set                          |
| Memory Size      | Depends on the size of the data set                          |
| glibc            | Must install version >= 2.14                                 |
| gcc              | Must install version >= 5.0                                  |
| g++              | Must install version >= 5.0                                  |
| make             | Must install                                                 |
| cmake            | Must install version >= 3.6                                  |
| pkg-config       | Must install                                                 |
| uuid             | Must install                                                 |
| boost            | Must install version >= 1.56 && <= 1.59                      |
| readline-devel   | Must install                                                 |
| curl-devel       | Must install                                                 |
| openssl-devel    | Must install version >= 1.1                                  |
| openjdk-devel    | If using the Java API, must install version = 1.8.x          |
| requests         | If using the Python http API, must install                   |
| node             | If using the Node.js http API, must install version >=10.9.0 |
| pthreads         | If using the PHP http API, must install                      |
| realpath         | If using gconsole, must install                              |
| ccache           | Optional, used to speed up compilation                       |

### 3.2 Installation Environment

---

Running the corresponding scripts in scripts/setup/ for your operating system will automatically solve most of your problems for you. For example, if you are an Ubuntu user, you can execute the following command:  

```bash
$ . scripts/setup/setup_ubuntu.sh
```

**Before running the script, **we recommend you install gcc and g++ 5.0 or later.

Of course, you can also choose to manually prepare the environment step-by-step; Detailed installation instructions for each system requirement are provided below.

#### 1. gcc and g++ installation

Check g++ version：

```bash
$ g++ --version
```

If the version is earlier than 5.0, reinstall 5.0 or later version. Using 5.4.0 as an example :(for Ubuntu and CentOS) 

```bash
$ wget http://ftp.tsukuba.wide.ad.jp/software/gcc/releases/gcc-5.4.0/gcc-5.4.0.tar.gz
$ tar xvf gcc-5.4.0.tar.gz 
$ cd gcc-5.4.0
$ ./contrib/download_prerequisites
$ cd .. 
$ mkdir gcc-build-5.4.0 
$ cd gcc-build-5.4.0 
$ ../gcc-5.4.0/configure --prefix=/opt/gcc-5.4.0 --enable-checking=release --enable-languages=c,c++ --disable-multilib
$ sudo make -j4   #Allows four compile commands to be executed simultaneously, speeding up the compilation process
$ sudo make install
```

Ubuntu can also be installed directly using the following commands:

```bash
$ apt install -y gcc-5 g++-5
```

After successful installation,

- **You need to change the default versions of GCC and g++** If gcc and g++ 5.0 or later are installed in the '/prefix/bin' directory, run the following command：

  ```bash
  $ export PATH=/prefix:$PATH
  ```

- **The dynamic link library path needs to be modified: **If gcc and g++ dynamic link libraries above 5.0 are in the '/prefix/lib' path, you need to run the following comman：

  ```bash
  $ export LD_LIBRARY_PATH=/prefix/lib:$LD_LIBRARY_PATH
  ```

#### 2. readline-devel installation

Check whether readline-devel is installed

```bash
$ yum list installed | grep readline-devel	# CentOS
$ dpkg -s readline						# Ubuntu
```

if not, install it

```bash
$ sudo yum install readline-devel		# CentOS
$ sudo apt install -y libreadline-dev	# Ubuntu
```

#### 3. boost installation（Please use 1.56-1.59）

Check whether boost is installed 

```bash
$ yum list installed | grep boost	    # CentOS
$ dpkg -s boost					        # Ubuntu
```

If not, install it：（use version 1.56.0 as example）

version:1.56.0

address: http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz

Installation script: (for CentOS and Ubuntu)

```bash
$ wget http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz$ tar -xzvf boost_1_56_0.tar.gz$ cd boost_1_56_0$ ./bootstrap.sh$ sudo ./b2$ sudo ./b2 install
```

Ubuntu can also be installed directly using the following commands:

```bash
$ sudo apt install -y libboost-all-dev
```

**Note: please install boost after ensuring that the g++ version is above 5.0** Undefined reference to 'boost::...' - undefined reference to 'boost::... '"), most likely because you compiled Boost with GCC versions lower than 5.0. At this point, recompile Boost using the following step:

- Clear old files: `./b2 --clean-all`
- In the user-config.jam file under./tools/build/ SRC (if this file does not exist under this path, Please find a sample user-config.jam file under./tools/build/example or some other directory and copy it to./tools/build/ SRC) to add  ：`using gcc : 5.4.0 : gcc-5.4.0's path ;`
- Run under ./  `./bootstrap.sh --with-toolset=gcc`
- `sudo ./b2 install --with-toolset=gcc`

Then recompile gStore (please start from 'make pre')

After successful installation，

- **Need to modify the dynamic link library path:** Assuming boost's dynamically linked library is in the '/prefix/lib' path, you need to execute the following command：

  ```bash
  $ export LD_LIBRARY_PATH=/prefix/lib:$LD_LIBRARY_PATH
  ```

- **The header file path needs to be changed: **Assuming boost's header file is in the '/prefix/include' path, you need to execute the following command:

  ```bash
  $ export CPATH=/prefix/include:$CPATH
  ```

#### 4. curl-devel installation 

Check whether curl-devel is installed

```bash
$ yum list installed | grep readline-devel	# CentOS
$ dpkg -s readline						# Ubuntu
```

if not, then install：

version: 7.55.1

address: https://curl.haxx.se/download/curl-7.55.1.tar.gz

Installation scripts (for CentOS and Ubuntu)

```bash
$ wget https://curl.haxx.se/download/curl-7.55.1.tar.gz
$ tar -xzvf  curl-7.55.1.tar.gz
$ cd curl-7.55.1
$ ./configure
$ make
$ make install  
```

Or use the following command to install

```bash
$ yum install -y curl libcurl-devel              # CentOS
$ apt-get install -y curl libcurl4-openssl-dev   # Ubuntu
```

#### 5. openssl-devel installation

Check whether openssl-devel is installed

```shell
$ yum list installed | grep openssl-devel    # CentOS
$ dpkg -s libssl-dev                         # Ubuntu
```

如果没有安装，则安装

```shell
$ yum install -y openssl-devel                # CentOS          
$ apt-get install -y libssl-dev zlib1g-dev    # Ubuntu
```

#### 6. cmake installation

Check whether cmake is installed

```bash
$ cmake --version				# CentOS
$ cmake --version				# Ubuntu
```

if not, install:

version: 3.6.2

address: https://curl.haxx.se/download/curl-7.55.1.tar.gz

Installation scripts (for CentOS and Ubuntu)

```bash
$ wget https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz
$ tar -xvf cmake-3.6.2.tar.gz && cd cmake-3.6.2/
$ ./bootstrap
$ make
$ make install
```

Ubuntu can also be installed directly using the following commands:

```bash
$ sudo apt install -y cmake
```

#### 7. pkg-config installation

Check whether pkg-config is installed

```bash
$ pkg-config --version		# CentOS
$ pkg-config --version		# Ubuntu
```

if not, install

```bash
$ sudo yum install pkgconfig.x86_64         # CentOS
$ sudo apt install -y pkg-config            # Ubuntu
```

#### 8. uuid-devel installation

Check whether uuid-devel is installed 

```bash
$ yum list installed | grep libuuid-devel    # CentOS
$ dpkg -s uuid-dev                           # Ubuntu
```

if not, install

```bash
$ sudo yum install libuuid-devel	    # CentOS
$ sudo apt install -y uuid-dev		    # Ubuntu
```

#### 9. unzip/bzip2 installation (Optional)

Used to decompress zip and bz2 packages during an offline installation of gStore (where there is no Internet connection and all dependencies are compressed packages); unnecessary during online installation.

Check whether unzip is installed:

```bash
$ yum list installed | grep unzip		# CentOS
$ dpkg -s unzip  						# Ubuntu
```

if not, install

```bash
$ sudo yum install -y unzip			# CentOS
$ sudo apt-get install unzip		# Ubuntu
```

Replace "unzip" with "bzip2" in the above commands to check whether bzip2 is installed and install it if necessary.

#### 10. OpenJDK installation (Optional)

OpenJDK is only necessary if you need to use the Java API. You can first check whether there is an existing JDK environment with the following commands; if there is an existing JDK environment, and its version is 1.8.X, re-installation is unnecessary.

```bash
$ java -version
$ javac -version
```

if there is no existing JDK 1.8.X environment, install

```bash
$ yum install -y java-1.8.0-openjdk-devel # CentOS
$ sudo apt install -y openjdk-8-jdk       # Ubuntu
```

<div STYLE="page-break-after: always;"></div>

### 3.3  gStore obtainment

---

If you encounter permission issues, please prefix the command with `sudo` .

#### Method 1: download 

gStore has been uploaded to gitee (code cloud), which is recommended for faster download for users in mainland China. The website is https://gitee.com/PKUMOD/gStore.
You can also open https://github.com/pkumod/gStore, download gStore.zip, then decompress the zip package.

#### Method 2: clone (Recommended)

Run the following code to clone:

```bash
$ git clone https://gitee.com/PKUMOD/gStore.git  # gitee (code cloud) faster for users in mainland China
$ git clone https://github.com/pkumod/gStore.git # GitHub

# using the -b parameter when getting historical versions, such as -b 0.9.1
$ git clone -b 0.9.1 https://gitee.com/PKUMOD/gStore.git
$ git clone -b 0.9.1 https://github.com/pkumod/gStore.git
```

Note: Git need to be installed first.

```bash
$ sudo yum install git		# CentOS
$ sudo apt-get install git	# Ubuntu
```

### 3.4 gStore compilation

---

Switch to the gStore directory:

```bash
$ cd gStore
```

Execute the following commands:

```bash
$ make pre
# The following command can speed up compilation with -j4
$ make
#If the compilation completes successfully, the following string will appear at the end: Compilation ends successfully!
```

If 'make pre' still asks for g++ 5.0 or later after installing g++ 5.0 or later, locate g++ 5.0 or later and
run the following command in the gStore directory:

```bash
$ export CXX=<5.0 or later g++'s path>
```

Then `make pre` again. If the same error is reported after this step, please manually delete `CMakeCache.txt` and the `CMakeFiles` directory under ` tools/antlr4-cpp-runtime-4/` and `make pre ` again.

<div STYLE="page-break-after: always;"></div>

### 3.5 Deploy gStore using Docker 

---

> We provide two ways to deploy gStore from containers:  
>
> One is to build it yourself from the Dockerfile file in the project root and then run the container.  
>
> Another option is to download the image that has been automatically built and run it directly.  

#### 3.5.1  environment preparation

Docker, refer to the address [docker](https://blog.csdn.net/A632189007/article/details/78662741)

#### 3.5.2 Run by pulling the image directly(recommend)

No need to download the project or build your own, just enter`sudo docker pull pkumod/gstore:latest` pull images that have been automatically built on the Docker Hub。After pulling, `sudo docker run -p 9000:80 -it pkumod/gstore:latest `It can be directly started and used in container. 

#### 3.5.3 Build the image from Dockerfile  

---

Waiting for adjustment

---

There are probably a lot of other things that need to be added, so for now I've just added a basic version. The basic environment build is just the first step in containerization

<div STYLE="page-break-after: always;"></div>		

## 4. Quick Start

​	

### 4.1 Data Format

---

`gStore` is a graph database engine based on the RDF model, and its data format follows the RDF model. RDF, the W3C standard for describing real-world resources, is a general way to describe information so that it can be read and understood by computer applications. Any entity in the real world can be represented as a resource in the RDF model, such as a book's title, author, modification date, content, and copyright information. These resources can be used to abstract concepts, entities, and events from the objective world in the knowledge graph. An attribute of each resource and its attribute value, or its relationship to other resources, is called a piece of knowledge. Properties and relationships can be represented as triples.  

A triad consists of three elements: Subject, Property 1, and Object. It usually describes the relationship between two resources or some Property of a resource. When a triple describes the attributes of a resource, its three elements are also called body, attribute, and Property Value. For example, the triad "Aristotle, Birthplace, Chalcis" expresses the fact that Aristotle was born in Chalcis.

Using these properties and relationships, a large number of resources can be linked together to form a large RDF knowledge graph dataset. Therefore, a knowledge graph can often be viewed as a collection of triples. These collections of triples, in turn, form an RDF dataset. The triplet set of knowledge graph can be stored in relational database or graph database.

RDF data should be provided in n-triple format (XML is not currently supported), and queries must be provided in SPARQL1.1 syntax. The following is an example of the n-triple format file:        

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

More specific information about N-Triple please check [N-Triple](https://www.w3.org/TR/n-triples/). Not all syntax in SPARQL1.1 is parsed and answered in gStore; for example, property paths are beyond the capabilities of the gStore system.

<div STYLE="page-break-after: always;"></div>

### 4.2 Initialize the system database

---

As soon as you download and compile the code for the gStore system, a database named System (the real directory name system.db) is automatically created. This is the database that manages system statistics, including all users and all databases. You can query this database using the gquery command, but do not modify it using the editor. 

The `system` database is the built-in system database of gStore. This database cannot be deleted. It is used to store the information related to the system, especially the information of the built database  

#### 4.2.1 Command line mode(ginit)

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
================================================================================
UPDATE
Insert:
{
	<system>	<built_time>	"2021-02-21 22:50:05".
	<lubm>	<database_status>	"already_built".
	<lubm>	<built_by>	<root>.
	<lubm>	<built_time>	"2021-02-21 22:50:05".
}
================================================================================
parse query  successfully! .
unlock the query_parse_lock .
after Parsing, used 96ms.
write priviledge of update lock acquired
QueryCache cleared
Total time used: 97ms.
update num : 4
system.db is built successfully!
```

<div STYLE="page-break-after: always;"></div>

### 4.3 Create database

---

Creating a database is one of the most important operations in gStore. It is also the first operation that users need to perform after gStore installation. gStore provides multiple methods to create a database.  

#### 4.3.1 Command line mode (gbuild)

The gbuild command is used to create a new database from an RDF file:

```shell
bin/gbuild -db dbname -f filename  
```

Parameter definition：

	dbname：database name
	filename：filepath of files with ".nt" or ".n3" as suffix

For example, we build a database called "lubm.db" from lubm.nt, which can be found in the data folder.

```shell
[root@localhost gStore]$ bin/gbuild -db lubm -f ./data/lubm/lubm.nt 
gbuild...
argc: 3 DB_store:lubm      RDF_data: ./data/lubm/lubm.nt  
begin encode RDF from : ./data/lubm/lubm.nt ...
```

Note：

- You cannot create a database from an empty RDF dataset
- Note that you cannot directly `CD` into the `bin` directory. Instead, you need to perform the gbuild operation in the gStore installation root director



#### 4.3.2 Visual tool (gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



#### 4.3.3 HTTP API (ghttp)

gStore provides GHTTP component as HTTP API service component. Users can realize relevant functions by sending HTTP requests to ghttp In ghttp, graph database is constructed by `build` request. 



#### 4.3.4 Socket API (gServer)

gStore provides gServer component as Socket API service component. Users can realize related functions by sending Socket request to gServer. gServer builds graph database through `build` request. See [Development document] - [Common API] - [gServer interface Description] for details.

<div STYLE="page-break-after: always;"></div>

### 4.4 Database list

---

The database list function is used to obtain information about all available databases in the following formats

#### 4.4.1 Command line mode (gshow)

gshow is used to get the list of all available databases.

instruction：

```shell
bin/gshow
```

example：

```shell
[root@localhost gStore]$ bin/gshow 
========================================
database: system
creator: root
built_time: "2019-07-28 10:26:00"
========================================
database: lubm
creator: root
built_time: "2019-07-28 10:27:24"
```



#### 4.4.2 Visual tool (gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



#### 4.4.3 HTTP API (ghttp)

gStore provides ghttp component as http API service component, users can send http request to ghttp to achieve relevant functions, ghttp through the `show` command to achieve relevant functions, details see [development document] - [common API] - [ghttp interface description].

#### 4.4.4 Socket API (gServer)

gStore provides gServer component as Socket API service component. Users can realize related functions by sending Socket request to gServer. gServer displays database list through `show` request. See [Development document] - [Common API] - [gServer interface Description] for details.



<div STYLE="page-break-after: always;"></div>

### 4.5 Database status Query

---

The database status query function is used to obtain statistics about a specified database in the following ways.

#### 4.5.1 Command line mode (gmonitor)

gmonitor is used to get statistics about a specified database.

instruction：

```shell
bin/gmonitor -db db_name
```

parameter definition：

	db_name：database name 

example：

```shell
[root@localhost gStore]$ bin/gmonitor -db lubm
database: lubm
creator: root
built_time: "2019-07-28 10:27:24"
triple num: 99550
entity num: 28413
literal num: 0
subject num: 14569
predicate num: 17
```



#### 4.5.2 Visual tool（gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



#### 4.5.3 HTTP API（ghttp）

gStore provides ghttp component as http API service component. Users can realize relevant functions by sending http requests to ghttp. In ghttp, database statistics can be obtained through `monitor`.



<div STYLE="page-break-after: always;"></div>

### 4.6 Database query

---

​       Database query is one of the most important functions of gStore. gStore supports SPARQL 1.1 query language defined by W3C. Users can use the gStore database query function in the following ways.

#### 4.6.1 Command line mdoe（gquery)

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
[root@localhost gStore]$ bin/gquery -db lubm -f ./data/lubm/lubm_q0.sql
There has answer: 15
final result is :
?x
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
```

2.To learn more about how to use gQuery, enter the following command:

```shell
bin/gquery --help
```

3. Command to enter gquery console:

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



#### 4.6.2 Visual tool (gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



#### 4.6.3 HTTP API（ghttp）

gStore provides ghttp component as http API service component, users can realize related functions by sending http request to ghttp, ghttp through `query` request to query graph database, including query, delete, insert, See [Development document] - [common API] - [ghttp interface Description] for details.



#### 4.6.4 Socket API（gServer）

gStore provides gServer component as Socket API service component. Users can realize related functions by sending Socket request to gServer. gServer can query graph database through `query` request, including query, delete and insert. See [Development document] - [Common API] - [gServer interface Description] for details.

<div STYLE="page-break-after: always;"></div>

### 4.7 Database export

---

The export database function enables you to export a database as an. Nt file. There are three forms:

#### 4.7.1 Command line mode (gexport)

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
after Handle, used 0 ms.
QueryCache didn't cache
after tryCache, used 0 ms.
in getFinal Result the first half use 0  ms
after getFinalResult, used 0ms.
Total time used: 1ms.
finish exporting the database.
```




#### 4.7.2 Visual tool (gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



#### 4.7.3  HTTP API（ghttp）

gStore provides ghttp component as httpAPI service component. Users can realize relevant functions by sending http requests to ghttp. The function of `export` is adopted in ghttp.

<div STYLE="page-break-after: always;"></div>

### 4.8 Database deletion

---

Deleting a database You can delete a database in the following three ways

#### 4.8.1 Command line mode (gdrop)

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
after tryCache, used 0 ms.
QueryCache cleared
Total time used: 97ms.
update num : 3
lubm2.db is dropped successfully!
```

To delete the database, you should not just type `rm -r db_name. Db` because this will not update the built-in database named `system`. Instead, you should type `bin/ gdrop-db db_name`.



#### 4.8.2 Visual tool（gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



#### 4.8.3 HTTP API（ghttp）

gStore provides ghttp component as http API service component, users can send http request to ghttp to achieve relevant functions, ghttp through the `drop` command to achieve relevant functions, details see [development document] - [common API] - [ghttp interface description]  



#### 4.8.4 Socket API（gServer）

gStore provides gServer component as Socket API service component. Users can realize related functions by sending Socket request to gServer. gServer can delete graph database by `drop` request. See [Development document] - [Common API] - [gServer interface Description] for details.

<div STYLE="page-break-after: always;"></div>

### 4.9 Additional data

---

Inserting RDF data is a routine gStore operation, and you can do it in one of the following ways.

#### 4.9.1 Command line mode（gadd)--file

gadd is use to insert triples from a file into an existing database.

instruction：

```shell
bin/gadd -db db_name -f rdf_triple_file_path
```

parameter definition：

	db_name：database name
	rdf_triple_file_path：The file path with the suffix ".nt" or ".n3"

example：

```shell
[bookug@localhost gStore]$ bin/gadd -db lubm -f ./data/lubm/lubm.nt
...
argc: 3 DB_store:lubm   insert file:./data/lubm/lubm.nt
get important pre ID
...
insert rdf triples done.
inserted triples num: 99550
```

** Note: **  

**1. Gadd is primarily used for data insertion into RDF files **  

**2. Do not go directly to the 'bin' directory. Instead, perform the 'gadd' operation in the gStore installation root directory **

#### 4.9.2 Command line mode（gquery)---SPARQL statement

SPARQL can be defined by `insert data` instruction to insert data. Based on this principle, users can also write SPARQL insert statements and then use gStore's `gQuery` tool to insert data. Examples of SPARQL insert statements are as follows:

```sql
insert data {
 <San Zhang> <gender> "male"^^<http://www.w3.org/2001/XMLSchema#String>.
 <San Zhang> <age> "28"^^<http://www.w3.org/2001/XMLSchema#Int>.
 <San Zhang> <friend> <Si Li>.
}
```

Multiple RDF data can be contained with `{}`, taking care that each one ends with a `. `  

Since the database query function can be used to insert data, the following functions can also be used to insert data.



#### 4.9.3 Visual tool （gWorkbench)

gWorkbench is a visual management tool of gStore. Through gWorkbench, you can connect to gStore and create a graph database through database management module. For details, see [Development document] - [Visual Tool Workbench] - [query function] - [database management] function].



#### 4.9.4 HTTP API（ghttp）

gStore provides ghttp component as AN http API service component. Users can realize relevant functions by sending http requests to ghttp. ghttp inserts data through `query` request and batch inserts data through `batchInsert`. See [Development document] - [common API] - [ghttp interface Description] for details.



#### 4.9.5 Socket API（gServer）

gStore provides gServer component as Socket API service component. Users can implement related functions by sending Socket requests to gServer. gServer inserts data through 'query' requests. See [Development document] - [Common API] - [gServer interface Description] for details.

<div STYLE="page-break-after: always;"></div>

### 4.10 Data deletion

---

Deleting RDF data is a routine gStore operation, and you can perform this operation in the following ways.

#### 4.10.1 Command line mode（gsub）--file deletion

gsub is used to remove triples from files in an existing database.

instruction：

```
bin/gsub db_name rdf_triple_file_path
```

parameter definition：

	rdf_triple_file_path：The path to the data file to be deleted with the suffix ".nt" or ".n3"

example：

    [root@localhost gStore]$ bin/gsub lubm ./data/lubm/lubm.nt
    ...
    argc: 3 DB_store:lubm  remove file: ./data/lubm/lubm.nt
    ...
    remove rdf triples done.
    removed triples num: 99550



#### 4.10.2 Command line mode（gquery)---SPARQL statement

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



#### 4.10.3 Visual tool（gWorkbench)

gWorkbench is a visual management tool for gStore. Through gWorkbench, you can connect to gStore and delete data by writing SPARQL statements through database query module. See [Development document] - [Visualization Tool Workbench] - [query function] - [Graph database query] for details.  



#### 4.10.4 HTTP API（ghttp）

gStore provides ghttp component as http API service component, users can send http request toghttp to achieve relevant functions, ghttp through `query` request to delete data and `batchRemove` to delete data in batches. See [Development document] - [common API] - [ghttp interface Description] for details



#### 4.10.5 Socket API（gServer）

gStore provides gServer component as Socket API service component. Users can realize related functions by sending Socket requests to gServer. gServer can delete data through `query` requests. See [Development document] - [Common API] - [gServer interface Description] for details.

<div STYLE="page-break-after: always;"></div>

### 4.11 Console Service

---

gStore provides a console service with contextual information via the gconsole module, which users can use to perform all types of database management operations.

#### 4.11.1 Start gconsole

##### Login

```bash
bin/gconsole [-u <usr_name>]
```

A prompt for input will appear if `usr_name` is missing.

```bash
# bin/gconsole
Enter user name: root
Enter password:

Gstore Console(gconsole), an interactive shell based utility to communicate with gStore repositories.
Gstore version: 1.0.0 Source distribution
Copyright (c) 2016, 2022, pkumod and/or its affiliates.

Welcome to the gStore Console.
Commands end with ;. Cross line input is allowed.
Comment start with #. Redirect (> and >>) is supported.
Type 'help;' for help. Type 'cancel;' to quit the current input statement.

gstore>
```

##### Help

```bash
bin/gconsole --help
```

```bash
# bin/gconsole --help
Gstore Ver 1.0.0 for Linux on x86_64 (Source distribution)
Gstore Console(gconsole), an interactive shell based utility to communicate with gStore repositories.
Copyright (c) 2016, 2022, pkumod and/or its affiliates.

Usage: bin/gconsole [OPTIONS]
-?, --help Display this help and exit.
-u, --user username.

Supported command in gconsole: Type "?" or "help" in the console to see info of all commands.

For bug reports and suggestions, see https://github.com/pkumod/gStore
```

##### Commands

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

#### 4.11.2 Database management

##### Build database

```bash
create <database_name> [<nt_file_path>]
```

- Read `.nt` file from nt_file_path and create a database named database_name, or create an empty database
- <database_name>: cannot be `system`
- <nt_file_path>：file path

```
gstore> create eg my_test_data/eg_rdf.nt;
... (this is build database process output, omitted in this document)
Database eg created successfully
```

##### Delete database

```
drop <database_name>
```

-  Cannot delete the current database
-  <database_name>：can only be specified as the name of a database for which the current user has full permissions

```
gstore> drop <database_name>;
```

##### Load/Switch database

```
use <database_name>
```

- Load/switch the current database: unload the previous current database from memory (if any) and load the specified database
- The target database needs to have been created
- <database_name>：can only be specified as the name of a database for which the current user have load and unload permissions

```
gstore> use mytest;
... (this is load process output, omitted in this document)
Current database switch to mytest successfully.
```

##### Query/update database

- Run a SPARQL query on the current database
- You need to specify the current database first by`use <database_name>`

###### Inputting a SPARQL query

```bash
gstore> # show all in db
	-> SELECT ?x ?y ?z
	-> WHERE{
	-> ?x ?y ?z. # comment
	-> } ;
... (this is query process output, omitted in this document)
final result is :
?x ?y ?z
<root> <has_password> "123456"
<system> <built_by> <root>
<CoreVersion> <value> "1.0.0"
query database successfully, Used 15 ms
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

###### sparql <sparql_file>

- Specify the sparql file: the file contains multiple sparql statements that need to be separated by `;` and the last `;` is optional
- The file content supports single-line comments starting with #

```bash
gstore> sparql query.sparql ;
... (this is query process output, omitted in this document)
final result is :
?x ?y ?z
<root> <has_password> "123456"
<system> <built_by> <root>
<CoreVersion> <value> "1.0.0"
query database successfully, Used 15 ms
```

- An example of query.sparql content:

```sql
# comment
SELECT ?t1_time
WHERE
{
	# comment
	<t1><built_time>?t1_time. # comment
};
# comment
SELECT ?t2_time
WHERE
{
	# comment
	<t2><built_time>?t2_time. # comment
};
```

##### Display database information

```
show [<database_name>] [-n <displayed_triple_num>]
```

- Display meta information and the first displayed_triple_num triples (the first 10 triples are displayed by default)
- <database_name>: the database name. If database_name is not specified, the information of the current database will be displayed.
- -n <displayed_triple_num>: number of lines displayed

```
gstore> show eg;
... (this is load and query process output, omitted in this document)
===================================================
Name: eg
TripleNum: 15
EntityNum: 6
LiteralNum: 0
SubNum: 3
PreNum: 3
===================================================
<Alice> <follow> <Bob>
<Bob> <follow> <Alice>
<Carol> <follow> <Bob>
<Dave> <follow> <Alice>
<Dave> <follow> <Eve>
<Alice> <like> <Bob>
<Bob> <like> <Eve>
<Eve> <like> <Carol>
<Carol> <like> <Bob>
<Francis> <like> <Carol>
```

##### View all databases

```
showdbs
```

- Only the databases for which the current user has query permissions can be displayed.

```bash
gstore> showdbs;
"database" "creater" status"
<system> <root>
<eg> <yuanzhiqiu> "already_built"
```

##### Display current database name

```
pdb
```

```
gstore> pdb;
eg
```

##### Backup current database

```
backup [<backup_folder>]
```

- <backup_folder> specifies the backup path.

```
gstore> backup;
... (this is backup process output, omitted in this document)
Database eg backup successfully.
```

```
gstore> backup back;
... (this is backup process output, omitted in this document)
Database eg backup successfully.
```

##### Export current database

```
export <file_path>
```

- <file_path> specifies the export path.

```
gstore> export eg.nt;
Database eg export successfully.
```

##### Restore database

```
restore <database_name> <backup_path>
```

- <database_name>: database name.
- <backup_path>: backup file path.

```
gstore> restore eg backups/eg.db_220929114732/
... (this is restore process output, omitted in this document)
Database eg restore successfully.
```

#### 4.11.3 User Identity

##### Refresh permissions

```bash
flushpriv
```

Read db and refresh current user permissions

```bash
gstore> flushpriv;
Privilige Flushed for current user successfully.
```

##### Display current username and permissions

```bash
pusr [<database_name>]
```

- `pusr` displays the current username
- `pusr <database_name>` displays the current user name and the current user's permissions on <database_name>

```bash
gstore> pusr;
usrname: yuanzhiqiu
gstore> pusr eg;
usrname: yuanzhiqiu
privilege on eg: query load unload update backup restore export
```

##### Modify the current user's password

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

#### 4.11.4 Configuration information viewing

##### View congifuration

```
settings [<conf_name>]
```

- Show all configuration information.

```
gstore> settings;
Settings:
thread_num 30
... (this is other settings, omitted in this document)
You can Edit configuration file to change settings: conf.ini
```

- <conf_name> displays the configuration information of the specified conf_name.

```bash
gstore> settings ip_deny_path;
"ipDeny.config"
You can Edit configuration file to change settings: conf.ini
```

##### View version

```bash
version
```

- Output current version information.

```bash
gstore> version;
Gstore version: 1.0 Source distribution
Copyright (c) 2016, 2022, pkumod and/or its affiliates.
```

#### 4.11.5 Permissions management

Only system users have the permission to execute commands related to permissions management. The permissions descriptions are as follows:

- Permissions: the operations a user is permitted to carry out on a database

- 7 types of permissions: `query`, `load`, `unload`, `update`, `backup`, `restore`, `export`

    ```
    [1]query [2]load [3]unload [4]update [5]backup [6]restore [7]export
    ```

     Users who have all seven permissions are considered to have full permissions.

- System users have full permissions on all databases. (System users are defined in conf.ini and system.db.)

##### Set user's permissions

```
setpriv <usrname> <database_name>
```

- A password is required to verify root identity.
- <usrname> : the user name to be set.
- <database_name>: database name.

```bash
gstore> setpriv zero eg;
Enter your password:
[1]query [2]load [3]unload [4]update [5]backup [6]restore [7]export [8]all
Enter privilege number to assign separated by whitespace:
1 2 3
[will set priv:]00001110
Privilege set successfully.
```

##### View user's permissions

```bash
pusr <database_name> <usrname>
```

- <usrname> : the user name.
- <database_name>: database name.

```bash
gstore> pusr eg yuanzhiqiu;
privilege on eg: query load unload update backup restore export
```

##### View all users and their permissions

```bash
showusrs
```

Show which users there are and the database permissions for each user (non-root users only; only databases with has_xxx_priv for this user are displayed.)

Output format:

```bash
usrname
------------------------
privilege on databases
```

```bash
gstore> showusrs;
root
----
all privilege on all db
yuanzhiqiu
----------
eg: query load unload update backup restore export
mytest1: query load unload
zero
----
mytest2: query load unload
```

#### 4.11.6 User management

Only system users have the permission to execute user management commands.

##### Add users

```bash
addusr <usrname>
```

- A password is required to verify root identity.
- <usrname>: the new user name.

```bash
gstore> addusr uki;
Enter your password:
Enter password for new user: hello
Add usr uki successfully.
```

##### Delete users

```bash
delusr <usrname>
```

- A password is required to verify root identity.
- <usrname>: the user name to be deleted.

```bash
gstore> delusr cat;
Enter your password:
Del usr cat successfully.
```

##### Reset passwords

```bash
setpswd <usrname>
```

- A password is required to verify root identity.
- <usrname>: user name.

```bash
gstore> setpswd zero;
Enter your password:
Enter new password:
Enter new password again:
Password set successfully.
```

##### View all users

```bash
showusrs
```

```bash
gstore> showusrs;
root
----
all privilege on all db
lubm
----
```

#### 4.11.7 Help and miscellaneous

##### End/Cancel the current command

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

##### Quit

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

##### Help

```bash
help [edit/usage/<command>]
```

- `help` displays the help information about all commands:

```bash
gstore> help;
Gstore Console(gconsole), an interactive shell based utility to communicate with
gStore repositories.
For information about gStore products and services, visit:
	http://www.gstore.cn/
For developer information, including the gStore Reference Manual, visit:
	http://www.gstore.cn/pcsite/index.html#/documentation

Commands end with ;. Cross line input is allowed.
Comment start with #.
Type 'cancel;' to quit the current input statement.

List of all gconsole commands:
sparql 			Answer SPARQL query(s) in file.
create 			Build a database from a dataset or create
...(this is help msg for other commands, omitted in this document)

Other help arg:
edit Display line editing shortcut keys supported by gconsole.
usage Display all commands as well as their usage.
```

`help edit` displays the line editing shortcut operations of `GNU readline`:

```bash
gstore> help edit;
Frequently used GNU Readline shortcuts:
CTRL-a move cursor to the beginning of line
CTRL-e move cursor to the end of line
CTRL-d delete a character
CTRL-f move cursor forward (right arrow)
CTRL-b move cursor backward (left arrow)
CTRL-p previous line, previous command in history (up arrow)
CTRL-n next line, next command in history (down arrow)
CTRL-k kill the line after the cursor, add to clipboard
CTRL-u kill the line before the cursor, add to clipboard
CTRL-y paste from the clipboard
ALT-b move cursor back one word
ALT-f move cursor forward one word
For more about GNU Readline shortcuts, see
https://en.wikipedia.org/wiki/GNU_Readline#Emacs_keyboard_shortcuts
```

`help usage` displays the description and usage of all commands:

```bash
gstore> help usage;
List of all gconsole commands:
Note that all text commands must be end with ';' but need not be in one line.
sparql 			Answer SPARQL query(s) in file.
				sparql <; separated SPARQL file>;
create 			Build a database from a dataset or create an empty database.
				create <database_name> [<nt_file_path>];
...(this is help msg for other commands, omitted in this document)
```

`help <command>` displays the help information about the command:

```bash
gstore> help use;
use 	Set current database.
		use <database_name>;
```

##### Clear screen

```bash
gstore> clear;
```

##### View current working path

```bash
gstore> pwd;
/<path_to_gstore>/gstore
```



### 4.12 HTTP API service

---

GStore provides two sets of http API services, namely grpc and ghttp. Users can remotely connect to and operate on gStore by sending http requests.

#### 4.12.1 Starting ghttp/gRPC service

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

#### 4.12.2 Shutting down the service

For safety, please use the following command to shut down the service instead of `Ctrl + C` or `kill`.

```bash
bin/shutdown
```

#### 4.12.3 HTTP API

ghttp and gRPC provide a rich API interface so that users can remotely operate most of the functions of gStore. For details, please see [Development document] - [common API] - [ghttp interface Description]  or [gRPC interface Description].

<div STYLE="page-break-after: always;"></div>

### 4.13 Socket API service

---

gServer is an external access interface provided by gStore and a Socket API service. Users can connect and operate gStore remotely through two-way socket communication.

#### 4.13.1 Start gServer service

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



#### 4.13.2 Shutdown gServer service

To stop the gServer service, use the following command to stop it. It is better not to simply enter the command 'Ctrl + C' or 'kill' to stop the gServer, because it is not safe.  

```shell
bin/gserver -t
```



#### 4.13.3 gServer related API

gServer provides rich API interfaces so that users can remotely operate most functions of gStore. See [Development document] - [common API] - [gServer interface Description] for specific interfaces.



<div STYLE="page-break-after: always;"></div>

## 5. API Usage

​	

### 5.1 Introduction to API

---

gStore provides API services to users through http and Socket services, and its components are gRPC, ghttp and gServer.

#### 5.1.1 HTTP API

We now provide C++, Java, Python, PHP, and Node.js APIs for gRPC and ghttp. Please refer to the examples in `api/http/cpp`, `api/http/java`, `api/http/python`, `api/http/php` and `api/http/nodejs`. To use these examples, make sure you have generated the executable files of gRPC or ghttp. **Next, please use the `bin/grpc` or `bin/ghttp` command to start the service.** If you know of a running gRPC or ghttp server that's available for connection, you can also directly connect to it. Then, for the C++ and Java code, you need to compile the sample code in the directory `api/http/cpp/example` or`api/http/java/example`.

**For details on how to start and shut down gRPC or ghttp, please see [development documentation] - [Quick Start] - [HTTP API service].**

**After the API service is started, the gRPC access address is as follows:**

```
http://serverip:port/grpc/
```

**The ghttp access address is as follows:**

```
http://serverip:port/
```

`ServerIP` is the IP address of the gStore server, and `port` is the port on which gRPC or ghttp is started.

#### 5.1.2 Socket API

We now provide C++ (Java, Python, PHP, and Node.js will be supported in future versions) API for gServer, the socket API service. Please see api/socket/cpp for the example code. To use these examples, make sure you have generated an executable file for gServer. **Next, use the `bin/gserver -s` command to start the gserver service.** If you know of a running gServer that's available for connection, you can also directly connect to it. Then, for the C++ code, you need to compile the sample code in the directory `api/socket/cpp/example`. 

**For details on how to start and shut down gServer, please see [development documentation] - [Quick Start] - [Socket API service].**

**After the Socket API is started, you can connect through the Socket. The default port of gServer is 9000.**

<div STYLE="page-break-after: always;"></div>

### 5.2 HTTP API framework

---

The gStore HTTP API is placed in the API/HTTP directory of the gStore root directory and contains the following: 

- gStore/api/http/
    - cpp/ (the C++ API)
        - example/ (Example program using the C++ API)
            - Benchmark.cpp
            - GET-example.cpp
            - POST-example.cpp
            - Transaction-example.cpp
            - Makefile (for compiling the sample code)
        - src/
            - GstoreConnector.cpp (C++ API's source code)
            - GstoreConnector.h
            - Makefile (for compiling the lib)
    - java/ (the Java API)
        - example/ (Example program using the Java API)
            - Benckmark.java
            - GETexample.java
            - POSTexample.java
            - Makefile
        - src/
            - jgsc/
                - GstoreConnector.java (Java API's source code)
            - Makefile
    - python/ (the Python API)
        - example/ (Example program using the Python API)
            - Benchmark.py
            - GET-example.py
            - POST-example.py
        - src/
            - GstoreConnector.py
    - nodejs/ (the Node.js API)
        - example/ (Example program using the Node.js API)
            - POST-example.js
            - GET-example.js
        - src
            - GstoreConnector.js (Node.js API's source code)
            - LICENSE
            - package.json
    - php/ (the PHP API)
        - example/ (Example program using the PHP API)
            - Benchmark.php
            - POST-example.php
            - GET-example.php
        - src/
            - GstoreConnector.php (the PHP API's source code)  

- - -

<div STYLE="page-break-after: always;"></div>

### 5.3 ghttp API instruction

---

#### 5.3.1API Interconnection Mode

> The ghttp interface adopts the `HTTP` protocol and supports multiple ways to access the interface. If the ghttp is started on the port `9000`, the interface interconnection content is as follows
>
> API address：
>
> ```json
> http://ip:9000/
> ```
>
> The interface supports both `GET` and `POST` requests, where `GET` requests place parameters in the URL and `POST` requests place parameters in the `body` request.
>
> 

> **Note: `GET` request parameters contain special characters, such as? , @,& and other characters, you need to use urlencode encoding, especially the `SPARQL` parameter must be encoded**
>
> 

#### 5.3.2 API list

| API name                   | Definition                                | Note                                                         |
| -------------------------- | ----------------------------------------- | ------------------------------------------------------------ |
| build                      | build graph database                      | The database file must be locally stored on the server       |
| load                       | load graph database                       | Load the database into memory                                |
| unload                     | unload graph database                     | Unload the database from memory                              |
| monitor                    | monitor graph database                    | Count information about the specified database (such as the number of triples, etc.) |
| drop                       | drop graph database                       | Logical deletion and physical deletion can be performed      |
| show                       | display graph database                    | Display a list of all databases                              |
| usermanage（added）        | user management                           | Add, delete, or modify user information                      |
| showuser                   | display all user list                     | Display a list of all users                                  |
| userprivilegemanage(added) | user privilege management                 | Add, delete, or modify user's privilege information          |
| backup                     | backup database                           | backup database information                                  |
| restore                    | restore database                          | restore database information                                 |
| query                      | query database                            | Including query, delete, and insert                          |
| export                     | export database                           | Export database as NT file                                   |
| login                      | login to database                         | It is used to authenticate user names and passwords          |
| check（rewrite）           | Detect ghttp heartbeat signal             |                                                              |
| init（abandon）            | Initialize system                         | This operation should not be initialized by ghttp            |
| refresh（abandon）         | reload database                           | This can be done by unload+ Load                             |
| parameter（abandon）       | Set parameters for backup                 | All parameters are currently boiled down to the config.ini file and cannot be changed remotely |
| begin                      | Start transaction                         | Transaction starts and needs to be used in conjunction with TQuery |
| tquery                     | Querying the database (with transactions) | Data queries with transaction mode (insert and DELETE only)  |
| commit                     | commit transactions                       | Commit the transaction after it completes                    |
| rollback                   | rollback transaction                      | Roll back the transaction to begin state                     |
| txnlog                     | Obtain transaction log information        | Return transcation log information as json                   |
| checkpoint                 | write data to a disk                      | After an INSERT or delete operation is performed on the database, manually checkpoint is required |
| testConnect                | testing connectivity                      | Used to check whether GHTTP is connected                     |
| getCoreVersion             | get gStore version                        | Get the gStore version number                                |
| batchInsert                | batch insert data                         | Batch insert NT data                                         |
| batchRemove                | batch delete data                         | Batch delete NT data                                         |
| querylog（added）          | Get query log information                 | Query log information is returned as json                    |
| ipmanage（added）          | black/white list management               | Maintains a blacklist and whitelist of IP addresses that access gStore |

<div STYLE="page-break-after: always;"></div>

#### 5.3.3 API specific intruction

> The input and output parameters of each interface are specified in this section. Assume that the IP address of the GHTTP server is 127.0.0.1 and the port is 9000

##### 5.3.3.1 build -build datbase

###### Brief description

- Create a database based on existing NT files
- Files must exist on the gStore server

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON `structure

###### Parameter

| parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **build**                     |
| db_name        | yes       | string | Database name (.db is not required)                          |
| db_path        | yes       | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |

###### Return value

| Parameter name | Tpye   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done."
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.2  check

###### Brief description

- Check whether the GHTTP service is online

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- ###### GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is **check** |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "the ghttp server is running..."
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.3 load (Updated in this version)

###### Brief description  

- Loading a database into memory is a prerequisite for many operations, such as query and monitor.
- Updates in this version: Added the load_csr parameter, default is false

###### Request URL

- `http://127.0.0.1:9000/`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request: the parameters are passed directly as the URL
- POST request: `raw` in `body` in `httprequest` , passed as `JSON`  

###### Parameter  

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `load`                        |
| username       | yes       | string | User name                                                    |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name (.db is not required)                          |
| csr            | no        | string | Whether to load CSR resources, default is 0 (set to 1 when using advanced query functions) |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| csr            | string | Whether to load CSR resources                                |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database loaded successfully.",
	"csr": "1"
}
```

##### 5.3.3.4 monitor (Updated in this version)

###### Brief description

- Get database statistics (database needs to be loaded)
- Updates in this version: parameter names in the return values are modified (the parameters with spaces and underscores are changed to camel case form, e.g. triple num -> tripleNum)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON `

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `monitor`                     |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |

###### Return value 

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| database       | string | database name                                                |
| creator        | string | creator                                                      |
| builtTime      | string | create time                                                  |
| tripleNum      | string | number of triples                                            |
| entityNum      | int    | number of entities                                           |
| literalNum     | int    | number of characters (attribute value)                       |
| subjectNum     | int    | number of subjects                                           |
| predicateNum   | int    | number of objects                                            |
| connectionNum  | int    | number of connections                                        |

###### Return sample 

``` json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"database": "test_lubm",
	"creator": "root",
	"builtTime": "2021-08-27 21:29:46",
	"tripleNum": "99550",
	"entityNum": 28413,
	"literalNum": 0,
	"subjectNum": 14569,
	"predicateNum": 17,
	"connectionNum": 0
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.5 unload

###### Brief description

- Unload the database from memory (all changes are flushed back to hard disk)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**unload** |
| db_name        | yes       | string | Database name (.db is not required)      |
| username       | yes       | string | user name                                |
| password       | yes       | string | Password (plain text)                    |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database unloaded."
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.6 drop

###### Brief description

- Delete the database (either logically or physically)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**drop**                       |
| db_name        | yes       | string | Database name (.db is not required)                          |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| is_backup      | no        | string | True: Logical deletion, false: represents physical deletion (default true)，if it's logical deletion, change the file folder to .bak file foder, user can change the folder name to. Db and add the folder to the system database by calling bin/ ginit-db database name |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Database test_lubm dropped."
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.7 show

###### Brief description

- Display all database list

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**show** |
| username       | yes       | string | user name                              |
| password       | yes       | string | Password (plain text)                  |

###### Return value

| Parameter name      | Type      | Note                                                         |
| :------------------ | :-------- | ------------------------------------------------------------ |
| StatusCode          | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg           | string    | Return specific information                                  |
| ResponseBody        | JSONArray | JSON arrays (each of which is a database information)        |
| -------- database   | string    | database name                                                |
| ---------creator    | string    | creator                                                      |
| ---------built_time | string    | create time                                                  |
| ---------status     | string    | database status                                              |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Get the database list successfully!",
    "ResponseBody": [
        {
            "database": "lubm",
            "creator": "root",
            "built_time": "2021-08-22 11:08:57",
            "status": "loaded"
        },
        {
            "database": "movie",
            "creator": "root",
            "built_time": "2021-08-27 20:56:56",
            "status": "unloaded"
        }
    ]
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.8 usermanage

###### Brief description

- Manage users (including adding, deleting, and changing users)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**usermanage**                 |
| type           | yes       | string | Operation Type（1：adduser ，2：deleteUser 3：alterUserPassword） |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| op_username    | yes       | string | User name for the operation                                  |
| op_password    | yes       | string | Password of the operation (if the password is to be changed, the password is the password to be changed) (If the operation contains special characters and the get request is adopted, the value must be urlencode-encoded) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 1004,    "StatusMsg": "username already existed, add user failed."}
```

###### Notes

The default interface permissions for the new user are: `login`, `check`, `testConnect`, `getCoreVersion`, `show`, `funquery`, `funcudb`, `funreview`, `userpassword`.

Users with `query` rights also have the following interface rights: `query`, `monitor`.

Users with the `update` permission also have the following interface permissions: `batchInsert`, `batchRemove`, `begin`, `tquery`, `commit`, `rollback`.

Only the root user can invoke the interface rights that are not within the scope of authorization management, for example: `build`, `drop`, `usermanage`, `showuser`, `userprivilege`, `manage`, `txnlog`, `checkpoint`, `shutdown`, `querylog`, `accesslog`, `ipmanage`.

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.9 showuser

###### Brief description

- Display all user information

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                       |
| :------------- | :-------- | :----- | ------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**showuser** |
| username       | yes       | string | user name                                  |
| password       | yes       | string | Password (plain text)                      |

###### Return value

| Parameter name        | Type      | Note                                                         |
| :-------------------- | :-------- | ------------------------------------------------------------ |
| StatusCode            | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg             | string    | Return specific information                                  |
| ResponseBody          | JsonArray | JSON object array                                            |
| ------username        | string    | user name                                                    |
| ------password        | string    | password                                                     |
| -----query_privilege  | string    | Query permissions (database names separated by commas)       |
| ----update_privilege  | string    | Update permissions (database names separated by commas)      |
| ----load_privilege    | string    | Load permissions (database names separated by commas)        |
| ---unload_privilege   | string    | Unload permissions (database names separated by commas)      |
| ----backup_privilege  | string    | Back up permissions (database names separated by commas)     |
| ----restore_privilege | string    | Restore permissions (database names separated by commas)     |
| ---export_privilege   | string    | Export permissions (database names separated by commas)      |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "success",    "ResponseBody": [        {            "username": "liwenjie",            "password": "shuaige1982",            "query privilege": "",            "update privilege": "",            "load privilege": "",            "unload privilege": "",            "backup privilege": "",            "restore privilege": "",            "export privilege": ""        },        {            "username": "liwenjie2",            "password": "shuaige19888@&",            "query privilege": "lubm,movie,",            "update privilege": "lubm,movie,",            "load privilege": "lubm,movie,",            "unload privilege": "lubm,movie,",            "backup privilege": "lubm,movie,",            "restore privilege": "",            "export privilege": ""        },        {            "username": "root",            "password": "123456",            "query privilege": "all",            "update privilege": "all",            "load privilege": "all",            "unload privilege": "all",            "backup privilege": "all",            "restore privilege": "all",            "export privilege": "all"        }    ]}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.10 userprivilegemanage

###### Brief description

- Manage user permissions (including adding, deleting, and changing users)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**userprivilegemanage**        |
| type           | yes       | string | Operation type（1：add privilege，2：delete privilege 3：clear Privilege ） |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| op_username    | yes       | string | User name for the operation                                  |
| privileges     | no        | string | Permissions to operate on (multiple permissions separated by commas) (can be null for clear Privilege)1:query,2:load,3:unload,4:update,5:backup,6:restore,7:export, you can set multi privileges by using , to split. |
| db_name        | no        | string | The database to operate on (this can be empty if it is clearPrivilege |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "add privilege query successfully. \r\nadd privilege load successfully. \r\nadd privilege unload successfully. \r\nadd privilege update successfully. \r\nadd privilege backup successfully. \r\n"}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.11 userpassword (New in this version)

###### Brief description

- Modify the user password.

###### Request URL

- `http://127.0.0.1:9000/`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`  

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `userpassword`                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password                                                     |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| op_password    | yes       | string | New password (plain text)                                    |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 1004,
	"StatusMsg": "change password done."
}
```

 

##### 5.3.3.12 backup

###### Brief description

- Back up database

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**backup**                     |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | database that need operaions                                 |
| backup_path    | no        | string | The backup file path can be relative or absolute. The relative path uses the gStore root directory as reference. The default path is the backup directory in the gStore root directory |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| backupfilepath | string | Backup file path (this value can be used as the input parameter value for restore) |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Database backup successfully.",    "backupfilepath": "testbackup/lubm.db_210828211529"}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.13 restore

###### Brief description

- Restore database

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**restore**                    |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | database that need operations                                |
| backup_path    | no        | string | The full time-stamped path of the backup file (can be a relative path or an absolute path. The relative path is based on the gStore root directory). The default path is the backup directory in the gStore root directory |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Database restore successfully."}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.14 query

###### Brief description

- query the database

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**query**                      |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | database that need operations                                |
| format         | no        | string | The result set returns in json, HTML, and file format. The default is JSOn |
| sparql         | yes       | string | Sparql statement to execute (SPARQL requires URL encoding if it is a GET request) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| head           | JSON   | head information                                             |
| results        | JSON   | Result information (see Return Sample for details)           |


###### Return sample 

``` json
{    "head": {        "link": [],        "vars": [            "x"        ]    },    "results": {        "bindings": [            {                "x": {                    "type": "uri",                    "value": "十面埋伏"                }            },            {                "x": {                    "type": "uri",                    "value": "投名状"                }            },            {                "x": {                    "type": "uri",                    "value": "如花"                }            }        ]    },    "StatusCode": 0,    "StatusMsg": "success"}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.15 export

###### Brief description

- export database

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                      |
| :------------- | :-------- | :----- | ----------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**restore** |
| username       | yes       | string | user name                                 |
| password       | yes       | string | Password (plain text)                     |
| db_name        | yes       | string | database that need operations             |
| db_path        | no        | string | Export path (gstore root by default)      |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| filepath       | string | Path for exporting files                                     |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Export the database successfully.",    "filepath": "export/lubm_210828214603.nt"}
```



<div STYLE="page-break-after: always;"></div>

##### 5.3.3.16 login (Updated in this version)

###### Brief description

- User login (verify username and password) 
- Updates in this version: the full path information of RootPath will be returned after successful login

###### Request URL

- `http://127.0.0.1:9000/`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `login`                       |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Kernel version                                               |
| licensetype    | string | Certificate type (Open Source or Enterprise)                 |
| RootPath       | string | Full path to the gStore root directory                       |
| type           | string | HTTP service type                                            |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "login successfully",
	"CoreVersion": "1.0.0",
	"licensetype": "opensource",
	"Rootpath": "/data/gstore",
	"type": "ghttp"
}
```

##### 5.3.3.17 begin

###### Brief description

- start transaction

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**begin**                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name                                                |
| isolevel       | yes       | string | Transaction isolation level 1:RC(read committed)  2:SI(snapshot isolation) 3:SR(seriablizable） |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| TID            | string | Transaction ID(this ID is important enough to take as a parameter |


###### Return sample 

``` json
{    "StatusCode": 1001,    "StatusMsg": "wrong password."}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.18 tquery

###### Brief description

- query the transaction type

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**tquery** |
| username       | yes       | string | User name                                |
| password       | yes       | string | Password (plain text)                    |
| db_name        | yes       | string | Database name                            |
| tid            | yes       | string | Transaction ID                           |
| sparql         | yes       | string | sparql statement                         |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "result": "",    "StatusCode": 0,    "StatusMsg": "success"}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.19 commit 

###### Brief description

- submit transaction

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**commit** |
| username       | yes       | string | User name                                |
| password       | yes       | string | Password (plain text)                    |
| db_name        | yes       | string | Database name                            |
| tid            | yes       | string | Transaction ID                           |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "transaction commit success. TID: 1"}
```

 <div STYLE="page-break-after: always;"></div>

##### 5.3.3.20 rollback 

###### Brief description

- Rollback trasnsaction

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                       |
| :------------- | :-------- | :----- | ------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**rollback** |
| username       | yes       | string | User name                                  |
| password       | yes       | string | Password (plain text)                      |
| db_name        | yes       | string | Database name                              |
| tid            | yes       | string | Transaction ID                             |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "transaction rollback success. TID: 2"}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.21 txnlog (Updated in this version)

###### Brief description

- Get transaction logs (this function is only available for the root user)  
- Updates in this version: added paging query parameters

###### Request URL

- `http://127.0.0.1:9000/`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `txnlog`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| pageNo         | yes       | int    | Page number, the value ranges from 1 to N, the default value is 1 |
| pageSize       | yes       | int    | The number of entries per page, the value ranges from 1 to N, the default value is 10 |

###### Return value

| Parameter name  | Type      | Note                                                         |
| --------------- | --------- | ------------------------------------------------------------ |
| StatusCode      | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg       | string    | Return specific information                                  |
| totalSize       | int       | Total size                                                   |
| totalPage       | int       | The total number of pages                                    |
| pageNo          | int       | The current page number                                      |
| pageSize        | int       | The number of entries per page                               |
| list            | JSONArray | Log array                                                    |
| ---- db_name    | string    | Database name                                                |
| ---- TID        | string    | Transaction ID                                               |
| ---- user       | string    | User                                                         |
| ---- state      | string    | State (COMMITED/RUNNING/ROLLBACK)                            |
| ---- begin_time | string    | Start time                                                   |
| ---- end_time   | string    | End time                                                     |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Get Transaction log success",
	"totalSize": 2,
	"totalPage": 1,
	"pageNo": 1,
	"pageSize": 10,
	"list": [
		{
			"db_name": "lubm2",
			"TID": "1",
			"user": "root",
			"begin_time": "1630376221590",
			"state": "COMMITED",
			"end_time": "1630376277349"
		},
		{
			"db_name": "lubm2",
			"TID": "2",
			"user": "root",
			"begin_time": "1630376355226",
			"state": "ROLLBACK",
			"end_time": "1630376379508"
		}
	]
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.22 checkpoint

###### Brief description

- Received Flush data back to hard disk (to make data final)

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                         |
| :------------- | :-------- | :----- | -------------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is**checkpoint** |
| username       | yes       | string | User name                                    |
| password       | yes       | string | Password (plain text)                        |
| db_name        | yes       | string | Database name                                |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Database saved successfully."}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.23 testConnect (Updated in this version)

###### Brief description

- Test whether the server can connect (for Workbench)
- Updates in this version: added HTTP service type in the return value

###### Request URL

- `http://127.0.0.1:9000/`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `testConnect`                 |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Kernel version                                               |
| licensetype    | string | Certificate type (Open Source or Enterprise)                 |
| type           | string | HTTP service type (fixed as ghttp)                           |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"CoreVersion": "1.0.0",
	"licensetype": "opensource",
	"type": "ghttp"
}
```

##### 5.3.3.24 getCoreVersion (Updated in this version)

###### Brief description

- Get the server version number (for Workbench)
- Updates in this version: added HTTP service type in the return value

###### Request URL

- `http://127.0.0.1:9000/`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `getCoreVersion`              |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Kernel version                                               |
| type           | string | HTTP service type (fixed as ghttp)                           |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"CoreVersion": "1.0.0",
	"type": "ghttp"
}
```

##### 5.3.3.25 batchInsert

###### Brief description

- batch insert data

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**batchInsert**                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name                                                |
| file           | yes       | string | The data NT file to insert (can be a relative or absolute path) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | string | Number of successful executions                              |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Batch Insert Data  Successfully.",    "success_num": "25"}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.26 batchRemove

###### Brief description

- batch remove data

###### Request URL

- ` http://127.0.0.1:9000/ `


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**batchRemove**                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name                                                |
| file           | yes       | string | Data NT files to be deleted (can be relative or absolute paths) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | int    | Number of successful executions                              |


###### Return sample 

``` json
{    "StatusCode": 0,    "StatusMsg": "Batch Remove Data  Successfully.",    "success_num": "25"}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.27 shutdown

###### Brief description

- close ghttp

###### Request URL

- ` http://127.0.0.1:9000/shutdown `[Note, address change]  


###### Request mode

- GET/POST 

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| username       | yes       | string | user name（default user name is system)                      |
| password       | yes       | string | Password（This password need to be viewed in the server's system.db/password[port].txt file，for example, if the port is 9000，then check password in password9000.txt file) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
If no value is returned, no information is received by default on success, and an error JSON message is returned on failure
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.28 querylog

###### Brief description

- Obtaining query Logs

###### Request URL

`http://127.0.0.1:9000`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as`JSON ` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**querylog**                   |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| date           | yes       | string | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int    | Page number. The value ranges from 1 to N. The default value is 1 |
| pageSize       | yes       | int    | The number of entries per page. The value ranges from 1 to N. The default value is 10 |

###### Return value

| Parameter     | Type   | Note                                                         |
| :------------ | :----- | ------------------------------------------------------------ |
| StatusCode    | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg     | string | Return specific information                                  |
| totalSize     | int    | Total number                                                 |
| totalPage     | int    | Total page number                                            |
| pageNo        | int    | Current page number                                          |
| pageSize      | int    | The number of entries per page                               |
| list          | Array  | Log array                                                    |
| QueryDateTime | string | Query date/time                                              |
| Sparql        | string | SPARQL statement                                             |
| Format        | string | Query return format                                          |
| RemoteIP      | string | Request IP                                                   |
| FileName      | string | Query result set files                                       |
| QueryTime     | int    | Time (ms)                                                    |
| AnsNum        | int    | Result number                                                |

###### Return sample

```json
{	"StatusCode":0,    "StatusMsg":"Get query log success",	"totalSize":64,	"totalPage":13,	"pageNo":2,	"pageSize":5,	"list":[		{			"QueryDateTime":"2021-11-16 14:55:52:90ms:467microseconds",			"Sparql":"select ?name where { ?name <dislike> <Eve>. }",			"Format":"json",			"RemoteIP":"183.67.4.126",			"FileName":"140163774674688_20211116145552_847890509.txt",			"QueryTime":0,			"AnsNum":2		}        ......    ]}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.29 querylogdate

###### Brief description

- Get the date of gStore's query log (for the date parameter of the query log interface)

###### Request URL

`http://127.0.0.1:9000`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON ` 

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `querylogdate`                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter  | Type   | Note                                                         |
| :--------- | :----- | ------------------------------------------------------------ |
| StatusCode | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg  | string | Return specific information                                  |
| list       | array  | The list of dates                                            |

###### Return sample

```json
{
	"StatusCode":0,
    "StatusMsg":"Get query log date success",
	"list":[
		"20220828",
        "20220826",
        "20220825",
        "20220820"
    ]
}
```

##### 5.3.3.30 accesslog

###### Brief description

- Get the access log of the API

###### Request URL

`http://127.0.0.1:9000`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON `

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `accesslog`                   |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| date           | yes       | string | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int    | Page number. The value ranges from 1 to N. The default value is 1 |
| pageSize       | yes       | int    | The number of entries per page. The value ranges from 1 to N. The default value is 10 |

###### Return value

| Parameter       | Type   | Note                                                         |
| :-------------- | :----- | ------------------------------------------------------------ |
| StatusCode      | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg       | string | Return specific information                                  |
| totalSize       | int    | Total number                                                 |
| totalPage       | int    | Total page number                                            |
| pageNo          | int    | Current page number                                          |
| pageSize        | int    | The number of entries per page                               |
| list            | Array  | Log array                                                    |
| ---- ip         | string | The IP that makes the access                                 |
| ----operation   | string | The type of operation performed                              |
| ---- createtime | string | The time of the operation                                    |
| ---- code       | string | The result of the operation (refer to Appendix: Return value code table for details) |
| ---- msg        | string | Log description                                              |

###### Return sample

```json
{
	"StatusCode":0,
    "StatusMsg":"Get access log success",
	"totalSize":64,
	"totalPage":13,
	"pageNo":2,
	"pageSize":5,
	"list":[
		{
            "ip":"127.0.0.1",
            "operation":"StopServer",
            "createtime":"2021-12-14 09:55:16", 
            "code":0,
            "msg":"Server stopped successfully."
        }
        ......
    ]
}
```

##### 5.3.3.31 accesslogdate

###### Brief description

- Get the date of gStore's access log (for the date parameter of the access log interface)

###### Request URL

`http://127.0.0.1:9000`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON `

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `querylogdate`                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter  | Type   | Note                                                         |
| :--------- | :----- | ------------------------------------------------------------ |
| StatusCode | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg  | string | Return specific information                                  |
| list       | array  | The list of dates                                            |

###### Return sample

```json
{
	"StatusCode":0,
    "StatusMsg":"Get access log date success",
	"list":[
		"20220913",
        "20220912",
        "20220911",
        "20220818",
        "20220731",
        "20220712",
        "20220620",
    ]
}
```

##### 5.3.3.32 ipmanage

###### Brief description

- Blacklist and whitelist management

###### Request URL

`http://127.0.0.1:9000`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest`, passed as `JSON ` structure

###### Parameter

Querying the blacklist and whitelist:

| Parameter name | Mandatory | Type   | Note                                       |
| -------------- | --------- | ------ | ------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**ipmanage** |
| username       | yes       | string | User name                                  |
| password       | yes       | string | Password (plain text)                      |
| type           | yes       | string | Operation type, fixed value is**1**        |

Saving the blacklist and whitelist:

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is**ipmanage**                   |
| username       | yes       | string | user name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| type           | yes       | string | Operation type, fixed value is**2**                          |
| whiteIPs       | yes       | string | Whitelist (multiple with **,** split, support range configuration, use **-** connection such as: IP1-1P2) |
| blackIPs       | yes       | string | Blacklist (multiple with **,** split, support range configuration, use **-** connection such as: IP1-1P2) |

```json
//保存POST示例{    "operation": "ipmanage",    "username": "root",    "password": "123456",    "type": "2",    "whiteIPs":"127.0.0.1,183.67.4.126-183.67.4.128",    "blackIPs": "192.168.1.141"}
```

###### Return value

| Parameter    | Type   | Note                                                         |
| :----------- | :----- | ------------------------------------------------------------ |
| StatusCode   | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg    | string | Return specific information                                  |
| ResponseBody | Object | Return data (only for queries)                               |
| whiteIPs     | array  | Whitelist                                                    |
| blackIPs     | array  | Blacklist                                                    |

###### Return sample

```json
// 查询黑白名单返回{    "StatusCode": 0,    "StatusMsg": "success",    "ResponseBody": {        "whiteIPs": [            "127.0.0.1",            "183.67.4.126-183.67.4.128"        ],        "blackIPs": [            "192.168.1.141"        ]    }}// 保存黑白名单返回{    "StatusCode": 0,    "StatusMsg": "success"}
```

<div STYLE="page-break-after: always;"></div>

##### 5.3.3.33 funquery (New in this version)

###### Brief description

- Show user-defined graph analysis functions

###### Request URL

- `http://127.0.0.1:9000/`

###### Request mode

- POST

###### Parameter transfer mode

- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

###### Parameter

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funquery`                    |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| funInfo        | no        | JSONObject | Query parameters                                             |
| ---- funName   | no        | string     | Function name                                                |
| ---- funStatus | no        | string     | Status (1- to compile; 2- compiled; 3- exception)            |

###### Return value

| Parameter name | Type      | Note                                                         |
| -------------- | --------- | ------------------------------------------------------------ |
| StatusCode     | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string    | Return specific information                                  |
| list           | JSONArray | JSON array (if there is no data, no array is returned)       |
| ---- funName   | string    | Name                                                         |
| ---- funDesc   | string    | Description                                                  |
| ---- funArgs   | string    | Parameter type (1- no K hop parameter; 2- with K hop parameter) |
| ---- funBody   | string    | Function content                                             |
| ---- funSubs   | string    | Child functions (can be called in funBody)                   |
| ---- funStatus | string    | Status (1- to compile; 2- compiled; 3- exception)            |
| ---- lastTime  | string    | Last edit time (yyyy-MM-dd HH:mm:ss)                         |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"list": [
		{
			"funName": "demo",
			"funDesc": "this is demo",
			"funArgs": "2",
			"funBody": "{\nstd::cout<<\"uid=\<<uid<<endl;\nstd::cout<<\"vid=\<<vid<<endl;\nstd::cout<<\"k=\"<<k<<endl;\nreturn \"success\";\n}",
			"funSubs": "",
			"funStatus": "1",
			"lastTime": "2022-03-15 11:32:25"
		}
	]
}
```

##### 5.3.3.34 funcudb (New in this version)

###### Brief description

- User-defined graph analysis function management (add, modify, delete, compile)

###### Request URL

- `http://127.0.0.1:9000/`

###### Request mode

- POST

###### Parameter transfer mode

- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

###### Parameter

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funcudb`                     |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string     | 1: add, 2: modify, 3: delete, 4: compile                     |
| funInfo        | yes       | JSONObject | Operator function                                            |
| ---- funName   | yes       | string     | Function name                                                |
| ---- funDesc   | no        | string     | Description                                                  |
| ---- funArgs   | no        | string     | Parameter type (1- no K hop parameter; 2- with K hop parameter). Required for add or modify |
| ---- funBody   | no        | string     | Function contents (wrapped in {}). Required for add or modify |
| ---- funSubs   | no        | string     | Child functions (can be called in funBody)                   |
| ---- funReturn | no        | string     | Return type. Required for add or modify                      |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success"
}
```

##### 5.3.3.35 funreview (New in this version)

###### Brief description

- Preview the user-defined graph analysis function

###### Request URL

- `http://127.0.0.1:9000/`

###### Request mode

- POST

###### Parameter transfer mode

- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

###### Parameter

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funreview`                   |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| funInfo        | yes       | JSONObject | Operator function                                            |
| ---- funName   | yes       | string     | Function name                                                |
| ---- funDesc   | no        | string     | Description                                                  |
| ---- funArgs   | no        | string     | Parameter type (1- no K hop parameter; 2- with K hop parameter) |
| ---- funBody   | no        | string     | Function contents (contents wrapped in {})                   |
| ---- funSubs   | no        | string     | Child function (can be called in funBody)                    |
| ---- funReturn | no        | string     | Return type (`path`: the path class result; `value`: value class result) |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| result         | string | Function source code (decode is required)                    |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
    "Result":"%23include+%3Ciostream%3E%0A%23include+%22..%2F..%2FDatabase%2FCSRUtil.h%22%0A%0Ausing+..."
}
```

 

##### Appendix: return value code table

| Code value | Definition                                  |
| ---------- | ------------------------------------------- |
| 0          | Success                                     |
| 1000       | The method type is not supported            |
| 1001       | Authentication Failed                       |
| 1002       | Check Privilege  Failed                     |
| 1003       | Param is illegal                            |
| 1004       | The operation conditions  are not satisfied |
| 1005       | Operation failed                            |
| 1006       | Add privilege Failed                        |
| 1007       | Loss of lock                                |
| 1008       | Transcation manage Failed                   |
| 1100       | The operation is  not defined               |
| 1101       | IP Blocked                                  |

<div STYLE="page-break-after: always;"></div>

### 5.4 grpc API instruction

#### 5.4.1 API Connection Mode

> The grpc interface adopts the HTTP protocol and supports multiple ways to access the interface. If the grpc is started on the port 9000, the interface connection information is as follows.
>
> API address:
>
> ```
> http://ip:9000/grpc
> ```
>
> The interface supports both `GET` and `POST` requests, where `GET` requests place parameters in the URL and `POST` requests place parameters in the `body` request or use `form` to express a request.
>
> Post request method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)
>
> Post request method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

> Note: If the `GET` request parameters contain special characters, such as? , @, and &, you need to use the urlencode encoding, especially for the `SPARQL` parameter.

#### 5.4.2 API list

| API name            | Definition                                | Note                                                         |
| ------------------- | ----------------------------------------- | ------------------------------------------------------------ |
| check               | Detect heartbeat signal                   | Detects service heartbeat signal                             |
| login               | login to database                         | Used to authenticate user names and passwords                |
| testConnect         | testing connectivity                      | Used to check whether GRPC is connected                      |
| getCoreVersion      | get gStore version                        | Gets the gStore version number                               |
| ipmanage            | black/white list management               | Maintains a blacklist and whitelist of IP addresses that access gStore |
| show                | display graph database                    | Displays the list of all databases                           |
| load                | load graph database                       | Loads the database into memory                               |
| unload              | unload graph database                     | Unloads the database from memory                             |
| monitor             | monitor graph database                    | Displays information about the specified database (such as the number of triples, etc.) |
| build               | build graph database                      | Requires the database file to be stored locally on the server |
| drop                | drop graph database                       | Logical deletion or physical deletion can be performed       |
| backup              | backup database                           | Backups database information                                 |
| restore             | restore database                          | Restores database information                                |
| query               | query database                            | Includes query, delete, and insert                           |
| export              | export database                           | Exports database as NT file                                  |
| begin               | Start transaction                         | Transaction starts and needs to be used in conjunction with TQuery |
| tquery              | Querying the database (with transactions) | Data queries with transaction mode (insert and delete only)  |
| commit              | commit transactions                       | Commits the transaction after it completes                   |
| rollback            | rollback transaction                      | Rolls back the transaction to begin state                    |
| checkpoint          | write data to a disk                      | After an insert or delete operation is performed on the database, manual checkpoint is required |
| batchInsert         | batch insert data                         | Batch inserts NT data                                        |
| batchRemove         | batch delete data                         | Batch deletes NT data                                        |
| usermanage          | user management                           | Adds, deletes, or modifies user information                  |
| showuser            | display all user list                     | Displays a list of all users                                 |
| userprivilegemanage | user privilege management                 | Adds, deletes, or modifies user's privilege information      |
| userpassword        | modify user's password                    |                                                              |
| txnlog              | Obtain transaction log information        | Returns transcation log information as JSON                  |
| querylog            | Get query log information                 | Returns query log information as JSON                        |
| querylogdate        | Gets the date list of query logs          | Returns the date list of existing query logs                 |
| accesslog           | Get API access logs                       | Returns API access log information as JSON                   |
| accesslogdate       | Get the date of the API log               | Returns the date list of existing API logs                   |
| funquery            | Query operator function                   | Gets a list of user-defined graph analysis functions         |
| funcudb             | Manage operator function                  | Adds, modifies, deletes, or compiles operator function       |
| funreview           | Preview operator function                 | View the latest generated graph analysis function source code |
| shutdown            | Stop gRPC service                         |                                                              |

#### 5.4.3 API specific intruction

> The input and output parameters of each interface are specified in this section. Assume that the IP address of the GRPC server is 127.0.0.1 and the port is 9000.

##### 5.4.3.1 check

###### Brief description

- Check whether the gRPC service is online

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

    method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

###### Parameter

| Parameter name | Mandatory | type   | Note                                     |
| -------------- | --------- | ------ | ---------------------------------------- |
| operation      | yes       | string | Operation name, fixed value is **check** |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "the grpc server is running..."
}
```

##### 5.4.3.2 login

###### Brief description

- User login (verify username and password)

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

    method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

###### Parameter

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **login**                     |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Kernel version                                               |
| licensetype    | string | Certificate type (Open Source or Enterprise)                 |
| RootPath       | string | Full path to the gStore root directory                       |
| type           | string | HTTP service type. The fixed value is grpc                   |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "login successfully",
	"CoreVersion": "1.2",
	"licensetype": "opensource",
	"Rootpath": "/data/gstore",
	"type": "grpc"
}
```

##### 5.4.3.3 testConnect

###### Brief description

- Test whether the server can connect (for Workbench)

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

    method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

###### Parameter

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **testConnect**               |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Kernel version                                               |
| licensetype    | string | Certificate type (Open Source or Enterprise)                 |
| type           | string | HTTP service type. The fixed value is grpc                   |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"CoreVersion": "1.2",
	"licensetype": "opensource",
	"type": "grpc"
}
```

##### 5.4.3.4 getCoreVersion

###### Brief description

- Get the server version number (for Workbench)

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

    method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

###### Parameter

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **getCoreVersion**            |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| CoreVersion    | string | Kernel version                                               |
| type           | string | HTTP service type. The fixed value is grpc                   |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"CoreVersion": "1.2",
	"type": "grpc"
}
```

##### 5.4.3.5  ipmanage

###### Brief description

- Blacklist and whitelist management

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

    method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

###### Parameter

Querying blacklist and whitelist:

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **ipmanage**                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string | Operation type, fixed value is 1                             |

Saving blacklist and whitelist:

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **ipmanage**                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string | Operation type, fixed value is 2                             |
| ip_type        | yes       | string | List type: 1-blacklist; 2- whitelist                         |
| ips            | yes       | string | IP list (use`,` to split; support range configuration with `-`, e.g. ip1-1p2) |

```json
//Example of Saving blacklist and whitelist
{
	"operation": "ipmanage",
	"username": "root",
	"password": "123456",
	"type": "2",
	"ip_type":"1",
	"ips": "192.168.1.111,192.168.1.120-192.168.1.129"
}
```

###### Return value

| Parameter name | Type       | Note                                                         |
| -------------- | ---------- | ------------------------------------------------------------ |
| StatusCode     | int        | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string     | Return specific information                                  |
| ResponseBody   | JSONObject | Data returned (only returned when queried)                   |
| ---- ip_type   | string     | List type: 1- blacklist; 2- whitelist                        |
| ---- ips       | array      | list                                                         |

###### Return sample

```json
//Query the blacklist and whitelist
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"ResponseBody": {
		"ip_type": "1",
		"ips": [
			"192.168.1.111",
			"192.168.1.120-192.168.1.129"
		]
	}
}
//Save the blacklist and whitelist
{
	"StatusCode": 0,
	"StatusMsg": "success"
}
```

##### 5.4.3.6 show

###### Brief description

- Display the list of all databases

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

    method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

###### Parameter

| Parameter name | Mandatory | type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is **show**                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter name  | Type      | Note                                                         |
| --------------- | --------- | ------------------------------------------------------------ |
| StatusCode      | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg       | string    | Return specific information                                  |
| ResponseBody    | JSONArray | JSON arrays (each of which is a database information)        |
| ---- database   | string    | database name                                                |
| ---- creator    | string    | creator                                                      |
| ---- built_time | string    | create time                                                  |
| ---- status     | string    | database status                                              |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Get the database list successfully!",
	"ResponseBody": [
		{
			"database": "lubm",
			"creator": "root",
			"built_time": "2021-08-22 11:08:57",
			"status": "loaded"
		},
		{
			"database": "movie",
			"creator": "root",
			"built_time": "2021-08-27 20:56:56",
			"status": "unloaded"
		}
	]
}
```

##### 5.4.3.7 load

###### Brief description

- Load the database into memory. Loading a database is a prerequisite for many operations, such as query and monitor.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

    method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `load`                        |
| username       | yes       | string | User name                                                    |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| password       | yes       | string | Password (plain text)                                        |
| db_name        | yes       | string | Database name (.db is not required)                          |
| csr            | no        | string | Whether to load CSR resources, default is 0 (set to 1 when using advanced query functions) |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| csr            | string | Whether to load CSR resources                                |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database loaded successfully.",
	"csr": "1"
}
```

##### 5.4.3.8 unload

###### Brief description

- Unload the database from memory (all changes are flushed to disk).

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

    method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `unload`                      |
| db_name        | yes       | string | Database name (.db is not required)                          |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database unloaded."
}
```

##### 5.4.3.9 monitor

###### Brief description

- Get database statistics (requires the database to be loaded)

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json)

    method 2: the parameter is passed as a form (Requires RequestHeader's parameter Content-Type to be application/x-www-form-urlencoded)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `monitor`                     |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| database       | string | database name                                                |
| creator        | string | creator                                                      |
| builtTime      | string | create time                                                  |
| tripleNum      | string | number of triples                                            |
| entityNum      | int    | number of entities                                           |
| literalNum     | int    | number of literals (i.e., attributes)                        |
| subjectNum     | int    | number of subjects                                           |
| predicateNum   | int    | number of objects                                            |
| connectionNum  | int    | number of connections                                        |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"database": "test_lubm",
	"creator": "root",
	"builtTime": "2021-08-27 21:29:46",
	"tripleNum": "99550",
	"entityNum": 28413,
	"literalNum": 0,
	"subjectNum": 14569,
	"predicateNum": 17,
	"connectionNum": 0
}
```

##### 5.4.3.10 build

###### Brief description

- Create a database based on an existing NT file.
- The file must be store locally on the gStore server.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `build`                       |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| db_path        | yes       | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Import RDF file to database done."
}
```

##### 5.4.3.11 drop

###### Brief description

- Delete the database (either logically or physically)

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `drop`                        |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| is_backup      | no        | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database test_lubm dropped."
}
```

##### 5.4.3.12 backup

###### Brief description

- Back up database

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `backup`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| backup_path    | no        | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| backupfilepath | string | Backup file path (this value can be used as the input parameter value for restore) |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database backup successfully.",
	"backupfilepath": "./backups/lubm.db_210828211529"
}
```

##### 5.4.3.13 restore

###### Brief description

- Restore database

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `restore`                     |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| backup_path    | no        | string | The full time-stamped path of the backup file (can be a relative path or an absolute path. The relative path is based on the gStore root directory). The default path is the backup directory in the gStore root directory |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database lumb restore successfully."
}
```

##### 5.4.3.14 query

###### Brief description

- Query the database

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `query`                       |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| format         | no        | string | The result set format (JSON/file/JSON+file; the default is JSON) |
| sparql         | yes       | string | The SPARQL query to execute. (Requires URL encoding for GET requests) |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| head           | JSON   | Head information                                             |
| results        | JSON   | Result information (see Return Sample for details)           |
| AnsNum         | int    | Number of results                                            |
| OutputLimit    | int    | Maximum number of results returned (-1 is not limited)       |
| ThreadId       | string | Query thread Number                                          |
| QueryTime      | string | Query time (ms)                                              |
| FileName       | string | Result file name (when the format is file or JSON+file)      |

###### Return sample

```json
// format: json
{
	"head": {
		"link": [],
		"vars": [
			"x"
		]
	},
	"results": {
		"bindings": [
			{
				"x": {
					"type": "uri",
					"value": "十面埋伏"
				}
			},
			{
				"x": {
					"type": "uri",
					"value": "投名状"
				}
			},
			{
				"x": {
					"type": "uri",
					"value": "如花"
				}
			}
		]
	},
	"StatusCode": 0,
	"StatusMsg": "success",
	"AnsNum": 15,
	"OutputLimit": -1,
	"ThreadId": "140595527862016",
	"QueryTime": "1"
}
```

```json
// format:file
// Result file root directory：%gstore_home%/query_result
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"AnsNum": 12,
	"OutputLimit": -1,
	"ThreadId": "140270360303360",
	"QueryTime": "1",
	"FileName": "140270360303360_20220914172612_258353606.txt"
}
```

##### 5.4.3.15 export

###### Brief description

- Export the database

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `export`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| db_path        | no        | string | The export file path can be relative or absolute. The relative path uses the gStore root directory as reference. |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| filepath       | string | Export file path                                             |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Export the database successfully.",
	"filepath": "export/lubm_210828214603.nt"
}
```

##### 5.4.3.16 begin

###### Brief description

- start transaction

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `begin`                       |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| isolevel       | yes       | string | Transaction isolation level. 1: RC (read committed); 2: SI (snapshot isolation); 3: SR (seriablizable） |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| TID            | string | Transaction ID                                               |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "transaction begin success",
	"TID": "1"
}
```

##### 5.4.3.17 tquery

###### Brief description

- Query as transaction

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `tquery`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| tid            | yes       | string | Transaction ID                                               |
| sparql         | yes       | string | The SPARQL query to execute                                  |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| head           | JSON   | Header information (this field is returned when a query statement is executed, but not an update statement) |
| results        | JSON   | Result information (this field is returned when a query statement is executed, but not an update statement) |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success"
}
```

##### 5.4.3.18 commit

###### Brief description

- Commit a transaction

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `commit`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| tid            | yes       | string | Transaction ID                                               |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Transaction commit success. TID: 1"
}
```

##### 5.4.3.19 rollback

###### Brief description

- Rollback trasnsaction.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `rollback`                    |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| tid            | yes       | string | Transaction ID                                               |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Transaction rollback success. TID: 2"
}
```

##### 5.4.3.20 checkpoint

###### Brief description

- Flush data back to hard disk to make it persistent.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `httprequest` , passed as `JSON`

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `checkpoint`                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Database saved successfully."
}
```

##### 5.4.3.21 batchInsert

###### Brief description

- Batch insert data.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `batchInsert`                 |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| file           | yes       | string | The NT data file to insert (can be a relative or absolute path) |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | string | Number of successfully inserted triples                      |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Batch insert data successfully.",
	"success_num": "25"
}
```

##### 5.4.3.22 batchRemove

###### Brief description

- Batch remove data.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `batchRemove`                 |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| db_name        | yes       | string | Database name (.db is not required)                          |
| file           | yes       | string | The data NT file to be deleted (can be a relative or absolute path) |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| success_num    | string | Number of successfully removed triples                       |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Batch remove data successfully.",
	"success_num": "25"
}
```

##### 5.4.3.23 usermanage

###### Brief description

- Manage users (including adding and deleting users, and modifying user information)

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `usermanage`                  |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string | Operation type（1: adduser, 2: deleteUser, 3: alterUserPassword） |
| op_username    | yes       | string | The name of the user to operate on                           |
| op_password    | no        | string | Password of the user operated on, which can be left empty when deleting the user. If the password is to be changed, input the new password here. (If the password contains special characters and the GET request is used, it must be urlencode-encoded.) |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Add user done."
}
```

###### Notes

The default interface permissions for a new user are: `login`, `check`, `testConnect`, `getCoreVersion`, `show`, `funquery`, `funcudb`, `funreview`, `userpassword`.

Users with `query` rights also have the following interface rights: `query`, `monitor`.

Users with the `update` permission also have the following interface permissions: `batchInsert`, `batchRemove`, `begin`, `tquery`, `commit`, `rollback`.

Only the root user can invoke the interface rights that are not within the scope of authorization management, for example: `build`, `drop`, `usermanage`, `showuser`, `userprivilege`, `manage`, `txnlog`, `checkpoint`, `shutdown`, `querylog`, `accesslog`, `ipmanage`.

##### 5.4.3.24 showuser

###### Brief description

- Display all users' information.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `showuser`                    |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value  

| Parameter name         | Type   | Note                                                         |
| ---------------------- | ------ | ------------------------------------------------------------ |
| StatusCode             | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg              | string | Return specific information                                  |
| ResponseBody           | string | JSON object array                                            |
| ---- username          | string | user name                                                    |
| ---- password          | string | password                                                     |
| ---- query_privilege   | string | Query permissions (database names separated by commas)       |
| ---- update_privilege  | string | Update permissions (database names separated by commas)      |
| ---- load_privilege    | string | Load permissions (database names separated by commas)        |
| ---- unload_privilege  | string | Unload permissions (database names separated by commas)      |
| ---- backup_privilege  | string | Backup permissions (database names separated by commas)      |
| ---- restore_privilege | string | Restore permissions (database names separated by commas)     |
| ---- export_privilege  | string | Export permissions (database names separated by commas)      |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"ResponseBody": [
		{
			"username": "test",
			"password": "123456",
			"query_privilege": "lubm10,lubm100",
			"update_privilege": "",
			"load_privilege": "lubm10,lubm100",
			"unload_privilege": "lubm10,lubm100",
			"backup_privilege": "",
			"restore_privilege": "",
			"export_privilege": ""
		},
		{
			"username": "root",
			"password": "123456",
			"query_privilege": "all",
  		"update_privilege": "all",
			"load_privilege": "all",
			"unload_privilege": "all",
			"backup_privilege": "all",
			"restore_privilege": "all",
			"export_privilege": "all"
		}
	]
}
```

##### 5.4.3.25 userprivilegemanage

###### Brief description

- Manage user permissions (including adding, deleting, and modifying them).

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `userprivilegemanage`         |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string | Operation type (1: add permission，2: delete permission, 3: clear permission) |
| op_username    | yes       | string | The name of the user to operate on                           |
| privileges     | no        | string | Permissions to operate on (multiple permissions are separated by commas; if it is a clear operation, this can be an empty string.) 1: query, 2: load, 3: unload, 4: update, 5: backup, 6: restore, 7: export |
| db_name        | no        | string | The database to operate on (this can be empty if it is a clear operation) |

###### Return value  

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "add privilege query successfully. \r\nadd privilege load successfully. \r\nadd privilege unload successfully. \r\nadd privilege update successfully. \r\nadd privilege backup successfully. \r\n"
}
```

##### 5.4.3.26 userpassword

###### Brief description

- Modify the user password.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `userpassword`                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password                                                     |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| op_password    | yes       | string | New password (plain text)                                    |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Change password done."
}
```

##### 5.4.3.27 txnlog

###### Brief description

- Get transaction logs.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `txnlog`                      |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password (plain text)                                        |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| pageNo         | yes       | int    | Page number, the value ranges from 1 to N, the default value is 1 |
| pageSize       | yes       | int    | The number of entries per page, the value ranges from 1 to N, the default value is 10 |

###### Return value

| Parameter name  | Type      | Note                                                         |
| --------------- | --------- | ------------------------------------------------------------ |
| StatusCode      | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg       | string    | Return specific information                                  |
| totalSize       | int       | Total size                                                   |
| totalPage       | int       | The total number of pages                                    |
| pageNo          | int       | The current page number                                      |
| pageSize        | int       | The number of entries per page                               |
| list            | JSONArray | Log array                                                    |
| ---- db_name    | string    | Database name                                                |
| ---- TID        | string    | Transaction ID                                               |
| ---- user       | string    | User                                                         |
| ---- state      | string    | State (COMMITED/RUNNING/ROLLBACK)                            |
| ---- begin_time | string    | Start time                                                   |
| ---- end_time   | string    | End time                                                     |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Get Transaction log success",
	"totalSize": 2,
	"totalPage": 1,
	"pageNo": 1,
	"pageSize": 10,
	"list": [
		{
			"db_name": "lubm2",
			"TID": "1",
			"user": "root",
			"begin_time": "1630376221590",
			"state": "COMMITED",
			"end_time": "1630376277349"
		},
		{
			"db_name": "lubm2",
			"TID": "2",
			"user": "root",
			"begin_time": "1630376355226",
			"state": "ROLLBACK",
			"end_time": "1630376379508"
		}
	]
}
```

##### 5.4.3.28 querylog

###### Brief description

- Obtaining query logs.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type         | Note                                                         |
| -------------- | --------- | ------------ | ------------------------------------------------------------ |
| operation      | yes       | string       | Operation name, fixed value is `querylog`                    |
| username       | yes       | string       | User name                                                    |
| password       | yes       | string       | Password (plain text)                                        |
| encryption     | noyes     | stringstring | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| date           | yes       | string       | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int          | Page number, the value ranges from 1 to N, the default value is 1 |
| pageSize       | yes       | int          | The number of entries per page, the value ranges from 1 to N, the default value is 10 |

###### Return value

| Parameter name     | Type      | Note                                                         |
| ------------------ | --------- | ------------------------------------------------------------ |
| StatusCode         | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg          | string    | Return specific information                                  |
| totalSize          | int       | Total size                                                   |
| totalPage          | int       | The total number of pages                                    |
| pageNo             | int       | The current page number                                      |
| pageSize           | int       | The number of entries per page                               |
| list               | JSONArray | Log array                                                    |
| ---- QueryDateTime | string    | Query time                                                   |
| ---- RemoteIP      | string    | The IP that sends the request                                |
| ---- Sparql        | string    | SPARQL query                                                 |
| ---- AnsNum        | int       | Result number                                                |
| ---- Format        | string    | Query return format                                          |
| ---- FileName      | string    | Query result set files                                       |
| ---- QueryTime     | int       | Time (ms)                                                    |
| ---- StatusCode    | int       | Status code                                                  |
| ---- DbName        | string    | Database name                                                |

###### Return sample

```json
{
  "StatusCode":0,
	"StatusMsg":"Get query log success",
	"totalSize":64,
	"totalPage":13,
	"pageNo":2,
	"pageSize":5,
	"list":[
		{
			"QueryDateTime":"2021-11-16 14:55:52:90ms:467microseconds",
			"Sparql":"select ?name where { ?name <dislike> <Eve>. }",
			"Format":"json",
			"RemoteIP":"183.67.4.126",
			"FileName":"140163774674688_20211116145552_847890509.txt",
			"QueryTime":0,
			"AnsNum":2,
			"StatusCode": 0,
			"DbName": "demo"
		}
		......
	]
}
```

##### 5.4.3.29 querylogdate

###### Brief description

- Get the date of gStore's query log (for the date parameter of the querylog interface).

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `querylogdate`                |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password                                                     |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| list           | array  | Date list                                                    |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Get query log date success",
	"list":[
		"20220828",
		"20220826",
		"20220825",
		"20220820"
	]
}
```

##### 5.4.3.30 accesslog

###### Brief description

- Get gStore access logs.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `accesslog`                   |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password                                                     |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| date           | yes       | string | Date format is yyyyMMdd                                      |
| pageNo         | yes       | int    | Page number, the value ranges from 1 to N, the default value is 1 |
| pageSize       | yes       | int    | The number of entries per page, the value ranges from 1 to N, the default value is 10 |

###### Return value

| Parameter name  | Type      | Note                                                         |
| --------------- | --------- | ------------------------------------------------------------ |
| StatusCode      | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg       | string    | Return specific information                                  |
| totalSize       | int       | Total size                                                   |
| totalPage       | int       | The total number of pages                                    |
| pageNo          | int       | The current page number                                      |
| pageSize        | int       | The number of entries per page                               |
| list            | JSONArray | Log array                                                    |
| ---- ip         | string    | The IP of the access                                         |
| ---- operation  | string    | Operation type                                               |
| ---- createtime | string    | Operation time                                               |
| ---- code       | string    | Operation results (refer to Appendix: Return value code table for details) |
| ---- msg        | string    | Log descriptions                                             |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Get access log success",
	"totalSize":64,
	"totalPage":13,
	"pageNo":2,
	"pageSize":5,
	"list":[
		{
			"ip":"127.0.0.1",
			"operation":"StopServer",
			"createtime":"2021-12-14 09:55:16",
			"code":0,
			"msg":"Server stopped successfully."
		}
    ......
	]
}
```

##### 5.4.3.31 accesslogdate

###### Brief description

- Get the date of API log (for the date parameter of the accesslog interface).

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL

- POST request

    method 1 (recommended) : the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (The parameter Content-Type: application/json of RequestHeader is required.)

    method 2: the parameter is passed as a form (The parameter Content-Type: application/x-www-form-urlencoded of RequestHeader is required.)

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| operation      | yes       | string | Operation name, fixed value is `accesslogdate`               |
| username       | yes       | string | User name                                                    |
| password       | yes       | string | Password                                                     |
| encryption     | no        | string | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| list           | array  | Date list                                                    |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Get access log date success",
	"list":[
		"20220913",
		"20220912",
		"20220911",
		"20220818",
		"20220731",
		"20220712",
		"20220620",
	]
}
```

##### 5.4.3.32 funquery

###### Brief description

- Show user-defined graph analysis functions.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json.)

###### Parameter

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funquery`                    |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| funInfo        | no        | JSONObject | Function information                                         |
| ---- funName   | no        | string     | Function name                                                |
| ---- funStatus | no        | string     | Status (1- to compile; 2- compiled; 3- exception)            |

###### Return value

| Parameter name | Type      | Note                                                         |
| -------------- | --------- | ------------------------------------------------------------ |
| StatusCode     | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string    | Return specific information                                  |
| list           | JSONArray | JSON array (if there is no data, no array is returned)       |
| ---- funName   | string    | Function name                                                |
| ---- funDesc   | string    | Function description                                         |
| ---- funArgs   | string    | Parameter type (1- no K-hop parameter; 2- with K-hop parameter) |
| ---- funBody   | string    | Function content                                             |
| ---- funSubs   | string    | Child functions (can be called from funBody)                 |
| ---- funStatus | string    | Status (1- to compile; 2- compiled; 3- exception)            |
| ---- lastTime  | string    | Last edit time (yyyy-MM-dd HH:mm:ss)                         |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
	"list": [
		{
			"funName": "demo",
			"funDesc": "this is demo",
			"funArgs": "2",
			"funBody": "{\nstd::cout<<\"uid=\"<<uid<<endl;\nstd::cout<<\"vid=\"<<vid<<endl;\nstd::cout<<\"k=\"<<k<<endl;\nreturn \"success\";\n}",
			"funSubs": "",
			"funStatus": "1",
			"lastTime": "2022-03-15 11:32:25"
		}
	]
}
```

##### 5.4.3.33 funcudb

###### Brief description

- User-defined graph analysis function management (add, modify, delete, compile).

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, the parameter is passed as `JSON` by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json.)

###### Parameter

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funcudb`                     |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| type           | yes       | string     | 1: add, 2: modify, 3: delete, 4: compile                     |
| funInfo        | yes       | JSONObject | Function information                                         |
| ---- funName   | yes       | string     | Function name                                                |
| ---- funDesc   | no        | string     | Function description                                         |
| ---- funArgs   | no        | string     | Parameter type (1- no K hop parameter; 2- with K hop parameter). Required for add or modify |
| ---- funBody   | no        | string     | Function contents (wrapped in {}). Required for add or modify |
| ---- funSubs   | no        | string     | Child functions (can be called in funBody)                   |
| ---- funReturn | no        | string     | Return type (`path`: single-path result; `value`: constant value result; `multipath`: multiple-path result). Required for add or modify |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Function create success"
}
```

##### 5.4.3.34 funreview

###### Brief description

- Preview the user-defined graph analysis function.

###### Request URL

- `http://127.0.0.1:9000/grpc/api`

###### Request mode

- POST

###### Parameter transfer mode

- POST request, the parameter is passed as `JSON` structure by `raw` in `body` in `httprequest` (Requires RequestHeader's parameter Content-Type to be application/json.)

###### Parameter

| Parameter name | Mandatory | Type       | Note                                                         |
| -------------- | --------- | ---------- | ------------------------------------------------------------ |
| operation      | yes       | string     | Operation name, fixed value is `funreview`                   |
| username       | yes       | string     | User name                                                    |
| password       | yes       | string     | Password (plain text)                                        |
| encryption     | no        | string     | If the value is null, the password is in plain text. If the value is 1, the password is encrypted using md5 |
| funInfo        | yes       | JSONObject | Function information                                         |
| ---- funName   | yes       | string     | Function name                                                |
| ---- funDesc   | no        | string     | Function description                                         |
| ---- funArgs   | no        | string     | Parameter type (1- no K-hop parameter; 2- with K-hop parameter) |
| ---- funBody   | no        | string     | Function contents (wrapped in {})                            |
| ---- funSubs   | no        | string     | Child functions (can be called in funBody)                   |
| ---- funReturn | no        | string     | Return type (`path`: single-path result; `value`: constant value result; `multipath`: multiple-path result) |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| result         | string | Function source code (decoding is required)                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "success",
  "Result":"%23include+%3Ciostream%3E%0A%23include+%22..%2F..%2FDatabase%2FCSRUtil.h%22%0A%0Ausing+..."
}
```

##### 5.4.3.35 shutdown

###### Brief description

- Shut down the gRPC service.

###### Request URL

- `http://127.0.0.1:9000/grpc/shutdown`

###### Request mode

- GET/POST

###### Parameter transfer mode

- GET request, the parameters are passed directly as the URL
- POST request, `raw` in `body` in `Httprequest` , passed as ` JSON` 

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| -------------- | --------- | ------ | ------------------------------------------------------------ |
| username       | yes       | string | User name (default user name is system)                      |
| password       | yes       | string | Password（This password need to be viewed in the server's system.db/password[port].txt file. For example, if the port is 9000, then the corresponding password is in password9000.txt) |

###### Return value

| Parameter name | Type   | Note                                                         |
| -------------- | ------ | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |

###### Return sample

```json
{
	"StatusCode": 0,
	"StatusMsg": "Server stopped successfully."
}
```

##### Appendix 1: return value code table

| Code value | Definition                                 |
| ---------- | ------------------------------------------ |
| 0          | Success                                    |
| 14         | Route not fund                             |
| 1000       | The method type is not supported           |
| 1001       | Authentication Failed                      |
| 1002       | Check Privilege Failed                     |
| 1003       | Param is illegal                           |
| 1004       | The operation conditions are not satisfied |
| 1005       | Operation failed                           |
| 1006       | Add privilege Failed                       |
| 1007       | Loss of lock                               |
| 1008       | Transcation manage Failed                  |
| 1100       | The operation is not defined               |
| 1101       | IP Blocked                                 |



### 5.5 C++ HTTP API

---

To use the C++ API, put the phrase '#include "client.h" in your CPP code, as shown below：

**Construct the initialization function**

	GstoreConnector(std::string serverIP, int serverPort, std::string httpType, std::string username, std::string password);
	Function: Initialize
	Parameter Definition：[Server IP], [HTTP port on the server], [HTTP service type], [Username], [password]
	Example：GstoreConnector gc("127.0.0.1", 9000, "ghttp", "root", "123456");

**Build database: build**
	

	std::string build(std::string db_name, std::string rdf_file_path, std::string request_type);
	Function：Create a new database from an RDF file
	Parameter Definition：[database name], [.nt file path], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.build("lubm", "data/lubm/lubm.nt");

**Load database: load**

	std::string load(std::string db_name, std::string request_type);
	Function：Load the database you created
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.load("lubm");

**Stopping database loading: unload**
	

	std::string unload(std::string db_name, std::string request_type);
	Function：Stopping database loading
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.unload("lubm");

**User management: user**

	std::string user(std::string type, std::string username2, std::string addition, std::string request_type);
	Function：The root user can add, delete, or modify the user's permission only
	1.Add or delete users：
	Parameter Definition：["add_user" adds user, "delete_user" deletes user],[user name],[password],[request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.user("add_user", "user1", "111111");
	2.Modify user's privilege：
	Parameter Definition：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission],[user name],[database name],[Request type "GET" and "POST ", if the request type is "GET", it can be omitted.]
	Example：gc.user("add_query", "user1", "lubm");

**Display user: showUser**

	std::string showUser(std::string request_type);
	Function：Display all Users
	Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.showUser();

**Database query: query**

	std::string query(std::string db_name, std::string format, std::string sparql, std::string request_type);
	Function：Query the database
	Parameter Definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [Request type "GET" and "POST ", if the request type is "GET", it can be omitted]
	Example：
	std::string res = gc.query("lubm", "json", sparql);
	std::cout << res << std::endl;  //output result

**Deleting a Database: drop**

	std::string drop(std::string db_name, bool is_backup, std::string request_type);
	Function：Delete the database directly or delete the database while leaving a backup
	Parameter Definition：[database name], [false not backup, true backup], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.drop("lubm", false);  //Delete the database without leaving a backup

**Monitoring database: monitor**

	std::string monitor(std::string db_name, std::string request_type);Function：Displays information for a specific database.
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：gc.monitor("lubm");

**Save the database: checkpoint**

	std::string checkpoint(std::string db_name, std::string request_type);Function：If you change the database, save the databas Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：gc.checkpoint("lubm");

**Show the database: show**

	std::string show(std::string request_type);Function：Displays all created databases Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"] Example：gc.show();

**The kernel version information is displayed: getCoreVersion**

	std::string getCoreVersion(std::string request_type);Function：Get kernel version information Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.getCoreVersion();

**The API version information is displaye: getAPIVersion**

	std::string getAPIVersion(std::string request_type);
	Function：Get the API version information
	Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.getAPIVersion();

**Query the database and save the file: fquery**

	void fquery(std::string db_name, std::string format, std::string sparql, std::string filename, std::string request_type);
	Function：Query the database and save the results to a file
	Parameter Definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name], [request type "GET" and "POST ", if the request type is "GET", it can be omitted]
	Example：gc.fquery("lubm", "json", sparql, "ans.txt");

**Exporting the Database**

	std::string exportDB(std::string db_name, std::string dir_path, std::string request_type);
	Function：Export the database to a folder
	Parameter Definition：[database name], [path to database export folder], [request type "GET" and "POST ", if the request type is "GET", can be omitted]
	Example：gc.exportDB("lubm", "/root/gStore/");



<div STYLE="page-break-after: always;"></div>

### 5.6  Java  HTTP API

---

To use the Java API, please refer to the gStore/API/HTTP/Java/SRC/JGSC/GstoreConnector. Java. Specific use is as follows:

**Construct the initialization function**

```java
public class GstoreConnector(String serverIP, int serverPort, String httpType,
String username, String password);
Function: Initialize
Parameter Definition：[Server IP], [HTTP port on the server], [HTTP service type], [Username], [password]
Example：GstoreConnector gc = new GstoreConnector("127.0.0.1", 9000, "ghttp",
"root", "123456");
```

**Building a database: build**

	public String build(String db_name, String rdf_file_path, String request_type);
	Function：Create a new database from an RDF file
	Parameter Definition：[database name], [.nt file path], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.build("lubm", "data/lubm/lubm.nt");

**Loading a database: load**

	public String load(String db_name, String request_type);
	Function：Load the database you created
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.load("lubm");

**Stopping database loading: unload**

	public String unload(String db_name, String request_type);
	Function：Stopping database loading
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.unload("lubm");

**User management：user**

	public String user(String type, String username2, String addition, String request_type);
	Function：The root user can add, delete, or modify the user's permission only.
	1.Add or delete users：
	Parameter Definition：["add_user" adds user, "delete_user" deletes user],[user name],[password],[request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.user("add_user", "user1", "111111");
	2.Modify user's privilege：
	Parameter Definition：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission],[user name],[database name],[Request type "GET" and "POST ", if the request type is "GET", it can be omitted.]
	Example：gc.user("add_query", "user1", "lubm");

**Display user：showUser**

	public String showUser(String request_type);
	Function：Display all users
	Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.showUser();

**Database query: query**

	public String query(String db_name, String format, String sparql, String request_type);
	Function：query databse
	Parameter Definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [Request type "GET" and "POST ", if the request type is "GET", it can be omitted]
	Example：
	String res = gc.query("lubm", "json", sparql);
	System.out.println(res); //output result

**Database deletion ：drop**

	public String drop(String db_name, boolean is_backup, String request_type);
	Function：Delete the database directly or delete the database while leaving a backup.
	Parameter Definition：[database name], [false not backup, true backup], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：gc.drop("lubm", false);  //Delete the database without leaving a backup

**Monitoring database: monitor**

	public String monitor(String db_name, String request_type);
	Function：Displays information for a specific database
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：gc.monitor("lubm");

**Save the database: checkpoint**

	public String checkpoint(String db_name, String request_type);
	Function：If you change the database, save the database 
	Parameter Definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：gc.checkpoint("lubm");

**Show database：show**

	public String show(String request_type);Function：Displays all created databasesParameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：gc.show();

**The kernel version information is displayed：getCoreVersion**

	public String getCoreVersion(String request_type);Function：Get kernel version information Parameter Definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：gc.getCoreVersion();

**Display API version：getAPIVersion**

	public String getAPIVersion(String request_type);Function：Get API version Parameter：[Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：gc.getAPIVersion();

**Query the database and save the file: fquery**

	public void fquery(String db_name, String format, String sparql, String filename, String request_type);Function：Query the database and save the result to a file Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：gc.fquery("lubm", "json", sparql, "ans.txt");

**Export database**

	public String exportDB(String db_name, String dir_path, String request_type);Function：Export database to parameter definition under file folder：[database name], [database export folder path]，[Request types "GET" and "POST" can be omitted if the request type is "GET"】Example：gc.exportDB("lubm", "/root/gStore/");

<div STYLE="page-break-after: always;"></div>

### 5.7  Python HTTP API

---

To use the Python API, please refer to the gStore/API/HTTP/Python/SRC/GstoreConnector. Py. Specific use is as follows:

**Construct the initialization function**

```python
public class GstoreConnector(self, serverIP, serverPort, httpType, username,
password):
Function: Initialize
Parameter Definition：[Server IP], [HTTP port on the server], [HTTP service type], [Username], [password]
Example：gc = GstoreConnector.GstoreConnector("127.0.0.1", 9000, "ghttp", "root",
"123456")
```

**Build database: build**

	def build(self, db_name, rdf_file_path, request_type):
	Function：Create a new database from an RDF file
	Parameter definition：[Database name]，[.nt文件路径]，[Request types "GET" and "POST" can be omitted if the request type is "GET"】
	Example：res = gc.build("lubm", "data/lubm/lubm.nt")

**Load database: load**

	def load(self, db_name, request_type):
	Function：load the database you have created
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：res = gc.load("lubm")

**Unload database: unload**

	def unload(self, db_name, request_type):
	Function：Unload database
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：res = gc.unload("lubm")

**User management: user**

	def user(self, type, username2, addition, request_type):
	Function：The root user can add, delete, or modify the user's permission only.
	1.Add or delete users：
	Parameter definition：["add_user" adds a user, "delete_user" deletes a user],  [username],[password],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：res = gc.user("add_user", "user1", "111111")
	2.Modify user's privilege：
	Parameter definition：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission],  [user name],[database name],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：res = gc.user("add_query", "user1", "lubm")

**Display users: showUser**

	def showUser(self, request_type):
	Function：Display all users
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.showUser()

**Query Database: query**

	def query(self, db_name, format, sparql, request_type):
	Function：Query the database
	Parameter definition：[Database name]，[Query result type JSON, HTML or text], [SPARQL statement], [Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：
	res = gc.query("lubm", "json", sparql)
	print(res) //output result

**Database deletion: drop**

	def drop(self, db_name, is_backup, request_type):
	Function：Delete the database directly or delete the database while leaving a backup
	Parameter definition：[database name], [false not backup, true backup],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.drop("lubm", false)  //Delete the database without leaving a backup

**Database Monitor ：monitor**   	

	def monitor(self, db_name, request_type):    Function：Displays information for a specific database Parameter definition：[Database name]，[Request types "GET" and "POST" can be omitted if the request type is "GET"] 
	Example：res = gc.monitor("lubm")

**Save database: checkpoint**

	def checkpoint(self, db_name, request_type):Function：If the database is changed, save the meaning of the database parameters：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted] Example：res = gc.checkpoint("lubm")

**Display database：show**

	def show(self, request_type):Function：Display the meanings of all created databases：[Request types "GET" and "POST" can be omitted if the request type is "GET"] 
	Example：res = gc.show()

**The kernel version information is displayed: getCoreVersion**

	def getCoreVersion(self, request_type):Function：Get the definition of kernel version parameter ：[Request types "GET" and "POST" can be omitted if the request type is "GET"] 
	Example：res = gc.getCoreVersion()

**Display API version: getAPIVersion**	

	def getAPIVersion(self, request_type):Function：Get the API version information parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.getAPIVersion()

**Query the database and save the file: fquery**

	def fquery(self, db_name, format, sparql, filename, request_type):Function：Query the database and save the result to a file Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：gc.fquery("lubm", "json", sparql, "ans.txt")

**Export databse**

	def exportDB(self, db_name, dir_path, request_type): Function：parameter definition of exprotin database to folders：[database name], [database export folder path]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.exportDB("lubm", "/root/gStore/")

<div STYLE="page-break-after: always;"></div>

### **5.8** Node.js HTTP API

---

Before using the Nodejs API, type `NPM install Request` and `NPM Install request-promise` to add the required modules under the Nodejs folder.

To use Nodejs API, please refer to the `gStore/API/http/Nodejs/GstoreConnector.js`. Specific use is as follows:

**Construct the initialization function**

	class GstoreConnector(ip = '', port, httpType = 'ghttp', username = '', password
	= '');
	Function: Initialize
	Parameter Definition：[Server IP], [HTTP port on the server], [HTTP service type], [Username], [password]
	Example：gc = new GstoreConnector("127.0.0.1", 9000, "ghttp", "root", "123456");

**Build database: build**

	async build(db_name = '', rdf_file_path = '', request_type);
	Function：Create a new database from an RDF file
	The defintion of parameters are as follows: [database name], [.nt file path],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.build("lubm", "data/lubm/lubm.nt");

**Load database：load**

	async load(db_name = '', request_type);
	Function：Load the database you have created
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：res = gc.load("lubm");

**Unload databse：unload**

	async unload(db_name = '', request_type);
	Function：Unload databse
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：res = gc.unload("lubm");

**User management：user**

	async user(type = '', username2 = '' , addition = '' , request_type);
	Function：The root user can add, delete, or modify the user's permission only
	1.Add or delete users：
	Parameter definition：["add_user" adds a user, "delete_user" deletes a user],  [user name],[password],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：res = gc.user("add_user", "user1", "111111");
	2.Privilege to modify user：
	Parameter definition：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission],  [user name],[database name],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]]
	Example：res = gc.user("add_query", "user1", "lubm");

**Display user：showUser**

	async showUser(request_type);
	Function：Display all users
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.showUser();

**Query database：query**

	async query(db_name = '', format = '' , sparql = '' , request_type);
	Function：Query database
	Parameter definition：[Database name], [query result type JSON, HTML or text], [SPARQL statement], [Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：
	res = gc.query("lubm", "json", sparql);
	console.log(JSON.stringify(res,",")); //output result

**Database deletion：drop**

	async drop(db_name = '', is_backup , request_type);
	Function：Delete the database directly or delete the database while leaving a backup
	Parameter definition：[database name]，[false no backup, true backup]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.drop("lubm", false);  //Delete the database without leaving a backup

**Database monitor: monitor**   	

	async monitor(db_name = '', request_type);     Function：Parameter definition for displaying information about a specific database：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：res = gc.monitor("lubm");

**Save database: checkpoint**

	async checkpoint(db_name = '', request_type);Function：If the database is changed, the parameter definition of saving database：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：res = gc.checkpoint("lubm");

**Display database: show**

	async show(request_type);Function：Displays all created databases Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：res = gc.show();

**Display kernel version information: getCoreVersion**	

	async getCoreVersion(request_type);Function：Get kernel version information 
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]Example：res = gc.getCoreVersion();

**Display API version: getAPIVersion**			

	async getAPIVersion(request_type);
	Function：Get the API version information	
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：res = gc.getAPIVersion();

**Query the database and save the file: fquery**

	async fquery(db_name = '', format = '' , sparql = '' , filename = '' , request_type);
	Function：Query the database and save the results to a file
	Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name],  [Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：gc.fquery("lubm", "json", sparql, "ans.txt");

**Export database**

	async exportDB(db_name = '' , dir_path = '' , request_type); 
	Function：export database to folders
	parameter definition：[database name], [directory where the database is exported], [Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：res = gc.exportDB("lubm", "/root/gStore/");

<div STYLE="page-break-after: always;"></div>

### 5.9 PHP HTTP API

---

To use the Php API, please refer to the gStore/API/HTTP/Php/SRC/GstoreConnector. Php. Specific use is as follows:

**Construct the initialization function**

```php
class GstoreConnector($ip, $port, $httpType, $username, $password)
Function: Initialize
Parameter Definition：[Server IP], [HTTP port on the server], [HTTP service type], [Username], [password]
Example：$gc = new GstoreConnector("127.0.0.1", 9000, "ghttp", "root", "123456");
```

**Build databse：build**

	function build($db_name, $rdf_file_path, $request_type)
	Function：Create a new database from an RDF file
	Parameter definition：[database name], [.nt file path],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$res = $gc->build("lubm", "data/lubm/lubm.nt");
	echo $res . PHP_EOL;

**Load database：load**

	function load($db_name, $request_type)
	Function：Load the database you have created
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：
	$ret = $gc->load("test");
	echo $ret . PHP_EOL;

**Unload databse：unload**

	function unload($db_name, $request_type)
	Function：Unload databse
	Parameter definition：[database name]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$ret = $gc->unload("test");
	echo $ret . PHP_EOL;

**User management：user**

	function user($type, $username2, $addition, $request_type)
	Function：Only the root user can add, delete, or modify the user's permission
	1.Add or delete users：
	Parameter definition：["add_user" adds a user, "delete_user" deletes a user],  [user name],[password],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]
	Example：
	$res = $gc->user("add_user", "user1", "111111");
	echo $res . PHP_EOL;
	2.Privilege to modify user：
	参数含义：["Add_query" adds query permission, "delete_query" deletes query permission, "add_load" adds load permission, "delete_load" deletes load permission, "add_unload" adds no load permission, "delete_unload" deletes no load permission,  "Add_update" adds update permission, "delete_update" deletes update permission, "add_backup" adds backup permission, "delete_bakup" deletes backup permission, "add_restore" adds restore permission,  "Delete_restore" deletes restore permission,"add_export" adds export permission," delete_export" deletes export permission],  [user name],[database name],[Request types "GET" and "POST" can be omitted if the Request type is "GET"]]
	Example：
	$res = $gc->user("add_user", "user1", "lubm");
	echo $res . PHP_EOL;

**Display user：showUser**

	function showUser($request_type)
	Function：Display all users
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$res = $gc->showUser();
	echo $res. PHP_EOL;

**Query database：query**

	function query($db_name, $format, $sparql, $request_type)
	Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$res = $gc->query("lubm", "json", $sparql);
	echo $res. PHP_EOL; //output result

**Database deletion：drop**

	function drop($db_name, $is_backup, $request_type)
	Function：Delete the database directly or delete the database while leaving a backup
	Parameter definition：[database name], [false not backup, true backup],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$res = $gc->drop("lubm", false); //Delete the database without leaving a backup
	echo $res. PHP_EOL;             

**Database monitor: monitor**  	

	function monitor($db_name, $request_type)Function：Displays information for a specific database 
	Parameter definition：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]
	Example：$res = $gc->monitor("lubm");echo $res. PHP_EOL;

**Save database: checkpoint**

	function checkpoint($db_name, $request_type)Function：the Parameter definition of save database if it has been changed：[database name], [request type "GET" and "POST ", if request type "GET" can be omitted]Example：$res = $gc->checkpoint("lubm");echo $res. PHP_EOL;

**Display database: show**

	function show($request_type)Function：Displays all created databases
	Parameter definition：[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：$res = $gc->show();echo $res. PHP_EOL;

**Display kernel version information: getCoreVersion**

	function getCoreVersion($request_type)Function：get kernel version information
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：$res = $gc->getCoreVersion();echo $res. PHP_EOL;

**Display API version: getAPIVersion**	

	function getAPIVersion($request_type)
	Function：Get API version	
	Parameter definition: [Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：
	$res = $gc->getAPIVersion();
	echo $res. PHP_EOL;

**Query the database and save the file：fquery**

	function fquery($db_name, $format, $sparql, $filename, $request_type)
	Function：Query the database and save the results to a file
	Parameter definition：[database name], [query result type JSON, HTML or text], [SPARQL statement], [file name],[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：$gc->fquery("lubm", "json", $sparql, "ans.txt");

**Export database**

	function exportDB($db_name, $dir_path, $request_type)
	Function：Export the database to a folder
	Parameter definition：[database name], [database export folder path]，[Request types "GET" and "POST" can be omitted if the request type is "GET"]
	Example：$res = $gc->exportDB("lubm", "/root/gStore/");

<div STYLE="page-break-after: always;"></div>

### 5.10 gServer API instruction

---

#### 5.10.1 API Interconnection Mode

> The gServer interface uses the `socket` protocol and supports multiple ways to access the interface. If the port `9000` is started from the gServer in the Main directory, the contents of the interface interconnection are as follows:
>
> API address：
>
> ```json
> http://ip:9000/
> ```
>
> The API supports the input of a parameter list in JSON format, as shown below：
>
> ```json
> {"op": "[op_type]", "[paramname1]": "[paramvalue1]", "[paramname2]": "[paramvalue2]"……}
> ```
>
> 



#### 5.10.2 API List

| API name | Definition                     | Note                                                    |
| -------- | ------------------------------ | ------------------------------------------------------- |
| build    | Build graph database           | The database file must be locally stored on the server  |
| load     | Load graph database            | Load the database into memory                           |
| unload   | Unload graph database          | Unload the database from memory                         |
| drop     | Delete graph database          | Logical deletion and physical deletion can be performed |
| show     | Display graph database         | Display list of all databases                           |
| query    | Query graph database           | Including query, delete, and insert                     |
| stop     | Close server                   | Only root user root can perform this operation          |
| close    | Close client server connection | Process client connection closure requests              |
| login    | login to database              | authenticate user names and password                    |
|          |                                |                                                         |

<div STYLE="page-break-after: always;"></div>

#### 5.10.3 API specific instruction

> This section describes the input and output parameters of each interface. Assume that the IP address of the gserver is 127.0.0.1 and the port is 9000

##### 5.10.3.1 build- build database

###### Brief description

- Create a database based on existing NT file
- Files must exist on the gStore server

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                                         |
| :------------- | :-------- | :----- | ------------------------------------------------------------ |
| op             | yes       | string | Operation name, fixed value is**build**                      |
| db_name        | yes       | string | Database name (.db is not required)                          |
| db_path        | yes       | string | Database file path (can be an absolute path or a relative path. The relative path uses the gStore installation root directory as a reference directory) |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Import RDF file to database done."
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.10.3.2 load

###### Brief description

- To load a database into memory, a load operation is a prerequisite for many operations, such as Query

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**load** |
| db_name        | yes       | string | Database name (.db is not required)    |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Load database successfully."
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.10.3.3 unload

###### Brief description

- Unmount the database from memory (all changes are flushed back to hard disk)

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                     |
| :------------- | :-------- | :----- | ---------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**unload** |
| db_name        | yes       | string | Database name (.db is not required)      |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Unload database done."
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.10.3.4 drop

###### Brief description

- Delete the database

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**drop** |
| db_name        | yes       | string | Database name (.db is not required)    |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Drop database done."
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.10.3.5 show

###### Brief description

- Display all database list

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**show** |

###### Return value

| Parameter name    | Type      | Note                                                         |
| :---------------- | :-------- | ------------------------------------------------------------ |
| StatusCode        | int       | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg         | string    | Return specific information                                  |
| ResponseBody      | JSONArray | JSON arrays (each of which is a database information0        |
| -------- database | string    | database name                                                |
| ---------status   | string    | database status                                              |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "success",
    "ResponseBody": [
           "lubm": "loaded",
           "lubm10K": "unloaded"
    ]
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.10.3.6 query

###### Brief description

- query database

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                            |
| :------------- | :-------- | :----- | ----------------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**query**         |
| db_name        | yes       | string | database that need operations                   |
| format         | no        | string | The result set return format is json by default |
| sparql         | yes       | string | The SPARQL statement to execute                 |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |
| head           | JSON   | Head information                                             |
| results        | JSON   | Result information (see Return Sample for details)           |


###### Return sample 

``` json
{
    "head": {
        "link": [],
        "vars": [
            "x"
        ]
    },
    "results": {
        "bindings": [
            {
                "x": {
                    "type": "uri",
                    "value": "十面埋伏"
                }
            },
            {
                "x": {
                    "type": "uri",
                    "value": "投名状"
                }
            },
            {
                "x": {
                    "type": "uri",
                    "value": "如花"
                }
            }
        ]
    },
    "StatusCode": 0,
    "StatusMsg": "success"
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.10.3.7 login

###### Brief description

- Login user (verify username and password)

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**login** |
| username       | yes       | string | user name                               |
| password       | yes       | string | Password (plain text)                   |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 1001,
    "StatusMsg": "wrong password."
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.10.3.8 stop

###### Brief description

- Close server

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                   |
| :------------- | :-------- | :----- | -------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**stop** |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Server stopped."
}
```

<div STYLE="page-break-after: always;"></div>

##### 5.10.3.9 close

###### Brief description

- Close the connection to the client

###### Request ip

- ` 127.0.0.1 `

###### Request port number

- ` 9000 `

###### Parameter transfer mode

- Pass it as a `JSON` structure

###### Parameter

| Parameter name | Mandatory | Type   | Note                                    |
| :------------- | :-------- | :----- | --------------------------------------- |
| op             | yes       | string | Operation name, fixed value is**close** |

###### Return value

| Parameter name | Type   | Note                                                         |
| :------------- | :----- | ------------------------------------------------------------ |
| StatusCode     | int    | Return value code value (refer to Appendix: Return value code table for details) |
| StatusMsg      | string | Return specific information                                  |


###### Return sample 

``` json
{
    "StatusCode": 0,
    "StatusMsg": "Connection disconnected."
}
```

<div STYLE="page-break-after: always;"></div>

##### Appendix 1 return value code table

| Code value | Definition                                  |
| ---------- | ------------------------------------------- |
| 0          | Success                                     |
| 1000       | The method type is  not support             |
| 1001       | Authentication Failed                       |
| 1002       | Check Privilege  Failed                     |
| 1003       | Param is illegal                            |
| 1004       | The operation conditions  are not satisfied |
| 1005       | Operation failed                            |
| 1006       | Add privilege Failed                        |
| 1007       | Loss of lock                                |
| 1008       | Transcation manage Failed                   |
| 1100       | The operation is  not defined               |
| 1101       | IP Blocked                                  |
|            |                                             |



<div STYLE="page-break-after: always;"></div>

<div STYLE="page-break-after: always;"></div>

## 6. SPARQL query syntax

### 6.1 Graph Patterns

---

This document mainly refer to the  [SPARQL 1.1 标准文档](https://www.w3.org/TR/sparql11-query/), but also increased the gStore own customized content, if you want to learn more about gStore SPARQL statement of support, Please read our documentation carefully!  

Unless otherwise specified, this document will continue to use the following RDF data instances as objects for queries:  

```
<Yifei Liu> <name> "Yifei Liu" .
<Yifei Liu> <name> "Crystal Liu" .
<Yifei Liu> <gender> "female" .
<Yifei Liu> <constellation> "Virgo" .
<Yifei Liu> <occupation> "actor" .

<Zhiying Lin> <name> "Zhiying Lin" .
<Zhiying Lin> <gender> "male" .
<Zhiying Lin> <occupation> "actor" .
<Zhiying Lin> <occupation> "director" .

<Jun Hu> <name> "Jun Hu" .
<Jun Hu> <gender> "male" .
<Jun Hu> <constellation> "pisces" .
<Jun Hu> <occupation> "actor" .
<Jun Hu> <occupation> "voice actors" .
<Jun Hu> <occupation> "producer" .
<Jun Hu> <occupation> "director" .

<Tianlong Babu> <featured> <Zhiying Lin> .
<Tianlong Babu> <featured> <Yifei Liu> .
<Tianlong Babu> <featured> <Jun Hu> .
<Tianlong Babu> <type> <action movie> .
<Tianlong Babu> <type> <Costume Films> .
<Tianlong Babu> <type> <romantic movie> .
<Tianlong Babu> <score> "8.3"^^<http://www.w3.org/2001/XMLSchema#float> .
<Tianlong Babu> <release time> "2003-12-11T00:00:00"^^<http://www.w3.org/2001/XMLSchema#dateTime> .

<The Love Winner> <featured> <Zhiying Lin> .
<The Love Winner> <featured> <Yifei Liu> .
<The Love Winner> <type> <romantic movie> .
<The Love Winner> <type> <feature film> .
<The Love Winner> <score> "6.1"^^<http://www.w3.org/2001/XMLSchema#float> .
<The Love Winner> <release time> "2004-11-30T00:00:00"^^<http://www.w3.org/2001/XMLSchema#dateTime> .
```

Since there is no official Chinese translation of the SPARQL 1.1 standard document, the English version of the term will be indicated when it first appears in the following paragraphs.

By standard, **keywords in SPARQL queries are case insensitive**

<br/>

####  6.1.1 The simplest graph mode

Let's start with the simplest query：

```sparql
SELECT ?movie
WHERE
{
	?movie <featured> <Yifei Liu> .
}
```

The query consists of two parts: the **SELECT statement ** specifies the variables that need to output the query results, and the **WHERE statement ** provides the graph pattern used to match the data graph. In the above query, the graph pattern consists of a single **triplet** `?movie <featured> <Yifei Liu>` `?movie` is **variable** , and `< featured >` as predicate and`<Yifei Liu > `as object are **IRI** (International Resource Identifier). This query will return all movies and TV works starring Yifei Liui. The results run on the sample data are as follows:

| ?movie              |
| ------------------- |
| \<Tianlong Babu\>   |
| \<The Love Winner\> |

The subject, predicate and object of a triple can all be IRI. Objects can also be **RDF literals **. The following query will give all the people in the sample data whose profession is director：

```sparql
SELECT ?person
WHERE
{
	?person <occupation> "director" .
}
```

Where 'director' is an RDF literal

Result are as follows：

| ?person         |
| --------------- |
| \<Jun Hu\>      |
| \<Zhiying Lin\> |

Under the current version of gStore, RDF literals with data types are queried with suffixes corresponding to those in the data file. For example, the following query will give a douban rating of 8.3:

```sparql
SELECT ?movie
WHERE
{
	?movie <score> "8.3"^^<http://www.w3.org/2001/XMLSchema#float> .
}
```

Results are as follows：

| ?movie            |
| ----------------- |
| \<Tianlong Babu\> |

Other common data types include `<http://www.w3.org/2001/XMLSchema#integer>`（integer），`<http://www.w3.org/2001/XMLSchema#decimal>`（point type），`xsd:double`（A double-precision floating point type），`<http://www.w3.org/2001/XMLSchema#string>`（String type），`<http://www.w3.org/2001/XMLSchema#boolean>`（Boolean），`<http://www.w3.org/2001/XMLSchema#dateTime>`（Date/time).Other data types may also appear in data files, simply using the form `^^< data type suffix >` in the query.

#### 6.1.2 Basic Graph Pattern

**Base graph pattern** is a collection of triples; The two queries in the previous section both have only the outermost braces and therefore belong to the **basic graph mode**; Enclosing the outermost braces is a single base graph pattern **Group Graph Pattern**。

The basic graph pattern in the two queries in the previous section consists of a single triple. The following query uses a basic graph pattern consisting of multiple triples to give all male leads of Tianlongba in the sample data:

```sparql
SELECT ?person
WHERE
{
	<Tianlong Babu> <featured> ?person .
	?person <gender> "male" .
}
```

Result are as follows：

| ?person        |
| -------------- |
| \<Jun Hu>      |
| \<Zhiying Lin> |

####  6.1.3 Group Graph Pattern

**Group graph patterns** are separated by paired braces. A group graph pattern can be composed of a single base graph pattern, as described in the previous section, or multiple subgroups of graph patterns nested with the following operations: **OPTIONAL**, **UNION**, and **MINUS**. **FILTER** filters the results within the range of a group graph pattern

##### OPTIONAL

The keyword OPTIONAL uses the following syntax:

```
pattern1 OPTIONAL { pattern2 }
```

The result of the query must match `pattern1` and selectively match `pattern2`. `Pattern2` is known as the OPTIONAL graph pattern. If there is a match for `pattern2`, add it to the match for `Pattern1`; Otherwise, the match for `pattern1`is still printed. For this reason, OPTIONAL is often used when some data is missing.   

The following query gives the gender and constellation information of the person in the sample data. Among them, as long as there is gender information of the character will be returned, regardless of whether there is the constellation information of the character; If both exist, additional returns are returned

```sparql
SELECT ?person ?gender ?horoscope
WHERE
{
	?person <gender> ?gender .
	OPTIONAL
	{
		?person <constellation> ?horoscope .
	}
}
```

Results are as follows：

| ?person        | ?gender  | ?horoscope |
| -------------- | -------- | ---------- |
| \<Yifei Liu>   | "female" | "Virgo"    |
| \<Zhiying Lin> | "male"   |            |
| \<Jun Hu>      | "male"   | "pisces"   |

##### UNION

The keyword UNION is syntactically similar to OPTIONAL. In a graph pattern joined by UNION, as long as there is one that matches a piece of data, that data matches the whole joined by UNION. Therefore, UNION can be understood as finding the set of matching results of each graph pattern it joins (actually using multiple set semantics because it allows repeating results).   

The following query gives the sample data for films and television works whose category is costume film or drama film：

```sparql
SELECT ?movie
WHERE
{
	{?movie <type> <Costume Films> .}
	UNION
	{?movie <type> <feature film> .}
}
```

Results are as follows：

| ?movie             |
| ------------------ |
| \<Tianlong Babu>   |
| \<The Love Winner> |

#####  MINUS

The usage syntax of the keywords MINUS is similar to OPTIONAL and UNION. MINUS The matching of the left and right graph patterns will be calculated, and the part that can match the right graph pattern will be removed from the matching result of the left graph pattern as the final result. Therefore, MINUS can be understood as the difference of the matching result set of the two graph patterns connected to it (the left is the subtracted set, multiple set semantics).   

The following query will give the sample data of the characters who starred in The Dragon Eight but did not star in the love winner：

```sparql
SELECT ?person
WHERE
{
	<Tianlong Babu> <featured> ?person .
	MINUS
	{<The Love Winner> <featured> ?person .}
}
```

Results are as follows：

| ?person   |
| --------- |
| \<Jun Hu> |

##### FILTER

The keyword FILTER is followed by a constraint condition, and the results that do not meet this condition in the current pattern group will be filtered out and not returned. FILTER conditions can use equations, inequalities, and various built-in functions.  

The following query will give the film and television works with douban score higher than 8 points in the sample data:

```sparql
SELECT ?movie
WHERE
{
	?movie <score> ?score .
	FILTER (?score > "8"^^<http://www.w3.org/2001/XMLSchema#float>)
}
```

Results are as follows ：

| ?movie           |
| ---------------- |
| \<Tianlong Babu> |

No matter where a FILTER is placed in a group graph pattern, as long as it remains in the same nesting layer, its semantics remain unchanged and the scope of the constraint remains the current group graph pattern. For example, the following query is equivalent to the previous one：

```sparql
SELECT ?movie
WHERE
{
	FILTER (?score > "8"^^<http://www.w3.org/2001/XMLSchema#float>)
	?movie <score> ?score .
}
```

One of the built-in functions commonly used for FILTER conditions is the regular expression **REGEX**. The following query gives the liu surname in the sample data：

```sparql
SELECT ?person
WHERE
{
	?person <name> ?name .
	FILTER REGEX(?name, Yi.*")
}
```

Results are as follows：

| ?person      |
| ------------ |
| \<Yifei Liu> |

<br/>

### 6.2 Assignment

The following keywords belong to assignment functions and can be used to define variables in the query body or provide inline data.

#### 6.2.1 BIND: assigning to a variable

```
BIND(value, name)
```

##### Parameter

`value`: a string value
`name`: name of a variable that has not yet appeared in the query

##### Sample

Query people Francis or Alice doesn't like and categorize the tags in the results returned:

```sql
SELECT ?a ?x WHERE
{
	{
		BIND("Francis" as ?a).
		<Francis> <dislike> ?x .
	}
	UNION
	{
		BIND("Alice" as ?a).
		<Alice> <dislike> ?x.
	}
}
```

The result is as follows: (For the convenience of reading, the escaping of the outermost double quotes and the inner double quotes of the string is omitted)

```json
{
	"bindings": [
		{
			"a": {"type": "literal", "value": "Francis"},
			"x": {"type": "uri", "value": "Eve"}
		},
		{
			"a": {"type": "literal", "value": "Francis"},
			"x": {"type": "uri", "value": "Dave"}
		},
		{
			"a": {"type": "literal", "value": "Alice"},
			"x": {"type": "uri", "value": "Eve"}
		}
	]
}
```

Further improvements will be made to BIND expressions/functions, such as support for binding an entity  (IRI) to a variable.

### 6.3 Aggregates

---

Aggregate functions are used in SELECT statements with the following syntax:

```sparql
SELECT (AGGREGATE_NAME(?x) AS ?y)
WHERE
{
	...
}
```

Where `AGGREGATE_NAME` is the name of the aggregation function, variable `?X`is the aggregate function on the object, variable`?Y` is the column name of the aggregate function value in the final result.  

The aggregate function acts on each group of outcomes. All results are returned as a set by default. The aggregation functions supported by gStore are as follows

##### COUNT

Aggregate function for counting.  

The following query gives the number of actors in the sample data：

```sparql
SELECT (COUNT(?person) AS ?count_person)
WHERE
{
	?person <occupation> "actor" .
}
```

Results are as follows：

| ?count_person                                    |
| ------------------------------------------------ |
| "3"^^\<http://www.w3.org/2001/XMLSchema#integer> |

##### SUM

Aggregate function for summation.  

The following query will give the sum of douban ratings for all movies in the sample data:

```sparql
SELECT (SUM(?score) AS ?sum_score)
WHERE
{
	?movie <score> ?score .
}
```

Results are as follows：

| ?sum_score                                            |
| ----------------------------------------------------- |
| "14.400000"^^<http://www.w3.org/2001/XMLSchema#float> |

#####  AVG

An aggregate function for averaging.  

The following query gives the average Douban score for all movies in the sample data:

```sparql
SELECT (AVG(?score) AS ?avg_score)
WHERE
{
	?movie <score> ?score .
}
```

Results are as follows：

| ?avg_score                                           |
| ---------------------------------------------------- |
| "7.200000"^^<http://www.w3.org/2001/XMLSchema#float> |

##### MIN

An aggregate function for minimizing.  

The following query will give the lowest Douban score for all movies in the sample data:

```sparql
SELECT (MIN(?score) AS ?min_score)
WHERE
{
	?movie <score> ?score .
}
```

Results are as follows：

| ?min_score                                      |
| ----------------------------------------------- |
| "6.1"^^<http://www.w3.org/2001/XMLSchema#float> |

##### MAX

Aggregate function used to find the maximum value.  

The following query will give the highest Douban ratings for all movies in the sample data:

```sparql
SELECT (MAX(?score) AS ?max_score)
WHERE
{
	?movie <score> ?score .
}
```

Results are as follows：

| ?max_score                                      |
| ----------------------------------------------- |
| "8.3"^^<http://www.w3.org/2001/XMLSchema#float> |

##### GROUP BY

If you want to GROUP the results BY the value of a variable, you can use the keyword GROUP BY. For example, the following query gives all occupations and their corresponding numbers in the sample data:

```sparql
SELECT ?occupation (COUNT(?person) AS ?count_person)
WHERE
{
	?person <occupation> ?occupation .
}
GROUP BY ?occupation
```

Results are as follows：

| ?occupation    | ?count_person                                    |
| -------------- | ------------------------------------------------ |
| "actor"        | "3"^^\<http://www.w3.org/2001/XMLSchema#integer> |
| "director"     | "2"^^\<http://www.w3.org/2001/XMLSchema#integer> |
| "voice actors" | "1"^^\<http://www.w3.org/2001/XMLSchema#integer> |
| "producer"     | "1"^^\<http://www.w3.org/2001/XMLSchema#integer> |

<br/>



### 6.4 Solution Sequences and Modifiers

The following keywords are result sequence modifiers that post process the query results to form the final returned results.

#####  DISTINCT: Removes duplicate results  

A query with a SELECT statement without the keyword DISTINCT will retain duplicate results in the final result. For example, the following query gives all the occupations in the sample data

```sparql
SELECT ?occupation
WHERE
{
	?person <occupation> ?occupation .
}
```

Results are as follows：

| ?occupation    |
| -------------- |
| "actor"        |
| "actor"        |
| "actor"        |
| "director"     |
| "director"     |
| "producer"     |
| "voice actors" |

If you want to see DISTINCT job categories, you can add the keyword DISTINCT in the SELECT statement:

```sparql
SELECT DISTINCT ?occupation
WHERE
{
	?person <occupation> ?occupation .
}
```

Results are as follows：

| ?occupation    |
| -------------- |
| "actor"        |
| "director"     |
| "producer"     |
| "voice actors" |

DISTINCT can also be used in the aggregate function COUNT. The following query gives the number of occupations in the sample data:

```sparql
SELECT (COUNT(DISTINCT ?occupation) AS ?count_occupation)
WHERE
{
	?person <occupation> ?occupation .
}
```

Results are as follows：

| ?count_occupation                                |
| ------------------------------------------------ |
| "4"^^\<http://www.w3.org/2001/XMLSchema#integer> |

##### ORDER BY: 排序

Query results are unordered by default. If you want to sort the results based on the values of some variables, you can add an ORDER BY statement after the WHERE statement. For example, the following query will sort the film and television works in the sample data according to douban score. If the order is not specified, it will be in ascending order by default

```sparql
SELECT ?movie ?score
WHERE
{
	?movie <score> ?score
}
ORDER BY ?score
```

Results are as follows：

| ?movie             | ?score                                           |
| ------------------ | ------------------------------------------------ |
| \<The Love Winner> | "6.1"^^\<http://www.w3.org/2001/XMLSchema#float> |
| \<Tianlong Babu>   | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |

If you want to sort in descending order, you need to modify the variable name with the keyword DESC:

```sparql
SELECT ?movie ?score
WHERE
{
	?movie <score> ?score
}
ORDER BY DESC(?score)
```

Results are as follows：

| ?movie             | ?score                                           |
| ------------------ | ------------------------------------------------ |
| \<Tianlong Babu>   | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |
| \<The Love Winner> | "6.1"^^\<http://www.w3.org/2001/XMLSchema#float> |

The ORDER BY statement can contain multiple space-separated variables, each of which can be decorated with DESC. gStore does not currently support the use of four-operation expressions and built-in functions in ORDER BY statements.  

#####  OFFSET: skips a certain number of results  

The OFFSET statement follows the WHERE statement and has the following syntax：

```sparql
OFFSET nonnegative_integer
```

`nonnegative_INTEGER` must be a non-negative integer, indicating the number of results to be skipped. `OFFSET 0` is syntactic but has no effect on the result. Because the query results are unordered by default, SPARQL semantics do not guarantee that the skipped results meet any deterministic conditions. Therefore, the OFFSET statement is typically used in conjunction with the ORDER BY statement。

The following query sorts the film and television works in the sample data by douban score from lowest to highest, and skips the film and television works with the lowest score:

```sparql
SELECT ?movie ?score
WHERE
{
	?movie <score> ?score .
}
ORDER BY ?score
OFFSET 1
```

Results are as follows：

| ?movie           | ?score                                           |
| ---------------- | ------------------------------------------------ |
| \<Tianlong Babu> | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |

##### LIMIT: Limit the number of results

The syntax of the LIMIT statement is similar to that of the OFFSET statement:

```sparql
LIMIT nonnegative_integer
```

`nonnegative_INTEGER` must be a non-negative integer, indicating the maximum number of results allowed. Similar to OFFSET, the LIMIT statement is typically used in conjunction with the ORDER BY statement because the query result defaults to unordered.  

The following query gives the film and television works with the highest douban score in the sample data:

```sparql
SELECT ?movie ?scoreWHERE{	?movie <score> ?score .}ORDER BY DESC(?score)LIMIT 1
```

Results are as follows：

| ?movie           | ?score                                           |
| ---------------- | ------------------------------------------------ |
| \<Tianlong Babu> | "8.3"^^\<http://www.w3.org/2001/XMLSchema#float> |

<br/>



### 6.5 Update graph

---

By **INSERT DATA**, **DELETE DATA**, and **DELETE WHERE** queries, we can INSERT or DELETE triples from the database.

##### INSERT DATA

INSERT DATA is used to INSERT triples into a database. The syntax is similar to that of a SELECT query, except that there are no variables in the triples that make up the group graph pattern.

The following query inserts the relevant information of the film and TELEVISION works Chinese Paladin into the sample data：

```sparql
INSERT DATA
{
	<Paladin> <featured> <Ge Hu> .
	<Paladin> <featured> <Yifei Liu> .
	<Paladin> <type> <action movie> .
	<Paladin> <type> <Costume Films> .
	<Paladin> <type> <romantic movie> .
	<Paladin> <score> "8.9"^^<http://www.w3.org/2001/XMLSchema#float> .
}
```

The query that appears in the "Graph pattern - The simplest Graph pattern" section

```sparql
SELECT ?movie
WHERE
{
	?movie <featured> <Yifei Liu> .
}
```

After inserting the above data, the result becomes:

| ?movie             |
| ------------------ |
| \<Tianlong Babu>   |
| \<The Love Winner> |
| \<Paladin>         |

##### DELETE DATA

DELETE DATA is used to DELETE triples from a database. The usage is exactly similar to INSERT DATA.

##### DELETE WHERE

DELETE DATA is used to DELETE eligible triples from the database; In contrast to DELETE DATA, its WHERE statement is exactly the same as the WHERE statement of a SELECT query, meaning that variables are allowed in triples. For example, the following query removes all information about swordsman films from the sample data:

```sparql
DELETE WHERE{	?movie <type> <action movie> .	?movie ?y ?z .}
```

Run the query that appeared in the "Graph patterns - The simplest Graph patterns" section again:

```sparql
SELECT ?movie
WHERE
{
	?movie <featured> <Yifei Liu> .
}
```

Result change to：

| ?movie             |
| ------------------ |
| \<The Love Winner> |

<br/>



### 6.6 Advanced functions

---

In **kernel version V0.9.1**, gStore has added a number of queries related to the path and centrality of nodes in the data graph, including loop query, shortest path query, K-hop reachable query and Personalized PageRank query.

When using advanced functions, you need to load the CSR resources. When starting the HTTP API service, you need to add the parameter `-c 1`. Please see [Quick Start] - [Common API] - [HTTP API service] for details.  

#### 6.6.1 Sample data

To better demonstrate the advanced functionality, use the following social relationship data as sample data：

```
<Alice> <focus on> <Bob> .
<Alice> <like> <Bob> .
<Alice> <dislike> <Eve> .
<Bob> <focus on> <Alice> .
<Bob> <like> <Eve> .
<Carol> <focus on> <Bob> .
<Carol> <like> <Bob> .
<Carol> <dislike> <Francis> .
<Dave> <focus on> <Alice> .
<Dave> <focus on> <Eve> .
<Dave> <dislike> <Francis> .
<Eve> <like> <Carol> .
<Francis> <like> <Carol> .
<Francis> <dislike> <Dave> .
<Francis> <dislike> <Eve> .
```

The above data are illustrated below:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE.png" alt="社交图" style="zoom:50%;" />

Unless otherwise specified, functions that return paths represent a path/a ring/a subgraph in JSON format as follows:

```JSON
{
    "src": "<src_IRI>", "dst": "<dst_IRI>",
  	"edges": [
    	{ "fromNode": 0, "toNode": 1, "predIRI": "<pred>" }
    ],
  	"nodes": [
        { "nodeIndex": 0, "nodeIRI": "<src_IRI>" },
        { "nodeIndex": 1, "nodeIRI": "<dst_IRI>" }
    ]
}
```

The final return value represents a set of paths/rings/subgraphs as follows :(where the 'paths' element has the format above)

```JSON
{ "paths": [{...}, {...}, ...] }
```

#### 6.6.2 Path-related query

##### 6.6.2.1 The cycle detection query

Queries for the existence of a cycle containing nodes ` u `and `v`

```
cyclePath(u, v, directed, pred_set)
cycleBoolean(u, v, directed, pred_set)
```

Used in SELECT statements, using the same syntax as aggregate functions

Parameter

`u`, `v` ：Variable or node IRI

`directed` ：A Boolean value, true for directed and false for undirected (all edges in the graph are considered bidirectional

`pred_set` ：The set of predicates that make up the edges of a ring. If set to null '{}', all predicates in the data are allowed

###### Return value

- `cyclePath` ：Returns a ring containing the nodes`u` and`v `(if any) in JSON form. If `u` or `v` is a variable, a loop is returned for each set of valid values of the variable
- `cycleBoolean` ：Return true if there is a ring containing nodes `u `and `v`; Otherwise, return false

The following query asks if there is a directed ring that contains Carol, a person Francis dislikes (Dave or Eve in the sample data), and whose edges can only be marked by a "like" relationship：

```sparql
select (cycleBoolean(?x, <Carol>, true, {<like>}) as ?y)where{	<Francis> <dislike> ?x .}
```

Results are as follows：

| ?y                                           |
| -------------------------------------------- |
| "true"^^\<http://www.w3.org/2001/XMLSchema#> |

If you want to output a loop that meets the above criteria, use the following query:

```sparql
SELECT (cyclePath(?x, <Carol>, true, {<like>}) as ?y)
WHERE
{
	<Francis> <dislike> ?x .
}
```

Results are as follows，It can be seen that one of the rings satisfying the condition is formed by Eve likes carol-carol likes bob-bob likes Eve in sequence :(the escape of the outermost double quotation marks and the inner double quotation marks are omitted for ease of reading)

```json
{
	"paths":[{
    "src":"<Eve>",
    "dst":"<Carol>",
    "edges":
    [{"fromNode":2,"toNode":3,"predIRI":"<like>"},{"fromNode":3,"toNode":1,"predIRI":"<like>"},{"fromNode":1,"toNode":2,"predIRI":"<like>"}],
    "nodes":
    [{"nodeIndex":1,"nodeIRI":"<Bob>"},{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":2,"nodeIRI":"<Eve>"}]
	}]
}
```

The red part below is the ring:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE2.png" alt="社交图2" style="zoom:50%;" />

#####  6.6.2.2 Shortest path Query

Query the shortest path from node `u` to node `v`

```
shortestPath(u, v, directed, pred_set)
shortestPathLen(u, v, directed, pred_set)
```

Used in SELECT statements, using the same syntax as aggregate function.

###### Parameter

`u` , `v` ：Variable or node IRI

`directed` ：Boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional)

`pred_set` ：The set of predicates that are allowed to occur on the side that makes up the shortest path. If set to null '{}', all predicates in the data are allowed

###### Return value

- `shortestPath` ：Returns a shortest path (if reachable) from node `u` to `v` in JSON form. If `u` or `v` is a variable, a shortest path is returned for each set of valid values of the variable.  
- `shortestPathLen` ：Returns the shortest path length (if reachable) from node `u` to `v`. If `u` or `v` is a variable, return a shortest path length value for each set of valid values of the variable。

The following query returns the shortest path from Francis to a person (Alice in the example data) that Bob likes, cares about, or dislikes, and is not disliked by Francis, with a relationship that can be like or care about.

```sparql
SELECT (shortestPath(<Francis>, ?x, true, {<like>, <focus on>}) AS ?y)WHERE{	<Bob> ?pred ?x .	MINUS { <Francis> <dislike> ?x . }}
```

The red part below is the shortest path:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE3.png" alt="社交图3" style="zoom:50%;" />

Results are as follows：(For easy reading, the outermost double quotation mark and the escape of the inner double quotation mark are omitted.)

```json
{	"paths":[{		"src":"<Francis>",		"dst":"<Alice>",		"edges":		[{"fromNode":4,"toNode":3,"predIRI":"<like>"},{"fromNode":3,"toNode":1,"predIRI":"<like>"},{"fromNode":1,"toNode":0,"predIRI":"<focus on>"}],		"nodes":		[{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"},{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":4,"nodeIRI":"<Francis>"}]		}]}
```

If you want to output only the shortest path length, use the following query:

```sparql
SELECT (shortestPathLen(<Francis>, ?x, true, {<like>, <focus on>}) AS ?y)WHERE{	<Bob> ?pred ?x .	MINUS { <Francis> <dislike> ?x . }}
```

Results are as follows：(For easy reading, the outermost double quotation mark and the escape of the inner double quotation mark are omitted)

```json
{"paths":[{"src":"<Francis>","dst":"<Alice>","length":3}]}
```

#####  6.6.2.3 Reachability/K-hop reachability query

Query whether node `u` is reachable or K-hop reachable to node `v` (i.e., there exists a path with `u` as its source and `v` as its destination whose length does not exceed `k`).

```
kHopReachable(u, v, directed, k, pred_set)kHopReachablePath(u, v, directed, k, pred_set)
```

###### Parameter

`u`, `v` ：Variable or node IRI

`k` ：If it is set to a non-negative integer, it is the upper limit of the path length (query k-hop reachability). If set to negative, query reachability

`directed` ：A Boolean value, true for directed and false for undirected (all edges in the graph are considered bidirectional

`pred_set` ：The set of predicates that are allowed to occur on the side that constitutes a path. If set to null `{}`, all predicates in the data are allowed

###### Return value

- `kHopReachable`：Return true if node `u` is reachable to node `v` (or K hop reachable, depending on the value of parameter `K`); Otherwise, return false. If `u` or `v` is a variable, return a true/false value for each set of valid values of the variable
- `kHopReachablePath`：Returns any path from node `u` to node `v` (if reachable) or a k-hop path, that is, a path with length less than or equal to `K` (if reachable, depending on the value of `K`). If `u` or `v` is a variable, return a path (if reachable) or a k-hop path (if reachable) for each set of valid values of the variable

The following query follows the example query from the previous section, "Shortest path Query" : It starts with Francis and ends with a person that Bob likes, cares about, or dislikes, and is not disliked by Francis (which is Alice in the example data). Ask if the relationship between the two people is 2 hops or within reach through liking or following.

```sparql
SELECT (kHopReachable(<Francis>, ?x, true, 2, {<like>, <focus on>}) AS ?y)WHERE{	<Bob> ?pred ?x .	MINUS { <Francis> <dislike> ?x . }}
```

Since the shortest path length satisfying the condition is known to be 3:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE3-9966466.png" alt="社交图3" style="zoom:50%;" />

Therefore, the above query results are false：

```json
{"paths":[{"src":"<Francis>","dst":"<Alice>","value":"false"}]}
```

Francis and Alice, on the other hand, are reachable, but the shortest path length exceeds the above limit. So a query for reachability (with `k` set to negative) returns true:

```sparql
SELECT (kHopReachable(<Francis>, ?x, true, -1, {<like>, <focus on>}) AS ?y)WHERE{	<Bob> ?pred ?x .	MINUS { <Francis> <dislike> ?x . }}
```

Results are as follows：

```json
{"paths":[{"src":"<Francis>","dst":"<Alice>","value":"true"}]}
```

If you want to return a path that satisfies the condition between two people, you can call the `kHopReachablePath` function：

```SPARQL
SELECT (kHopReachablePath(<Francis>, ?x, true, -1, {<like>, <focus on>}) AS ?y)WHERE{	<Bob> ?pred ?x .	MINUS { <Francis> <dislike> ?x . }}
```

The result may be the shortest path described above：

```json
{	"paths":[{		"src":"<Francis>",		"dst":"<Alice>",		"edges":		[{"fromNode":4,"toNode":3,"predIRI":"<like>"},{"fromNode":3,"toNode":1,"predIRI":"<like>"},{"fromNode":1,"toNode":0,"predIRI":"<focus on>"}],		"nodes":		[{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"},{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":4,"nodeIRI":"<Francis>"}]		}]}
```

It could also be a non-shortest path with a ring in it, as shown in the figure below:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE4.png" alt="社交图4" style="zoom:50%;" />

##### 6.6.2.4 Personalized PageRank

Compute the value of PPR relative to `u`。


```
PPR(u, {v1, v2, ...}, pred_set)
PPR(u, k, pred_set)
```

##### Parameter

`u`：Variable or IRI, representing the source node

`{v1, v2, ...}`：Variable or IRI list, representing the target node. If the list is empty, all nodes in the graph are target nodes by default.

`k`：Integer, indicating that IRI of node with TOP-K PPR value and its corresponding PPR are returned (if the sum of points is less than K, IRI of all node and its corresponding PPR are returned)

`pred_set` ：The set of predicates to be considered (given empty, the default is to consider all predicates

##### Return value

The return value is in the following form, where SRC is the result of IRI or variable query corresponding to u. Which destination nodes DST contains depends on the second argument to the function; The corresponding PPR value is a double precision floating point number.

```
{"paths":
	[
		{"src":"<Francis>", 
			[{"dst":"<Alice>", "PPR":0.1}, {"dst":"<Bob>", "PPR": 0.01}, ...]
		}
	],
	...
}
```

##### Example

```sparql
PREFIX ns:   <http://example.org/ns#>
PREFIX xsd:  <http://www.w3.org/2001/XMLSchema#>

SELECT (PPR(ns:a, 10, {ns:pred2}) AS ?z)
WHERE
{
	?y ns:pred1 ns:b .
}
```

##### 6.6.2.4 kHopEnumeratePath 

Query all K-hop reachable paths from node `u` to node `v`

```c++
kHopEnumerate(u, v, directed, k, pred_set)
```

###### Parameter

`u` , `v`: variable or node IRI
`k`: If set to a non-negative integer, it is the upper limit of the path length (check K-hop reachability); if set to a negative number, check the reachability
`directed`: boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional) 
`pred_set`: The set of predicates that are allowed to occur on the edges that makes up the shortest path. If set to null `{}`, all predicates in the data are allowed.

###### Return value

Returns all paths from node `u` to node `v` (if reachable) or K-hop paths, that is, paths with length less than or equal to `k` (if K-hops are reachable, depending on the value of parameter `k`). If `u` or `v` are variables, return all paths (if reachable) or K-hop paths (if K-hop reachable) for each set of valid values of the variable.

###### Return sample

The query starts with Alice and ends with a person that Francis likes, follows or dislikes, and who is not disliked by Alice (Carol in the example data). Query the paths between two people that are reachable within 3 hops or less through the like or follow relationship.

```sql
SELECT (kHopEnumerate(<Alice>, ?x, true, 3, {<like>, <focus on>}) AS ?y)
WHERE
{
	<Francis> ?pred ?x.
	MINUS { <Alice> <dislike> ?x . }
}
```

The red part of the figure below is the path reachable within 3 hops or less:

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE3.png" alt="社交图3" style="zoom:50%;" />

The result is as follows: (For the convenience of reading, the escaping of the outermost double quotes and the inner double quotes of the string is omitted)

```json
{
	"paths":[{
		"src":"<Alice>",
		"dst":"<Carol>",
		"edges":[{"fromNode":0,"toNode":1,"predIRI":"<like>"},{"fromNode":1,"toNode":2,"predIRI":"<like>"},{"fromNode":2,"toNode":3,"predIRI":"<like>"}],
		"nodes":[{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":2,"nodeIRI":"
<Eve>"},{"nodeIndex":0,"nodeIRI":"<Alice>"},{"nodeIndex":1,"nodeIRI":"<Bob>"}]}, {"src":"<Alice>","dst":"<Carol>","edges":[{"fromNode":0,"toNode":1,"predIRI":"<focus on>"} {"fromNode":1,"toNode":2,"predIRI":"<like>"}, {"fromNode":2,"toNode":3,"predIRI":"<like>"}],"nodes":[{"nodeIndex":3,"nodeIRI":"<Carol>"},{"nodeIndex":2,"nodeIRI":"<Eve>"}, {"nodeIndex":0,"nodeIRI":"<Alice>"}, {"nodeIndex":1,"nodeIRI":"<Bob>"}]}]
}
```



##### 6.6.2.5 bfsCount

The query starts from node u and outputs the number of nodes accessed in different layers in the breadth-first traversal order.

```
bfsCount(u, directed, pred_set)
```

###### Parameter

`u`: variable or node IRI, indicating the source node
`directed`: boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional) 
`pred_set`: The set of predicates that are allowed to occur on the side that makes up the shortest path. If set to null `{}`, all predicates in the data are allowed.

###### Return value

The return value is in the following form, where src is the IRI corresponding to u; depth is the number of layers/height (only accesses u when depth is 0); count is the total number of nodes accessed at the layer, of integer type.

```json
{"paths":
	[
		{"src":"<Alice>",
		"results":[{"depth":0, "count":1}, ...]
		}
	]
}
```

###### Sample

The following query returns the directed breadth-first traversal count with Alice as the source node. The relationship on the edge can be like, follow, or dislike, and the query is:

```json
SELECT(bfsCount(<Alice>,true,{<like>,<focus on>,<dislike>}) AS ?y)
WHERE{}
```

The results are as follows:

```json
{"paths":
	[
		{"src":"<Alice>",
         "results":[{"depth":0, "count":1}, {"depth":1, "count":2}, {"depth":2, "count":1}, {"depth":3, "count":1}, {"depth":4, "count":1}]
		}
	]
}
```

#### 6.6.3 Importance analysis

##### 6.6.3.1 Personalized PageRank

```
PPR(u, hopCnt, pred_set, retNum)
```

The FORA algorithm [1] is invoked to calculate the top-K PPR value relative to u.

[1] S. Wang, R. Yang, X. Xiao, Z. Wei, and Y. Yang, “FORA: Simple and Effective Approximate Single Source Personalized PageRank,” in Proceedings of the 23rd ACM SIGKDD International Conference on Knowledge Discovery and Data Mining, Halifax NS Canada, Aug. 2017, pp. 505–514. doi: 10.1145/3097983.3098072.  

###### Parameter

`u`: variable or node IRI, indicating the source node
`hopCnt`: an integer. When it's -1, PPR can be calculated without limitation. Otherwise, it is restricted to `hopCnt` hops
`pred_set`: The set of predicates that are allowed to occur on the side that makes up the shortest path. If set to null '{}', all predicates in the data are allowed

`retNum`: an integer, indicating that the `retNum` node IRIs with the largest PPR and their corresponding PPR values need to be returned (If the total number of nodes in the graph is fewer than `retNum`, then return all node IRIs and their corresponding PPR values)

###### Return value

The return value is in the following form, where src is u's IRI, or its query result if u is a variable; which destination nodes dst contains depends on the last argument of the function; the PPR value is a double-precision floating point number.

(Note: Since FORA is an approximate algorithm with randomness, it is normal for the return value to be slightly different each time.)

```json
{"paths":
	[
		{"src":"<Francis>", "results":
			[{"dst":"<Alice>", "PPR":0.1}, {"dst":"<Bob>", "PPR": 0.01}, ...]
		}, ...
	]
}
```

###### Sample

Return the three people (and their PPR values) with top-3 PPR values corresponding to all the people Bob likes, follows, or dislikes:

```sql
select (PPR(?x, -1, {}, 3) as ?y)
where
{
	<Bob> ?pred ?x .
}
```

##### 6.6.3.2 closenessCentrality

Query how easy it is for a node to reach other nodes.

```
closenessCentrality(u, directed, pred_set)
```

###### Parameter

`u`: variable or node IRI, indicating the source node
`directed`: boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional) 
`pred_set`: The set of predicates that are allowed to occur on the side that makes up the shortest path. If set to null `{}`, all predicates in the data are allowed.

###### Return value

The return value is in the following form, where src is the IRI corresponding to u; result is the closeness centrality of node u in the graph and is of double-precision floating-point type.

```json
{
	"paths": [
		{
			"src": "<Alice>",
			"result": 0.5
		}
	]
}
```

###### Sample

Example 1. The query returns the closeness centrality of **Alice** in an **undirected graph** (all edges in the graph are regarded as bidirectional), and the relationship between edges can be **like** or **follow**. The SPARQL query is:

```sql
SELECT (closenessCentrality(<Alice>, false, {<like>, <focus on>}) AS ?x) WHERE{}
```

The result is as follows: (For the convenience of reading, the escaping of the outermost double quotes and the inner double quotes of the string is omitted)

```json
{
	"paths":[
		{
			"src": "<Alice>",
			"result": 0.555556
		}
	]
}
```

In the above query, the shortest distance for Alice to reach the other nodes is as follows, and the average distance can be calculated as 1.8, and the value of closeness centrality is 1/1.8 =0.555556, which is consistent with the result.

```json
{
	"Bob" : 1,
	"Dave" : 1,
	"Eve" : 2,
	"Carol" : 2,
	"Francis" : 3
}
```

Example 2. The query returns **Alice**'s closeness centrality in the **directed graph**, and the relationship of edges can be **like** or **follow**. The SPARQL query is:

```sql
SELECT (closenessCentrality(<Alice>, true, {<like>, <focus on>}) AS ?x) WHERE{}
```

The result is as follows: (For the convenience of reading, the escaping of the outermost double quotes and the inner double quotes of the string is omitted)

```json
{
	"paths":[
		{
			"src": "<Alice>",
			"result": 0.500000
		}
	]
}
```

In the above query, the shortest distance for Alice to reach the other nodes is as follows, and the average distance can be calculated as 2, and the value of closeness centrality is 1/2 =0.5, which is consistent with the result.

```json
{
	"Bob" : 1,
	"Eve" : 2,
	"Carol" : 3
}
```

##### 6.6.3.3  triangleCounting

Count the number of triangles in the graph.

```
triangleCounting(directed, pred_set)
```

###### Parameter

`u`: variable or node IRI, indicating the source node
`directed`: boolean value, true for directed, false for undirected (all edges in the graph are considered bidirectional) 
`pred_set`: The set of predicates that are allowed to occur on the side that makes up the shortest path. If set to null `{}`, all predicates in the data are allowed.

###### Return value

The return value is in the following form:

```json
{
	"paths": [2]
}
```

###### Sample

Query the number of directed triangles in the graph, and the edges that form it can only be labeled by the **like** relation. The SPARQL query statement is:

```sql
select (triangleCounting(true, {<like>}) as ?y) where {}
```

<img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/%E7%A4%BE%E4%BA%A4%E5%9B%BE4-9966557.png" alt="社交图4" style="zoom:50%;" />

The result is as follows. The number of directed triangles in the graph whose sides can only be marked by the "like" relation is 1, that is, Bob -> Eve -> Carol-> Bob:

```json
{
	"paths": [1]
}
```

<div STYLE="page-break-after: always;"></div>

<div STYLE="page-break-after: always;"></div>

##  7. gStore Chronology

### Year 2022

- In October, gStore 1.0 version released
- In August, the knowledge graph integrated platform released

### Year 2021

- In November，gStore 0.9.1 version released
- In October，gBuilder 2.0 version released
- In February，gStore products have completed the "Graph Database Basic Ability Test" project of China Academy of Information and Communication Technology；
- In February，gStore's new official website launched；

### Year 2020

- In December, gStore added advanced query functions such as shortest/longest path, K-hop reachable query and loop detection to further enrich the gStore algorithm library;  
- In December, gStore Beta (V0.9) and gStore Stable (V0.8) were officially released on Github and Gitee;  
- In November, gBuilder V0.1 version of knowledge graph automation construction platform was launched;  
- In October, gStore distributed version gMaster was demonstrated in related projects of institute of Computing Technology, Chinese Academy of Sciences;  
- In July, gStore was successfully adapted with TONGxin UOS operating system and domestic CPUS of Kunpeng/Haiguang/Zhaoxin/Feiteng  

### Year 2019


- In December, PKU graphics database system gStore launched China Science and Technology Cloud 2.0;  


- In November, China Software Evaluation Center conducted a performance test on gStore distributed system, and the test results showed that the average query response time of gStore distributed system was 1.79 seconds under the condition of 10.6 billion data storage scale;  


- In October, pKU Graphics database system gStore cloud platform was deployed and launched;  

- In September, THE GRAPH database system was successfully adapted with the domestic "PK "system (Feiteng CPU+ Kirin operating system);  

### Year 2018

- Multi-query Optimization in Federated RDF Systems 23rd International Conferenceon Database Systems for Advanced Applications (DASFAA) BEST PAPER AWARD  


- Related theoretical research work of gStore system "Large-scale Graph structure Data Management", won the second prize of Natural Science of Ministry of Education of China (Zou Lei ranked first)  


- GAnswer system is officially open source on Github with version NUMBER V0.1  

- gAnswer system took part in the Knowledge base Natural language question contest QALD-9 held by eu and won the first prize 

### Year 2017

- The PKUMOD research team has released gStore milestone V0.5 on Github.

### Year 2016

- PKUMOD research team was funded by the Key research and development project "Key Technology and System of Graph Data Management" of Ministry of Science and Technology of China.

### Year 2015 

- gStore code is officially open source on Github, version 0.1 

### Year 2014

- PKUMOD Graph data management related theoretical research "Massive graph structure data storage and query optimization theory research", won the second prize of Natural Science of China Computer Society (Zou Lei ranked first)  


- The first academic paper related to natural language question answering based on knowledge graph was published  

- Lei Zou, Ruizhe Huang, Haixun Wang, Jeffery Xu Yu, Wenqiang He, Dongyan Zhao, Natural Language Question Answering over RDF ---- A Graph Data Driven Approach, SIGMOD 2014  

- PKUMOD research team was supported by the National Natural Science Foundation of China (NSFC) project "Key Technology research on Graph Based Matching Query for Large-scale Heterogeneous Information Network"  

### Year 2011

-  The first academic paper of gStore was published  

- Lei Zou., et al., gStore: Answering SPARQL Queries via Subgraph Matching. PVLDB 4(8): 482-493 (2011)  

- PKUMOD research team was supported by the "Research on Massive RDF Data Storage and query Method based on Graph Database Theory" project of natural Science Foundation of China (NSFC)  



<div STYLE="page-break-after: always;"></div>

## 8. Open source and legal provision



### 8.1 Open Source and Community

---

gStore system from January 2015 open source in the making, comply with the BSD, 3 - Clause open source licenses ` ` https://github.com/pkumod/gStore is open source address; We advocate users to use and modify gStore freely on the premise of respecting the copyright of code authors, develop various industry applications of knowledge graph based on gStore, and promote the healthy and sustainable development of knowledge graph industry software. We encourage users to actively use the gStore system, report problems, make suggestions, and contribute code to the gStore open Source project to join us and make the gStore system better.  

If you have any questions in the process of using gStore, if you would like to tell us your name, organization, purpose of using gStore and email address, we will timely reply to you by sending an email to service@gstore.cn. We guarantee that the privacy of you and your company will not be disclosed, only used to enhance the gStore system itself

### 8.2 Legal Issues

---

The gStore system always adopts the BSD 3-clause which is widely used in the open source community. Under this Agreement, User is free to modify and redistribute the Code, subject to the following terms, and user is free to develop, distribute and sell commercial software based on the gStore code. The specific terms are as follows:

Copyright (c) 2016 gStore team All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

- Neither the name of the Peking University nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

中文条款声明如下（具有同等法律效应）：

版权所有(c) 2016 gStore团队保留所有权利。

在遵守以下条件的前提下，可以以源代码及二进制形式再发布或使用软件，包括进行修改或不进行修改:

- 源代码的再发布必须保持上述版权通知，本条件列表和以下声明。
- 以二进制形式再发布软件时必须在文档和/或发布提供的其他材料中复制上述版权通知，本条件列表和以下声明。
- 未经事先书面批准的情况下，不得利用北京大学或贡献者的名字用于支持或推广该软件的衍生产品。

本软件为版权所有人和贡献者“按现状”为根据提供，不提供任何明确或暗示的保证，包括但不限于本软件针对特定用途的可售性及适用性的暗示保证。在任何情况下，版权所有人或其贡献者均不对因使用本软件而以任何方式产生的任何直接、间接、偶然、特殊、典型或因此而生的损失（包括但不限于采购替换产品或服务；使用价值、数据或利润的损失；或业务中断）而根据任何责任理论，包括合同、严格责任或侵权行为（包括疏忽或其他）承担任何责任，即使在已经提醒可能发生此类损失的情况下。


我们严格要求使用者，在其所发布的基于gStore代码基础上开发的软件和基于gStore的应用软件产品上标有“powered by gStore”和gStore标识（标识参考开发文档中的“gStore标识”）。

<div STYLE="page-break-after: always;"></div>

## 9. gStore Logo

### 9.1 gStore's Logo is as follow：

---

![logo六边形 1](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/logo%E5%85%AD%E8%BE%B9%E5%BD%A2%201.png)



<div STYLE="page-break-after: always;"></div>

### 9.2 Powered by gStore  recommend logo is as follow

---

![powered by](https://gstore-web.oss-cn-zhangjiakou.aliyuncs.com/mdimg/powered%20by.png)

















