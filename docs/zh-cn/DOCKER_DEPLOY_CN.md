## Docker方式部署gStore

> 我们提供两种方式通过容器部署gStore：
> 一种是通过项目根目录的Dockerfile文件自主构建，然后运行容器.
> 另一种是直接下载已经自动构建完成的镜像，然后直接运行.

### 环境准备

关于安装使用Docker，参考地址：[docker](https://blog.csdn.net/A632189007/article/details/78662741)

### 直接拉取镜像运行(推荐)

目前已经在docker hub上构建并发布了gstore镜像，该镜像是基于ubuntu 16.04版本，可以采用如下方式进行安装和部署。

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

进入容器后，可以在根目录上看到有gstore目录，该gstore已经编译安装完毕,可以像使用本地gstore一样操作即可。

需要退出docker 容器，可以采用如下命令

```shell
exit
```

## 通过Dockerfile构建镜像 

### 构建生成名为gstore的镜像

```shell
# 构建镜像
docker build -t gstore .

# 查看镜像
docker image list
```

如下图所示

![](http://gstore-bucket.oss-cn-zhangjiakou.aliyuncs.com/liwenjie-image/image-docker.jpg)

### 启动本地构建的镜像

```bash
# 将宿主机的9999端口映射到容器的9000端口(gstore API服务的默认端口)，a5d51ff4f121为image id
docker run -itd -p 9999:9000 a5d51ff4f121 /bin/bash 

# 查看容器状态
docker ps -a
#CONTAINER ID   IMAGE          COMMAND                  CREATED         STATUS         PORTS                                       NAMES
#317e4ce0a667   a5d51ff4f121   "bash /docker-entryp…"   6 minutes ago   Up 6 minutes   0.0.0.0:9999->9000/tcp, :::9999->9000/tcp   awesome_greider
```

### 测试API服务是否正常

```bash
curl http://127.0.0.1:9999 -X POST -H 'Content-Type: application/json' -d '{"operation":"check"}'

# 如果控制台打印如下信息表示gstore http api服务正常启动
# {"StatusCode":0,"StatusMsg":"the ghttp server is running..."}
```

---

其他可能也有不少需要补充，所以目前只是**抛砖引玉**，添加了一个最基本的版本。基本的环境构建只是容器化的第一步