# Deploy gStore by Docker

[点击查看中文文档](DOCKER_DEPLOY_CN.md)

>Roughly speaking, there are two ways to deploy gStore via Docker.
>
>The first one is using the Dockerfile file in the root directory of the project to automatically build, and then run the container.
>
>Another one is downloading the pre-built image, and running it directly.

## 0x00. prepare the environment

Official doc of Docker has explained how to download and use it on the common Liunx release version in detail. And here is the link: [English doc](https://docs.docker.com/install/linux/docker-ce/ubuntu/), [中文文档](https://docs.docker-cn.com/engine/installation/linux/docker-ce/centos/#%E5%85%88%E5%86%B3%E6%9D%A1%E4%BB%B6)

## 0x01. Build the image via Dockerfile

After having the correct Docker environment and network, use `git clone` to download the project firstly. Then enter the root directory and input the command `docker build -t gstore .` to start building. The more specific explanation has been written in the Dockerfile.

After the building, using `docker run -p 9000:80 -it gstore` directly to start and enter the container and execute other operations. 

## 0x02. pulling the pre-built image directly to run(recommendation)

Instead of downloading this project or building by yourself, input `docker pull pkumodlab/gstore:latest` to pull the docker image which has been built well on the docker hub. Then input `docker run -p 9000:80 -it pkumodlab/gstore:latest` to start and enter the container and execute other operations. 

## 0x03. Follow-up

### A. Performance testing

The proportion of the loss of the performance of the container under the conditions of different file numbers/networks. The performance of running gStore in the native environment and in the container.

It's waiting for supplement.

### B. Test of connecting other containers

Waiting for supplement.

---

There are still a large number of content waiting for supplement.

Up to now we have just given a basic version. It's only the first step of containerization.

It's the document ver1.0
