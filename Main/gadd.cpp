#include "../Database/Database.h"
#include "../Util/Util.h"

using namespace std;

int
main(int argc, char * argv[])
{
	//chdir(dirname(argv[0]));
#ifdef DEBUG
	Util util;
#endif

	cout << "argc: " << argc << "\t";
	cout << "DB_store:" << argv[1] << "\t";
	cout << "insert file:" << argv[2] << endl;

	string db_folder = string(argv[1]);
	Database _db(db_folder);
	_db.load();
	cout << "finish loading" << endl;
	//_db.insert(argv[2]);
	//_db.remove(argv[2]);
	vector<int> vertices;
	vector<int> predicates;
	_db.insert(argv[2], vertices, predicates);

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

	return 0;
}
