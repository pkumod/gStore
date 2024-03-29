### 系统要求

| 项目           | 需求                                           |
| :------------- | ---------------------------------------------- |
| 操作系统       | Linux, 例如CentOS, Ubuntu等                    |
| 架构           | x86_64, AMD64                                  |
| 磁盘容量       | 根据数据集的大小                               |
| 内存大小       | 根据数据集的大小                               |
| glibc          | 必须安装 version >= 2.14                       |
| gcc            | 必须安装 version >= 5.0                        |
| g++            | 必须安装 version >= 5.0                        |
| make           | 必须安装                                       |
| cmake          | 必须安装 version >=3.6                         |
| pkg-config     | 必须安装                                       |
| uuid-devel     | 必须安装                                       |
| boost          | 必须安装 version >= 1.56 && <= 1.59            |
| readline-devel | 必须安装                                       |
| curl-devel     | 必须安装                                       |
| openssl-devel  | 1.0版本及以上，必须安装                        |
| jemalloc       | 1.2版本及以上，必须安装                        |
| openjdk        | 如果使用Java api，则需要 version = 1.8.x       |
| requests       | 如果使用Python http api，则需要                |
| node           | 如果使用Nodejs http api则需要 version >=10.9.0 |
| pthreads       | 如果使用php http api，则需要                   |
| realpath       | 如果使用gconsole，则需要                       |
| ccache         | 可选，用于加速编译                             |

### 安装环境准备

根据您的操作系统运行 scripts/setup/ 中相应的脚本能够自动为您解决大部分问题。比如，若您是 Ubuntu 用户，可执行以下指令：

```bash
$ . scripts/setup/setup_ubuntu.sh
```

**在运行脚本之前，**建议先安装 5.0 以上版本的 gcc 和 g++ 。

当然，您也可以选择手动逐步准备环境；下面提供了各系统要求的详细安装指导。

在执行安装命令之前，建议执行如下命令临时切换到超级用户权限模式：

```bash
$ sudo -i
请输入密码
[sudo] xxx 的密码：
```

####.1 gcc 和 g++ 安装

检查 g++ 版本：

```bash
$ g++ --version
```

若版本低于 5.0, 则重新安装 5.0 以上版本。以安装 5.4.0 为例：（适用于 Ubuntu 和 CentOS ）

```bash
$ wget http://ftp.tsukuba.wide.ad.jp/software/gcc/releases/gcc-5.4.0/gcc-5.4.0.tar.gz
$ tar xvf gcc-5.4.0.tar.gz
$ mv gcc-5.4.0 /opt
$ cd /opt/gcc-5.4.0
$ ./contrib/download_prerequisites
$ cd .. 
$ mkdir gcc-build-5.4.0 
$ cd gcc-build-5.4.0 
$ ../gcc-5.4.0/configure --prefix=/opt/gcc-5.4.0 --enable-checking=release --enable-languages=c,c++ --disable-multilib
$ make -j4   #允许4个编译命令同时执行，加速编译过程
$ make install
```

Ubuntu 也可直接使用以下命令安装：

```bash
$ apt install -y gcc-5 g++-5
```

安装成功后，

- **需要修改 gcc 和 g++ 的默认版本：**假设 5.0 以上版本的 gcc 和 g++ 安装在了`/prefix/bin`路径下，则需要执行以下命令：

  ```bash
  $ export PATH=/prefix:$PATH
  ```

- **需要修改动态链接库路径：**假设 5.0 以上版本的 gcc 和 g++ 动态链接库在`/prefix/lib`路径下，则需要执行以下命令：

  ```bash
  $ export LD_LIBRARY_PATH=/prefix/lib:$LD_LIBRARY_PATH
  ```

####.2 readline-devel 安装

判断 readline 是否安装

```bash
$ yum list installed | grep readline-devel	#centos系统
$ dpkg -s libreadline-dev					#ubuntu系统
```

如果没有安装，则安装

```bash
$ yum install -y readline-devel	    #centos系统
$ apt install -y libreadline-dev	#ubuntu系统
```

