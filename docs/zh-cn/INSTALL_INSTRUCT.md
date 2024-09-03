## 系统要求

| 项目           | 需求                                           |
| :------------- | ---------------------------------------------- |
| 操作系统       | Linux, 例如CentOS, Ubuntu等                    |
| 架构           | x86_64, amd64, arm64, aarch64, loongarch64    |
| 磁盘容量       | 根据数据集的大小                               |
| 内存大小       | 根据数据集的大小                               |
| glibc          | 必须安装 version >= 2.14                       |
| unzip/bzip2    | 必须安装                                       |
| gcc            | 必须安装 推荐 version >= 9.3.0                 |
| g++            | 必须安装 推荐 version >= 9.3.0                 |
| make           | 必须安装                                       |
| cmake          | 必须安装 version >=3.23.2                      |
| readline-devel | 必须安装                                       |
| boost          | 必须安装 推荐 version >= 1.74                  |
| curl-devel     | 必须安装                                       |
| pkg-config     | 必须安装                                       |
| uuid-devel     | 必须安装                                       |
| openssl-devel  | 1.0版本及以上，必须安装                        |
| jemalloc       | 1.2版本及以上，必须安装                        |
| openjdk        | 如果使用Java api，则需要 version = 1.8.x       |
| requests       | 如果使用Python http api，则需要                |
| node           | 如果使用Nodejs http api则需要 version >=10.9.0 |
| pthreads       | 如果使用php http api，则需要                   |
| ccache         | 可选，用于加速编译                             |

## 编译安装

### 安装环境准备

根据您的操作系统运行 scripts/setup/ 中相应的脚本能够自动为您解决大部分问题（推荐）。比如，若您是 Ubuntu 用户，可执行以下指令：

```bash
$ . scripts/setup/setup_ubuntu.sh
```

若您是 Centos 用户，可以执行以下指令：

```shell
$ . scripts/setup/setup_centos.sh
```

**在运行脚本之前，**建议先安装 5.0 以上版本的 gcc 和 g++ 。

当然，您也可以选择手动逐步准备环境；下面提供了各系统要求的详细安装指导。

在执行安装命令之前，建议执行如下命令临时切换到超级用户权限模式：

```bash
$ sudo -i
请输入密码
[sudo] xxx 的密码：
```

#### unzip/bzip2 安装

用于离线安装时解压 gStore zip包和bz2包

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

#### gcc 和 g++ 安装

检查 g++ 版本：

```bash
$ g++ --version
```

以安装 9.3.0 为例：（适用于 Ubuntu 和 CentOS ）

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

Ubuntu 也可直接使用以下命令安装：

```bash
$ apt install -y gcc-9 g++-9
```

#### readline-devel 安装

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

#### pkg-config 安装

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

#### uuid-devel 安装

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

#### boost 安装

推荐使用1.74.0版本进行安装

判断 boost 是否安装

```bash
$ yum list installed | grep boost	    #centos系统
$ dpkg -s libboost-all-dev				#ubuntu系统
```

如果没有安装，则安装：（以版本 1.74.0 为例）

版本:1.74.0

地址：http://sourceforge.net/projects/boost/files/boost/1.74.0/boost_1_74_0.tar.gz

安装脚本： （适用于 CentOS 和 Ubuntu）

```bash
$ wget http://sourceforge.net/projects/boost/files/boost/1.74.0/boost_1_74_0.tar.gz
$ tar -xzvf boost_1_74_0.tar.gz
$ cd boost_1_74_0
$ ./bootstrap.sh --with-libraries=system,regex,thread,filesystem
$ ./b2 -j4
$ ./b2 install
```

Ubuntu 也可直接使用以下命令安装：

```bash
$ apt install -y libboost-all-dev
```

**注意：请在确保 g++ 版本高于 9.3.0 后安装 boost 。**若在编译 gStore 时遇到与 boost 链接错误（形如 "undefined reference to `boost::...`"），很可能是因为您使用低于 9.3.0 的 gcc 版本编译 boost 。此时，请使用以下步骤重新编译 boost ：

