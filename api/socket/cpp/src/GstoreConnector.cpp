/*=============================================================================
# Filename: GstoreConnector.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2016-02-21 21:24
# Description: achieve functions in GstoreConnector.h
=============================================================================*/

#include "GstoreConnector.h"
#include <iostream>

using namespace std;

const string GstoreConnector::defaultServerIP = "127.0.0.1";
const unsigned short GstoreConnector::defaultServerPort = 3305;

GstoreConnector::GstoreConnector()
{
  this->serverIP = GstoreConnector::defaultServerIP;
}

GstoreConnector::GstoreConnector(string _ip)
{
  this->serverIP = _ip;
  this->serverPort = GstoreConnector::defaultServerPort;
}

GstoreConnector::GstoreConnector(unsigned short _port)
{
  this->serverIP = GstoreConnector::defaultServerIP;
  this->serverPort = _port;
}

GstoreConnector::GstoreConnector(string _ip, unsigned short _port)
{
  if (_ip == "localhost")
    this->serverIP = "127.0.0.1";
  else
    this->serverIP = _ip;
  this->serverPort = _port;
}

GstoreConnector::~GstoreConnector()
{
  this->disconnect();
}

bool GstoreConnector::test()
{
  bool connect_return = this->connect();
  if (!connect_return) {
    cerr << "failed to connect to server. @GstoreConnector::test" << endl;
    return false;
  }

  string cmd = "test";
  bool send_return = this->socket.send(cmd);
  if (!send_return) {
    cerr << "send test command error. @GstoreConnector::test" << endl;
    return false;
  }

  string recv_msg;
  this->socket.recv(recv_msg);

  this->disconnect();

  cout << recv_msg << endl;
  if (recv_msg == "OK") {
    return true;
  }
  return false;
}

bool
GstoreConnector::load(string _db_name)
{
  bool connect_return = this->connect();
  if (!connect_return) {
    cerr << "connect to server error. @GstoreConnector::load" << endl;
    return false;
  }

  string cmd = "load " + _db_name;
  bool send_return = this->socket.send(cmd);
  if (!send_return) {
    cerr << "send load command error. @GstoreConnector.load" << endl;
    return false;
  }

  string recv_msg;
  this->socket.recv(recv_msg);

  this->disconnect();

  cout << recv_msg << endl;
  if (recv_msg == "load database done.") {
    return true;
  }

  return false;
}

bool
GstoreConnector::unload(string _db_name)
{
  bool connect_return = this->connect();
  if (!connect_return) {
    cerr << "connect to server error. @GstoreConnector::unload" << endl;
    return false;
  }

  string cmd = "unload " + _db_name;
  bool send_return = this->socket.send(cmd);
  if (!send_return) {
    cerr << "send unload command error. @GstoreConnector::unload" << endl;
    return false;
  }

  string recv_msg;
  this->socket.recv(recv_msg);

  this->disconnect();

  cout << recv_msg << endl;
  if (recv_msg == "unload database done.") {
    return true;
  }

  return false;
}

bool
GstoreConnector::build(string _db_name, string _rdf_file_path)
{
  bool connect_return = this->connect();
  if (!connect_return) {
    cerr << "connect to server error. @GstoreConnector::build" << endl;
    return false;
  }

  string cmd = "import " + _db_name + " " + _rdf_file_path;
  bool send_return = this->socket.send(cmd);
  if (!send_return) {
    cerr << "send import command error. @GstoreConnector::build" << endl;
    return false;
  }

  string recv_msg;
  this->socket.recv(recv_msg);

  this->disconnect();

  cerr << recv_msg << endl;
  if (recv_msg == "import RDF file to database done.") {
    return true;
  }

  return false;
}

bool
GstoreConnector::drop(string _db_name)
{
  bool connect_return = this->connect();
  if (!connect_return) {
    cerr << "connect to server error. @GstoreConnector::unload" << endl;
    return false;
  }

  string cmd = "drop " + _db_name;
  bool send_return = this->socket.send(cmd);
  if (!send_return) {
    cerr << "send unload command error. @GstoreConnector::unload" << endl;
    return false;
  }

  string recv_msg;
  this->socket.recv(recv_msg);

  this->disconnect();

  cout << recv_msg << endl;
  if (recv_msg == "drop database done.") {
    return true;
  }
  return false;
}

string
GstoreConnector::query(string _sparql, string _output)
{
  bool connect_return = this->connect();
  if (!connect_return) {
    cerr << "connect to server error. @GstoreConnector::query" << endl;
    return "connect to server error.";
  }

  string cmd = "query " + _sparql;
  bool send_return = this->socket.send(cmd);
  if (!send_return) {
    cerr << "send query command error. @GstoreConnector::query";
    return "send query command error.";
  }

  string recv_msg;
  this->socket.recv(recv_msg);

  this->disconnect();

  return recv_msg;
}

string
GstoreConnector::show(bool _type)
{
  bool connect_return = this->connect();
  if (!connect_return) {
    cerr << "connect to server error. @GstoreConnector::show" << endl;
    return "connect to server error.";
  }

  string cmd;
  if (_type) {
    cmd = "show all";
  } else {
    cmd = "show databases";
  }
  bool send_return = this->socket.send(cmd);
  if (!send_return) {
    cerr << "send show command error. @GstoreConnector::show";
    return "send show command error.";
  }

  string recv_msg;
  this->socket.recv(recv_msg);
  this->disconnect();

  return recv_msg;
}

bool
GstoreConnector::connect()
{
  bool flag = this->socket.create();
  if (!flag) {
    cerr << "cannot create socket. @GstoreConnector::connect" << endl;
    return false;
  }

  flag = this->socket.connect(this->serverIP, this->serverPort);

  if (!flag) {
    cerr << "cannot connect to server. @GstoreConnector::connect" << endl;
    return false;
  }

  return true;
}

bool
GstoreConnector::disconnect()
{
  bool flag = this->socket.close();

  return flag;
}
