You are advised to read init.conf file, and modify it as you wish. (this file will configure the basic options of gStore system)

gStore is a green software, and you just need to compile it with one command. Please run

`make` 

in the gStore root directory to compile the gStore code, link the ANTLR lib, and build executable "gbuild", "gquery", "ghttp", "gserver", "gclient", "gconsole". What is more, the api of gStore is also built now.

If you want to use API examples of gStore, please run `make APIexample` to compile example codes for both C++ API and Java API. For details of API, please visit [API](API.md) chapter.

Use `make clean` command to clean all objects, executables, and use `make dist` command to clean all objects, executables, libs, datasets, databases, debug logs, temp/text files in the gStore root directory.

You are free to modify the source code of gStore and create your own project while respecting our work, and type `make tarball` command to compress all useful files into a .tar.gz file, which is easy to carry.

Type `make gtest` to compile the gtest program if you want to use this test utility. You can see the [HOW TO USE](USAGE.md) for details of gtest program.

