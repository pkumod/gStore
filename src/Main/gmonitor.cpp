/*=============================================================================
# Filename: gmonitor.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2019-07-26 16:00
# Description: used to show information of the database
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

int main(int argc, char *argv[])
{
	Util util;
	string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
	size_t _len_suffix = _db_suffix.length();
	// Log.init("slog.properties");
	string db_name;
	if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		cout << "Invalid arguments! Input \"bin/gmonitor -h\" for help." << endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "gStore Monitor Data Tools(gmonitor)" << endl;
			cout << endl;
			cout << "Usage:\tbin/gmonitor -db [dbname] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database,\t\t the database name. Notice that the name can not end with "<< _db_suffix << endl;
			cout << endl;
			return 0;
		}
		else
		{
			// cout << "the command is not complete." << endl;
			cout << "Invalid arguments! Input \"bin/gmonitor -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		db_name = Util::getArgValue(argc, argv, "db", "database");
		if (db_name.empty())
		{
			cout << "You need to input the database name that you want to monitor. Input \"bin/gmonitor -h\" for help." << endl;
			return 0;
		}
		int len = db_name.length();
		if (db_name.length() > _len_suffix && db_name.substr(len - _len_suffix, _len_suffix) == _db_suffix)
		{
			cout<<"The database name can not end with " + _db_suffix + "! Input \"bin/gmonitor -h\" for help." << endl;
			return 0;
		}
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

		cout << "start loading the database......" << endl;
		Database _db(db_name);
		// _db.load();
		_db.loadDBInfoFile();
		cout << "finish load database info...."<< endl;

		sparql = "select ?x ?y where{<" + db_name + "> <built_by> ?x. <" + db_name + "> <built_time> ?y.}";
		ResultSet _rs;
		FILE *ofp = nullptr;
		system_db.query(sparql, _rs, ofp);
		string creator;
		string built_time;
		if (_rs.ansNum > 0)
		{
			creator = Util::clear_angle_brackets(_rs.answer[0][0]); // remove <>
			built_time = Util::replace_all(_rs.answer[0][1], "\"", "");
		}
		string db_path = _db_home + "/" + db_name + _db_suffix;
		string real_path = Util::getExactPath(db_path.c_str());
		unsigned diskUsed = 0;
		if (!real_path.empty()) {
			long long unsigned count_size_byte = Util::count_dir_size(real_path.c_str());
			// byte to MB
			diskUsed = count_size_byte>>20;
		}

		std::vector<std::string> header = {"name", "value"};
		std::vector<std::vector<std::string>> rows;
		rows.push_back({"database", db_name});
		rows.push_back({"creator", creator});
		rows.push_back({"built_time", built_time});
		rows.push_back({"triple_num", Util::int2string(_db.getTripleNum())});
		rows.push_back({"entity_num", Util::int2string(_db.getEntityNum())});
		rows.push_back({"literal_num", Util::int2string(_db.getLiteralNum())});
		rows.push_back({"subject_num", Util::int2string(_db.getSubNum())});
		rows.push_back({"predicate_num", Util::int2string(_db.getPreNum())});
		rows.push_back({"disk_used", to_string(diskUsed) + " MB"});
		Util::printConsole(header, rows);

		return 0;
	}

	/*if (argc == 1)
	{
		cout << "You need to input the database name that you want to show." << endl;
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
	}
	else
	{
		cout << "The number of parameters is not correct." << endl;
		return 0;
	}

	Database system_db("system");
	system_db.load();

	string sparql = "ASK WHERE{<" + db_name + "> <database_status> \"already_built\".}";
	ResultSet ask_rs;
	FILE* ask_ofp = stdout;
	int ret = system_db.query(sparql, ask_rs, ask_ofp);
	if (ask_rs.answer[0][0] == "false")
	{
		cout << "The database does not exist." << endl;
		return 0;
	}

	cout << "start loading the database......" << endl;
	Database _db(db_name);
	_db.load();
	cout << "finish loading" << endl;

	sparql = "select ?p ?o where{<" + db_name + "> ?p ?o.}";
	ResultSet _rs;
	FILE* ofp = stdout;
	ret = system_db.query(sparql, _rs, ofp);
	string creator;
	string built_time;
	for (int i = 0; i < _rs.ansNum; i++)
	{
		string p = _rs.answer[i][0];
		string o = _rs.answer[i][1];
		if(p == "<built_by>")
			creator = o.substr(1,o.length()-2);
		else if(p == "<built_time>")
			built_time = o;
	}
	unsigned triple_num = _db.getTripleNum();
	unsigned entity_num = _db.getEntityNum();
	unsigned literal_num = _db.getLiteralNum();
	unsigned subject_num = _db.getSubNum();
	unsigned predicate_num = _db.getPreNum();

	cout<<"\n========================================\n";
	string output = "database: " + db_name + "\ncreator: " + creator + "\nbuilt_time: " + built_time + "\n";
	output = output + "triple num: " + Util::int2string(triple_num) + "\nentity num: " + Util::int2string(entity_num) + "\nliteral num: " + Util::int2string(literal_num) + "\nsubject num: " + Util::int2string(subject_num) + "\npredicate num: " + Util::int2string(predicate_num)
			+ "\n========================================\n";
	cout<<output;
	return 0;*/
}