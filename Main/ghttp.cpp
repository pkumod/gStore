/*=============================================================================
# Filename: ghttp.cpp
# Author: Bookug Lober suxunbin
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2019-5-9 17:00
# Description: created by lvxin, improved by lijing and suxunbin and liwenjie
# Last Modified:2020-03-26 14:16
# Description: add the initVersion function to init the version info by data/system/version.nt
=============================================================================*/
//SPARQL Endpoint:  log files in logs/endpoint/
//operation.log: not used
//query.log: query string, result num, and time of answering

//TODO: to add db_name to all URLs, and change the index.js using /show to get name, save and set
//TODO: use gzip for network transfer, it is ok to download a gzip file instead of the original one

#include "../Server/server_http.hpp"
#include "../Server/client_http.hpp"
//db
#include "../Database/Database.h"
#include "../Util/Util.h"

#include "../tools/rapidjson/document.h"
#include "../tools/rapidjson/prettywriter.h"
#include "../tools/rapidjson/writer.h"
#include "../tools/rapidjson/stringbuffer.h"
#include <iostream>
#include <fstream>
using namespace rapidjson;
using namespace std;
//Added for the json-example:
using namespace boost::property_tree;

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

#define THREAD_NUM 30
#define MAX_DATABASE_NUM 100
#define MAX_USER_NUM 1000
#define ROOT_USERNAME "root"
#define ROOT_PASSWORD "123456"
#define DEFAULT_USERNAME "a"
#define DEFAULT_PASSWORD "1"
#define SYSTEM_PATH "data/system/system.nt"
#define MAX_QUERYLOG_size 800000000
#define QUERYLOG_PATH "logs/endpoint/"
#define SYSTEM_USERNAME "system"
#define MAX_OUTPUT_SIZE 100000
#define TEST_IP "106.13.13.193"
#define DB_PATH "."
#define BACKUP_PATH "./backups"

int initialize(int argc, char* argv[]);
int copy(string src_path, string dest_path);
//Added for the default_resource example
void default_resource_send(const HttpServer& server, const shared_ptr<HttpServer::Response>& response,
                           const shared_ptr<ifstream>& ifs);
void download_result(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string download, string filepath);
std::string CreateJson(int StatusCode, string StatusMsg, bool body, string ResponseBody = "response");
pthread_t start_thread(void* (*_function)(void*));
bool stop_thread(pthread_t _thread);
void* func_timer(void* _args);
void* func_scheduler(void* _args);
void thread_sigterm_handler(int _signal_num);
bool addPrivilege(string username, string type, string db_name);
bool delPrivilege(string username, string type, string db_name);
bool checkPrivilege(string username, string type, string db_name);
void DB2Map();
string querySys(string sparql);
bool sysrefresh();
bool updateSys(string sparql);
int db_reload(string db_name);
//bool doQuery(string format, string db_query, const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

//=============================================================================
bool build_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool load_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool unload_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool export_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool query_handler0(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool query_handler1(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool monitor_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool checkpoint_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool show_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool delete_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool download_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool default_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool login_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool user_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool showUser_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool check_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool stop_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool drop_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool backup_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool restore_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool auto_backup_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool getCoreVersion_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool getAPIVersion_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool setCoreVersion_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool initVersion_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

bool setAPIVersion_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

void query_thread(bool update_flag, string db_name, string format, string db_query, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool checkall_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool refresh_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);

void signalHandler(int signum);
//=============================================================================

//TODO: use lock to protect logs when running in multithreading environment
FILE* query_logfp = NULL;
string queryLog = "logs/endpoint/query.log";
mutex query_log_lock;
string CoreVersion;
string APIVersion;
string system_password;
string NAMELOG_PATH = "name.log";
int port;

pthread_rwlock_t databases_map_lock;
pthread_rwlock_t already_build_map_lock;
pthread_rwlock_t users_map_lock;

Database* system_database;

std::map<std::string, Database*> databases;

//database information
struct DBInfo {
  private:
  std::string db_name;
  std::string creator;
  std::string built_time;

  public:
  pthread_rwlock_t db_lock;

  DBInfo()
  {
    pthread_rwlock_init(&db_lock, NULL);
  }
  DBInfo(string _db_name, string _creator, string _time)
  {
    db_name = _db_name;
    creator = _creator;
    built_time = _time;
    pthread_rwlock_init(&db_lock, NULL);
  }
  DBInfo(string _db_name)
  {
    db_name = _db_name;
    pthread_rwlock_init(&db_lock, NULL);
  }
  ~DBInfo()
  {
    pthread_rwlock_destroy(&db_lock);
  }
  std::string getName()
  {
    return db_name;
  }
  std::string getCreator()
  {
    return creator;
  }
  void setCreator(string _creator)
  {
    creator = _creator;
  }
  std::string getTime()
  {
    return built_time;
  }
  void setTime(string _time)
  {
    built_time = _time;
  }
};

//user information
struct User {
  private:
  std::string username;
  std::string password;

  public:
  std::set<std::string> query_priv;
  std::set<std::string> update_priv;
  std::set<std::string> load_priv;
  std::set<std::string> unload_priv;
  std::set<std::string> backup_priv;
  std::set<std::string> restore_priv;
  std::set<std::string> export_priv;

  pthread_rwlock_t query_priv_set_lock;
  pthread_rwlock_t update_priv_set_lock;
  pthread_rwlock_t load_priv_set_lock;
  pthread_rwlock_t unload_priv_set_lock;
  pthread_rwlock_t backup_priv_set_lock;
  pthread_rwlock_t restore_priv_set_lock;
  pthread_rwlock_t export_priv_set_lock;
  /*
		Database *build_priv[MAX_DATABASE_NUM];
		Database *load_priv[MAX_DATABASE_NUM];
		Database *unload_priv[MAX_DATABASE_NUM];
		*/

  User()
  {
    pthread_rwlock_init(&query_priv_set_lock, NULL);
    pthread_rwlock_init(&update_priv_set_lock, NULL);
    pthread_rwlock_init(&load_priv_set_lock, NULL);
    pthread_rwlock_init(&unload_priv_set_lock, NULL);
    pthread_rwlock_init(&backup_priv_set_lock, NULL);
    pthread_rwlock_init(&restore_priv_set_lock, NULL);
    pthread_rwlock_init(&export_priv_set_lock, NULL);
  }
  User(string _username, string _password)
  {
    if (_username == "")
      username = DEFAULT_USERNAME;
    else
      username = _username;
    if (_password == "")
      password = DEFAULT_PASSWORD;
    else
      password = _password;

    pthread_rwlock_init(&query_priv_set_lock, NULL);
    pthread_rwlock_init(&update_priv_set_lock, NULL);
    pthread_rwlock_init(&load_priv_set_lock, NULL);
    pthread_rwlock_init(&unload_priv_set_lock, NULL);
    pthread_rwlock_init(&backup_priv_set_lock, NULL);
    pthread_rwlock_init(&restore_priv_set_lock, NULL);
    pthread_rwlock_init(&export_priv_set_lock, NULL);
  }
  ~User()
  {
    pthread_rwlock_destroy(&query_priv_set_lock);
    pthread_rwlock_destroy(&update_priv_set_lock);
    pthread_rwlock_destroy(&load_priv_set_lock);
    pthread_rwlock_destroy(&unload_priv_set_lock);
    pthread_rwlock_destroy(&backup_priv_set_lock);
    pthread_rwlock_destroy(&restore_priv_set_lock);
    pthread_rwlock_destroy(&export_priv_set_lock);
  }
  std::string getPassword()
  {
    return password;
  }
  std::string getUsername()
  {
    return username;
  }
  std::string getQuery()
  {
    std::string query_db;
    if (username == ROOT_USERNAME) {
      query_db = "all";
      return query_db;
    }
    std::set<std::string>::iterator it = query_priv.begin();
    while (it != query_priv.end()) {
      query_db = query_db + *it + " ";
      ++it;
    }
    return query_db;
  }
  std::string getUpdate()
  {
    std::string update_db;
    if (username == ROOT_USERNAME) {
      update_db = "all";
      return update_db;
    }
    std::set<std::string>::iterator it = update_priv.begin();
    while (it != update_priv.end()) {
      update_db = update_db + *it + " ";
      ++it;
    }
    return update_db;
  }
  std::string getLoad()
  {
    std::string load_db;
    if (username == ROOT_USERNAME) {
      load_db = "all";
      return load_db;
    }

    std::set<std::string>::iterator it = load_priv.begin();
    while (it != load_priv.end()) {
      load_db = load_db + *it + " ";
      ++it;
    }
    return load_db;
  }
  std::string getUnload()
  {
    std::string unload_db;
    if (username == ROOT_USERNAME) {
      unload_db = "all";
      return unload_db;
    }

    std::set<std::string>::iterator it = unload_priv.begin();
    while (it != unload_priv.end()) {
      unload_db = unload_db + *it + " ";
      ++it;
    }
    return unload_db;
  }
  std::string getrestore()
  {
    std::string restore_db;
    if (username == ROOT_USERNAME) {
      restore_db = "all";
      return restore_db;
    }
    std::set<std::string>::iterator it = restore_priv.begin();
    while (it != restore_priv.end()) {
      restore_db = restore_db + *it + " ";
      ++it;
    }
    return restore_db;
  }
  std::string getbackup()
  {
    std::string backup_db;
    if (username == ROOT_USERNAME) {
      backup_db = "all";
      return backup_db;
    }
    std::set<std::string>::iterator it = backup_priv.begin();
    while (it != backup_priv.end()) {
      backup_db = backup_db + *it + " ";
      ++it;
    }
    return backup_db;
  }
  std::string getexport()
  {
    std::string export_db;
    if (username == ROOT_USERNAME) {
      export_db = "all";
      return export_db;
    }
    std::set<std::string>::iterator it = export_priv.begin();
    while (it != export_priv.end()) {
      export_db = export_db + *it + " ";
      ++it;
    }
    return export_db;
  }
  void setPassword(string psw)
  {
    password = psw;
  }
};
//struct User root = User(ROOT_USERNAME, ROOT_PASSWORD);
std::map<std::string, struct DBInfo*> already_build;
std::map<std::string, struct User*> users;
//struct User root = User(ROOT_USERNAME, ROOT_PASSWORD);
//users.insert(pair<std::string, struct User*>(ROOT_USERNAME, &root));
//users[ROOT_USERNAME] = &root;
//struct User temp_user = User(username2, password2);

int connection_num = 0;

long next_backup = 0;
pthread_t scheduler = 0;

int query_num = 0;
//WARN: if the response or request is freed by main thread
void test_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
  int num = query_num;
  cout << "test thread: " << num << endl;
  sleep(10);
  string error = "test thread here: query ";
  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error<<query_num;
  *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() + 4;
  *response << "\r\nContent-Type: text/plain";
  *response << "\r\nCache-Control: no-cache"
            << "\r\nPragma: no-cache"
            << "\r\nExpires: 0";
  *response << "\r\n\r\n" << error << num;
  cout << "thread ends: " << num << endl;
}

//DEBUG+TODO: why the result transfered to client has no \n \t??
//using json is a good way to avoid this problem

//BETTER: How about change HttpConnector into a console?
//TODO: we need a route
//JSON parser: http://www.tuicool.com/articles/yUJb6f
//(or use boost spirit to generate parser when compiling)
//
//NOTICE: no need to close connection here due to the usage of shared_ptr
//http://www.tuicool.com/articles/3Ub2y2
//
//TODO: the URL format is terrible, i.e. 127.0.0.1:9000/build/lubm/data/LUBM_10.n3
//we should define some keys like operation, database, dataset, query, path ...
//127.0.0.1:9000?operation=build&database=lubm&dataset=data/LUBM_10.n3
//
//TODO: control the authority, check it if requesting for build/load/unload
//for sparql endpoint, just load database when starting, and comment out all functions except for query()

//REFERENCE: C++ URL encoder and decoder
//http://blog.csdn.net/nanjunxiao/article/details/9974593
string UrlDecode(string& SRC)
{
  string ret;
  char ch;
  int ii;
  for (size_t i = 0; i < SRC.length(); ++i) {
    if (int(SRC[i]) == 37) {
      sscanf(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
      ch = static_cast<char>(ii);
      ret += ch;
      i = i + 2;
    } else if (SRC[i] == '+') {
      ret += ' ';
    } else {
      ret += SRC[i];
    }
  }
  return (ret);
}

class Task {
  public:
  bool update;
  string db_name;
  string format;
  string db_query;
  const shared_ptr<HttpServer::Response> response;
  const shared_ptr<HttpServer::Request> request;
  Task(bool flag, string name, string ft, string query, const shared_ptr<HttpServer::Response>& res, const shared_ptr<HttpServer::Request>& req);
  ~Task();
  void run();
};
Task::Task(bool flag, string name, string ft, string query, const shared_ptr<HttpServer::Response>& res, const shared_ptr<HttpServer::Request>& req)
    : response(res)
    , request(req)
{
  update = flag;
  db_name = name;
  format = ft;
  db_query = query;
}
Task::~Task()
{
}
void Task::run()
{
  query_thread(update, db_name, format, db_query, response, request);
}

class Thread {
  public:
  thread TD;
  int ID;
  static int threadnum;
  Task* task;
  Thread();
  ~Thread();
  int GetThreadID();
  void assign(Task* t);
  void run();
  void start();
  friend bool operator==(Thread t1, Thread t2);
  friend bool operator!=(Thread t1, Thread t2);
};

list<Thread*> busythreads;
vector<Thread*> freethreads;
mutex busy_mutex;
mutex free_mutex;
mutex task_mutex;
condition_variable task_cond;

void BackToFree(Thread* t)
{
  busy_mutex.lock();
  busythreads.erase(find(busythreads.begin(), busythreads.end(), t));
  busy_mutex.unlock();

  free_mutex.lock();
  freethreads.push_back(t);
  free_mutex.unlock();
}

int Thread::threadnum = 0;

Thread::Thread()
{
  threadnum++;
  ID = threadnum;
}
Thread::~Thread()
{
}
int Thread::GetThreadID()
{
  return ID;
}
void Thread::assign(Task* t)
{
  task = t;
}
void Thread::run()
{
  cout << "Thread:" << ID << " run\n";
  task->run();
  delete task;
  BackToFree(this);
}
void Thread::start()
{
  TD = thread(&Thread::run, this);
  TD.detach();
}
bool operator==(Thread t1, Thread t2)
{
  return t1.ID == t2.ID;
}
bool operator!=(Thread t1, Thread t2)
{
  return !(t1.ID == t2.ID);
}

class ThreadPool {
  public:
  int ThreadNum;
  bool isclose;
  thread ThreadsManage;
  queue<Task*> tasklines;
  ThreadPool();
  ThreadPool(int t);
  ~ThreadPool();
  void create();
  void SetThreadNum(int t);
  int GetThreadNum();
  void AddTask(Task* t);
  void start();
  void close();
};
ThreadPool::ThreadPool()
{
  isclose = false;
  ThreadNum = 10;
  busythreads.clear();
  freethreads.clear();
  for (int i = 0; i < ThreadNum; i++) {
    Thread* p = new Thread();
    freethreads.push_back(p);
  }
}
ThreadPool::ThreadPool(int t)
{
  isclose = false;
  ThreadNum = t;
  busythreads.clear();
  freethreads.clear();
  for (int i = 0; i < t; i++) {
    Thread* p = new Thread();
    freethreads.push_back(p);
  }
}
ThreadPool::~ThreadPool()
{
  for (vector<Thread*>::iterator i = freethreads.begin(); i != freethreads.end(); i++)
    delete *i;
}
void ThreadPool::create()
{
  ThreadsManage = thread(&ThreadPool::start, this);
  ThreadsManage.detach();
}
void ThreadPool::SetThreadNum(int t)
{
  ThreadNum = t;
}
int ThreadPool::GetThreadNum()
{
  return ThreadNum;
}
void ThreadPool::AddTask(Task* t)
{
  unique_lock<mutex> locker(task_mutex);
  tasklines.push(t);
  locker.unlock();
  task_cond.notify_one();
}
void ThreadPool::start()
{
  while (true) {
    if (isclose == true) {
      busy_mutex.lock();
      if (busythreads.size() != 0) {
        busy_mutex.unlock();
        continue;
      }
      busy_mutex.unlock();
      break;
    }

    free_mutex.lock();
    if (freethreads.size() == 0) {
      free_mutex.unlock();
      continue;
    }
    free_mutex.unlock();

    unique_lock<mutex> locker(task_mutex);
    while (tasklines.size() == 0)
      task_cond.wait(locker);

    Task* job = tasklines.front();
    tasklines.pop();
    locker.unlock();

    free_mutex.lock();
    Thread* t = freethreads.back();
    freethreads.pop_back();
    t->assign(job);
    free_mutex.unlock();

    busy_mutex.lock();
    busythreads.push_back(t);
    busy_mutex.unlock();

    t->start();
  }
}
void ThreadPool::close()
{
  isclose = true;
}

ThreadPool pool(THREAD_NUM);

int main(int argc, char* argv[])
{
  Util util;
  srand(time(NULL));

  //Notice that current_database is assigned in the child process, not in the father process
  //when used as endpoint, or when the database is indicated in command line, we can assign
  //current_database in father process(but this is resource consuming)
  //TODO+DEBUG: when will the restarting error?
  while (true) {
    //NOTICE: here we use 2 processes, father process is used for monitor and control(like, restart)
    //Child process is used to deal with web requests, can also has many threads
    pid_t fpid = fork();

    if (fpid == 0) {
      int ret = initialize(argc, argv);
      //TODO: which to use here!
      exit(ret);
    } else if (fpid > 0) {
      int status;
      waitpid(fpid, &status, 0);
      if (WIFEXITED(status)) {
        return 0;
      } else {
          cout << "Status Code:" << status << endl;
        cout << "Server stopped abnormally, restarting server..." << endl;
        //break;
      }
    } else {
      cout << "Failed to start server: deamon fork failure." << endl;
      return -1;
    }
  }

  return 0;
}

bool isNum(char* str)
{
  for (int i = 0; i < strlen(str); i++) {
    int tmp = (int)(str[i]);
    if (tmp < 48 || tmp > 57)
      return false;
  }
  return true;
}

int initialize(int argc, char* argv[])
{
  cout << "enter initialize." << endl;
  //Server restarts to use the original database
  //current_database = NULL;

  //users.insert(pair<std::string, struct User *>(ROOT_USERNAME, &root));

  //load system.db when initialize
  if (!boost::filesystem::exists("system.db")) {
    cout << "Can not find system.db." << endl;
    return -1;
  }
  struct DBInfo* temp_db = new DBInfo("system");
  temp_db->setCreator("root");
  already_build.insert(pair<std::string, struct DBInfo*>("system", temp_db));
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find("system");
  if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
    cout << "Unable to load the database system.db due to loss of lock." << endl;
    return -1;
  }
  system_database = new Database("system");
  bool flag = system_database->load();
  if (!flag) {
    cout << "Failed to load the database system.db." << endl;

    return -1;
  }
  databases.insert(pair<std::string, Database*>("system", system_database));
  pthread_rwlock_unlock(&(it_already_build->second->db_lock));

  /*//check databases that already built in the directory
	DIR * dir;
	struct dirent * ptr;
	dir = opendir("."); //open a directory
	while((ptr = readdir(dir)) != NULL) //get the names of the files in the directory
	{
		//cout << "name: " << ptr->d_name << endl;
		int nameLen = strlen(ptr->d_name);
		//cout << "nameLen: " << nameLen << endl;
		if(nameLen > 3 && std::string(ptr->d_name).substr(nameLen-3, nameLen) == ".db")
		{
			if(boost::filesystem::exists("./"+ std::string(ptr->d_name) +"/success.txt"))
			{
				std::string s = std::string(ptr->d_name).substr(0,nameLen-3);
				//cout << "already_built db_name: " << s << endl;
				pthread_rwlock_t temp_lock;
				pthread_rwlock_init(&temp_lock, NULL);
				already_build.insert(pair<std::string, pthread_rwlock_t>(s, temp_lock));
				pthread_rwlock_destroy(&temp_lock);
			}
			else
			{
				string cmd = "rm -r " + string(ptr->d_name);
				system(cmd.c_str());
			}
		}
	}
	closedir(dir);//close the pointer of the directory*/

  //insert user from system.db to user map
  DB2Map();

  HttpServer server;
  string db_name;
  if (argc == 1) {
    server.config.port = 9000;
    db_name = "";
  } else if (argc == 2) {
    if (isNum(argv[1])) {
      server.config.port = atoi(argv[1]);
      db_name = "";
    } else {
      server.config.port = 9000;
      db_name = argv[1];
    }
  } else {
    if (isNum(argv[1])) {
      server.config.port = atoi(argv[1]);
      db_name = argv[2];
    } else if (isNum(argv[2])) {
      server.config.port = atoi(argv[2]);
      db_name = argv[1];
    } else {
      cout << "wrong format of parameters, please input the server port and the database." << endl;
      return -1;
    }
  }
  port = server.config.port;
  cout << "server port: " << server.config.port << " database name: " << db_name << endl;
  //USAGE: then user can use http://localhost:port/ to visit the server or coding with RESTful API
  //HTTP-server at port 9000 using 1 thread
  //Unless you do more heavy non-threaded processing in the resources,
  //1 thread is usually faster than several threads

  //GET-example for the path /load/[db_name], responds with the matched string in path
  //For instance a request GET /load/db123 will receive: db123
  //server.resource["^/load/(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {

  //string success = db_name;
  cout << "Database system.db loaded successfully." << endl;

  string database = db_name;
  //if(current_database == NULL && database != "")
  //{
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    cout << "Your db name to be built should not end with \".db\"." << endl;
    return -1;
  } else if (database == "system") {
    cout << "You can not load system files." << endl;
    return -1;
  }
  cout << database << endl;
  Database* current_database = new Database(database);
  if (database.length() != 0) {
    if (!boost::filesystem::exists(database + ".db")) {
      cout << "Database " << database << ".db has not been built." << endl;
      return -1;
    }
    if (!boost::filesystem::exists(database + ".db/success.txt")) {
      cout << "Database " << database << ".db has not been built successfully." << endl;
      string cmd = "rm -r " + database + ".db";
      system(cmd.c_str());
      return -1;
    }

    bool flag = current_database->load();
    if (!flag) {
      cout << "Failed to load the database." << endl;
      delete current_database;
      current_database = NULL;
      return -1;
    }

    //string success = db_name;
    //already_build.insert(db_name);
    databases.insert(pair<std::string, Database*>(db_name, current_database));
    //}
  }
  //get the log name
  string namelog_name = QUERYLOG_PATH + NAMELOG_PATH;
  FILE* name_logfp = fopen(namelog_name.c_str(), "r+");
  string querylog_name;
  if (name_logfp == NULL) //file not exist, create one
  {
    name_logfp = fopen(namelog_name.c_str(), "w");
    querylog_name = Util::get_date_time();
    int index_space = querylog_name.find(' ');
    querylog_name = querylog_name.replace(index_space, 1, 1, '_');
    fprintf(name_logfp, "%s", querylog_name.c_str());
  } else {
    char name_char[100];
    fscanf(name_logfp, "%s", &name_char);
    querylog_name = name_char;
  }
  fclose(name_logfp);
  //cout << "querylog_name: " << querylog_name << endl;
  //open the query log
  queryLog = QUERYLOG_PATH + querylog_name + ".log";
  cout << "queryLog: " << queryLog << endl;
  query_logfp = fopen(queryLog.c_str(), "a");
  if (query_logfp == NULL) {
    cerr << "open query log error" << endl;
    return -1;
  }
  long querylog_size = ftell(query_logfp);
  //cout << "querylog_size: " << querylog_size << endl;
  //cout << "Util::getTimeName: " << Util::getTimeName() << endl;
  //cout << "Util::get_cur_time: " << Util::get_cur_time() << endl;
  //cout << "Util::getTimeString: " << Util::getTimeString() << endl;
  //cout << "Util::get_date_time: " << Util::get_date_time() << endl;

  string cmd = "lsof -i:" + Util::int2string(server.config.port) + " > system.db/ep.txt";
  system(cmd.c_str());
  fstream ofp;
  ofp.open("system.db/ep.txt", ios::in);
  int ch = ofp.get();
  if (!ofp.eof()) {
    ofp.close();
    cout << "Port " << server.config.port << " is already in use." << endl;
    string cmd = "rm system.db/ep.txt";
    system(cmd.c_str());
    return -1;
  }
  ofp.close();
  cmd = "rm system.db/ep.txt";
  system(cmd.c_str());

  system_password = Util::int2string(rand()) + Util::int2string(rand());
  ofp.open("system.db/password" + Util::int2string(server.config.port) + ".txt", ios::out);
  ofp << system_password;
  ofp.close();

//time_t cur_time = time(NULL);
//long time_backup = Util::read_backup_time();
//long next_backup = cur_time - (cur_time - time_backup) % Util::gserver_backup_interval + Util::gserver_backup_interval;
//NOTICE: no need to backup for endpoint
//TODO: we give up the backup function here
#ifndef ONLY_READ
//scheduler = start_thread(func_scheduler);
#endif

  pool.create();
//pthread_rwlock_init(&database_load_lock, NULL);

#ifndef SPARQL_ENDPOINT

  //GET-example for the path /?operation=build&db_name=[db_name]&ds_path=[ds_path]&username=[username]&password=[password], responds with the matched string in path
  //i.e. database name and dataset path
  server.resource["^/%3[F|f]operation%3[D|d]build%26db_name%3[D|d](.*)%26ds_path%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		build_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=build&db_name=(.*)&ds_path=(.*)&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		build_handler(server, response, request, "GET");
  };

  //POST-example for the path /build, responds with the matched string in path
  server.resource["/build"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		build_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=load&db_name=[db_name]&username=[username]&password=[password], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]load%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		load_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=load&db_name=(.*)&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		load_handler(server, response, request, "GET");
  };

  //POST-example for the path /load, responds with the matched string in path
  server.resource["/load"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		load_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=unload&db_name=[db_name]&username=[username]&password=[password], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]unload%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		unload_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=unload&db_name=(.*)&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		unload_handler(server, response, request, "GET");
  };

  //POST-example for the path /unload, responds with the matched string in path
  server.resource["/unload"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		unload_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=login&username=[username]&password=[password], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]login%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		login_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=login&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		login_handler(server, response, request, "GET");
  };

  //POST-example for the path /login, responds with the matched string in path
  server.resource["/login"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		login_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=user&type=[type]&username1=[username1]&password1=[password1]&username2=[username2]&addition=[password2 || db_name], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]user%26type%3[D|d](.*)%26username1%3[D|d](.*)%26password1%3[D|d](.*)%26username2%3[D|d](.*)%26addition%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		user_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=user&type=(.*)&username1=(.*)&password1=(.*)&username2=(.*)&addition=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		user_handler(server, response, request, "GET");
  };

  //POST-example for the path /user, responds with the matched string in path
  server.resource["/user"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		user_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=showUser&username=[username]&password=[password], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]showUser%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		showUser_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=showUser&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		showUser_handler(server, response, request, "GET");
  };

  //POST-example for the path /showUser, responds with the matched string in path
  server.resource["/showUser"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		showUser_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=query&username=[username]&password=[password]&db_name=[db_name]&format=[format]&sparql=[sparql], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]query%26username%3[D|d](.*)%26password%3[D|d](.*)%26db_name%3[D|d](.*)%26format%3[D|d](.*)%26sparql%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		query_handler1(server, response, request, "GET");
  };

  server.resource["^/?operation=query&username=(.*)&password=(.*)&db_name=(.*)&format=(.*)&sparql=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		query_handler1(server, response, request, "GET");
  };

  //POST-example for the path /query, responds with the matched string in path
  server.resource["/query"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		query_handler1(server, response, request, "POST");
  };

  //GET-example for the path /?operation=export&db_name=[db_name]&ds_path=[ds_path]&username=[username]&password=[password], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]export%26db_name%3[D|d](.*)%26ds_path%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		export_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=export&db_name=(.*)&ds_path=(.*)&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		export_handler(server, response, request, "GET");
  };

  //POST-example for the path /export, responds with the matched string in path
  server.resource["/export"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		export_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=check&username=[username]&password=[password], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]check%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		check_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=check&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		check_handler(server, response, request, "GET");
  };

  //POST-example for the path /check, responds with the matched string in path
  server.resource["/check"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		check_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=drop&db_name=[db_name]&username=[username]&password=[password]&is_backup=[true|false], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]drop%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)%26is_backup%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		drop_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=drop&db_name=(.*)&username=(.*)&password=(.*)&is_backup=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		drop_handler(server, response, request, "GET");
  };

  //POST-example for the path /drop, responds with the matched string in path
  server.resource["/drop"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		drop_handler(server, response, request, "POST");
  };

  server.resource["^/%3[F|f]operation%3[D|d]refresh%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		refresh_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=refresh&db_name=(.*)&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		refresh_handler(server, response, request, "GET");
  };

  //POST-example for the path /drop, responds with the matched string in path
  server.resource["/refresh"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		refresh_handler(server, response, request, "POST");
  };

