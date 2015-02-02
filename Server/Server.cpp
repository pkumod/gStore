/*
 * Server.cpp
 *
 *  Created on: 2014-10-14
 *      Author: hanshuo
 */

#include"Server.h"
#include"../Database/Database.h"
#include<iostream>
#include<sstream>

Server::Server()
{
    this->connectionPort = Socket::DEFAULT_CONNECT_PORT; // default communication port is 3305.
    this->connectionMaxNum = Socket::MAX_CONNECTIONS;
    this->databaseMaxNum = 1; // will be updated when supporting multiple databases.
    this->database = NULL;
}

Server::Server(unsigned short _port)
{
    this->connectionPort = _port;
    this->connectionMaxNum = Socket::MAX_CONNECTIONS;
    this->databaseMaxNum = 1; // will be updated when supporting multiple databases.
    this->database = NULL;
}

Server::~Server()
{
    delete this->database;
}

bool Server::createConnection()
{
    bool flag;

    flag = this->socket.create();
    if (!flag)
    {
        cerr << "cannot create socket. @Server::createConnection" << endl;
        return false;
    }

    flag = this->socket.bind(this->connectionPort);
    if (!flag)
    {
        cerr << "cannot bind to port " << this->connectionPort << ". @Server::createConnection" << endl;
        return false;
    }

    flag = this->socket.listen();
    if (!flag)
    {
        cerr << "cannot listen to port" << this->connectionPort << ". @Server::createConnection" << endl;
        return false;
    }

    return true;
}

bool Server::deleteConnection()
{
    bool flag = this->socket.close();

    return flag;
}

bool Server::response(Socket _socket, std::string& _msg)
{
    bool flag = _socket.send(_msg);

    return flag;
}

void Server::listen()
{
    while (true)
    {
        Socket new_server_socket;

        cout << "Wait for input..." << endl;

        this->socket.accept(new_server_socket);

        cout << "accept new socket." << endl;

        string recv_cmd;
        bool recv_return = new_server_socket.recv(recv_cmd);
        if (!recv_return)
        {
            cerr << "receive command from client error. @Server::listen" << endl;
            continue;
        }

        cout <<"received msg: " << recv_cmd << endl;

        Operation operation;
        bool parser_return = this->parser(recv_cmd, operation);
        cout << "parser_return=" << parser_return << endl; //debug
        if (!parser_return)
        {
            cout << "parser command error. @Server::listen" << endl;
            string ret_msg = "invalid command.";
            this->response(new_server_socket, ret_msg);
            new_server_socket.close();
            continue;
        }

        string ret_msg;
        CommandType cmd_type = operation.getCommand();
        switch (cmd_type)
        {
            case CMD_LOAD:
            {
                string db_name = operation.getParameter(0);
                this->loadDatabase(db_name, "", ret_msg);
                break;
            }
            case CMD_UNLOAD:
            {
                string db_name = operation.getParameter(0);
                this->unloadDatabase(db_name, "", ret_msg);
                break;
            }
            case CMD_IMPORT:
            {
                string db_name = operation.getParameter(0);
                string rdf_path = operation.getParameter(1);
                this->importRDF(db_name, "", rdf_path, ret_msg);
                break;
            }
            case CMD_QUERY:
            {
                string query = operation.getParameter(0);
                this->query(query, ret_msg);
                break;
            }
            case CMD_SHOW:
            {
                string para = operation.getParameter(0);
                if (para == "databases")
                {
                    this->showDatabases("", ret_msg);
                }
                else
                {
                    ret_msg = "invalid command.";
                }
                break;
            }
            case CMD_INSERT:
            {
                string db_name = operation.getParameter(0);
                string rdf_path = operation.getParameter(1);
                this->insertTriple(db_name, "", rdf_path, ret_msg);
                break;
            }
            default:
                cerr << "this command is not supported by now. @Server::listen" << endl;
        }

        this->response(new_server_socket, ret_msg);
        new_server_socket.close();
    }
}

