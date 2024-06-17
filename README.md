<div align="center">
  <img src="https://www.gstore.cn/pcsite/docs/logo.png" style="height: 200px;" alt="gStore logo"/>
</div>

**English** | [中文](README_ZH.md) | [Website](https://en.gstore.cn) | [网站](https://www.gstore.cn)

<div align="center">
  <a href="https://github.com/pkumod/gStore/commits/1.2/"><img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/m/pkumod/gstore?logo=github"/></a>
  <a href="https://github.com/pkumod/gStore/graphs/contributors"><img alt="GitHub contributors" src="https://img.shields.io/github/contributors-anon/pkumod/gstore?logo=github&color=%23ffd664"/></a>
  <a href="https://hub.docker.com/repository/docker/pkumodlab/gstore-docker/general"><img alt="Docker Image Version (V1.2)" src="https://img.shields.io/docker/v/pkumod/gstore?logo=docker&label=docker&color=%2328a8ea"/></a>
  <span><img alt="Static Badge" src="https://img.shields.io/badge/arch-x86_amd64_arm64_loongarch-%23f23f46?logo=amazonec2"></span>
</div>

gStore is an open-source graph database engine (or "triple store") born for managing large [RDF](http://www.w3.org/TR/rdf11-concepts/) datasets with the [SPARQL](https://www.w3.org/TR/sparql11-query/) query language. It works with Linux systems and amd64, arm64, and loongarch processors. gStore is a collaborative effort between the [Data Management Lab of Peking University](https://mod.icst.pku.edu.cn/english/index.htm), [University of Waterloo](https://uwaterloo.ca/), and awesome [contributors](https://github.com/pkumod/gStore/contributors) from the open-source community.

:key: gStore is released under the [BSD 3-Caluse](https://opensource.org/license/bsd-3-clause/) License, with several third-party libraries under their own licenses. Check [LICENSE](LICENSE) for details.

:bug: Check out [FAQ](docs/en-us/FAQ.md) for frequently asked questions. Known bugs and limitations are listed in [BUGS](docs/en-us/BUGS.md) and [LIMIT](docs/en-us/LIMIT.md). If you find any bugs, please feel free to [open an issue](https://github.com/pkumod/gStore/issues/new/choose).

:microphone: If you have any questions or suggestions, please open a thread in [GitHub Discussions](https://github.com/pkumod/gStore/discussions).

:book: For recommendations, project roadmap, and more, check [online documentation](https://www.gstore.cn/pcsite/index.html#/documentation).

**The formal help document is in [English(EN)](docs/help/gStore_help.pdf) and [中文(ZH)](docs/help/gStore_help_ZH.pdf).**

**The formal experiment result is in [Experiment](docs/test/formal_experiment.pdf).**

**We have built an IRC channel named #gStore on freenode, and you can visit [the homepage of gStore](https://en.gstore.cn/#/enHome).**

## Get gStore

gStore has been uploaded to gitee (code cloud), which is recommended for faster download for users in mainland China. The website is https://gitee.com/PKUMOD/gStore.

You can also open https://github.com/pkumod/gStore, download gStore.zip, then decompress the zip package.

### From Docker


```bash
$ docker pull pkumodlab/gstore-docker:latest
```

Complete instruction documentation is on the [Docker Deployment Instructions](docs/en-us/DOCKER_DEPLOY_EN.md).

### From Source


To compile gStore, first clone the repository:


```bash
git clone https://github.com/pkumod/gStore.git
```

Complete instruction documentation is on the [Installation Instructions](docs/en-us/INSTALL_INSTRUCT.md).

## Quick Start

**N-Triple Data format introduction**

​      RDF data should be provided in n-triple format (XML is not currently supported), and queries must be provided in [SPARQL1.1](https://www.w3.org/TR/sparql11-query/) syntax. The following is an example of the n-triple format file:        

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

​    More specific information about N-Triple please check [N-Triple](https://www.w3.org/TR/n-triples/). Not all syntax in [SPARQL1.1](https://www.w3.org/TR/sparql11-query/) is parsed and answered in gStore; for example, property paths are beyond the capabilities of the gStore system.

**Initialize the system database**

```shell
bin/ginit
```

**Create database**

```shell
bin/gbuild -db lubm -f data/lubm/lubm.nt 
```

**Database list**

```shell
bin/gshow
```

**Database query**

```shell
bin/gquery -db lubm -q data/lubm/lubm_q0.sql 
```

Complete instruction documentation is on the [Quick Start](docs/en-us/QUICK_START.md).


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

<!--**You can write your information in [survey](http://59.108.48.38/survey) if you like.**-->

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

Bugs are recorded in [BUG REPORT](docs/en-us/BUGS.md).
You are welcomed to submit the bugs  through [Community Web](https://www.gstore.cn/pcsite/index.html#/forum) questioning when you discover if they do not exist in this file.

We have written a series of short essays addressing recurring challenges in using gStore to realize applications, which are placed in [Recipe Book](docs/en-us/TIPS.md).

You are welcome to report any advice or errors in the github Issues part of this repository, if not requiring in-time reply. However, if you want to urgent on us to deal with your reports, please email to <gstore@pku.edu.cn> to submit your suggestions and report bugs. A full list of our whole team is in [Mailing List](docs/en-us/MAIL.md).

There are some restrictions when you use the current gStore project, you can see them on [Limit Description](docs/en-us/LIMIT.md).

Sometimes you may find some strange phenomena(but not wrong case), or something hard to understand/solve(don't know how to do next), then do not hesitate to visit the [Frequently Asked Questions](docs/en-us/FAQ.md) page.

Graph database engine is a new area and we are still trying to go further. Things we plan to do next is in [Future Plan](docs/en-us/PLAN.md) chapter, and we hope more and more people will support or even join us. You can support in many ways:

- watch/star our project

- fork this repository and submit pull requests to us

- download and use this system, report bugs or suggestions

- ...

People who inspire us or contribute to this project will be listed in the [Thanks List](docs/en-us/THANK.md) chapter.



<!--This whole document is divided into different pieces, and each them is stored in a markdown file. You can see/download the combined markdown file in [help_markdown](docs/gStore_help.md), and for html file, please go to [help_html](docs/gStore_help.html). What is more, we also provide help file in pdf format, and you can visit it in [help_pdf](docs/latex/gStore_help.pdf).-->