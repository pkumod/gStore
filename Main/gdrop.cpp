/*=============================================================================
# Filename: gdrop.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2018-10-30 16:21
# Description: used to drop database
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"
#include "../api/http/cpp/client.h"

using namespace std;
using namespace rapidjson;

int main(int argc, char * argv[])
{
	Util util;
	string db_name;
	if (argc == 1)
	{
		cout << "You need to input the database name that you want to drop." << endl;
		return -1;
	}
	else if (argc == 2)
	{
		db_name = argv[1];
		int len = db_name.length();
		if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db")
		{
			cout << "The database name can not end with .db" << endl;
			return -1;
		}
	}
	else
	{
		cout << "The number of parameters is not correct." << endl;
		return -1;
	}
	if (!boost::filesystem::exists(db_name + ".db"))
	{
		cout << "The database that you want to drop does not exist." << endl;
		return -1;
	};

	fstream ofp;
	ofp.open("./system.db/port.txt", ios::in);
	int ch = ofp.get();
	if(ofp.eof()){
		cout << "ghttp is not running!" << endl;
		return 0;
	}
	ofp.close();
	ofp.open("./system.db/port.txt", ios::in);
	int port;
	ofp >> port;
	ofp.close();
	string username = "root";
	string password = "123456";
	string IP = "127.0.0.1";
	GstoreConnector gc(IP, port, username, password);
	string res = gc.drop(db_name, false);
	Document document;
	document.Parse(res.c_str());
	cout << "StatusCode: " << document["StatusCode"].GetInt() << endl;
	cout << "StatusMsg: " << document["StatusMsg"].GetString() << endl;
	return 0;
}
