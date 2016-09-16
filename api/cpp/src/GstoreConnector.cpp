/*
 * GstoreConnector.cpp
 *
 *  Created on: 2014-11-1
 *      Author: hanshuo
 */

#include"GstoreConnector.h"
#include<iostream>

std::string GstoreConnector::defaultServerIP = "127.0.0.1";
unsigned short GstoreConnector::defaultServerPort = 3305;

GstoreConnector::GstoreConnector()
{
    this->serverIP = GstoreConnector::defaultServerIP;
    this->serverPort = GstoreConnector::defaultServerPort;
}

GstoreConnector::GstoreConnector(unsigned short _port)
{
    this->serverIP = GstoreConnector::defaultServerIP;
    this->serverPort = _port;
}

GstoreConnector::GstoreConnector(std::string _ip, unsigned short _port)
{
    this->serverIP = _ip;
    this->serverPort = _port;
}

GstoreConnector::~GstoreConnector()
{

}

bool GstoreConnector::load(std::string _db_name)
{
    bool connect_return = this->connect();
    if (!connect_return)
    {
        std::cerr << "connect to server error. @GstoreConnector::load" << std::endl;
        return false;
    }

    std::string cmd = "load " + _db_name;
    bool send_return = this->socket.send(cmd);
    if (!send_return)
    {
        std::cerr << "send load command error. @GstoreConnector.load" << std::endl;
        return false;
    }

    std::string recv_msg;
    this->socket.recv(recv_msg);

    this->disconnect();

    std::cout << recv_msg << std::endl; //debug
    if (recv_msg == "load database done.")
    {
        return true;
    }

    return false;
}

bool GstoreConnector::unload(std::string _db_name)
{
    bool connect_return = this->connect();
    if (!connect_return)
    {
        std::cerr << "connect to server error. @GstoreConnector::unload" << std::endl;
        return false;
    }

    std::string cmd = "unload " + _db_name;
    bool send_return = this->socket.send(cmd);
    if (!send_return)
    {
        std::cerr << "send unload command error. @GstoreConnector::unload" << std::endl;
        return false;
    }

    std::string recv_msg;
    this->socket.recv(recv_msg);

    this->disconnect();

    std::cout << recv_msg << std::endl; //debug
    if (recv_msg == "unload database done.")
    {
        return true;
    }

    return false;
}

bool GstoreConnector::build(std::string _db_name, std::string _rdf_file_path)
{
    bool connect_return = this->connect();
    if (!connect_return)
    {
        std::cerr << "connect to server error. @GstoreConnector::build" << std::endl;
        return false;
    }

    std::string cmd = "import " + _db_name + " " + _rdf_file_path;
    bool send_return = this->socket.send(cmd);
    if (!send_return)
    {
        std::cerr << "send import command error. @GstoreConnector::build" << std::endl;
        return false;
    }

    std::string recv_msg;
    this->socket.recv(recv_msg);

    this->disconnect();

    std::cerr << recv_msg << std::endl; //debug
    if (recv_msg == "import RDF file to database done.")
    {
        return true;
    }

    return false;
}

std::string GstoreConnector::query(std::string _sparql)
{
    bool connect_return = this->connect();
    if (!connect_return)
    {
        std::cerr << "connect to server error. @GstoreConnector::query" << std::endl;
        return "connect to server error.";
    }

    std::string cmd = "query " + _sparql;
    bool send_return = this->socket.send(cmd);
    if (!send_return)
    {
        std::cerr << "send query command error. @GstoreConnector::query";
        return "send query command error.";
    }

    std::string recv_msg;
    this->socket.recv(recv_msg);

    this->disconnect();

    return recv_msg;
}

bool GstoreConnector::connect()
{
    bool flag = this->socket.create();
    if (!flag)
    {
        std::cerr << "cannot create socket. @GstoreConnector::connect" << std::endl;
        return false;
    }

    flag = this->socket.connect(this->serverIP, this->serverPort);

    if (!flag)
    {
        std::cerr << "cannot connect to server. @GstoreConnector::connect" << std::endl;
        return false;
    }

    return true;
}

bool GstoreConnector::disconnect()
{
    bool flag = this->socket.close();

    return flag;
}
