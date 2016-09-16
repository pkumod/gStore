/*
 * Client.h
 *
 *  Created on: 2014-10-18
 *      Author: hanshuo
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include"Socket.h"
#include<string>

class Client
{
public:
    Client();
    Client(std::string _ip, unsigned short _port);
    Client(unsigned short _port);
    ~Client();

    bool connectServer();
    bool disconnectServer();
    bool send(const std::string& _msg);
    bool receiveResponse(std::string& _recv_msg);

    void run();
private:
    std::string ip;
    unsigned short port;
    std::string password;
    Socket socket;
    int random_key;
};



#endif /* CLIENT_H_ */
