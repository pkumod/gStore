/*=============================================================================
# Filename: Client.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-02-23 13:40
# Description: originally written by hanshuo, modified by zengli
=============================================================================*/

#ifndef _SERVER_CLIENT_H
#define _SERVER_CLIENT_H

#include "../Util/Util.h"
#include "Socket.h"

class Client {
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

#endif // _SERVER_CLIENT_H
