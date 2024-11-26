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
static_pkg_name="$product_name_lower-$version-static-$os-$architecture"

echo "系统: $os"
echo "架构: $architecture"
echo "版本: $version"
echo "产品名称: $product_name"
echo "安装包名: $static_pkg_name"

echo "start build static package-------"

# clear build
echo "start clean build-------"
rm -rf ${static_pkg_name}.tar.gz
rm -rf ${product_name_lower}
rm -rf build
mkdir -p build
cd build/
cmake .. -DCMAKE_BUILD_TYPE=Static
make pre
if [ $? -eq 0 ]; then
    make -j$(nproc)
    if [ $? -eq 0 ]; then
        cd ..
        echo "begin build package-------"
        mkdir -p ${product_name_lower}/.tmp
        mkdir -p ${product_name_lower}/conf
        mkdir -p ${product_name_lower}/scripts
        cp -r conf bin data LICENSE README.md ${product_name_lower}/
        cp -f conf/* ${product_name_lower}/conf/
        cp -f scripts/test/api_test.sh ${product_name_lower}/scripts/api_test.sh
        rm -f ${product_name_lower}/bin/.gitignore
        rm -rf ${product_name_lower}/bin/.gconsole_history
        tar -czvf ${static_pkg_name}.tar.gz ${product_name_lower}
        echo "build static package successfully!!!"
    else
        echo "build static package make fail!!!"
    fi
else
    echo "build static package cmake fail!!!"
fi

