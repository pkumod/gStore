#include "../api/http/cpp/src/GstoreConnector.h"
#include <iostream>
#include "../Util/Util.h"

using namespace std;
using namespace rapidjson;

//gtxn [db_name] [isolationlevel]
typedef unsigned long long txn_id_t;

string get_TID(string res)
{
	StringStream is(res.c_str());
    Document d;
    d.ParseStream(is);
	if(d.HasMember("TID"))
	{		
		string TID_s = d["TID"].GetString();
        return TID_s;
	}
	return "-1";
}

enum class status {Begin = 1, Running, Commit, Rollback};
int main(int argc, char * argv[])
{
	string db_name;
	string isolevel;
	string TID;
	
	if(argc < 2)
	{
		cout << "the usage of gtxn: " << endl;
		cout << "bin/gtxn your_database_name isolationlevel(default=2) " << endl ;
		cout << "isolationlevel" << endl;
		cout << "READ_COMMITTED = 1" << endl;
		cout << "SNAPSHOT = 2" << endl;
		cout << "SERIALIZABLE = 3" << endl;
		return -1;
	}
	if(argc == 2)
	{
		db_name = argv[1];
		isolevel = 3;
	}
	else
	{
		db_name = argv[1];
		isolevel = argv[2];
	}
	
	const string func = "GET";
	std::string IP = "127.0.0.1";
	int Port = 9000;
	std::string httpType = "ghttp";
	std::string username = "root";
	std::string password = "123456";
	
	GstoreConnector gc(IP, Port, httpType, username, password);
	status s = status::Begin;
	string cmd;
	string res; 
	res = gc.load(db_name, func);
	cout << res << endl;

	while(1)
	{
		cout << "please enter a comand" << endl;
		getline(cin, cmd);
		if(cmd == "Begin")
		{
			if(s != status::Begin)
			{
				cout << "Wrong: Transaction is running!"  << endl;
				continue;	
			}
			else
			{
				cout << "isolevel" << isolevel << endl;
				res = gc.begin(db_name, isolevel, func);
				cout << res << endl;
				TID = get_TID(res);
				//cout << "Transaction Begin." << endl;
				s = status::Running;
			}
		}
		else if(cmd == "Commit")
		{
			if(s != status::Running)
			{
				cout << "Transaction is not running! CANNOT Commit!" << endl;
				continue;
			}
			else{
				res = gc.commit(db_name, TID, func);
				cout << res << endl;
				cout << "Trasaction Commit." << endl;
				s = status::Commit;
				break;
			}
		}
		else if(cmd == "Rollback")
		{
			if(s != status::Running)
			{
				cout << "Transaction is not running! CANNOT Rollback!" << endl;
				continue;
			}
			else
			{
				res = gc.rollback(db_name, TID, func);
				cout << res << endl;
				cout << "Transaction Rollback." << endl;
				s = status::Rollback;
				break;
			}
		}
		else
		{
			cout << cmd << endl;
			res = gc.tquery(db_name, TID, cmd,  func);
			cout << res << endl;
		}
	}
	cout << "gtxn exit............." << endl;
	return 0;
}