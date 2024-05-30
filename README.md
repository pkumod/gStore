<div align="center">
  <img src="https://www.gstore.cn/pcsite/docs/logo.png" style="height: 200px;" alt="gStore logo"/>
</div>

<div align="center">
  <h4> English | <a href="#/zh-cn/"> 中文 </a> | <a href="https://en.gstore.cn" target="_blank"> Website </a> | <a href="https://www.gstore.cn" target="_blank"> 网站 </a>  </h4>
</div>

<div align="center">
  <a href="https://github.com/pkumod/gStore/commits/1.2/"><img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/m/pkumod/gstore?logo=github"/></a>
  <a href="https://github.com/pkumod/gStore/graphs/contributors"><img alt="GitHub contributors" src="https://img.shields.io/github/contributors-anon/pkumod/gstore?logo=github&color=%23ffd664"/></a>
  <a href="https://hub.docker.com/repository/docker/pkumodlab/gstore-docker/general"><img alt="Docker Image Version (V1.2)" src="https://img.shields.io/docker/v/pkumod/gstore?logo=docker&label=docker&color=%2328a8ea"/></a>
  <span><img alt="Static Badge" src="https://img.shields.io/badge/arch-x86_amd64_arm64_loongarch-%23f23f46?logo=amazonec2"></span>
</div>

