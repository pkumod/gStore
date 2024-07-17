# SPDX-License-Identifier: BSD-3-Clause

FROM ubuntu:22.04 AS builder

LABEL vendor="pkumod"
LABEL description="gStore RDF Database Engine"

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    wget \
    libboost-regex-dev \
    libboost-system-dev \
    libboost-thread-dev \
    libboost-system-dev \
    curl \
    libcurl4 \
    libcurl4-openssl-dev \
    libssl-dev \
    libzmq3-dev \
    pkg-config \
    zlib1g-dev \
    uuid-dev \
    libjemalloc-dev \
    libreadline-dev


RUN wget https://cmake.org/files/v3.23/cmake-3.23.2.tar.gz \
    && tar -xvf cmake-3.23.2.tar.gz \
    && cd cmake-3.23.2 \
    && ./bootstrap \
    && make -j$(nproc) \
    && make install \
    && cd ..

RUN mkdir -p /src

WORKDIR /usr/src/gstore

RUN mkdir .tmp 

# Copy gStore source code; run `make tarball` to generate this file
ADD gstore.tar.gz /usr/src/gstore

RUN mkdir -p build

RUN cd build && cmake ..

RUN cd build && make pre && make -j$(nproc)

FROM ubuntu:22.04 AS runtime

RUN apt-get update && apt-get install -y \
    libboost-regex1.74.0 \
    libboost-system1.74.0 \
    libboost-thread1.74.0 \
    libcurl4 \
    libssl3 \
    libzmq5 \
    uuid-runtime \
    libjemalloc2 \
    libreadline8 \
    libopenmpi3 \
    coreutils \
    g++ \
    gcc \
    && rm -rf /var/lib/apt/lists/*
# executable files and library files
COPY --from=builder /usr/src/gstore/bin/ /gstore/bin/
COPY --from=builder /usr/src/gstore/pfn/ /gstore/pfn/
COPY --from=builder /usr/src/gstore/lib/ /gstore/lib/
COPY --from=builder /usr/src/gstore/src/Query/Algorithm/PathQueryHandler.h /gstore/src/Query/Algorithm/PathQueryHandler.h
COPY --from=builder /usr/src/gstore/src/Database/CSR.h /gstore/src/Database/CSR.h
# configure files
COPY --from=builder /usr/src/gstore/conf/ /gstore/conf/
COPY --from=builder /usr/src/gstore/data/ /gstore/data/
COPY --from=builder /usr/src/gstore/api/ /gstore/api/
COPY --from=builder /usr/src/gstore/scripts/ /gstore/scripts/
COPY --from=builder /usr/src/gstore/docs/ /gstore/docs/
COPY --from=builder /usr/src/gstore/README.md /gstore/README.md
COPY --from=builder /usr/src/gstore/README_ZH.md /gstore/README_ZH.md
COPY --from=builder /usr/src/gstore/LICENSE /gstore/LICENSE
# Entry Point Script
COPY scripts/docker-entrypoint.sh /

WORKDIR /gstore/
VOLUME [ "/gstore/" ]

RUN echo "*    -    nofile    65535" >> /etc/security/limits.conf \
 && echo "*    -    noproc    65535" >> /etc/security/limits.conf

EXPOSE 9000

# Default API service is ghttp, which can be configured with -e API_SERVICE=grpc
# Default root password is 123456, witch can be configured with -e ROOT_PASSWD=your password
# For example: 
#    # this container will run grpc api server with 9999 and init root password with 123@abc
#    # you can test server api with this command :
#    # curl -X POST -H 'Content-Type: application/json' -d '{"username":"root","password":"123@abc","operation": "login"}' http://127.0.0.1:9999/grpc/api
#    docker run -itd -p 9999:9000 -e API_SERVICE=grpc -e ROOT_PASSWD=123@abc gstore:latest
ENTRYPOINT [ "sh", "/docker-entrypoint.sh" ]
