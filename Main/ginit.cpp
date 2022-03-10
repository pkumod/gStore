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
//#include "../Util/Slog.h"
using namespace std;

int main(int argc, char * argv[])
{
	
	Util util;
	//Log.init("slog.properties");
	
	if (argc == 1)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		long tv_begin = Util::get_cur_time();
		cout<<"begin rebuild the system database ...."<<endl;
		if (Util::dir_exist("system.db"))
		{
			string cmd;
			cmd = "rm -r system.db";
			system(cmd.c_str());
			
		}
		string _db_path = "system";
		string _rdf = "./data/system/system.nt";
		Database* _db = new Database(_db_path);
		bool flag = _db->build(_rdf);
		if (flag)
		{
			
			ofstream f;
			f.open("./" + _db_path + ".db/success.txt");
			f.close();

			cout<<"system.db rebuild successfully!"<<endl;
			delete _db;
			_db = NULL;
			Util::init_backuplog();
			cout<<"init backuplog successfully!"<<endl;
			Util::configure_new();
			string version = Util::getConfigureValue("version");
			string update_sparql = "insert data {<CoreVersion> <value> \"" + version + "\". }";
			cout<<"version:"<<version<<",update_sparql:"<<update_sparql<<endl;
			ResultSet _rs;
			FILE* ofp = stdout;
			string msg;
			_db = new Database(_db_path);
			_db->load();
			int ret = _db->query(update_sparql, _rs, ofp);
			if (ret >= 0)
				msg = "update num : " + Util::int2string(ret);
			else //update error
				msg = "update failed.";
			if (ret != -100)
				cout<<"Insert data result:" + msg<<endl;

			
		
			delete _db;
		    _db = NULL;
			long tv_end = Util::get_cur_time();
			//stringstream ss;
			cout << "system.db init successfully! Used " << (tv_end - tv_begin) << " ms"<<endl;
			//Log.Info(ss.str().c_str());
			return 0;

		}
		else
		{
			cout<<"Build RDF database failure!"<<endl;
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
			if (Util::dir_exist("system.db") == false)
			{
				cout << "system.db is not exist. Now create it." << endl;
				long tv_begin = Util::get_cur_time();

				string _db_path = "system";
				string _rdf = "./data/system/system.nt";
				Database *_db = new Database(_db_path);
				bool flag = _db->build(_rdf);
				if (flag)
				{

					ofstream f;
					f.open("./" + _db_path + ".db/success.txt");
					f.close();

					cout << "system.db built successfully!" << endl;
					delete _db;
					_db = NULL;
					Util::init_backuplog();
					Util::configure_new();
					string version = Util::getConfigureValue("version");
					string update_sparql = "insert data {<CoreVersion> <value> \"" + version + "\". }";

					ResultSet _rs;
					FILE *ofp = stdout;
					string msg;
					_db = new Database(_db_path);
					_db->load();
					int ret = _db->query(update_sparql, _rs, ofp);
					if (ret >= 0)
						msg = "update num : " + Util::int2string(ret);
					else //update error
						msg = "update failed.";
					if (ret != -100)
						cout << "Insert data result:" + msg << endl;

					delete _db;
					_db = NULL;
					long tv_end = Util::get_cur_time();
					//stringstream ss;
					cout << "system.db init successfully! Used " << (tv_end - tv_begin) << " ms" << endl;
					//Log.Info(ss.str().c_str());
					return 0;
				}
				else
				{
					cout << "Build RDF database failure!" << endl;
					return 0;
				}
			}
			else{
				cout<<"the system database is exist, skip the init system database."<<endl;
			}
		}
		else
		{
			//cout << "the command is not complete." << endl;
			cout << "Invalid arguments! Input \"bin/ginit -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		
		long tv_begin = Util::get_cur_time();
		if (Util::dir_exist("system.db")==false)
		{
			cout<<"The system database is not exist,please use bin/ginit to rebuild the system database at first!"<<endl;
			return -1;

		}
		string db_namestr = Util::getArgValue(argc, argv, "db", "database");
		if (db_namestr.empty())
		{
			cout<<"You need to input the database name that you want to init. Input \"bin/ginit -h\" for help."<<endl;
			return 0;
		}
		else
		{
			vector<string> db_names;
			if (db_namestr.substr(db_namestr.length()-1,1) != ",")
			{
				db_namestr = db_namestr + ",";
			}
		
			Util::split(db_namestr, ",", db_names);
			string sparql = "insert data {";
			string time = Util::get_date_time();
			for (int i = 0; i < db_names.size(); i++)
			{
				string db_name =db_names[i];
				if (db_name.empty())
				{
					cout<<"the database name is empty!"<<endl;
					continue;
				}
				if (Util::dir_exist(db_name + ".db")==false)
				{
					cout<<"The database " + db_name + " is not exist"<<endl;
					continue;
				}
				sparql = sparql + "<" + db_name + "> <database_status> \"already_built\".";
				sparql = sparql + "<" + db_name + "> <built_by> <root>.";
				sparql = sparql + "<" + db_name + "> <built_time> \"" + time + "\".";
				Util::add_backuplog(db_name);
			}
			sparql = sparql + "}";
			FILE* ofp = stdout;
			string msg;
			string _db_path = "system";
			
			ResultSet _rs;
			Database* _db = new Database(_db_path);
			_db->load();
			int ret = _db->query(sparql, _rs, ofp);
			if (ret <= -100) // select query
			{
				if (ret == -100)
					msg = _rs.to_str();
				else //query error
					msg = "query failed";
			}
			else //update query
			{
				if (ret >= 0)
					msg = "update num : " + Util::int2string(ret);
				else //update error
					msg = "update failed.";
				
			}
			delete _db;
			_db = NULL;
			long tv_end = Util::get_cur_time();
			//stringstream ss;
			cout << "system.db init successfully! Used " << (tv_end - tv_begin) << " ms"<<endl;
			//Log.Info(ss.str().c_str());
			
			return 0;

		}
	}
	return 0;
}
