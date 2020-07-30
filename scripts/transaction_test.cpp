#include "../Database/Txn_manager.h"
#include "../Util/Util.h"
#include "../Database/Database.h"
bool do_query(Database* db, string sparql)
{
	int ret_val;
	ResultSet rs;
	FILE* output = NULL;
	ret_val = db->query(sparql, rs, output);
	cout << ret_val << endl;
	if (ret_val < -1)   //non-update query
	{
		bool ret = (ret_val == -100);
		return ret;
	}
	return true;
}

void txn1(Txn_manager* txn_m)
{
	string query1 = "select ?x where { ?x <mingzhi> ?y}";
	string update1 = "insert data { <me> <mingzhi> <zhangzhe>}";
	string update2 = "delete where {?x <mingzhi> ?y}";
	txn_id_t id = txn_m->Begin();
	txn_m->Query(id, query1);
	txn_m->Query(id, update1);
	txn_m->Query(id, query1);
	txn_m->Query(id, update2);
	txn_m->Query(id, query1);
	txn_m->Commit(id);
	cout << "transaction #1 end" << endl;
}

void txn2(Txn_manager* txn_m)
{
	/*
	string query = "select ?x where { ?x <born> ?y}";
	string update1 = "insert data { <me> <born> <jjjj>}";
	string update2 = "delete where {?x <born> ?y}";
	*/
	string query, update1, update2;
	ifstream in;
	in.open("q1.sql", ios::in);
	getline(in, query);
	in.close();
	in.open("q2.sql", ios::in);
	getline(in, update1);
	in.close();
	in.open("q3.sql", ios::in);
	getline(in, update2);
	in.close();
	txn_id_t id = txn_m->Begin();
	txn_m->Query(id, query);
	txn_m->Query(id, update1);
	txn_m->Query(id, query);
	txn_m->Query(id, update2);
	txn_m->Query(id, query);
	txn_m->Abort(id);
	cout << "transaction #2 end" << endl;
}

void txn3(Txn_manager* txn_m)
{
	string remove = "delete where {?x ?y ?z}";
	txn_id_t id = txn_m->Begin();
	bool no_exception = true;
	no_exception = do_query(txn_m->GetDatabase(), remove);
	cout << no_exception << endl;
	if (!no_exception) txn_m->Abort(id);
	txn_m->Commit(id);
	cout << "transaction #3 end" << endl;
}

int main(int argc, char* argv[])
{
	Util util;
	cout << "hello world" << endl;
	cout << "argc: " << argc << "\t";
	cout << "DB_store:" << argv[1] << "\t";

	string db_folder = "lubm";

	Database _db(db_folder);
	_db.load();
	cout << "finish loading" << endl;
	Txn_manager txn_m(&_db, db_folder);

	/*string query1 = "select ?x where { ?x <born> ?y}";
	string update1 = "insert data { <me> <born> \" ";
	update1 += "jjjjj" ;
	update1 += " \"}";
	string update2 = "delete where {?x <born> ?y}";
	int ret_val;
	ResultSet rs;
	FILE* output = NULL;
	ret_val = _db.query(query1, rs, output);
	ret_val = _db.query(update1, rs, output);
	ret_val = _db.query(query1, rs, output);

	ret_val = _db.query(query1, rs, output);*/
	//ret_val = _db.query(update2, rs, output);

	//txn2(&txn_m);
	//thread t1(txn1, &txn_m);
	thread t2(txn2, &txn_m);
	//thread t3(txn3, &txn_m);
	//t1.join();
	t2.join();
	//t3.join();
	return 0;
}
