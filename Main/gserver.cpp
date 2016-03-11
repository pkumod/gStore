/*=============================================================================
# Filename: gserver.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-02-26 19:15
# Description: first written by hanshuo, modified by zengli
=============================================================================*/

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

