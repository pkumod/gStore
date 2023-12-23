#!/usr/bin/env bash

BLUE='\033[0;34m'
NC='\033[0m'

cd $(dirname "${BASH_SOURCE[0]}")

mkdir -p ../lib ../include

for f in *.sh; do
    if [ "$f" != "init.sh" ]; then
        echo "${BLUE}Running $f${NC}"
        bash "$f"
        if [ $? -ne 0 ]; then
            echo "${BLUE}Failed to run $f${NC}"
            exit 1
        fi
    fi
done

