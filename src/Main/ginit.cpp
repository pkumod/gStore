/*=============================================================================
# Filename: ginit.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2018-10-17 14:59
# Description: used to initialize the system.db
# Modified by liwenjie
# Modified Date: 2020-03-26 10:33
# Description£ºAdd the args "-cv" for updating the coreversion and the args "-av" for updating the apiversion in ginit function
# Description£ºAdd the args "-u" for updating the version information when updating the gstore.
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"
#include <iostream>
#include <fstream>
// #include "../Util/Slog.h"
using namespace std;

int main(int argc, char *argv[])
{

	Util util;
	// Log.init("slog.properties");
	string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
	int _suffix_len = _db_suffix.length();
	string _db_name = "system";
	string _db_path = _db_home + "/" + _db_name + _db_suffix;
	string _rdf = Util::system_path;
	if (argc == 1)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		long tv_begin = Util::get_cur_time();
		cout << "begin rebuild the system database ...." << endl;
		if (Util::dir_exist(_db_path))
		{
			Util::remove_path(_db_path);
		}
		Database *_db = new Database(_db_name);
		bool flag = _db->build(_rdf);
		if (flag)
		{
			ofstream f;
			f.open(_db_path + "/success.txt");
			f.close();

			cout << _db_name + _db_suffix + " rebuild successfully!" << endl;
			delete _db;
			_db = NULL;
			Util::init_backuplog();
			cout << "init backuplog successfully!" << endl;
			Util::configure_new();
			string version = Util::getConfigureValue("version");
			string root_pwd = Util::getConfigureValue("root_password");
			string update_sparql = "insert data {<CoreVersion> <value> \"" + version + "\". <root> <has_password> \"" + root_pwd + "\" .}";
			cout << "version:" << version << ",update_sparql:" << update_sparql << endl;
			ResultSet _rs;
			FILE *ofp = stdout;
			string msg;
			_db = new Database(_db_name);
			_db->load();
			int ret = _db->query(update_sparql, _rs, ofp);
			if (ret >= 0)
				msg = "update num : " + Util::int2string(ret);
			else // update error
				msg = "update failed.";
			if (ret != -100)
				cout << "Insert data result:" + msg << endl;

			delete _db;
			_db = NULL;
			long tv_end = Util::get_cur_time();
			// stringstream ss;
			cout << _db_name + _db_suffix + " init successfully! Used " << (tv_end - tv_begin) << " ms" << endl;
			// Log.Info(ss.str().c_str());
			return 0;
		}
		else
		{
			cout << "Build RDF database failure!" << endl;
			return 0;
		}
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Initialize Data Tools(ginit)" << endl;
			cout << endl;
			cout << "Usage:\tbin/ginit  -db [dbname] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t The database names.Use , to split database name. e.g. databaseA,databaseB" << endl;
			cout << "If you want to rebuild the system database,please use bin/ginit \n"
				 << endl;
			cout << "If you want to add database info into system database,please use bin/ginit -db db1,db2,...\n"
				 << endl;
			cout << endl;
			return 0;
		}
		else if (command == "-m" || command == "--make")
		{
			if (Util::dir_exist(_db_path) == false)
			{
				cout << _db_name + _db_suffix + " is not exist. Now create it." << endl;
				long tv_begin = Util::get_cur_time();
				Database *_db = new Database(_db_name);
				bool flag = _db->build(_rdf);
				if (flag)
				{
					ofstream f;
					f.open(_db_path + "/success.txt");
					f.close();

					cout << _db_name + _db_suffix + " built successfully!" << endl;
					delete _db;
					_db = NULL;

					Util::init_backuplog();
					cout << "init backuplog successfully!" << endl;
					Util::configure_new();
					string version = Util::getConfigureValue("version");
					string root_pwd = Util::getConfigureValue("root_password");
					string update_sparql = "insert data {<CoreVersion> <value> \"" + version + "\". <root> <has_password> \"" + root_pwd + "\" .}";

					ResultSet _rs;
					FILE *ofp = stdout;
					string msg;
					_db = new Database(_db_name);
					_db->load();
					int ret = _db->query(update_sparql, _rs, ofp);
					if (ret >= 0)
						msg = "update num : " + Util::int2string(ret);
					else // update error
						msg = "update failed.";
					if (ret != -100)
						cout << "Insert data result:" + msg << endl;

					delete _db;
					_db = NULL;
					long tv_end = Util::get_cur_time();
					// stringstream ss;
					cout << _db_name + _db_suffix + " init successfully! Used " << (tv_end - tv_begin) << " ms" << endl;
					// Log.Info(ss.str().c_str());
					return 0;
				}
				else
				{
					cout << "Build RDF database failure!" << endl;
					return 0;
				}
			}
			else
			{
				cout << "the system database is exist, skip the init system database." << endl;
			}
		}
		else
		{
			// cout << "the command is not complete." << endl;
			cout << "Invalid arguments! Input \"bin/ginit -h\" for help." << endl;
			return 0;
		}
	}
	else
	{

		long tv_begin = Util::get_cur_time();
		if (Util::dir_exist(_db_path) == false)
		{
			cout << "The system database is not exist,please use bin/ginit to rebuild the system database at first!" << endl;
			return -1;
		}
		string db_namestr = Util::getArgValue(argc, argv, "db", "database");
		if (db_namestr.empty())
		{
			cout << "You need to input the database name that you want to init. Input \"bin/ginit -h\" for help." << endl;
			return -1;
		}
		vector<string> db_names;
		if (db_namestr.substr(db_namestr.length() - 1, 1) != ",")
		{
			db_namestr = db_namestr + ",";
		}
		Util::split(db_namestr, ",", db_names);
		string sparql = "insert data {";
		string time = Util::get_date_time();
		for (auto db_name : db_names)
		{
			if (!db_name.empty())
			{
				string db_path = _db_home + "/" + db_name + _db_suffix;
				if (Util::dir_exist(db_path) == false)
				{
					cout << "The database " + db_name + " is not exist, now create it." << endl;
					int len = db_name.length();
					if (len < _suffix_len || (len >= _suffix_len && db_name.substr(len - _suffix_len, _suffix_len) == _db_suffix))
					{
						cout << "your database can not end with " + _db_suffix + " or less than "<< _suffix_len <<" characters. Skip it." << endl;
						continue;
					}
					if (db_name == "system")
					{
						cout<<"Your database's name can not be system. Skip it."<<endl;
						continue;
					}
					Database db(db_name);
					db.BuildEmptyDB();
				}
				sparql = sparql + "<" + db_name + "> <database_status> \"already_built\".";
				sparql = sparql + "<" + db_name + "> <built_by> <root>.";
				sparql = sparql + "<" + db_name + "> <built_time> \"" + time + "\".";
				Util::add_backuplog(db_name);
			}
		}
		sparql = sparql + "}";
		FILE *ofp = stdout;
		string msg;

		ResultSet _rs;
		Database *_db = new Database(_db_name);
		_db->load();
		int ret = _db->query(sparql, _rs, ofp);
		if (ret <= -100) // select query
		{
			if (ret == -100)
				msg = _rs.to_str();
			else // query error
				msg = "query failed";
		}
		else // update query
		{
			if (ret >= 0)
				msg = "update num : " + Util::int2string(ret);
			else // update error
				msg = "update failed.";
		}
		delete _db;
		_db = NULL;
		long tv_end = Util::get_cur_time();
		// stringstream ss;
		cout << _db_name + _db_suffix + " init successfully! Used " << (tv_end - tv_begin) << " ms" << endl;
		// Log.Info(ss.str().c_str());
		return 0;
	}
}
