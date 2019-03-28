# Deploy gStore by Docker

[点击查看中文文档](Docker方式部署gStore.md)

>Roughly speaking, there are two ways to deploy gStore via Docker.
>
>The first one is using Dockerfile file in the root directory of project to automatically build it. And then run the container.
>
>Another one is downloading the mirror which has been automatically built directly, then just run it.

## 0x00. prepare the environment

Official doc of Docker has explained how to download and use it on common Liunx release version in details. And here is the link: [English doc](https://docs.docker.com/install/linux/docker-ce/ubuntu/), [中文文档](https://docs.docker-cn.com/engine/installation/linux/docker-ce/centos/#%E5%85%88%E5%86%B3%E6%9D%A1%E4%BB%B6)

It's worth noting that the Docker with too high version may lead to some problems. Please read the precautions carefully. The current version of test environment is <u>Docker CE 18.06.1</u>

## 0x01. Build the mirror via Dockerfile

After having the correct Docker environment and network, use `git clone` to download the project firstly. Then enter the root directory and input command `make pre ` to unzip some toolkits.  After inputting command `docker build -t gstore` it's available to start building. In the default case, it will use the Dockerfile in the root directory. More specific explanation has been written in the Dockerfile.

After the building, using `docker run -p 9000:80 -it gstore` directly to start and enter the container and execute other operations. 

## 0x02. pulling the mirror directly to run(recommendation)

Instead of downloading project or building on your own, input `docker pull pkumod/gstore:latest` to pull the mirror which has been automatically built well on the docker hub.  Then input `docker run -p 9000:80 -it pkumod/gstore:latest` to start and enter the container and execute other operations. 

## 0x03. Follow-up

### A. Performance testing

The proportion of the loss of the performance of the container under the conditions of different file numbers/networks. The performance of running gStore in the native environment and in the container.

It's waiting for supplement.

### B. Test of connecting other containers

Waiting for supplement.

### C. Simplification and optimization of building

The mirror of gcc:8 has conquered the space of 1.7G, bringing a lot of unnecessary things(including the environment of Go). Hoping to optimize the mirror source in the future excepting lowing the gcc's version.

---

There are still a large number of content waiting for supplement.

Up to now we have just given a basic version. It's only the first step of containerization.

It's the document ver1.0
