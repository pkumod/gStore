#This Dockerfile is just a primitive one. We welcome optimization and simplification.

#Attention: 
#It is best to update the process of modification in the rear of the original code(reduce the order changes).
#Otherwise the speed of construction will be influenced on a large scale.

FROM lsvih/gcc-boost-cmake-java:v1
RUN apt update \
	&& apt install -y --no-install-recommends \
	libcurl4-openssl-dev \
	libreadline-dev \
	uuid-dev \
	&& ldconfig -v \
	&& echo "*    -    nofile    65535" >> /etc/security/limits.conf \
	&& echo "*    -    noproc    65535" >> /etc/security/limits.conf

COPY . /usr/src/gstore
WORKDIR /usr/src/gstore

ENV LANG C.UTF-8

EXPOSE 80

RUN make pre -j && make -j \
	&& apt autoclean && apt clean \
	&& rm -rf /tmp/* /var/tmp/* \
	&& rm -rf /usr/share/doc/* \
	&& rm -rf /var/lib/apt/lists/* 