#endif

  //GET-example for the path /?operation=query&db_name=[db_name]&format=[format]&sparql=[sparql], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]query%26db_name%3[D|d](.*)%26format%3[D|d](.*)%26sparql%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		query_handler0(server, response, request, "GET");
  };

  server.resource["^/?operation=query&db_name=(.*)&format=(.*)&sparql=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		query_handler0(server, response, request, "GET");
  };

  //POST-example for the path /query0, responds with the matched string in path
  server.resource["/query0"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		query_handler0(server, response, request, "POST");
  };

  //GET-example for the path /?operation=monitor&db_name=[db_name]&username=[username]&password=[password], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]monitor%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		monitor_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=monitor&db_name=(.*)&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		monitor_handler(server, response, request, "GET");
  };

  //POST-example for the path /monitor, responds with the matched string in path
  server.resource["/monitor"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		monitor_handler(server, response, request, "POST");
  };

  server.resource["^/%3[F|f]operation%3[D|d]stop%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		bool flag = stop_handler(server, response, request);
		if (flag)
			exit(0);
  };
  server.resource["^/?operation=stop&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		bool flag = stop_handler(server, response, request);
		if(flag)
			exit(0);
  };

  //GET-example for the path /?operation=checkpoint&db_name=[db_name]&username=[username]&password=[password], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]checkpoint%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		checkpoint_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=checkpoint&db_name=(.*)&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		checkpoint_handler(server, response, request, "GET");
  };

  //POST-example for the path /checkpoint, responds with the matched string in path
  server.resource["/checkpoint"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		checkpoint_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=show&username=[username]&password=[password], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]show%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		show_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=show&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		show_handler(server, response, request, "GET");
  };

  //POST-example for the path /show, responds with the matched string in path
  server.resource["/show"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		show_handler(server, response, request, "POST");
  };

  server.resource["^/%3[F|f]operation%3[D|d]getCoreVersion%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		getCoreVersion_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=getCoreVersion&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		getCoreVersion_handler(server, response, request, "GET");
  };

  //POST-example for the path /getCoreVersion, responds with the matched string in path
  server.resource["/getCoreVersion"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		getCoreVersion_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=setCoreVersion&username=[username]&password=[password]&version=[version], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]setCoreVersion%26username%3[D|d](.*)%26password%3[D|d](.*)%26version%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		setCoreVersion_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=setCoreVersion&username=(.*)&password=(.*)&version=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		setCoreVersion_handler(server, response, request, "GET");
  };

  //POST-example for the path /setCoreVersion, responds with the matched string in path
  server.resource["/setCoreVersion"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		setCoreVersion_handler(server, response, request, "POST");
  };

  //initVersion
  server.resource["^/%3[F|f]operation%3[D|d]initVersion%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		initVersion_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=initVersion&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		initVersion_handler(server, response, request, "GET");
  };

  //POST-example for the path /getAPIVersion, responds with the matched string in path
  server.resource["/initVersion"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		initVersion_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=getAPIVersion&username=[username]&password=[password], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]getAPIVersion%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		getAPIVersion_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=getAPIVersion&username=(.*)&password=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		getAPIVersion_handler(server, response, request, "GET");
  };

  //POST-example for the path /getAPIVersion, responds with the matched string in path
  server.resource["/getAPIVersion"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		string req_url = request->path;
		cout << "request url: " << req_url << endl;
		getAPIVersion_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=setAPIVersion&username=[username]&password=[password]&version=[version], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]setAPIVersion%26username%3[D|d](.*)%26password%3[D|d](.*)%26version%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		setAPIVersion_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=setAPIVersion&username=(.*)&password=(.*)&version=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		setAPIVersion_handler(server, response, request, "GET");
  };

  //POST-example for the path /setAPIVersion, responds with the matched string in path
  server.resource["/setAPIVersion"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		setAPIVersion_handler(server, response, request, "POST");
  };

  //USAGE: in endpoint, if user choose to display via html, but not display all because the result's num is too large.
  //Then, user can choose to download all results in TXT format, and this URL is used for download in this case
  //WARN: this URL rule should not be published!
  //
  //NOTICE: we need to remove temp result files after download once, otherwise the disk of server will be run out easily.
  //
  //Safety: user can attack by using this URL and point a specific filePath to be removed, which is not the download one!
  //We do not have the http session ID here and can not provide best safety, but we can set the filePath complicated.
  //(then attacker is hard to guess, and the directory is restricted)
  //BETTER+TODO: associate the thread/session ID with download fileName, otherwise error may come in parallism

  //GET-example for the path /?operation=delete&filepath=[filepath], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]delete%26filepath%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		delete_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=delete&filepath=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		delete_handler(server, response, request, "GET");
  };

  //POST-example for the path /delete, responds with the matched string in path
  server.resource["/delete"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		delete_handler(server, response, request, "POST");
  };

  //GET-example for the path /?operation=download&filepath=[filepath], responds with the matched string in path
  server.resource["^/%3[F|f]operation%3[D|d]download%26filepath%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		download_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=download&filepath=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		download_handler(server, response, request, "GET");
  };

  //POST-example for the path /download, responds with the matched string in path
  server.resource["/download"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		download_handler(server, response, request, "POST");
  };

  //Default GET-example. If no other matches, this anonymous function will be called.
  //Will respond with content in the web/-directory, and its subdirectories.
  //Default file: index.html
  //Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
  server.default_resource["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		default_handler(server, response, request);
  };

  server.default_resource["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		default_handler(server, response, request);
  };

  server.resource["^/%3[F|f]operation%3[D|d]backup%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)%26path%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		backup_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=backup&db_name=(.*)&username=(.*)&password=(.*)&path=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		backup_handler(server, response, request, "GET");
  };

  //POST-example for the path /unload, responds with the matched string in path
  server.resource["/restore"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		backup_handler(server, response, request, "POST");
  };

  server.resource["^/%3[F|f]operation%3[D|d]restore%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)%26path%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		restore_handler(server, response, request, "GET");
  };

  server.resource["^/?operation=restore&db_name=(.*)&username=(.*)&password=(.*)&path=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		restore_handler(server, response, request, "GET");
  };

  //POST-example for the path /unload, responds with the matched string in path
  server.resource["/restore"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		restore_handler(server, response, request, "POST");
  };

  thread server_thread([&server]() {
			//handle the Ctrl+C signal
		    signal(SIGINT, signalHandler);
            //Start server
            server.start();
			cout<<"check: server started"<<endl;
  });

  //Wait for server to start so that the client can connect
  this_thread::sleep_for(chrono::seconds(1));

  //Client examples
  //HttpClient client("localhost:9000");
  //auto r1=client.request("GET", "/match/123");
  //cout << r1->content.rdbuf() << endl;

  // string json_string="{\"firstName\": \"John\",\"lastName\": \"Smith\",\"age\": 25}";
  //auto r2=client.request("POST", "/string", json_string);

  //cout << r2->content.rdbuf() << endl;

  //auto r3=client.request("POST", "/json", json_string);
  //cout << r3->content.rdbuf() << endl;

  server_thread.join();
  cout << "check: server stoped" << endl;
  //pthread_rwlock_destroy(&database_load_lock);
  pthread_rwlock_destroy(&databases_map_lock);
  pthread_rwlock_destroy(&already_build_map_lock);
  pthread_rwlock_destroy(&users_map_lock);

  return 0;
}

