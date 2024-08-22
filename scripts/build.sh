#!/bin/bash

# build cmake
source /etc/profile
mkdir -p ./build
cd ./build/
cmake ..
make pre
make -j4
# init system.db
make init

echo "Compilation ends successfully!"