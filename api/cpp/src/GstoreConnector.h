/*
 * GstoreConnector.h
 *
 *  Created on: 2014-11-1
 *      Author: hanshuo
 */

#ifndef GSTORECONNECTOR_H_
#define GSTORECONNECTOR_H_

#include"../../../Server/Socket.h"
#include<cstring>

class GstoreConnector
{
public:
    GstoreConnector();
    GstoreConnector(std::string _ip, unsigned short _port);
    GstoreConnector(unsigned short _port);
    ~GstoreConnector();

    bool load(std::string _db_name);
    bool unload(std::string _db_name);
    bool build(std::string _db_name, std::string _rdf_file_path);
    std::string query(std::string _sparql);

private:
    static std::string defaultServerIP;
    static unsigned short defaultServerPort;

    std::string serverIP;
    unsigned short serverPort;
//    std::string username;
//    std::string password;
    Socket socket;

    bool connect();
    bool disconnect();
};

#endif /* GSTORECONNECTOR_H_ */
