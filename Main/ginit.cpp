/*=============================================================================
# Filename: ginit.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2018-10-17 14:59
# Description: used to initialize the system.db
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

int main(int argc, char * argv[])
{
	if(argc > 1)
	{
		if(boost::filesystem::exists("system.db"))
			return 0;
	}

	//build system.db
	Util util;
	string _db_path = "system";
	string _rdf = "data/system/system.nt";
	Database* _db = new Database(_db_path);
	bool flag = _db->build(_rdf);
	if (flag)
	{
		cout << "import RDF file to database done." << endl;
		ofstream f;
		f.open("./"+ _db_path +".db/success.txt");
		f.close();
	}
	else //if fails, drop system.db and return
	{
		cout << "import RDF file to database failed." << endl;
		string cmd = "rm -r " + _db_path + ".db";
		system(cmd.c_str());
		delete _db;
		_db = NULL;
		return 0;
	}

	//insert built_time of system.db
	delete _db;
	_db = new Database(_db_path);
	_db->load();
	string time = Util::get_date_time();
	string sparql = "INSERT DATA {<system> <built_time> \"" + time + "\".}";
	ResultSet _rs;
	FILE* ofp = stdout;
	string msg;
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
	cout << "system.db is built successfully!" << endl;
	return 0;
}
