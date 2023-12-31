#!/usr/bin/env bash

mkdir -p ../include/
rm -rf antlr4-cpp-runtime-4
tar -xzvf antlr4-cpp-runtime-4.tar.gz;
cd antlr4-cpp-runtime-4 || echo "./antlr4-cpp-runtime-4 not found"
cmake .
make -j2
# if GSTORE_USE_DYNAMIC_LIB is defined, then we need to copy the dynamic library to the lib directory
# otherwise, we need to copy the static library to the lib directory
if [ -n "$GSTORE_USE_DYNAMIC_LIB" ]; then
    cp dist/libantlr4-runtime.so ../../lib/libantlr4-runtime.so.4.7.2
    cd ../../lib/ || echo "./lib not found"
    ln -s libantlr4-runtime.so.4.7.2 libantlr4-runtime.so.4
    ln -s libantlr4-runtime.so.4.7.2 libantlr4-runtime.so
else
    cp dist/libantlr4-runtime.a ../../lib/
fi
cp -r runtime/src/* ../../include/