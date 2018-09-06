#!/bin/bash

# USAGE: run this script in bash using root priviledge
# TODO: check versions of system software, disk size and memory

yum install gcc gcc-c++
echo "g++ installed \n"

yum install make
echo "make installed \n"

yum install readline readline-devel
echo "readline-devel installed \n"

yum install java-1.8.0-openjdk-devel
echo "openjdk 8 installed \n"

yum install libcurl-devel
echo "libcurl-devel installed \n"

echo "Optional: requests for python api, pthreads and curl-devel for php api, realpath for gconsole, ccache for faster compilation\n"
echo "For help: https://github.com/pkumod/gStore/blob/master/docs/DEMAND.md \n"

# TODO: boost-devel 1.54(or larger) can not be installed by `yum`, please install it by yourselve from source code.
echo -e "\033[43;35m Please install boost-devel 1.54(or larger by yourself) \033[0m \n"

ldconfig -v
echo "dynamic libraries path set \n"
echo -e "\033[43;35m Please run [ldconfig -v] again if you install other softwares \033[0m \n"

#NOTICE: in Linux both threads and processes are created by vfork() in kernel with different sharing options.
#As a result, thread will consume PID and must be counted in the noproc number
echo "*    -    nofile    65535" >> /etc/security/limits.conf
echo "*    -    noproc    65535" >> /etc/security/limits.conf
#* means [for all users]，noproc means [maximum prcess number]，nofile means [maximum number of open files]
#- means [restrictions on both soft and hard]

echo "system environment variables set \n"

echo "when running program if you get a [can not find -lxxx] prompt, please search this dynamic library by [ldconfig -p|grep xxx] \n"




# colored output: https://blog.csdn.net/david_dai_1108/article/details/70478826

