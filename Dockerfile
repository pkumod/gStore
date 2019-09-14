FROM ubuntu:16.04

COPY . /usr/src/gstore
WORKDIR /usr/src/gstore

ENV LANG C.UTF-8

EXPOSE 80

RUN ./scripts/setup/build_with_docker.sh && make clean && make pre && make && rm -rf /var/lib/apt/lists/*
