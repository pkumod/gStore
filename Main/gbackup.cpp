/*
 * @Author: zhe zhang,liwenjie
 * @Date: 2021-08-20 10:29:41
 * @LastEditTime: 2022-09-23 14:14:26
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: backup a database by copy its source files to a backup location
 * @FilePath: /gstore/Main/gbackup.cpp
 */

// backup
// TODO:database_path is designed to support the customized database path
// the defalut database_path is ./database_name.db

#include "../Util/Util.h"
#include "../Database/Database.h"
#include <sstream>

using namespace std;

int copy(string src_path, string dest_path)
{

	string sys_cmd;
	if (!Util::file_exist(src_path))
	{
		// check the source path
		cout << "Source Path Error, Please Check It Again!" << endl;
		return 1;
	}
	if (!Util::file_exist(dest_path))
	{
		// check the destnation path
		sys_cmd = "mkdir -p ./" + dest_path;
		system(sys_cmd.c_str());
	}

	sys_cmd = "cp -r " + src_path + " ./" + dest_path;
	system(sys_cmd.c_str());
	return 0; // success
}

int main(int argc, char *argv[])
{
	Util util;
	// Log.init("slog.properties");
	string db_name, backup_path;
	if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		// Log.Error("Invalid arguments! Input \"bin/gbackup -h\" for help.");
		cout << "Invalid arguments! Input \"bin/gbackup -h\" for help." << endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Backup Tools(gbackup)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gbackup -db [dbname] -p [backup_path] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. " << endl;
			cout << "\t-p,--path [optional],\t\tthe backup path,defalut backup_path = ./backups,the path should not include your database's name!" << endl;
			cout << endl;
			return 0;
		}
		else
		{
			// cout << "the command is not complete." << endl;
			// Log.Error("Invalid arguments! Input \"bin/gbackup -h\" for help.");
			cout << "Invalid arguments! Input \"bin/gbackup -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		util.configure_new();
		string default_backup_path = util.getConfigureValue("backup_path");
		db_name = Util::getArgValue(argc, argv, "db", "database");
		backup_path = Util::getArgValue(argc, argv, "p", "path");
		if (backup_path.empty())
		{
			backup_path = default_backup_path;
		}
		int len = db_name.length();
		if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db")
		{

			// Log.Error("your database name can not end with .db! Input \"bin/gbackup -h\" for help.");
			cout << "your database name can not end with .db! Input \"bin/gbackup -h\" for help." << endl;
			return -1;
		}
		if (db_name == "system")
		{
			cout << "Your database's name can not be system." << endl;
			/*Log.Error("Your database's name can not be system!");*/
			return -1;
		}
		if (backup_path == "." || backup_path == "./")
		{
			cout << "Backup Path Can not be root, Backup Failed!" << endl;
			// Log.Error("Backup Path Can not be root, Backup Failed!");
			return 0;
		}
		// TODO: We need two column in system.db :
		// build_path and backup_path;
		// we can get the build_path as database_path should write the backup_path when we finish the backup
		// query database_name build_path
		// insert database_name backup_path
		if (backup_path[0] == '/')
			backup_path = '.' + backup_path;
		if (backup_path[backup_path.length() - 1] == '/')
			backup_path = backup_path.substr(0, backup_path.length() - 1);
		backup_path = "./" + backup_path;
		string db_path = db_name + ".db";
		if (backup_path == "")
			backup_path = default_backup_path;
		long tv_begin = Util::get_cur_time();

		int ret = copy(db_path, backup_path);
		if (ret == 1)
		{
			cout << "Database Name Error, Backup Failed!" << endl;
			// Log.Error("Database Name Error, Backup Failed!");
		}
		else
		{
			string time = Util::get_date_time();
			string timestamp = Util::get_timestamp();
			backup_path = backup_path + "/" + db_path;
			string _backup_path = backup_path + "_" + timestamp;
			string sys_cmd = "mv " + backup_path + " " + _backup_path;
			system(sys_cmd.c_str());
			long tv_end = Util::get_cur_time();

			cout << "DB:" << db_name << " Backup Successfully! Used " << (tv_end - tv_begin) << " ms" << endl;
			/*stringstream ss;
			ss << "DB:"<<db_name<<" Backup Successfully! Used " << (tv_end - tv_begin) << " ms";
			Log.Info(ss.str().c_str());*/
			/*cout << "Time:" + time << endl;
			cout << "DB:" + db_name + " Backup done!" << endl;*/
		}
		return 0;
	}
}
