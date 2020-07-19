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
  //this->disconnect();
}
/*
bool GstoreConnector::test() {
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
*/
bool
GstoreConnector::load(string _db_name, string username, string password)
{
  string url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort);

  string cmd = url + "/?operation=load&db_name=" + _db_name + "&username=" + username + "&password=" + password;
  string recv_msg;
  int ret;
  ret = hc.Get(cmd, recv_msg);
  cout << recv_msg << endl;
  if (recv_msg == "load database done.") {
    return true;
  }

  return false;
}

bool
GstoreConnector::unload(string _db_name, string username, string password)
{
  string url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort);

  string cmd = url + "/?operation=unload&db_name=" + _db_name + "&username=" + username + "&password=" + password;
  string recv_msg;
  int ret = hc.Get(cmd, recv_msg);
  cout << recv_msg << endl;
  if (recv_msg == "unload database done.") {
    return true;
  }

  return false;
}

bool
GstoreConnector::build(string _db_name, string _rdf_file_path, string username, string password)
{
  string url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort);

  string cmd = url + "/?operation=build&db_name=" + _db_name + "&ds_path=" + _rdf_file_path + "&username=" + username + "&password=" + password;
  string recv_msg;
  int ret = hc.Get(cmd, recv_msg);
  cerr << recv_msg << endl;
  if (recv_msg == "import RDF file to database done.") {
    return true;
  }

  return false;
}
/*
bool
GstoreConnector::drop(string _db_name)
{
	bool connect_return = this->connect();
	if (!connect_return)
	{
		cerr << "connect to server error. @GstoreConnector::unload" << endl;
		return false;
	}

	string cmd = "drop " + _db_name;
	bool send_return = this->socket.send(cmd);
	if (!send_return)
	{
		cerr << "send unload command error. @GstoreConnector::unload" << endl;
		return false;
	}

	string recv_msg;
	this->socket.recv(recv_msg);

	this->disconnect();

	cout << recv_msg << endl;
	if (recv_msg == "drop database done.")
	{
		return true;
	}
	return false;
}
*/
string
GstoreConnector::query(string username, string password, string db_name, string sparql)
{
  string url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort);

  string cmd = url + "/?operation=query&username=" + username + "&password=" + password + "&db_name=" + db_name + "&format=json&sparql=" + sparql;
  string recv_msg;
  int ret = hc.Get(cmd, recv_msg);
  return recv_msg;
}

void GstoreConnector::query(string username, string password, string db_name, string sparql, string filename)
{
  string url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort);
  string cmd = url + "/?operation=query&username=" + username + "&password=" + password + "&db_name=" + db_name + "&format=json&sparql=" + sparql;
  int ret = hc.Get(cmd, filename, true);
  return;
}

string
GstoreConnector::show(string username, string password)
{
  string url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort);

  string cmd = url + "/?operation=show&username=" + username + "&password=" + password;
  string recv_msg;
  int ret = hc.Get(cmd, recv_msg);
  return recv_msg;
}
/*
bool
GstoreConnector::connect()
{
	bool flag = this->socket.create();
	if (!flag)
	{
		cerr << "cannot create socket. @GstoreConnector::connect" << endl;
		return false;
	}

	flag = this->socket.connect(this->serverIP, this->serverPort);

	if (!flag)
	{
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
*/
string
GstoreConnector::user(string type, string username1, string password1, string username2, string addition)
{
  string url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort);

  string cmd = url + "/?operation=user&type=" + type + "&username1=" + username1 + "&password1=" + password1 + "&username2=" + username2 + "&addition=" + addition;
  string recv_msg;
  int ret = hc.Get(cmd, recv_msg);
  return recv_msg;
}
string
GstoreConnector::showUser()
{
  string url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort);

  string cmd = url + "/?operation=showUser";
  string recv_msg;
  int ret = hc.Get(cmd, recv_msg);
  return recv_msg;
}
string
GstoreConnector::monitor(string db_name, string username, string password)
{
  string url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort);

  string cmd = url + "/?operation=monitor&db_name=" + db_name + "&username=" + username + "&password=" + password;
  string recv_msg;
  int ret = hc.Get(cmd, recv_msg);
  return recv_msg;
}
string
GstoreConnector::checkpoint(string db_name, string username, string password)
{
  string url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort);

  string cmd = url + "/?operation=checkpoint&db_name=" + db_name + "&username=" + username + "&password=" + password;
  string recv_msg;
  int ret = hc.Get(cmd, recv_msg);
  return recv_msg;
}
