## System Requirements

| Project          | Requirement                                                  |
| :--------------- | :----------------------------------------------------------- |
| Operating System | Linux, such as CentOS, Ubuntu etc.                           |
| Framework        | x86_64                                                       |
| Disk Size        | Depends on the size of the data set                          |
| Memory Size      | Depends on the size of the data set                          |
| glibc            | Must install version >= 2.14                                 |
| gcc              | Must install version >= 5.0                                  |
| g++              | Must install version >= 5.0                                  |
| make             | Must install                                                 |
| cmake            | Must install version >= 3.17                                  |
| pkg-config       | Must install                                                 |
| uuid-devel       | Must install                                                 |
| boost            | Must install version >= 1.56 && <= 1.59                      |
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

## Installation Environment

Running the corresponding scripts in scripts/setup/ for your operating system will automatically solve most of your problems for you. For example, if you are an Ubuntu user, you can execute the following command:  

```bash
$ . scripts/setup/setup_ubuntu.sh
```

**Before running the script, **we recommend you install gcc and g++ 5.0 or later.

Of course, you can also choose to manually prepare the environment step-by-step; Detailed installation instructions for each system requirement are provided below.

```bash
$ sudo -i
Please input your password
[sudo] xxx's password：
```

### gcc and g++ installation

Check g++ version：

```bash
$ g++ --version
```

If the version is earlier than 5.0, reinstall 5.0 or later version. Using 5.4.0 as an example :(for Ubuntu and CentOS) 

```bash
$ wget http://ftp.tsukuba.wide.ad.jp/software/gcc/releases/gcc-5.4.0/gcc-5.4.0.tar.gz
$ tar xvf gcc-5.4.0.tar.gz 
$ cd gcc-5.4.0
$ ./contrib/download_prerequisites
$ cd .. 
$ mkdir gcc-build-5.4.0 
$ cd gcc-build-5.4.0 
$ ../gcc-5.4.0/configure --prefix=/opt/gcc-5.4.0 --enable-checking=release --enable-languages=c,c++ --disable-multilib
$ sudo make -j4   #Allows four compile commands to be executed simultaneously, speeding up the compilation process
$ sudo make install
```

Ubuntu can also be installed directly using the following commands:

```bash
$ apt install -y gcc-5 g++-5
```

After successful installation,

- **You need to change the default versions of GCC and g++** If gcc and g++ 5.0 or later are installed in the '/prefix/bin' directory, run the following command：

  ```bash
  $ export PATH=/prefix:$PATH
  ```

- **The dynamic link library path needs to be modified: **If gcc and g++ dynamic link libraries above 5.0 are in the '/prefix/lib' path, you need to run the following comman：

  ```bash
  $ export LD_LIBRARY_PATH=/prefix/lib:$LD_LIBRARY_PATH
  ```

### readline-devel installation

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

### boost installation（Please use 1.56-1.59）

Check whether boost is installed 

```bash
$ yum list installed | grep boost	    # CentOS
$ dpkg -s boost					        # Ubuntu
```

If not, install it：（use version 1.56.0 as example）

version:1.56.0

address: http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz

Installation script: (for CentOS and Ubuntu)

```bash
$ wget http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz$ tar -xzvf boost_1_56_0.tar.gz$ cd boost_1_56_0$ ./bootstrap.sh$ sudo ./b2$ sudo ./b2 install
```

Ubuntu can also be installed directly using the following commands:

```bash
$ sudo apt install -y libboost-all-dev
```

**Note: please install boost after ensuring that the g++ version is above 5.0** Undefined reference to 'boost::...' - undefined reference to 'boost::... '"), most likely because you compiled Boost with GCC versions lower than 5.0. At this point, recompile Boost using the following step:

- Clear old files: `./b2 --clean-all`
- In the user-config.jam file under./tools/build/ SRC (if this file does not exist under this path, Please find a sample user-config.jam file under./tools/build/example or some other directory and copy it to./tools/build/ SRC) to add  ：`using gcc : 5.4.0 : gcc-5.4.0's path ;`
- Run under ./  `./bootstrap.sh --with-toolset=gcc`
- `sudo ./b2 install --with-toolset=gcc`

Then recompile gStore (please start from 'make pre')

After successful installation，

- **Need to modify the dynamic link library path:** Assuming boost's dynamically linked library is in the '/prefix/lib' path, you need to execute the following command：

  ```bash
  $ export LD_LIBRARY_PATH=/prefix/lib:$LD_LIBRARY_PATH
  ```

- **The header file path needs to be changed: **Assuming boost's header file is in the '/prefix/include' path, you need to execute the following command:

  ```bash
  $ export CPATH=/prefix/include:$CPATH
  ```

### curl-devel installation 

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

### openssl-devel installation

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

