/*=============================================================================
# Filename: gbuild.cpp
# Author: Bookug Lobert suxunbin
# Mail: 1181955272@qq.com suxunbin@pku.edu.cn
# Last Modified: 2018-10-19 20:30
# Description: firstly written by liyouhuan, modified by zengli and suxunbin
TODO: add -h/--help for help message
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"
#include "../api/http/cpp/client.h"

using namespace std;
using namespace rapidjson;
#define SYSTEM_PATH "data/system/system.nt"

//[0]./gbuild [1]data_folder_path  [2]rdf_file_path
int
main(int argc, char * argv[])
{
	//chdir(dirname(argv[0]));
//#ifdef DEBUG
	Util util;
//#endif
	if(argc < 3)  //./gbuild
	{
		//output help info here
		cout << "the usage of gbuild: " << endl;
		cout << "bin/gbuild your_database_name.db your_dataset_path" << endl;
		return 0;
	}
	//system("clock");
	cout << "gbuild..." << endl;
	{
		cout << "argc: " << argc << "\t";
		cout << "DB_store:" << argv[1] << "\t";
		cout << "RDF_data: " << argv[2] << "\t";
		cout << endl;
	}

	string _db_path = string(argv[1]);
	int len = _db_path.length();
	if(_db_path.length() > 3 && _db_path.substr(len-3, 3) == ".db")
	{
		cout<<"your database can not end with .db or less than 3 characters"<<endl;
		return -1;
	}

	//check if the db_name is system
	if (_db_path == "system")
	{
		cout<< "Your database's name can not be system."<<endl;
		return -1;
	}

	//if(_db_path[0] != '/' && _db_path[0] != '~')  //using relative path
	//{
	//_db_path = string("../") + _db_path;
	//}
	string _rdf = string(argv[2]);

	//check if the db_path is the path of system.nt
	if (_rdf == SYSTEM_PATH)
	{
		cout<< "You have no rights to access system files"<<endl;
		return -1;
	}

	//if(_rdf[0] != '/' && _rdf[0] != '~')  //using relative path
	//{
	//_rdf = string("../") + _rdf;
	//}

	//check if the database is already built
	int isbuilt;
	if (boost::filesystem::exists(_db_path + ".db"))
		isbuilt = 1;
	else
		isbuilt = 0;

	
	if (!boost::filesystem::exists("system.db")){
		cout << "system.db not found! please run bin/ginit" << endl;
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
	string res = gc.build(_db_path, _rdf);
	Document document;
	document.Parse(res.c_str());
	cout << "StatusCode: " << document["StatusCode"].GetInt() << endl;
	cout << "StatusMsg: " << document["StatusMsg"].GetString() << endl;
	return 0;
}

