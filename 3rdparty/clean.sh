#!/usr/bin/env bash

# rm include
rm -rf ../include/rapidjson
rm -rf ../include/antlr4
rm -rf ../include/workflow
rm -rf ../include/log4cplus
rm -rf ../include/indicators
rm -rf ../include/minizip
# rm static lib
rm -rf ../lib/libantlr4-runtime.a
rm -rf ../lib/libworkflow.a
rm -rf ../lib/liblog4cplus.a
rm -rf ../lib/libminizip.a

echo "clean 3rdparty success"