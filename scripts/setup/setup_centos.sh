#!/bin/bash

echo -e "\033[43;35m run this script in bash using root priviledge \033[0m \n"
#echo -e "\033[43;35m please place this script in the home directory of the gStore repository \033[0m \n"
# TODO: check versions of system software, disk size and memory
sleep 5s

yum install -y make
echo -e "make installed "
# download source code path
SRC_PATH="/var/local/"
CURRENT_PATH=`pwd`
# build install gcc gcc-c++
GCC_VERSION="9.3.0"
# GCC base path
INSTALL_BASE="/opt/gcc/"
# gcc build configures
GCC_CONFIGURE_WITH='--enable-checking=release --enable-languages=c,c++'
# mirror url
MIRRORS_URL="http://mirror.linux-ia64.org/gnu/gcc"
# arrchitecture
ARCH=`uname -m`
# install zip unzip bzip2 tar
yum -y install zip unzip bzip2 tar
# check gcc version
yum -y install gcc gcc-c++
GCC_INSTALL_VERSION=`gcc --version|grep -P 'gcc'|awk '{print $3}'`
if [ -n $GCC_INSTALL_VERSION ]; then
VERSION_FIRST_NUM=`echo $GCC_INSTALL_VERSION | cut -d '.' -f1`
if [ "$VERSION_FIRST_NUM" -gt 9 ]; then
echo -e "g++ installed "
echo `g++ --version`
else
GCC_INSTALL_VERSION=""
fi
fi
if [ -z $GCC_INSTALL_VERSION ]; then
cd $SRC_PATH
if [ ! -d "gcc" ];then
mkdir gcc
fi
cd gcc
if [ ! -e "gcc-$GCC_VERSION.tar.gz" ]; then
#download from https://mirror.linux-ia64.org/gnu/gcc/releases/gcc-9.3.0/gcc-9.3.0.tar.gz
echo "download gcc-$GCC_VERSION.tar.gz ";
wget $MIRRORS_URL/releases/gcc-$GCC_VERSION/gcc-$GCC_VERSION.tar.gz 2>&1
fi
if [ ! -e "gcc-$GCC_VERSION.tar.gz" ]; then
echo "download gcc-$GCC_VERSION.tar.gz fail! "
exit
fi
if [ -d "gcc-$GCC_VERSION" ]; then
rm -rf gcc-$GCC_VERSION
fi
#decompression
echo "decompression gcc-$GCC_VERSION.tar.gz ";
tar -zxf gcc-$GCC_VERSION.tar.gz
if [ ! -d "gcc-$GCC_VERSION" ]; then
echo "gcc-$GCC_VERSIONdir is not exists "
exit
fi
echo "install gcc gcc-c++ dependence"
yum install -y gcc-c++ ntpdate m4
ntpdate -u ntp.api.bz
cd gcc-$GCC_VERSION
# download configure file
echo "download pre requisites"
if [ ! -d "gmp-6.1.0"] || [ ! -d "mpfr-3.1.4" ] || [ ! -d "mpc-1.0.3" ]; then
./contrib/download_prerequisites
fi
echo "create soft link"
ln -sf gmp-6.1.0 gmp
ln -sf mpfr-3.1.4 mpfr
ln -sf mpc-1.0.3 mpc
echo "install gcc"
# disable multilib
if [ $ARCH = x86_64 ]; then
GCC_CONFIGURE_WITH=$GCC_CONFIGURE_WITH' --disable-multilib'
fi
mkdir gcc-make-tmp
cd gcc-make-tmp
echo "../configure --prefix=$INSTALL_BASE$GCC_VERSION $GCC_CONFIGURE_WITH"
../configure --prefix=$INSTALL_BASE$GCC_VERSION $GCC_CONFIGURE_WITH 2>&1
make -j4 2>&1
make install 2>&1
# check install status
if [ ! -d "$INSTALL_BASE$GCC_VERSION"] || [ -z "`ls $INSTALL_BASE$GCC_VERSION`" ]; then
echo -e "[error] install gcc-$GCC_VERSION fail!"
exit
else
if [ ! -e "/etc/ld.so.conf.d/libgcc-$ARCH.conf" ]; then
echo "$INSTALL_BASE$GCC_VERSION/lib64" >> /etc/ld.so.conf.d/libgcc-$ARCH.conf
fi
# mv py file to parent dir
for PY_FILE in `find $INSTALL_BASE$GCC_VERSION/lib64/ -name "*.py"`
do
if [ -n "$PY_FILE" ] && [ -e "$PY_FILE" ]; then
echo "mv $PY_FILE $INSTALL_BASE$GCC_VERSION"
mv $PY_FILE $INSTALL_BASE$GCC_VERSION
fi
done
if [ -z "`cat /etc/profile|grep 'export PATH=$PATH:'"$INSTALL_BASE$GCC_VERSION/bin"`" ]; then
echo 'export PATH=$PATH:'"$INSTALL_BASE$GCC_VERSION/bin" >> /etc/profile
fi
yum remove -y gcc
yum remove -y gcc-c++
cd ../..
rm -rf gcc-$GCC_VERSION
ldconfig
source /etc/profile
echo -e "gcc-$GCC_VERSION installed"
sleep 5s
fi
fi

