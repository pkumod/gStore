#!/usr/bin/env bash

rm -rf workflow
tar -xzvf workflow-0.10.3.tar.gz
cd ./workflow || echo "./workflow not found"

make -j$(nproc)
# if GSTORE_USE_DYNAMIC_LIB is defined, then we need to copy the dynamic library to the lib directory
# otherwise, we need to copy the static library to the lib directory

if [ -n "$GSTORE_USE_DYNAMIC_LIB" ]; then
    cp _lib/libworkflow.so* ../../lib/
    cd ../../lib/ || echo "./lib not found"
else
    cp _lib/libworkflow.a ../../lib/libworkflow.a
fi
cp _include/workflow ../../include/workflow -r