gStore is an open-source graph database engine (or "triple store") born for managing large [RDF](http://www.w3.org/TR/rdf11-concepts/) datasets with the [SPARQL](https://www.w3.org/TR/sparql11-query/) query language. It works with Linux systems and amd64, arm64, and loongarch processors. gStore is a collaborative effort between the [Data Management Lab of Peking University](https://mod.icst.pku.edu.cn/english/index.htm), [University of Waterloo](https://uwaterloo.ca/), and awesome [contributors](https://github.com/pkumod/gStore/contributors) from the open-source community.

:key: gStore is released under the [BSD 3-Caluse](https://opensource.org/license/bsd-3-clause/) License, with several third-party libraries under their own licenses. Check [LICENSE](LICENSE) for details.

:bug: Check out [FAQ](docs/en-us/FAQ.md) for frequently asked questions. Known bugs and limitations are listed in [BUGS](/zh-cn/docs/BUGS.md) and [LIMIT](/zh-cn/docs/LIMIT.md). If you find any bugs, please feel free to [open an issue](https://github.com/pkumod/gStore/issues/new/choose).

:microphone: If you have any questions or suggestions, please open a thread in [GitHub Discussions](https://github.com/pkumod/gStore/discussions).

:book: For recommendations, project roadmap, and more, check [online documentation](https://www.gstore.cn/pcsite/index.html#/documentation).

**The formal help document is in [English(EN)](docs/help/gStore_help.pdf) and [中文(ZH)](docs/help/gStore_help_ZH.pdf).**

**The formal experiment result is in [Experiment](docs/test/formal_experiment.pdf).**

**We have built an IRC channel named #gStore on freenode, and you can visit [the homepage of gStore](https://www.gstore.cn).**

## Get gStore

gStore has been uploaded to gitee (code cloud), which is recommended for faster download for users in mainland China. The website is https://gitee.com/PKUMOD/gStore.

You can also open https://github.com/pkumod/gStore, download gStore.zip, then decompress the zip package.

### From Docker


```bash
$ docker pull pkumodlab/gstore-docker:latest
```


### From Source


To compile gStore, first clone the repository:


```bash
git clone https://github.com/pkumod/gStore.git
```

Complete instruction documentation is on the [Installation Instructions](docs/en-us/INSTALL_INSTRUCT.md).

## Quick Start


**gconsole command**


Login gconsole


```shell
$ bin/gconsole -u root
```


Create database


```shell
$ create lubm data/lubm/lubm.nt;
```


**Api server**


Start the server


```shell
# Default port 9000, you can change it with -p [port]
$ nohup bin/ghttp &
```


Then, you can access the server via HTTP protocol:


```shell
curl http://127.0.0.1:9000/?operation=check
```


Full list of commands introduce is on the [API Usage](docs/en-us/API_USAGE.md).


## Cite gStore


If you use gStore in your research, please cite the following paper:


```bibtex
@article{zou2014gstore,
  title={gStore: a graph-based SPARQL query engine},
  author={Zou, Lei and {\"O}zsu, M Tamer and Chen, Lei and Shen, Xuchuan and Huang, Ruizhe and Zhao, Dongyan},
  journal={The VLDB journal},
  volume={23},
  pages={565--590},
  year={2014},
  publisher={Springer}
}
```


Or cite this repository:


```bibtex
@misc{gStore,
  author = {gStore Authors},
  title = {gStore},
  year = {2021},
  publisher = {GitHub},
  journal = {GitHub repository},
  howpublished = {\url{https://github.com/pkumod/gStore}},
}
```


## Change log
**1.2(stable):2023-11-11**

New features in gStore 1.2 are listed as follows:

  - **Optimizing ORDER BY statements**: streamlining the execution logic of ORDER BY, removing unnecessary type judgments and conversions, and significantly improving execution efficiency.
  - **Optimized Build Module**: Supports building empty libraries. 
  - **Optimizing the Triple Parser**: Supports pure numeric IRIs, IRIs consisting only of numbers and letters, and IRIs starting with numbers. 
  - **New API interfaces**: gStore 1.2's ghttp and gRPC services have added five interfaces for **uploading files**, **downloading files**, **counting system resources**, **renaming**, and **obtaining backup paths**.
  - **New built-in advanced functions**: gStore 1.2 version adds seven advanced functions, namely **single source shortest path (`SSSP`, `SSSPLen`)**, **label propagation (`labelProp`)**, **weakly connected component (`WCC`)**, **global/local clustering coefficient (`clusteringCoeff`)**, **louvain algorithm (`louvain`)**, **K-hop count (`kHopCount`)**, and **K-hop neighbor (`kHopNeighbor`)**.
  - **Added support for calling `CONCAT` functions in `SELECT` statements**.
  - **Optimizing some local commands and API interfaces**: Optimizing the local command gconsole, optimizing the interfaces for building, loading, and statistical graph databases, and fixing potential bugs that may lead to memory leaks.
  - **Support for Multiple Data Formats**: Added support for multiple formats such as **Turtle**, **TriG**, **RDF/XML**, **RDFa**, and **JSON-LD**.
  - **Optimization of custom graph analysis algorithm editing function**: Redesign the interface of the custom graph analysis algorithm editing function, optimize the dynamic compilation algorithm, and improve compilation efficiency.
  - **Bug fixes**: Fixed a series of bugs.


**1.0：2022-10-01**

New features in gStore 1.0 are listed as follows:

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



**0.9.1：2021-11-25**

New features in gStore 0.9.1 are listed as follows:

- Decoupling the parsing and execution of queries in kernel, and further improvements on the query performance through optimized join ordering and other techniques. On complex queries, the performance is improved by over 40%.
- Rewriting of the HTTP service component, ghttp, with improved robustness and the addition of functions such as user permission, heartbeat detection, batch import, and batch deletion; API documents are added.
- Implementation of the Personalized PageRank (PPR) extension function, which can be invoked in the SELECT clause to calculate the correlation between entities.
- Support for arithmetic operations (e.g., `?x + ?y = 5`) in the FILTER clause.
- Support for transactional operations, such as begin, tquery (transactional query), commit, and rollback;
- A new executive component, gserver, is added to provide another pathway for remote access of gStore aside from the ghttp component, which implements two-way communication via the socket API.
- Unification of the format of command line arguments of executive components. The `--help` option is uniformly introduced (e.g., `$ bin/gbuild --help` or `$ bin/gbuild -h`), by which users can view the command manual including the meaning of each option.
- A number of bug fixes.

**0.9：2021-02-10**

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

**0.8**

The version is a stable version ,you can get it by 
```
 git clone -b 0.8 https://github.com/pkumod/gStore.git
```

<!--**You can write your information in [survey](http://59.108.48.38/survey) if you like.**-->

## Getting Started
### Compile from Source
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

### Deploy  via the Static compilation package
You can obtain the static compilation package from developers by sending an email to <gstore@pku.edu.cn>, and directly decompress the static compilation package to use it.

### Run
To run gStore, please type `bin/gbuild database_name dataset_path` to build a database named by yourself. And you can use `bin/gquery database_name` command to query an existing database. What is more, `bin/ghttp` is a wonderful tool designed for you, as a database server which can be accessed via HTTP protocol. Notice that all commands should be typed in the root directory of gStore, and your database name should not end with ".db".

- - -

## Advanced Help

If you want to understand the details of the gStore system, or you want to try some advanced operations(for example, using the API, server/client), please see the chapters below.


- [Document Download](docs/en-us/DOCUMENT_DOWNLOAD.md)


- [Update Log](docs/en-us/UPDATE_LOG.md)


- [Introduction of Knowledge Graph and gStore](docs/en-us/GRAPH_INTRODUCE.md)


- [Installation Instructions](docs/en-us/INSTALL_INSTRUCT.md)


- [Quick Start](docs/en-us/QUICK_START.md)


- [API Usage](docs/en-us/API_USAGE.md)


- [SPARQL query syntax](docs/en-us/SPARQL_QUERY.md)


- [gStore Visual Tool Workbench](docs/en-us/WORKBENCH_USAGE.md)


- [gStore Cloud platform user manual ](docs/en-us/CLOUD_INTRODUCE.md)


- [gStore Chronology](docs/en-us/CHRONOLOGY.md)


- [Open source and legal provision](docs/en-us/LEGAL_PROVISION.md)


- [gStore Logo](docs/en-us/GSTORE_LOGO.md)


- [FAQ](docs/en-us/FAQ.md)

- - -


## Other Business

Bugs are recorded in [BUG REPORT](docs/BUGS.md).
You are welcomed to submit the bugs  through [Community Web](https://www.gstore.cn/pcsite/index.html#/forum) questioning when you discover if they do not exist in this file.

We have written a series of short essays addressing recurring challenges in using gStore to realize applications, which are placed in [Recipe Book](docs/TIPS.md).

You are welcome to report any advice or errors in the github Issues part of this repository, if not requiring in-time reply. However, if you want to urgent on us to deal with your reports, please email to <gstore@pku.edu.cn> to submit your suggestions and report bugs. A full list of our whole team is in [Mailing List](docs/MAIL.md).

There are some restrictions when you use the current gStore project, you can see them on [Limit Description](docs/LIMIT.md).

Sometimes you may find some strange phenomena(but not wrong case), or something hard to understand/solve(don't know how to do next), then do not hesitate to visit the [Frequently Asked Questions](docs/FAQ.md) page.

Graph database engine is a new area and we are still trying to go further. Things we plan to do next is in [Future Plan](docs/PLAN.md) chapter, and we hope more and more people will support or even join us. You can support in many ways:

- watch/star our project

- fork this repository and submit pull requests to us

- download and use this system, report bugs or suggestions

- ...

People who inspire us or contribute to this project will be listed in the [Thanks List](docs/THANK.md) chapter.



<!--This whole document is divided into different pieces, and each them is stored in a markdown file. You can see/download the combined markdown file in [help_markdown](docs/gStore_help.md), and for html file, please go to [help_html](docs/gStore_help.html). What is more, we also provide help file in pdf format, and you can visit it in [help_pdf](docs/latex/gStore_help.pdf).-->