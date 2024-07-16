#!/bin/bash

BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m'
SERVER="$API_SERVICE"
PASSWD="$ROOT_PASSWD"

# Check init
if [ ! -e /gstore/init.lock ]; then
    if [ -n $PASSWD ] && [ $PASSWD != "123456" ]; then
         # Replace the line in the file
        sed -i -e "s/^#\\?\\s*root_password=.*/root_password=${PASSWD}/" conf/conf.ini
    else
        PASSWD="123456"
    fi
    /gstore/bin/ginit
    touch /gstore/init.lock
    echo "${BLUE}[INIT] Initialization complete${NC}"
    echo "${BLUE}[INIT] Root password ${PASSWD}${NC}"
fi

echo "${BLUE}[INFO] API SERVER ${SERVER}${NC}"

if [ -n $SERVER ] && [ $SERVER = grpc ]; then
    /gstore/bin/grpc
else
    /gstore/bin/ghttp
fi

exec "$@"