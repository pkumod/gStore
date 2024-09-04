/*=============================================================================
# Filename: POST-example.cpp
# Author: suxunbin
# Last Modified: 2021-07-20 20:21
# Description: a simple POST-example of C++ API
=============================================================================*/

#include <stdio.h> 
#include <iostream>
#include <string>
#include <fstream>
#include "GstoreConnector.h"

using namespace std;

std::string getValue(const std::string &key, const std::string &json)
{
    std::string::size_type pos1 = json.find(key);
    if (pos1 == std::string::npos)
    {
        return std::string();
    }
    std::string file = json.substr(pos1+ key.size() + 3, -1);
    std::string::size_type pos2 = file.find("\"");
    if (pos2 == std::string::npos)
    {
        return std::string();
    }
    return file.substr(0, pos2);
}

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp [port])
int main(int argc, char* argv[])
{
	std::string IP = "127.0.0.1";
	int Port = 20355;
	std::string httpType = "grpc";
	std::string username = "root";
	std::string password = "123456";
	std::string sparql = "select ?x where \
                         { \
                         ?x    <rdf:type>    <ub:UndergraduateStudent>. \
                         ?y    <ub:name> <Course1>. \
                         ?x    <ub:takesCourse>  ?y. \
                         ?z    <ub:teacherOf>    ?y. \
                         ?z    <ub:name> <FullProfessor1>. \
                         ?z    <ub:worksFor>    ?w. \
                         ?w    <ub:name>    <Department0>. \
                         }";
	std::string filename = "res.txt";

	// start a gc with given IP, Port, username and password
	GstoreConnector gc(IP, Port, httpType, username, password);

	// build a database with a RDF graph
	std::string res = gc.check("POST");
	cout << res << endl;

	// login server
	res = gc.login("POST");
	cout << res << endl;

	// query version
	res = gc.getCoreVersion("POST");
	cout << res << endl;

	res = gc.check("POST");
	cout << res << endl;

	res = gc.stat("POST");
	cout << res << endl;

	res = gc.build("lubm", "data/lubm/lubm.nt", "", "", "POST");
	cout << res << endl;

	// load the database
	res = gc.load("lubm", "0", "POST");
	cout << res << endl;

	// show all users
	res = gc.showuser("POST");
	cout << res << endl;

	// query
	res = gc.query("lubm", "json", sparql, "POST");
	cout << res << endl;

	// query and save the result in a file
	gc.fquery("lubm", "json", sparql, filename, "POST");

	// save the database if you have changed the database
	res = gc.checkpoint("lubm", "POST");
	cout << res << endl;

	// show information of the database
	res = gc.monitor("lubm", "POST");
	cout << res << endl;

	// show all databases
	res = gc.show("POST");
	cout << res << endl;

	// export the database
	res = gc.exportDB("lubm", "export/lubm/post", "POST");
	cout << res << endl;

	// export the database
	res = gc.exportDB("lubm", "export/lubm/get");
	cout << res << endl;

	// insert nt
	res = gc.batchInsert("lubm", "data/bbug/bbug.nt", "", "", "", "POST");
	cout << res << endl;

	std::string opt_id = getValue("opt_id", res);
	if (!opt_id.empty())
	{
		res = gc.checkOperationState(opt_id, "POST");
		cout << res << endl;
	}

	// remove nt
	res = gc.batchRemove("lubm", "data/bbug/bbug.nt", "", "", "POST");
	cout << res << endl;

	opt_id = getValue("opt_id", res);
	if (!opt_id.empty())
	{
		res = gc.checkOperationState(opt_id, "POST");
		cout << res << endl;
	}

	// user mange
	do
	{
		// show user
		res = gc.showuser("POST");
		cout << res << endl;

		// add usr
		res = gc.usermanage("1", "test", "123456", "POST");
		cout << res << endl;

		// set user privile
		res = gc.userprivilegemanage("1", "test", "1,2,3", "lubm", "POST");
		cout << res << endl;

		// clean user privile
		res = gc.userprivilegemanage("3", "test", "", "", "POST");
		cout << res << endl;

		// update user password
		res = gc.userpassword("test", "123456", "111111", "POST");
		cout << res << endl;

		// delete user
		res = gc.usermanage("2", "test", "", "POST");
		cout << res << endl;
	}while(0);
	
	// reason Mange
	do
	{
		std::string reason_info = "{\"rulename\":\"1\",\"description\":\"互相关注的人为朋友\",\"isenable\": 1,\"type\": 1,\"logic\": 1,\"conditions\": [{\"patterns\": [{\"subject\": \"?x \",	\"predicate\": \"<关注>\",\"object\":\"?y\"},{\"subject\": \"?y \",\"predicate\": \"<关注>\",\"object\": \"?x\"}],\"filters\": [],\"count_info\": {}}],\"return\": {\"source\": \"?x\",\"target\": \"?k\",\"label\": \"朋友\",\"value\": \"\"}}";
		gc.build("friend_reason", "scripts/test/parser_test/parser_d33.ttl", "", "", "POST");
		gc.load("friend_reason", "0", "POST");
		res = gc.show("friend_reason");
		cout << res << endl;

		// add reason
		res = gc.addReason("1", "friend_reason", reason_info);
		cout << res << endl;

		// list reason
		res = gc.reasonManage("2", "friend_reason");
		cout << res << endl;

		// compile reason
		res = gc.reasonManage("3", "friend_reason", "1");
		cout << res << endl;

		// excute reason
		res = gc.reasonManage("4", "friend_reason", "1");
		cout << res << endl;

		// disable reason
		res = gc.reasonManage("5", "friend_reason", "1");
		cout << res << endl;

		// show reason
		res = gc.reasonManage("6", "friend_reason", "1");
		cout << res << endl;

		// delete reason
		res = gc.reasonManage("7", "friend_reason", "1");
		cout << res << endl;

		gc.unload("friend_reason");
		gc.drop("friend_reason", false);
	}while(0);

	// unload the database
	res = gc.unload("lubm", "POST");
	cout << res << endl;

	// drop the database
	res = gc.drop("lubm", false, "POST"); //delete the database directly
	//res = gc.drop("lubm", true, "POST"); //leave a backup
	cout << res << endl;

	return 0;
}

