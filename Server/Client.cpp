/*
 * Client.cpp
 *
 *  Created on: 2014-10-19
 *      Author: hanshuo
 */

#include"Client.h"
#include<iostream>

Client::Client()
{
    this->ip = Socket::DEFAULT_SERVER_IP;
    this->port = Socket::DEFAULT_CONNECT_PORT;
}

Client::Client(std::string _ip, unsigned short _port)
{
    this->ip = _ip;
    this->port = _port;
}

Client::Client(unsigned short _port)
{
    this->ip = Socket::DEFAULT_SERVER_IP;
    this->port = _port;
}

Client::~Client()
{
}

bool Client::connectServer()
{
    bool flag = this->socket.create();
    if (!flag)
    {
        std::cerr << "cannot create socket. @Client::connectServer" << std::endl;
        return false;
    }

    flag = this->socket.connect(this->ip, this->port);

    if (!flag)
    {
        std::cerr << "cannot connect to server. @Client::connectServer" << std::endl;
        return false;
    }

    return true;
}

bool Client::disconnectServer()
{
    bool flag = this->socket.close();

    return flag;
}

bool Client::send(const std::string& _msg)
{
    bool flag = this->socket.send(_msg);

    return flag;
}

bool Client::receiveResponse(std::string& _recv_msg)
{
    bool flag = this->socket.recv(_recv_msg);

    return flag;
}

void Client::run()
{
    while (true)
    {
        std::string cmd;

        while (true)
        {
            std::cout << "->";
            std::string line;
            std::getline(std::cin, line);
            int line_len = line.size();
            if (line_len >0 && line[line_len-1] == ';')
            {
                line.resize(line_len - 1);
                cmd += line;
                break;
            }
            cmd += line + "\n";
        }

//        std::cout << "input end" << std::endl;

        if (cmd == "exit")
        {
            break;
        }

        bool flag = this->connectServer();
        if (!flag)
        {
            std::cerr << "connect server error. @Client::run" << std::endl;
            continue;
        }

        flag = this->send(cmd);
        if (!flag)
        {
            std::cerr << "sent message error. @Client::run" << std::endl;
            continue;
        }

        std::string recv_msg;
        flag = this->receiveResponse(recv_msg);
        std::cout << recv_msg << std::endl;

        this->disconnectServer();
        if (!flag)
        {
            std::cerr << "disconnect server error. @Client::run" << std::endl;
            continue;
        }
    }
}
