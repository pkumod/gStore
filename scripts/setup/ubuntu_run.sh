#!/usr/bin/env bash

apt-get update
apt-get install -y \
    libgomp1 \
    libssl3 \
    libjemalloc2 \
    libreadline8 \
    libuuid1 \
    && rm -rf /var/lib/apt/lists/*

echo "*    -    nofile    65535" >> /etc/security/limits.conf
echo "*    -    noproc    65535" >> /etc/security/limits.conf