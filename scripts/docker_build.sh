#!/bin/bash
current_date=$(date +%Y%m%d)
zip_file="gstore-1.4-image.$current_date.zip"
tar_file="gstore-1.4.tar"
img_name="pkumod/gstore:1.4"

tar -czf gstore.tar.gz api conf 3rdparty data scripts src CMakeLists.txt README.md README_ZH.md LICENSE include/nlohmann/json.hpp
docker rmi -f "$img_name"
echo "----------1/5 remove $img_name ok----------"
docker build -t "$img_name" .
echo "----------2/5 build $img_name ok----------"
docker save -o "$tar_file" "$img_name"
echo "----------3/5 save $img_name to $tar_file ok----------"
zip -r "$zip_file" "$tar_file"
echo "----------4/5 zip $tar_file to $zip_file ok----------"
rm -f "$tar_file" gstore.tar.gz
echo "----------5/5 clear tmp file ok----------"