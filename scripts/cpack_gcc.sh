#/bin/bash

# get os info
os="linux"
if [ -f /etc/os-release ]; then
    . /etc/os-release
    os=$ID
fi
architecture=`uname -m`
version=1.4
product_name=gStore
product_name_lower=$(echo "$product_name" | tr '[:upper:]' '[:lower:]')

echo "系统: $os"
echo "架构: $architecture"
echo "版本: $version"
echo "产品名称: $product_name"

echo "start cpack package-------"

# clear build
echo "start clean build-------"
rm -rf build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=CpackGcc
make pre
make -j$(nproc)
bash ./scripts/pfn_build.sh
cpack