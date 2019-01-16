You are advised to read init.conf file, and modify it as you wish. (this file will configure the basic options of gStore system)

gStore is a green software, and you just need to compile it with three commands. Please run

```
sudo ./scripts/setup/setup_$(ARCH).sh 
make pre
make

```
in the gStore home directory to prepare the dependency, link the ANTLR lib, compile the gStore code, and build executable "gbuild", "gquery", "gadd", "gsub", "gmonitor", "gshow", "ghttp", "ginit", "gdrop", "shutdown", "gserver", "gclient", "gconsole". 

(Please substitute the $(ARCH) with your system version, like setup_archlinux.sh, setup_centos.sh and setup_ubuntu.sh)

What is more, the api of gStore is also built now.

Setup scripts and dependency preparation only need to be done once, later you can directly use `make` to compile the code.

(For faster compiling speed, use `make -j4` instead, using how many threads is up to your machine)

To check the correctness of the program, please type `make test` command.

Only if you use the `make dist` command, then you need to run `make pre` command again.

If you want to use API examples of gStore, please run `make APIexample` to compile example codes for both C++ API and Java API. For details of API, please visit [API](API.md) chapter.

Use `make clean` command to clean all objects, executables, and use `make dist` command to clean all objects, executables, libs, datasets, databases, debug logs, temp/text files in the gStore root directory.

You are free to modify the source code of gStore and create your own project while respecting our work, and type `make tarball` command to compress all useful files into a .tar.gz file, which is easy to carry.

Type `make gtest` to compile the gtest program if you want to use this test utility. You can see the [HOW TO USE](USAGE.md) for details of gtest program.

