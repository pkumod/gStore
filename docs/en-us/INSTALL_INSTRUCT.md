## System Requirements

| Project          | Requirement                                                  |
| :--------------- | :----------------------------------------------------------- |
| Operating System | Linux, such as CentOS, Ubuntu etc.                           |
| Framework        | x86_64, amd64, arm64, aarch64, loongarch64                   |
| Disk Size        | Depends on the size of the data set                          |
| Memory Size      | Depends on the size of the data set                          |
| unzip/bzip2      | Must install                                                 |
| glibc            | Must install version >= 2.14                                 |
| gcc              | Must install , recommend version >= 9.3.0                    |
| g++              | Must install , recommend version >= 9.3.0                    |
| make             | Must install                                                 |
| cmake            | Must install version >= 3.23.2                               |
| pkg-config       | Must install                                                 |
| uuid-devel       | Must install                                                 |
| boost            | Must install , recommend version >= 1.74                     |
| readline-devel   | Must install                                                 |
| curl-devel       | Must install                                                 |
| openssl-devel    | Must install version >= 1.1                                  |
| jemalloc         | 1.2 version and above, must be installed                     |
| openjdk-devel    | If using the Java API, must install version = 1.8.x          |
| requests         | If using the Python http API, must install                   |
| node             | If using the Node.js http API, must install version >=10.9.0 |
| pthreads         | If using the PHP http API, must install                      |
| realpath         | If using gconsole, must install                              |
| ccache           | Optional, used to speed up compilation                       |

## Compilation and Installation 

### Installation Environment

Running the corresponding scripts in scripts/setup/ for your operating system will automatically solve most of your problems for you(recommend). For example, if you are an Ubuntu user, you can execute the following command:  

```bash
$ . scripts/setup/setup_ubuntu.sh
```

If you are a Centos user,you can execute the following command:

```shell
$ . scripts/setup/setup_centos.sh
```

**Before running the script, **we recommend you install gcc and g++ 5.0 or later.

Of course, you can also choose to manually prepare the environment step-by-step; Detailed installation instructions for each system requirement are provided below.

```bash
$ sudo -i
Please input your password
[sudo] xxx's password：
```

#### unzip/bzip2 installation

Used for extracting the gStore zip and bz2 packages during offline installation

Determine if unzip is installed

```bash
$ yum list installed | grep unzip		#centos系统
$ dpkg -s unzip  						#ubuntu系统
```

If not installed ,install

```bash
$ yum install -y unzip	            #centos系统
$ apt-get install -y unzip         #ubuntu系统
```

Change the unzip in the above command to bzip2 can install bzip2

#### gcc and g++ installation

Check g++ version：

```bash
$ g++ --version
```

Using 9.3.0 as an example :(for Ubuntu and CentOS) 

```bash
$ cd /var/local/
$ mkdir gcc
$ cd gcc
$ wget https://mirror.linux-ia64.org/gnu/gcc/releases/gcc-9.3.0/gcc-9.3.0.tar.gz 2>&1
$ tar -zxf gcc-9.3.0.tar.gz
$ yum install -y gcc-c++ unzip bzip2 ntpdate m4
$ ntpdate -u ntp.api.bz
$ cd gcc-9.3.0
$ ./contrib/download_prerequisites
$ ln -sf gmp-6.1.0 gmp
$ ln -sf mpfr-3.1.4 mpfr
$ ln -sf mpc-1.0.3 mpc
$ mkdir gcc-make-tmp
$ cd gcc-make-tmp
$ ../configure --prefix=/opt/gcc/9.3.0 --enable-checking=release --enable-languages=c,c++ --disable-multilib 2>&1
$ make -j4 2>&1
$ make install 2>&1
$ echo "/opt/gcc/9.3.0/lib64" >> /etc/ld.so.conf.d/libgcc-x86_64.conf
$ mv /opt/gcc/9.3.0/lib64/libstdc++.so.6.0.28-gdb.py /opt/gcc/9.3.0
$ echo 'export PATH=$PATH:'"/opt/gcc/9.3.0/bin" >> /etc/profile
$ yum remove -y gcc
$ yum remove -y gcc-c++
$ cd ../../
$ rm -rf gcc-9.3.0
$ ldconfig
$ source /etc/profile
```

