/*=============================================================================
# Filename: Client.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2016-02-23 13:39
# Description: implement functions in Client.h
=============================================================================*/

#include "Client.h"

using namespace std;

Client::Client()
{
  this->ip = Socket::DEFAULT_SERVER_IP;
  this->port = Socket::DEFAULT_CONNECT_PORT;
}

Client::Client(string _ip, unsigned short _port)
{
  this->ip = _ip;
  this->port = _port;
}

Client::Client(unsigned short _port)
{
  this->ip = Socket::DEFAULT_SERVER_IP;
  this->port = _port;
}

Client::~Client()
{
}

bool Client::connectServer()
{
  bool flag = this->socket.create();
  if (!flag) {
    cerr << "cannot create socket. @Client::connectServer" << endl;
    return false;
  }

  flag = this->socket.connect(this->ip, this->port);

  if (!flag) {
    cerr << "cannot connect to server. @Client::connectServer" << endl;
    return false;
  }

  return true;
}

bool Client::disconnectServer()
{
  bool flag = this->socket.close();

  return flag;
}

bool Client::send(const string& _msg)
{
  bool flag = this->socket.send(_msg);

  return flag;
}

bool Client::receiveResponse(string& _recv_msg)
{
  bool flag = this->socket.recv(_recv_msg);

  return flag;
}

void Client::run()
{
  string cmd;

#ifdef READLINE_ON
  char* buf, prompt[] = "gsql>";
  //printf("Type `help` for information of all commands\n");
  //printf("Type `help command_t` for detail of command_t\n");
  rl_bind_key('\t', rl_complete);
  //QUERY: should add ';'?
  while (true) {
    //BETTER:write in multi lines as in below comments
    buf = readline(prompt);
    if (buf == NULL)
      continue;
    else
      add_history(buf);
    if (strncmp(buf, "help", 4) == 0) {
      if (strcmp(buf, "help") == 0) {
        //print commands message
        printf("help - print commands message\n");
        printf("quit - quit the console normally\n");
        printf("import - build a database for a given dataset\n");
        printf("load - load an existen database\n");
        printf("unload - unload an existen database\n");
        printf("sparql - load query from the second argument\n");
        printf("show - show the current database's name\n");
      } else {
        //TODO: help for a given command
      }
      continue;
    } else if (strcmp(buf, "quit") == 0)
      break;
    else if (strncmp(buf, "import", 6) != 0 && strncmp(buf, "load", 4) != 0 && strncmp(buf, "unload", 6) != 0 && strncmp(buf, "sparql", 6) != 0 && strncmp(buf, "show", 4) != 0) {
      printf("unknown commands\n");
      continue;
    }

    string query_file;
    string query;
    FILE* fp = stdout; ///default to output on screen
    bool ifredirect = false;
    //BETTER:build a parser for this console
    //spaces/tabs can be before commands

    if (strncmp(buf, "sparql", 6) == 0) {
      //NOTICE: if using query string, '>' is ok to exist!
      char* rp = buf;
      int pos = strlen(buf) - 1;
      while (pos > -1) {
        if (*(rp + pos) == '"') {
          break;
        } else if (*(rp + pos) == '>') {
          ifredirect = true;
          break;
        }
        pos--;
      }
      rp += pos;
      //DEBUG:redirect sometimes not work for path query

      char* p = buf + strlen(buf) - 1;
      if (ifredirect) {
        printf("redirected!\n");
        char* tp = p;
        while (*tp == ' ' || *tp == '\t')
          tp--;
        *(tp + 1) = '\0';
        tp = rp + 1;
        while (*tp == ' ' || *tp == '\t')
          tp++;
        printf("redirect: %s\n", tp);
        fp = fopen(tp, "w"); //NOTICE:not judge here!
        p = rp - 1;          //NOTICE: all separated with ' ' or '\t'
      }
      while (*p == ' ' || *p == '\t' || *p == '"') //set the end of path
        p--;
      *(p + 1) = '\0';
      p = buf + 6;
      while (*p == ' ' || *p == '\t') //acquire the start of path
        p++;

      bool isPath = true;
      if (*p == '"') {
        isPath = false;
        p++;
      }

      char* q;
      //BETTER:get real path in server port?
      if (isPath) {
        //TODO: support the soft links(or hard links)
        //there are also readlink and getcwd functions for help
        //http://linux.die.net/man/2/readlink
        //NOTICE:getcwd and realpath cannot acquire the real path of file
        //in the same directory and the program is executing when the
        //system starts running
        //NOTICE: use realpath(p, NULL) is ok, but need to free the memory
        q = realpath(p, NULL); //QUERY:still not work for soft links
#ifdef DEBUG_PRECISE
        printf("%s\n", p);
#endif
        if (q == NULL) {
          printf("invalid path!\n");
          free(q);
          free(buf);
          continue;
        } else
          printf("%s\n", q);
        //query = getQueryFromFile(p);
        query = Util::getQueryFromFile(q);
      } else {
        //BETTER:check query in ""
        query = string(p);
      }

      if (query.empty()) {
        if (isPath)
          free(q);
        //free(resolved_path);
        free(buf);
        if (ifredirect)
          fclose(fp);
        continue;
      }

      printf("query is:\n%s\n\n", query.c_str());
      if (isPath)
        free(q);
      cmd = string("query ") + query;
    } else if (strncmp(buf, "show", 4) == 0) {
      cmd = string("show databases");
    } else {
      cmd = string(buf);
    }
    printf("%s\n", cmd.c_str());

    free(buf);
//free(resolved_path);
#ifdef DEBUG_PRECISE
    printf("after buf freed!\n");
#endif

    //interacte with server
    bool flag = this->connectServer();
    if (!flag) {
      cerr << "connect server error. @Client::run" << endl;
      if (ifredirect)
        fclose(fp);
      continue;
    }

    flag = this->send(cmd);
    if (!flag) {
      cerr << "sent message error. @Client::run" << endl;
      if (ifredirect)
        fclose(fp);
      continue;
    }

    string recv_msg;
    flag = this->receiveResponse(recv_msg);
    //BETTER:the string maybe too large?
    fprintf(fp, "%s\n", recv_msg.c_str());

    this->disconnectServer();
    if (!flag) {
      cerr << "disconnect server error. @Client::run" << endl;
      if (ifredirect)
        fclose(fp);
      continue;
    }
  }
//#else
//while (true)
//{
//        while(true)
//        {
//			//BETTER:readline and parser
//            cout << "->";
//            string line;
//            getline(cin, line);
//            int line_len = line.size();
//            if (line_len >0 && line[line_len-1] == ';')
//            {
//                line.resize(line_len - 1);
//                cmd += line;
//                break;
//            }
//            cmd += line + "\n";
//        }
//
//		//cout << "input end" << endl;
//
//        if(cmd == "quit")
//        {
//            break;
//        }
//
//        bool flag = this->connectServer();
//        if (!flag)
//        {
//            cerr << "connect server error. @Client::run" << endl;
//            continue;
//        }
//
//        flag = this->send(cmd);
//        if (!flag)
//        {
//            cerr << "sent message error. @Client::run" << endl;
//            continue;
//        }
//
//        string recv_msg;
//        flag = this->receiveResponse(recv_msg);
//        cout << recv_msg << endl;
//
//        this->disconnectServer();
//        if (!flag)
//        {
//            cerr << "disconnect server error. @Client::run" << endl;
//            continue;
//        }
//}
#endif
}
