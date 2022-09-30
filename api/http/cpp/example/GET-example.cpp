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

	// build a database with a RDF graph
	std::string res = gc.build("lubm", "data/lubm/lubm.nt");
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

	// unload the database
	res = gc.unload("lubm");
	cout << res << endl;

	// drop the database
	res = gc.drop("lubm", false); //delete the database directly
	//res = gc.drop("lubm", true); //leave a backup
	cout << res << endl;

	return 0;
}

