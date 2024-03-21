/*
 * @Author: zhe zhang,liwenjie
 * @Date: 2021-08-20 10:29:41
 * @LastEditTime: 2022-11-23 10:14:05
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

int main(int argc, char *argv[])
{
	Util util;
	string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
	string default_backup_path = util.backup_path;
	size_t _len_suffix = _db_suffix.length();
	string db_name, backup_path, cmd;
	if (argc < 2)
	{
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
			cout << "\t-p,--path [optional],\t\tthe backup path,defalut backup_path is \""
			     << default_backup_path +"\",the path should not include your database's name!" << endl;
			cout << endl;
			return 0;
		}
		else
		{
			cout << "Invalid arguments! Input \"bin/gbackup -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		db_name = Util::getArgValue(argc, argv, "db", "database");
		backup_path = Util::getArgValue(argc, argv, "p", "path");
		if (backup_path.empty())
		{
			backup_path = default_backup_path;
		}
		int len = db_name.length();
		if (db_name.length() > _len_suffix && db_name.substr(len - _len_suffix, _len_suffix) == _db_suffix)
		{
			cout << "the database name can not end with " + _db_suffix + "! Input \"bin/gbackup -h\" for help." << endl;
			return 0;
		}
		if (db_name == "system")
		{
			cout << "Your database's name can not be system." << endl;
			return 0;
		}
		if (backup_path == "." || Util::getExactPath(backup_path.c_str()) == Util::getExactPath(_db_home.c_str()))
		{
			cout << "Backup path can not be root or \"" + _db_home + "\", Backup Failed!" << endl;
			return 0;
		}
		// TODO: We need two column in system.db :
		// build_path and backup_path;
		// we can get the build_path as database_path should write the backup_path when we finish the backup
		// query database_name build_path
		// insert database_name backup_path
		long tv_begin = Util::get_cur_time();
		Database system_db("system");
		system_db.load();

		string sparql = "ASK WHERE{<" + db_name + "> <database_status> \"already_built\".}";
		ResultSet ask_rs;
		FILE *ask_ofp = stdout;
		system_db.query(sparql, ask_rs, ask_ofp);
		if (ask_rs.answer[0][0] == "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>")
		{
			cout << "The database does not exist." << endl;
			return 0;
		}
		// create it if backup path does not exist
		if (Util::dir_exist(backup_path) == false)
		{
			Util::create_dirs(backup_path);
		}
		Database _db(db_name);
		// call Database::backup()
		bool flag = _db.backup();
		if(flag == false) 
		{
			cout << "Backup Failed!" << endl;
			return 0;
		}
		string timestamp = Util::get_timestamp();
		string new_folder =  db_name + _db_suffix + "_" + timestamp;
		cmd = "mv " + default_backup_path + "/" + db_name + _db_suffix + " " + backup_path + "/" + new_folder;
		system(cmd.c_str());
		long tv_end = Util::get_cur_time();
		cout << "Used " << (tv_end - tv_begin) << " ms" << endl;
		return 0;
	}
}
