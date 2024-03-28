# Compile gStore from Source

## Prepare

*We have tested on linux server with CentOS 6.2 x86_64 and CentOS 6.6 x86_64. The version of GCC should be 4.8 or later.*

| Item             | Requirement                                           |
| :--------------- | :---------------------------------------------------- |
| operation system | Linux, such as CentOS, Ubuntu and so on               |
| architecture     | x86_64                                                |
| disk size        | according to size of dataset                          |
| memory size      | according to size of dataset                          |
| glibc            | version >= 2.14                                       |
| gcc              | version >= 5.0                                        |
| g++              | version >= 5.0                                        |
| make             | need to be installed                                  |
| cmake            | version >= 3.6                                        |
| pkg-config       | need to be installed                                  |
| uuid-devel       | need to be installed                                  |
| boost            | version >= 1.54                                       |
| readline-devel   | need to be installed                                  |
| curl-devel       | need to be installed                                  |
| openssl-devel    | version >= 1.1                                        |
| jemalloc         | 1.2 version and above, must be installed              |
| openjdk-devel    | needed if using Java api                              |
| requests         | needed if using Python http api                       |
| node             | needed if using Nodejs http api and version >= 10.9.0 |
| pthreads         | needed if using php http api                          |
| realpath         | needed if using gconsole                              |
| ccache           | optional, used to speed up the compilation            |


>**To help ease the burden of setting environments, several scripts are provided in [setup](https://github.com/pkumod/gStore/tree/master/scripts/setup/) for different Linux distributions. Please select the setup script corresponding to your system and run it with root(or sudo) priviledge. (As for CentOS system, you need to install boost-devel by yourselves.)**

1. The name of some packages may be different in different platforms, just install the corresponding one in your own operation system.

2. To install readline and readline-devel, just type `dnf install readline-devel` in Redhat/CentOS/Fedora, or `apt-get install libreadline-dev` in Debian/Ubuntu. Please use corresponding commands in other systems. If you use ArchLinux, just type `pacman -S readline` to install the readline and readline-devel.(so do other packages)

3. You do not have to install realpath to use gStore, but if you want to use the gconsole for its convenience, please do so by using `dnf install realpath` or `apt-get install realpath`. However, if you can not install realpath in your system, please go to modify the file Main/gconsole.cpp(just find the place using realpath command and remove the realpath command).

4. Our programs use regEx functions, which are provided by GNU/Linux by default. 

5. ANTLR3.4 is used in gStore to produce lexer and parser code for SPARQL query. However, you do not need to install the corresponding antlr libraries because we have merged the libantlr3.4 in our system.

6. When you type `make` in the root directory of the gStore project, the Java api will also be compiled. You can modify the makefile if you do not have JDK in your system. However, you are advised to install openjdk-devel in your Linux system.

7. To install ccache, you need to add epel repository if using CentOS, while in Ubuntu you can directly install it by `apt-get install ccache` command. If you can not install ccache(or maybe you do not want to), please go to modify the makefile(just change the CC variable to g++).

8. If you need to use the HTTP server in gStore, then Boost Library(like boost-devel, including boost headers for developing) must be installed and the version should not be less than 1.54. Remember to check the makefile for your installed path of Boost. To use Python api, you need to install requests by `pip install requests` in CentOS. To use php api, you need to install pthreads and curl in CentOS as follows:


1. get curl-devel

```bash
yum install curl-devel
```

2. get php & pthreads

```bash
wget -c http://www.php.net/distributions/php-5.4.36.tar.gz
wget -c http://pecl.php.net/get/pthreads-1.0.0.tgz
```

3. extract both

```bash
tar zxvf php-5.4.36.tar.gz
tar zxvf pthreads-1.0.0.tgz
```

4. move pthreads to php/ext folder 

```bash
mv pthreads-1.0.0 php-5.4.36/ext/pthreads
```

5. reconfigure sources

```bash
./buildconf --force
./configure --help | grep pthreads
```

You have to see --enable-pthreads listed. If do not, clear the builds with this commands:

```bash
rm -rf aclocal.m4
rm -rf autom4te.cache/
./buildconf --force
```

6. Inside php folder run configure command to set what we need:

```bash
./configure --enable-debug --enable-maintainer-zts --enable-pthreads --prefix=/usr --with-config-file-path=/etc --with-curl
```

7. install php
   We will run make clear just to be sure that no other crashed build will mess our new one.

```bash
make clear
make
make install
```

8. install node

```bash
wget https://npm.taobao.org/mirrors/node/v10.9.0/node-v10.9.0.tar.gz
tar -xvf node-v10.9.0.tar.gz
cd node-v10.9.0
./configure
make
sudo make install
```

9. Copy configuration file of PHP and add local lib to include path

```bash
cp php.ini-development /etc/php.ini
```

Edit php.ini and set Include_path to be like this:

```ini
Include_path = "/usr/local/lib/php"
```

10. Check Modules

```bash
php -m (check pthread loaded)
```

You have to see pthreads listed

11. If pthread is not listed, update php.ini

```bash
echo "extension=pthreads.so" >> /etc/php.ini
```

12. Any other questions, please go to [FAQ](FAQ.md) page.


## Compile

You are advised to read init.conf file, and modify it as you wish. (this file will configure the basic options of gStore system)

gStore is a green software, and you just need to compile it with three commands. Please run

```bash
sudo ./scripts/setup/setup_$(ARCH).sh 
sudo bash scripts/build.sh

```

in the gStore home directory to prepare the dependency, link the ANTLR lib, compile the gStore code, and build executable "gbuild", "gquery", "gadd", "gsub", "gmonitor", "gshow", "ghttp", "ginit", "gdrop", "shutdown", "gserver", "gclient", "gconsole". 

(Please substitute the $(ARCH) with your system version, like setup_archlinux.sh, setup_centos.sh and setup_ubuntu.sh)

What is more, the api of gStore is also built now.

Setup scripts and dependency preparation only need to be done once, later you can directly use `make` to compile the code.

(For faster compiling speed,  go to the build directory and use `make -j4` instead, using how many threads is up to your machine)

To check the correctness of the program, please type `make test` command in the build directory..

Only if you use the `make dist` command, then you need to run `make pre` command again in the build directory.

If you want to use API examples of gStore, please run `make APIexample` to compile example codes for both C++ API and Java API. For details of API, please visit [API](API.md) chapter.

Use `make clean` command to clean all objects, executables, and use `make dist` command to clean all objects, executables, libs, datasets, databases, debug logs, temp/text files in the gStore build directory.

You are free to modify the source code of gStore and create your own project while respecting our work, and go to the build directory and type `make tarball` command to compress all useful files into a .tar.gz file, which is easy to carry.

Type `make gtest` to compile the gtest program if you want to use this test utility in the build directory. You can see the [HOW TO USE](USAGE.md) for details of gtest program.

