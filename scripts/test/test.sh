#!/bin/bash

# basic test
echo "basic build/query/add/sub/drop test start......"
bash basic_test.sh
echo "basic build/query/add/sub/drop test end"

# parser test
echo "parser test start......"
bash parser_test.sh
echo "parser test end"

# update test gtest
# build cmake
mkdir -p ./bin ./lib
cp -r ../../build/lib/* ./lib/
mkdir -p build
cd ./build/
cmake ..
make -j4
cd ../

echo "repeatedly insertion/deletion test start......"
#bin/update_test 100
echo "repeatedly insertion/deletion test end"

echo "repeatedly insertion/deletion gtest start......"
#bin/gtest
echo "repeatedly insertion/deletion gtest end"

rm -rf build bin lib

