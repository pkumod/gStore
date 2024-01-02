# SPDX-License-Identifier: BSD-3-Clause

FROM ubuntu:22.04 AS builder

LABEL vendor="pkumod"
LABEL description="gStore RDF Database Engine"

# Install system dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    mold \
    python3-pip \
    pkg-config \
    uuid-dev \
    libjemalloc-dev \
    libreadline-dev \
    libssl-dev

RUN mkdir -p /src

WORKDIR /usr/src/gstore

# Install conan dependencies\
RUN pip3 install conan && conan profile detect

COPY conanfile.py /usr/src/gstore/

RUN conan install . --output-folder=build --build=missing -s "build_type=Release" -s "compiler.libcxx=libstdc++11" -s "compiler.cppstd=17"

# Copy gStore source code; run `make tarball` to generate this file
ADD gstore.tar.gz /usr/src/gstore

RUN mkdir -p build && cd build && \
    cmake .. -G Ninja \
      -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake \
      -DCMAKE_CXX_FLAGS="-fuse-ld=mold" \
      -DCMAKE_BUILD_TYPE=Release

RUN mkdir -p build && cd build && \
    ninja -v prepare && \
    ninja -v install

FROM ubuntu:22.04 AS runtime

RUN apt-get update && apt-get install -y \
    libgomp1 \
    libssl3 \
    libjemalloc2 \
    libreadline8 \
    libuuid1 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /usr/src/gstore/bin/ /bin/
COPY --from=builder /usr/src/gstore/lib/ /lib/
COPY --from=builder /usr/src/gstore/defaults/ /defaults/
COPY --from=builder /usr/src/gstore/data/ /data/
COPY --from=builder /usr/src/gstore/scripts/init.sh /docker-entrypoint.sh

WORKDIR /app/
VOLUME [ "/app/" ]

RUN echo "*    -    nofile    65535" >> /etc/security/limits.conf \
	&& echo "*    -    noproc    65535" >> /etc/security/limits.conf

EXPOSE 9000

ENTRYPOINT [ "bash", "/docker-entrypoint.sh" ]

CMD [ "/bin/ghttp" ]