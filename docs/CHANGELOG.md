## Nov 4, 2018

Zongyue, Qin fixes several severe bugs in indices, to support the management of freebase and larger datasets.
For example, the bug of multiple repeated insertion/deletion has been solved now.
In addition, trie index is forbidded due to its useless optimization to `id2string` index.

Zhenya, Tian modifies `makefile` to enable multithreading compilation, which has cut down the time of building executables a lot.

Xunbin, Su and Haiqing, Yu collaborate with each other to propose a detailed test standard, as well as several important test scripts including build/query and insert/delete.
Meanwhile, Xunbin, Su and Jing, Li design a `system` database to store the information of databases and users.

---

## Oct 14, 2018

Xunbin, Su adds the function of shutdowning the `ghttp` server. In addition, he addes the `ginit` command to system and prepares the nodejs API.
What is more, `drop` command is supported in `ghttp` by Xunbin, Su.

Chaofan, Yang fixes the bug in the encoder part of `ghttp` api.

Li, Zeng fixes bugs in build process and in the indices module, addes some scripts for demo.

Yinnian, Lin finds a way to scale the contributions of all developers and Li, Zeng adds this function in Makefile.

Yuyan, Chen addes help document to the system, both in Chinese and English.

Jing, Li uses JSON format in response to the request of clients, and addes `system.db` to store information of multiple users.

Wenjie, Li reviews the code of gStore system, and proposes many feasible suggestions for optimization. We have made a plan to optimize the system in all aspects and all effects of the plans will be seen in the next release version.
When the version of release rises to 1.0.0, and that is the time we will call the system a formal release rather than a pre-release.

---

## Sep 20, 2018

Zongyue, Qin fixes the remaining bugs in Trie, ISArray and IVArray.
In addition, he improves the performance of parallism by providing fine-grained locks to key-value indices instead of locking the whole KVstore.

In order to support applications in Wuhan University, Li, Zeng fixes many existing bugs in parallism, 'ghttp', indices, caches(string buffers are not used now) and APIs.
What is more, setup scripts are added for several wellknown Linux distributions(CentOS, Archlinux, Ubuntu) to ease the burden of installing softwares and setting system variables.

As for documents, new figures of architecture and query processing are added by Li, Zeng and Yuyan, Chen.
This will help other developers on Github to view our code and modify it as they wish.

---

## Jun 6, 2018

Xunbin, Su designs a thread pool for 'ghttp' based on the SimpleWeb framework of Boost Library. 
In addition, Xunbin, Sun and imbajin(a Github user) add a docker image(only works for Docker CE 17.06.1) to gStore, which can ease the usage of this database system.

Meanwhile, Zongyue, Qin fixes several bugs in the new indices and designs caches for them.
Chaofan, Yang adds and improves APIs(Application Program Interface) of several programming languages to 'ghttp' interface.

---

## Apr 24, 2018

Multithreading is enabled by zengli in ghttp, to improve the performance of this HTTP web server.

In addition, openmp is added by suxunbin for sort and qsort to excavate hidden performance. However, this optimization does not yield good result.
As a result, we reserve the code of openmp, but still use standard sort and qsort functions.

Lijing adds support for multiple users and databases in ghttp, and improve the functions of web server, as well as the SPARQL query endpoint.

New key-value indices are designed by qinzongyue to take place of the original B+ trees. 
In detail, we think the original ISTree and IVTree are not efficient enough, so we choose to implement array+hash method instead of B+ tree.
What needs to be noticed is that all these indices does not support parallism now, so when multiple queries are running concurrently, we must add locks to ensure that the indices are visited in sequence.
Furthermore, qinzongyue designs a new method to compress the original string in RDF dataset. 
For example, the prefix can be extracted and compressed using some special characters.

Hulin fixes the bugs in preFilter and Join functions, which has no impact on the performance of answering SPARQL queries.

---

## Oct 2, 2017

Bind and GroupBy are supported in SPARQL queries now, and the Join module has been optimized to harvest a big improvement.

In addition, VSTree module is deprecated to save memory and support larger datasets like freebase, which has almost 2.5B triples.
In fact, we have finish experiments on a microbiology dataset with 3.5B triples, and the time of query answering is almost in a scale of 10s.