####.3 boost 安装

请使用1.56-1.59版本进行安装

判断 boost 是否安装

```bash
$ yum list installed | grep boost	    #centos系统
$ dpkg -s libboost-all-dev				#ubuntu系统
```

如果没有安装，则安装：（以版本 1.56.0 为例）

版本:1.56.0

地址：http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz

安装脚本： （适用于 CentOS 和 Ubuntu）

```bash
$ wget http://sourceforge.net/projects/boost/files/boost/1.56.0/boost_1_56_0.tar.gz
$ tar -xzvf boost_1_56_0.tar.gz
$ cd boost_1_56_0
$ ./bootstrap.sh
$ ./b2
$ ./b2 install
```

Ubuntu 也可直接使用以下命令安装：

```bash
$ apt install -y libboost-all-dev
```

**注意：请在确保 g++ 版本高于 5.0 后安装 boost 。**若在编译 gStore 时遇到与 boost 链接错误（形如 "undefined reference to `boost::...`"），很可能是因为您使用低于 5.0 的 gcc 版本编译 boost 。此时，请使用以下步骤重新编译 boost ：

- 清除旧文件：`./b2 --clean-all`
- 在 ./tools/build/src 下的 user-config.jam 文件中（若此路径下不存在此文件，请在 ./tools/build/example 或其他路径下找到一个示例 user-config.jam 文件并拷贝到 ./tools/build/src 下）添加：`using gcc : 5.4.0 : gcc-5.4.0的路径 ;`
- 在 ./ 下运行 `./bootstrap.sh --with-toolset=gcc`
- `./b2 install --with-toolset=gcc`

然后重新编译 gStore （请从 `make pre` 开始重做）。

安装成功后，

- **需要修改动态链接库路径：**假设 boost 的动态链接库在`/prefix/lib`路径下，则需要执行以下命令：

  ```bash
  $ export LD_LIBRARY_PATH=/prefix/lib:$LD_LIBRARY_PATH
  ```

- **需要修改头文件路径：**假设 boost 的头文件在`/prefix/include`路径下，则需要执行以下命令：

  ```bash
  $ export CPATH=/prefix/include:$CPATH
  ```

####.4 curl-devel 安装 

判断 curl 是否安装

```bash
$ yum list installed | grep libcurl-devel    #centos系统
$ dpkg -s libcurl4-openssl-dev		         #ubuntu系统
```

如果没有安装，则安装：

版本：7.55.1

地址：https://curl.haxx.se/download/curl-7.55.1.tar.gz

安装脚本（适用于 CentOS 和 Ubuntu）

```bash
$ wget https://curl.haxx.se/download/curl-7.55.1.tar.gz
$ tar -xzvf  curl-7.55.1.tar.gz
$ cd curl-7.55.1
$ ./configure
$ make
$ make install  
```

或者直接用下面命令安装

```bash
$ yum install -y curl libcurl-devel              #centos系统
$ apt-get install -y curl libcurl4-openssl-dev   #ubuntu系统
```

####.5 openssl-devel 安装

判断 openssl-devel 是否安装

```shell
$ yum list installed | grep openssl-devel    #centos系统
$ dpkg -s libssl-dev                         #ubuntu系统
```

如果没有安装，则安装

```shell
$ yum install -y openssl-devel               #centos系统          
$ apt-get install -y libssl-dev              #ubuntu系统
```

####.6 cmake 安装 

判断 cmake 是否安装

```bash
$ cmake --version				#centos系统
$ cmake --version				#ubuntu系统
```

如果没有安装，则安装：

版本：3.6.2

地址：https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz

安装脚本 （适用于 CentOS 和 Ubuntu）

```bash
$ wget https://cmake.org/files/v3.6/cmake-3.6.2.tar.gz
$ tar -xvf cmake-3.6.2.tar.gz && cd cmake-3.6.2/
$ ./bootstrap
$ make
$ make install
```

Ubuntu 也可直接使用以下命令安装：

