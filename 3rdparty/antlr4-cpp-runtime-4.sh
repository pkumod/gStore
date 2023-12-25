#!/usr/bin/env bash

mkdir -p ../include/
rm -rf antlr4-cpp-runtime-4
tar -xzvf antlr4-cpp-runtime-4.tar.gz;
cd antlr4-cpp-runtime-4 || echo "./antlr4-cpp-runtime-4 not found"
cmake .
make -j2
cp dist/libantlr4-runtime.a ../../lib/
cp -r runtime/src/* ../../include/