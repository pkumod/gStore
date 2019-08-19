/*=============================================================================
# Filename: gexport.cpp
# Author: suxunbin
# Last Modified: 2019-07-23 15:37
# Description: export a database to get .nt file
=============================================================================*/

#include "../Database/Database.h"
#include "../Util/Util.h"
#include "../api/http/cpp/client.h"

using namespace std;
using namespace rapidjson;

int
main(int argc, char * argv[])
{
	Util util;

	string db_name;
	string filepath;
	if (argc == 1)
	{
		cout << "You need to input the database name that you want to export." << endl;
		return 0;
	}
	else if (argc == 2)
	{
		db_name = argv[1];
		int len = db_name.length();
		if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db")
		{
			cout << "The database name can not end with .db" << endl;
			return 0;
		}
		filepath = db_name + ".nt";
	}
	else if (argc == 3)
	{
		db_name = argv[1];
		int len = db_name.length();
		if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db")
		{
			cout << "The database name can not end with .db" << endl;
			return 0;
		}
		filepath = argv[2];
		if(filepath[filepath.length()-1] != '/')
			filepath = filepath + "/";
		if(!boost::filesystem::exists(filepath))
				boost::filesystem::create_directories(filepath);
		filepath = filepath + db_name + ".nt";
	}

	cout << "gexport..." << endl;

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
	string res = gc.exportDB(db_name, filepath);
	Document document;
	document.Parse(res.c_str());
	cout << "StatusCode: " << document["StatusCode"].GetInt() << endl;
	cout << "StatusMsg: " << document["StatusMsg"].GetString() << endl;

	return 0;
}
