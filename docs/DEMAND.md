*We have tested on linux server with CentOS 6.2 x86_64 and CentOS 6.6 x86_64. The version of GCC should be 4.8 or later.*

Item | Requirement
:-- | :--
operation system | Linux, such as CentOS, Ubuntu and so on
architecture | x86_64
disk size | according to size of dataset 
memory size | according to size of dataset
glibc | version >= 2.14
gcc | version >= 4.8
g++ | version >= 4.8
make | need to be installed
boost | version >= 1.54
readline | need to be installed
readline-devel | need to be installed
openjdk | needed if using Java api
openjdk-devel | needed if using Java api
requests | needed if using Python http api
node | needed if using Nodejs http api and version >= 10.9.0
pthreads | needed if using php http api 
curl-devel | needed if using php http api
realpath | needed if using gconsole
ccache | optional, used to speed up the compilation
libcurl-devel | needed to be installed

NOTICE:

**To help ease the burden of setting environments, several scripts are provided in [setup](../scripts/setup/) for different Linux distributions. Please select the setup script corresponding to your system and run it with root(or sudo) priviledge. (As for CentOS system, you need to install boost-devel by yourselves.)**

1. The name of some packages may be different in different platforms, just install the corresponding one in your own operation system.

2. To install readline and readline-devel, just type `dnf install readline-devel` in Redhat/CentOS/Fedora, or `apt-get install libreadline-dev` in Debian/Ubuntu. Please use corresponding commands in other systems. If you use ArchLinux, just type `pacman -S readline` to install the readline and readline-devel.(so do other packages)

3. You do not have to install realpath to use gStore, but if you want to use the gconsole for its convenience, please do so by using `dnf install realpath` or `apt-get install realpath`. However, if you can not install realpath in your system, please go to modify the file Main/gconsole.cpp(just find the place using realpath command and remove the realpath command).

4. Our programs use regEx functions, which are provided by GNU/Linux by default. 

5. ANTLR3.4 is used in gStore to produce lexer and parser code for SPARQL query. However, you do not need to install the corresponding antlr libraries because we have merged the libantlr3.4 in our system.

6. When you type `make` in the root directory of the gStore project, the Java api will also be compiled. You can modify the makefile if you do not have JDK in your system. However, you are advised to install openjdk-devel in your Linux system.

7. To install ccache, you need to add epel repository if using CentOS, while in Ubuntu you can directly install it by `apt-get install ccache` command. If you can not install ccache(or maybe you do not want to), please go to modify the makefile(just change the CC variable to g++).

8. If you need to use the HTTP server in gStore, then Boost Library(like boost-devel, including boost headers for developing) must be installed and the version should not be less than 1.54. Remember to check the makefile for your installed path of Boost. To use Python api, you need to install requests by `pip install requests` in CentOS. To use php api, you need to install pthreads and curl in CentOS as follows:
```
1- get curl-devel
# yum install curl-devel

2- get php
# wget -c http://www.php.net/distributions/php-5.4.36.tar.gz

   get pthreads
# wget -c http://pecl.php.net/get/pthreads-1.0.0.tgz

3- extract both
# tar zxvf php-5.4.36.tar.gz
# tar zxvf pthreads-1.0.0.tgz

4- move pthreads to php/ext folder 
# mv pthreads-1.0.0 php-5.4.36/ext/pthreads

5- reconfigure sources
# ./buildconf --force
# ./configure --help | grep pthreads

You have to see --enable-pthreads listed. If do not, clear the builds with this commands:

# rm -rf aclocal.m4
# rm -rf autom4te.cache/
# ./buildconf --force

6- Inside php folder run configure command to set what we need:
# ./configure --enable-debug --enable-maintainer-zts --enable-pthreads --prefix=/usr --with-config-file-path=/etc --with-curl

7- install php
We will run make clear just to be sure that no other crashed build will mess our new one.

# make clear
# make
# make install

8- install node
# wget https://npm.taobao.org/mirrors/node/v10.9.0/node-v10.9.0.tar.gz
# tar -xvf node-v10.9.0.tar.gz
# cd node-v10.9.0
# ./configure
# make
# sudo make install

9- Copy configuration file of PHP and add local lib to include path
# cp php.ini-development /etc/php.ini

Edit php.ini and set Include_path to be like this:

Include_path = "/usr/local/lib/php"

10- Check Modules
# php -m (check pthread loaded)

You have to see pthreads listed

11- If pthread is not listed, update php.ini
# echo "extension=pthreads.so" >> /etc/php.ini

```

9. Any other questions, please go to [FAQ](FAQ.md) page.

