# Deploy gStore by Docker

[点击查看中文文档]([gStore](https://github.com/0x00Geek/gStore)/[docs](https://github.com/0x00Geek/gStore/tree/master/docs)/**Docker方式部署gStore.md** )

>Roughly speaking, there are two ways to deploy gStore via Docker.
>
>The first one is using Dockerfile file in the root directory of project to automatically build it. And then run the container to compile automatically(make).
>
>Another one is downloading the mirror which has been built directly, then just run it.

## 0x00. prepare the environment

Official doc of Docker has explained how to download and use it on common Liunx release version in details. And here is the link: [English doc](https://docs.docker.com/install/linux/docker-ce/ubuntu/), [中文文档](https://docs.docker-cn.com/engine/installation/linux/docker-ce/centos/#%E5%85%88%E5%86%B3%E6%9D%A1%E4%BB%B6)

It's worth noting that the Docker with too high version may lead to some problems. Please read the precautions carefully. The current version of test environment is <u>Docker CE 17.04</u>

## 0x01. Build the mirror via Dockerfile

After having the correct Docker environment and network, use `git clone` to download the project firstly. Then enter the root directory and modify the `CC = gcc`(official setting) into `CC = ccache g++` in the makefile file. After inputting command `docker build -t gstore` it's available to start building. In the default case, it will use the Dockerfile in the root directory. More specific explanation has been written in the Dockerfile.

After the building, using `docker run -it gstore` directly to enter the container. It will trigger make to compile automatically. Other operation can be executed after finishing all the procedure shown above. The subsequent document is waiting for completing. (The reason why we didn't merge the make to the building process is that it will trigger exception)

## 0x02. pulling the mirror directly to run

Instead of downloading project or building on your own, input `docker pull imbajin/test:gs` to pull the mirror which has been built well by the Dockerfile. But the make hasn't been executed yet. Then input `docker run -it imbajin/test:gs` to compile, which will cost 5 to 10 minutes or so. (You could add alias to the container in the sake of convenience)

Another optional version we provided is `docker pull imbajin/test:gsmake` . This mirror will protect the status after the completion of make, which is out of box after your pulling. It's a ideal one and its still waiting for test(just for reference).

*PS: The repository is personally maintained now, it will be handed over to the team to maintain later(including automatic build)*

## 0x03. Problems that exist

Owing to the uncertain influence accompanying the containerization, including but not limited to some problems about network, lock, caching, rights and so on, it's quite difficult to locate the problem when debugging. It has been known that the following problems may exist: (The host is Centos 7.4)

1. If execute the make command directly in the container straightforward, the last execution of the testall script will fill up the temporary space, resulting in interruption. (The specific reason is being investigated)
2. Strange tar command error occurred when associating Dockerhub automatic building via Dockerfile. See this [address](https://hub.docker.com/r/imbajin/docker/builds/b3rwguoffywc6equeajyg7g/)(reason is waiting for investigation)
3. If add the environment `ENV CC="ccache g++"` in the process of Dockerfile building, it will lead to compile error. The reason is still unknown and it may affect the mirror cache layer, which results in repeated errors in the later process of building.
4. The case of shutdown automatically as soon as its reboot may occur after starting the container via docker run.

## 0x04. Follow-up

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