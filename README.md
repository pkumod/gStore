<div align = center><img src="https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/image/logo%201.png" width="300px" /></div>

# gStore- Native graph database system for large-scale knowledge graph application

<font size=5>**The latest version:** </font>

The latest version is 0.9.1, updated on November 25, 2021.

<font size=5>**Help document:** </font>

[gStore帮助手册-0.9.1版本-中文版](http://file.gstore.cn/f/50f350b7bac047688dab/?dl=1)

[gStore User Guide - 0.9.1 Version-English](http://file.gstore.cn/f/d69814aabd2e40fead22/?dl=1)

<font size=5>**Homepage:** </font>

[ http://gstore.cn](http://gstore.cn)

<font size=5>**Product trial:** </font>

[http://cloud.gstore.cn](http://cloud.gstore.cn)

<font size=5>**Discussion group:** </font>

[http://www.gstore.cn/pcsite/index.html#/medical（only chinese）](http://cloud.gstore.cn)

[https://github.com/pkumod/gStore/issues](https://github.com/pkumod/gStore/issues)

<font size=5>**Development team:** </font>

Data Management Laboratory (PKUMOD) of Wangxuan Institute of Computer Technology of Peking University.

<font size=5>**License:** </font>

[ BSD-3-Clause License](https://github.com/pkumod/gStore/blob/0.9.1/LICENSE)

<font size=5>**Contact us:** </font>

Technical problems: gstore@pku.edu.cn , service@gstore.cn

Business issues:gstore@pku.edu.cn , jiuhua.qi@cqbdri.pku.edu.cn



## Introduction

gStore is a graph-based RDF data management system(or what is commonly called a "triple store") that maintains the graph structure of the original [RDF](http://www.w3.org/TR/rdf11-concepts/) data. Its data model is a labeled, directed multi edge graph, where each vertex corresponds to a subject or an object. 

We represent a given [SPARQL](http://www.w3.org/TR/sparql11-overview/) query by a query graph Q. Query processing involves finding subgraph matches of Q over the RDF graph G, instead of joining tables in relational data management system. gStore incorporates an index over the RDF graph (called VS-tree) to speed up query processing. VS-tree is a height balanced tree with a number of associated pruning techniques to speed up subgraph matching.

**(NOTICE: Homomorphism is used here, instead of isomorphism)**



## Fature

The important features of gStore are as follows:

- gStore manages RDF repository from a graph database perspective.
- Support SPARQL 1.1 standard query language (including operation primitives such as UNION, FILTER, OPTIONAL and add, delete and modify).
- gStore supports both query and update efficiently.
- gStore can handle, in a uniform manner, different data types (strings and numerical data) and SPARQL queries with aggregate, range operators.
- Support distributed deployment(Paid support).
- The standalone version supports 5 billion triples.
- The system has independent backup and error recovery functions, providing high reliable performance guarantee.
- Complete support for transaction ACID.



## Publication

The first essay to come up with Gstore System is [gStore_VLDB](http://file.gstore.cn/f/f8dcf8d1476b4982a182/).

- Lei Zou, Jinghui Mo, Lei Chen,M. Tamer Özsu, Dongyan Zhao, [gStore: Answering SPARQL Queries Via Subgraph Matching](http://www.icst.pku.edu.cn/intro/leizou/projects/papers/p482-zou.pdf), Proc. VLDB 4(8): 482-493, 2011.
- Lei Zou, M. Tamer Özsu,Lei Chen, Xuchuan Shen, Ruizhe Huang, Dongyan Zhao, [gStore: A Graph-based SPARQL Query Engine](http://www.icst.pku.edu.cn/intro/leizou/projects/papers/gStoreVLDBJ.pdf), VLDB Journal , 23(4): 565-590, 2014.
- Xuchuan Shen, Lei Zou, M. Tamer Özsu, Lei Chen, Youhuan Li, Shuo Han, Dongyan Zhao, [A Graph-based RDF Triple Store](http://www.icst.pku.edu.cn/intro/leizou/projects/papers/demo.pdf), in Proc. 31st International Conference on Data Engineering (ICDE), 2015; To appear (demo).
- Dong Wang, Lei Zou, Yansong Feng, Xuchuan Shen, Jilei Tian, and Dongyan Zhao, [S-store: An Engine for Large RDF Graph Integrating Spatial Information](http://www.icst.pku.edu.cn/intro/leizou/projects/papers/Store.pdf), in Proc. 18th International Conference on Database Systems for Advanced Applications (DASFAA), pages 31-47, 2013.
- Dong Wang, Lei Zou and Dongyan Zhao, [gst-Store: An Engine for Large RDF Graph Integrating Spatiotemporal Information](http://www.icst.pku.edu.cn/intro/leizou/projects/papers/edbtdemo2014.pdf), in Proc. 17th International Conference on Extending Database Technology (EDBT), pages 652-655, 2014 (demo).
- Lei Zou, Yueguo Chen, [A Survey of Large-Scale RDF Data Management](http://www.icst.pku.edu.cn/intro/leizou/documentation/pdf/2012CCCF.pdf), Comunications of CCCF Vol.8(11): 32-43, 2012 (Invited Paper, in Chinese).



## Change log

**0.9.1（beta）：2021-11-25**

New features in gStore 0.9.1 are listed as follows:

- Decoupling the parsing and execution of queries in kernel, and further improvements on the query performance through optimized join ordering and other techniques. On complex queries, the performance is improved by over 40%.
- Rewriting of the HTTP service component, ghttp, with improved robustness and the addition of functions such as user permission, heartbeat detection, batch import, and batch deletion; API documents are added.
- Implementation of the Personalized PageRank (PPR) extension function, which can be invoked in the SELECT clause to calculate the correlation between entities.
- Support for arithmetic operations (e.g., `?x + ?y = 5`) in the FILTER clause.
- Support for transactional operations, such as begin, tquery (transactional query), commit, and rollback;
- A new executive component, gserver, is added to provide another pathway for remote access of gStore aside from the ghttp component, which implements two-way communication via the socket API.
- Unification of the format of command line arguments of executive components. The `--help` option is uniformly introduced (e.g., `$ bin/gbuild --help` or `$ bin/gbuild -h`), by which users can view the command manual including the meaning of each option.
- A number of bug fixes.

**0.9（beta）：2021-02-10**

New features in version 0.9 include:

- Upgrade of the SPARQL parser generator from ANTLR v3 to the newest, well-documented and well-maintained v4;
- Support for writing numeric literals without datatype suffixes in SPARQL queries;
- Support for arithmetic and logical operators in SELECT clause;
- Support for the aggregates SUM, AVG, MIN and MAX in SELECT clause;
- Additional support for built-in functions functions in FILTERs, including `datatype`, `contains`, `ucase`, `lcase`, `strstarts`, `now`, `year`, `month`, `day`, and `abs`;
- Support for path-related functions as an extension of SPARQL 1.1, including cycle detection, shortest paths and K-hop reachability;
- Support for full & incremental backup and recovery of databases, and automatic full backup can be enabled upon admin configuration;
- Support for log-based rollback opertions;
- Support for transactions with three levels of isolation: *read committed*, *snapshot isolation* and *serializable*;
- Expanding data structures to hold large-scale graphs of up to five billion triples.

The version is a beta version, you can get it by :

```
git clone https://github.com/pkumod/gStore.git
```

**0.8（Stable）**

The version is a stable version ,you can get it by 

```
 git clone -b 0.8 https://github.com/pkumod/gStore.git
```

<!--**You can write your information in [survey](http://59.108.48.38/survey) if you like.**-->



## Getting Started

### <font size=5>**Compile from Source** </font>
This system is really user-friendly and you can pick it up in several minutes. Remember to check your platform where you want to run this system by viewing [System Requirements](docs/DEMAND.md). After all are verified, please get this project's source code. There are several ways to do this:

- (suggested)type `git clone https://github.com/pkumod/gStore.git` in your terminal or use git GUI to acquire it

- download the zip from this repository and extract it

- fork this repository in your github account

Then you need to compile the project, for the first time you need to type `make pre` to prepare the `ANTLR` library and some Lexer/Parser programs.
Later you do not need to type this command again, just use the `make` command in the home directory of gStore, then all executables will be generated.
(For faster compiling speed, use `make -j4` instead, using how many threads is up to your machine)
To check the correctness of the program, please type `make test` command.

The first strategy is suggested to get the source code because you can easily acquire the updates of the code by typing `git pull` in the home directory of gStore repository. 
In addition, you can directly check the version of the code by typing `git log` to see the commit logs.
If you want to use code from other branches instead of master branch, like 'dev' branch, then:

- clone the master branch and type `git checkout dev` in your terminal

- clone the dev branch directly by typing `git clone -b dev`

### Deploy via Docker
You can easily deploy gStore via Docker. We provide both of Dockerfile and docker image. Please see our [Docker Deployment Doc(EN)](docs/DOCKER_DEPLOY_EN.md) or [Docker部署文档(中文)](docs/DOCKER_DEPLOY_CN.md) for details.

### Run
To run gStore, please type `bin/gbuild database_name dataset_path` to build a database named by yourself. And you can use `bin/gquery database_name` command to query an existing database. What is more, `bin/ghttp` is a wonderful tool designed for you, as a database server which can be accessed via HTTP protocol. Notice that all commands should be typed in the root directory of gStore, and your database name should not end with ".db".

- - -



## Performance

**The formal experiment report is in [EXPERIMENT](test/formal_experiment.pdf).**

## Preparation

We have compared the performance of gStore with several other database systems, such as [Jena](http://jena.apache.org/), [Sesame](http://www.rdf4j.org/), [Virtuoso](http://virtuoso.openlinksw.com/) and so on. Contents to be compared are the time to build database, the size of the built database, the time to answer single SPARQL query and the matching case of single query's results. In addition, if the memory cost is very large(>20G), we will record the memory cost when running these database systems.(not accurate, just for your reference)

To ensure all database systems can run correctly on all datasets and queries, the format of datasets must be supported by all database systems and the queries should not contain update operations, aggregate operations and operations related with uncertain predicates. Notice that when measuring the time to answer queries, the time of loading database index should not be included. To ensure this principle, we load the database index first for some database systems, and warm up several times for others.

Datasets used here are WatDiv, Lubm, Bsbm and DBpedia. Some of them are provided by websites, and others are generated by algorithms. Queries are generated by algorithms or written by us.

The experiment environment is a CentOS server, whose memory size is 82G and disk size is 7T. We use [full_test](../test/full_test.sh) to do this test.

## Result

This program produces many logs placed in result.log/, load.log/ and time.log/. You can see that all results of all queries are matched by viewing files in result.log/, and the time cost and space cost of gStore to build database are larger than others by viewing files in load.log/. More precisely, there is an order of magnitude difference between gStore and others in the time/space cost of building database. 

Through analysing time.log/, we can find that gStore behave better than others on very complicated queries(many variables, circles, etc). For other simple queries, there is not much difference between the time of these database systems.

Generally speaking, the memory cost of gStore when answering queries is higher than others. More complicated the query is and more large the dataset is, more apparent the phenomenon is.

You can find more detailed information in [test report](pdf/gstore_test_report.pdf). Notice that some questions in the test report have already be solved now.



## Advanced Help

If you want to understand the details of the gStore system, or you want to try some advanced operations(for example, using the API, server/client), please see the chapters below.

- [How To Use](docs/USAGE.md): detailed information about using the gStore system

- [API Explanation](docs/API.md): guide you to develop applications based on our API

- [Project Structure](docs/STRUCT.md): show the whole structure and process of this project

- - -

    

## Submit questions

Bugs are recorded in [BUG REPORT](docs/BUGS.md).

You are welcome to submit any advice or errors in the Github Issues part of this repository and [official website forum](http://www.gstore.cn/pcsite/index.html#/medical), if not requiring in-time reply. However, if you want to urgent on us to deal with your reports, please email to <gstore@pku.edu.cn>,<jiuhua.qi@cqbdri.pku.edu.cn> to submit your suggestions and report bugs. A full list of our whole team is in [Mailing List](docs/MAIL.md).

We have written a series of short essays addressing recurring challenges in using gStore to realize applications, which are placed in [Recipe Book](docs/TIPS.md).

Sometimes you may find some strange phenomena(but not wrong case), or something hard to understand/solve(don't know how to do next), then do not hesitate to visit the [Frequently Asked Questions](docs/FAQ.md) page.

Graph database engine is a new area and we are still trying to go further, and we hope more and more people will support or even join us. You can support in many ways:

- watch/star our project

- fork this repository and submit pull requests to us

- download and use this system, report bugs or suggestions

- ...

People who inspire us or contribute to this project will be listed in the [Thanks List](docs/THANK.md) chapter.

