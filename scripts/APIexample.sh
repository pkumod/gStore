#/bin/bash

cd api/http/cpp/src
make clean
make -j4
cd ../example
make clean
make -j4