/*=============================================================================
# Filename: Server.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-25 13:47
# Description:
=============================================================================*/

#include "Server.h"

using namespace std;

//Server::Server()
//{
//this->connectionPort = Socket::DEFAULT_CONNECT_PORT;
//this->connectionMaxNum = Socket::MAX_CONNECTIONS;
//this->databaseMaxNum = 1; // will be updated when supporting multiple databases.
//this->database = NULL;
//this->db_home = Util::global_config["db_home"];
//this->db_suffix = Util::global_config["db_suffix"];
//}

Server::Server(unsigned short _port)
{
  this->connectionPort = _port;
  this->connectionMaxNum = Socket::MAX_CONNECTIONS;
  this->databaseMaxNum = 1; // will be updated when supporting multiple databases.
  this->database = NULL;
  this->db_home = Util::global_config["db_home"];
  this->db_suffix = Util::global_config["db_suffix"];
  this->next_backup = 0;
  this->scheduler_pid = 0;
}

Server::~Server()
{
  delete this->database;
}

bool
Server::createConnection()
{
  bool flag;

  flag = this->socket.create();
  if (!flag) {
    cerr << Util::getTimeString() << "cannot create socket. @Server::createConnection" << endl;
    return false;
  }

  flag = this->socket.bind(this->connectionPort);
  if (!flag) {
    cerr << Util::getTimeString() << "cannot bind to port " << this->connectionPort << ". @Server::createConnection" << endl;
    return false;
  }

  flag = this->socket.listen();
  if (!flag) {
    cerr << Util::getTimeString() << "cannot listen to port" << this->connectionPort << ". @Server::createConnection" << endl;
    return false;
  }

  return true;
}

bool
Server::deleteConnection()
{
  bool flag = this->socket.close();

  return flag;
}

bool
Server::response(Socket _socket, std::string& _msg)
{
  bool flag = _socket.send(_msg);

  return flag;
}

void
Server::listen()
{
  while (true) {
    Socket new_server_socket;

    cout << Util::getTimeString() << "Wait for input..." << endl;

    this->socket.accept(new_server_socket);

    cout << Util::getTimeString() << "accept new socket." << endl;

    string recv_cmd;
    bool recv_return = new_server_socket.recv(recv_cmd);
    if (!recv_return) {
      cerr << Util::getTimeString() << "receive command from client error. @Server::listen" << endl;
      continue;
    }

    cout << Util::getTimeString() << "received msg: " << recv_cmd << endl;

    Operation operation;
    bool parser_return = this->parser(recv_cmd, operation);
    cout << Util::getTimeString() << "parser_return=" << parser_return << endl; //debug
    if (!parser_return) {
      cout << Util::getTimeString() << "parser command error. @Server::listen" << endl;
      string ret_msg = "invalid command.";
      this->response(new_server_socket, ret_msg);
      new_server_socket.close();
      continue;
    }

    string ret_msg;
    bool _stop = false;
    CommandType cmd_type = operation.getCommand();
    switch (cmd_type) {
    case CMD_TEST: {
      ret_msg = "OK";
      break;
    }
    case CMD_LOAD: {
      string db_name = operation.getParameter(0);
      this->loadDatabase(db_name, "", ret_msg);
      break;
    }
    case CMD_UNLOAD: {
      string db_name = operation.getParameter(0);
      this->unloadDatabase(db_name, "", ret_msg);
      break;
    }
    case CMD_IMPORT: {
      string db_name = operation.getParameter(0);
      string rdf_path = operation.getParameter(1);
      this->importRDF(db_name, "", rdf_path, ret_msg);
      break;
    }
    case CMD_DROP: {
      string db_name = operation.getParameter(0);
      this->dropDatabase(db_name, "", ret_msg);
      break;
    }
    case CMD_QUERY: {
      string query = operation.getParameter(0);
      pthread_t timer = Server::start_timer();
      if (timer == 0) {
        cerr << Util::getTimeString() << "Failed to start timer." << endl;
      }
      this->query(query, ret_msg);
      if (timer != 0 && !Server::stop_timer(timer)) {
        cerr << Util::getTimeString() << "Failed to stop timer." << endl;
      }
      break;
    }
    case CMD_SHOW: {
      string para = operation.getParameter(0);
      if (para == "databases" || para == "all") {
        this->showDatabases(para, "", ret_msg);
      } else {
        ret_msg = "invalid command.";
      }
      break;
    }
    //case CMD_INSERT:
    //{
    //string db_name = operation.getParameter(0);
    //string rdf_path = operation.getParameter(1);
    //this->insertTriple(db_name, "", rdf_path, ret_msg);
    //break;
    //}
    case CMD_STOP: {
      this->stopServer(ret_msg);
      _stop = true;
      break;
    }
    case CMD_BACKUP: {
      string para = operation.getParameter(0);
      stringstream ss(para);
      long time_backup;
      ss >> time_backup;
      if (this->next_backup == 0) {
        break;
      }
      while (this->next_backup < time_backup) {
        this->next_backup += Util::gserver_backup_interval;
      }
      if (this->next_backup == time_backup) {
        this->backup(ret_msg);
      } else {
        ret_msg = "done";
      }
      break;
    }
    default:
      cerr << Util::getTimeString() << "this command is not supported by now. @Server::listen" << endl;
    }

    this->response(new_server_socket, ret_msg);
    new_server_socket.close();
    if (_stop) {
      this->deleteConnection();
      cout << Util::getTimeString() << "server stopped." << endl;
      break;
    }
    if (this->next_backup > 0) {
      time_t cur_time = time(NULL);
      if (cur_time >= this->next_backup) {
        string str;
        this->backup(str);
      }
    }
  }
}

