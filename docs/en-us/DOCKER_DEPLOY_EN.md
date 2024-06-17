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