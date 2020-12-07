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

void print_ans(int ret, string &res)
{
	if(ret >= 0)
	{
		cout << "update nums: " << ret << endl;
	}
	else if(ret == -100)
	{
		cout << "query success" << endl;
		cout << res << endl;
	}
	else
	{
		cout << "query failed!" << endl;
	}
}
void txn1(Txn_manager* txn_m)
{
	string query1 = "select ?x where { ?x <mingzhi> ?y}";
	string update1 = "insert data { <me> <mingzhi> <zhangzhe>}";
	string update3 = "insert data { <you> <mingzhi> <SBBBBB>}";
	string update4 = "delete data { <you> <mingzhi> <SBBBBB>}";
	string update2 = "delete where {?x <mingzhi> ?y}";
	string res;
	txn_id_t id = txn_m->Begin();
	txn_m->Query(id, update2, res);
	txn_m->Query(id, query1, res);
	txn_m->Query(id, update1, res);
	txn_m->Query(id, update3, res);
	txn_m->Query(id, query1, res);
	txn_m->Query(id, update4, res);
	txn_m->Query(id, query1, res);
	txn_m->Commit(id);
	txn_m->print_txn_dataset(id);
	cout << "transaction #1 end" << endl;
}

void txn2(Txn_manager* txn_m)
{
	/*
	string query = "select ?x where { ?x <born> ?y}";
	string update1 = "insert data { <me> <born> <jjjj>}";
	string update2 = "delete where {?x <born> ?y}";
	*/
	string query1 = "select ?x where { ?x <mingzhi> ?y}";
	string update1 = "insert data { <me> <mingzhi> <zhangzhe>}";
	string update3 = "insert data { <you> <mingzhi> <SBBBBB>}";
	string update4 = "delete data { <you> <mingzhi> <SBBBBB>}";
	string update2 = "delete where {?x <mingzhi> ?y}";
	string res;
	txn_id_t id = txn_m->Begin();
	int ret;
	ret = txn_m->Query(id, query1, res);
	print_ans(ret, res);
	ret = txn_m->Query(id, update1,res);
	print_ans(ret, res);
	ret = txn_m->Query(id, query1, res);
	print_ans(ret, res);
	ret = txn_m->Query(id, update2, res);
	print_ans(ret, res);
	ret = txn_m->Query(id, query1, res);
	print_ans(ret, res);
	ret = txn_m->Rollback(id);
	cout << "transaction #2 end" << endl;
}

void txn3(Txn_manager* txn_m)
{
	string remove = "delete where {?x ?y ?z}";
	txn_id_t id = txn_m->Begin();
	bool no_exception = true;
	no_exception = do_query(txn_m->GetDatabase(), remove);
	cout << no_exception << endl;
	if (!no_exception) txn_m->Rollback(id);
	txn_m->Commit(id);
	cout << "transaction #3 end" << endl;
}

void txn4(Txn_manager* txn_m)
{
	string query = "select ?x ?y where{<gStore> ?x ?y}";
	string insert = "insert data {<gStore> <test> <commit>.<gStore> <test> <abort>.<gStore> <test> <begin>.<gStore> <test> <all>.}";
	string remove = "delete where {<gStore> ?x ?y}";
	txn_id_t id = txn_m->Begin();
	string res;
	txn_m->Query(id, query,res);
	txn_m->Query(id, insert,res);
	txn_m->Query(id, query, res);
	txn_m->Query(id, remove, res);
	txn_m->Query(id, query, res);
	txn_m->print_txn_dataset(id);
	txn_m->Commit(id);

}

void test(Txn_manager* txn_m)
{
	string inc1 = "insert data {<V1> <R1> \"0\".}";
	string inc2 = "insert data {<V1> <R1> \"10\".}";
	string inc3 = "insert data {<V2> <R1> \"20\".}";
	string query = "select ?x ?y where{?x <R1> ?y}";
	string res;
	txn_id_t id = txn_m->Begin();
	txn_m->print_txn_dataset(id);
	txn_m->Query(id, inc1, res);
	txn_m->print_txn_dataset(id);
	txn_m->Query(id, query, res);
	txn_m->Query(id, inc2, res);
	txn_m->Query(id, query, res);
	txn_m->Query(id, inc3, res);
	txn_m->Query(id, query, res);
	//txn_m->print_txn_dataset(id);
	txn_m->Commit(id);

}


void test2(Txn_manager* txn_m)
{
	string query = "select ?v {<V1> <R1> ?v.}";
	string delete1  = "delete data { <V1> <R1> \"0\"^^<http://www.w3.org/2001/XMLSchema#integer> }";
	string insert1 = "insert data { <V1> <R1> \"1\"^^<http://www.w3.org/2001/XMLSchema#integer> }";
	string delete2 = "delete data { <V1> <R1> \"1\"^^<http://www.w3.org/2001/XMLSchema#integer> }";
	string insert2 = "insert data { <V1> <R1> \"2\"^^<http://www.w3.org/2001/XMLSchema#integer> }";
	string res;
	txn_id_t id = txn_m->Begin();
	txn_m->print_txn_dataset(id);
	txn_m->Query(id, query, res);
	txn_m->print_txn_dataset(id);
	txn_m->Query(id, delete1, res);
	txn_m->print_txn_dataset(id);
	txn_m->Query(id, insert1, res);
	txn_m->Query(id, query, res);
	txn_m->Query(id, delete2, res);
	txn_m->Query(id, insert2, res);
	txn_m->Query(id, query, res);
	txn_m->print_txn_dataset(id);
	txn_m->Commit(id);
}
int main(int argc, char* argv[])
{
	Util util;

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
	//thread t2(txn2, &txn_m);
	//thread t3(txn3, &txn_m);
	//t1.join();
	//t2.join();
	//t3.join();
	test2(&txn_m);
	return 0;
}
