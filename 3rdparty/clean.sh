#!/usr/bin/env bash

make clean
rm -rf ./*
# rm include
rm -rf ../include
# rm static lib
rm -rf ../lib

echo "clean 3rdparty success"