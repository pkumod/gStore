#!/usr/bin/env bash

rm -rvf ../lib/libworkflow.a
rm -rvf ../include/workflow
rm -rvf ./workflow/_*
tar -xzvf workflow-0.10.3.tar.gz
cd ./workflow || echo "./workflow not found"
make -j 2
cp _lib/libworkflow.a ../../lib
cp _include/workflow ../../include/workflow -r
