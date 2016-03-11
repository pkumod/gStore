# Gstore System

Gstore System(also called gStore) is a graph database engine for managing large graph-structured data, which is open-source and targets at Linux operation systems. The whole project is written in C++, and we try our best to avoid using STL. Only source tarballs are provided currently, which means you have to compile the source code if you want to use our system.

## Getting Started

This system is really user-friendly and you can pick it up in several minutes. Remember to check your platform where you want to run this system by viewing [System Requirements](docs/DEMAND.md). After all are verified, please get this project's source code. There are several ways to do this:

- download the zip from this repository and extract it

- fork this repository in your github account

- type `git clone git@github.com:Caesar11/gStore.git` in your terminal or use git GUI to acquire it

Then you need to compile the project, just type `make` in the gStore root directory, and all executables will be ok. To run gStore, please type `./gload database_name dataset_path` to build a database named by yourself. And you can use `./gquery database_name` command to query a existing database.

**Notice: Some files(*.jpg, *.docx, *.pdf) in the docs/ folder are stored using Git Large File Storage, you need to install [git-lfs](https://git-lfs.github.com/) to clone/download if you want to include them in the project. Do not care it if you do not want to see them.**

- - -

## Advanced Help

If you want to understand the details of the gStore system, or you want to try some advanced operations(for example, using the API, server/client), please see the chapters below.

- [Basic Introduction](docs/INTRO.md): introduce the theory and features of gStore

- [Install Guide](docs/INSTALL.md): instructions on how to install this system

- [How To Use](docs/USAGE.md): detailed information about using the gStore system

- [API Explanation](docs/API.md): guide you to develop applications based on our API

- [Project Structure](docs/STRUCT.md): show the whole structure and sequence of this project

- [Related Essays](docs/ESSAY.md): contain essays and publications related with gStore

- [Update Logs](docs/CHANGELOG.md): keep the logs of the system updates

- [Test Results](docs/TEST.md): present the test results of a series of experiments

- - -

## Other Business

We have written a series of short essays addressing recurring challenges in using gStore to realize applications, which are placed in [Recipe Book](docs/TIPS.md).

You are welcome to report any advice or errors in the github Issues part of this repository, if not requiring in-time reply. However, if you want to urgent on us to deal with your reports, please email to <chenjiaqi93@163.com> to submit your suggestions and report bugs to us by emailing to <zengli-syzz@pku.edu.cn>. A full list of our whole team is in [Mailing List](docs/MAIL.md).

There are some restrictions when you use the current gStore project, you can see them on [Limit Description](docs/LIMIT.md).

Sometimes you may find some strange phenomena(but not wrong case), or something hard to understand/solve(don't know how to do next), then go to the [Frequently Asked Questions](docs/FAQ.md) page.

Graph database engine is a new area and we are still trying to go further. Things we plan to do next is in [Future Plan](docs/PLAN.md) chapter, and we hope more and more people will support or even join us. You can support in many ways:

- watch/star our project

- fork this repository and submit pull requests to us

- download and use this system, report bugs or suggestions

- ...

People who inspire us or contribute to this project will be listed in the [Thanks List](docs/THANK.md) chapter.