- 进入 boost 目录
- 清除旧文件：`./b2 --clean-all
- 在 ./tools/build/src 下的 user-config.jam 文件中（若此路径下不存在此文件，请在 ./tools/build/example 或其他路径下找到一个示例 user-config.jam 文件并拷贝到 ./tools/build/src 下）添加：`using gcc : 9.3.0 : gcc-9.3.0的路径 ;`
- 在 ./ 下运行 `./bootstrap.sh --with-toolset=gcc`
- `./b2 install --with-toolset=gcc`

进入gStore下的build目录然后重新编译 gStore 

```shell
# 进入gStore下的build目录
$ cd build
# 清理编译生成的三方库文件
$ make clean_pre
# 重新预编译
$ make pre
# 编译项目
$ make -j4
# 初始化数据库
$ make init
```

#### curl-devel 安装 

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

#### openssl-devel 安装

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

#### cmake 安装 

判断 cmake 是否安装

```bash
$ cmake --version				#centos系统
$ cmake --version				#ubuntu系统
```

如果没有安装，则安装：

版本：3.23.2

地址：https://cmake.org/files/v3.23.2/cmake-3.23.2.tar.gz

安装脚本 （适用于 CentOS 和 Ubuntu）

```bash
$ wget https://cmake.org/files/v3.23.2/cmake-3.23.2.tar.gz
$ tar -xvf cmake-3.23.2.tar.gz && cd cmake-3.23.2/
$ ./bootstrap
$ make -j4
$ make install
```

Ubuntu 也可直接使用以下命令安装：

```bash
$ apt install -y cmake
```



#### jemalloc安装

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

创建软连接

```bash
# 查找动态库位置，默认安装位置一般是在/usr/lib64
$ find / -name libjemalloc*
/usr/lib64/libjemalloc.so.1

# 如果没有搜索到/usr/lib64/libjemalloc.so，则需要创建软连接
$ cd /usr/lib64
$ ln -s libjemalloc.so.1 libjemalloc.so
```

### gStore获取 

如果遇到权限问题，请在命令前加 `sudo` 。

#### 方式一：download

gStore目前已经上传到gitee（码云），国内用户推荐使用码云下载，速度更快，网址为 https://gitee.com/PKUMOD/gStore

也可打开 https://github.com/pkumod/gStore ，下载gStore.zip；解压zip包。

#### 方式二：clone (推荐)

通过如下命令 clone:

```bash
$ git clone https://gitee.com/PKUMOD/gStore.git # gitee(码云) 国内下载速度更快
$ git clone https://github.com/pkumod/gStore.git # GitHub

# 拉取历史版本时可使用-b参数,如-b 1.2
$ git clone -b 1.2 https://gitee.com/PKUMOD/gStore.git
$ git clone -b 1.2 https://github.com/pkumod/gStore.git
```

注意：这一方法需要先安装 Git 。

```bash
$ sudo yum install git		# CentOS系统
$ sudo apt-get install git	# Ubuntu系统
```



### gStore编译

切换到 gStore 目录下：

```bash
$ cd gStore
```

目前支持两种编译方式，脚本编译(推荐)和手动编译

脚本编译（推荐）

```bash
$ bash scripts/build.sh
#若编译顺利完成，最后会出现 Compilation ends successfully! 结果
```

手动编译

```bash
#切换到gStore目录
$ cd gStore
# 创建build目录
$ mkdir build
$ cd build
# 生成makefile等文件
$ cmake ..
# 预编译
$ make pre
# 编译
$ make -j4
# 初始化数据库
$ make init
# 最后出现如下输出表示编译和初始化成功
---Compilation ends successfully!
---system.db is built successfully!
```

 <div STYLE="page-break-after: always;"></div> 
## Docker方式部署gStore

> 我们提供两种方式通过容器部署gStore：
> 一种是通过项目根目录的Dockerfile文件自主构建，然后运行容器.
> 另一种是直接下载已经自动构建完成的镜像，然后直接运行.

### 环境准备

关于安装使用Docker，参考地址：[docker](https://www.runoob.com/docker/ubuntu-docker-install.html)

### 直接拉取镜像运行(推荐)

目前已经在docker hub上构建并发布了gStore镜像，该镜像是基于ubuntu 16.04版本，可以采用如下方式进行安装和部署。

**拉取docker 镜像**

```shell
docker pull pkumodlab/gstore-docker:latest #拉取最新版docker镜像
```

**运行镜像**

通过如下命令查看docker 镜像列表

```
docker image list
```

如下图所示

![image-20221018223737566](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221018223737566.png)

采用如下命令启动镜像

```shell
docker run -itd -p 9999:9000 9ca4388fc81e /bin/bash 
# 将容器9000端口映射到宿主机9999端口
# 9ca4388fc81e 为image id
```

启动完成后，输入如下命令可以查看容器信息

```shell
docker ps
```

![image-20221018224207124](https://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-20221018224207124.png)

采用如下命令进入容器

```shell
docker exec -it a5016bd46094 /bin/bash #推荐采用exec方式进入容器，退出后容器不会停止
```

进入容器后，可以在根目录上看到有gStore目录，该gStore已经编译安装完毕,可以像使用本地gStore一样操作即可。

需要退出docker 容器，可以采用如下命令

```shell
exit
```

### 本地构建镜像运行

#### 构建生成名为gStore的镜像

```shell
# 构建镜像
docker build -t gstore .

