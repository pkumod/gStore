/**
  * @file		gadd.cpp
  * @author		WenjieLi
  * @email		liwenjiehn@pku.edu.cn
  * @version	v1.0
  * @date		2021-08-23
  * @license	Apache 2.0
  * @brief 		batch insert data to gstore
  * @detail		batch insert data to gstore by a file
  * @attention  
  */

#include "../Database/Database.h"
#include "../Util/Util.h"

using namespace std;

/*!
 * @brief		the main function
 * @param[in]	argc:the length of argc
 * @param[in]	argv:the param list\n
 *                   -db:the database name\n
 *                   -file:the insert data file\n
 *                   -help:print help info
 * @return 		the int value.
*/
int
main(int argc, char * argv[])
{
	//chdir(dirname(argv[0]));
//#ifdef DEBUG
	Util util;
//#endif
	cout << "argc:" << argc << ",argv[1]:" << argv[1] << endl;
	if (argc < 2)
	{
		cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "-help")
		{
			cout << endl;
			cout << "gStore Batch Insert Data Tools(gadd)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gadd -db [dbname] -file [filename] [option]" << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t[dbname],\t\t the database name. " << endl;
			cout << "\t[filename],\t\tthe file path for inserting data." << endl;
			cout << endl;
			return 0;
		}
		else
		{
			cout << "the command is not complete." << endl;
			return 0;
		}
	}
	else
	{
		string db_folder =Util::getArgValue(argc, argv, "db", "");
		if (db_folder.empty())
		{
			cout << "please input the database name " << endl;
			return 0;
		}
		int len = db_folder.length();

		if (db_folder.substr(len - 3, 3) == ".db")
		{
			cout << "your database can not end with .db" << endl;
			return -1;
		}
		string filename=Util::getArgValue(argc, argv, "filename", "");
		if (filename.empty())
		{
			cout << "please input the file path." << endl;
			return 0;
		}
		Database _db(db_folder);
		_db.load();
		cout << "finish loading" << endl;
		//_db.insert(argv[2]);
		//_db.remove(argv[2]);
		_db.insert(filename, false, nullptr);
		cout << "finish insert data" << endl;

	}
	

	return 0;
}
