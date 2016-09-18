/*
 * Socket.h
 *
 *  Created on: 2014-10-14
 *      Author: hanshuo
 */

#ifndef _SERVER_SOCKET_H
#define _SERVER_SOCKET_H

#include "../Util/Util.h"

class Socket
{
public:
    Socket();
    ~Socket();

    bool create();
    bool close();
    bool bind(const unsigned short _port); //0~65535
    bool listen()const;
    bool accept(Socket& _new_socket)const;

    bool connect(const std::string _hostname, const unsigned short _port);

    bool send(const std::string& _msg)const;
    int recv(std::string& _msg)const;

    bool isValid()const;

    static const int MAX_CONNECTIONS = 20;
    static const unsigned short DEFAULT_CONNECT_PORT = 3305;
    static const std::string DEFAULT_SERVER_IP;

private:
    int sock;
    sockaddr_in addr;
};

#endif // _SERVER_SOCKET_H

