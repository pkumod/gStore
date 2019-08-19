/*=============================================================================
# Filename: gmonitor.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2019-07-26 16:00
# Description: used to show information of the database
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
			cout << "You need to input the database name that you want to show." << endl;
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
		}
		else
		{
			cout << "The number of parameters is not correct." << endl;
			return 0;
		}

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
        string res = gc.load(db_name);
        res = gc.monitor(db_name);
        Document document;
		document.Parse(res.c_str());
		cout << "StatusCode: " << document["StatusCode"].GetInt() << endl;
		cout << "StatusMsg: " << document["StatusMsg"].GetString() << endl;
		cout << "=============================================================" << endl;
		cout << "database: " << document["database"].GetString() << endl;
		cout << "creator: " << document["creator"].GetString() << endl;
		cout << "built_time: " << document["built_time"].GetString() << endl;
		cout << "triple num: " << document["triple num"].GetInt() << endl;
		cout << "entity num: " << document["entity num"].GetInt() << endl;
		cout << "literal num: " << document["literal num"].GetInt() << endl;
		cout << "subject num: " << document["subject num"].GetInt() << endl;
		cout << "predicate num: " << document["predicate num"].GetInt() << endl;
		cout << "connection num: " << document["connection num"].GetInt() << endl;
		cout << "=============================================================" << endl;
        return 0;
}