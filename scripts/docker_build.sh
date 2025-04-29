#!/bin/bash
# get os info
os="linux"
if [ -f /etc/os-release ]; then
    . /etc/os-release
    os=$ID
fi
architecture=`uname -m`
version=1.4
current_date=$(date +%Y%m%d)
zip_file="gstore-$version-image-$os-$architecture.$current_date.zip"
tar_file="gstore-$version.tar"
img_name="pkumod/gstore:$version"

tar -czf gstore.tar.gz api conf 3rdparty data scripts src CMakeLists.txt README.md README_ZH.md LICENSE include/nlohmann/json.hpp
docker rmi -f "$img_name"
echo "----------1/5 remove $img_name ok----------"
docker build -t "$img_name" .
echo "----------2/5 build $img_name ok----------"
# 如果第一个参数为zip，则执行
if [ "$1" = "zip" ]; then
docker save -o "$tar_file" "$img_name"
echo "----------3/5 save $img_name ok----------"
zip -rq "$zip_file" "$tar_file"
echo "----------4/5 zip $tar_file ok----------"
echo "$zip_file"
else
echo "----------3/5 skip export docker image----------"
echo "----------4/5 skip zip docker image----------"
fi
rm -f "$tar_file" gstore.tar.gz
echo "----------5/5 clear tmp file ok----------"