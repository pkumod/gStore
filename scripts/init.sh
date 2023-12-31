#!/bin/bash
###
 # @Author: your name
 # @Date: 2021-09-10 09:55:24
 # @LastEditTime: 2021-09-10 09:56:38
 # @LastEditors: Please set LastEditors
 # @Description: In User Settings Edit
 # @FilePath: /gstore/scripts/init.sh
###
#set -v
#initialize system.db
#"bin/ginit" "-make" >& /dev/null

# Get the directory name of the current script
ROOT_DIR=$(dirname "${BASH_SOURCE[0]}")

[ -d "./.tmp" ] || mkdir -p "./.tmp"
[ -d "./.debug" ] || mkdir -p "./.debug"
[ -d "./data" ] || cp -r "${ROOT_DIR}/../tests/data" "./data"
cp -rv "${ROOT_DIR}/../lib/*.so" "./lib/"

"bin/ginit" "--make"