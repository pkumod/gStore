/**
* @file  CppAPIExample.cpp
* @author  suxunbin
* @date  13-AUG-2021
* @brief  a simple example of a socket client
*/

#include "Client.h"
#include <string>
#include <iostream>

/**
* @note  Before running this example, you must start up the GStore server at first (use command bin/gserver in the root directory).
*/
int main(int argc, char* argv[])
{
	std::string ip = "127.0.0.1";
	int port = 9000;
	std::string username = "root";
	std::string password = "123456";
	std::string db_name = "LUBM10";
	std::string rdf_path = "data/lubm/lubm.nt";
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
	std::string cmd;
	std::string recv_msg;

	/**
	* @brief  Initialize the GstoreConnector with the server's IP address and port.
	*/
	GstoreConnector gc(ip, port);

	/**
	* @brief  Connect the server.
	*/
	gc.connect();

	/**
	* @brief  Login the server using the root user.
	*/
	cmd = "{\"op\":\"login\", \"username\":\"" + username + "\", \"password\":\"" + password + "\"}";
	gc.send(cmd);
	gc.recv(recv_msg);

	/**
	* @brief  Build a database.
	*/
	cmd = "{\"op\":\"build\", \"db_name\":\"" + db_name + "\", \"rdf_path\":\"" + rdf_path + "\"}";
	gc.send(cmd);
	gc.recv(recv_msg);

	/**
	* @brief  Load a database.
	*/
	cmd = "{\"op\":\"load\", \"db_name\":\"" + db_name + "\"}";
	gc.send(cmd);
	gc.recv(recv_msg);

	/**
	* @brief  Execute a sparql query on a database.
	*/
	cmd = "{\"op\":\"query\", \"db_name\":\"" + db_name + "\", \"sparql\":\"" + sparql + "\"}";
	gc.send(cmd);
	gc.recv(recv_msg);

	/**
	* @brief  show all databases.
	*/
	cmd = "{\"op\":\"show\"}";
	gc.send(cmd);
	gc.recv(recv_msg);

	/**
	* @brief  Unload a database.
	*/
	cmd = "{\"op\":\"unload\", \"db_name\":\"" + db_name + "\"}";
	gc.send(cmd);
	gc.recv(recv_msg);

	/**
	* @brief  Drop a database.
	*/
	cmd = "{\"op\":\"drop\", \"db_name\":\"" + db_name + "\"}";
	gc.send(cmd);
	gc.recv(recv_msg);

	/**
	* @brief  Close the connection.
	*/
	gc.disconnect();

	return 0;
}

