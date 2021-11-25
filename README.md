# gStore System

Gstore System(also called gStore) is a graph database engine for managing large graph-structured data, which is open-source and targets at Linux operation systems. The whole project is written in C++, with the help of some libraries such as readline, antlr, and so on. Only source tarballs are provided currently, which means you have to compile the source code if you want to use our system.

**The formal help document is in [English(EN)](docs/help/gStore_help.pdf) and [中文(ZH)](docs/help/gStore_help_ZH.pdf).**

**The formal experiment result is in [Experiment](docs/test/formal_experiment.pdf).**

**We have built an IRC channel named #gStore on freenode, and you can visit [the homepage of gStore](http://gstore.cn).**

## Change log

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

### Run
To run gStore, please type `bin/gbuild database_name dataset_path` to build a database named by yourself. And you can use `bin/gquery database_name` command to query an existing database. What is more, `bin/ghttp` is a wonderful tool designed for you, as a database server which can be accessed via HTTP protocol. Notice that all commands should be typed in the root directory of gStore, and your database name should not end with ".db".

- - -

## Advanced Help

If you want to understand the details of the gStore system, or you want to try some advanced operations(for example, using the API, server/client), please see the chapters below.

- [Basic Introduction](docs/INTRO.md): introduce the theory and features of gStore

- [Install Guide](docs/INSTALL.md): instructions on how to install this system

- [How To Use](docs/USAGE.md): detailed information about using the gStore system

- [API Explanation](docs/API.md): guide you to develop applications based on our API

- [Project Structure](docs/STRUCT.md): show the whole structure and process of this project

- [Related Essays](docs/ESSAY.md): contain essays and publications related with gStore

- [Update Logs](docs/CHANGELOG.md): keep the logs of the system updates

- [Test Results](docs/TEST.md): present the test results of a series of experiments

- - -

## Other Business

Bugs are recorded in [BUG REPORT](docs/BUGS.md).
You are welcomed to submit the bugs you discover if they do not exist in this file.

We have written a series of short essays addressing recurring challenges in using gStore to realize applications, which are placed in [Recipe Book](docs/TIPS.md).

You are welcome to report any advice or errors in the github Issues part of this repository, if not requiring in-time reply. However, if you want to urgent on us to deal with your reports, please email to <gjsjdbgroup@pku.edu.cn> to submit your suggestions and report bugs. A full list of our whole team is in [Mailing List](docs/MAIL.md).

There are some restrictions when you use the current gStore project, you can see them on [Limit Description](docs/LIMIT.md).

Sometimes you may find some strange phenomena(but not wrong case), or something hard to understand/solve(don't know how to do next), then do not hesitate to visit the [Frequently Asked Questions](docs/FAQ.md) page.

Graph database engine is a new area and we are still trying to go further. Things we plan to do next is in [Future Plan](docs/PLAN.md) chapter, and we hope more and more people will support or even join us. You can support in many ways:

- watch/star our project

- fork this repository and submit pull requests to us

- download and use this system, report bugs or suggestions

- ...

People who inspire us or contribute to this project will be listed in the [Thanks List](docs/THANK.md) chapter.



<!--This whole document is divided into different pieces, and each them is stored in a markdown file. You can see/download the combined markdown file in [help_markdown](docs/gStore_help.md), and for html file, please go to [help_html](docs/gStore_help.html). What is more, we also provide help file in pdf format, and you can visit it in [help_pdf](docs/latex/gStore_help.pdf).-->

