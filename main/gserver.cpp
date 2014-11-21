/*
 * gserver.cpp
 *
 *  Created on: 2014-10-19
 *      Author: hanshuo
 */

#include"../Server/Server.h"

int main(int argc, char * argv[])
{
    Server server;

    server.createConnection();
    server.listen();

    return 0;
}

