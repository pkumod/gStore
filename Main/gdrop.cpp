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
			cout << "\t-db,--database,\t\t the database name. Notice that the name not end with .db " << endl;
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
			return -1;
		}
		else
		{
			int len = db_name.length();
			if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db")
			{
				cout<<"The database name can not end with .db"<<endl;
				return -1;
			}
			if (!Util::dir_exist(db_name + ".db"))
			{
				cout<<"The database that you want to drop does not exist."<<endl;
				return -1;
			}
			cout<<"Begin to drop database...."<<endl;
			long tv_begin = Util::get_cur_time();
			Database system_db("system");
			system_db.load();
			string sparql = "DELETE WHERE {<" + db_name + "> ?x ?y.}";
			ResultSet _rs;
			FILE* ofp = stdout;
			string msg;
			int ret = system_db.query(sparql, _rs, ofp);
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
			cout<<"Delete the database info from system database successfully!"<<endl;
			string cmd = "rm -r " + db_name + ".db";
			system(cmd.c_str());
			Util::delete_backuplog(db_name);
			long tv_end = Util::get_cur_time();
			//stringstream ss;
			cout << db_name << ".db is dropped successfully! Used " << (tv_end - tv_begin) << " ms"<<endl;
			//Log.Info(ss.str().c_str());
			return 0;
		}
		
	}


	
}