#yum install -y java-1.8.0-openjdk-devel
#echo -e "openjdk 8 installed"

yum install -y readline-devel
echo -e "readline-devel installed"

yum install -y libcurl-devel openssl-devel
echo -e "libcurl-devel installed"

if [ $ARCH = x86_64 ]; then
if [ -n "`yum list |grep -P 'pkgconfig.x86_64'`" ]; then
yum install -y pkgconfig.x86_64
else
yum install -y pkgconf.x86_64
fi
else
yum install -y pkgconf
fi

echo -e "pkg-config installed"

yum install -y libuuid-devel
echo -e "uuid installed"

# boost-devel 1.74(or larger) can not be installed by `yum`, so we install it from source code.
# echo -e "\033[43;35m Please install boost 1.74 \033[0m"
BOOSTFILES=`ls /usr/local/lib/ |grep -Po 'libboost_(thread|system|regex|filesystem).so.1.74.0' -wc`
if [ $BOOSTFILES == "4" ]; then
echo -e "boost-1.74 installed"
else
cd $SRC_PATH
if [ ! -d "boost" ];then
mkdir boost
fi
cd boost
if [ ! -e "boost_1_74_0.tar.gz" ]; then
wget http://sourceforge.net/projects/boost/files/boost/1.74.0/boost_1_74_0.tar.gz 2>&1
fi
if [ ! -e "boost_1_74_0.tar.gz" ]; then
echo "download boost_1_74_0.tar.gz fail!"
exit
fi
if [ -d "boost_1_74_0" ]; then
rm -rf boost_1_74_0
fi
#decompression
echo "decompression boost_1_74_0.tar.gz"
tar -xzvf boost_1_74_0.tar.gz
cd boost_1_74_0
# by default: /usr/local/include and /usr/local/lib
./bootstrap.sh --with-libraries=system,regex,thread,filesystem
./b2 -j4
./b2 install
cd ..
rm -rf boost_1_74_0
echo -e "boost-1.74 installed"
sleep 5s
fi

if [ -n "`cmake -version|grep -P '3.23.2'`" ]; then
echo -e "cmake installed"
echo `cmake -version`
else
cd $SRC_PATH
if [ ! -d "cmake" ];then
mkdir cmake
fi
cd cmake
if [ ! -e "cmake-3.23.2.tar.gz" ]; then
wget https://cmake.org/files/v3.23/cmake-3.23.2.tar.gz 2>&1
fi
if [ ! -e "cmake-3.23.2.tar.gz" ]; then
echo "download cmake-3.23.2.tar.gz fail!"
exit
fi
#decompression
echo "decompression cmake-3.23.2.tar.gz"
tar -xvf cmake-3.23.2.tar.gz
cd cmake-3.23.2
./bootstrap
make -j4
make install
echo -e "cmake 3.23.2 installed"
cd ..
rm -rf cmake-3.23.2
sleep 5s
fi

