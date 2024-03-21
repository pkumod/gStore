/*=============================================================================
# Filename: gexport.cpp
# Author: suxunbin,liwenjie
# Last Modified: 2021-8-15 23:15:16
# Description: export a database to get .nt file
=============================================================================*/

#include "../Database/Database.h"
#include "../Util/Util.h"
#include "../Util/CompressFileUtil.h"
//#include "../Util/Slog.h"
using namespace std;

int
main(int argc, char * argv[])
{
	Util util;
	//Log.init("slog.properties");
	string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
	size_t _len_suffix = _db_suffix.length();
	string db_name;
	string filepath;
	if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		cout<<"Invalid arguments! Input \"bin/gexport -h\" for help."<<endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Export Data Tools(gexport)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gexport -db [dbname] -f [backup_path] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. " << endl;
			cout << "\t-f,--file [optional],\t\tthe export path,defalut export file path is current path,the path should not include your database's name!" << endl;
			cout << "\t-z,--zip [optional],\t\t0:export nt file, 1:export zip file" << endl;
			cout << endl;
			return 0;
		}
		else
		{
			//cout << "the command is not complete." << endl;
			cout<<"Invalid arguments! Input \"bin/gexport -h\" for help."<<endl;
			return 0;
		}
	}
	else
	{
		db_name= Util::getArgValue(argc, argv, "db", "database");
		if (db_name.empty())
		{
			cout<<"You need to input the database name that you want to export. Input \"bin/gexport -h\" for help."<<endl;
			return 0;
		}
		int len = db_name.length();
		if (db_name.length() > _len_suffix && db_name.substr(len - _len_suffix, _len_suffix) == _db_suffix)
		{
			cout<<"The database name can not end with " + _db_suffix + "! Input \"bin/gexport -h\" for help." << endl;
			return 0;
		}
		filepath= Util::getArgValue(argc, argv, "f", "file");
		std::string zip_path;
		if (filepath.empty())
		{
			filepath = db_name + "_" + Util::get_timestamp() + ".nt";
			zip_path = db_name + "_" + Util::get_timestamp() + ".zip";
		}
		else
		{
			if (filepath[filepath.length() - 1] != '/')
				filepath = filepath + "/";
			if (!Util::dir_exist(filepath))
				Util::create_dirs(filepath);
			zip_path = filepath + db_name  + "_" + Util::get_timestamp() +  ".zip";
			filepath = filepath + db_name  + "_" + Util::get_timestamp() +  ".nt";
		}
		cout << "gexport..." << endl;

		Database system_db("system");
		system_db.load();

		string sparql = "ASK WHERE{<" + db_name + "> <database_status> \"already_built\".}";
		ResultSet ask_rs;
		FILE* ask_ofp = stdout;
		// todo: check this return value
		system_db.query(sparql, ask_rs, ask_ofp);
		// int ret = system_db.query(sparql, ask_rs, ask_ofp);
		if (ask_rs.answer[0][0] == "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>")
		{
			cout<<"The database does not exist."<<endl;
			return 0;
		}
		long tv_begin = Util::get_cur_time();

		cout<<"start exporting the database......"<<endl;
		Database _db(db_name);
		_db.load();
		cout<<"finish loading"<<endl;

		// sparql = "select * where{?x ?y ?z.}";
		// ResultSet _rs;
		// FILE* ofp = fopen(filepath.c_str(), "w");
		// ret = _db.query(sparql, _rs, ofp, false, true);
		// fflush(ofp);
		// fclose(ofp);
		// ofp = NULL;
		// long tv_end = Util::get_cur_time();
		// /*stringstream ss;*/
		// cout << db_name << ".db exported successfully! Used " << (tv_end - tv_begin) << " ms"<<endl;
		// cout << db_name << ".db export path: " << filepath << endl;
		// //Log.Info(ss.str().c_str());

		FILE* ofp = fopen(filepath.c_str(), "w");
		_db.export_db(ofp);
		fflush(ofp);
		fclose(ofp);
		ofp = NULL;
		std:string zip = Util::getArgValue(argc, argv, "z", "zip");
		if (zip == "0")
		{
			long tv_end = Util::get_cur_time();
			cout << db_name << _db_suffix + " exported successfully! Used " << (tv_end - tv_begin) << " ms"<<endl;
			cout << db_name << _db_suffix + " export path: " << filepath << endl;
		}
		else
		{
			if (!CompressUtil::FileHelper::compressExportZip(filepath, zip_path))
			{
				cout << db_name << _db_suffix + " export compress fail"<<endl;
				std::string cmd = filepath + " " + zip_path;
				Util::remove_path(cmd);
				return -1;
			}
			long tv_end = Util::get_cur_time();
			cout << db_name << _db_suffix + " exported successfully! Used " << (tv_end - tv_begin) << " ms"<<endl;
			cout << db_name << _db_suffix + " export path: " << zip_path << endl;
			Util::remove_path(filepath);
		}
		return 0;
	}
	/*if (argc == 1)
	{
		cout << "You need to input the database name that you want to export." << endl;
		return 0;
	}
	else if (argc == 2)
	{
		db_name = argv[1];
		int len = db_name.length();
		if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db")
		{
			cout << "The database name can not end with .db" << endl;
			return 0;
		}
		filepath = db_name + ".nt";
	}
	else if (argc == 3)
	{
		db_name = argv[1];
		int len = db_name.length();
		if (db_name.length() > 3 && db_name.substr(len - 3, 3) == ".db")
		{
			cout << "The database name can not end with .db" << endl;
			return 0;
		}
		filepath = argv[2];
		if(filepath[filepath.length()-1] != '/')
			filepath = filepath + "/";
		if(!boost::filesystem::exists(filepath))
				boost::filesystem::create_directories(filepath);
		filepath = filepath + db_name + ".nt";
	}

	cout << "gexport..." << endl;

	Database system_db("system");
	system_db.load();

	string sparql = "ASK WHERE{<" + db_name + "> <database_status> \"already_built\".}";
	ResultSet ask_rs;
	FILE* ask_ofp = stdout;
	int ret = system_db.query(sparql, ask_rs, ask_ofp);
	if (ask_rs.answer[0][0] == "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>")
	{
		cout << "The database does not exist." << endl;
		return 0;
	}

	cout << "start exporting the database......" << endl;
	Database _db(db_name);
	_db.load();
	cout << "finish loading" << endl;

	FILE* ofp = fopen(filepath.c_str(), "w");
    _db.export_db(ofp);
    fflush(ofp);
	fclose(ofp);
	ofp = NULL;
	cout << "finish exporting the database." << endl;

	return 0;*/
}