bool Server::parser(std::string _raw_cmd, Operation& _ret_oprt)
{
    int cmd_start_pos = 0;
    int raw_len = _raw_cmd.size();

    for (int i=0;i<raw_len;i++)
        if (_raw_cmd[i] == '\n')
        {
            _raw_cmd[i] = ' ';
        }

    while (cmd_start_pos < raw_len && _raw_cmd[cmd_start_pos] == ' ')
        cmd_start_pos ++;
    if (cmd_start_pos == raw_len)
    {
        return false;
    }

    int idx1 = raw_len;
    for (int i=cmd_start_pos;i<raw_len;i++)
        if (_raw_cmd[i] == ' ')
        {
            idx1 = i;
            break;
        }

    //cout << _raw_cmd << "\n" << cmd_start_pos << " " << idx1 << endl; //debug

    string cmd = _raw_cmd.substr(cmd_start_pos, idx1-cmd_start_pos);
    int para_start_pos = idx1;
    while (para_start_pos < raw_len && _raw_cmd[para_start_pos] == ' ')
        para_start_pos ++;

    int para_cnt;
    if (cmd == "load" || cmd == "unload")
    {
        if (cmd == "load")
        {
            _ret_oprt.setCommand(CMD_LOAD);
        }
        else
        {
            _ret_oprt.setCommand(CMD_UNLOAD);
        }
        para_cnt = 1;
    }
    else if (cmd == "import")
    {
        _ret_oprt.setCommand(CMD_IMPORT);
        para_cnt = 2;
    }
    else if (cmd == "query")
    {
        _ret_oprt.setCommand(CMD_QUERY);
        para_cnt = 1;
    }
    else if (cmd == "show")
    {
        _ret_oprt.setCommand(CMD_SHOW);
        para_cnt = 1;
    }
    else if (cmd == "insert")
    {
        _ret_oprt.setCommand(CMD_INSERT);
        para_cnt = 2;
    }
    else
    {
        return false;
    }

    vector<string> paras;
    int cur_idx = para_start_pos;
    for (int i=1;i<=para_cnt;i++)
    {
        if (cur_idx >= raw_len)
        {
            return false;
        }

        int next_idx = raw_len;
        if (i < para_cnt)
        {
            for (int j=cur_idx;j<raw_len;j++)
                if (_raw_cmd[j] == ' ')
                {
                    next_idx = j;
                    break;
                }
        }
        else
        {
            for (int j=raw_len-1;j>cur_idx;j--)
                if (_raw_cmd[j] != ' ')
                {
                    next_idx = j+1;
                    break;
                }
        }

        paras.push_back(_raw_cmd.substr(cur_idx, next_idx - cur_idx));

        cur_idx = next_idx;
        while (cur_idx < raw_len && _raw_cmd[cur_idx] == ' ')
            cur_idx ++;
    }

    if (cur_idx != raw_len)
    {
        return false;
    }

    _ret_oprt.setParameter(paras);

    return true;
}

bool Server::createDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg)
{
    // to be implemented...

    return false;
}

bool Server::deleteDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg)
{
    // to be implemented...
    return false;
}

bool Server::loadDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg)
{
    this->database = new Database(_db_name);

    bool flag = this->database->load();

    if (flag)
    {
        _ret_msg = "load database done.";
    }
    else
    {
        _ret_msg = "load database failed.";
    }

    return flag;
}

bool Server::unloadDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg)
{
    if (this->database == NULL || this->database->getName() != _db_name)
    {
        _ret_msg = "database:" + _db_name + " is not loaded.";
        return false;
    }

    delete this->database;
    this->database = NULL;
    _ret_msg = "unload database done.";

    return true;
}

bool Server::importRDF(std::string _db_name, std::string _ac_name, std::string _rdf_path, std::string& _ret_msg)
{
    if (this->database != NULL && this->database->getName() != _db_name)
    {
        this->database->unload();
        delete this->database;
    }

    this->database = new Database(_db_name);
    bool flag = this->database->build(_rdf_path);

    if (flag)
    {
        _ret_msg = "import RDF file to database done.";
    }
    else
    {
        _ret_msg = "import RDF file to database failed.";
    }

    return flag;
}

bool Server::insertTriple(std::string _db_name, std::string _ac_name, std::string _rdf_path, std::string& _ret_msg)
{
    if (this->database != NULL)
    {
        this->database->unload();
        delete this->database;
    }

    this->database = new Database(_db_name);
    bool flag = this->database->insert(_rdf_path);

    if (flag)
    {
        _ret_msg = "insert triple file to database done.";
    }
    else
    {
        _ret_msg = "import triple file to database failed.";
    }

    return flag;
}

bool Server::query(const std::string _query, std::string& _ret_msg)
{
    if (this->database == NULL)
    {
        _ret_msg = "database has not been loaded.";
        return false;
    }

    ResultSet res_set;
    bool flag = this->database->query(_query, res_set);
    if (flag)
    {
        _ret_msg = res_set.to_str();
    }
    else
    {
        _ret_msg = "query failed.";
    }

    return flag;
}

bool Server::showDatabases(std::string _ac_name, std::string& _ret_msg)
{
    if (this->database != NULL)
    {
        _ret_msg = "\n" + this->database->getName() + "\n";
    }
    else
    {
        _ret_msg = "\n[empty]\n";
    }

    return true;
}

