#!/bin/bash

# go to gstore dir
# basic test

cd ../../
echo "basic build/query/add/sub/drop test start......"
bash scripts/test/basic_test.sh
echo "basic build/query/add/sub/drop test end"

# parser test
echo "parser test start......"
bash scripts/test/parser_test.sh
echo "parser test end"


cd scripts/test/
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