What's more, we redesign the Database Server using HTTP1.1 protocol and provide REST interface and Java API example now.
Users can visit the server by URL in a browser directly, and we have built several SPARQL endpoints based on it(including freebase, dbpedia, and openKG).

What is not mentioned above is the robutness of Database Server, which supports restart function and query timeout handler(1 hour by default).
Backup function is also included in Database Server now, as well as the REDO function to finish a update query which was interrupted by a system crash.

---

## Jan 10, 2017

preFilter() function in Join module is optimazed using the pre2num structure, as well as the choose_next_node() function.
A global string buffer is added to lower teh cost of getFinalResult(), and the time of answering queries is reduced greatly.

In addition, we assign buffers of different size for all B+ trees.(some of them are more important and more frequently used)
WangLibo merges several B+ trees into one, and the num of all B+ trees are reduced to 9 from 17.
This strategy not only reduces teh space cost, but also reduces the memory cosy, meanwhile speeding up the build process and query process.

What is more, ChenJiaqi has done a lot of work to optimaze the SPARQL query.
For example, some unconnected SPARQL query graphs are dealed specially.

---

## Sep 15, 2016

ZengLi splits the KVstore into 3 parts according to the types of key and value, i.e. int2string, string2int and string2string.
In addition, updates are supported now. 
You can insert, delete or modify some triples in the gStore database.
In fact, only insert() and remove() are implemented, while the modify() are supported by removing first and insert again.

--- 

## Jun 20, 2016

ZengLi has enabled the gStore to answer queries with predicate variables. 
In addition, the structures of many queries have been studied to speed up the query processing.
ChenJiaqi rewrites the sparql query plan to acquire a more efficient one, which brings many benefits to us.

---

## Apr 01, 2016

The structure of this project has changed a lot now. A new join method has been achieved and we use it to replace the old one. The test result shows that speed is improved and the memory cost is lower. We also do some change to Parser/Sparql*, which are all generated by ANTLR. They must be modified because the code is in C, which brings several multiple definition problems, and its size is too large. 

There is a bug in the original Stream module, which brings some control characters to the output, such as ^C, ^V and so on. We have fixed it now and enabled the Stream to sort the output strings(both internal and external). In addition, SPARQL queries which are not BGP(Basic Graph Pattern) are also supported now, using the naive method.

A powerful interactive console, which is named `gconsole` now, is achieved to bring convenience to users. What is more, we use valgrind tools to test our system, and deal with several memory leaks.

The docs and API have also changed, but this is of little importance.

- - -

## Nov 06, 2015

We merge several classes(like Bstr) and adjust the project structure, as well as the debug system. 

In addition, most warnings are removed, except for warnings in Parser module, which is due to the use of ANTLR.

What is more, we change RangeValue module to Stream, and add Stream for ResultSet. We also better the gquery console, so now you can redirect query results to a specified file in the gsql console.

Unable to add Stream for IDlist due to complex operations, but this is not necessary. Realpath is used to supported soft links in the gquery console, but it not works in Gstore.(though works if not in Gstore)

- - -

## Oct 20, 2015

We add a gtest tool for utility, you can use it to query several datasets with their own queries.

In addition, gquery console is improved. Readline lib is used for input instead of fgets, and the gquery console can support commands history, modifying command and commands completion now.

What is more, we found and fix a bug in Database/(a pointer for debugging log is not set to NULL after fclose operation, so if you close one database and open another, the system will fail entirely because the system think that the debugging log is still open)

- - -

## Sep 25, 2015 

We implement the version of B+Tree, and replace the old one.

After testing on DBpedia, LUBM, and WatDiv benchmark, we conclude that the new BTree performs more efficient than
the old version. For the same triple file, the new version spends shorter time on executing gload command.

Besides, the new version can handle the long literal objects efficiently, while triples whose object's length exceeds 4096 bytes result in frequent inefficient split operations on the old version BTree. 

- - -

## Feb 2, 2015

We modify the RDF parser and SPARQL parser.

Under the new RDF parser, we also redesign the encode strategy, which reduces RDF file scanning times.

Now we can parse the standard SPARQL v1.1 grammar correctly, and can support basic graph pattern(BGP) SPARQL queries written by this standard grammar.

- - -

## Dec 11, 2014

We add API for C/CPP and JAVA.

- - -

## Nov 20, 2014

We share our gStore2.0 code as an open-source project under BSD license on github.

