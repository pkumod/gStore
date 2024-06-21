# SPDX-License-Identifier: BSD-3-Clause

FROM ubuntu:22.04 AS builder

LABEL vendor="pkumod"
LABEL description="gStore RDF Database Engine"

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
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
    wget \
    zlib1g-dev \
    uuid-dev \
    libjemalloc-dev \
    libreadline-dev

RUN mkdir -p /src

WORKDIR /usr/src/gstore

RUN mkdir .debug \
    && mkdir .tmp \
    && mkdir .objs \
    && mkdir logs \
    && mkdir -p backups/logs \
    && mkdir bin \
    && mkdir lib

# Copy gStore source code; run `make tarball` to generate this file
ADD gstore.tar.gz /usr/src/gstore

# Compile gStore
RUN make pre && make

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
    gcc \
    g++ \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/src/gstore/bin/ /gstore/bin/
COPY --from=builder /usr/src/gstore/lib/ /gstore/lib/
COPY --from=builder /usr/src/gstore/data/ /gstore/data/
COPY --from=builder /usr/src/gstore/backups/ /gstore/backups/
COPY --from=builder /usr/src/gstore/logs/ /gstore/logs/
COPY --from=builder /usr/src/gstore/.tmp/ /gstore/.tmp/
# configure file
COPY --from=builder /usr/src/gstore/Query/ /gstore/Query/
COPY --from=builder /usr/src/gstore/Database/ /gstore/Database/
COPY --from=builder /usr/src/gstore/backup.json /gstore/
COPY --from=builder /usr/src/gstore/init.conf /gstore/
COPY --from=builder /usr/src/gstore/conf.ini /gstore/
COPY --from=builder /usr/src/gstore/ipAllow.config /gstore/
COPY --from=builder /usr/src/gstore/ipDeny.config /gstore/
COPY --from=builder /usr/src/gstore/slog.properties /gstore/
COPY --from=builder /usr/src/gstore/slog.stdout.properties /gstore/

# Entry Point Script
COPY docker-entrypoint.sh /

WORKDIR /gstore/
VOLUME [ "/gstore/" ]

RUN echo "*    -    nofile    65535" >> /etc/security/limits.conf \
 && echo "*    -    noproc    65535" >> /etc/security/limits.conf

EXPOSE 9000

# Default API service is ghttp, which can be configured with -e API_SERVICE=grpc
# Default root password is 123456, witch can be configured with -e ROOT_PASSWD=your password
# For example: docker run -itd -p 9999:9000 -e API_SERVICE=grpc -e ROOT_PASSWD=123@abc gstore:latest
ENTRYPOINT [ "sh", "/docker-entrypoint.sh" ]