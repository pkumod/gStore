/*
 * Socket.h
 *
 *  Created on: 2014-10-14
 *      Author: hanshuo
 */

#ifndef SOCKET_H_
#define SOCKET_H_

#include<sys/types.h>
#include<sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

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

#endif /* SOCKET_H_ */
