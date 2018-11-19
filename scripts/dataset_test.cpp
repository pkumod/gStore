/*=============================================================================
# Filename: dataset_test.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2018-11-04 20:11
# Description: used for size test
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"

using namespace std;

int main(int argc, char * argv[])
{
	Util util;
	Database* db;
	string db_name = string(argv[1]);
	string db_path = string(argv[2]);
	string sql_path = string(argv[3]);
	string ans_path = string(argv[4]);
	priority_queue<string> sparql;
	priority_queue<string> answer;
	DIR* dir;
	struct dirent * ptr;
	dir = opendir(argv[3]);
	while ((ptr = readdir(dir)) != NULL)
	{
		int nameLen = strlen(ptr->d_name);
		if (nameLen > 3 && string(ptr->d_name).substr(nameLen - 4, nameLen) == ".sql")
		{
			string s = sql_path + string(ptr->d_name);
			sparql.push(s);
		}
	}
	closedir(dir);
	dir = opendir(argv[4]);
	while ((ptr = readdir(dir)) != NULL)
	{
		int nameLen = strlen(ptr->d_name);
		if (nameLen > 3 && string(ptr->d_name).substr(nameLen - 4, nameLen) == ".txt")
		{
			string s = ans_path + string(ptr->d_name);
			answer.push(s);
		}
	}
	closedir(dir);
	if (!boost::filesystem::exists("./" + db_name + ".db/success.txt"))
	{
		db = new Database(db_name);
		bool flag = db->build(db_path);
		if (flag)
		{
			string msg = db_name + ".db is built done.";
			cerr << msg << endl;
			ofstream f;
			f.open("./" + db_name + ".db/success.txt");
			f.close();
			delete db;
			db = NULL;
		}
		else //if fails, drop databsase and return
		{
			string msg = db_name + ".db is built failed.";
			cerr << msg << endl;
			string cmd = "rm -r " + db_name + ".db";
			system(cmd.c_str());
			delete db;
			db = NULL;
			return -1;
		}
	}

	db = new Database(db_name);
	db->load();
	int sql_num = sparql.size();
	for (int i = 0; i < sql_num; i++)
	{
		string query = sparql.top();
		string result = answer.top();
		query = Util::getQueryFromFile(query.c_str());
		ResultSet _rs;
		FILE* ofp = fopen("ans.txt", "w");
		int ret = db->query(query, _rs, ofp);
		fclose(ofp);
		string cmd = "cat ans.txt |sort -u > ans_sorted.txt";
		system(cmd.c_str());
		cmd = "rm ans.txt";
		system(cmd.c_str());
		cmd = "diff ans_sorted.txt " + result + " > diff.txt";
		system(cmd.c_str());
		fstream file;
		file.open("diff.txt", ios::in);
		int ch = file.get();
		if (!file.eof())
		{
			string msg = "query " + sparql.top() + " in " + db_name + ".db exists errors";
			cerr << msg << endl;
			string cmd = "rm ans_sorted.txt diff.txt";
			system(cmd.c_str());
			sparql.pop();
			answer.pop();
			delete db;
			db = NULL;
			return -1;
		}
		else
		{
			string msg = "query " + sparql.top() + " in " + db_name + ".db has passed";
			cerr << msg << endl;
			string cmd = "rm ans_sorted.txt diff.txt";
			system(cmd.c_str());
			sparql.pop();
			answer.pop();
		}
	}
	delete db;
	db = NULL;
	string cmd = "rm -r " + db_name + ".db";
	system(cmd.c_str());
	string msg = db_name + ".db test passed";
	cerr << msg << endl;
	return 0;
}
