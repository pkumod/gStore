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
#include "../Util/Slog.h"
using namespace std;

int main(int argc, char * argv[])
{
	string op;
	Util util;
	Log.init("slog.properties");
	ResultSet _rs;
	bool no_sysdb = false;

	if (argc == 1)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		Log.Info("begin rebuild the system database ....");
		if (boost::filesystem::exists("system.db"))
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

			Log.Info("system.db rebuild successfully!");
			delete _db;
			_db = NULL;
			Util::init_backuplog();
			Util::configure_new();
			string version = Util::getConfigureValue("version");
			string update_sparql = "insert data {<CoreVersion> <value> " + version + ". }";
			Log.Info(("update_sparql:" + update_sparql + "").c_str());
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
				Log.Info(("Insert data result:" + msg).c_str());
			Log.Info("import RDF file to database done.");
			delete _db;
		    _db = NULL;
			return 0;

		}
		else
		{
			Log.Error("Build RDF database failure!");
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
			cout << "Usage:\tbin/ginit -a -db [dbname] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-a,--append\t\tAppend the database info into system database." << endl;
			cout << "\t-db,--database,\t\t The database names.Use & to split database name. e.g. databaseA&databaseB" << endl;
		    cout << endl;
			return 0;
		}
		else
		{
			//cout << "the command is not complete." << endl;
			Log.Error("Invalid arguments! Input \"bin/ginit -h\" for help.");
			return 0;
		}
	}
	else
	{
		Log.Info("begin rebuild the system database ....");
		if (boost::filesystem::exists("system.db"))
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

			Log.Info("system.db rebuild successfully!");
			delete _db;
			_db = NULL;
			Util::init_backuplog();
			Util::configure_new();
			string version = Util::getConfigureValue("version");
			string update_sparql = "insert data {<CoreVersion> <value> " + version + ". }";
			Log.Info(("update_sparql:" + update_sparql + "").c_str());
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
				Log.Info(("Insert data result:" + msg).c_str());
			Log.Info("import RDF file to database done.");
			delete _db;
			_db = NULL;
			

		}
		else
		{
			Log.Error("Build RDF database failure!");
			return 0;
		}
		string db_namestr = Util::getArgValue(argc, argv, "db", "database");
		if (db_namestr.empty())
		{
			Log.Error("You need to input the database name that you want to init. Input \"bin/ginit -h\" for help.");
			return 0;
		}
		else
		{
			vector<string> db_names;
			Util::split(db_namestr, "&", db_names);
			string sparql = "insert data {";
			string time = Util::get_date_time();
			for (int i = 0; i < db_names.size(); i++)
			{
				string db_name =db_names[i];
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
				if (ret != -100)
					cout << msg << endl;
			}
			delete _db;
			_db = NULL;
			Log.Info("system.db init successfully!");
			return 0;

		}
	}
	return 0;



	//if(argc > 1)
	//{
	//	op = argv[1];
	//	if(op == "-make")
	//	{
	//		if(boost::filesystem::exists("system.db"))
	//			return 0;			
	//	}
	//	else if(op == "-d")
	//	{
	//		if(argc == 2)
	//		{
	//			cout << "You need to input at least one database name." << endl;
	//			return 0;
	//		}
	//	}
	//	else if (op == "-cv"||op=="-av")
	//	{
	//		if (argc == 2)
	//		{
	//			cout << "You need to input the value of version." << endl;
	//			return 0;
	//		}
	//		else
	//		{
	//			string version = argv[2];
	//			string versionname = "<CoreVersion>";
	//			if (op == "-av")
	//			{
	//				versionname = "<APIVersion>";
	//			}
	//			string sparql = "DELETE where {"+versionname+" <value> ?x.}";
	//		    
	//			string _db_path = "system";
	//			Util util;
	//			Database* _db = new Database(_db_path);
	//		
	//			_db->load();
	//			ResultSet _rs;
	//			FILE* ofp = stdout;
	//			string msg;
	//			int ret = _db->query(sparql, _rs, ofp);
	//			sparql = "INSERT DATA {"+versionname+" <value>	\"" + version + "\".}";
	//			ret = _db->query(sparql, _rs, ofp);
	//			if (ret <= -100) // select query
	//			{
	//				if (ret == -100)
	//					msg = _rs.to_str();
	//				else //query error
	//					msg = "query failed";
	//			}
	//			else //update query
	//			{
	//				if (ret >= 0)
	//					msg = "update num : " + Util::int2string(ret);
	//				else //update error
	//					msg = "update failed.";
	//				if (ret != -100)
	//					cout << msg << endl;
	//			}
	//			delete _db;
	//			_db = NULL;
	//			cout << "the "<<versionname<<" is updated successfully!" << endl;
	//			return 0;
	//		}
	//	}
	//	else if (op == "-u")
	//	{
	//		//update the gstore, and init the version info 
	//		string file = "data/system/version.nt";
	//		if (boost::filesystem::exists(file) == false)
	//		{
	//			cout << "the file of version information is not found!" << endl;
	//			
	//			return 0;
	//		}
	//		string sparql = "Delete WHERE { <CoreVersion> ?x ?y. <APIVersion> ?x1 ?y1.}";


	//		string _db_path = "system";
	//		Util util;
	//		Database* _db = new Database(_db_path);

	//		_db->load();
	//		ResultSet _rs;
	//		FILE* ofp = stdout;
	//		string msg;
	//		int ret = _db->query(sparql, _rs, ofp);
	//		ifstream infile;
	//		infile.open(file.data());   //½«ÎÄ¼þÁ÷¶ÔÏóÓëÎÄ¼þÁ¬½ÓÆðÀ´ 
	//		string s;
	//		sparql = "INSERT DATA {";
	//		while (getline(infile, s))
	//		{
	//			if (s != "")
	//				sparql = sparql + s;
	//		}
	//		infile.close();
	//		sparql = sparql + "}";
	//		cout << "the sparql of initversion is:" << sparql << endl;


	//		//sparql = "INSERT DATA {" + versionname + " <value>	\"" + version + "\".}";
	//		ret = _db->query(sparql, _rs, ofp);
	//		if (ret <= -100) // select query
	//		{
	//			if (ret == -100)
	//				msg = _rs.to_str();
	//			else //query error
	//				msg = "query failed";
	//		}
	//		else //update query
	//		{
	//			if (ret >= 0)
	//				msg = "update num : " + Util::int2string(ret);
	//			else //update error
	//				msg = "update failed.";
	//			if (ret != -100)
	//				cout << msg << endl;
	//		}
	//		delete _db;
	//		_db = NULL;
	//		cout << "the value of version is updated successfully!" << endl;
	//		return 0;
	//	}
	//	else
	//	{
	//		cout << "The initialization option is not correct." << endl;
	//		return 0;			
	//	}
	//}
	//
	//if(boost::filesystem::exists("system.db"))
	//{
	//	Database system_db("system");
	//	system_db.load();
	//	string sparql = "select ?s where{?s <database_status> \"already_built\".}";
	//	FILE* ofp = stdout;
	//	int ret = system_db.query(sparql, _rs, ofp);
	//}
	//else
	//   no_sysdb = true;
	//if(argc > 2){
	//	cout << "db reserved db reserved db reserved db reserved db reserved " << endl;
	//	vector<string> db_names;
	//	for(int i = 2; i < argc; i++){
	//		cout << argv[i] << endl;
	//		db_names.push_back(argv[i]);
	//	}
	//	for (int i = 0; i < _rs.ansNum; i++)
	//	{
	//		string db_name = _rs.answer[i][0];
	//		db_name.erase(0,1);
	//		db_name.pop_back();
	//		//cout << "db num:db num:db num:db num:db num:db num:db num:db num:" << db_name << endl;
	//		if(find(db_names.begin(), db_names.end(), db_name) == db_names.end() || db_name == "system")
	//		{
	//			string cmd;
	//			cmd = "rm -r " + db_name + ".db";
	//			system(cmd.c_str()); //maybe failed
	//		}
	//	}
	//}
	//else{
	//	if (!no_sysdb) {
	//		string cmd;
	//		cmd = "rm -r system.db";
	//		system(cmd.c_str());
	//		cout << _rs.ansNum << endl;
	//	}
	//	for (int i = 0; i < _rs.ansNum; i++)
	//	{
	//		string db_name = _rs.answer[i][0];
	//		db_name.erase(0,1);
	//		db_name.pop_back();
	//		string cmd;
	//		cmd = "rm -r " + db_name + ".db";
	//		system(cmd.c_str());
	//	}
	//}
	////build system.db
	//string _db_path = "system";
	//string _rdf = "./data/system/system.nt";
	//Database* _db = new Database(_db_path);
	//bool flag = _db->build(_rdf);
	//if (flag)
	//{
	//	cout << "import RDF file to database done." << endl;
	//	ofstream f;
	//	f.open("./"+ _db_path +".db/success.txt");
	//	f.close();
	//	Util::init_backuplog();
	//}
	//else //if fails, drop system.db and return
	//{
	//	cout << "import RDF file to database failed." << endl;
	//	string cmd = "rm -r " + _db_path + ".db";
	//	system(cmd.c_str());
	//	delete _db;
	//	_db = NULL;
	//	return 0;
	//}

	////insert built_time of system.db
	//delete _db;
	//_db = new Database(_db_path);
	//_db->load();
	//string time = Util::get_date_time();
	//string sparql = "INSERT DATA {<system> <built_time> \"" + time + "\".";
	//if(argc > 1)
	//{
	//	op = argv[1];
	//	if(op == "-d")
	//	{
	//		for(int i=2; i<argc; i++)
	//		{
	//			string db_name = argv[i];
	//			sparql = sparql + "<" + db_name + "> <database_status> \"already_built\".";
	//			sparql = sparql + "<" + db_name + "> <built_by> <root>.";
	//			sparql = sparql + "<" + db_name + "> <built_time> \"" + time + "\".";
	//			Util::add_backuplog(db_name);
	//		}		
	//	}
	//}
	//sparql = sparql + "}";
	//FILE* ofp = stdout;
	//string msg;
	//int ret = _db->query(sparql, _rs, ofp);
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
	//delete _db;
	//_db = NULL;
	//cout << "system.db is built successfully!" << endl;
	//return 0;
}
