#!/bin/bash

mkdir -p ../include ../lib

# rm include
rm -rf ../include/antlr4/
rm -rf ../include/workflow
rm -rf ../include/log4cplus
rm -rf ../include/indicators
rm -rf ../include/minizip
# rm static lib
rm -rf ../lib/libantlr4-runtime.a 
rm -rf ../lib/libworkflow.a
rm -rf ../lib/liblog4cplus.a
rm -rf ../lib/libminizip.a
# tar
tar -xzvf antlr4-cpp-runtime-4.tar.gz;
tar -xzvf indicators.tar.gz;
tar -xzvf workflow-0.10.3.tar.gz;
tar -xzvf log4cplus-2.0.8.tar.gz;
tar -xzvf zlib-1.3.tar.gz;
cd ./antlr4-cpp-runtime-4/; cmake .; make -j$(nproc); cp dist/libantlr4-runtime.a ../../lib/;
cd ../workflow; make -j$(nproc); cp _lib/libworkflow.a ../../lib/;
cd ../log4cplus; ./configure --enable-static; make -j$(nproc); cp .libs/liblog4cplus.a ../../lib/;
cd ../zlib-1.3; ./configure; make -j$(nproc); cp *.h ./include/; cd contrib/minizip; make -j$(nproc); cp *.h ../../include/; cp libminizip.a ../../../../lib/;
# mv to include
cd ../../../../include
mkdir -p antlr4 workflow log4cplus indicators minizip
cd ../3rdparty
cp -r antlr4-cpp-runtime-4/runtime/src/* ../include/antlr4/
cp -r workflow/_include/workflow/* ../include/workflow/
cp -r log4cplus/include/log4cplus/* ../include/log4cplus/
cp -r indicators/* ../include/indicators/
cp -r zlib-1.3/include/* ../include/minizip/
rm -rf antlr4-cpp-runtime-4 workflow log4cplus indicators zlib-1.3

echo "prepare successfully"