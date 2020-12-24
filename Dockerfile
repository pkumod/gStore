#This Dockerfile is just a primitive one. We welcome optimization and simplification.

#Attention: 
#It is best to update the process of modification in the rear of the original code(reduce the order changes).
#Otherwise the speed of construction will be influenced on a large scale.

#TODO: some space can be saved by using the low version of gcc mirror(e.g-gcc:5).
#But its mobility and dependence has not been tested yet and waiting for confirmation.

FROM registry.docker-cn.com/library/gcc:8

#download all the optional installation library in gstore's document.
RUN apt-get update && apt-get install -y --no-install-recommends realpath \
         ccache \
         vim \
	 lsof \
         openjdk-8-jdk \
         libreadline-dev \
         libboost-all-dev \
         && rm -rf /var/lib/apt/lists/

COPY . /usr/src/gstore
WORKDIR /usr/src/gstore

#The solution to the problem of java, whose default setting is using ansii to encode.
ENV LANG C.UTF-8

EXPOSE 80

RUN make pre

RUN make
