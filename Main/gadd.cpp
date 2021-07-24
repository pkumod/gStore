#include "../Database/Database.h"
#include "../Util/Util.h"

using namespace std;

int
main(int argc, char * argv[])
{
	//chdir(dirname(argv[0]));
//#ifdef DEBUG
	Util util;
//#endif

	cout << "argc: " << argc << "\t";
	cout << "DB_store:" << argv[1] << "\t";
	cout << "insert file:" << argv[2] << endl;

	string db_folder = string(argv[1]);
	int len = db_folder.length();

	if(db_folder.substr(len-3, 3) == ".db")
	{
		cout<<"your database can not end with .db"<<endl;
		return -1;
	}

	Database *_db = new Database(db_folder);
	_db->load();
	cout << "finish loading" << endl;
	//_db.insert(argv[2]);
	//_db.remove(argv[2]);
	long tv_begin = Util::get_cur_time();
	_db->insert(argv[2], false, nullptr);
	long tv_end = Util::get_cur_time();
	cout << "after insert, used " << (tv_end - tv_begin) << " ms" << endl;
	//string query = string(argv[2]);
	//query = Util::getQueryFromFile(query.c_str());
	//if (query.empty())
	//{
	//return 0;
	//}
	//printf("query is:\n%s\n\n", query.c_str());

	//ResultSet _rs;
	//_db.query(query, _rs, stdout);

	//TODO:to test insert, delete and modify
	//read from file or just several triples written here
	delete _db;
	long tv_end1 = Util::get_cur_time();
	cout << "persistence on disk" << (tv_end1 - tv_end) << " ms" << endl;
	return 0;
}