Ubuntu can also be installed directly using the following commands:

```bash
$ apt install -y gcc-9 g++-9
```

#### readline-devel installation

Check whether readline-devel is installed

```bash
$ yum list installed | grep readline-devel	# CentOS
$ dpkg -s readline						# Ubuntu
```

if not, install it

```bash
$ sudo yum install readline-devel		# CentOS
$ sudo apt install -y libreadline-dev	# Ubuntu
```

#### pkg-config installation

Check whether pkg-config is installed

```bash
$ pkg-config --version		#centos
$ pkg-config --version		#ubuntu
```
if not , install it

```bash
$ yum install -y pkgconfig.x86_64      #centos
$ apt install -y pkg-config            #ubuntu
```
#### uuid-devel installation

Check whether uuid-devel is installed

```bash
$ yum list installed | grep libuuid-devel    #centos
$ dpkg -s uuid-dev                           #ubuntu
```

if not, install it

```bash
$ yum install -y libuuid-devel	    #centos
$ apt install -y uuid-dev		    #ubuntu
```

#### boost installation

Recommend using version 1.74.0 to install

Check whether boost is installed 

```bash
$ yum list installed | grep boost	    # CentOS
$ dpkg -s boost					        # Ubuntu
```

If not, install it：（use version 1.74.0 as example）

version:1.74.0

address: http://sourceforge.net/projects/boost/files/boost/1.74.0/boost_1_74_0.tar.gz

Installation script: (for CentOS and Ubuntu)

```bash
$ wget http://sourceforge.net/projects/boost/files/boost/1.74.0/boost_1_74_0.tar.gz
$ tar -xzvf boost_1_74_0.tar.gz
$ cd boost_1_74_0
$ ./bootstrap.sh --with-libraries=system,regex,thread,filesystem
$ ./b2 -j4
$ ./b2 install
```
Ubuntu can also be installed directly using the following commands:

```bash
$ sudo apt install -y libboost-all-dev
```

**Note: please install boost after ensuring that the g++ version is above 9.3.0** Undefined reference to 'boost::...' - undefined reference to 'boost::... '"), most likely because you compiled Boost with GCC versions lower than 9.3.0. At this point, recompile Boost using the following step:

- Clear old files: `./b2 --clean-all`
- In the user-config.jam file under./tools/build/ SRC (if this file does not exist under this path, Please find a sample user-config.jam file under./tools/build/example or some other directory and copy it to./tools/build/ SRC) to add  ：`using gcc : 9.3.0 : gcc-9.3.0's path ;`
- Run under ./  `./bootstrap.sh --with-toolset=gcc`
- `sudo ./b2 install --with-toolset=gcc`

Then recompile gStore (please start from 'make pre')

```shell
# change directory
$ cd build
# clean 3rd lib
$ make clean_pre
# precompile again
$ make pre
# compile
$ make -j4
# init database
$ make init
```


#### curl-devel installation 

Check whether curl-devel is installed

```bash
$ yum list installed | grep readline-devel	# CentOS
$ dpkg -s readline						# Ubuntu
```

if not, then install：

version: 7.55.1

address: https://curl.haxx.se/download/curl-7.55.1.tar.gz

Installation scripts (for CentOS and Ubuntu)

```bash
$ wget https://curl.haxx.se/download/curl-7.55.1.tar.gz
$ tar -xzvf  curl-7.55.1.tar.gz
$ cd curl-7.55.1
$ ./configure
$ make
$ make install  
```

Or use the following command to install

```bash
$ yum install -y curl libcurl-devel              # CentOS
$ apt-get install -y curl libcurl4-openssl-dev   # Ubuntu
```

#### openssl-devel installation

Check whether openssl-devel is installed

```shell
$ yum list installed | grep openssl-devel    # CentOS
$ dpkg -s libssl-dev                         # Ubuntu
```

If not installed, install it

```shell
$ yum install -y openssl-devel                # CentOS          
$ apt-get install -y libssl-dev zlib1g-dev    # Ubuntu
```