yum install -y jemalloc
if [ ! -e "/usr/lib64/libjemalloc.so" ] && [ -n "`ls /usr/lib64 |grep -P 'libjemalloc.so.\d+$'`" ]; then
LIB_JEMALLOC=echo "`ls /usr/lib64 |grep -P 'libjemalloc.so.\d+$'`"
ln -sf /usr/lib64/$LIB_JEMALLOC /usr/lib64/libjemalloc.so
if [ ! -e "/etc/ld.so.preload" ] || [-z "`cat /etc/ld.so.preload|grep '/usr/lib64/libjemalloc.so'`"]; then
echo "/usr/lib64/libjemalloc.so" >> /etc/ld.so.preload
fi
echo -e "jemalloc installed"
else
if [ -e "/usr/lib64/libjemalloc.so" ]; then
echo -e "jemalloc installed"
else
cd $SRC_PATH
if [ ! -d "jemalloc" ];then
mkdir jemalloc
fi
cd jemalloc
if [ ! -e "jemalloc-5.3.0.tar.bz2" ]; then
wget http://sourceforge.net/projects/jemalloc.mirror/files/5.3.0/jemalloc-5.3.0.tar.bz2 2>&1
fi
if [ ! -e "jemalloc-5.3.0.tar.bz2" ]; then
echo "download jemalloc-5.3.0.tar.bz2 fail!"
exit
fi
#decompression
echo "decompression jemalloc-5.3.0.tar.bz2"
tar -jxf jemalloc-5.3.0.tar.bz2
cd jemalloc-5.3.0
yum -y install autoconf
./autogen.sh
make build_lib_shared
make install_lib_shared
cd ..
rm -rf jemalloc-5.3.0
if [ -e "/usr/local/lib/libjemalloc.so" ]; then
if [ ! -e "/etc/ld.so.preload" ] || [ -z "`cat /etc/ld.so.preload|grep '/usr/local/lib/libjemalloc.so'`" ]; then
echo "/usr/local/lib/libjemalloc.so" >> /etc/ld.so.preload
fi
echo -e "jemalloc installed"
else
echo -e "\033[43;35m [ERROR]failed to install jemalloc, please install it manually \033[0m \n"
fi
fi
fi

echo -e "Optional: requests for python api, pthreads and curl-devel for php api, realpath for gconsole, ccache for faster compilation\n"
echo -e "For help: https://github.com/pkumod/gStore/blob/master/docs/DEMAND.md"
sleep 5s

if [ ! -e "/etc/ld.so.conf.d/libboost-$ARCH.conf" ]; then
echo "/usr/local/lib" >> /etc/ld.so.conf.d/libboost-$ARCH.conf
fi
if [ -z "`cat ~/.bash_profile|grep 'CPATH=/usr/local/include'`" ]; then
echo "export CPATH=/usr/local/include" >> ~/.bash_profile
source ~/.bash_profile
ldconfig
fi
cd $CURRENT_PATH
echo -e "dynamic libraries path set"
echo -e "\033[43;35m Please run [ldconfig -v] again if you install other softwares \033[0m \n"
sleep 5s

#NOTICE: in Linux both threads and processes are created by vfork() in kernel with different sharing options.
#As a result, thread will consume PID and must be counted in the noproc number
echo "*    -    nofile    65535" >> /etc/security/limits.conf
echo "*    -    noproc    65535" >> /etc/security/limits.conf
#* means [for all users]，noproc means [maximum prcess number]，nofile means [maximum number of open files]
#- means [restrictions on both soft and hard]
echo -e "system environment variables set"

echo -e "when running program if you get a [can not find -lxxx] prompt, please search this dynamic library by [ldconfig -p|grep xxx]"


#sleep 5s
# compile the gStore system here
#logout  # exit from root account
#exit
#make


# colored output: https://blog.csdn.net/david_dai_1108/article/details/70478826

