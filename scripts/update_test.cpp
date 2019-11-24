/*=============================================================================
# Filename: update_test.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2018-11-02 11:13
# Description: used to test the correctness of update triples
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

bool isNum(char *str)
{
	for (int i = 0; i < strlen(str); i++)
	{
		int tmp = (int)(str[i]);
		if (tmp < 48 || tmp > 57)
			return false;
	}
	return true;
}

//triple information
class triple{
public:
	string subject;
	string predicate;
	string object;

	triple(int s, int p, int o){
		subject = "<s" + Util::int2string(s) + ">";
		predicate = "<p" + Util::int2string(p) + ">";
		if (rand()%2)
			object = "<o" + Util::int2string(o) + ">";
		else
			object = "\"o" + Util::int2string(o) + "\"";
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

void print()
{
	cerr << "update_triples:" << endl;
	std::set<triple>::iterator it1;
	for (it1 = update_triples.begin(); it1 != update_triples.end(); it1++)
		cerr << it1->subject << " " << it1->predicate << "" << it1->object << endl;
	cerr << "db_triples:" << endl;
	std::set<triple>::iterator it2;
	for (it2 = db_triples.begin(); it2 != db_triples.end(); it2++)
		cerr << it2->subject << " " << it2->predicate << "" << it2->object << endl;
}

int main(int argc, char * argv[])
{
	Util util;
	int test_type = 0;
	int test_group_num = 10000;
	int test_group_size = 5;
	int test_value_region = 10;
	string db_name = "update_test";
	string db_path = "data/update_test.nt";
	if(argc == 2)
	{
		string s = argv[1];
		if (isNum(argv[1]))
			test_group_num = atoi(argv[1]);
		else if (s == "DBpedia170M")
		{
			test_type = 1;
			test_group_num = 10;
			test_group_size = 10000;
			test_value_region = 10000;
			db_name = "DBpedia170M";
			db_path = "/home/data/DBpedia/database/dbpedia170M.nt";
		}
		else
		{
			cout << "wrong format of parameters" << endl;
			return 0;
		}
	}
	else if (argc == 3)
	{
		if (isNum(argv[1]))
			test_group_num = atoi(argv[1]);
		else
		{
			cout << "wrong format of test_group_num" << endl;
			return 0;
		}
		if (isNum(argv[2]))
			test_group_size = atoi(argv[2]);
		else
		{
			cout << "wrong format of test_group_size" << endl;
			return 0;
		}
	}
	else if (argc == 4)
	{
		if (isNum(argv[1]))
			test_group_num = atoi(argv[1]);
		else
		{
			cout << "wrong format of test_group_num" << endl;
			return 0;
		}
		if (isNum(argv[2]))
			test_group_size = atoi(argv[2]);
		else
		{
			cout << "wrong format of test_group_size" << endl;
			return 0;
		}
		if (isNum(argv[3]))
			test_value_region = atoi(argv[3]);
		else
		{
			cout << "wrong format of test_value_region" << endl;
			return 0;
		}
	}
	else if (argc > 4)
	{
		cout << "The number of parameters is not correct." << endl;
		return 0;
	}

	//build database
	db = new Database(db_name);
	bool flag = db->build(db_path);
	if (flag)
	{
		string msg = db_name + ".db is built done.";
		cerr << msg << endl;
		ofstream f;
		f.open("./" + db_name + ".db/success.txt");
		f.close();
	}
	else //if fails, drop database and return
	{
		string msg = db_name + ".db is built failed.";
		cerr << msg << endl;
		string cmd = "rm -r " + db_name + ".db";
		system(cmd.c_str());
		delete db;
		db = NULL;
		return 0;
	}

	//load database
	srand((unsigned)time(NULL));
	delete db;
	db = new Database(db_name);
	db->load();

	//update triples test
	if (test_type == 0)
	{
		update_triples.clear();
		triple temp("<s0>", "<p0>", "<o0>");
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
			//print();
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
	}
	else
	{
		int triple_num = db->getTripleNum();
		string *update;
		for (int i = 0; i < test_group_num; i++)
		{
			//random extract update_num triples
			int update_num = rand() % test_group_size + 1;
			update = new string[update_num];
			ifstream file;
			file.open(db_path, ios::in);
			int update_line = rand() % test_value_region + 1;
			int line_num = 1;
			for (int j = 0; j < update_num; j++)
			{
				string s;
				while (getline(file, s))
				{
					if (line_num != update_line)
						line_num++;
					else
					{
						update[j] = s;
						update_line = update_line + rand() % test_value_region + 1;
						line_num++;
						break;
					}
				}
			}
			file.close();

			//check the correctness of deleting and inserting triples
			string query = "DELETE DATA{";
			for (int j = 0; j < update_num; j++)
				query = query + update[j];
			query = query + "}";
			ResultSet delete_rs;
			FILE* delete_ofp = stdout;
			int ret = db->query(query, delete_rs, delete_ofp);
			if (triple_num != (update_num + db->getTripleNum()))
			{
				cerr << "Delete triples exist errors." << endl;
				delete update;
				update = NULL;
				delete db;
				db = NULL;
				string cmd = "rm -r " + db_name + ".db";
				system(cmd.c_str());
				return 0;
			}
			int temp_num = triple_num - update_num;
			for (int j = 0; j < update_num; j++)
			{
				query = "ASK WHERE{" + update[j] + "}";
				ResultSet ask_rs;
				FILE* ask_ofp = stdout;
				int ret = db->query(query, ask_rs, ask_ofp);
				if (ask_rs.answer[0][0] == "true")
				{
					cerr << "Delete triples exist errors." << endl;
					delete update;
					update = NULL;
					delete db;
					db = NULL;
					string cmd = "rm -r " + db_name + ".db";
					system(cmd.c_str());
					return 0;
				}
			}
			query = "INSERT DATA{";
			for (int j = 0; j < update_num; j++)
				query = query + update[j];
			query = query + "}";
			ResultSet insert_rs;
			FILE* insert_ofp = stdout;
			ret = db->query(query, insert_rs, insert_ofp);
			if (temp_num != (db->getTripleNum() - update_num))
			{
				cerr << "Insert triples exist errors." << endl;
				delete update;
				update = NULL;
				delete db;
				db = NULL;
				string cmd = "rm -r " + db_name + ".db";
				system(cmd.c_str());
				return 0;
			}
			query = "ASK WHERE{";
			for (int j = 0; j < update_num; j++)
				query = query + update[j];
			query = query + "}";
			ResultSet ask_rs;
			FILE* ask_ofp = stdout;
			ret = db->query(query, ask_rs, ask_ofp);
			if (ask_rs.answer[0][0] == "false")
			{
				cerr << "Insert triples exist errors." << endl;
				delete update;
				update = NULL;
				delete db;
				db = NULL;
				string cmd = "rm -r " + db_name + ".db";
				system(cmd.c_str());
				return 0;
			}
			delete[] update;
			update = NULL;
		}
		delete db;
		db = NULL;
		string cmd = "rm -r " + db_name + ".db";
		system(cmd.c_str());
	}
	cerr << "Test passed!" << endl;
	return 0;
}