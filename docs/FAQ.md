> gStore Version:1.0
>
> Last Modification Data:2022-12-31
>
> Modified by:JiuhuaQi
>
> Modification description: We have added some key questions of concern.

#### 1. What is the data model of gStore?

Answer: The data model of gStore is a native graph model that maintains the graph structure of the original RDF knowledge graph.

------

#### 2. What query language does gStore use?

Answer: gStore uses the SPARQL 1.1 standard query language and converts SPARQL queries for RDF into subgraph matching queries for RDF graphs.

------

#### 3. What is the storage capacity of the stand-alone version of gStore?

Answer: A single machine supports 5 billion RDF triples.

------

#### 4. What operating systems does gStroe support?

Answer: gStore currently supports Linux-based kernel operating systems, such as CentOS and Ubuntu.

------

#### 5. What are the requirements for the installation environment of gStore?

Answer: For the environment requirements of gStore, please refer to Developer Resources > Installation Instructions > System Requirements.

------

#### 6. What are the installation methods of gStore?

Answer: The latest version 1.0 of gStore currently supports three installation methods:

(1) Static compiled version installation: download and unzip it to use (click [here](http://file.gstore.cn/f/9a89ea8572e64262b1af/?dl=1) to download)

(2) Source code compilation and installation: For details, please refer to Developer Resources > Installation Instructions > gStore Compilation (the required environment needs to be installed first)

(3) Docker installation: For details, please refer to Developer Resources > Installation Instructions > Deploy gStore using Docker

------

#### 7. Can gStore be operated through interfaces?

Answer: gStore currently provides three interface modes: HTTP, RPC, and Socket, corresponding respectively to the three services: bin/ghttp, bin/grpc, and bin/gserver. For specific usage, please refer to Developer Resources > Quick Start > HTTP API Service and Socket API Service.

------

#### 8. How to start up the interface service of gStore?

Answer: Starting up ghttp: bin/ghtp - p [port]

Starting up grpc: bin/grpc - p [port]

Starting up server: bin/gserver - s

For specific usage, please refer to Developer Resources > Quick Start > HTTP API Service and Socket API Service.

------

#### 9. Which path queries does gStore support?

Answer: Path queries currently supported by gStore include: shortest path query, cycle query, K-hop reachability query, and breadth-first search count.

------

#### 10. What data file formats does gStore support for building databases?

Answer: The file formats supported by gStore are nt, n3, ttl and rdf.

------

> gStore Version: 0.1-0.8
>
> Last Modification Data:2022-12-31
>
> Modified by:JiuhuaQi
>
> Modification description:

#### 11. How can I redirect the output to disk files from the console, even the content is output from stderr?

Answer: You can redirect the output of the whole console and use `tail -f` to see the immediate updates to the disk file.
For example, if you are using `gquery` console, you can do it in the way below:

```
In one terminal, named x
bin/gquery ${YOUR_DATABASE} >& log.txt
type console commands in terminal x
In another terminal y
tail -f log.txt

```

---

#### 12. Why can not I run correctly after `make` again, even the new code has fixed bugs or added new features?

Answer: Sometimes file dependency is not complete, and some objects are not re-compiled in practice.
In this case you are advised to run `make clean` first, and then run `make`.

---

#### 13. When I use the newer gStore system to query the original database, why error?

Answer: The database produced by gStore contains several indexes, whose structures may have been chnaged in the new gStore version. So, please rebuild your dataset just in case.

- - -

#### 14. Why error when I try to write programs based on gStore, just like the Main/gconsole.cpp?

Answer: You need to add these phrases at the beginning of your main program, otherwise gStore will not run correctly:
	//NOTICE:this is needed to ensure the file path is the work path
	chdir(dirname(argv[0]));
	//NOTICE:this is needed to set several debug files
    Util util;

- - -

#### 15. Why does gStore report "garbage collection failed" error when I use teh Java API?

Answer: You need to adjust the parameters of jvm, see [url1](http://www.cnblogs.com/edwardlauxh/archive/2010/04/25/1918603.html) and [url2](http://www.cnblogs.com/redcreen/archive/2011/05/04/2037057.html) for details.

- - -

#### 16. When I compile the code in ArchLinux, why the error that "no -ltermcap" is reported?

Answer: In ArchLinux, you only need to use `-lreadline` to link the readline library. Please remove the `-ltermcap` in the makefile which is located in the root of the gStore project if you would like to use ArchLinux.

- - -

#### 17. Why does gStore report errors that the format of some RDF datasets are not supported?

Answer: gStore does not support all RDF formats currently, please see [formats](../test/format_question.txt) for details.

- - -

#### 18. When I read on GitHub, why are some documents unable to be opened?

Answer: Codes, markdowns or other text files, and pictures can be read directly on GitHub. However, if you are using some light weight browsers like midori, for files in pdf type, please download them and read on your computer or other devices.

- - -

#### 19. Why sometimes strange characters appear when I use gStore?

Answer:  There are some documents's names are in Chinese, and you don't need to worry about it.

- - -

#### 20. What is the .gitattributes file in this project?

Answer:  We use [git-lfs](https://github.com/github/git-lfs) in our system, and the .gitattributes file is used to record the file types to be tracked by git-lfs. You are advised to use git-lfs also if you want to join us.(git-lfs is used to track datasets, pdf files and pictures here)

- - -

#### 21. In centos7, if the watdiv.db(a generated database after gload) is copied or compressed/uncompressed, the size of watdiv.db will be different(generally increasing) if using `du -h` command to check?

Answer:  It's the change of B+-trees' size in watdiv/kv_store/ that causes the change of the whole database's size. The reason is that in storage/Storage.cpp, many operations use fseek to move file pointer. As everyone knows, file is organized in blocks, and if we request for new block, file pointer may be moved beyond the end of this file(file operations are all achieved by C in gStore, no errors are reported), then contents will be written in the new position!

In **Advanced Programming In The Unix Environment**, "file hole" is used to describe this phenomenon. "file hole" will be filled with 0, and it's also one part of the file. You can use `ls -l` to see the size of file(computing the size of holes), while `du -h` command shows the size of blocks that directory/file occupies in system. Generally, the output of `du -h` is large than that of `ls -l`, but if "file hole" exists, the opposite is the case because the size of holes are neglected.

The actual size of files containing holes are fixed, while in some operation systems, holes will be transformed to contents(also 0) when copied. Operation `mv` will not affect the size if not across different devices.(only need to adjust the file tree index) However, `cp` and all kinds of compress methods need to scan the file and transfer data.(there are two ways to achieve `cp` command, neglect holes or not, while the output size of `ls -l` not varies)

It is valid to use "file hole" in C, and this is not an error, which means you can go on using gStore. We achieve a small [program](../test/hole.c) to describe the "file holes", you can download and try it yourself.

- - -

#### 22. In gclient console, a database is built, queried, and then I quit the console. Next time I enter the console, load the originally imported database, but no output for any queries(originally the output is not empty)?

Answer:  You need to unload the using database before quiting the gclient console, otherwise errors come.

- - -

#### 23. If query results contain null value, how can I use the [full_test](../test/full_test.sh) utility? Tab separated method will cause problem here because null value cannot be checked!

Answer:  You may use other programming language(for example, Python) to deal with the null value cases. For example, you can change null value in output to special character like ',', later you can use the [full_test](../test/full_test.sh) utility.

- - -

#### 24. When I compile and run the API examples, it reports the "unable to connect to server" error?

Answer:  Please use `./gserver` command to start up a gStore server first, and notice that the server ip and port must be matched.

- - -

#### 25. When I use the Java API to write my own program, it reports "not found main class" error?

Answer:  Please ensure that you include the position of your own program in class path of java. The whole command should be something like `java -cp /home/bookug/project/devGstore/api/java/lib/GstoreJavaAPI.jar:. JavaAPIExample`, and the ":." in this command cannot be neglected.

