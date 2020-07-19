/*
 * Socket.h
 *
 *  Created on: 2014-10-14
 *      Author: hanshuo
 */

#ifndef _SERVER_SOCKET_H
#define _SERVER_SOCKET_H

//#include "../Util/Util.h"
#include <iostream>
#include <string>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
//DBL_MAX is contained in the header below
#include <float.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include <locale.h>
#include <assert.h>
#include <libgen.h>
#include <signal.h>

class Socket {
  public:
  Socket();
  ~Socket();

  bool create();
  bool close();
  bool bind(const unsigned short _port); //0~65535
  bool listen() const;
  bool accept(Socket& _new_socket) const;

  bool connect(const std::string _hostname, const unsigned short _port);

  bool send(const std::string& _msg) const;
  int recv(std::string& _msg) const;

  bool isValid() const;

  static const int MAX_CONNECTIONS = 20;
  static const unsigned short DEFAULT_CONNECT_PORT = 3305;
  static const std::string DEFAULT_SERVER_IP;

  private:
  int sock;
  sockaddr_in addr;
};

#endif // _SERVER_SOCKET_H
