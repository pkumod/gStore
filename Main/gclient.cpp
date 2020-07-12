/*=============================================================================
# Filename: gclient.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2016-02-26 19:14
# Description: first written by hanshuo, modified by zengli
=============================================================================*/

#include "../Server/Client.h"
#include "../Util/Util.h"

int main(int argc, char* argv[])
{
  //chdir(dirname(argv[0]));
  //#ifdef DEBUG
  Util util;
  //#endif

  std::string ip = Socket::DEFAULT_SERVER_IP;
  unsigned short port = Socket::DEFAULT_CONNECT_PORT;

  if (argc == 2) {
    std::stringstream ss;
    ss << argv[1];
    ss >> port;
  } else if (argc >= 3) {
    std::stringstream ss;
    ss << argv[1] << " " << argv[2];
    ss >> ip >> port;
  }

  std::cout << "ip=" << ip << " port=" << port << std::endl; //debug
  Client client(ip, port);

  client.run();

  return 0;
}