bool
Server::parser(std::string _raw_cmd, Operation& _ret_oprt)
{
  int cmd_start_pos = 0;
  int raw_len = (int)_raw_cmd.size();

  for (int i = 0; i < raw_len; i++) {
    if (_raw_cmd[i] == '\n') {
      _raw_cmd[i] = ' ';
    }
  }

  while (cmd_start_pos < raw_len && _raw_cmd[cmd_start_pos] == ' ') {
    cmd_start_pos++;
  }
  if (cmd_start_pos == raw_len) {
    return false;
  }

  int idx1 = raw_len;
  for (int i = cmd_start_pos; i < raw_len; i++) {
    if (_raw_cmd[i] == ' ') {
      idx1 = i;
      break;
    }
  }

  string cmd = _raw_cmd.substr(cmd_start_pos, idx1 - cmd_start_pos);
  int para_start_pos = idx1;
  while (para_start_pos < raw_len && _raw_cmd[para_start_pos] == ' ')
    para_start_pos++;

  int para_cnt;
  if (cmd == "test") {
    _ret_oprt.setCommand(CMD_TEST);
    para_cnt = 0;
  } else if (cmd == "load") {
    _ret_oprt.setCommand(CMD_LOAD);
    para_cnt = 1;
  } else if (cmd == "unload") {
    _ret_oprt.setCommand(CMD_UNLOAD);
    para_cnt = 1;
  } else if (cmd == "import") {
    _ret_oprt.setCommand(CMD_IMPORT);
    para_cnt = 2;
  } else if (cmd == "query") {
    _ret_oprt.setCommand(CMD_QUERY);
    para_cnt = 1;
  } else if (cmd == "show") {
    _ret_oprt.setCommand(CMD_SHOW);
    para_cnt = 1;
  } else if (cmd == "insert") {
    _ret_oprt.setCommand(CMD_INSERT);
    para_cnt = 2;
  } else if (cmd == "drop") {
    _ret_oprt.setCommand(CMD_DROP);
    para_cnt = 1;
  } else if (cmd == "stop") {
    _ret_oprt.setCommand(CMD_STOP);
    para_cnt = 0;
  } else if (cmd == "backup") {
    _ret_oprt.setCommand(CMD_BACKUP);
    para_cnt = 1;
  } else {
    return false;
  }

  vector<string> paras;
  int cur_idx = para_start_pos;
  for (int i = 1; i <= para_cnt; i++) {
    if (cur_idx >= raw_len) {
      return false;
    }

    int next_idx = raw_len;
    if (i < para_cnt) {
      for (int j = cur_idx; j < raw_len; j++)
        if (_raw_cmd[j] == ' ') {
          next_idx = j;
          break;
        }
    } else {
      for (int j = raw_len - 1; j > cur_idx; j--)
        if (_raw_cmd[j] != ' ') {
          next_idx = j + 1;
          break;
        }
    }

    paras.push_back(_raw_cmd.substr(cur_idx, next_idx - cur_idx));

    cur_idx = next_idx;
    while (cur_idx < raw_len && _raw_cmd[cur_idx] == ' ')
      cur_idx++;
  }

  if (cur_idx != raw_len) {
    return false;
  }

  _ret_oprt.setParameter(paras);

  return true;
}

