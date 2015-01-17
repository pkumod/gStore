/*
 * gclient.cpp
 *
 *  Created on: 2014-10-19
 *      Author: hanshuo
 */

#include"../Server/Client.h"
#include<string>
#include<sstream>
#include<iostream>

int main(int argc, char * argv[])
{
    std::string ip = Socket::DEFAULT_SERVER_IP;
    unsigned short port = Socket::DEFAULT_CONNECT_PORT;

    if (argc == 2)
    {
        std::stringstream ss;
        ss << argv[1];
        ss >> port;
    }
    else if (argc >= 3)
    {
        std::stringstream ss;
        ss << argv[1] << " " << argv[2];
        ss >> ip >> port;
    }

    std::cout << "ip=" << ip << " port=" << port << std::endl; //debug
    Client client(ip, port);

    client.run();

    return 0;
}
