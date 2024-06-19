#!/bin/bash

echo "start build static package-------"

# clear build
echo "start clean build-------"
cd build/ 
if [ $? -eq 0 ]; then
    make clean
    rm -rf *
else
    echo "build dir not exist"
    mkdir -p build
    cd build/ 
fi
cmake .. -DCMAKE_BUILD_TYPE=Static
if [ $? -eq 0 ]; then
    make -j4 && make init
    if [ $? -eq 0 ]; then
        cd ..
        echo "begin build package-------"
        rm -rf gstore-1.3-static-centos-x86_64.tar.gz
        rm -rf gstore-1.3-static
        mkdir -p gstore-1.3-static/logs
        mkdir -p gstore-1.3-static/.tmp
        mkdir -p gstore-1.3-static/scripts
        mkdir -p gstore-1.3-static/conf
        mkdir -p gstore-1.3-static/dbhome
        cp -r conf backups bin docs data LICENSE README.md gstore-1.3-static/
        cp -r dbhome/system.db gstore-1.3-static/dbhome/
        cp -f conf/* gstore-1.3-static/conf/
        cp -rf scripts/test gstore-1.3-static/scripts/
        rm -f gstore-1.3-static/bin/.gitignore
        rm -f gstore-1.3-static/backups/.gitkeep
        tar -czvf gstore-1.3-static-centos-x86_64.tar.gz gstore-1.3-static/
        echo "build static package successfully!!!"
    else
        echo "build static package make fail!!!"
    fi
else
    echo "build static package cmake fail!!!"
fi