bool
Server::createDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg)
{
  // to be implemented...

  return false;
}

bool
Server::dropDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg)
{
  if (this->database != NULL) {
    _ret_msg = "please do not use this command when you are using a database.";
    return false;
  }

  size_t length = _db_name.length();
  if (length < 3 || _db_name.substr(length - 3, 3) == ".db") {
    _ret_msg = "you can not only drop databases whose names end with \".db\"";
    return false;
  }

  string store_path = this->db_home + "/" + _db_name + this->db_suffix;

  std::string cmd = std::string("rm -rf ") + store_path;
  int ret = system(cmd.c_str());
  if (ret == 0) {
    _ret_msg = "drop database done.";
    return true;
  } else {
    _ret_msg = "drop database failed.";
    return false;
  }
}

bool
Server::loadDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg)
{
  if (this->database == NULL) {
    this->database = new Database(_db_name);
  } else {
    _ret_msg = "please unload the current db first: " + this->database->getName();
    return false;
  }

  bool flag = this->database->load();

  if (flag) {
    _ret_msg = "load database done.";
  } else {
    _ret_msg = "load database failed.";
    delete this->database;
    this->database = NULL;
    return false;
  }

  pid_t fpid = vfork();

  // child, scheduler
  if (fpid == 0) {
    time_t cur_time = time(NULL);
    long time_backup = Util::read_backup_time();
    long first_backup = cur_time - (cur_time - time_backup) % Util::gserver_backup_interval
                        + Util::gserver_backup_interval;
    this->next_backup = first_backup;
    string s_port = Util::int2string(this->connectionPort);
    string s_next_backup = Util::int2string(first_backup);
    execl("bin/gserver_backup_scheduler", "gserver_backup_scheduler", s_port.c_str(), s_next_backup.c_str(), NULL);
    exit(0);
    return true;
  }
  // parent
  if (fpid > 0) {
    this->scheduler_pid = fpid;
  }
  // fork failure
  else if (fpid < 0) {
    cerr << Util::getTimeString() << "Database will not be backed-up automatically." << endl;
  }

  //_ret_msg = "load database done.";
  return true;
  //return flag;
}

bool
Server::unloadDatabase(std::string _db_name, std::string _ac_name, std::string& _ret_msg)
{
  if (this->database == NULL || this->database->getName() != _db_name) {
    _ret_msg = "database:" + _db_name + " is not loaded.";
    return false;
  }

  delete this->database;
  this->database = NULL;
  _ret_msg = "unload database done.";

  this->next_backup = 0;
  //string cmd = "kill " + Util::int2string(this->scheduler_pid);
  //system(cmd.c_str());
  kill(this->scheduler_pid, SIGTERM);
  waitpid(this->scheduler_pid, NULL, 0);
  this->scheduler_pid = 0;

  return true;
}

