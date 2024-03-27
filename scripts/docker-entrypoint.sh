#!/bin/bash

BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

if [ ! -f /app/conf/conf.ini ]; then
    echo "${BLUE}[INIT] No init.conf file found. Copying default...${NC}"
    cp -r /docker-init/* /app/

    # Check if GSTORE_ROOT_PASSWORD is set
    if [ -z "$GSTORE_ROOT_PASSWORD" ]; then
        echo "${RED}[INIT] GSTORE_ROOT_PASSWORD is not set. We strongly recommend setting a strong password.${NC}"
    else
        echo "${BLUE}[INIT] Setting root password...${NC}"
        # Replace the line in the file
        sed -i -e "s/^#\\?\\s*root_password=.*/root_password=${GSTORE_ROOT_PASSWORD}/" init.conf
    fi

fi

if [ ! -d /app/bin ]; then
    echo "${BLUE}[INIT] Creating directories...${NC}"
    mkdir -p bin lib backups data logs .tmp
fi

if [ ! -d /app/dbhome/system.db ]; then
    echo "${BLUE}[INIT] Creating system.db...${NC}"
    /usr/local/bin/ginit --make

    # list all directories in /app/data
    for dir in /app/data/*; do
        # get the directory name
        dir_name=$(basename "$dir")
        if [ $dir_name != "system" ] && [ -d "data/$dir_name" ] && [ -f "data/$dir_name/$dir_name.nt" ] ; then
            # create the database
            echo "${BLUE}[INIT] Creating $dir_name...${NC}"
            /usr/local/bin/gbuild -db "$dir_name" -f "data/$dir_name/$dir_name.nt"
        fi
    done
fi

echo "${BLUE}[INIT] Command: $@${NC}"

exec "$@"

/usr/local/bin/ghttp