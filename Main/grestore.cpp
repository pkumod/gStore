/*
 * @Author: zhe zhang, liwenjie
 * @Date: 2021-08-22 23:31:50
 * @LastEditTime: 2021-08-22 23:32:04
 * @LastEditors: liwenjie
 * @Description: restore a database by copy its backup files to root location
 * @FilePath: /gstore/Main/grestore.cpp
 */

#include "../Util/Util.h"
#include "../Database/Database.h"
//#include "../Util/Slog.h"

using namespace std;
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

	if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		//Log.Error("Invalid arguments! Input \"bin/gadd -h\" for help.");
		cout << "Invalid arguments! Input \"bin/grestore -h\" for help." << endl;
		return 0;
	}

	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Restore Database Tools(grestore)" << endl;
			cout << endl;
			cout << "Usage:\tbin/grestore -db [dbname] -p [path] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. " << endl;
			cout << "\t-p,--path,\t\tthe backup path, notice that it is relative path for the gstore root path." << endl;
			cout << endl;
			return 0;
		}
		else
		{
			//cout << "the command is not complete." << endl;
			//Log.Error("Invalid arguments! Input \"bin/gadd -h\" for help.");
			cout << "Invalid arguments! Input \"bin/gadd -h\" for help." << endl;
			return 0;
		}
	}
	else
	{

		db_name = Util::getArgValue(argc, argv, "db", "database");
		if (db_name.empty())
		{
			/*		cout << "please input the database name " << endl;*/
			//Log.Error("the database name is empty! Input \"bin/gbackup -h\" for help.");
			cout << "the database name is empty! Input \"bin/grestore -h\" for help." << endl;
			return 0;
		}
		int len = db_name.length();

		if (db_name.substr(len - 3, 3) == ".db")
		{
			/*cout << "your database can not end with .db" << endl;*/
			//Log.Error("your database can not end with .db.! Input \"bin/gadd -h\" for help.");
			cout << "your database can not end with .db.! Input \"bin/grestore -h\" for help." << endl;
			return -1;
		}
		backup_path = Util::getArgValue(argc, argv, "p", "path");
		if (backup_path.empty())
		{
			//cout << "please input the file path." << endl;
			//Util::formatPrint("please input the file path.", "Error");
			//Log.Error("the add data file is empty! Input \"bin/gadd -h\" for help.");
			cout << "the backup path is empty! Input \"bin/grestore -h\" for help." << endl;
			return 0;
		}

		if (backup_path[0] == '/')
			backup_path = '.' + backup_path;
		if (backup_path[backup_path.length() - 1] == '/')
			backup_path = backup_path.substr(0, backup_path.length() - 1);

		if (!Util::dir_exist(backup_path))
		{
			cout << "Backup Path Error, Restore Failed" << endl;
			return 0;
		}
		//system.db
		Database system_db("system");
		system_db.load();

		string sparql = "ASK WHERE{<" + db_name + "> <database_status> \"already_built\".}";
		ResultSet ask_rs;
		FILE *ask_ofp = stdout;
		system_db.query(sparql, ask_rs, ask_ofp);
		if (ask_rs.answer[0][0] == "false")
		{
			cout << "The database does not exist. Rebuild" << endl;
			string time = Util::get_backup_time(backup_path, db_name);
			if (time.size() == 0)
			{
				cout << "Backup Path Does not Match DataBase Name, Restore Failed" << endl;
				return 0;
			}
			string sparql = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." + "<" + db_name + "> <built_by> <root>." + "<" + db_name + "> <built_time> \"" + time + "\".}";
			ResultSet _rs;
			FILE *ofp = stdout;
			string msg;
			int ret = system_db.query(sparql, _rs, ofp);

			if (ret >= 0)
				msg = "update num : " + Util::int2string(ret);
			else
			{
				//update error
				cout << "Rebuild Error, Restore Failed" << endl;
				return 0;
			}
			cout << msg << endl;
			Util::add_backuplog(db_name);
		}

		int ret = copy(backup_path, DEFALUT_BUILD_PATH);

		if (ret == 1)
		{
			cout << "Backup Path Error, Restore Failed!" << endl;
		}
		else
		{
			//TODO update the in system.db
			string time = Util::get_date_time();
			cout << "Time:" + time << endl;
			cout << "DB:" + db_name + " Restore done!" << endl;
		}

		db_path = db_name + ".db";
		string sys_cmd = "rm -rf " + db_path;
		system(sys_cmd.c_str());

		path = Util::get_folder_name(backup_path, db_name);
		sys_cmd = "mv " + path + ' ' + db_path;
		system(sys_cmd.c_str());

		return 0;
	}

	// if(argc < 2)  
	// {
	// 	//output help info here
	// 	cout << "the usage of grestore: " << endl;
	// 	cout << "./bin/restore your_database_name your_backup_path" << endl;
	// 	cout << "the path should include your database folder name!" << endl;
	// 	return 0;
	// }

	// db_name = string(argv[1]);
	// if(argc > 2)
	// 	backup_path = string(argv[2]);

	// cout << "gbackup..." << endl;
	// {
	// 	cout << "argc: " << argc << "\t";
	// 	cout << "DB_name:" << db_name << "\t";
	// 	cout << "Backup_path: " << backup_path << "\t";
	// 	cout << endl;
	// }

	
	// int len = db_name.length();
	// if(db_name.length() > 3 && db_name.substr(len-3, 3) == ".db")
	// {
	// 	cout<<"your database name can not end with .db"<<endl;
	// 	return -1;
	// }

	// if (db_name == "system")
	// {
	// 	cout<< "Your database's name can not be system."<<endl;
	// 	return -1;
	// }	

	// if(backup_path[0] == '/') backup_path = '.' + backup_path;
	// if(backup_path[backup_path.length() - 1] == '/') backup_path = backup_path.substr(0, backup_path.length()-1);

	// if(!boost::filesystem::exists(backup_path)){
	// 	cout << "Backup Path Error, Restore Failed"  << endl;
	// 	return 0;
	// }

	// //system.db
	// Database system_db("system");
	// system_db.load();

	// string sparql = "ASK WHERE{<" + db_name + "> <database_status> \"already_built\".}";
	// ResultSet ask_rs;
	// FILE* ask_ofp = stdout;
	// system_db.query(sparql, ask_rs, ask_ofp);
	// if (ask_rs.answer[0][0] == "false")
	// {
	// 	cout << "The database does not exist. Rebuild" << endl;
	// 	string time = Util::get_backup_time(backup_path, db_name);
	// 	if(time.size() == 0){
	// 		cout << "Backup Path Does not Match DataBase Name, Restore Failed" << endl;
	// 		return 0;
	// 	}
	// 	string sparql = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." + "<" + db_name + "> <built_by> <root>."
	// 		+ "<" + db_name + "> <built_time> \"" + time + "\".}";
	// 	ResultSet _rs;
	// 	FILE* ofp = stdout;
	// 	string msg;
	// 	int ret = system_db.query(sparql, _rs, ofp);

	// 	if (ret >= 0)
	// 		msg = "update num : " + Util::int2string(ret);
	// 	else {
	// 		//update error
	// 		cout << "Rebuild Error, Restore Failed" << endl;
	// 		return 0;
	// 	}
	// 	cout << msg << endl;
	// 	Util::add_backuplog(db_name);
	// }
	
	// int ret = copy(backup_path, DEFALUT_BUILD_PATH);

	// if(ret == 1){
	// 	cout << "Backup Path Error, Restore Failed!" << endl;
	// }else{
	// 	//TODO update the in system.db
	// 	string time = Util::get_date_time();
	// 	cout << "Time:" + time << endl;
	// 	cout << "DB:" + db_name + " Restore done!" << endl;
	// }

	// db_path = db_name + ".db";
	// string sys_cmd = "rm -rf " + db_path;
	// system(sys_cmd.c_str());

	// path = Util::get_folder_name(backup_path, db_name);
	// sys_cmd = "mv " + path + ' ' + db_path;
	// system(sys_cmd.c_str());

	// return 0;
}