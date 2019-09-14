FROM ubuntu:16.04
RUN apt-get update && apt-get install -y --no-install-recommends realpath \
         ccache \
	     lsof \
         openjdk-8-jdk \
         libreadline-dev \
         libboost-all-dev \
         && rm -rf /var/lib/apt/lists/*

COPY . /usr/src/gstore
WORKDIR /usr/src/gstore

ENV LANG C.UTF-8

EXPOSE 80

RUN ./scripts/setup/setup_ubuntu.sh && make pre && make
