# SPDX-License-Identifier: BSD-3-Clause

FROM debian:buster-slim AS builder

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

# Copy gStore source code; run `make tarball` to generate this file
ADD gstore.tar.gz /usr/src/gstore

RUN mkdir -p build

RUN cd build && cmake ..

RUN cd build && make pre && make -j$(nproc)

FROM debian:buster-slim AS runtime

RUN apt-get update && apt-get install -y \
    libboost-regex1.67.0 \
    libboost-system1.67.0 \
    libboost-thread1.67.0 \
    libcurl4 \
    libssl1.1 \
    libzmq5 \
    uuid-runtime \
    libjemalloc2 \
    libreadline7 \
    libopenmpi3 \
    coreutils \
    g++ \
    gcc \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/src/gstore/bin/ /usr/local/bin/

COPY --from=builder /usr/src/gstore/pfn/lib/ /lib/

COPY --from=builder /usr/src/gstore/lib/ /docker-init/lib/

COPY --from=builder /usr/src/gstore/build/lib/ /docker-init/build/lib/

COPY --from=builder /usr/src/gstore/pfn/lib/ /docker-init/pfn/lib/

COPY conf /docker-init/conf/
COPY data /docker-init/data/
COPY src /docker-init/src/
COPY scripts/docker-entrypoint.sh /

WORKDIR /app/
VOLUME [ "/app/" ]

RUN echo "*    -    nofile    65535" >> /etc/security/limits.conf \
        && echo "*    -    noproc    65535" >> /etc/security/limits.conf

EXPOSE 9000

ENTRYPOINT [ "sh", "/docker-entrypoint.sh" ]

# CMD [ "/usr/local/bin/ghttp" ]