#### cmake installation

Check whether cmake is installed

```bash
$ cmake --version				# CentOS
$ cmake --version				# Ubuntu
```

if not, install:

version: 3.23.2

address: https://curl.haxx.se/download/curl-7.55.1.tar.gz

Installation scripts (for CentOS and Ubuntu)

```bash
$ wget https://cmake.org/files/v3.23/cmake-3.23.2.tar.gz
$ tar -xvf cmake-3.23.2.tar.gz && cd cmake-3.23.2/
$ ./bootstrap
$ make -j4
$ make install
```

Ubuntu can also be installed directly using the following commands:

```bash
$ sudo apt install -y cmake
```

#### jemalloc installation

Determine whether jemalloc is installed

```bash
$ yum list installed | grep jemalloc		#Centos
$ dpkg -s libjemalloc-dev				#Ubuntu
```

If not installed, install it

```bash
$ yum install -y jemalloc 		           #Centos 
$ apt-get install -y libjemalloc-dev    #Ubuntu 
```

create soft link

```bash
# Locate the dynamic library, the default installation path is usually /usr/lib64
$ find / -name libjemalloc*
/usr/lib64/libjemalloc.so.1

# if /usr/lib64/libjemalloc.so not find，create soft link
$ cd /usr/lib64
$ ln -s libjemalloc.so.1 libjemalloc.so
```



<div STYLE="page-break-after: always;"></div>
### gStore obtainment

If you encounter permission issues, please prefix the command with `sudo` .

#### Method 1: download 

gStore has been uploaded to gitee (code cloud), which is recommended for faster download for users in mainland China. The website is https://gitee.com/PKUMOD/gStore.
You can also open https://github.com/pkumod/gStore, download gStore.zip, then decompress the zip package.

#### Method 2: clone (Recommended)

Run the following code to clone:

```bash
$ git clone https://gitee.com/PKUMOD/gStore.git  # gitee (code cloud) faster for users in mainland China
$ git clone https://github.com/pkumod/gStore.git # GitHub

# using the -b parameter when getting historical versions, such as -b 1.2
$ git clone -b 1.2 https://gitee.com/PKUMOD/gStore.git
$ git clone -b 1.2 https://github.com/pkumod/gStore.git
```

Note: Git need to be installed first.

```bash
$ sudo yum install git		# CentOS
$ sudo apt-get install git	# Ubuntu
```

### gStore compilation

Switch to the gStore directory:

```bash
$ cd gstore
```

There are two methods to compile gstore , compile by script (recommend) and compile manually

compile by script(recommend)

```bash
$ bash scripts/build.sh
#若编译顺利完成，最后会出现 Compilation ends successfully! 结果
```
compile manually

```bash
#change dirctory
$ cd gStore
# create build dirctory
$ mkdir build
$ cd build
# create the makefile  and other files
$ cmake ..
# precompile
$ make pre
# compile
$ make -j4
# init databases
$ make init
# the following result shows compilation and initalzation successful
---Compilation ends successfully!
---system.db is built successfully!
```

<!-- <div STYLE="page-break-after: always;"></div> -->

## Deploy gStore using Docker 

> We provide two ways to deploy gStore from containers:  
>
> One is to build it yourself from the Dockerfile file in the project root and then run the container.  
>
> Another option is to download the image that has been automatically built and run it directly.  

### environment preparation

Docker, refer to the address [docker](https://docs.docker.com/desktop/)

### Run by pulling the image directly(recommend)

Currently, the gstore image has been built and published on Docker Hub. The image is based on Ubuntu 16.04 and can be installed and deployed using the following methods.

**(1) Pull the docker image**

```bash
docker pull pkumodlab/gstore-docker:latest #Pull up the latest docker image
```

**(2) Run the image** 

View the list of Docker images with the following command

```
docker image list
```

As shown in the following figure：

![image-20221018223737566](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221018223737566.png)

Start the image using the following command

```shell
docker run -itd -p 9999:9000 9ca4388fc81e /bin/bash 
# Map container 9000 port to host 9999 port
# 9ca4388fc81e is image id
```

After the startup is complete, enter the following command to view container information

```shell
docker ps
```

![image-20221018224207124](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221018224207124.png)

Use the following command to enter the container

```shell
docker exec -it a5016bd46094 /bin/bash 
#It is recommended to use exec to enter the container, and the container will not stop after exiting
```

After entering the container, you can see the gstore directory in the root directory, which has already been compiled and installed.

 You can operate it like using local gstore. To exit the docker container, you can use the following command:

```shell
exit
```



### Build and run image locally

#### Build an image named gstore

```bash
# build docker image
build -t gstore 

# show docker image
docker image list
```

 As shown below

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-docker.jpg)

