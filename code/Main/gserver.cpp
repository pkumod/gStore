/*
 * gserver.cpp
 *
 *  Created on: 2014-10-19
 *      Author: hanshuo
 */

#include "../Server/Server.h"
#include "../Util/Util.h"

int main(int argc, char * argv[])
{
#ifdef DEBUG
	Util util;
#endif
    unsigned short port = Socket::DEFAULT_CONNECT_PORT;

    if (argc > 1)
    {
        std::stringstream ss(argv[1]);
        ss >> port;
    }

    Server server(port);

    std::cout << "port=" << port << std::endl; //debug

    server.createConnection();
    server.listen();

    return 0;
}