bool
Server::importRDF(std::string _db_name, std::string _ac_name, std::string _rdf_path, std::string& _ret_msg)
{
  //if (this->database != NULL && this->database->getName() != _db_name)
  if (this->database != NULL) {
    //delete this->database;
    //NOTICE:if there is a db loaded, we should not build directly, tell user to unload it first
    _ret_msg = "please unload the current db first: " + this->database->getName();
    return false;
  }

  this->database = new Database(_db_name);
  bool flag = this->database->build(_rdf_path);

  delete this->database;
  this->database = NULL;

  if (flag) {
    _ret_msg = "import RDF file to database done.";
  } else {
    _ret_msg = "import RDF file to database failed.";
  }

  return flag;
}

//bool
//Server::insertTriple(std::string _db_name, std::string _ac_name, std::string _rdf_path, std::string& _ret_msg)
//{
//if (this->database != NULL)
//{
//this->database->unload();
//delete this->database;
//}

//this->database = new Database(_db_name);
//bool flag = this->database->insert(_rdf_path);

//if (flag)
//{
//_ret_msg = "insert triple file to database done.";
//}
//else
//{
//_ret_msg = "import triple file to database failed.";
//}

//return flag;
//}

bool
Server::query(const string _query, string& _ret_msg)
{
  //cout<<"Server query()"<<endl;
  //cout<<_query<<endl;
  cout << Util::getTimeString() << "Server query(): " << _query << endl;

  if (this->database == NULL) {
    _ret_msg = "database has not been loaded.";
    return false;
  }

  FILE* output = NULL;
#ifdef OUTPUT_QUERY_RESULT
  string path = "logs/gserver_query.log";
  output = fopen(path.c_str(), "w");
#endif

  ResultSet res_set;
  int query_ret = this->database->query(_query, res_set, output);
  if (output != NULL) {
    fclose(output);
  }

  bool flag = true;
  //cout<<"Server query ret: "<<query_ret<<endl;
  if (query_ret <= -100) //select query
  {
    //_ret_msg = "results are too large!";
    //BETTER: divide and transfer if too large to be placed in memory, using Stream
    if (query_ret == -100) {
#ifdef SERVER_SEND_JSON
      _ret_msg = res_set.to_JSON();
#else
      _ret_msg = res_set.to_str();
#endif
    } else //query error
    {
      flag = false;
      _ret_msg = "query failed.";
      //BETTER: {type:select} {success:false}
    }
  } else //update query
  {
    if (query_ret >= 0) {
      _ret_msg = "update num: " + Util::int2string(query_ret);
    } else //update error
    {
      flag = false;
      _ret_msg = "update failed.";
    }
  }

  return flag;
}

bool
Server::showDatabases(string _para, string _ac_name, string& _ret_msg)
{
  if (_para == "all") {
    _ret_msg = Util::getItemsFromDir(this->db_home);
    return true;
  }
  if (this->database != NULL) {
    _ret_msg = "\n" + this->database->getName() + "\n";
  } else {
    _ret_msg = "\n[empty]\n";
  }

  return true;
}

bool Server::stopServer(string& _ret_msg)
{
  if (this->database != NULL) {
    delete this->database;
    this->database = NULL;
  }
  _ret_msg = "server stopped.";
  return true;
}

bool Server::backup(string& _ret_msg)
{
  this->next_backup += Util::gserver_backup_interval;
  if (this->database == NULL) {
    _ret_msg = "No database in use.";
    return false;
  }
  if (!this->database->backup()) {
    _ret_msg = "Backup failed.";
    return false;
  }
  _ret_msg = "done";
  return true;
}

pthread_t Server::start_timer()
{
  pthread_t timer_thread;
  if (pthread_create(&timer_thread, NULL, Server::timer, NULL) == 0) {
    return timer_thread;
  }
  return 0;
}

bool Server::stop_timer(pthread_t _timer)
{
  return pthread_kill(_timer, SIGTERM) == 0;
}

void* Server::timer(void* _args)
{
  signal(SIGTERM, Server::timer_sigterm_handler);
  sleep(Util::gserver_query_timeout);
  cerr << Util::getTimeString() << "Query out of time." << endl;
  abort();
}

void Server::timer_sigterm_handler(int _signal_num)
{
  pthread_exit(0);
}
