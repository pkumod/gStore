# SPDX-License-Identifier: BSD-3-Clause

FROM debian:buster-slim AS builder

LABEL vendor="pkumod"
LABEL description="gStore RDF Database Engine"

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    libcurl4 \
    libcurl4-openssl-dev \
    libssl-dev \
    libzmq3-dev \
    pkg-config \
    wget \
    uuid-dev \
    ninja-build \
    mold \
    python3-pip \

RUN mkdir -p /src

WORKDIR /usr/src/gstore

# Install conan dependencies\
RUN pip3 install conan && conan profile detect

COPY conanfile.py /usr/src/gstore/

RUN conan install . --build=missing -s "build_type=Release" -s "compiler.libcxx=libstdc++11"

# Compile gStore dependencies
COPY 3rdparty/ /usr/src/gstore/3rdparty

COPY conanfile.py /usr/src/gstore/
COPY CMakeLsits.txt /usr/src/gstore/

RUN mkdir -p lib

# Copy gStore source code; run `make tarball` to generate this file
ADD gstore.tar.gz /usr/src/gstore

FROM debian:buster-slim AS runtime

RUN cd cmake-build-release && \
    cmake .. \
      -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release

RUN cd cmake-build-release && \
    make pre && \ make

RUN apt-get update && apt-get install -y \
    libssl3 \
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

COPY conf/ /docker-init/
COPY data/ /docker-init/data/
COPY docker-entrypoint.sh /

WORKDIR /app/
VOLUME [ "/app/" ]

RUN echo "*    -    nofile    65535" >> /etc/security/limits.conf \
	&& echo "*    -    noproc    65535" >> /etc/security/limits.conf

EXPOSE 9000

ENTRYPOINT [ "sh", "/docker-entrypoint.sh" ]

CMD [ "/usr/local/bin/ghttp" ]
