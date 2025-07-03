#!/bin/bash

# build cmake
# source /etc/profile
mkdir -p ./build
cd ./build/
if [ "$1" = "debug" ]; then
    echo "debug mode"
    cmake .. -DCMAKE_BUILD_TYPE=Debug
else
    cmake ..
fi
make pre
make -j$(nproc)
cd ..
bash scripts/pfn_build.sh 
# # init system.db
# make init
# make APIexample

echo "Compilation ends successfully!"