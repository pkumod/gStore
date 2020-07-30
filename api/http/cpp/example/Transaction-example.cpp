#include <stdio.h> 
#include <iostream>
#include <string>
#include <fstream>
#include "client.h"

using namespace std;

int main(int argc, char* argv[])
{
	const string func = "GET";
	std::string IP = "127.0.0.1";
	int Port = 9000;
	std::string username = "root";
	std::string password = "123456";
	string query = "select ?x ?y where{<gStore> ?x ?y}";
	string insert = "insert data {<gStore> <test> <commit>.<gStore> <test> <abort>.<gStore> <test> <begin>.<gStore> <test> <all>.}";
	string remove = "delete where {<gStore> ?x ?y}";
	string outdegree_query1 = "select (count(?y) as ?yCount) where{<gStore> ?x ?y}";
	string indegree_query1 = "select (count(?x) as ?yCount) where{?x ?y <gStore> }";
	string outdegree_query2 = "select (count(?y) as ?yCount) where{<all> ?x ?y}";
	string indegree_query2 = "select (count(?x) as ?yCount) where{?x ?y <all>  }";
	string remove_triple = "delete data {<gStore> <test> <all>}";

	GstoreConnector gc(IP, Port, username, password);


	std::string res = gc.build("lubm", "data/lubm/lubm.nt", func);
	cout << res << endl;

	// load the database
	res = gc.load("lubm", func);
	cout << res << endl;

	//test 1
	res = gc.begin("lubm", func);
	cout << res << endl;

	res = gc.query("lubm", "json", query);
	cout << res << endl;

	res = gc.query("lubm", "json", insert);
	cout << res << endl;

	res = gc.query("lubm", "json", query);
	cout << res << endl;

	res = gc.commit("lubm", "", func);
	cout << res << endl;

	res = gc.query("lubm", "json", remove);
	cout << res << endl;

	//test2
	res = gc.begin("lubm", func);
	cout << res << endl;

	res = gc.query("lubm", "json", query);
	cout << res << endl;

	res = gc.query("lubm", "json", insert);
	cout << res << endl;

	res = gc.query("lubm", "json", query);
	cout << res << endl;

	res = gc.rollback("lubm", "", func);
	cout << res << endl;

	res = gc.query("lubm", "json", remove);
	cout << res << endl;

	//test4
	res = gc.query("lubm", "json", insert);
	cout << res << endl;

	res = gc.query("lubm", "json", query);
	cout << res << endl;

	res = gc.query("lubm", "json", outdegree_query1);
	cout << res << endl;

	res = gc.query("lubm", "json", indegree_query1);
	cout << res << endl;

	res = gc.query("lubm", "json", outdegree_query2);
	cout << res << endl;

	res = gc.query("lubm", "json", indegree_query2);
	cout << res << endl;

	res = gc.query("lubm", "json", remove_triple);
	cout << res << endl;

	res = gc.query("lubm", "json", outdegree_query1);
	cout << res << endl;

	res = gc.query("lubm", "json", indegree_query1);
	cout << res << endl;

	res = gc.query("lubm", "json", outdegree_query2);
	cout << res << endl;

	res = gc.query("lubm", "json", indegree_query2);
	cout << res << endl;

	res = gc.query("lubm", "json", remove);
	cout << res << endl;

	res = gc.query("lubm", "json", outdegree_query1);
	cout << res << endl;

	// unload the database
	res = gc.unload("lubm");
	cout << res << endl;

	// drop the database
	res = gc.drop("lubm", false); //delete the database directly
	//res = gc.drop("lubm", true); //leave a backup
	cout << res << endl;

	//transaction log
	res = gc.getTransLog(func);
	cout << res << endl;

	return 0;
}
