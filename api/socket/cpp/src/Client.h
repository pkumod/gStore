/**
* @file  Client.h
* @author  suxunbin
* @date  13-AUG-2021
* @brief  a gStore socket client
*/

#ifndef _CLIENT_H
#define _CLIENT_H

#include "../../../../Server/Socket.h"
#include <cstring>
#include <iostream>

#define defaultServerIP "127.0.0.1"
#define defaultServerPort 9000

class GstoreConnector
{
public:
	GstoreConnector();
	GstoreConnector(std::string _ip);
	GstoreConnector(int _port);
	GstoreConnector(std::string _ip, int _port);
	~GstoreConnector();

	bool connect();
	bool send(std::string _cmd);
	bool recv(std::string& _recv_msg);
	bool disconnect();
	int parseJson(std::string strJson);

private:
	std::string serverIP;
	int serverPort;
	Socket socket;
};

#endif /* _CLIENT_H */
