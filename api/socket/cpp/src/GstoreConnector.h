/*=============================================================================
# Filename: GstoreConnector.h
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2016-02-21 21:22
# Description: originally written by hanshuo, modified by zengli
=============================================================================*/

#ifndef _GSTORECONNECTOR_H
#define _GSTORECONNECTOR_H

#include "../../../../Server/Socket.h"
#include <cstring>

class GstoreConnector {
  public:
  GstoreConnector();
  GstoreConnector(std::string _ip);
  GstoreConnector(unsigned short _port);
  GstoreConnector(std::string _ip, unsigned short _port);
  ~GstoreConnector();

  bool test();
  bool load(std::string _db_name);
  bool unload(std::string _db_name);
  bool build(std::string _db_name, std::string _rdf_file_path);
  bool drop(std::string _db_name);
  std::string query(std::string _sparql, std::string _output = "/");
  std::string show(bool _type = false); //show current or all databases

  static const std::string defaultServerIP;
  static const unsigned short defaultServerPort;

  private:
  std::string serverIP;
  unsigned short serverPort;
  //    std::string username;
  //    std::string password;
  Socket socket;

  bool connect();
  bool disconnect();
};

#endif // _GSTORECONNECTOR_H
