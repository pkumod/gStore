#!/bin/bash

BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'

if [ ! -f /gstore/init.conf ]; then
    echo "${BLUE}[INIT] No init.conf file found. Copying default...${NC}"
    cp -r /docker-init/* /gstore/

    # Check if GSTORE_ROOT_PASSWORD is set
    if [ -z "$GSTORE_ROOT_PASSWORD" ]; then
        echo "${RED}[INIT] GSTORE_ROOT_PASSWORD is not set. We strongly recommend setting a strong password.${NC}"
    else
        echo "${BLUE}[INIT] Setting root password...${NC}"
        # Replace the line in the file
        sed -i -e "s/^#\\?\\s*root_password=.*/root_password=${GSTORE_ROOT_PASSWORD}/" init.conf
    fi

fi

if [ ! -d /gstore/bin ]; then
    echo "${BLUE}[INIT] Creating directories...${NC}"
    mkdir -p bin lib backups data logs .tmp
fi

if [ ! -d /gstore/system.db ]; then
    echo "${BLUE}[INIT] Creating system.db...${NC}"
    /usr/local/bin/ginit --make

    # list all directories in /gstore/data
    # for dir in /gstore/data/*; do
    #     # get the directory name
    #     dir_name=$(basename "$dir")
    #     if [ $dir_name != "system" ] && [ -d "data/$dir_name" ] && [ -f "data/$dir_name/$dir_name.nt" ] ; then
    #         # create the database
    #         echo "${BLUE}[INIT] Creating $dir_name...${NC}"
    #         /usr/local/bin/gbuild -db "$dir_name" -f "data/$dir_name/$dir_name.nt"
    #     fi
    # done
fi

echo "${BLUE}[INIT] Command: $@${NC}"

if [ -n "${API_SERVER}" ] && [ "'${API_SERVER}'" == 'grpc' ]; then
/usr/local/bin/grpc
else
/usr/local/bin/ghttp
fi

exec "$@"