#### Start the locally built image

```shell
# Map host 9999 port to container 9000 port(Default port for the gstore API service) 
# a5d51ff4f121 is image id
docker run -itd -p 9999:9000 a5d51ff4f121 /bin/bash

# Show container status
docker ps -a
#CONTAINER ID   IMAGE          COMMAND                  CREATED         STATUS         PORTS                                       NAMES
#317e4ce0a667   a5d51ff4f121   "bash /docker-entryp…"   6 minutes ago   Up 6 minutes   0.0.0.0:9999->9000/tcp, :::9999->9000/tcp   awesome_greider
```

#### Check API Service

```bash
curl http://127.0.0.1:9999 -X POST -H 'Content-Type: application/json' -d '{"operation":"check"}'

# If the following information is displayed on the console, the gstore http api service starts normally
# {"StatusCode":0,"StatusMsg":"the ghttp server is running..."}
```

------

There are probably a lot of other things that need to be added, so for now I've just added a basic version. The basic environment build is just the first step in containerization.	

## Deploy gStore Using a Static Installation Package

### Static Package Download Links
We now offer four types of static installation packages for download and installation:

```
http://file.gstore.cn/f/eb7fa4c9d0d3421695f7/?dl=1   #linux-arm
http://file.gstore.cn/f/bb59558af0be44a6970d/?dl=1   #linux-x86
http://file.gstore.cn/f/158654beb04343ba9afe/?dl=1   #ubuntu-arm_64
http://file.gstore.cn/f/db5a9ac955ea45bfba27/?dl=1   #ubuntu-x86_64
```

### Download static installation package

```shell
$ wget --content-disposition http://file.gstore.cn/f/75643cb217604efca75b/?dl=1  #the address of the static package
$ tar -zxvf gstore-1.3-static-linux-arm.tar.gz  
```
### Initialize system

```shell
$ bin/ginit
```

## Version upgrade

### Method 1：Older versions are deployed as an upgrade by clone

#### Preparation before upgrade

```shell
# Go to the directory where gstore is deployed
$ cd gstore
# Pull the update script
$ git pull
# Check whether the version_checkout.sh file exists
$ ls scripts/version_checkout.sh
# If not, specify a branch fetch, such as 1.2
$ git status
1.2
$ git pull origin 1.2
# Check whether the upgrade version exists on the remote device, such as 1.3
$ git branch -r | grep origin/1.3
# Check whether there are conflicting branch names in the local area, such as 1.3(if yes, back up the branch and delete the conflicting branch)
$ git branch | grep 1.3
```

#### upgrade

```shell
# Go to the directory where gstore is deployed
$ cd gstore
# Specify the upgrade version and execute the script to upgrade, such as1.3
$ bash scripts/version_checkout.sh 1.3
# To choose whether to overwrite the database, enter y or n
the new version already has a xxx.db, overwrite [Y/n]
# Upgrade successfully
version update successfully .
```

Important data will be backed up. File directory: version+Version number+Date

### Method 2：Older versions are deployed by downloading zip packages or static packages

```shell
# Get the old version gstore path
$ pwd
/xxx/xxxx/gstore
# Unzip the update package and go to the gstore directory
$ cd gstore/
# Specify the path of the old version and execute the script to upgrade
$ bash scripts/version_update.sh /xxx/xxxx/gstore
# To choose whether to overwrite the database, enter y or n
the new version already has a xxx.db, overwrite [Y/n]
# Upgrade successfully
version update successfully .
```