void signalHandler(int signum)
{
  pthread_rwlock_rdlock(&databases_map_lock);
  std::map<std::string, Database*>::iterator iter;
  for (iter = databases.begin(); iter != databases.end(); iter++) {
    string database_name = iter->first;
    if (database_name == "system")
      continue;
    Database* current_database = iter->second;
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(database_name);
    pthread_rwlock_unlock(&already_build_map_lock);
    if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
      string error = "Unable to checkpoint due to loss of lock";
      cout << error << endl;
      pthread_rwlock_unlock(&databases_map_lock);
      //cout << "Stop server failed." << endl;
      exit(signum);
    }
    current_database->save();
    delete current_database;
    current_database = NULL;
    //cout << "Database " << database_name << " saved successfully." << endl;
    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
  }
  system_database->save();
  delete system_database;
  system_database = NULL;
  //cout << "Database system saved successfully." << endl;
  pthread_rwlock_unlock(&databases_map_lock);

  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build;
  for (it_already_build = already_build.begin(); it_already_build != already_build.end(); it_already_build++) {
    struct DBInfo* temp_db = it_already_build->second;
    delete temp_db;
    temp_db = NULL;
  }
  pthread_rwlock_unlock(&already_build_map_lock);

  string cmd = "rm system.db/password" + Util::int2string(port) + ".txt";
  system(cmd.c_str());
  //cout << "Server stopped." << endl;
  exit(signum);
}

//QUERY: can server.send() in default_resource_send run in parallism?

//TCP is slower than UDP, but more safely, and can support large data transfer well
//http://blog.csdn.net/javaniuniu/article/details/51419348
void default_resource_send(const HttpServer& server, const shared_ptr<HttpServer::Response>& response,
                           const shared_ptr<ifstream>& ifs)
{
  //read and send 128 KB at a time
  static vector<char> buffer(131072); // Safe when server is running on one thread
  streamsize read_length;
  cout << "int 0" << endl;
  if ((read_length = ifs->read(&buffer[0], buffer.size()).gcount()) > 0) {
    response->write(&buffer[0], read_length);
    cout << "int 1" << endl;
    if (read_length == static_cast<streamsize>(buffer.size())) {
      cout << "int 2" << endl;
      server.send(response, [&server, response, ifs](const boost::system::error_code& ec) {
	cout<<"int 3"<<endl;
                    if(!ec)
                    default_resource_send(server, response, ifs);
                    else
                    cerr << "Connection interrupted" << endl;
      });
    }
  }
}

pthread_t start_thread(void* (*_function)(void*))
{
  pthread_t thread;
  if (pthread_create(&thread, NULL, _function, NULL) == 0) {
    return thread;
  }
  return 0;
}

bool stop_thread(pthread_t _thread)
{
  return pthread_kill(_thread, SIGTERM) == 0;
}

//DEBUG+TODO: the whole process exits and need to reload the database
//(too slow: quit and restart)
void* func_timer(void* _args)
{
  signal(SIGTERM, thread_sigterm_handler);
  sleep(Util::gserver_query_timeout);
  cerr << "Query out of time." << endl;
  //TODO: not use SIGTERM and abort() here, which will cause process to quit abnormally
  //here shoudl just end the timer thread
  abort();
}
/*
void* func_scheduler(void* _args) {
	signal(SIGTERM, thread_sigterm_handler);
	while (true) {
		time_t cur_time = time(NULL);
		while (cur_time >= next_backup) {
			next_backup += Util::gserver_backup_interval;
		}
		sleep(next_backup - cur_time);
		if (!current_database->backup()) {
			return NULL;
		}
	}
}
*/
void thread_sigterm_handler(int _signal_num)
{
  pthread_exit(0);
}

//NOTICE: what if several requests to delete the same file? we assume all are valid and no attackers
void delete_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is delete" << endl;

  string filepath;

  if (RequestType == "GET") {
    filepath = request->path_match[1];
    filepath = UrlDecode(filepath);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    filepath = document["filepath"].GetString();
  }

  try
  {
    //set the home directory of the web server
    //NOTICE: we use .tmp/web instead of . to avoid attack: delete other files rather than the download one
    auto root_path = boost::filesystem::canonical(".tmp/web");
    auto path = boost::filesystem::canonical(root_path / filepath);
    cout << "path: " << path << endl;
    //Check if path is within root_path
    if (distance(root_path.begin(), root_path.end()) > distance(path.begin(), path.end()) || !equal(root_path.begin(), root_path.end(), path.begin()))
      throw invalid_argument("path must be within root path.");

    if ((boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path))) {
      //delete file in delpath.
      //char name[60];
      //strcpy(name, path.c_str());
      //cout << "name: " << name << endl;
      int ret = remove(path.c_str());
      if (ret == -1) {
        cout << "could not delete file." << endl;
        perror("remove");
      } else {
        cout << "file delete." << endl;
        //Notice: if file deleted successfully, service need to response to the browser, if not the browser will not execute the call-back function.
        string success = "file delete successfully.";
        string resJson = CreateJson(0, success, 0);
        *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

        //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
      }
    }
  }
  catch (const exception& e)
  {
    //cout<<"can not open file!!!"<<endl;
    string content = "Could not open path " + request->path + ": " + e.what();

    string resJson = CreateJson(101, content, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
  }
}

bool delete_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&delete_thread, response, request, RequestType);
  t.detach();
  //delete_thread(response, request);
  return true;
}

void download_thread(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is download" << endl;

  string filepath;

  if (RequestType == "GET") {
    filepath = request->path_match[1];
    filepath = UrlDecode(filepath);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    filepath = document["filepath"].GetString();
  }

  try
  {
    //set the home directory of the web server
    //NOTICE: we use .tmp/web instead of . to avoid attack: delete other files rather than the download one
    cout << "filepath: " << filepath << endl;
    auto root_path = boost::filesystem::canonical(".tmp/web");
    cout << "root_path: " << root_path << endl;
    auto path = boost::filesystem::canonical(root_path / filepath);
    cout << "path: " << path << endl;
    //Check if path is within root_path
    if (distance(root_path.begin(), root_path.end()) > distance(path.begin(), path.end()) || !equal(root_path.begin(), root_path.end(), path.begin()))
      throw invalid_argument("path must be within root path.");

    std::string cache_control, etag;

    // Uncomment the following line to enable Cache-Control
    // cache_control="Cache-Control: max-age=86400\r\n";

    //return file in path
    auto ifs = make_shared<ifstream>();
    ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

    if (*ifs) {
      //cout<<"open file!!!"<<endl;
      auto length = ifs->tellg();
      ifs->seekg(0, ios::beg);
      //!Notice: remember to set the Content-Disposition and the Content-type to let the browser to download.
      *response << "HTTP/1.1 200 OK\r\n"
                << "Content-Length: " << length << "\r\n";
      *response << "Content-Disposition: attachment; filename=sparql.txt"
                << "\r\n";
      *response << "Content-Type: application/octet-stream"
                << "\r\n\r\n";
      default_resource_send(server, response, ifs);
    } else
      throw invalid_argument("could not read file.");
  }
  catch (const exception& e)
  {
    //cout<<"can not open file!!!"<<endl;
    string content = "Could not open path " + request->path + ": " + e.what();
    string resJson = CreateJson(101, content, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json \r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
  }
}

bool download_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  download_thread(server, response, request, RequestType);
  //thread t(&download_thread, server, response, request);
  //t.detach();
  return true;
}

