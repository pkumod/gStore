/*
 * gserver.cpp
 *
 *  Created on: 2014-10-19
 *      Author: hanshuo
 */

#include"../Server/Server.h"
#include<sstream>

int main(int argc, char * argv[])
{
    unsigned short port = Socket::DEFAULT_CONNECT_PORT;

    if (argc > 1)
    {
        std::stringstream ss(argv[1]);
        ss >> port;

        std::cout << "port=" << port << std::endl; //debug
    }

    Server server(port);

    server.createConnection();
    server.listen();

    return 0;
}

