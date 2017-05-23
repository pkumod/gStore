/*=============================================================================
# Filename:		gpath.cpp
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified:	2017-05-22 23:27
# Description: 
=============================================================================*/

//BETTER: run queries in batch, so as to reduce duplicate computing
//

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
		cout << "the usage of gpath: " << endl;
		cout << "bin/gpath -s dataset_name" << endl;
		cout << "or use an existing database:"<<endl;
		cout << "bin/gpath -d database_name" << endl;
		cout <<"after the databse is built/loaded, you can input queries in the console"
		return 0;
	}
	//system("clock");

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

	//TODO: query console - input or a file

	//TODO: property path handling

	//system("clock");
	return 0;
}

