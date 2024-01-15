#!/bin/bash

# Exit on error
set -e

# Get the directory name of the current script
ROOT_DIR=$(dirname "${BASH_SOURCE[0]}")

BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

if [ ! -d ./.tmp ]; then
    echo -e "${BLUE} [INIT] Creating directories... ${NC}"
    mkdir -p bin lib backups logs .tmp .debug
fi

if [ ! -f ./init.conf ]; then
    echo -e "${BLUE} [INIT] No init.conf file found. Copying default... ${NC}"
    cp -n -r "${ROOT_DIR}"/../defaults/* "./"
    cp -n -r "${ROOT_DIR}"/../lib/*.so* "./lib/"
    if [ -d /data ] && [ ! -d ./data/system ]; then
        echo -e "${BLUE} [INIT] Creating symlink... ${NC}"
        ln -s /data ./data
    fi


    # Check if GSTORE_ROOT_PASSWORD is set
    if [ -z "$GSTORE_ROOT_PASSWORD" ]; then
        echo -e "${RED} [INIT] GSTORE_ROOT_PASSWORD is not set. We strongly recommend setting a strong password. ${NC}"
    else
        echo -e "${BLUE} [INIT] Setting root password... ${NC}"
        # Replace the line in the file
        sed -i -e "s/^#\\?\\s*root_password=.*/root_password=${GSTORE_ROOT_PASSWORD}/" init.conf
    fi
fi

if [ ! -d ./system.db ]; then
    echo -e "${BLUE} [INIT] Creating system.db... ${NC}"
    "${ROOT_DIR}/../bin/ginit" --make

    # list all directories in /app/data
    for dir in "${ROOT_DIR}"/../data/*; do
        # get the directory name
        dir_name=$(basename "$dir")
        if [ "$dir_name" != "system" ] && [ -d "data/$dir_name" ] && [ -f "data/$dir_name/$dir_name.nt" ] ; then
            # create the database
            echo -e "${BLUE} [INIT] Creating $dir_name... ${NC}"
            "${ROOT_DIR}/../bin/gbuild" -db "$dir_name" -f "data/$dir_name/$dir_name.nt"
        fi
    done
fi

echo -e "${BLUE} [INIT] Command:" "$@" "${NC}"

exec "$@"