#/bin/bash
# get os info
os="linux"
if [ -f /etc/os-release ]; then
    . /etc/os-release
    os=$ID
fi
current_date=$(date +%Y%m%d)
architecture=`uname -m`
version=1.4
product_name=gStore
product_name_lower=$(echo "$product_name" | tr '[:upper:]' '[:lower:]')
static_pkg_name="$product_name_lower-$version-static-$os-$architecture.$current_date.tar.gz"
static_gcc_pkg_name="$product_name_lower-$version-static-gcc-$os-$architecture.$current_date.tar.gz"

echo "系统: $os"
echo "架构: $architecture"
echo "版本: $version"
echo "产品名称: $product_name"

echo "------cpack package start-----"
rm -rf bin/*
rm -rf build
rm -rf pfn
# cpack pacakage
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Cpack
make pre
make -j$(nproc)
cpack
cd ..
echo "------cpack package ok-----"
sleep 3s
# static pacakage
echo "------static package start-----"
mkdir -p ${product_name_lower}/.tmp
mkdir -p ${product_name_lower}/conf
mkdir -p ${product_name_lower}/scripts
cp -r conf bin data LICENSE README.md ${product_name_lower}/
cp -f conf/* ${product_name_lower}/conf/
cp -f scripts/test/api_test.sh ${product_name_lower}/scripts/api_test.sh
rm -f ${product_name_lower}/bin/.gitignore
rm -rf ${product_name_lower}/bin/.gconsole_history
tar -czf ${static_pkg_name} ${product_name_lower}
echo "------static package ok-----"
sleep 3s
# cpack_gcc pacakage
echo "------cpack gcc package start-----"
rm -rf bin/*
rm -rf build
rm -rf pfn
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=CpackGcc
make pre
make -j$(nproc)
make pfnbuild
cpack
cd ..
echo "------cpack gcc package ok-----"
# static_gcc pacakage
echo "------static gcc package start-----"
rm -rf ${product_name_lower}
mkdir -p ${product_name_lower}/.tmp
mkdir -p ${product_name_lower}/conf
mkdir -p ${product_name_lower}/scripts
cp -r conf bin data LICENSE README.md ${product_name_lower}/
cp -f conf/* ${product_name_lower}/conf/
cp -r pfn ${product_name_lower}/
cp -f scripts/test/api_test.sh ${product_name_lower}/scripts/api_test.sh
rm -f ${product_name_lower}/bin/.gitignore
rm -rf ${product_name_lower}/bin/.gconsole_history
tar -czf ${static_gcc_pkg_name} ${product_name_lower}
echo "------static gcc package ok-----"
sleep 3s
#docker pacakage
echo "------docker package start-----"
bash ./scripts/docker_build.sh zip
echo "------docker package ok-----"