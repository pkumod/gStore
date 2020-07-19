/*=============================================================================
# Filename: GstoreConnector.h
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2016-02-21 21:22
# Description: originally written by hanshuo, modified by zengli
=============================================================================*/

#ifndef _GSTORECONNECTOR_H
#define _GSTORECONNECTOR_H

#include <cstring>
#include "client.h"

//TODO: C++ URL encoder
//http://blog.csdn.net/nanjunxiao/article/details/9974593
//TODO: for a string, we should add \"\" in the head and tail, while for a path not do so

class GstoreConnector {
  public:
  GstoreConnector();
  GstoreConnector(std::string _ip);
  GstoreConnector(unsigned short _port);
  GstoreConnector(std::string _ip, unsigned short _port);
  ~GstoreConnector();

  //bool test();
  bool load(std::string _db_name, std::string username, std::string password);
  bool unload(std::string _db_name, std::string username, std::string password);
  bool build(std::string _db_name, std::string _rdf_file_path, std::string username, std::string password);
  bool drop(std::string _db_name);
  std::string query(std::string username, std::string password, std::string db_name, std::string sparql);
  void query(std::string username, std::string password, std::string db_name, std::string sparql, std::string filename);
  std::string show(std::string username, std::string password); //show all databases
  std::string user(std::string type, std::string username1, std::string password1, std::string username2, std::string addtion);
  std::string showUser();
  std::string monitor(std::string db_name, std::string username, std::string password);
  std::string checkpoint(std::string db_name, std::string username, std::string password);
  static const std::string defaultServerIP;
  static const unsigned short defaultServerPort;

  private:
  std::string serverIP;
  unsigned short serverPort;
  //    std::string username;
  //    std::string password;
  //Socket socket;
  CHttpClient hc;
  bool connect();
  bool disconnect();
};

#endif // _GSTORECONNECTOR_H
