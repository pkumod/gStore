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
#include "../Util/Slog.h"
//#include <stdio.h>
//#include <log4cplus/logger.h>
//#include <log4cplus/loggingmacros.h>
//#include <log4cplus/consoleappender.h>
//#include <log4cplus/config.hxx>
//#include <log4cplus/logger.h>
//#include <log4cplus/configurator.h>
//#include <log4cplus/helpers/loglog.h>
//#include <log4cplus/helpers/stringhelper.h>
//#include <log4cplus/helpers/socket.h>
//#include <log4cplus/spi/loggerimpl.h>
//#include <log4cplus/spi/loggingevent.h>



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
//*! [important] you should init the Util class.
Util util;
//#endif
Log.init("slog.properties");

	if (argc < 2)
	{
		cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;
		Log.Error("Invalid arguments! Input \"bin/gadd -h\" for help.");
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Batch Insert Data Tools(gadd)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gadd -db [dbname] -f [filename] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. " << endl;
			cout << "\t-f,--file,\t\tthe file path for inserting data." << endl;
			cout << endl;
			return 0;
		}
		else
		{
			//cout << "the command is not complete." << endl;
			Util::formatPrint("the command is not complete.", "Error");
			return 0;
		}
	}
	else
	{
		string db_folder =Util::getArgValue(argc, argv, "db","database");
		if (db_folder.empty())
		{
	/*		cout << "please input the database name " << endl;*/
			Util::formatPrint("please input the database name.", "Error");
			return 0;
		}
		int len = db_folder.length();

		if (db_folder.substr(len - 3, 3) == ".db")
		{
			/*cout << "your database can not end with .db" << endl;*/
			Util::formatPrint("your database can not end with .db.", "Error");
			return -1;
		}
		string filename=Util::getArgValue(argc, argv, "f","file");
		if (filename.empty())
		{
			//cout << "please input the file path." << endl;
			Util::formatPrint("please input the file path.", "Error");
			return 0;
		}
	
		Database _db(db_folder);
		_db.load();
		//cout << "finish loading" << endl;
	
		Log.Info("finish loading.");
		//_db.insert(argv[2]);
		//_db.remove(argv[2]);
		_db.insert(filename, false, nullptr);
		//cout << "finish insert data" << endl;
		
		Log.Info("please input the complete command:");


	Database *_db = new Database(db_folder);
	_db->load();
	cout << "finish loading" << endl;
	//_db.insert(argv[2]);
	//_db.remove(argv[2]);
	long tv_begin = Util::get_cur_time();
	_db->insert(argv[2], false, nullptr);
	long tv_end = Util::get_cur_time();
	cout << "after insert, used " << (tv_end - tv_begin) << " ms" << endl;
	//string query = string(argv[2]);
	//query = Util::getQueryFromFile(query.c_str());
	//if (query.empty())
	//{
	//return 0;
	//}
	//printf("query is:\n%s\n\n", query.c_str());

	//ResultSet _rs;
	//_db.query(query, _rs, stdout);

	//TODO:to test insert, delete and modify
	//read from file or just several triples written here
	delete _db;
	long tv_end1 = Util::get_cur_time();
	cout << "persistence on disk" << (tv_end1 - tv_end) << " ms" << endl
	}
	

	return 0;
}
