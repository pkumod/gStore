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
#include "../Api/HttpUtil.h"
#include "../Database/Database.h"
#include <iostream>
#include <fstream>
// #include "../Util/Slog.h"
using namespace std;

int init_system_db(string _db_path, string _db_name, string _db_suffix, Util& util)
{
	std::string port = util.getConfigureValue("port");
	std::string base_url = "http://127.0.0.1:" + port + "/api";
	httpentities::CheckRequest check_request;
	httpentities::CheckResponse check_response = HttpUtil::check(base_url, check_request);
	if(check_response.success()) 
	{
		cout << "Server is active (running)." << endl;
		cout << "Please stop server(use bin/gserver -t) and try again." << endl;
		return -1;
	}
	string _rdf = Util::system_path;
	long tv_begin = Util::get_cur_time();
	SLOG_INFO("begin init the system database ....");
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

		SLOG_CORE(_db_name + _db_suffix + " rebuild successfully!");
		delete _db;
		_db = NULL;
		Util::init_backuplog();
		SLOG_CORE("init backuplog successfully!");
		string version = util.getConfigureValue("version");
		string root_pwd = util.getConfigureValue("root_password");
		string update_sparql = "insert data {<CoreVersion> <value> \"" + version + "\". <root> <has_password> \"" + root_pwd + "\" .}";
		SLOG_CORE("version: " << version << ", update_sparql:" << update_sparql);
		ResultSet _rs;
		FILE *ofp = nullptr;
		string msg;
		_db = new Database(_db_name);
		_db->load();
		int ret = _db->query(update_sparql, _rs, ofp);
		if (ret >= 0)
		{	
			long tv_end = Util::get_cur_time();
			SLOG_INFO("Insert data success, update num : " << ret);
			SLOG_INFO(_db_name + _db_suffix + " init successfully! Used " << (tv_end - tv_begin) << " ms" << endl);
		}
		else // update error
		{	
			Util::remove_path(_db_path);
			SLOG_ERROR(_db_name + _db_suffix + " init failure!");
		}	
		delete _db;
		_db = NULL;
		return 0;
	}
	else
	{
		SLOG_ERROR(_db_name + _db_suffix + " init failure!");
		return 0;
	}
}

int main(int argc, char *argv[])
{

	Util util;
	string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
	int _suffix_len = _db_suffix.length();
	string _db_name = "system";
	string _db_path = _db_home + "/" + _db_name + _db_suffix;
	if (argc == 1)
	{
		int ret = init_system_db(_db_path, _db_name, _db_suffix, util);
		return ret;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Initialize Data Tools(ginit)" << endl;
			cout << endl;
			cout << "Usage:\tbin/ginit" << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-m,--make\t\tInitialize if the system database does not exist, otherwise skip." << endl;
			cout << "If you want to rebuild the system database,please use bin/ginit" << endl;
			cout << endl;
			return 0;
		}
		else if (command == "-m" || command == "--make")
		{
			if (Util::file_exist(Util::initfile) == false)
			{
				int ret = init_system_db(_db_path, _db_name, _db_suffix, util);
				return ret;
			}
			else
			{
				SLOG_INFO("the system database is exist, skip the init system database.");
				return 0;
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
		cout << "Invalid arguments! Input \"bin/ginit -h\" for help." << endl;
		return 0;
	}
}
