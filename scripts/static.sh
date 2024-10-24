#/bin/bash

os="linux"
if [ -f /etc/os-release ]; then
    . /etc/os-release
    os=$ID
fi
architecture=`uname -m`
version=$(awk -F '=' '/version/ {print$2}' "conf/conf.ini")
product_name=$(awk -F '=' '/product_name/ {print$2}' "conf/conf.ini")
product_name_lower=$(echo "$product_name" | tr '[:upper:]' '[:lower:]')
static_pkg_name="$product_name_lower-$version-static-$os-$architecture"

echo "os: $os"
echo "architecture: $architecture"
echo "version: $version"
echo "production: $product_name"
echo "package: $static_pkg_name.tar.gz"

echo "start build static package-------"

# clear build
echo "start clean build-------"
cd build/ 
if [ $? -eq 0 ]; then
    make clean
    rm -rf *
    cmake .. -DCMAKE_BUILD_TYPE=Static
else
    echo "build dir not exist"
    mkdir -p build
    cd build/
    cmake .. -DCMAKE_BUILD_TYPE=Static
    make pre 
fi
if [ $? -eq 0 ]; then
    make -j4 && make init
    if [ $? -eq 0 ]; then
        cd ..
        echo "begin build package-------"
        rm -rf ${static_pkg_name}.tar.gz
        rm -rf ${product_name_lower}
        mkdir -p ${product_name_lower}/logs
        mkdir -p ${product_name_lower}/.tmp
        mkdir -p ${product_name_lower}/scripts
        mkdir -p ${product_name_lower}/conf
        cp -r conf backups bin docs data LICENSE README.md ${product_name_lower}/
        cp -f conf/* ${product_name_lower}/conf/
        cp -rf scripts/test ${product_name_lower}/scripts/
        rm -f ${product_name_lower}/bin/.gitignore
        rm -f ${product_name_lower}/backups/.gitkeep
        tar -czvf ${static_pkg_name}.tar.gz ${product_name_lower}
        echo "build static package successfully!!!"
    else
        echo "build static package make fail!!!"
    fi
else
    echo "build static package cmake fail!!!"
fi

