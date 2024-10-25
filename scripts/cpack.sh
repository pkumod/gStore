#/bin/bash

# get os info
os="linux"
if [ -f /etc/os-release ]; then
    . /etc/os-release
    os=$ID
fi
architecture=`uname -m`
version=$(awk -F '=' '/version/ {print$2}' "conf/conf.ini")
product_name=$(awk -F '=' '/product_name/ {print$2}' "conf/conf.ini")
product_name_lower=$(echo "$product_name" | tr '[:upper:]' '[:lower:]')

echo "系统: $os"
echo "架构: $architecture"
echo "版本: $version"
echo "产品名称: $product_name"

echo "start cpack package-------"

# clear build
echo "start clean build-------"
cd build/ 
if [ $? -eq 0 ]; then
    make clean
    rm -rf *
    cmake .. -DCMAKE_BUILD_TYPE=Cpack
    make -j4
    cpack
else
    echo "build dir not exist"
    mkdir -p build
    cd build/
    make clean
    rm -rf *
    cmake .. -DCMAKE_BUILD_TYPE=Cpack
    make pre
    make -j4
    cpack
fi

