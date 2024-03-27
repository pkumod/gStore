#!/bin/bash

# build cmake
mkdir -p ./build
cd ./build/
cmake ..
make pre
make -j4
# init system.db
make init