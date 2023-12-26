# SPDX-License-Identifier: BSD-3-Clause

FROM debian:buster-slim AS builder

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

# Compile gStore dependencies
COPY tools/ /usr/src/gstore/tools

COPY makefile /usr/src/gstore/

RUN mkdir -p lib && make pre

# Copy gStore source code; run `make tarball` to generate this file
ADD gstore.tar.gz /usr/src/gstore

RUN make

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
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/src/gstore/bin/ /usr/local/bin/

COPY --from=builder /usr/src/gstore/lib/ /docker-init/lib/

COPY backup.json init.conf conf.ini ipAllow.config ipDeny.config slog.properties slog.stdout.properties \
    /docker-init/
COPY data/ /docker-init/data/
COPY docker-entrypoint.sh /

WORKDIR /app/
VOLUME [ "/app/" ]

RUN echo "*    -    nofile    65535" >> /etc/security/limits.conf \
	&& echo "*    -    noproc    65535" >> /etc/security/limits.conf

EXPOSE 9000

ENTRYPOINT [ "sh", "/docker-entrypoint.sh" ]

CMD [ "/usr/local/bin/ghttp" ]