# 查看镜像
docker image list
```

如下图所示

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-docker.jpg)

#### 启动本地构建的镜像

```bash
# 将宿主机的9999端口映射到容器的9000端口(gStore API服务的默认端口)，a5d51ff4f121为image id
docker run -itd -p 9999:9000 a5d51ff4f121 /bin/bash 

# 查看容器状态
docker ps -a
#CONTAINER ID   IMAGE          COMMAND                  CREATED         STATUS         PORTS                                       NAMES
#317e4ce0a667   a5d51ff4f121   "bash /docker-entryp…"   6 minutes ago   Up 6 minutes   0.0.0.0:9999->9000/tcp, :::9999->9000/tcp   awesome_greider
```

#### 测试API服务是否正常

```bash
curl http://127.0.0.1:9999 -X POST -H 'Content-Type: application/json' -d '{"operation":"check"}'

# 如果控制台打印如下信息表示gStore http api服务正常启动
# {"StatusCode":0,"StatusMsg":"the ghttp server is running..."}
```

---

其他可能也有不少需要补充，所以目前只是**抛砖引玉**，添加了一个最基本的版本。基本的环境构建只是容器化的第一步

## 静态安装包部署gStore

### 静态包下载地址

现提供 4 种静态安装包下载：

```shell
http://file.gstore.cn/f/eb7fa4c9d0d3421695f7/?dl=1   #linux-arm
http://file.gstore.cn/f/bb59558af0be44a6970d/?dl=1   #linux-x86
http://file.gstore.cn/f/158654beb04343ba9afe/?dl=1   #ubuntu-arm_64
http://file.gstore.cn/f/db5a9ac955ea45bfba27/?dl=1   #ubuntu-x86_64
```

### 下载静态安装包

```shell
$ wget --content-disposition http://file.gstore.cn/f/75643cb217604efca75b/?dl=1  #静态包获取地址
$ tar -zxvf gstore-1.3-static-linux-arm.tar.gz  
```

### 初始化系统

```shell
$ bin/ginit
```

## 版本更新

### 方式1：旧版本是通过clone部署的升级方式

#### 升级前准备

```shell
# 进入部署gstore的根目录
$ cd gstore
# 拉取更新脚本
$ git pull
# 查看是否获取到version_checkout.sh文件
$ ls scripts/version_checkout.sh
# 如果没有, 请指定分支获取, 例如1.2
$ git status
1.2
$ git pull origin 1.2
# 确认远端是否有自己想要升级的版本，例如1.3
$ git branch -r | grep origin/1.3
# 确认本地是否有命名冲突的分支名, 例如1.3(如果有请备份分支，删除冲突分支)
$ git branch | grep 1.3
```

#### 升级

```shell
# 进入部署gstore的根目录
$ cd gstore
# 指定升级版本，执行升级脚本进行版本更新，例如1.3
$ bash scripts/version_checkout.sh 1.3
# 选择是否覆盖数据库, 输入y或n
the new version already has a xxx.db, overwrite [Y/n]
# 升级成功
version update successfully .
```

重要数据会进行备份，文件目录：version版本号_日期

### 方式2：旧版本通过下载zip包或静态包部署的升级方式

```shell
# 获取旧版本gstore路径
$ pwd
/xxx/xxxx/gstore
# 解压更新包，进入gstore目录
$ cd gstore/
# 指定旧版本路径，执行升级脚本进行更新
$ bash scripts/version_update.sh /xxx/xxxx/gstore
# 选择是否覆盖数据库, 输入y或n
the new version already has a xxx.db, overwrite [Y/n]
# 升级成功
version update successfully .
```

