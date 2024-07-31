/*=============================================================================
# Filename: gdrop.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2021-8-15 23:00:33
# Modified By:liwenjie
# Description: used to drop database
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"
//#include "../Util/Slog.h"

using namespace std;

int main(int argc, char * argv[])
{
	Util util;
	//Log.init("slog.properties");
	string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
	int _len_suffix = _db_suffix.length();
	string db_name;
	if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		cout<<"Invalid arguments! Input \"bin/gdrop -h\" for help."<<endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Drop Database Tools(gdrop)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gdrop -db [dbname] -f [filename] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. Notice that the name not end with "<<_db_suffix<< endl;
			cout << endl;
			return 0;
		}
		else
		{
			cout<<"Invalid arguments! Input \"bin/gdrop -h\" for help."<<endl;
			return 0;
		}
	}
	else
	{
		db_name = Util::getArgValue(argc, argv, "db", "database");
		if (db_name.empty())
		{
			cout<<"The database name can not been empty! Input \"bin/gdrop -h\" for help."<<endl;
			return 0;
		}
		else
		{
			int len = db_name.length();
			if (db_name.length() > _len_suffix && db_name.substr(len - _len_suffix, _len_suffix) == _db_suffix)
			{
				cout<<"The database name can not end with " + _db_suffix + "! Input \"bin/gdrop -h\" for help." << endl;
				return 0;
			}
			if (db_name == "system")
			{
				cout << "the database name can not be system." << endl;
				return 0;
			}
			cout<<"Begin to drop database...."<<endl;
			long tv_begin = Util::get_cur_time();
			Database system_db("system");
			system_db.load();
			string db_path = _db_home + db_name + _db_suffix;
			if (!Util::dir_exist(db_path))
			{
				cout<<"The database that you want to drop does not exist."<<endl;
				string sparql = "ASK WHERE{<" + db_name + "> <database_status> \"already_built\".}";
				ResultSet ask_rs;
				system_db.query(sparql, ask_rs, nullptr);
				if (ask_rs.answer[0][0] == "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>")
				{
					return 0;
				}
				else
				{
					SLOG_INFO("The database db file " + db_path + " not exist but system info exist.");
				}
			}
			ResultSet _rs;
			int ret;
			string sparql = "DELETE WHERE {<" + db_name + "> <built_by> ?bb; <built_time> ?bt; <database_status> ?bs.}";
			ret = system_db.query(sparql, _rs, nullptr);
			SLOG_DEBUG("delete num: " + to_string(ret));
			if (ret > 0)
				system_db.save();
			else
				cout << "WARN: Drop info about database " << db_name << " failed! Please check system db." << endl;
			Util::remove_path(db_path);
			Util::delete_backuplog(db_name);
			long tv_end = Util::get_cur_time();
			cout << "Database " << db_name << " dropped successfully! Used " << (tv_end - tv_begin) << " ms"<<endl;
			return 0;
		}
	}
}
