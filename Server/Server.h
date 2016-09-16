/*
 * Server.h
 *
 *  Created on: 2014-10-14
 *      Author: hanshuo
 */

#ifndef SERVER_H_
#define SERVER_H_

#include"Socket.h"
#include"Operation.h"
#include"../Database/Database.h"

/*
 * the Server is only at a original and simple version.
 * the following features are not supported by now:
 *     logging in with legal account and password.
 *     multiple active databases.
 *     configurable parameters with a config.xml file.
 * the following several commands are supported:
 *     load <db_name>;
 *     unload <db_name>;
 *     import <db_name> <rdf_file_path>;
 *     query <SPARQL>;
 *     show databases;
 *     exit;
 */

class Server
{
public:
    Server();
    Server(unsigned short _port);
    ~Server();

    bool createConnection();
    bool deleteConnection();
    void listen();

    bool response(Socket _socket, std::string& _msg);
    bool parser(std::string _raw_cmd, Operation& _ret_oprt);
    bool createDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg);
    bool deleteDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg);
    bool loadDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg);
    bool unloadDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg);
    bool showDatabases(std::string _ac_name, std::string& _ret_msg);
    bool importRDF(std::string _db_name, std::string _ac_name, std::string _rdf_path, std::string& _ret_msg);
    bool insertTriple(std::string _db_name, std::string _ac_name, std::string _rdf_path, std::string& _ret_msg);
    bool query(const std::string _query, std::string& _ret_msg);


private:
    unsigned short connectionPort;
    int connectionMaxNum;
    int databaseMaxNum;
    Socket socket;
    Database* database;
};



#endif /* SERVER_H_ */
