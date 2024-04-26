#!/bin/bash

BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'
SERVER="$API_SERVER"

# Check if GSTORE_ROOT_PASSWORD is set
if [ -z "$GSTORE_ROOT_PASSWORD" ]; then
    echo "${RED}[INIT] GSTORE_ROOT_PASSWORD is not set. We strongly recommend setting a strong password.${NC}"
else
    echo "${BLUE}[INIT] Setting root password...${NC}"
    # Replace the line in the file
    sed -i -e "s/^#\\?\\s*root_password=.*/root_password=${GSTORE_ROOT_PASSWORD}/" init.conf
fi

if [ ! -d /gstore/system.db ]; then
    echo "${BLUE}[INIT] Creating system.db...${NC}"
    /gstore/bin/ginit --make
fi

echo "${BLUE}[INIT] Command: $@${NC}"

if [ -n $SERVER ] && [ $SERVER = "grpc" ]; then
    /gstore/bin/grpc
else
    /gstore/bin/ghttp
fi

exec "$@"