void build_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is build" << endl;

  string db_name;
  string db_path;
  string username;
  string password;

  if (RequestType == "GET") {
    db_name = request->path_match[1];
    db_path = request->path_match[2];
    username = request->path_match[3];
    password = request->path_match[4];
    db_name = UrlDecode(db_name);
    db_path = UrlDecode(db_path);
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    db_path = document["ds_path"].GetString();
    username = document["username"].GetString();
    password = document["password"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);

  cout << "check identity successfully." << endl;

  //check if the db_path is the path of system.nt
  if (db_path == SYSTEM_PATH) {
    string error = "You have no rights to access system files";
    string resJson = CreateJson(205, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  //check if the db_name is system
  if (db_name == "system") {
    string error = "Your db name to be built can not be system.";
    string resJson = CreateJson(206, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  if (db_name == "" || db_path == "") {
    string error = "Exactly 2 arguments required!";
    // error = db_name + " " + db_path;
    string resJson = CreateJson(905, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }

  string database = db_name;
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    string error = "Your db name to be built should not end with \".db\".";
    string resJson = CreateJson(202, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }

  //check if database named [db_name] is already built
  pthread_rwlock_rdlock(&already_build_map_lock);
  if (already_build.find(db_name) != already_build.end()) {
    string error = "database already built.";
    string resJson = CreateJson(201, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    pthread_rwlock_unlock(&already_build_map_lock);
    return;
  }
  pthread_rwlock_unlock(&already_build_map_lock);

  //database += ".db";
  string dataset = db_path;

  //TODO:MODIFY
  //if(pthread_rwlock_tryrdlock(&database_load_lock) != 0)
  //{
  //string error = "Unable to build due to the loss of lock!";
  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
  //return;
  //}
  //if(current_database != NULL)
  //{
  //	string error = "Please unload your database first.";
  //	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
  //	pthread_rwlock_unlock(&database_load_lock);
  //	return;
  //}

  cout << "Import dataset to build database..." << endl;
  cout << "DB_store: " << database << "\tRDF_data: " << dataset << endl;
  int len = database.length();

  //pthread_rwlock_unlock(&database_load_lock);
  //if(pthread_rwlock_trywrlock(&database_load_lock) != 0)
  //{
  //	string error = "Unable to build due to the loss of lock!";
  //	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
  //	return;
  //}

  Database* current_database = new Database(database);
  bool flag = current_database->build(dataset);
  delete current_database;
  current_database = NULL;

  if (!flag) {
    string error = "Import RDF file to database failed.";
    string cmd = "rm -r " + database + ".db";
    system(cmd.c_str());
    string resJson = CreateJson(204, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;

    //	pthread_rwlock_unlock(&database_load_lock);
    return;
  }

  ofstream f;
  f.open("./" + database + ".db/success.txt");
  f.close();

  //by default, one can query or load or unload the database that is built by itself, so add the database name to the privilege set of the user
  if (addPrivilege(username, "query", db_name) == 0 || addPrivilege(username, "load", db_name) == 0 || addPrivilege(username, "unload", db_name) == 0 || addPrivilege(username, "backup", db_name) == 0 || addPrivilege(username, "restore", db_name) == 0 || addPrivilege(username, "export", db_name) == 0) {
    string error = "add query or load or unload privilege failed.";
    string resJson = CreateJson(912, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    return;
  }
  cout << "add query and load and unload privilege succeed after build." << endl;

  //add database information to system.db
  pthread_rwlock_wrlock(&already_build_map_lock);
  cout << "already_build_map_lock acquired." << endl;
  string time = Util::get_date_time();
  struct DBInfo* temp_db = new DBInfo(db_name, username, time);
  already_build.insert(pair<std::string, struct DBInfo*>(db_name, temp_db));
  pthread_rwlock_unlock(&already_build_map_lock);
  string update = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." + "<" + db_name + "> <built_by> <" + username + "> ." + "<" + db_name + "> <built_time> \"" + time + "\".}";
  updateSys(update);
  cout << "database add done." << endl;
  // string success = db_name + " " + db_path;
  string success = "Import RDF file to database done.";
  string resJson = CreateJson(0, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
  //	pthread_rwlock_unlock(&database_load_lock);
}

bool build_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&build_thread, response, request, RequestType);
  t.detach();
  return true;
}

void load_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is load" << endl;

  string db_name;
  string username;
  string password;

  if (RequestType == "GET") {
    db_name = request->path_match[1];
    username = request->path_match[2];
    password = request->path_match[3];
    db_name = UrlDecode(db_name);
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    username = document["username"].GetString();
    password = document["password"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);

  cout << "check identity successfully." << endl;

  //check if the db_name is system
  if (db_name == "system") {
    string error = "no load privilege, operation failed.";
    string resJson = CreateJson(302, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  //	string db_name = argv[1];
  if (db_name == "") {
    string error = "Exactly 1 argument is required!";
    string resJson = CreateJson(904, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 ok\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }

  string database = db_name;
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    //cout << "Your db name to be built should not end with \".db\"." << endl;
    string error = "Your db name to be built should not end with \".db\".";
    string resJson = CreateJson(202, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }

  //check if database named [db_name] is already build.
  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
  if (it_already_build == already_build.end()) {
    string error = "Database not built yet.";
    string resJson = CreateJson(203, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    pthread_rwlock_unlock(&already_build_map_lock);
    return;
  }
  pthread_rwlock_unlock(&already_build_map_lock);

  //check if database named [db_name] is already load
  pthread_rwlock_rdlock(&databases_map_lock);
  if (databases.find(db_name) != databases.end()) {
    string error = "database already load.";
    string resJson = CreateJson(301, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    pthread_rwlock_unlock(&databases_map_lock);
    return;
  }
  pthread_rwlock_unlock(&databases_map_lock);

  //check privilege
  if (checkPrivilege(username, "load", db_name) == 0) {
    string error = "no load privilege, operation failed.";
    string resJson = CreateJson(302, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    return;
  }
  cout << "check privilege successfully." << endl;

  //	if(pthread_rwlock_tryrdlock(&database_load_lock) != 0)
  //	{
  //	    string error = "Unable to load due to loss of lock";
  //		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
  //		return;
  //	}

  //database += ".db";
  /*
	if(current_database != NULL)
	{
		//cout << "Please unload your current database first." <<endl;
		string error = "Please unload your current database first.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		pthread_rwlock_unlock(&database_load_lock);
		return;
	}
*/
  cout << database << endl;

  //	pthread_rwlock_unlock(&database_load_lock);
  //	if(pthread_rwlock_trywrlock(&database_load_lock) != 0)
  //	{
  //	    string error = "Unable to load due to loss of lock";
  //		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
  //		return;
  //	}

  if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
    string error = "Unable to load due to loss of lock";
    string resJson = CreateJson(303, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }
  Database* current_database = new Database(database);
  bool flag = current_database->load();
  //delete current_database;
  //current_database = NULL;
  //cout << "load done." << endl;
  if (!flag) {
    string error = "Failed to load the database.";
    string resJson = CreateJson(305, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //pthread_rwlock_unlock(&database_load_lock);
    return;
  }

  //TODO+DEBUG: if we use the scheduler here, then each load will bring an asleep thread!
  //time_t cur_time = time(NULL);
  //long time_backup = Util::read_backup_time();
  //long next_backup = cur_time - (cur_time - time_backup) % Util::gserver_backup_interval + Util::gserver_backup_interval;
  //scheduler = start_thread(func_scheduler);

  //string success = db_name;
  //cout << "Database loaded successfully." << endl;
  pthread_rwlock_wrlock(&databases_map_lock);
  databases.insert(pair<std::string, Database*>(db_name, current_database));
  pthread_rwlock_unlock(&databases_map_lock);

  cout << "database insert done." << endl;
  string success = "Database loaded successfully.";
  string resJson = CreateJson(0, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
  pthread_rwlock_unlock(&(it_already_build->second->db_lock));
}

bool load_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&load_thread, response, request, RequestType);
  t.detach();
  return true;
}

void unload_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is unload" << endl;

  string db_name;
  string username;
  string password;

  if (RequestType == "GET") {
    db_name = request->path_match[1];
    username = request->path_match[2];
    password = request->path_match[3];
    db_name = UrlDecode(db_name);
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    username = document["username"].GetString();
    password = document["password"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);

  cout << "check identity successfully." << endl;

  //check if the db_name is system
  if (db_name == "system") {
    string error = "no unload privilege, operation failed.";
    string resJson = CreateJson(601, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  //check privilege
  if (checkPrivilege(username, "unload", db_name) == 0) {
    string error = "no unload privilege, operation failed.";
    string resJson = CreateJson(601, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    return;
  }
  cout << "check privilege successfully." << endl;
  /*
	if(current_database == NULL)
	{
		string error = "No database used now.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		pthread_rwlock_unlock(&database_load_lock);
		return;
	}
	cout << database << endl;
	if(current_database->getName() != database)
	{
		string error = "Database Name not matched.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		pthread_rwlock_unlock(&database_load_lock);
		return;
	}
	pthread_rwlock_unlock(&database_load_lock);
	if(pthread_rwlock_trywrlock(&database_load_lock) != 0)
	{
	    string error = "Unable to unload due to loss of lock";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return;
	}
*/
  pthread_rwlock_wrlock(&databases_map_lock);
  std::map<std::string, Database*>::iterator iter = databases.find(db_name);
  if (iter == databases.end()) {
    string error = "Database not load yet.";
    string resJson = CreateJson(304, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    pthread_rwlock_unlock(&databases_map_lock);
    return;
  }
  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
  pthread_rwlock_unlock(&already_build_map_lock);
  if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
    string error = "Unable to unload due to loss of lock";
    string resJson = CreateJson(602, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }
  Database* current_database = iter->second;
  delete current_database;
  current_database = NULL;
  databases.erase(db_name);
  string success = "Database unloaded.";
  string resJson = CreateJson(0, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
  //	pthread_rwlock_unlock(&database_load_lock);
  pthread_rwlock_unlock(&(it_already_build->second->db_lock));
  pthread_rwlock_unlock(&databases_map_lock);
}

bool unload_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&unload_thread, response, request, RequestType);
  t.detach();
  return true;
}

void drop_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is drop" << endl;

  string db_name;
  string username;
  string password;
  string is_backup;

  if (RequestType == "GET") {
    db_name = request->path_match[1];
    username = request->path_match[2];
    password = request->path_match[3];
    is_backup = request->path_match[4];
    db_name = UrlDecode(db_name);
    username = UrlDecode(username);
    password = UrlDecode(password);
    is_backup = UrlDecode(is_backup);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    username = document["username"].GetString();
    password = document["password"].GetString();
    is_backup = document["is_backup"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);

  cout << "check identity successfully." << endl;

  if (checkPrivilege(username, "drop", db_name) == 0) {
    string error = "no load privilege, operation failed.";
    string resJson = CreateJson(302, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    return;
  }
  cout << "check privilege successfully." << endl;

  //check if the db_name is system
  if (db_name == "system") {
    string error = "no drop privilege, operation failed.";
    string resJson = CreateJson(701, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  //check if database named [db_name] is already build.
  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
  if (it_already_build == already_build.end()) {
    string error = "Database not built yet.";
    string resJson = CreateJson(203, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&already_build_map_lock);
    return;
  }
  pthread_rwlock_unlock(&already_build_map_lock);

  //check privilege
  string creator = it_already_build->second->getCreator();
  if (creator != username) {
    if (username != ROOT_USERNAME) {
      string error = "no drop privilege, operation failed.";
      string resJson = CreateJson(701, error, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
      return;
    }
  }
  cout << "check privilege successfully." << endl;

  //if database named [db_name] is loaded, unload it firstly
  pthread_rwlock_wrlock(&databases_map_lock);
  std::map<std::string, Database*>::iterator iter = databases.find(db_name);
  if (iter != databases.end()) {
    if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
      string error = "Unable to unload due to loss of lock";
      string resJson = CreateJson(602, error, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

      //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
      pthread_rwlock_unlock(&databases_map_lock);
      return;
    }
    Database* current_database = iter->second;
    delete current_database;
    current_database = NULL;
    databases.erase(db_name);
    //string success = "Database unloaded.";
    //string resJson = CreateJson(0, success, 0);
    //*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
  }
  pthread_rwlock_unlock(&databases_map_lock);

  //drop database named [db_name]
  pthread_rwlock_rdlock(&already_build_map_lock);
  struct DBInfo* temp_db = it_already_build->second;
  string time = temp_db->getTime();
  delete temp_db;
  temp_db = NULL;
  already_build.erase(db_name);
  string success = "Database " + db_name + " dropped.";
  string resJson = CreateJson(0, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
  pthread_rwlock_unlock(&already_build_map_lock);

  string update = "DELETE DATA {<" + db_name + "> <database_status> \"already_built\"." + "<" + db_name + "> <built_by> <" + creator + "> ." + "<" + db_name + "> <built_time> \"" + time + "\".}";
  updateSys(update);
  string cmd;
  if (is_backup == "false")
    cmd = "rm -r " + db_name + ".db";
  else if (is_backup == "true")
    cmd = "mv " + db_name + ".db " + db_name + ".bak";
  system(cmd.c_str());
  return;
}

bool drop_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&drop_thread, response, request, RequestType);
  t.detach();
  return true;
}

void writeLog(FILE* fp, string _info)
{
  //Another way to locka many: lock(lk1, lk2...)
  query_log_lock.lock();
  fprintf(fp, "%s", _info.c_str());
  Util::Csync(fp);
  long logSize = ftell(fp);
  cout << "logSize: " << logSize << endl;
  //if log size too big, we create a new log file
  if (logSize > MAX_QUERYLOG_size) {
    //close the old query log file
    fclose(query_logfp);
    //create and open a new query log file, then save the log name to name.log
    string querylog_name = Util::get_date_time();
    int index_space = querylog_name.find(' ');
    querylog_name = querylog_name.replace(index_space, 1, 1, '_');
    string namelog_name = QUERYLOG_PATH + NAMELOG_PATH;
    FILE* name_logfp = fopen(namelog_name.c_str(), "w");
    fprintf(name_logfp, "%s", querylog_name.c_str());

    queryLog = QUERYLOG_PATH + querylog_name + ".log";
    //queryLog = querylog_name;
    cout << "querLog: " << queryLog << endl;
    query_logfp = fopen(queryLog.c_str(), "a");
    if (query_logfp == NULL) {
      cerr << "open query log error" << endl;
    }
  }
  //another way to get the log file size
  /*
	struct stat statbuf;
	stat(queryLog.c_str(), &statbuf);
	int size = statbuf.st_size;
	cout << "logSize in statbuf: " << size << endl;
	*/
  query_log_lock.unlock();
}

void export_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is export" << endl;

  string db_name;
  string db_path;
  string username;
  string password;

  if (RequestType == "GET") {
    db_name = request->path_match[1];
    db_path = request->path_match[2];
    username = request->path_match[3];
    password = request->path_match[4];
    db_name = UrlDecode(db_name);
    db_path = UrlDecode(db_path);
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    db_path = document["ds_path"].GetString();
    username = document["username"].GetString();
    password = document["password"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  if (checkPrivilege(username, "export", db_name) == 0) {
    string error = "no load privilege, operation failed.";
    string resJson = CreateJson(302, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    return;
  }
  cout << "check privilege successfully." << endl;

  //check if the db_name is system
  if (db_name == "system") {
    string error = "You can not export the system database.";
    string resJson = CreateJson(206, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  if (db_name == "" || db_path == "") {
    string error = "Exactly 2 arguments required!";
    string resJson = CreateJson(905, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  string database = db_name;
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    string error = "Your db name to be built should not end with \".db\".";
    string resJson = CreateJson(202, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  //check if database named [db_name] is already built
  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
  if (it_already_build == already_build.end()) {
    string error = "Database not built yet.";
    string resJson = CreateJson(203, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    pthread_rwlock_unlock(&already_build_map_lock);
    return;
  }
  cout << log_prefix << "HTTP: this is export check success." << endl;
  pthread_rwlock_unlock(&already_build_map_lock);

  boost::filesystem::path filePath(db_path);
  if (!boost::filesystem::exists(filePath.parent_path()))
    boost::filesystem::create_directories(filePath.parent_path());
  cout << log_prefix << "export: build file success." << endl;
  //check if database named [db_name] is already load
  Database* current_database;
  pthread_rwlock_rdlock(&databases_map_lock);
  std::map<std::string, Database*>::iterator iter = databases.find(db_name);
  if (iter == databases.end()) {
    pthread_rwlock_unlock(&databases_map_lock);

    //check privilege
    if (checkPrivilege(username, "load", db_name) == 0) {
      string error = "no load privilege, operation failed.";
      string resJson = CreateJson(302, error, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
      return;
    }
    cout << "export:check privilege successfully." << endl;

    if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
      string error = "Unable to load due to loss of lock";
      string resJson = CreateJson(303, error, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
      return;
    }
    current_database = new Database(db_name);
    bool flag = current_database->load();

    if (!flag) {
      string error = "Failed to load the database.";
      string resJson = CreateJson(305, error, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
      return;
    }

    pthread_rwlock_wrlock(&databases_map_lock);
    databases.insert(pair<std::string, Database*>(db_name, current_database));
    pthread_rwlock_unlock(&databases_map_lock);

    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
  } else {
    current_database = iter->second;
    pthread_rwlock_unlock(&databases_map_lock);
  }
  cout << "begin export the data by query function" << endl;
  pthread_rwlock_rdlock(&(it_already_build->second->db_lock));

  string sparql = "select * where{?x ?y ?z.}";
  ResultSet rs;
  string db_path2 = db_path + "/" + db_name + ".nt";
  cout << "open the result file:" << db_path2 << endl;

  FILE* ofp = fopen(db_path2.c_str(), "w");
  int ret = current_database->query(sparql, rs, ofp, true, true);
  fflush(ofp);
  fclose(ofp);
  ofp = NULL;
  cout << "begin export the data by query function" << endl;
  string success = "Export the database successfully.";
  string resJson = CreateJson(0, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

  pthread_rwlock_unlock(&(it_already_build->second->db_lock));
  return;
}

bool export_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&export_thread, response, request, RequestType);
  t.detach();
  return true;
}

void query_thread(bool update_flag, string db_name, string format, string db_query, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
  //Notice: update the log in mutithreading environment
  //1. add therad ID to each line
  //2. combine information in a therad then write
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is query" << endl;
  //sleep(5);

  //	string format = request->path_match[1];
  //string format = "html";
  //	string db_query=request->path_match[2];
  //	format = UrlDecode(format);
  //	db_query = UrlDecode(db_query);
  cout << log_prefix << "check: " << db_query << endl;
  string str = db_query;
      cout << "begin lock databases_map_lock" << endl;
	pthread_rwlock_rdlock(&databases_map_lock);
    cout << " lock databases_map_lock successful " << endl;
	std::map<std::string, Database *>::iterator iter = databases.find(db_name);
	if(iter == databases.end())
	{
		string error = "Database not load yet.";
	//cout << error << endl;
		string resJson = CreateJson(304, error, 0);
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
		
		
		//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		pthread_rwlock_unlock(&databases_map_lock);
		return;
	}
	Database *current_database = iter->second;
	pthread_rwlock_unlock(&databases_map_lock);
    cout << "unlock databases_map_lock successful" << endl;
	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(db_name);
	pthread_rwlock_unlock(&already_build_map_lock);
    cout << "debug lock already_build_map_lock" << endl;
	//if(pthread_rwlock_tryrdlock(&(it_already_build->second)) != 0)
	//{
	//	string error = "Unable to query due to loss of lock";
	//	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
	//	return;
	//}
	pthread_rwlock_rdlock(&(it_already_build->second->db_lock));

  //TODO:MODIFY
  //	if(pthread_rwlock_tryrdlock(&database_load_lock) != 0)
  //	{
  //		string error = "Unable to query due to the loss of lock!";
  //		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
  //		return;
  //	}

  //	if(current_database == NULL)
  //	{
  //		string error = "No database in use!";
  //		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
  //		pthread_rwlock_unlock(&database_load_lock);
  //		return;
  //	}

	string sparql;
	sparql = db_query;
	if (sparql.empty()) {
		cerr <<log_prefix<< "Empty SPARQL." << endl;
		string error = "Empty SPARQL.";
		string resJson = CreateJson(401, error, 0);
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
		
		
		//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
		return;
	}

    cout << "debug lock it_already_build->second->db_lock" << endl;
	FILE* output = NULL;

  //just for DEBUG
  //sleep(3);
  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << sparql.length() << "\r\n\r\n" << sparql;
  //cout<<log_prefix<<"ends here"<<endl;
  //return;

  //TODO+DEBUG: manage the timer, not causing the db to restart
  //pthread_t timer = start_thread(func_timer);
  //if (timer == 0) {
  //cerr <<log_prefix<< "Failed to start timer." << endl;
  //}

	ResultSet rs;
	int query_time = Util::get_cur_time();
	int ret_val;
	//catch exception when this is an update query and has no update privilege
	try{
        cout << "begin query" << endl;
		ret_val = current_database->query(sparql, rs, output, update_flag);
	}catch(string exception_msg){
	
		string content=exception_msg;
		cout << exception_msg;
		string resJson = CreateJson(405, content, 0);
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
		return;	
	}
	bool ret = false, update = false;
	if(ret_val < -1)   //non-update query
	{
		ret = (ret_val == -100);
	}
	else  //update query, -1 for error, non-negative for num of triples updated
	{
		update = true;
	}

  query_time = Util::get_cur_time() - query_time;
  string query_time_s = Util::int2string(query_time);
  //if (timer != 0 && !stop_thread(timer))
  //{
  //cerr <<log_prefix<< "Failed to stop timer." << endl;
  //}

  ostringstream stream;
  stream << rs.ansNum;
  string ansNum_s = stream.str();
  cout << log_prefix << "ansNum_s: " << ansNum_s << endl;
  string filename = "thread_" + thread_id + "_" + Util::getTimeName() + "_query";
  string localname = ".tmp/web/" + filename;

  if (ret) {
    cout << log_prefix << "search query returned successfully." << endl;

    //record each query operation, including the sparql and the answer number
    //accurate down to microseconds
    char time_str[100];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int s = tv.tv_usec / 1000;
    int y = tv.tv_usec % 1000;

    string query_start_time = Util::get_date_time() + ":" + Util::int2string(s) + "ms" + ":" + Util::int2string(y) + "microseconds";
    string remote_ip;
    //get the real IP of the client, because we use nginx here
    unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals> m = request->header;
    string map_key = "X-Real-IP";
    //for (auto it = m.begin(); it != m.end(); it ++){
    pair<std::unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals>::iterator, std::unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals>::iterator> lu = m.equal_range(map_key);
    if (lu.first != lu.second)
      remote_ip = lu.first->second;
    else
      remote_ip = request->remote_endpoint_address;
    cout << "remote_ip: " << remote_ip << endl;

    //filter the IP from the test server
    if (remote_ip != TEST_IP) {
      Document doc;
      doc.SetObject();
      Document::AllocatorType& doc_allocator = doc.GetAllocator();
      doc.AddMember("QueryDateTime", StringRef(query_start_time.c_str()), doc_allocator);
      doc.AddMember("RemoteIP", StringRef(remote_ip.c_str()), doc_allocator);
      doc.AddMember("Sparql", StringRef(sparql.c_str()), doc_allocator);
      doc.AddMember("AnsNum", rs.ansNum, doc_allocator);
      string QueryTime = Util::int2string(query_time) + "ms";
      doc.AddMember("QueryTime", StringRef(QueryTime.c_str()), doc_allocator);
      StringBuffer buffer;
      PrettyWriter<StringBuffer> writer(buffer);
      doc.Accept(writer);
      writeLog(query_logfp, buffer.GetString());
    }

    //string log_info = Util::get_date_time() + "\n" + sparql + "\n\nanswer num: " + Util::int2string(rs.ansNum)+"\nquery time: "+Util::int2string(query_time) +" ms\n-----------------------------------------------------------\n";

    //to void someone downloading all the data file by sparql query on purpose and to protect the data
    //if the ansNum too large, for example, larger than 100000, we limit the return ans.
    if (rs.ansNum > MAX_OUTPUT_SIZE) {
      if (rs.output_limit == -1 || rs.output_limit > MAX_OUTPUT_SIZE)
        rs.output_limit = MAX_OUTPUT_SIZE;
    }

    ofstream outfile;
    string ans = "";
    string success = "";
    //TODO: if result is stored in Stream instead of memory?  (if out of memory to use to_str)
    //BETTER: divide and transfer, in multiple times, getNext()
    if (format == "json") {
      success = rs.to_JSON();
      Document resDoc;
      Document::AllocatorType& allocator = resDoc.GetAllocator();
      resDoc.Parse(success.c_str());
      resDoc.AddMember("StatusCode", 0, allocator);
      resDoc.AddMember("StatusMsg", "success", allocator);
      StringBuffer resBuffer;
      PrettyWriter<StringBuffer> resWriter(resBuffer);
      resDoc.Accept(resWriter);
      success = resBuffer.GetString();
    } else {
      //	cout << "query success, transfer to str." << endl;
      success = rs.to_str();
    }
    if (format == "html") {
      localname = localname + ".txt";
      filename = filename + ".txt";
    } else {
      localname = localname + "." + format;
      filename = filename + "." + format;
    }
    cout << log_prefix << "filename: " << filename << endl;
    if (format == "html") {
      outfile.open(localname);
      outfile << success;
      outfile.close();
      if (rs.ansNum > 100) {
        if (rs.output_limit == -1 || rs.output_limit > 100)
          rs.output_limit = 100;
      }
      success = rs.to_JSON();

      Document resDoc;
      Document::AllocatorType& allocator = resDoc.GetAllocator();
      resDoc.Parse(success.c_str());
      resDoc.AddMember("StatusCode", 0, allocator);
      resDoc.AddMember("StatusMsg", "success", allocator);
      resDoc.AddMember("AnsNum", rs.ansNum, allocator);
      string QueryTime = Util::int2string(query_time) + "ms";
      resDoc.AddMember("QueryTime", StringRef(QueryTime.c_str()), allocator);
      resDoc.AddMember("Filename", StringRef(filename.c_str()), allocator);

      StringBuffer resBuffer;
      PrettyWriter<StringBuffer> resWriter(resBuffer);
      resDoc.Accept(resWriter);
      string resJson = resBuffer.GetString();

      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length();
      *response << "\r\nCache-Control: no-cache"
                << "\r\nPragma: no-cache"
                << "\r\nExpires: 0";
      *response << "\r\n\r\n" << resJson;

      //!Notice: remember to set no-cache in the response of query, Firefox and chrome works well even if you don't set, but IE will act strange if you don't set
      //beacause IE will defaultly cache the query result after first query request, so the following query request of the same url will not be send if the result in cache isn't expired.
      //then the following query will show the same result without sending a request to let the service run query
      //so the download function will go wrong because there is no file in the service.
      //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << query_time_s.length()+ansNum_s.length()+filename.length()+success.length()+4;
      //*response << "\r\nContent-Type: text/plain";
      //*response << "\r\nCache-Control: no-cache" << "\r\nPragma: no-cache" << "\r\nExpires: 0";
      //*response  << "\r\n\r\n" << "0+" << query_time_s<< '+' << rs.ansNum << '+' << filename << '+' << success;
      pthread_rwlock_unlock(&(it_already_build->second->db_lock));
      //return true;
      return;
    } else {
      string filename = "";
      filename = "sparql." + format;
      cout << log_prefix << "filename: " << filename << endl;
      *response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length();
      *response << "\r\nContent-Type: application/octet-stream";
      *response << "\r\nContent-Disposition: attachment; filename=\"" << filename << '"';
      *response << "\r\n\r\n" << success;

      pthread_rwlock_unlock(&(it_already_build->second->db_lock));
      return;
      //return true;
      //
    }
  } else {
    string error = "";
    int error_code;
    if (update) {
      cout << log_prefix << "update query returned correctly." << endl;
      error = "update query returns true.";
      error_code = 402;
    } else {
      cout << log_prefix << "search query returned error." << endl;
      error = "search query returns false.";
      error_code = 403;
    }
    string resJson = CreateJson(error_code, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
    //return false;
    return;
  }

  //return true;
}

bool query_handler0(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is query_handler0" << endl;
  cout << "request->path: " << request->path << endl;

  string db_name;
  string format;
  string db_query;

  if (RequestType == "GET") {
    db_name = request->path_match[1];
    format = request->path_match[2];
    db_query = request->path_match[3];
    db_name = UrlDecode(db_name);
    format = UrlDecode(format);
    db_query = UrlDecode(db_query);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    format = document["format"].GetString();
    db_query = document["sparql"].GetString();
  }

  //check if the db_name is system
  if (db_name == "system") {
    string error = "no query privilege, operation failed.";
    string resJson = CreateJson(404, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return false;
  }

  query_num++;
  Task* task = new Task(0, db_name, format, db_query, response, request);
  pool.AddTask(task);
}

bool query_handler1(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is query_handler1" << endl;
  cout << "request->path: " << request->path << endl;

  string username;
  string password;
  string db_name;
  string format;
  string db_query;

  if (RequestType == "GET") {
    username = request->path_match[1];
    password = request->path_match[2];
    db_name = request->path_match[3];
    format = request->path_match[4];
    db_query = request->path_match[5];
    db_name = UrlDecode(db_name);
    username = UrlDecode(username);
    password = UrlDecode(password);
    format = UrlDecode(format);
    db_query = UrlDecode(db_query);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    username = document["username"].GetString();
    password = document["password"].GetString();
    db_name = document["db_name"].GetString();
    format = document["format"].GetString();
    db_query = document["sparql"].GetString();
  }

  cout << "check: " << db_query << endl;
  string str = db_query;

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return false;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return false;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  //check if the db_name is system
  if (db_name == "system") {
    string error = "no query privilege, operation failed.";
    string resJson = CreateJson(404, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return false;
  }

  //check privilege
  if (checkPrivilege(username, "query", db_name) == 0) {
    string error = "no query privilege, operation failed.";
    string resJson = CreateJson(404, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return false;
  } else if (checkPrivilege(username, "update", db_name) == 0) {
    cout << "check privilege successfully: non_update query." << endl;

    query_num++;
    Task* task = new Task(0, db_name, format, db_query, response, request);
    pool.AddTask(task);
    return true;

  } else {
    cout << "check privilege successfully: update" << endl;

    //current_database = iter->second;
    //doQuery(format, db_query, server, response, request);
    query_num++;
    Task* task = new Task(1, db_name, format, db_query, response, request);
    pool.AddTask(task);
    //thread t(&query_thread, db_name, format, db_query, response, request);
    //t.detach();
    return true;
  }
}

//void query_handler(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
//{
//	query_num++;
//cout<<"content: "<<request->content.string()<<endl;
//cout<<"Header: "<<endl;
//Usage of auto: http://blog.csdn.net/qq_26399665/article/details/52267734
//for(auto it : request->header)
//{
//cout<<it.first<<" "<<it.second<<endl;
//}
//cout<<"client addr: "<<request->remote_endpoint_address<<endl;
//cout<<"client port: "<<request->remote_endpoint_port<<endl;

//DEBUG: when using a thread for a query and run in parallism, the QueryParser will fail due to ANTLR
//In addition, addRequest and io buffer in StringIndex will also cause error
//	thread t(&query_thread, response, request);
//	t.detach();
//t.join();

//DEBUG: here will cause error due to the use of socket in 2 processes
//NOTICE: we should sue multithreading here because frequently creating/destroying processes will be costly
//Another reason is the log4cplus supports multithreading but not multiprocessing
//pid_t child = fork();
//if(child == -1)
//{
//cout<<"child fork error in query processing of ghttp"<<endl;
////here we do it sequentially
//query_handler(response, request);
//}
//else if(child == 0)
//{
//query_handler(response, request);
////sleep(10); //wait the IO to end in this process
//cout<<"now the child process to exit"<<endl;
////DEBUG: if we use this , then error? why? restart endlessly
////exit(0);
//}
//else
//{
////if father process exits, all child process will exit
//pid_t ret = waitpid(child, NULL, 0);
//if(ret == child)
//{
//cout<<"father process get child exit code: "<<ret<<endl;
////DEBUG: if not exit before, the process won't end, the mmeory of query won't be released
////TODO: error here
////return 0;
//}
//else
//{
//cout<<"error occured"<<endl;
////return -1;
//}
//}
//The upper is blocked-waiting(the child process should not use exit),
//below is non-blocked-waiting(the child process should use exit)
//do {
//pid_t ret = waitpid(child, NULL, WNOHANG);
//if(ret == 0) {
//cout<<"xxx"<<endl;
//sleep(1); //query again for each second
//}
//}while(ret == 0);
//if(ret == child){};
//}

void monitor_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is monitor" << endl;

  //	if(pthread_rwlock_tryrdlock(&database_load_lock) != 0)
  //	{
  //	    string error = "Unable to monitor due to loss of lock";
  //		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
  //		return;
  //	}

  string db_name;
  string username;
  string password;

  if (RequestType == "GET") {
    db_name = request->path_match[1];
    username = request->path_match[2];
    password = request->path_match[3];
    db_name = UrlDecode(db_name);
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    username = document["username"].GetString();
    password = document["password"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  //check if the db_name is system and if the username is root
  if ((db_name == "system") && (username != ROOT_USERNAME)) {
    string error = "no monitor privilege, operation failed.";
    string resJson = CreateJson(502, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  //check if database named [db_name] is already load
  pthread_rwlock_rdlock(&databases_map_lock);
  std::map<std::string, Database*>::iterator iter = databases.find(db_name);
  if (iter == databases.end()) {
    //cout << "database not loaded yet." << endl;
    string error = "Database not load yet.\r\n";
    string resJson = CreateJson(304, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&databases_map_lock);
    return;
  }
  Database* _database = iter->second;
  pthread_rwlock_unlock(&databases_map_lock);

  //show statistical information of database named db_name
  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
  string creator = it_already_build->second->getCreator();
  string time = it_already_build->second->getTime();
  pthread_rwlock_unlock(&already_build_map_lock);

  if (pthread_rwlock_tryrdlock(&(it_already_build->second->db_lock)) != 0) {
    string error = "Unable to monitor due to loss of lock";
    string resJson = CreateJson(501, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }
  //use JSON format to send message
  Document resDoc;
  resDoc.SetObject();
  Document::AllocatorType& allocator = resDoc.GetAllocator();
  resDoc.AddMember("StatusCode", 0, allocator);
  resDoc.AddMember("StatusMsg", "success", allocator);
  string name = _database->getName();
  resDoc.AddMember("database", StringRef(name.c_str()), allocator);
  resDoc.AddMember("creator", StringRef(creator.c_str()), allocator);
  resDoc.AddMember("built_time", StringRef(time.c_str()), allocator);
  resDoc.AddMember("triple num", _database->getTripleNum(), allocator);
  resDoc.AddMember("entity num", _database->getEntityNum(), allocator);
  resDoc.AddMember("literal num", _database->getLiteralNum(), allocator);
  resDoc.AddMember("subject num", _database->getSubNum(), allocator);
  resDoc.AddMember("predicate num", _database->getPreNum(), allocator);
  int conn_num = connection_num / 2;
  resDoc.AddMember("connection num", conn_num, allocator);

  StringBuffer resBuffer;
  PrettyWriter<StringBuffer> resWriter(resBuffer);
  resDoc.Accept(resWriter);
  string resJson = resBuffer.GetString();

  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
  pthread_rwlock_unlock(&(it_already_build->second->db_lock));
}

bool monitor_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&monitor_thread, response, request, RequestType);
  t.detach();
  return true;
}

void default_thread(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is default" << endl;

  //BETTER: use lock to ensure thread safe
  if (request->path == "/admin.html" || request->path == "/admin_root.html") {
    cout << "request->path is admin.html or admin_root.html" << endl;
    request->path = "/login.html";
    cout << "request->path changed to: " << request->path << endl;
  }

  //	connection_num++;
  //NOTICE: it seems a visit will output twice times
  //And different pages in a browser is viewed as two connections here
  //cout<<"new connection"<<endl;

  try
  {
    auto web_root_path = boost::filesystem::canonical("./Server/web");
    auto path = boost::filesystem::canonical(web_root_path / request->path);
    //Check if path is within web_root_path
    if (distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) || !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
      throw invalid_argument("path must be within root path");
    if (boost::filesystem::is_directory(path))
      path /= "index.html";
    if (!(boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)))
      throw invalid_argument("file does not exist");

    std::string cache_control, etag;
    // Uncomment the following line to enable Cache-Control
    // cache_control="Cache-Control: max-age=86400\r\n";

    auto ifs = make_shared<ifstream>();
    ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);
    string extName = path.extension().string();
    cout << "extName: " << extName << endl;

    if (*ifs) {
      auto length = ifs->tellg();
      ifs->seekg(0, ios::beg);

      *response << "HTTP/1.1 200 OK\r\n" << cache_control << etag << "Content-Length: " << length << "\r\n";
      //!Notice: IE need to declare the Content-type of the response, if not the browser will not show the result with an unknow format.
      //it works well in Firefox and Chrome if you don't declare the Content-type, but in IE the css file will be ignored because of unmatched MIME type, then the web page will show in a form without css style and it looks ugly.
      if (extName == ".html")
        *response << "Content-Type: text/html"
                  << "\r\n\r\n";
      else if (extName == ".js")
        *response << "Content-Type: application/x-javascript"
                  << "\r\n\r\n";
      else if (extName == ".css")
        *response << "Content-Type: text/css"
                  << "\r\n\r\n";
      else if (extName == ".png")
        *response << "Content-Type: image/png"
                  << "\r\n\r\n";
      else if (extName == ".jpg")
        *response << "Content-Type: image/jpeg"
                  << "\r\n\r\n";
      else if (extName == ".ico")
        *response << "Content-Type: image/x-icon"
                  << "\r\n\r\n";

      default_resource_send(server, response, ifs);

    } else
      throw invalid_argument("could not read file");
  }
  catch (const exception& e)
  {
    string content = "Could not open path " + request->path + ": " + e.what();
    string resJson = CreateJson(101, content, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
  }
}

bool default_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
  //NOTICE: it seems a visit will output twice times
  //And different pages in a browser is viewed as two connections here
  //cout<<"new connection"<<endl;
  connection_num++;
  string req_url = request->path;
  cout << "request url: " << req_url << endl;

  default_thread(server, response, request);
  //thread t(&default_thread, response, request);
  //t.detach();S
  return true;
}

bool check_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is check" << endl;

  string req_url = request->path;
  cout << "request url: " << req_url << endl;
  string username;
  string password;

  if (RequestType == "GET") {
    username = request->path_match[1];
    password = request->path_match[2];
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    username = document["username"].GetString();
    password = document["password"].GetString();
  }
  cout << "username = " << username << endl;
  cout << "password = " << password << endl;

  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "wrong username.";
    cout << error << endl;
    string resJson = CreateJson(901, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return false;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    cout << error << endl;
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return false;
  } else {
    cout << "login successfully." << endl;
    string success = "check identity successfully.";
    string resJson = CreateJson(0, success, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
    pthread_rwlock_unlock(&users_map_lock);
    return true;
  }
}

bool login_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is login" << endl;

  string req_url = request->path;
  cout << "request url: " << req_url << endl;
  string username;
  string password;

  if (RequestType == "GET") {
    username = request->path_match[1];
    password = request->path_match[2];
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    username = document["username"].GetString();
    password = document["password"].GetString();
  }
  cout << "username = " << username << endl;
  cout << "password = " << password << endl;

  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "wrong username.";
    string resJson = CreateJson(901, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return false;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return false;
  } else {
    cout << "login successfully." << endl;
    pthread_rwlock_unlock(&users_map_lock);

    try
    {
      auto web_root_path = boost::filesystem::canonical("./Server/web");
      if (username == ROOT_USERNAME)
        request->path = "/admin_root.html";
      else
        request->path = "/admin.html";
      auto path = boost::filesystem::canonical(web_root_path / request->path);
      //Check if path is within web_root_path
      if (distance(web_root_path.begin(), web_root_path.end()) > distance(path.begin(), path.end()) || !equal(web_root_path.begin(), web_root_path.end(), path.begin()))
        throw invalid_argument("path must be within root path");
      if (boost::filesystem::is_directory(path))
        path /= "login.html";
      if (!(boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)))
        throw invalid_argument("file does not exist");

      std::string cache_control, etag;

      // Uncomment the following line to enable Cache-Control
      // cache_control="Cache-Control: max-age=86400\r\n";

      auto ifs = make_shared<ifstream>();
      ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);
      string extName = path.extension().string();
      cout << "extName: " << extName << endl;

      if (*ifs) {
        auto length = ifs->tellg();
        ifs->seekg(0, ios::beg);

        *response << "HTTP/1.1 200 OK\r\n" << cache_control << etag << "Content-Length: " << length << "\r\n";
        //!Notice: IE need to declare the Content-type of the response, if not the browser will not show the result with an unknow format.
        //it works well in Firefox and Chrome if you don't declare the Content-type, but in IE the css file will be ignored because of unmatched MIME type, then the web page will show in a form without css style and it looks ugly.
        if (extName == ".html")
          *response << "Content-Type: text/html"
                    << "\r\n\r\n";
        else if (extName == ".js")
          *response << "Content-Type: application/x-javascript"
                    << "\r\n\r\n";
        else if (extName == ".css")
          *response << "Content-Type: text/css"
                    << "\r\n\r\n";
        else if (extName == ".png")
          *response << "Content-Type: image/png"
                    << "\r\n\r\n";
        else if (extName == ".jpg")
          *response << "Content-Type: image/jpeg"
                    << "\r\n\r\n";
        else if (extName == ".ico")
          *response << "Content-Type: image/x-icon"
                    << "\r\n\r\n";

        default_resource_send(server, response, ifs);
      } else
        throw invalid_argument("could not read file");
    }
    catch (const exception& e)
    {
      string content = "Could not open path " + request->path + ": " + e.what();
      string resJson = CreateJson(101, content, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

      //*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
    }

    return true;
  }
}

//to stop the ghttp server
bool stop_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is stop" << endl;

  string username = request->path_match[1];
  string password = request->path_match[2];
  username = UrlDecode(username);
  password = UrlDecode(password);

  //check identity.
  if (username != SYSTEM_USERNAME) {
    string error = "You have no rights to stop the server.";
    string resJson = CreateJson(702, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    cout << "Stop server failed." << endl;
    return false;
  }
  if (system_password != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    cout << "Stop server failed." << endl;
    return false;
  }
  cout << "check identity successfully." << endl;

  //string success = "Server stopped.";
  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
  bool flag = checkall_thread(response, request);
  if (flag == false) {
    cout << "Stop server failed." << endl;
    return false;
  }
  string cmd = "rm system.db/password" + Util::int2string(server.config.port) + ".txt";
  system(cmd.c_str());
  cout << "Server stopped." << endl;
  return true;
}

void checkpoint_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is checkpoint" << endl;

  //TODO:MODIFY
  //	if(pthread_rwlock_tryrdlock(&database_load_lock) != 0)
  //	{
  //		string error = "Unbale to checkpoint due to the loss of lock.";
  //		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
  //		return;
  //	}

  string db_name;
  string username;
  string password;

  if (RequestType == "GET") {
    db_name = request->path_match[1];
    username = request->path_match[2];
    password = request->path_match[3];
    db_name = UrlDecode(db_name);
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    username = document["username"].GetString();
    password = document["password"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  //check if the db_name is system and if the username is root
  if ((db_name == "system") && (username != ROOT_USERNAME)) {
    string error = "no checkpoint privilege, operation failed.";
    string resJson = CreateJson(703, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  //check if database named [db_name] is loaded.
  pthread_rwlock_rdlock(&databases_map_lock);
  std::map<std::string, Database*>::iterator iter = databases.find(db_name);
  if (iter == databases.end()) {
    string error = "Database not load yet.";
    string resJson = CreateJson(304, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&databases_map_lock);
    return;
  }
  Database* current_database = iter->second;
  pthread_rwlock_unlock(&databases_map_lock);

  //check privilege
  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
  string creator = it_already_build->second->getCreator();
  pthread_rwlock_unlock(&already_build_map_lock);
  if (creator != username) {
    if (username != ROOT_USERNAME) {
      string error = "no checkpoint privilege, operation failed.";
      string resJson = CreateJson(703, error, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
      return;
    }
  }
  cout << "check privilege successfully." << endl;

  if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
    string error = "Unable to checkpoint due to loss of lock";
    string resJson = CreateJson(704, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }
  //For database checkpoint or log/transaction:
  //http://www.newsmth.net/nForum/#!article/LinuxDev/37802?p=2
  //http://blog.csdn.net/cywosp/article/details/8767327
  //http://www.fx114.net/qa-63-143449.aspx
  current_database->save();
  //NOTICE: this info is in header
  string success = "Database saved successfully.";
  //header and content are split by an empty line
  string resJson = CreateJson(0, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

  pthread_rwlock_unlock(&(it_already_build->second->db_lock));
}

//TODO+BETTER: server choose to save a database when system is not busy
//If user send this command too frequently, the performance may be awful if updates are large
bool checkpoint_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&checkpoint_thread, response, request, RequestType);
  t.detach();
  return true;
}

bool checkall_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is checkall" << endl;

  pthread_rwlock_rdlock(&databases_map_lock);
  std::map<std::string, Database*>::iterator iter;
  string success;
  for (iter = databases.begin(); iter != databases.end(); iter++) {
    string database_name = iter->first;
    if (database_name == "system")
      continue;
    Database* current_database = iter->second;
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(database_name);
    pthread_rwlock_unlock(&already_build_map_lock);
    if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
      string error = "Unable to checkpoint due to loss of lock";
      string resJson = CreateJson(704, error, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

      //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
      pthread_rwlock_unlock(&databases_map_lock);
      return false;
    }
    current_database->save();
    delete current_database;
    current_database = NULL;
    cout << "Database " << database_name << " saved successfully." << endl;
    success = success + "Database " + database_name + " saved successfully.\r\n";
    string resJson = CreateJson(0, success, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
  }
  system_database->save();
  delete system_database;
  system_database = NULL;
  cout << "Database system saved successfully." << endl;
  pthread_rwlock_unlock(&databases_map_lock);

  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build;
  for (it_already_build = already_build.begin(); it_already_build != already_build.end(); it_already_build++) {
    struct DBInfo* temp_db = it_already_build->second;
    delete temp_db;
    temp_db = NULL;
  }
  pthread_rwlock_unlock(&already_build_map_lock);

  return true;
}

// to add, delete users or modify the privilege of a user, operation be done by the root user
void user_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{

  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is user" << endl;

  string req_url = request->path;
  cout << "request url: " << req_url << endl;
  string type;
  string username1;
  string password1;
  string username2;
  string addition;

  if (RequestType == "GET") {
    type = request->path_match[1];
    username1 = request->path_match[2];
    password1 = request->path_match[3];
    username2 = request->path_match[4];
    addition = request->path_match[5];
    type = UrlDecode(type);
    username1 = UrlDecode(username1);
    password1 = UrlDecode(password1);
    username2 = UrlDecode(username2);
    addition = UrlDecode(addition);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    type = document["type"].GetString();
    username1 = document["username1"].GetString();
    password1 = document["password1"].GetString();
    username2 = document["username2"].GetString();
    addition = document["addition"].GetString();
  }

  cout << "type = " << type << endl;
  cout << "username1 = " << username1 << endl;
  cout << "password1 = " << password1 << endl;

  //if root user
  pthread_rwlock_rdlock(&users_map_lock);
  string root_password = (users.find(ROOT_USERNAME))->second->getPassword();
  pthread_rwlock_unlock(&users_map_lock);

  if (username1 == ROOT_USERNAME && password1 == root_password) {
    string password2 = addition;

    cout << "username2 = " << username2 << endl;
    cout << "password2 = " << password2 << endl;

    if (type == "add_user") {
      pthread_rwlock_wrlock(&users_map_lock);
      if (users.find(username2) == users.end()) {
        cout << "user ready to add." << endl;
        struct User* temp_user = new User(username2, password2);
        users.insert(pair<std::string, struct User*>(username2, temp_user));
        string update = "INSERT DATA {<" + username2 + "> <has_password> \"" + password2 + "\".}";
        updateSys(update);
        cout << "user add done." << endl;

      } else {
        string error = "username already existed, add user failed.";
        string resJson = CreateJson(907, error, 0);
        *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

        //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
        pthread_rwlock_unlock(&users_map_lock);
        return;
      }
      pthread_rwlock_unlock(&users_map_lock);

    } else if (type == "delete_user") {
      pthread_rwlock_wrlock(&users_map_lock);
      std::map<std::string, struct User*>::iterator iter;
      iter = users.find(username2);
      if (iter != users.end() && username2 != ROOT_USERNAME) {
        delete iter->second;
        iter->second = NULL;
        users.erase(username2);
        string update = "DELETE where {<" + username2 + "> ?p ?o.}";
        updateSys(update);

      } else {
        string error;
        int error_code;
        if (username2 == ROOT_USERNAME) {
          error = "you cannot delete root, delete user failed.";
          error_code = 908;
        } else {
          error = "username not exist, delete user failed.";
          error_code = 909;
        }
        string resJson = CreateJson(error_code, error, 0);
        *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

        //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
        pthread_rwlock_unlock(&users_map_lock);
        return;
      }
      pthread_rwlock_unlock(&users_map_lock);

    } else if (type == "change_psw") {
      pthread_rwlock_wrlock(&users_map_lock);
      std::map<std::string, struct User*>::iterator iter;
      iter = users.find(username2);

      if (iter == users.end()) {
        string error = "username not exist, change password failed.";
        string resJson = CreateJson(916, error, 0);
        *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
        pthread_rwlock_unlock(&users_map_lock);

        return;

      } else {
        iter->second->setPassword(password2);
        string update = "DELETE WHERE {<" + username2 + "> <has_password> ?o.}";
        updateSys(update);
        string update2 = "INSERT DATA {<" + username2 + "> <has_password>  \"" + password2 + "\".}";
        updateSys(update2);
      }
      pthread_rwlock_unlock(&users_map_lock);

    } else if (type == "add_query" || type == "add_load" || type == "add_unload" || type == "add_update" || type == "add_backup" || type == "add_restore" || type == "add_export") {
      if (username2 == ROOT_USERNAME) {
        string error = "you can't add privilege to root user.";
        string resJson = CreateJson(910, error, 0);
        *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

        //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
        return;
      }
      int len = type.length();
      string subType = type.substr(4, len - 4);
      cout << "subType: " << subType << endl;
      string db_name = addition;
      if (addPrivilege(username2, subType, db_name) == 0) {
        string error = "add privilege failed.";
        string resJson = CreateJson(911, error, 0);
        *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

        //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
        return;
      }
    } else if (type == "delete_query" || type == "delete_load" || type == "delete_unload" || type == "delete_update" || type == "delete_backup" || type == "delete_restore" || type == "delete_export") {
      if (username2 == ROOT_USERNAME) {
        string error = "you can't delete privilege of root user.";
        string resJson = CreateJson(913, error, 0);
        *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

        //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
        return;
      }
      int len = type.length();
      string subType = type.substr(7, len - 7);
      cout << "subType: " << subType << endl;
      string db_name = addition;
      if (delPrivilege(username2, subType, db_name) == 0) {
        string error = "delete privilege failed.";
        string resJson = CreateJson(914, error, 0);
        *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

        //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
        return;
      }
    }
    string success = "operation on users succeeded.";
    string resJson = CreateJson(0, success, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
    return;
  }

  //if not root user, no privilege to perform this operation
  string error = "Not root user, no privilege to perform this operation.";
  string resJson = CreateJson(915, error, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
  return;

  //TODO:MODIFY
  //NOTICE: this info is in header
  //string success = current_database->getName();
  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
  //pthread_rwlock_unlock(&database_load_lock);
}

bool user_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&user_thread, response, request, RequestType);
  t.detach();
  return true;
}

//BETTER: indicate the db_name when query
void show_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is show" << endl;

  string username;
  string password;

  if (RequestType == "GET") {
    username = request->path_match[1];
    password = request->path_match[2];
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    username = document["username"].GetString();
    password = document["password"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build;
  string success;
  Document resDoc;
  resDoc.SetObject();
  Document::AllocatorType& allocator = resDoc.GetAllocator();

  resDoc.AddMember("StatusCode", 0, allocator);
  resDoc.AddMember("StatusMsg", "success", allocator);
  Value jsonArray(kArrayType);

  for (it_already_build = already_build.begin(); it_already_build != already_build.end(); it_already_build++) {
    string database_name = it_already_build->first;
    string creator = it_already_build->second->getCreator();
    string time = it_already_build->second->getTime();
    if ((database_name == "system") && (username != ROOT_USERNAME))
      continue;
    Value obj(kObjectType);

    Value _database_name;
    _database_name.SetString(database_name.c_str(), database_name.length(), allocator);
    obj.AddMember("database", _database_name, allocator);

    Value _creator;
    _creator.SetString(creator.c_str(), creator.length(), allocator);
    obj.AddMember("creator", _creator, allocator);

    Value _time;
    _time.SetString(time.c_str(), time.length(), allocator);
    obj.AddMember("built_time", _time, allocator);
    pthread_rwlock_rdlock(&databases_map_lock);
    if (databases.find(database_name) == databases.end())
      obj.AddMember("status", "unloaded", allocator);
    else
      obj.AddMember("status", "loaded", allocator);
    pthread_rwlock_unlock(&databases_map_lock);
    jsonArray.PushBack(obj, allocator);
  }
  resDoc.AddMember("ResponseBody", jsonArray, allocator);
  StringBuffer resBuffer;
  PrettyWriter<StringBuffer> resWriter(resBuffer);
  resDoc.Accept(resWriter);
  string resJson = resBuffer.GetString();

  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json"
            << "\r\n\r\n" << resJson;

  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
  //return true;
  pthread_rwlock_unlock(&already_build_map_lock);
  return;
}

bool show_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&show_thread, response, request, RequestType);
  t.detach();
  return true;
}

void getCoreVersion_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is setCoreVersion" << endl;

  string username;
  string password;

  if (RequestType == "GET") {
    username = request->path_match[1];
    password = request->path_match[2];
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    username = document["username"].GetString();
    password = document["password"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  Document resDoc;
  resDoc.SetObject();
  Document::AllocatorType& allocator = resDoc.GetAllocator();
  resDoc.AddMember("StatusCode", 0, allocator);
  resDoc.AddMember("StatusMsg", "success", allocator);
  resDoc.AddMember("CoreVersion", StringRef(CoreVersion.c_str()), allocator);
  StringBuffer resBuffer;
  PrettyWriter<StringBuffer> resWriter(resBuffer);
  resDoc.Accept(resWriter);
  string resJson = resBuffer.GetString();

  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json"
            << "\r\n\r\n" << resJson;

  return;
}

bool getCoreVersion_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&getCoreVersion_thread, response, request, RequestType);
  t.detach();
  return true;
}

void setAPIVersion_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is setAPIVersion" << endl;

  string username;
  string password;
  string version;
  if (RequestType == "GET") {
    username = request->path_match[1];
    password = request->path_match[2];
    version = request->path_match[3];
    username = UrlDecode(username);
    password = UrlDecode(password);
    version = UrlDecode(version);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    username = document["username"].GetString();
    password = document["password"].GetString();
    version = document["version"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  string update = "Delete WHERE { <APIVersion> ?x ?y.}";
  if (updateSys(update) == false) {
    string error = "APIVersion Set failed";
    string resJson = CreateJson(979, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }
  APIVersion = version;
  update = "INSERT DATA { <APIVersion> <value> \"" + APIVersion + "\"}";
  if (updateSys(update) == false) {
    string error = "APIVersion Set failed";
    string resJson = CreateJson(979, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }
  string success = "APIVersion Set success";
  string resJson = CreateJson(975, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
  return;
}

bool setAPIVersion_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&setAPIVersion_thread, response, request, RequestType);
  t.detach();
  return true;
}

void setCoreVersion_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is setCoreVersion" << endl;

  string username;
  string password;
  string version;
  if (RequestType == "GET") {
    username = request->path_match[1];
    password = request->path_match[2];
    version = request->path_match[3];
    username = UrlDecode(username);
    password = UrlDecode(password);
    version = UrlDecode(version);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    username = document["username"].GetString();
    password = document["password"].GetString();
    version = document["version"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  string update = "Delete WHERE { <CoreVersion> ?x ?y.}";
  if (updateSys(update) == false) {
    string error = "CoreVersion Set failed";
    string resJson = CreateJson(977, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }
  CoreVersion = version;
  update = "INSERT DATA { <CoreVersion> <value> \"" + CoreVersion + "\"}";
  if (updateSys(update) == false) {
    string error = "CoreVersion Set failed";
    string resJson = CreateJson(977, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }
  string success = "CoreVersion Set success";
  string resJson = CreateJson(976, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
  return;
}

void initVersion_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is initVersion" << endl;

  string username;
  string password;
  //string version;
  if (RequestType == "GET") {
    username = request->path_match[1];
    password = request->path_match[2];
    //version = request->path_match[3];
    username = UrlDecode(username);
    password = UrlDecode(password);
    //version = UrlDecode(version);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    username = document["username"].GetString();
    password = document["password"].GetString();
    //version = document["version"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  string update = "Delete WHERE { <CoreVersion> ?x ?y. <APIVersion> ?x1 ?y1.}";
  if (updateSys(update) == false) {
    string error = "Version Set failed";
    string resJson = CreateJson(977, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }
  //CoreVersion = version;
  string file = "data/system/version.nt";
  if (boost::filesystem::exists(file) == false) {
    string error = "version.nt is not found";
    string resJson = CreateJson(977, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }
  ifstream infile;
  infile.open(file.data()); //将文件流对象与文件连接起来
  string s;
  update = "INSERT DATA {";
  while (getline(infile, s)) {
    if (s != "")
      update = update + s;
  }
  infile.close();
  update = update + "}";
  cout << "the sparql of initversion is:" << update << endl;

  //update = "INSERT DATA { <CoreVersion> <value> \"" + CoreVersion + "\"}";
  if (updateSys(update) == false) {
    string error = "Version Init failed";
    string resJson = CreateJson(977, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }
  string success = "Version Init success";
  string resJson = CreateJson(976, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
  return;
}

bool setCoreVersion_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&setCoreVersion_thread, response, request, RequestType);
  t.detach();
  return true;
}

bool initVersion_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&initVersion_thread, response, request, RequestType);
  t.detach();
  return true;
}

void getAPIVersion_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is getAPIVersion" << endl;

  string username;
  string password;

  if (RequestType == "GET") {
    username = request->path_match[1];
    password = request->path_match[2];
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    username = document["username"].GetString();
    password = document["password"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  Document resDoc;
  resDoc.SetObject();
  Document::AllocatorType& allocator = resDoc.GetAllocator();
  resDoc.AddMember("StatusCode", 0, allocator);
  resDoc.AddMember("StatusMsg", "success", allocator);
  resDoc.AddMember("APIVersion", StringRef(APIVersion.c_str()), allocator);
  StringBuffer resBuffer;
  PrettyWriter<StringBuffer> resWriter(resBuffer);
  resDoc.Accept(resWriter);
  string resJson = resBuffer.GetString();

  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json"
            << "\r\n\r\n" << resJson;

  return;
}

bool getAPIVersion_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&getAPIVersion_thread, response, request, RequestType);
  t.detach();
  return true;
}

void showUser_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();
  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is showUser" << endl;

  string username;
  string password;

  if (RequestType == "GET") {
    username = request->path_match[1];
    password = request->path_match[2];
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    username = document["username"].GetString();
    password = document["password"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);
  cout << "check identity successfully." << endl;

  pthread_rwlock_rdlock(&users_map_lock);
  if (users.empty()) {
    string error = "No Users.\r\n";
    string resJson = CreateJson(802, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  string success;

  Document resDoc;
  resDoc.SetObject();
  Document::AllocatorType& allocator = resDoc.GetAllocator();
  resDoc.AddMember("StatusCode", 0, allocator);
  resDoc.AddMember("StatusMsg", "success", allocator);
  Value json_array(kArrayType);

  for (it = users.begin(); it != users.end(); it++) {
    Value obj(kObjectType);
    string username = it->second->getUsername();
    Value _username;
    _username.SetString(username.c_str(), username.length(), allocator);
    obj.AddMember("username", _username, allocator);

    string password = it->second->getPassword();
    Value _password;
    _password.SetString(password.c_str(), password.length(), allocator);
    obj.AddMember("password", _password, allocator);

    string query_db = it->second->getQuery();
    Value _query_db;
    _query_db.SetString(query_db.c_str(), query_db.length(), allocator);
    obj.AddMember("query privilege", _query_db, allocator);

    string update_db = it->second->getUpdate();
    Value _update_db;
    _update_db.SetString(update_db.c_str(), update_db.length(), allocator);
    obj.AddMember("update privilege", _update_db, allocator);

    string load_db = it->second->getLoad();
    Value _load_db;
    _load_db.SetString(load_db.c_str(), load_db.length(), allocator);
    obj.AddMember("load privilege", _load_db, allocator);

    string unload_db = it->second->getUnload();
    Value _unload_db;
    _unload_db.SetString(unload_db.c_str(), unload_db.length(), allocator);
    obj.AddMember("unload privilege", _unload_db, allocator);

    string backup_db = it->second->getbackup();
    Value _backup_db;
    _backup_db.SetString(backup_db.c_str(), backup_db.length(), allocator);
    obj.AddMember("backup privilege", _backup_db, allocator);

    string restore_db = it->second->getrestore();
    Value _restore_db;
    _restore_db.SetString(restore_db.c_str(), restore_db.length(), allocator);
    obj.AddMember("restore privilege", _restore_db, allocator);

    string export_db = it->second->getexport();
    Value _export_db;
    _export_db.SetString(export_db.c_str(), export_db.length(), allocator);
    obj.AddMember("export privilege", _export_db, allocator);

    json_array.PushBack(obj, allocator);
  }
  resDoc.AddMember("ResponseBody", json_array, allocator);
  StringBuffer resBuffer;
  PrettyWriter<StringBuffer> resWriter(resBuffer);
  resDoc.Accept(resWriter);
  string resJson = resBuffer.GetString();

  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

  //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
  pthread_rwlock_unlock(&users_map_lock);
  return;
}

bool showUser_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&showUser_thread, response, request, RequestType);
  t.detach();
  return true;
}

bool addPrivilege(string username, string type, string db_name)
{
  if (username == ROOT_USERNAME) {
    return 1;
  }
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it != users.end() && db_name != "system") {
    pthread_rwlock_unlock(&users_map_lock);
    if (type == "query") {
      pthread_rwlock_wrlock(&(it->second->query_priv_set_lock));
      it->second->query_priv.insert(db_name);
      string update = "INSERT DATA {<" + username + "> <has_query_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->query_priv_set_lock));

    } else if (type == "update") {
      pthread_rwlock_wrlock(&(it->second->update_priv_set_lock));
      it->second->update_priv.insert(db_name);
      string update = "INSERT DATA {<" + username + "> <has_update_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->update_priv_set_lock));

    } else if (type == "load") {
      pthread_rwlock_wrlock(&(it->second->load_priv_set_lock));
      it->second->load_priv.insert(db_name);
      string update = "INSERT DATA {<" + username + "> <has_load_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
    } else if (type == "unload") {
      pthread_rwlock_wrlock(&(it->second->unload_priv_set_lock));
      it->second->unload_priv.insert(db_name);
      string update = "INSERT DATA {<" + username + "> <has_unload_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
    } else if (type == "restore") {
      pthread_rwlock_wrlock(&(it->second->restore_priv_set_lock));
      it->second->restore_priv.insert(db_name);
      string update = "INSERT DATA {<" + username + "> <has_restore_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
    } else if (type == "backup") {
      pthread_rwlock_wrlock(&(it->second->backup_priv_set_lock));
      it->second->backup_priv.insert(db_name);
      string update = "INSERT DATA {<" + username + "> <has_backup_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
    } else if (type == "export") {
      pthread_rwlock_wrlock(&(it->second->export_priv_set_lock));
      it->second->export_priv.insert(db_name);
      string update = "INSERT DATA {<" + username + "> <has_export_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
    }
    return 1;
  } else {
    pthread_rwlock_unlock(&users_map_lock);
    return 0;
  }
}
bool delPrivilege(string username, string type, string db_name)
{
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it != users.end() && username != ROOT_USERNAME) {
    pthread_rwlock_unlock(&users_map_lock);
    if (type == "query" && it->second->query_priv.find(db_name) != it->second->query_priv.end()) {
      pthread_rwlock_wrlock(&(it->second->query_priv_set_lock));
      it->second->query_priv.erase(db_name);
      string update = "DELETE DATA {<" + username + "> <has_query_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
      return 1;
    } else if (type == "update" && it->second->update_priv.find(db_name) != it->second->update_priv.end()) {
      pthread_rwlock_wrlock(&(it->second->update_priv_set_lock));
      it->second->update_priv.erase(db_name);
      string update = "DELETE DATA {<" + username + "> <has_update_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
      return 1;
    } else if (type == "load" && it->second->load_priv.find(db_name) != it->second->load_priv.end()) {
      pthread_rwlock_wrlock(&(it->second->load_priv_set_lock));
      it->second->load_priv.erase(db_name);
      string update = "DELETE DATA {<" + username + "> <has_load_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
      return 1;
    } else if (type == "unload" && it->second->unload_priv.find(db_name) != it->second->unload_priv.end()) {
      pthread_rwlock_wrlock(&(it->second->unload_priv_set_lock));
      it->second->unload_priv.erase(db_name);
      string update = "DELETE DATA {<" + username + "> <has_load_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
      return 1;
    } else if (type == "backup" && it->second->backup_priv.find(db_name) != it->second->backup_priv.end()) {
      pthread_rwlock_wrlock(&(it->second->backup_priv_set_lock));
      it->second->backup_priv.erase(db_name);
      string update = "DELETE DATA {<" + username + "> <has_backup_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
      return 1;
    } else if (type == "restore" && it->second->restore_priv.find(db_name) != it->second->restore_priv.end()) {
      pthread_rwlock_wrlock(&(it->second->restore_priv_set_lock));
      it->second->restore_priv.erase(db_name);
      string update = "DELETE DATA {<" + username + "> <has_restore_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
      return 1;
    } else if (type == "export" && it->second->export_priv.find(db_name) != it->second->export_priv.end()) {
      pthread_rwlock_wrlock(&(it->second->export_priv_set_lock));
      it->second->export_priv.erase(db_name);
      string update = "DELETE DATA {<" + username + "> <has_export_priv> <" + db_name + ">.}";
      updateSys(update);

      pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
      return 1;
    }
  }
  pthread_rwlock_unlock(&users_map_lock);
  return 0;
}
bool checkPrivilege(string username, string type, string db_name)
{
  if (db_name == "system")
    return 0;

  if (username == ROOT_USERNAME)
    return 1;
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  //pthread_rwlock_unlock(&users_map_lock);
  if (type == "query") {
    pthread_rwlock_rdlock(&(it->second->query_priv_set_lock));
    if (it->second->query_priv.find(db_name) != it->second->query_priv.end()) {
      pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
      pthread_rwlock_unlock(&users_map_lock);
      return 1;
    }
    pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
  } else if (type == "update") {
    pthread_rwlock_rdlock(&(it->second->update_priv_set_lock));
    if (it->second->update_priv.find(db_name) != it->second->update_priv.end()) {
      pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
      pthread_rwlock_unlock(&users_map_lock);
      return 1;
    }
    pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
  } else if (type == "load") {
    pthread_rwlock_rdlock(&(it->second->load_priv_set_lock));
    if (it->second->load_priv.find(db_name) != it->second->load_priv.end()) {
      pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
      pthread_rwlock_unlock(&users_map_lock);
      return 1;
    }
    pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
  } else if (type == "unload") {
    pthread_rwlock_rdlock(&(it->second->unload_priv_set_lock));
    if (it->second->unload_priv.find(db_name) != it->second->unload_priv.end()) {
      pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
      pthread_rwlock_unlock(&users_map_lock);
      return 1;
    }
    pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
  } else if (type == "restore") {
    pthread_rwlock_rdlock(&(it->second->restore_priv_set_lock));
    if (it->second->restore_priv.find(db_name) != it->second->restore_priv.end()) {
      pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
      pthread_rwlock_unlock(&users_map_lock);
      return 1;
    }
    pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
  } else if (type == "backup") {
    pthread_rwlock_rdlock(&(it->second->backup_priv_set_lock));
    if (it->second->backup_priv.find(db_name) != it->second->backup_priv.end()) {
      pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
      pthread_rwlock_unlock(&users_map_lock);
      return 1;
    }
    pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
  } else if (type == "export") {
    pthread_rwlock_rdlock(&(it->second->export_priv_set_lock));
    if (it->second->export_priv.find(db_name) != it->second->export_priv.end()) {
      pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
      pthread_rwlock_unlock(&users_map_lock);
      return 1;
    }
    pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
  }
  pthread_rwlock_unlock(&users_map_lock);
  return 0;
}
std::string CreateJson(int StatusCode, string StatusMsg, bool body, string ResponseBody)
{
  StringBuffer s;
  PrettyWriter<StringBuffer> writer(s);
  writer.StartObject();
  writer.Key("StatusCode");
  writer.Uint(StatusCode);
  writer.Key("StatusMsg");
  writer.String(StringRef(StatusMsg.c_str()));
  if (body) {
    writer.Key("ResponseBody");
    writer.String(StringRef(ResponseBody.c_str()));
  }
  writer.EndObject();
  return s.GetString();
}
void DB2Map()
{
  string sparql = "select ?x ?y where{?x <has_password> ?y.}";
  string strJson = querySys(sparql);
  //cout << "DDDDDDDDDDDDDDDB2Map: strJson : " << strJson << endl;
  Document document;
  document.Parse(strJson.c_str());
  Value& p1 = document["results"];
  Value& p2 = p1["bindings"];
  //int i = 0;
  for (int i = 0; i < p2.Size(); i++) {
    Value& pp = p2[i];
    Value& pp1 = pp["x"];
    Value& pp2 = pp["y"];
    string username = pp1["value"].GetString();
    string password = pp2["value"].GetString();
    //cout << "DDDDDDDDDDDDDDDDB2Map: username: " + username << " password: " << password << endl;
    struct User* user = new User(username, password);

    string sparql2 = "select ?x ?y where{<" + username + "> ?x ?y.}";
    string strJson2 = querySys(sparql2);
    //cout << "strJson2: " << strJson2 << endl;
    Document document2;
    document2.Parse(strJson2.c_str());

    Value& p12 = document2["results"];
    Value& p22 = p12["bindings"];
    for (int j = 0; j < p22.Size(); j++) {
      Value& ppj = p22[j];
      Value& pp12 = ppj["x"];
      Value& pp22 = ppj["y"];
      string type = pp12["value"].GetString();
      string db_name = pp22["value"].GetString();
      //cout << "DDDDDDDDDDDDDDDDDB2Map: type: " + type << " db_name: " << db_name << endl;

      if (type == "has_query_priv") {
        //cout << username << type << db_name << endl;
        user->query_priv.insert(db_name);
      } else if (type == "has_update_priv") {
        //cout << username << type << db_name << endl;
        user->update_priv.insert(db_name);
      } else if (type == "has_load_priv") {
        user->load_priv.insert(db_name);
      } else if (type == "has_unload_priv") {
        user->unload_priv.insert(db_name);
      } else if (type == "has_restore_priv") {
        user->restore_priv.insert(db_name);
      } else if (type == "has_backup_priv") {
        user->backup_priv.insert(db_name);
      } else if (type == "has_export_priv") {
        user->export_priv.insert(db_name);
      }
    }
    //users.insert(pair<std::string, struct User*>(username, &user));
    users.insert(pair<std::string, struct User*>(username, user));

    //cout << ".................." << user->getUsername() << endl;
    //cout << ".................." << user->getPassword() << endl;
    //cout << ".................." << user->getLoad() << endl;
    //cout << ".................." << user->getQuery() << endl;
    //cout << ".................." << user->getUnload() << endl;
    //cout << "i: " << i << endl;
    //i++;
  }
  //cout << "out of first ptree" << endl;

  //insert already_built database from system.db to already_build map
  sparql = "select ?x where{?x <database_status> \"already_built\".}";
  strJson = querySys(sparql);
  document.Parse(strJson.c_str());
  p1 = document["results"];
  p2 = p1["bindings"];

  for (int i = 0; i < p2.Size(); i++) {
    Value& pp = p2[i];
    Value& pp1 = pp["x"];
    string db_name = pp1["value"].GetString();
    struct DBInfo* temp_db = new DBInfo(db_name);

    string sparql2 = "select ?x ?y where{<" + db_name + "> ?x ?y.}";
    string strJson2 = querySys(sparql2);
    Document document2;
    document2.Parse(strJson2.c_str());

    Value& p12 = document2["results"];
    Value& p22 = p12["bindings"];

    for (int j = 0; j < p22.Size(); j++) {
      Value& ppj = p22[j];
      Value& pp12 = ppj["x"];
      Value& pp22 = ppj["y"];
      string type = pp12["value"].GetString();
      string info = pp22["value"].GetString();

      if (type == "built_by")
        temp_db->setCreator(info);
      else if (type == "built_time")
        temp_db->setTime(info);
    }
    already_build.insert(pair<std::string, struct DBInfo*>(db_name, temp_db));
  }

  //add bulit_time of system.db to already_build map
  sparql = "select ?x where{<system> <built_time> ?x.}";
  strJson = querySys(sparql);
  document.Parse(strJson.c_str());
  p1 = document["results"];
  p2 = p1["bindings"];

  for (int i = 0; i < p2.Size(); i++) {
    Value& pp = p2[i];
    Value& pp1 = pp["x"];
    string built_time = pp1["value"].GetString();
    already_build.find("system")->second->setTime(built_time);
  }

  //get CoreVersion and APIVersion
  sparql = "select ?x where{<CoreVersion> <value> ?x.}";
  strJson = querySys(sparql);
  document.Parse(strJson.c_str());
  p1 = document["results"];
  p2 = p1["bindings"];
  for (int i = 0; i < p2.Size(); i++) {
    Value& pp = p2[i];
    Value& pp1 = pp["x"];
    CoreVersion = pp1["value"].GetString();
  }
  sparql = "select ?x where{<APIVersion> <value> ?x.}";
  strJson = querySys(sparql);
  document.Parse(strJson.c_str());
  p1 = document["results"];
  p2 = p1["bindings"];
  for (int i = 0; i < p2.Size(); i++) {
    Value& pp = p2[i];
    Value& pp1 = pp["x"];
    APIVersion = pp1["value"].GetString();
  }
}

string querySys(string sparql)
{
  string db_name = "system";
  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
  pthread_rwlock_unlock(&already_build_map_lock);

  pthread_rwlock_rdlock(&(it_already_build->second->db_lock));
  ResultSet rs;
  FILE* output = NULL;

  int ret_val = system_database->query(sparql, rs, output);
  bool ret = false, update = false;
  if (ret_val < -1) //non-update query
  {
    ret = (ret_val == -100);
  } else //update query, -1 for error, non-negative for num of triples updated
  {
    update = true;
  }

  if (ret) {
    cout << "search query returned successfully." << endl;

    string success = rs.to_JSON();
    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
    return success;
  } else {
    string error = "";
    int error_code;
    if (!update) {
      cout << "search query returned error." << endl;
      error = "search query returns false.";
      error_code = 403;
    }

    pthread_rwlock_unlock(&(it_already_build->second->db_lock));

    return error;
  }
}

bool sysrefresh()
{
  pthread_rwlock_rdlock(&databases_map_lock);
  system_database->save();
  delete system_database;
  system_database = NULL;
  system_database = new Database("system");
  bool flag = system_database->load();
  cout << "system database refresh" << endl;
  pthread_rwlock_unlock(&databases_map_lock);
  return flag;
}

bool updateSys(string query)
{
  if (query.empty()) {
    return 0;
  }
  printf("query is:\n%s\n\n", query.c_str());
  ResultSet _rs;
  FILE* ofp = stdout;

  string msg;
  int ret = system_database->query(query, _rs, ofp);
  //cout<<"gquery ret: "<<ret<<endl;
  if (ret <= -100) //select query
  {
    if (ret == -100) {
      msg = _rs.to_str();
    } else //query error
    {
      msg = "query failed.";
    }

    return false;
  } else //update query
  {
    if (ret >= 0) {
      msg = "update num: " + Util::int2string(ret);
      cout << msg << endl;
      sysrefresh();
      //system_database->save();
      //delete system_database;
      //system_database=NULL;
      return true;
    } else //update error
    {
      msg = "update failed.";
      cout << msg << endl;
      return false;
    }
  }
}

int copy(string src_path, string dest_path)
{
  string sys_cmd;
  if (!boost::filesystem::exists(src_path)) {
    //check the source path
    cout << "Source Path Error, Please Check It Again!" << endl;
    return 1;
  }
  if (!boost::filesystem::exists(dest_path)) {
    //check the destnation path
    sys_cmd = "mkdir -p ./" + dest_path;
    system(sys_cmd.c_str());
  }

  sys_cmd = "cp -r " + src_path + ' ' + dest_path;
  system(sys_cmd.c_str());
  return 0; // success
}

void backup_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();

  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is backup" << endl;

  //get parameter
  string db_name;
  string username;
  string password;
  string path, _path;
  string timestamp;
  string sys_cmd;
  if (RequestType == "GET") {
    db_name = request->path_match[1];
    username = request->path_match[2];
    password = request->path_match[3];
    path = request->path_match[4];
    db_name = UrlDecode(db_name);
    username = UrlDecode(username);
    password = UrlDecode(password);
    path = UrlDecode(path);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    username = document["username"].GetString();
    password = document["password"].GetString();
    path = document["path"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);

  cout << "check identity successfully." << endl;

  if (checkPrivilege(username, "backup", db_name) == 0) {
    string error = "no unload privilege, operation failed.";
    string resJson = CreateJson(601, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    return;
  }
  cout << "check privilege successfully." << endl;

  //check if the db_name is system
  if (db_name == "system") {
    string error = "no backup privilege, operation failed.";
    string resJson = CreateJson(302, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  //	string db_name = argv[1];
  if (db_name == "") {
    string error = "Exactly 1 argument is required!";
    string resJson = CreateJson(904, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 ok\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }

  string database = db_name;
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    //cout << "Your db name to be built should not end with \".db\"." << endl;
    string error = "Your db name to be built should not end with \".db\".";
    string resJson = CreateJson(202, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    return;
  }

  //check if database named [db_name] is already build.
  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
  if (it_already_build == already_build.end()) {
    string error = "Database not built yet.";
    string resJson = CreateJson(203, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    pthread_rwlock_unlock(&already_build_map_lock);
    return;
  }
  pthread_rwlock_unlock(&already_build_map_lock);

  cout << database << endl;

  if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
    string error = "Unable backup  due to loss of lock";
    string resJson = CreateJson(303, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }
  //backup
  if (path == "")
    path = BACKUP_PATH;
  if (path == ".") {
    cout << "Backup Path Can not be root or empty, Backup Failed!" << endl;
    string error = "Failed to backup the database. Backup Path Can not be root or empty.";
    string resJson = CreateJson(304, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
    return;
  }
  if (path[0] == '.')
    path = path.substr(1, path.length() - 1);
  if (path[0] == '/')
    path = path.substr(1, path.length() - 1);
  if (path[path.length() - 1] == '/')
    path = path.substr(0, path.length() - 1);
  string db_path = db_name + ".db";
  pthread_rwlock_wrlock(&databases_map_lock);
  int ret = copy(db_path, path);
  pthread_rwlock_unlock(&databases_map_lock);
  if (ret == 1) {
    cout << "Database Folder Misssing, Backup Failed!" << endl;
    string error = "Failed to backup the database. Database Folder Misssing.";
    string resJson = CreateJson(305, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
    return;
  } else {
    string time = Util::get_date_time();
    timestamp = Util::get_timestamp();
    cout << "Time:" + time << endl;
    cout << "DB:" + db_name + " Backup done!" << endl;
  }
  path = path + "/" + db_path;
  _path = path + "_" + timestamp;
  sys_cmd = "mv " + path + " " + _path;
  system(sys_cmd.c_str());

  cout << "database backup done." << endl;
  string success = "Database backup successfully.";
  string resJson = CreateJson(0, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

  pthread_rwlock_unlock(&(it_already_build->second->db_lock));
}

bool backup_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&backup_thread, response, request, RequestType);
  t.detach();
  return true;
}

void restore_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();

  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is restore" << endl;

  //get parameter
  string db_name;
  string username;
  string password;
  string path;
  string _path;

  if (RequestType == "GET") {
    db_name = request->path_match[1];
    username = request->path_match[2];
    password = request->path_match[3];
    path = request->path_match[4];
    db_name = UrlDecode(db_name);
    username = UrlDecode(username);
    password = UrlDecode(password);
    path = UrlDecode(path);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    username = document["username"].GetString();
    password = document["password"].GetString();
    path = document["path"].GetString();
  }

  //check identity.
  pthread_rwlock_rdlock(&users_map_lock);
  std::map<std::string, struct User*>::iterator it = users.find(username);
  if (it == users.end()) {
    string error = "username not find.";
    string resJson = CreateJson(903, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  } else if (it->second->getPassword() != password) {
    string error = "wrong password.";
    string resJson = CreateJson(902, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    pthread_rwlock_unlock(&users_map_lock);
    return;
  }
  pthread_rwlock_unlock(&users_map_lock);

  cout << "check identity successfully." << endl;

  if (checkPrivilege(username, "restore", db_name) == 0) {
    string error = "no unload privilege, operation failed.";
    string resJson = CreateJson(601, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    //return false;
    return;
  }
  cout << "check privilege successfully." << endl;

  //check if the db_name is system
  if (db_name == "system") {
    string error = "no restore privilege, operation failed.";
    string resJson = CreateJson(302, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  //	string db_name = argv[1];
  if (db_name == "") {
    string error = "Exactly 1 argument is required!";
    string resJson = CreateJson(904, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 ok\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }

  string database = db_name;
  if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db") {
    //cout << "Your db name to be built should not end with \".db\"." << endl;
    string error = "Your db name to be built should not end with \".db\".";
    string resJson = CreateJson(202, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    return;
  }

  if (path[0] == '/')
    path = '.' + path;
  if (path[path.length() - 1] == '/')
    path = path.substr(0, path.length() - 1);

  if (!boost::filesystem::exists(path)) {
    string error = "Backup Path Error, Restore Failed";
    string resJson = CreateJson(307, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    return;
  }

  cout << database << endl;

  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
  if (it_already_build == already_build.end()) {
    string error = "Database not built yet. Rebuild Now";
    string time = Util::get_backup_time(path, db_name);
    if (time.size() == 0) {
      string error = "Backup Path Does not Match DataBase Name, Restore Failed";
      string resJson = CreateJson(308, error, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
      pthread_rwlock_unlock(&already_build_map_lock);
      return;
    }
    if (addPrivilege(username, "query", db_name) == 0 || addPrivilege(username, "load", db_name) == 0 || addPrivilege(username, "unload", db_name) == 0 || addPrivilege(username, "backup", db_name) == 0 || addPrivilege(username, "restore", db_name) == 0 || addPrivilege(username, "export", db_name) == 0) {
      string error = "add query or load or unload or backup or restore privilege failed.";
      string resJson = CreateJson(912, error, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

      //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
      //return false;
      pthread_rwlock_unlock(&already_build_map_lock);
      return;
    }

    cout << "add query and load and unload and backup and restore privilege succeed after build." << endl;
    struct DBInfo* temp_db = new DBInfo(db_name, username, time);
    already_build.insert(pair<std::string, struct DBInfo*>(db_name, temp_db));
    string update = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." + "<" + db_name + "> <built_by> <" + username + "> ." + "<" + db_name + "> <built_time> \"" + time + "\".}";
    updateSys(update);
    it_already_build = already_build.find(db_name);
  }
  pthread_rwlock_unlock(&already_build_map_lock);

  if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
    string error = "Unable to restore due to loss of lock";
    string resJson = CreateJson(303, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

    //*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
    return;
  }
  //restore
  string sys_cmd = "rm -rf " + db_name + ".db";
  system(sys_cmd.c_str());

  pthread_rwlock_wrlock(&databases_map_lock);
  int ret = copy(path, DB_PATH);
  pthread_rwlock_unlock(&databases_map_lock);

  if (ret == 1) {
    string error = "Failed to restore the database. Backup Path Error";
    string resJson = CreateJson(307, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    cout << "Backup Path Error, Restore Failed!" << endl;
    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
    return;
  } else if (ret == -1) {
    string error = "Failed to restore the database. Database Path Error";
    string resJson = CreateJson(308, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    cout << "Database Path Error, Restore Failed!" << endl;
    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
    return;
  } else {
    //TODO update the in system.db
    string time = Util::get_date_time();
    cout << "Time:" + time << endl;
    cout << "DB:" + db_name + " Restore done!" << endl;
  }

  pthread_rwlock_unlock(&(it_already_build->second->db_lock));
  path = Util::get_folder_name(path, db_name);
  sys_cmd = "mv " + path + " " + db_name + ".db";
  system(sys_cmd.c_str());

  cout << "database restore done." << endl;
  string success = "Database restore successfully.";
  string resJson = CreateJson(0, success, 0);
  *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
}

bool restore_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&restore_thread, response, request, RequestType);
  t.detach();
  return true;
}

bool auto_backup_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
}

void refresh_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  string thread_id = Util::getThreadID();

  string log_prefix = "thread " + thread_id + " -- ";
  cout << log_prefix << "HTTP: this is refresh" << endl;

  //get parameter
  string db_name;
  string username;
  string password;

  if (RequestType == "GET") {
    db_name = request->path_match[1];
    username = request->path_match[2];
    password = request->path_match[3];
    db_name = UrlDecode(db_name);
    username = UrlDecode(username);
    password = UrlDecode(password);
  } else if (RequestType == "POST") {
    auto strJson = request->content.string();
    Document document;
    document.Parse(strJson.c_str());
    db_name = document["db_name"].GetString();
    username = document["username"].GetString();
    password = document["password"].GetString();
  }
  //check the username and password
  pthread_rwlock_rdlock(&users_map_lock);
  string root_password = (users.find(ROOT_USERNAME))->second->getPassword();
  pthread_rwlock_unlock(&users_map_lock);
  //check the identity of root
  if (username == ROOT_USERNAME && password == root_password) {
    if (db_reload(db_name) == 0) {
      string success = "database refresh success!";
      string resJson = CreateJson(908, success, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    } else {
      string error = "database refresh failed";
      string resJson = CreateJson(909, error, 0);
      *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
    }

  } else {
    string error = "Root User Only!";
    string resJson = CreateJson(907, error, 0);
    *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
  }
}

bool refresh_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
  thread t(&refresh_thread, response, request, RequestType);
  t.detach();
  return true;
}

int db_reload(string db_name)
{
  pthread_rwlock_rdlock(&databases_map_lock);
  std::map<std::string, Database*>::iterator iter = databases.find(db_name);
  if (iter == databases.end()) {
    cout << "Database not load yet." << endl;
    pthread_rwlock_unlock(&databases_map_lock);
    return -1;
  }
  pthread_rwlock_rdlock(&already_build_map_lock);
  std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
  pthread_rwlock_unlock(&already_build_map_lock);
  if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0) {
    cout << "Unable to reload due to loss of lock";
    pthread_rwlock_unlock(&databases_map_lock);
    return -1;
  }
  Database* current_database = iter->second;
  delete current_database;
  cout << "Database unloaded." << endl;

  current_database = new Database(db_name);
  iter->second = current_database;
  bool flag = current_database->load();

  if (!flag) {
    cout << "Database loaded failed" << endl;
    pthread_rwlock_unlock(&(it_already_build->second->db_lock));
    pthread_rwlock_unlock(&databases_map_lock);
    return -1;
  }
  cout << "database " << db_name << " reloaded" << endl;
  pthread_rwlock_unlock(&(it_already_build->second->db_lock));
  pthread_rwlock_unlock(&databases_map_lock);
  return 0;
}
