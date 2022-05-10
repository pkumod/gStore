/*
 * @Author: liwenjie
 * @Date: 2021-08-22 18:00:17
 * @LastEditTime: 2021-08-23 13:12:31
 * @LastEditors: Please set LastEditors
 * @Description: Batch delete data from database
 * @FilePath: /gstore/Main/gsub.cpp
 */


#include "../Database/Database.h"
#include "../Util/Util.h"

//NOTICE+WARN:
//ok to remove the whole tree
//but if add again, error will come
//TODO:deal with empty tree and add eles

using namespace std;

int
main(int argc, char * argv[])
{
	//chdir(dirname(argv[0]));
//#ifdef DEBUG
	Util util;
//#endif

	if (argc < 2)
	{

		cout << "Invalid arguments! Input \"bin/gsub -h\" for help." << endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Batch Delete Data Tools(gadd)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gsub -db [dbname] -f [filename] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. " << endl;
			cout << "\t-f,--file,\t\tthe file path to delete data." << endl;
			cout << endl;
			return 0;
		}
		else
		{
			//cout << "the command is not complete." << endl;
			//Log.Error("Invalid arguments! Input \"bin/gadd -h\" for help.");
			cout << "Invalid arguments! Input \"bin/gsub -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		string db_folder = Util::getArgValue(argc, argv, "db", "database");
		if (db_folder.empty())
		{
			/*		cout << "please input the database name " << endl;*/
			//Log.Error("the database name is empty! Input \"bin/gbackup -h\" for help.");
			cout << "the database name is empty! Input \"bin/gsub -h\" for help." << endl;
			return 0;
		}
		int len = db_folder.length();

		if (db_folder.substr(len - 3, 3) == ".db")
		{
			/*cout << "your database can not end with .db" << endl;*/
			//Log.Error("your database can not end with .db.! Input \"bin/gadd -h\" for help.");
			cout << "your database can not end with .db.! Input \"bin/gsub -h\" for help." << endl;
			return -1;
		}
		string filename = Util::getArgValue(argc, argv, "f", "file");
		if (filename.empty())
		{
			//cout << "please input the file path." << endl;
			//Util::formatPrint("please input the file path.", "Error");
			//Log.Error("the add data file is empty! Input \"bin/gadd -h\" for help.");
			cout << "the delete data file is empty! Input \"bin/gsub -h\" for help." << endl;
			return 0;
		}
		if (Util::file_exist(filename) == false)
		{
			cout << "the delete data file is not exist! Input \"bin/gsub -h\" for help." << endl;
			return 0;
		}
		Database _db(db_folder);
		_db.load();
		cout << "finish loading" << endl;
		//_db.insert(argv[2]);
		//_db.remove(argv[2]);
		long tv_begin = Util::get_cur_time();
		_db.batch_remove(filename, false, nullptr);
		long tv_end = Util::get_cur_time();
		cout << "after remove, used " << (tv_end - tv_begin) << " ms" << endl;
		_db.save();
	}

	return 0;
}
