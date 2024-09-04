/*=============================================================================
# Filename: GET-example.cpp
# Author: suxunbin
# Last Modified: 2021-07-20 20:51
# Description: a simple GET-example of C++ API
=============================================================================*/

#include <stdio.h> 
#include <iostream>
#include <string>
#include <fstream>
#include "GstoreConnector.h"

using namespace std;

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp [port])
// "GET" is a default parameter that can be omitted

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

int main(int argc, char* argv[])
{
	std::string IP = "127.0.0.1";
	int Port = 9000;
	std::string httpType = "ghttp";
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

	// check server
	std::string res = gc.check();
	cout << res << endl;

	// login server
	res = gc.login();
	cout << res << endl;

	// query version
	res = gc.getCoreVersion();
	cout << res << endl;

	res = gc.check();
	cout << res << endl;

	res = gc.stat();
	cout << res << endl;

	// build a database with a RDF graph
	res = gc.build("lubm", "data/lubm/lubm.nt");
	cout << res << endl;

	// load the database
	res = gc.load("lubm", "0");
	cout << res << endl;

	// show all users
	res = gc.showuser();
	cout << res << endl;

	// query
	res = gc.query("lubm", "json", sparql);
	cout << res << endl;

	// query and save the result in a file
	gc.fquery("lubm", "json", sparql, filename);

	// save the database if you have changed the database
	res = gc.checkpoint("lubm");
	cout << res << endl;

	// show information of the database
	res = gc.monitor("lubm");
	cout << res << endl;

	// show all databases
	res = gc.show();
	cout << res << endl;

	// export the database
	res = gc.exportDB("lubm", "export/lubm/get");
	cout << res << endl;

	// insert nt
	res = gc.batchInsert("lubm", "data/bbug/bbug.nt");
	cout << res << endl;

	std::string opt_id = getValue("opt_id", res);
	if (!opt_id.empty())
	{
		res = gc.checkOperationState(opt_id);
		cout << res << endl;
	}

	// remove nt
	res = gc.batchRemove("lubm", "data/bbug/bbug.nt");
	cout << res << endl;

	opt_id = getValue("opt_id", res);
	if (!opt_id.empty())
	{
		res = gc.checkOperationState(opt_id);
		cout << res << endl;
	}

	// user mange
	do
	{
		// show user
		res = gc.showuser();
		cout << res << endl;

		// add usr
		res = gc.usermanage("1", "test", "123456");
		cout << res << endl;

		// set user privile
		res = gc.userprivilegemanage("1", "test", "1,2,3", "lubm");
		cout << res << endl;

		// clean user privile
		res = gc.userprivilegemanage("3", "test");
		cout << res << endl;

		// update user password
		res = gc.userpassword("test", "123456", "111111");
		cout << res << endl;

		// delete user
		res = gc.usermanage("2", "test");
		cout << res << endl;
	}while(0);
	
	// reason Mange not support

	// unload the database
	res = gc.unload("lubm");
	cout << res << endl;

	// drop the database
	res = gc.drop("lubm", false); //delete the database directly
	//res = gc.drop("lubm", true); //leave a backup
	cout << res << endl;

	return 0;
}