### cmake installation

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
$ make
$ make install
```

Ubuntu can also be installed directly using the following commands:

```bash
$ sudo apt install -y cmake
```

### pkg-config installation

Check whether pkg-config is installed

```bash
$ pkg-config --version		# CentOS
$ pkg-config --version		# Ubuntu
```

if not, install

```bash
$ sudo yum install pkgconfig.x86_64         # CentOS
$ sudo apt install -y pkg-config            # Ubuntu
```

### uuid-devel installation

Check whether uuid-devel is installed 

```bash
$ yum list installed | grep libuuid-devel    # CentOS
$ dpkg -s uuid-dev                           # Ubuntu
```

if not, install

```bash
$ sudo yum install libuuid-devel	    # CentOS
$ sudo apt install -y uuid-dev		    # Ubuntu
```

### unzip/bzip2 installation (Optional)

Used to decompress zip and bz2 packages during an offline installation of gStore (where there is no Internet connection and all dependencies are compressed packages); unnecessary during online installation.

Check whether unzip is installed:

```bash
$ yum list installed | grep unzip		# CentOS
$ dpkg -s unzip  						# Ubuntu
```

if not, install

```bash
$ sudo yum install -y unzip			# CentOS
$ sudo apt-get install unzip		# Ubuntu
```

Replace "unzip" with "bzip2" in the above commands to check whether bzip2 is installed and install it if necessary.

### OpenJDK installation (Optional)

OpenJDK is only necessary if you need to use the Java API. You can first check whether there is an existing JDK environment with the following commands; if there is an existing JDK environment, and its version is 1.8.X, re-installation is unnecessary.

```bash
$ java -version
$ javac -version
```

if there is no existing JDK 1.8.X environment, install

```bash
$ yum install -y java-1.8.0-openjdk-devel # CentOS
$ sudo apt install -y openjdk-8-jdk       # Ubuntu
```

<div STYLE="page-break-after: always;"></div>
### jemalloc installation

Determine whether jemalloc is installed

```bash
$ yum list installed | grep jemalloc		#centos system
$ dpkg -s libjemalloc-dev				#ubuntu system
```

If not installed, install it

```bash
$ yum install -y jemalloc 		           #centos system
$ apt-get install -y libjemalloc-dev    #ubuntu system
```



## gStore obtainment

If you encounter permission issues, please prefix the command with `sudo` .

### Method 1: download 

gStore has been uploaded to gitee (code cloud), which is recommended for faster download for users in mainland China. The website is https://gitee.com/PKUMOD/gStore.
You can also open https://github.com/pkumod/gStore, download gStore.zip, then decompress the zip package.

### Method 2: clone (Recommended)

Run the following code to clone:

```bash
$ git clone https://gitee.com/PKUMOD/gStore.git  # gitee (code cloud) faster for users in mainland China
$ git clone https://github.com/pkumod/gStore.git # GitHub

# using the -b parameter when getting historical versions, such as -b 0.9.1
$ git clone -b 0.9.1 https://gitee.com/PKUMOD/gStore.git
$ git clone -b 0.9.1 https://github.com/pkumod/gStore.git
```

Note: Git need to be installed first.

```bash
$ sudo yum install git		# CentOS
$ sudo apt-get install git	# Ubuntu
```

## gStore compilation

Switch to the gStore directory:

```bash
$ cd gStore
```

Execute the following commands:

#### Method 1

```bash
$ sudo bash scripts/build.sh
#If the compilation completes successfully, the following string will appear at the end: Compilation ends successfully!
```

#### Method 2
```bash
# Create build directory
$ mkdir build
$ cd build
# Generate makefiles and other files
$ cmake ..
# precompile
$ make pre
# compile
$ make -j4
# Initialize the system.db database
$ make init
```

If 'make pre' still asks for g++ 5.0 or later after installing g++ 5.0 or later, locate g++ 5.0 or later and
run the following command in the gStore directory:

```bash
$ export CXX=<5.0 or later g++'s path>
```

Then `make pre` again. If the same error is reported after this step, please manually delete `CMakeCache.txt` and the `CMakeFiles` directory under ` tools/antlr4-cpp-runtime-4/` and `make pre ` again.

#### 针对开发人员使用

```bash
$ cd build
# compile
$ make -j4
# Test script
$ make test
# Clear lib,include files generated by 3rdparty precompilation (Run this command only when 3rdparty changes).
$ make clean_pre
# Clean up files such *.o generated by compilation
$ make clean
# test update_test gtest script (equivalent to make test)
$ cd scripts/test
$ bash test.sh
```

<!-- <div STYLE="page-break-after: always;"></div> -->
## Deploy gStore using Docker 

> We provide two ways to deploy gStore from containers:  
>
> One is to build it yourself from the Dockerfile file in the project root and then run the container.  
>
> Another option is to download the image that has been automatically built and run it directly.  

### environment preparation

Docker, refer to the address [docker](https://blog.csdn.net/A632189007/article/details/78662741)

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



## Build the image from Dockerfile 

### Build an image named gstore

```bash
# build docker image
build -t gstore 

# show docker image
docker image list
```

 As shown below

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-docker.jpg)

### Start the locally built image

```shell
# Map host 9999 port to container 9000 port(Default port for the gstore API service) 
# a5d51ff4f121 is image id
docker run -itd -p 9999:9000 a5d51ff4f121 /bin/bash

# Show container status
docker ps -a
#CONTAINER ID   IMAGE          COMMAND                  CREATED         STATUS         PORTS                                       NAMES
#317e4ce0a667   a5d51ff4f121   "bash /docker-entryp…"   6 minutes ago   Up 6 minutes   0.0.0.0:9999->9000/tcp, :::9999->9000/tcp   awesome_greider
```

### Check API Service

```bash
curl http://127.0.0.1:9999 -X POST -H 'Content-Type: application/json' -d '{"operation":"check"}'

# If the following information is displayed on the console, the gstore http api service starts normally
# {"StatusCode":0,"StatusMsg":"the ghttp server is running..."}
```

------

There are probably a lot of other things that need to be added, so for now I've just added a basic version. The basic environment build is just the first step in containerization.	