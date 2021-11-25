#### How can I redirect the output to disk files from the console, even the content is output from stderr?

You can redirect the output of the whole console and use `tail -f` to see the immediate updates to the disk file.
For example, if you are using `gquery` console, you can do it in the way below:

```
In one terminal, named x
bin/gquery ${YOUR_DATABASE} >& log.txt
type console commands in terminal x
In another terminal y
tail -f log.txt

```

---

#### Why can not I run correctly after `make` again, even the new code has fixed bugs or added new features?

Sometimes file dependency is not complete, and some objects are not re-compiled in practice.
In this case you are advised to run `make clean` first, and then run `make`.

---

#### When I use the newer gStore system to query the original database, why error?

The database produced by gStore contains several indexes, whose structures may have been chnaged in the new gStore version. So, please rebuild your dataset just in case.

- - -

#### Why error when I try to write programs based on gStore, just like the Main/gconsole.cpp?

You need to add these phrases at the beginning of your main program, otherwise gStore will not run correctly:
	//NOTICE:this is needed to ensure the file path is the work path
	chdir(dirname(argv[0]));
	//NOTICE:this is needed to set several debug files
    Util util;

- - -

#### Why does gStore report "garbage collection failed" error when I use teh Java API?

You need to adjust the parameters of jvm, see [url1](http://www.cnblogs.com/edwardlauxh/archive/2010/04/25/1918603.html) and [url2](http://www.cnblogs.com/redcreen/archive/2011/05/04/2037057.html) for details.

- - -

#### When I compile the code in ArchLinux, why the error that "no -ltermcap" is reported?

In ArchLinux, you only need to use `-lreadline` to link the readline library. Please remove the `-ltermcap` in the makefile which is located in the root of the gStore project if you would like to use ArchLinux.

- - -

#### Why does gStore report errors that the format of some RDF datasets are not supported?

gStore does not support all RDF formats currently, please see [formats](../test/format_question.txt) for details.

- - -

#### When I read on GitHub, why are some documents unable to be opened?

Codes, markdowns or other text files, and pictures can be read directly on GitHub. However, if you are using some light weight browsers like midori, for files in pdf type, please download them and read on your computer or other devices.

- - -

#### Why sometimes strange characters appear when I use gStore?

There are some documents's names are in Chinese, and you don't need to worry about it.

- - -

#### What is the .gitattributes file in this project?

We use [git-lfs](https://github.com/github/git-lfs) in our system, and the .gitattributes file is used to record the file types to be tracked by git-lfs. You are advised to use git-lfs also if you want to join us.(git-lfs is used to track datasets, pdf files and pictures here)

- - -

#### In centos7, if the watdiv.db(a generated database after gload) is copied or compressed/uncompressed, the size of watdiv.db will be different(generally increasing) if using `du -h` command to check?

It's the change of B+-trees' size in watdiv/kv_store/ that causes the change of the whole database's size. The reason is that in storage/Storage.cpp, many operations use fseek to move file pointer. As everyone knows, file is organized in blocks, and if we request for new block, file pointer may be moved beyond the end of this file(file operations are all achieved by C in gStore, no errors are reported), then contents will be written in the new position!

In **Advanced Programming In The Unix Environment**, "file hole" is used to describe this phenomenon. "file hole" will be filled with 0, and it's also one part of the file. You can use `ls -l` to see the size of file(computing the size of holes), while `du -h` command shows the size of blocks that directory/file occupies in system. Generally, the output of `du -h` is large than that of `ls -l`, but if "file hole" exists, the opposite is the case because the size of holes are neglected.

The actual size of files containing holes are fixed, while in some operation systems, holes will be transformed to contents(also 0) when copied. Operation `mv` will not affect the size if not across different devices.(only need to adjust the file tree index) However, `cp` and all kinds of compress methods need to scan the file and transfer data.(there are two ways to achieve `cp` command, neglect holes or not, while the output size of `ls -l` not varies)

It is valid to use "file hole" in C, and this is not an error, which means you can go on using gStore. We achieve a small [program](../test/hole.c) to describe the "file holes", you can download and try it yourself.

- - -

#### In gclient console, a database is built, queried, and then I quit the console. Next time I enter the console, load the originally imported database, but no output for any queries(originally the output is not empty)?

You need to unload the using database before quiting the gclient console, otherwise errors come.

- - -

#### If query results contain null value, how can I use the [full_test](../test/full_test.sh) utility? Tab separated method will cause problem here because null value cannot be checked!

You may use other programming language(for example, Python) to deal with the null value cases. For example, you can change null value in output to special character like ',', later you can use the [full_test](../test/full_test.sh) utility.

- - -

#### When I compile and run the API examples, it reports the "unable to connect to server" error?

Please use `./gserver` command to start up a gStore server first, and notice that the server ip and port must be matched.

- - -

#### When I use the Java API to write my own program, it reports "not found main class" error?

Please ensure that you include the position of your own program in class path of java. The whole command should be something like `java -cp /home/bookug/project/devGstore/api/java/lib/GstoreJavaAPI.jar:. JavaAPIExample`, and the ":." in this command cannot be neglected.