```bash
$ apt install -y cmake
```

####.7 pkg-config 安装

判断 pkg-config 是否安装

```bash
$ pkg-config --version		#centos系统
$ pkg-config --version		#ubuntu系统
```

如果没有安装，则安装

```bash
$ yum install -y pkgconfig.x86_64      #centos系统
$ apt install -y pkg-config            #ubuntu系统
```

####.8 uuid-devel 安装

判断 uuid-devel 是否安装

```bash
$ yum list installed | grep libuuid-devel    #centos系统
$ dpkg -s uuid-dev                           #ubuntu系统
```

如果没有安装，则安装

```bash
$ yum install -y libuuid-devel	    #centos系统
$ apt install -y uuid-dev		    #ubuntu系统
```

####.9 unzip/bzip2 安装（选装）

用于离线安装时解压 gStore zip包和bz2包，如果采用在线安装方式可不用安装。

判断 unzip是否安装

```bash
$ yum list installed | grep unzip		#centos系统
$ dpkg -s unzip  						#ubuntu系统
```

如果没有安装，则安装

```bash
$ yum install -y unzip	            #centos系统
$ apt-get install -y unzip         #ubuntu系统
```

将上述命令中的unzip改为bzip2，即可判断bzip2是否安装、并安装之。

####.10 openjdk 安装（选装）

如果需要使用java api接口则需要安装，安装前可通过`java -version` 和 `javac -verison`查看是否已经存在JDK环境，若存在且版本为1.8.X可不用再安装。

判断 jdk 是否安装

```bash
$ java -version
$ javac -version
```

如果没有安装，则安装

```bash
$ yum install -y java-1.8.0-openjdk-devel     #centos系统
$ apt install -y openjdk-8-jdk           #ubuntu系统
```

####.11 jemalloc安装

判断 jemalloc是否安装

```bash
$ yum list installed | grep jemalloc		#centos系统
$ dpkg -s libjemalloc-dev				#ubuntu系统
```

如果没有安装，则安装

```bash
$ yum install -y jemalloc 		           #centos系统
$ apt-get install -y libjemalloc-dev    #ubuntu系统
```

###  gStore获取 

---

如果遇到权限问题，请在命令前加 `sudo` 。

####.1 方式一：download

gStore目前已经上传到gitee（码云），国内用户推荐使用码云下载，速度更快，网址为 https://gitee.com/PKUMOD/gStore

也可打开 https://github.com/pkumod/gStore ，下载gStore.zip；解压zip包。

####.2 方式二：clone (推荐)

通过如下命令 clone:

```bash
$ git clone https://gitee.com/PKUMOD/gStore.git # gitee(码云) 国内下载速度更快
$ git clone https://github.com/pkumod/gStore.git # GitHub

# 拉取历史版本时可使用-b参数,如-b 0.9.1
$ git clone -b 0.9.1 https://gitee.com/PKUMOD/gStore.git
$ git clone -b 0.9.1 https://github.com/pkumod/gStore.git
```

注意：这一方法需要先安装 Git 。

```bash
$ sudo yum install git		# CentOS系统
$ sudo apt-get install git	# Ubuntu系统
```



### gStore编译

---

切换到 gStore 目录下：

```bash
$ cd gstore
```

执行如下指令：

```bash
$ make pre
# 以下命令可通过 -j4方式加速编译
$ make            
#若编译顺利完成，最后会出现 Compilation ends successfully! 结果
```

如果在已安装 5.0 以上版本的 g++ 后 `make pre` 仍报要求 5.0 以上版本 g++ 的错误，请先定位 5.0 以上版本 g++ 的路径，并在 gStore 目录下执行以下命令：

```bash
$ export CXX=<5.0 以上版本g++的路径>
```

然后重新 `make pre` 。假如在这步操作后仍然报相同的错误，请手动删除 `tools/antlr4-cpp-runtime-4/` 下的 `CMakeCache.txt` 和 `CMakeFiles` 文件夹，再重新 `make pre` 。