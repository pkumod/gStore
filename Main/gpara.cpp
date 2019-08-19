/*=============================================================================
# Filename: gpara.cpp
# Author: zhezhang
# Mail: zhezhang99@gmail.com
# Last Modified: 2019-08-06 10:17
# Description: written by zhezhang, modify parameters of every database.
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"
#include "../api/http/cpp/client.h"

using namespace std;
using namespace rapidjson;

int main(int argc, char * argv[]){
	Util util;

	if(argc < 4)  
	{
		cout << "the usage of gparameter: " << endl;
		cout << "bin/gparameter your_database_name -parameter value " << endl << endl << endl;
		cout << "parameter supported and value example: " << endl;
		cout << "is_backup: true/false" << endl;
		cout << "backup_interval: 6(hours)" << endl;
		cout << "backup_timer: 6(hours) PS: This parameter is only owned by system, so your_database_name must be system!" << endl;
		return 0;
	}

	cout << "gparameter..." << endl;
	{
		cout << "argc: " << argc << "\t";
		cout << "DB_name:" << argv[1] << "\t";
		cout << "parameter: " << argv[2] << "\t";
		cout << "value: " << argv[3] << "\t";
		cout << endl;
	}

	string db_name = argv[1];
	string parameter = argv[2];
	string value = argv[3];

	int isbuilt;
	if (boost::filesystem::exists(db_name + ".db"))
		isbuilt = 1;
	else
		isbuilt = 0;

	if(!isbuilt){
		cout << "database " + db_name + "is not built yet! please check your database name" << endl;
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
	GstoreConnector gc;
	string res;
	string username = "root";
	string password = "123456";
	int ret;
	ret = gc.Get("http://127.0.0.1:" + Util::int2string(port) + "/?operation=parameter&db_name=" + db_name + "&type=" + parameter + "&username=" + username + "&password=" + password + "&value=" + value, res);
	Document document;
	document.Parse(res.c_str());
	cout << "StatusCode: " << document["StatusCode"].GetInt() << endl;
	cout << "StatusMsg: " << document["StatusMsg"].GetString() << endl;
	return 0;
}