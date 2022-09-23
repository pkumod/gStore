/*
 * 
 * @Author: Bookug Lobert suxunbin liwenjie
 * @Date: 2021-08-20 10:29:41
 * @LastEditTime: 2022-09-23 10:50:25
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: The build database tool 
 * @FilePath: /gstore/Main/gbuild.cpp
 */


#include "../Util/Util.h"
#include "../Database/Database.h"
//#include "../Util/Slog.h"

using namespace std;

//[0]./gbuild [1]data_folder_path  [2]rdf_file_path
int
main(int argc, char * argv[])
{
	//chdir(dirname(argv[0]));
//#ifdef DEBUG
	Util util;

//#endif
	//Log.init("slog.properties");
	if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		//Log.Error("Invalid arguments! Input \"bin/gbuild -h\" for help.");
		cout<<"Invalid arguments! Input \"bin/gbuild -h\" for help."<<endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Build Tools(gbuild)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gbuild -db [dbname] -f [filename] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. " << endl;
			cout << "\t-f,--file,\t\tthe file path for building." << endl;
			cout << endl;
			return 0;
		}
		else
		{
			//cout << "the command is not complete." << endl;
			cout<<"Invalid arguments! Input \"bin/gbuild -h\" for help."<<endl;
			return 0;
		}
	}
	else
	{
		string _db_path = Util::getArgValue(argc, argv, "db", "database");
		int len = _db_path.length();
		if (_db_path.empty())
		{
			cout<<"You must input the database name for building database!"<<endl;
			return -1;
		}
		if (_db_path.length() > 3 && _db_path.substr(len - 3, 3) == ".db")
		{
			
			cout << "your database can not end with .db or less than 3 characters" << endl;
			return -1;
		}

		//check if the db_name is system
		if (_db_path == "system")
		{
			cout<<"Your database's name can not be system."<<endl;
			return -1;
		}

		string _rdf = Util::getArgValue(argc, argv, "f", "file");

		//check if the db_path is the path of system.nt
		util.configure_new();
		string system_path = util.getConfigureValue("system_path");
		if (_rdf == system_path)
		{
			cout<<"You have no rights to access system files"<<endl;
			return -1;
		}
		if (_rdf.empty())
		{
			cout<<"You must input the rdf file path for building database!"<<endl;
			return -1;
		}

		int isbuilt;
		if (Util::file_exist(_db_path + ".db"))
			isbuilt = 1;
		else
			isbuilt = 0;
		if(isbuilt == 1)
		{
			cout<<"the database "+_db_path+" has been built. please use bin/gdrop to remove it at first."<<endl;
			return -1;
		}
		else
		{
			cout<<"Begin to build database...."<<endl;
			long tv_begin = Util::get_cur_time();
			Database _db(_db_path);
			bool flag = _db.build(_rdf);
			if (flag)
			{
			
				cout<<"Build Database Successfully!"<<endl;
				ofstream f;
				f.open("./" + _db_path + ".db/success.txt");
				f.close();
			}
			else //if fails, drop database and return
			{
			
				cout<<"Build Database Failed!"<<endl;
				string cmd = "rm -r " + _db_path + ".db";
				system(cmd.c_str());
				return 0;
			}
			if (!Util::dir_exist("system.db"))
				return 0;
			//system("clock");
			cout<<"Save the database info to system database...."<<endl;
			Database system_db("system");
			system_db.load();

			//if isbuilt is false, add database information to system.db
			if (isbuilt == 0)
			{
				string time = Util::get_date_time();
				string sparql = "INSERT DATA {<" + _db_path + "> <database_status> \"already_built\"." + "<" + _db_path + "> <built_by> <root>."
					+ "<" + _db_path + "> <built_time> \"" + time + "\".}";
				cout<<"sparql:"<<sparql<<endl;
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
					/*if (ret != -100)
						cout << msg << endl;*/
				}
				Util::add_backuplog(_db_path);
				cout<<"Saving database info:" + msg<<endl;
				
			}
			long tv_end = Util::get_cur_time();
			//stringstream ss;
			cout<< "Build RDF database "<<_db_path<<" successfully! Used " << (tv_end - tv_begin) << " ms"<<endl;
			//Log.Info(ss.str().c_str());
			return 0;
		}
	}
	//if(argc < 3)  //./gbuild
	//{
	//	//output help info here
	//	cout << "the usage of gbuild: " << endl;
	//	cout << "bin/gbuild your_database_name.db your_dataset_path" << endl;
	//	return 0;
	//}
	////system("clock");
	//cout << "gbuild..." << endl;
	//{
	//	cout << "argc: " << argc << "\t";
	//	cout << "DB_store:" << argv[1] << "\t";
	//	cout << "RDF_data: " << argv[2] << "\t";
	//	cout << endl;
	//}

	//string _db_path = string(argv[1]);
	//int len = _db_path.length();
	//if(_db_path.length() > 3 && _db_path.substr(len-3, 3) == ".db")
	//{
	//	cout<<"your database can not end with .db or less than 3 characters"<<endl;
	//	return -1;
	//}

	////check if the db_name is system
	//if (_db_path == "system")
	//{
	//	cout<< "Your database's name can not be system."<<endl;
	//	return -1;
	//}

	////if(_db_path[0] != '/' && _db_path[0] != '~')  //using relative path
	////{
	////_db_path = string("../") + _db_path;
	////}
	//string _rdf = string(argv[2]);

	////check if the db_path is the path of system.nt
	//if (_rdf == SYSTEM_PATH)
	//{
	//	cout<< "You have no rights to access system files"<<endl;
	//	return -1;
	//}

	////if(_rdf[0] != '/' && _rdf[0] != '~')  //using relative path
	////{
	////_rdf = string("../") + _rdf;
	////}

	////check if the database is already built
	//int isbuilt;
	//if (boost::filesystem::exists(_db_path + ".db"))
	//	isbuilt = 1;
	//else
	//	isbuilt = 0;

	////build database
	//Database _db(_db_path);
	//bool flag = _db.build(_rdf);
	//if (flag)
	//{
	//	cout << "import RDF file to database done." << endl;
	//	ofstream f;
	//	f.open("./"+ _db_path +".db/success.txt");
	//	f.close();
	//}
	//else //if fails, drop database and return
	//{
	//	cout << "import RDF file to database failed." << endl;
	//	string cmd = "rm -r " + _db_path + ".db";
	//	system(cmd.c_str());
	//	return 0;
	//}
	//if (!boost::filesystem::exists("system.db"))
	//	return 0;
	////system("clock");

	//Database system_db("system");
	//system_db.load();

	////if isbuilt is false, add database information to system.db
	//if (isbuilt == 0)
	//{
	//	string time = Util::get_date_time();
	//	string sparql = "INSERT DATA {<" + _db_path + "> <database_status> \"already_built\"." + "<" + _db_path + "> <built_by> <root>."
	//		+ "<" + _db_path + "> <built_time> \"" + time + "\".}";
	//	ResultSet _rs;
	//	FILE* ofp = stdout;
	//	string msg;
	//	int ret = system_db.query(sparql, _rs, ofp);
	//	if (ret <= -100) // select query
	//	{
	//		if (ret == -100)
	//			msg = _rs.to_str();
	//		else //query error
	//			msg = "query failed";
	//	}
	//	else //update query
	//	{
	//		if (ret >= 0)
	//			msg = "update num : " + Util::int2string(ret);
	//		else //update error
	//			msg = "update failed.";
	//		if (ret != -100)
	//			cout << msg << endl;
	//	}
	//	Util::add_backuplog(_db_path);
	//	return 0;
	//}
	//else //if isbuilt is true, update built_time of the database
	//{
	//	string sparql = "DELETE {<" + _db_path + "> <built_time> ?t .}"
	//		+ "WHERE{<" + _db_path + "> <built_time> ?t .}";
	//	ResultSet _rs;
	//	FILE* ofp = stdout;
	//	string msg;
	//	int ret = system_db.query(sparql, _rs, ofp);
	//	if (ret <= -100) // select query
	//	{
	//		if (ret == -100)
	//			msg = _rs.to_str();
	//		else //query error
	//			msg = "query failed";
	//	}
	//	else //update query
	//	{
	//		if (ret >= 0)
	//			msg = "update num : " + Util::int2string(ret);
	//		else //update error
	//			msg = "update failed.";
	//		if (ret != -100)
	//			cout << msg << endl;
	//	}
	//	cout << "delete successfully" << endl;
	//}
	//string time = Util::get_date_time();
	//string sparql = "INSERT DATA {<" + _db_path + "> <built_time> \"" + time + "\".}";
	//ResultSet _rs;
	//FILE* ofp = stdout;
	//string msg;
	//int ret = system_db.query(sparql, _rs, ofp);
	//if (ret <= -100) // select query
	//{
	//	if (ret == -100)
	//		msg = _rs.to_str();
	//	else //query error
	//		msg = "query failed";
	//}
	//else //update query
	//{
	//	if (ret >= 0)
	//		msg = "update num : " + Util::int2string(ret);
	//	else //update error
	//		msg = "update failed.";
	//	if (ret != -100)
	//		cout << msg << endl;
	//}
	//cout << "backuplog" << endl;
	//
	//cout << "insert successfully" << endl;

	//cout<<endl<<endl<<endl<< "import RDF file to database done." << endl;
	//return 0;
}

