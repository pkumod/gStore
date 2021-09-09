/**
* @file  Client.cpp
* @author  suxunbin
* @date  13-AUG-2021
* @brief  a gStore socket client
*/

#include "Client.h"

using namespace rapidjson;
using namespace std;

GstoreConnector::GstoreConnector()
{
	this->serverIP = defaultServerIP;
	this->serverPort = defaultServerPort;
}

GstoreConnector::GstoreConnector(string _ip)
{
	if (_ip == "localhost")
		this->serverIP = defaultServerIP;
	else
		this->serverIP = _ip;
	this->serverPort = defaultServerPort;
}

GstoreConnector::GstoreConnector(int _port)
{
	this->serverIP = defaultServerIP;
	this->serverPort = _port;
}

GstoreConnector::GstoreConnector(string _ip, int _port)
{
	if (_ip == "localhost")
		this->serverIP = defaultServerIP;
	else
		this->serverIP = _ip;
	this->serverPort = _port;
}

GstoreConnector::~GstoreConnector()
{

}

bool
GstoreConnector::connect()
{
	bool flag = this->socket.create();
	if (!flag)
	{
		cerr << "Cannot create socket. @GstoreConnector::connect" << endl;
		return false;
	}

	flag = this->socket.connect(this->serverIP, this->serverPort);

	if (!flag)
	{
		cerr << "Cannot connect to server. @GstoreConnector::connect" << endl;
		return false;
	}

	cout << "Connect to server successfully!" << endl;
	return true;
}

bool GstoreConnector::send(std::string _cmd) {
	bool send_return = this->socket.send(_cmd);
	if (!send_return) {
		cerr << "Send command error. @GstoreConnector::send" << endl;
		return false;
	}
	return true;
}

bool GstoreConnector::recv(std::string& _recv_msg) {
	while (true)
	{
		this->socket.recv(_recv_msg);
		if (parseJson(_recv_msg) != 1)
		{
			cout << _recv_msg << endl;
			break;
		}
		cout << _recv_msg << endl;
	}
	return true;
}

bool
GstoreConnector::disconnect()
{
	std::string _cmd = "{\"op\":\"close\"}";
	bool send_return = this->socket.send(_cmd);
	if (!send_return) {
		cerr << "Disconnect command error. @GstoreConnector::disconnect" << endl;
		return false;
	}

	std::string _recv_msg;
	this->socket.recv(_recv_msg);
	cout << _recv_msg << endl;

	bool flag = this->socket.close();
	return flag;
}

int
GstoreConnector::parseJson(std::string strJson)
{
	Document document;
	document.Parse(strJson.c_str());
	Value& p = document["StatusCode"];
	int StatusCode = p.GetInt();
	return StatusCode;
}

