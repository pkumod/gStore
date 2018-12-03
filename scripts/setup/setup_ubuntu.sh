#!/bin/bash

echo -e "\033[43;35m run this script in bash using root priviledge \033[0m \n"
#echo -e "\033[43;35m please place this script in the home directory of the gStore repository \033[0m \n"
# TODO: check versions of system software, disk size and memory
sleep 5s

apt install -y aptitude
echo -e "aptitude installed to search softwares conveniently \n"

apt install -y gcc g++
echo -e "g++ installed \n"

apt install -y make
echo -e "make installed \n"

apt install -y libreadline-dev
echo -e "readline-devel installed \n"

apt install -y openjdk-8-jdk
echo -e "openjdk 8 installed \n"

apt install -y curl libcurl4 libcurl4-openssl-dev
echo -e "libcurl-devel installed \n"

echo -e "Optional: requests for python api, pthreads and curl-devel for php api, realpath for gconsole, ccache for faster compilation\n"
echo -e "For help: https://github.com/pkumod/gStore/blob/master/docs/DEMAND.md \n"
sleep 5s

apt install -y libboost-all-dev
echo -e "libboost-all-dev installed \n"

ldconfig -v
echo -e "dynamic libraries path set \n"
echo -e "\033[43;35m Please run [ldconfig -v] again if you install other softwares \033[0m \n"
sleep 5s

#NOTICE: in Linux both threads and processes are created by vfork() in kernel with different sharing options.
#As a result, thread will consume PID and must be counted in the noproc number
echo "*    -    nofile    65535" >> /etc/security/limits.conf
echo "*    -    noproc    65535" >> /etc/security/limits.conf
#* means [for all users]，noproc means [maximum prcess number]，nofile means [maximum number of open files]
#- means [restrictions on both soft and hard]
echo -e "system environment variables set \n"

echo -e "when running program if you get a [can not find -lxxx] prompt, please search this dynamic library by [ldconfig -p|grep xxx] \n"

#sleep 5s
# compile the gStore system here
#logout  # exit from root account
#exit
#make


# colored output: https://blog.csdn.net/david_dai_1108/article/details/70478826

