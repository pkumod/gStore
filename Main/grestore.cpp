/*=============================================================================
# Filename: grestore.cpp
# Author: zhe zhang
# Mail: zhezhang99@gmail.com
# Last Modified: 2019.7.22
# Description: restore a database by copy its backup files to root location
=============================================================================*/




#include "../Util/Util.h"
#include "../Database/Database.h"
#include "../api/http/cpp/client.h"

using namespace std;
using namespace rapidjson;
#define DEFALUT_BUILD_PATH "."
#define DEFALUT_BACKUP_PATH "./backups"

int copy(string src_path, string dest_path)
{
	if(!boost::filesystem::exists(src_path)){
		//check the source path
		cout << "Source Path Error, Please Check It Again!" << endl;
		return 1;
	}
	if(!boost::filesystem::exists(dest_path)){
		//check the destnation path 
		cout << "Destnation Path Error, Please Check It Again!" << endl;
		return -1;
	}
	string sys_cmd;
	sys_cmd = "cp -r " + src_path + ' ' +  dest_path ;
	system(sys_cmd.c_str());
    return 0;// success 
}

int
main(int argc, char * argv[])
{
	Util util;
	string db_name, backup_path, db_path, path;

	if(argc < 2)  
	{
		//output help info here
		cout << "the usage of grestore: " << endl;
		cout << "./bin/restore your_database_name your_backup_path" << endl;
		cout << "the path should include your database folder name!" << endl;
		return 0;
	}

	db_name = string(argv[1]);
	if(argc > 2)
		backup_path = string(argv[2]);

	cout << "gbackup..." << endl;
	{
		cout << "argc: " << argc << "\t";
		cout << "DB_name:" << db_name << "\t";
		cout << "Backup_path: " << backup_path << "\t";
		cout << endl;
	}

	
	int len = db_name.length();
	if(db_name.length() > 3 && db_name.substr(len-3, 3) == ".db")
	{
		cout<<"your database name can not end with .db"<<endl;
		return -1;
	}

	if (db_name == "system")
	{
		cout<< "Your database's name can not be system."<<endl;
		return -1;
	}	

	if(backup_path[0] == '/') backup_path = '.' + backup_path;
	if(backup_path[backup_path.length() - 1] == '/') backup_path = backup_path.substr(0, backup_path.length()-1);

	if(!boost::filesystem::exists(backup_path)){
		cout << "Backup Path Error, Restore Failed"  << endl;
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
	GstoreConnector gc;
	string res;
	string username = "root";
	string password = "123456";
	int ret;
	ret = gc.Get("http://127.0.0.1:" + Util::int2string(port) + "/?operation=restore&db_name=" + db_name + "&username=" + username + "&password=" + password + "&path=" + backup_path, res);
	Document document;
	document.Parse(res.c_str());
	cout << "StatusCode: " << document["StatusCode"].GetInt() << endl;
	cout << "StatusMsg: " << document["StatusMsg"].GetString() << endl;
	return 0;
}