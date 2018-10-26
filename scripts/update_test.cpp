/*=============================================================================
# Filename: update_test.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2018-10-25 21:25
# Description: used to test the correctness of update triples
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

//triple information
class triple{
public:
	string subject;
	string predicate;
	string object;

	triple(int s, int p, int o){
		subject = "<s" + Util::int2string(s) + ">";
		predicate = "<p" + Util::int2string(p) + ">";
		object = "<o" + Util::int2string(o) + ">";
	}
	triple(string s, string p, string o){
		subject = s;
		predicate = p;
		object = o;
	}
	inline bool operator<(const triple& t) const
	{
		if (this->subject < t.subject)
			return true;
		else if (this->subject > t.subject)
			return false;
		else
		{
			if (this->predicate < t.predicate)
				return true;
			else if (this->predicate > t.predicate)
				return false;
			else
			{
				if (this->object < t.object)
					return true;
				else
					return false;
			}
		}
	}
	inline bool operator==(const triple& t) const
	{
		if (this->subject != t.subject)
			return false;
		else
		{
			if (this->predicate != t.predicate)
				return false;
			else
			{
				if (this->object != t.object)
					return false;
				else
					return true;
			}
		}
	}
};

std::set<triple> update_triples;
std::set<triple> db_triples;
Database* db;

int main(int argc, char * argv[])
{
	//build update_test.db
	Util util;
	int test_group_num = 10000;
	if(argc > 1)
	{
		test_group_num = atoi(argv[1]);
	}
	int test_group_size = 5;
	int test_value_region = 10;

	string db_name = "update_test";
	string db_path = "data/update_test.nt";
	db = new Database(db_name);
	bool flag = db->build(db_path);
	if (flag)
	{
		cerr << "update_test.db is built done." << endl;
		ofstream f;
		f.open("./" + db_name + ".db/success.txt");
		f.close();
	}
	else //if fails, drop update_test.db and return
	{
		cerr << "update_test.db is built failed." << endl;
		string cmd = "rm -r " + db_name + ".db";
		system(cmd.c_str());
		delete db;
		db = NULL;
		return 0;
	}

	//load update_test.db
	delete db;
	db = new Database(db_name);
	db->load();

	//update triples test
	srand((unsigned)time(NULL));
	update_triples.clear();
	triple temp(0, 0, 0);
	update_triples.insert(temp);
	for (int i = 0; i < test_group_num; i++)
	{
		int a = rand() % test_group_size + 1;
		int b = rand() % test_group_size + 1;
		for (int j = 0; j < a; j++)
		{
			int s = rand() % test_value_region;
			int p = rand() % test_value_region;
			int o = rand() % test_value_region;
			triple t(s, p, o);
			update_triples.insert(t);
			string query = "INSERT DATA{" + t.subject + " " + t.predicate + " " + t.object + ".}";
			ResultSet _rs;
			FILE* ofp = stdout;
			int ret = db->query(query, _rs, ofp);
		}
		for (int j = 0; j < b; j++)
		{
			int s = rand() % test_value_region;
			int p = rand() % test_value_region;
			int o = rand() % test_value_region;
			triple t(s, p, o);
			std::set<triple>::iterator it = update_triples.find(t);
			if (it != update_triples.end())
				update_triples.erase(it);
			string query = "DELETE DATA{" + t.subject + " " + t.predicate + " " + t.object + ".}";
			ResultSet _rs;
			FILE* ofp = stdout;
			int ret = db->query(query, _rs, ofp);
		}
		db_triples.clear();
		string query = "select ?s ?p ?o where{?s ?p ?o.}";
		ResultSet _rs;
		FILE* ofp = NULL;
		int ret = db->query(query, _rs, ofp);
		for (int i = 0; i < _rs.ansNum; i++)
		{
			string s = _rs.answer[i][0];
			string p = _rs.answer[i][1];
			string o = _rs.answer[i][2];
			triple t(s, p, o);
			db_triples.insert(t);
		}
		if (update_triples.size() != db_triples.size())
		{
			cerr << "Update triples exist errors." << endl;
			delete db;
			db = NULL;
			string cmd = "rm -r " + db_name + ".db";
			system(cmd.c_str());
			return 0;
		}
		std::set<triple>::iterator it1;
		std::set<triple>::iterator it2;
		for (it1 = update_triples.begin(), it2 = db_triples.begin(); it1 != update_triples.end(); it1++, it2++)
		{
			if (*it1 == *it2)
				continue;
			else
			{
				cerr << "Update triples exist errors." << endl;
				delete db;
				db = NULL;
				string cmd = "rm -r " + db_name + ".db";
				system(cmd.c_str());
				return 0;
			}
		}
	}

	delete db;
	db = NULL;
	string cmd = "rm -r " + db_name + ".db";
	system(cmd.c_str());
	cerr << "Test passed!" << endl;

	return 0;
}

