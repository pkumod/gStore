*We have tested on linux server with CentOS 6.2 x86_64 and CentOS 6.6 x86_64. The version of GCC should be 4.4.7 or later.*

Item | Requirement
:-- | :--
operation system | Linux, such as CentOS, Ubuntu and so on
architecture | x86_64
disk size | according to size of dataset 
memory size | according to size of dataset
glibc | version >= 2.14
gcc | version >= 4.4.7
g++ | version >= 4.4.7
make | need to be installed
boost | version >= 1.54
readline | need to be installed
readline-devel | need to be installed
openjdk | needed if using Java api
openjdk-devel | needed if using Java api
realpath | needed if using gconsole
ccache | optional, used to speed up the compilation

NOTICE:

1. The name of some packages may be different in different platforms, just install the corresponding one in your own operation system.

2. To install readline and readline-devel, just type `dnf install readline-devel` in Redhat/CentOS/Fedora, or `apt-get install libreadline-dev` in Debian/Ubuntu. Please use corresponding commands in other systems. If you use ArchLinux, just type `pacman -S readline` to install the readline and readline-devel.(so do other packages)

3. You do not have to install realpath to use gStore, but if you want to use the gconsole for its convenience, please do so by using `dnf install realpath` or `apt-get install realpath`. However, if you can not install realpath in your system, please go to modify the file Main/gconsole.cpp(just find the place using realpath command and remove the realpath command).

4. Our programs use regEx functions, which are provided by GNU/Linux by default. 

5. ANTLR3.4 is used in gStore to produce lexer and parser code for SPARQL query. However, you do not need to install the corresponding antlr libraries because we have merged the libantlr3.4 in our system.

6. When you type `make` in the root directory of the gStore project, the Java api will also be compiled. You can modify the makefile if you do not have JDK in your system. However, you are advised to install openjdk-devel in your Linux system.

7. To install ccache, you need to add epel repository if using CentOS, while in Ubuntu you can directly install it by `apt-get install ccache` command. If you can not install ccache(or maybe you do not want to), please go to modify the makefile(just change the CC variable to g++).

8. If you need to use the HTTP server in gStore, then Boost Library(like boost-devel, including boost headers for developing) must be installed and the version should not be less than 1.54. Remember to check the makefile for your installed path of Boost.

9. Any other questions, please go to [FAQ](FAQ.md) page.

