#!/usr/bin/env bash

apt-get update
apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    mold \
    python3-pip \
    pkg-config \
    uuid-dev \
    libjemalloc-dev \
    libreadline-dev \
    libssl-dev \
    lcov

echo "*    -    nofile    65535" >> /etc/security/limits.conf
echo "*    -    noproc    65535" >> /etc/security/limits.conf