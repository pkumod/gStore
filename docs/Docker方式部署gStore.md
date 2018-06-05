# Docker方式部署gStore（中文）



> 简单说，我们提供两种方式通过容器部署gStore：
>
> 一种是通过项目根目录的Dockerfile文件自主构建，然后运行容器自动编译(make).
>
> 另一种是直接下载已经构建完成的镜像，然后直接运行.

## 0x00.环境准备

关于安装使用Docker，官方针对常见Linux发行版文档已经写得很详细，就直接给出参考地址：[英文文档](https://docs.docker.com/install/linux/docker-ce/ubuntu/)，[中文文档](https://docs.docker-cn.com/engine/installation/linux/docker-ce/centos/#%E5%85%88%E5%86%B3%E6%9D%A1%E4%BB%B6) 。

需要注意的是，Docker版本过高可能导致一些问题，建议仔细阅读注意事项。当前测试环境版本是<u>Docker CE 17.04</u>

## 0x01.通过Dockerfile构建镜像

假设已经拥有正常的Docker环境跟网络后，首先通过`git clone ` 下载项目，然后进入项目根目录，**修改**根目录的makefile文件的`CC = ccache g++` (*建议官方改回这个默认*)，输入命令`docker build -t gstore .` 即可开始构建，默认使用根目录的Dockerfile，关于具体说明Dockerfile文件内也有补充。

构建完成后，直接通过`docker run -it gstore` 即可进入容器，自动触发make进行编译。完成后即可执行其他操作，后续待文档完善。（之所以未将make并入构建过程是因为触发了异常）

## 0x02.直接拉取镜像运行

无需下载项目或自己构建，直接输入`docker pull imbajin/test:gs` 拉取已经通过Dockerfile构建完成的镜像，但是没有执行make 。拉取完成后 `docker run -it imbajin/test:gs ` 即可进入编译，耗时大概要5~10分钟。（您可以给容器添加别名以便方面使用）

另外提供一个可选的版本是`docker pull imbajin/test:gsmake`  ，这个镜像保护make完成后的状态，拉取完成后可开箱即用。是未来的理想状态，但是尚未具体测试，仅供参考。

*<u>补：目前仓库地址为个人维护，后续应会移交团队自行维护（包括自动构建） </u>*

## 0x03.存在的问题

因为容器化伴随着一些不确定的影响，包括不限于网络，锁，缓存，权限等问题，遇到调试起来很难定位，已知可能存在以下问题：(宿主机是Centos7.4)

1. 如果在容器内直接执行make命令，最后执行testall脚本会写满临时空间，导致中断（具体原因排查中）

2. 通过Dockerfile关联Dockerhub自动构建时出现了奇怪的tar命令报错，[详见地址](https://hub.docker.com/r/imbajin/docker/builds/b3rwguoffywc6equeajyg7g/)末尾。（原因也待查）

3. 在Dockerfile的构建过程中如果添加环境`ENV CC="ccache g++" ` 会导致编译错误，原因未知且可能影响镜像缓存层，导致之后构建反复报错。。

4. 通过docker run启动容器后可能出现启动后，即自动关闭的情况。

   

## 0x04.后续工作

#### A.性能测试

这个待后续补充，容器化跟原生运行gStore，在不同文件数/网络等情况下，容器的性能损耗具体比重。。

#### B.连接其他容器测试

文档也待后续补充更新，因为目前也是抽空做的

#### C.构建精简优化

因为gcc:8镜像就有1.7G了，自带了很多不必要的东西（包括go的环境之类的），希望后续除了在调低gcc版本之外，能对源镜像本身改进出精简版来。

---

其他可能也有不少需要补充，所以目前只是**抛砖引玉**，添加了一个最基本的版本。基本的环境构建只是容器化的第一步，后续工作还不少~  这只是初版的V1.0文档。

