/*=============================================================================
# Filename: gbuild.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-24 19:27
# Description: firstly written by liyouhuan, modified by zengli
TODO: add -h/--help for help message
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

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
	if(_db_path.substr(len-3, 3) == ".db")
	{
		cout<<"your database can not end with .db"<<endl;
		return -1;
	}

	//if(_db_path[0] != '/' && _db_path[0] != '~')  //using relative path
	//{
	//_db_path = string("../") + _db_path;
	//}
	string _rdf = string(argv[2]);
	//if(_rdf[0] != '/' && _rdf[0] != '~')  //using relative path
	//{
	//_rdf = string("../") + _rdf;
	//}
	Database _db(_db_path);
	bool flag = _db.build(_rdf);
	if (flag)
	{
		cout << "import RDF file to database done." << endl;
	}
	else
	{
		cout << "import RDF file to database failed." << endl;
	}
	//system("clock");
	return 0;
}
