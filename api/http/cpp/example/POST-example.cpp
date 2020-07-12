/*=============================================================================
# Filename: POST-example.cpp
# Author: suxunbin
# Last Modified: 2019-5-17 20:21
# Description: a simple POST-example of C++ API
=============================================================================*/

#include <stdio.h>
#include <iostream>
#include <string>
#include <fstream>
#include "client.h"

using namespace std;

// before you run this example, make sure that you have started up ghttp service (using bin/ghttp port)
int main(int argc, char* argv[])
{
  std::string IP = "127.0.0.1";
  int Port = 9000;
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
  GstoreConnector gc(IP, Port, username, password);

  // build a database with a RDF graph
  std::string res = gc.build("lubm", "data/lubm/lubm.nt", "POST");
  cout << res << endl;

  // load the database
  res = gc.load("lubm", "POST");
  cout << res << endl;

  // to add, delete a user or modify the privilege of a user, operation must be done by the root user
  //res = gc.user("add_user", "user1", "111111", "POST");
  //cout << res << endl;

  // show all users
  res = gc.showUser("POST");
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
  res = gc.exportDB("lubm", "export/lubm/lubm_post.nt", "POST");
  cout << res << endl;

  // unload the database
  res = gc.unload("lubm", "POST");
  cout << res << endl;

  // drop the database
  res = gc.drop("lubm", false, "POST"); //delete the database directly
  //res = gc.drop("lubm", true, "POST"); //leave a backup
  cout << res << endl;

  // get CoreVersion and APIVersion
  res = gc.getCoreVersion("POST");
  cout << res << endl;
  res = gc.getAPIVersion("POST");
  cout << res << endl;

  return 0;
}
