<!-- insert a mobile friendly image -->
<div align="center">
  <img src="docs/logo.png" style="height: 200px;" alt="gStore logo"/>
</div>

<div align="center">
  <h4> English | <a href="docs/zh-cn/README.md" target="_blank"> 中文 </a> | <a href="https://en.gstore.cn" target="_blank"> Website </a> | <a href="https://www.gstore.cn" target="_blank"> 网站 </a> |  <a href="https://gstore-docs.pages.dev" target="_blank"> Documentation </a> | <a href="https://gstore-docs.pages.dev/zh-cn/" target="_blank"> 文档 </a> </h4>
</div>

<div align="center">
  <a href="https://github.com/hrz6976/gstore/commits"><img alt="GitHub commit activity" src="https://img.shields.io/github/commit-activity/y/pkumod/gstore?logo=github"/></a>
  <a href="https://github.com/hrz6976/gstore/contributors"><img alt="GitHub contributors" src="https://img.shields.io/github/contributors-anon/hrz6976/gstore?logo=github&color=%23ffd664"/></a>
  <a href="https://github.com/hrz6976/gstore/actions" ><img alt="GitHub Actions Workflow Status" src="https://img.shields.io/github/actions/workflow/status/hrz6976/gStore/ci.yml?logo=github"/></a>
  <a href="https://codecov.io/github/hrz6976/gStore" ><img src="https://codecov.io/github/hrz6976/gStore/graph/badge.svg?token=0NE3KM5AV8"/></a>
  <a href="https://hub.docker.com/repository/docker/hrz6976/gstore"><img alt="Docker Image Version (latest semver)" src="https://img.shields.io/docker/v/hrz6976/gstore?logo=docker&label=docker&color=%2328a8ea"/></a>
  <span><img alt="Static Badge" src="https://img.shields.io/badge/arch-amd64_arm64_loongarch-%23f23f46?logo=amazonec2"></span>
</div>

<p></p>

gStore is an open-source graph database engine (or "triple store") born for managing large [RDF](http://www.w3.org/TR/rdf11-concepts/) datasets with the [SPARQL](https://www.w3.org/TR/sparql11-query/) query language. It works with Linux systems and amd64, arm64, and loongarch processors. gStore is a collaborative effort between the [Data Management Lab of Peking University](https://mod.icst.pku.edu.cn/english/index.htm), [University of Waterloo](https://uwaterloo.ca/), and awesome [contributors](https://github.com/pkumod/gStore/contributors) from the open-source community.

:key: gStore is released under the [BSD 3-Caluse](https://opensource.org/license/bsd-3-clause/) License, with several third-party libraries under their own licenses. Check [LICENSE](/zh-cn/LICENSE.md) for details.

:bug: Check out [FAQ](/docs/FAQ.md) for frequently asked questions. Known bugs and limitations are listed in [BUGS](/zh-cn/docs/BUGS.md) and [LIMIT](/zh-cn/docs/LIMIT.md). If you find any bugs, please feel free to [open an issue](https://github.com/pkumod/gStore/issues/new/choose).

:microphone: If you have any questions or suggestions, please open a thread in [GitHub Discussions](https://github.com/pkumod/gStore/discussions).

:book: For recommendations, project roadmap, and more, check [online documentation](https://gstore-docs.pages.dev/zh-cn/).


## Highlights

* First-tire performance in the [WatDiv](http://dsg.uwaterloo.ca/watdiv/), [LUBM](http://swat.cse.lehigh.edu/projects/lubm/) and [DBPedia](https://wiki.dbpedia.org/) benchmarks. ([Benchmark Results](docs/test/formal_experiment.pdf))

* Supports multiple RDF formats, including Turtle, TriG, RDF/XML, RDFa, and JSON-LD.
  
* Supports user-defined graph analysis functions. (Added in 1.0)

* ACID-compliant transactions. (Added in 1.0)

## Get gStore

### From Docker

We host cross-platform Docker images on [Docker Hub](https://hub.docker.com/repository/docker/hrz6976/gstore). You can pull and run the latest image with:

```bash
docker run -it --rm hrz6976/gstore sh -c "(/usr/local/bin/ghttp &); bash"
```

To serve `ghttp` and persist the data:
```bash
export GSTORE_PORT=9000 # or any port you want
export GSTORE_PATH=$(realpath ./gstore) # or any local directory with write permission
docker run -d -p ${GSTORE_PORT}:9000 -v ${GSTORE_PATH}:/app gstore
```

Check [documentation](https://gstore-docs.pages.dev/#/DOCKER_DEPLOY) for more detailed instructions.

### From Source

To compile gStore, first clone the repository:

```bash
git clone https://github.com/pkumod/gStore.git --depth=1
```

Then, install the system packages required to compile gStore:

```bash
cd gStore
bash scripts/setup-dev.sh
```

Install conan and 3rd-party libraries:

```bash
pip3 install conan --user
conan detect
conan install . --build=missing -s build_type=Release
```

Finally, compile gStore:

```bash
cd cmake-build-release
cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
ninja prepare
ninja install
```

Check [documentation](https://gstore-docs.pages.dev/#/COMPILE) for more details.

## Quick Start

Create the databases (docker containers automatically do this on startup):

```bash
export GSTORE_ROOT_PASSWORD=a_strong_password # or any password you want
bash script/init.sh
```

Run a query:
  
```bash
bin/gquery -db small -q data/small/small_q0.sql
```

Start the server:

```bash
bin/ghttp &
```

Then, you can access the server via HTTP protocol:

```bash
curl http://127.0.0.1:9000/grpc/api
```

To shutdown the server:

```bash
bin/shutdown
```

Full list of commands is on the [documentation website](https://gstore-docs.pages.dev/#/USAGE).

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

## Changelog

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


<!--This whole document is divided into different pieces, and each them is stored in a markdown file. You can see/download the combined markdown file in [help_markdown](docs/gStore_help.md), and for html file, please go to [help_html](docs/gStore_help.html). What is more, we also provide help file in pdf format, and you can visit it in [help_pdf](docs/latex/gStore_help.pdf).-->

