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
readline | need to be installed
readline-devel | need to be installed
openjdk | needed if using Java api
openjdk-devel | needed if using Java api
realpath | needed if using gconsole

#### NOTICE:

1. To install readline and readline-devel, just type `dnf install readline-devel` in Redhat/CentOS/Fedora, or `apt-get install libreadline-dev` in Debian/Ubuntu. Please use corresponding commands in other systems.

2. You donot have to install realpath to use gStore, but if you want to use the gconsole for its convinence, please do so by using `dnf install realpath` or `apt-get install realpath`.

