/* 
 * Copyright 2021 gStore, All Rights Reserved. 
 * @Author: Bookug Lober suxunbin liwenjie
 * @Date: 2021-08-22 00:37:57
 * @LastEditTime: 2021-09-19 14:30:28
 * @LastEditors: Please set LastEditors
 * @Description: the http server to handler the user's request, which is main access entrance of gStore
 * @FilePath: /gstore/Main/ghttp.cpp
 */

//SPARQL Endpoint:  log files in logs/endpoint/
//operation.log: not used
//query.log: query string, result num, and time of answering

//TODO: to add db_name to all URLs, and change the index.js using /show to get name, save and set
//TODO: use gzip for network transfer, it is ok to download a gzip file instead of the original one

#include "../Server/server_http.hpp"
#include "../Server/client_http.hpp"
//db
#include "../Database/Database.h"
#include "../Database/Txn_manager.h"
#include "../Util/Util.h"

#include "../tools/rapidjson/document.h"
#include "../tools/rapidjson/prettywriter.h"  
#include "../tools/rapidjson/writer.h"
#include "../tools/rapidjson/stringbuffer.h"
#include <iostream>
#include <fstream>
#include "../Util/IPWhiteList.h"
#include "../Util/IPBlackList.h"
#include "../Util/INIParser.h"
#include "../Util/WebUrl.h"

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
#define	MAX_QUERYLOG_size 800000000 
#define QUERYLOG_PATH "logs/endpoint/"
#define SYSTEM_USERNAME "system"
#define MAX_OUTPUT_SIZE 100000
#define TEST_IP "106.13.13.193"
#define DB_PATH "."
#define BACKUP_PATH "./backups"
//! init the ghttp server
int initialize(int argc, char *argv[]);

int copy(string src_path, string dest_path);

//Added for the default_resource example
void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
        const shared_ptr<ifstream> &ifs);

//! Create a json object to return the request's result.
std::string CreateJson(int StatusCode, string StatusMsg, bool body, string ResponseBody="response");
//! start a thread 
pthread_t start_thread(void *(*_function)(void*));
//！stop a thread
bool stop_thread(pthread_t _thread);
void* func_timer(void* _args);
//! backup scheduler
void* backup_scheduler(void* _args);
void backup_auto(int backup_interval, string backup_path);
void thread_sigterm_handler(int _signal_num);
bool addPrivilege(string username, string type, string db_name);
bool delPrivilege(string username, string type, string db_name);
bool checkPrivilege(string username, string type, string db_name);
void DB2Map();
string querySys(string sparql);
bool sysrefresh();
bool updateSys(string sparql);
bool initSys();
//bool refreshSys();
int db_reload(string db_name);
bool db_checkpoint(string db_name);
txn_id_t get_txn_id(string db_name, string user);


//bool doQuery(string format, string db_query, const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

//=============================================================================

bool request_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType);


void signalHandler(int signum);


//=============================================================================

string checkparamValue(string paramname, string value);

bool checkdbexist(string db_name);

bool checkdbload(string db_name);

string getRemoteIp(const shared_ptr<HttpServer::Request>& request);

bool trylockdb(std::map<std::string, struct DBInfo*>::iterator it_already_build);

void writeLog(FILE* fp, string _info);

void sendResponseMsg(int code, string msg, const shared_ptr<HttpServer::Response>& response);

void build_thread_new(const shared_ptr<HttpServer::Response>& response, string db_name, string db_path, string username, string password);

void load_thread_new(const shared_ptr<HttpServer::Response>& response, string db_name);

void monitor_thread_new(const shared_ptr<HttpServer::Response>& response, string db_name);

void unload_thread_new(const shared_ptr<HttpServer::Response>& response, string db_name);

void drop_thread_new(const shared_ptr<HttpServer::Response>& response, string db_name,string is_backup);

void show_thread_new(const shared_ptr<HttpServer::Response>& response);

void userManager_thread_new(const shared_ptr<HttpServer::Response>& response,string username,string password,string type);

void showuser_thread_new(const shared_ptr<HttpServer::Response>& response);

void userPrivilegeManage_thread_new(const shared_ptr<HttpServer::Response>& response,string username,string privilege,
string type,string db_name);

void backup_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string backup_path);

void restore_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string backup_path,string username);

void query_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string sparql,string format,
string update_flag,string remote_ip,string thread_id,string log_prefix);

void export_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string db_path,string username);

void login_thread_new(const shared_ptr<HttpServer::Response>& response);

void check_thread_new(const shared_ptr<HttpServer::Response>& response);

void begin_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string isolevel,string username);

void tquery_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string TID_s,string sparql);

void commit_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string TID_s);

void rollback_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string TID_s);

void txnlog_thread_new(const shared_ptr<HttpServer::Response>& response,string username);

void checkpoint_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name);

void test_connect_thread_new(const shared_ptr<HttpServer::Response>& response);

void getCoreVersion_thread_new(const shared_ptr<HttpServer::Response>& response);
//TODO: use lock to protect logs when running in multithreading environment
FILE* query_logfp = NULL;
string queryLog = "logs/endpoint/query.log";
mutex query_log_lock;
string CoreVersion;
string APIVersion;
string system_password;
string NAMELOG_PATH  = "name.log";
string backup_path;
int port;
int backup_interval;
int blackList = 0;
int whiteList = 0;
string ipBlackFile = "ipDeny.config";
string ipWhiteFile = "ipAllow.config";
IPWhiteList* ipWhiteList;
IPBlackList* ipBlackList;
pthread_rwlock_t databases_map_lock;
pthread_rwlock_t already_build_map_lock;
pthread_rwlock_t users_map_lock;
pthread_rwlock_t txn_m_lock;

Database *system_database;

std::map<std::string, Database *> databases;
std::map<std::string, shared_ptr<Txn_manager>> txn_managers;
std::map<std::string, txn_id_t> running_txn;
//database information
struct DBInfo{
	private:
		std::string db_name;//! the name of database
		std::string creator; //! the creator of database
		std::string built_time;//! the built time of database;
	public:
		pthread_rwlock_t db_lock;

		DBInfo(){
			pthread_rwlock_init(&db_lock, NULL);
		}
		DBInfo(string _db_name, string _creator, string _time){
			db_name = _db_name;
			creator = _creator;
			built_time = _time;
			pthread_rwlock_init(&db_lock, NULL);
		}
		DBInfo(string _db_name){
			db_name = _db_name;
			pthread_rwlock_init(&db_lock, NULL);
		}
		~DBInfo(){
			pthread_rwlock_destroy(&db_lock);
		}
		std::string getName(){
			return db_name;
		}
		std::string getCreator(){
			return creator;
		}
		void setCreator(string _creator){
			creator = _creator;
		}
		std::string getTime(){
			return built_time;
		}
		void setTime(string _time){
			built_time = _time;
		}
};

//user information
struct User{
	private:
		std::string username;
		std::string password;
	public:
		std::set<std::string> query_priv;//! the query privilege
		std::set<std::string> update_priv;//! the update privilege
		std::set<std::string> load_priv;//! the load privilege
		std::set<std::string> unload_priv;//! the unload privilege
		std::set<std::string> backup_priv;//! the backup privilege
		std::set<std::string> restore_priv;//! the restore privilege
		std::set<std::string> export_priv;//! the export privilege

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

		User(){
			pthread_rwlock_init(&query_priv_set_lock, NULL);
			pthread_rwlock_init(&update_priv_set_lock, NULL);
			pthread_rwlock_init(&load_priv_set_lock, NULL);
			pthread_rwlock_init(&unload_priv_set_lock, NULL);
			pthread_rwlock_init(&backup_priv_set_lock, NULL);
			pthread_rwlock_init(&restore_priv_set_lock, NULL);
			pthread_rwlock_init(&export_priv_set_lock, NULL);
		}
		User(string _username, string _password){
			if(_username == "")
				username = DEFAULT_USERNAME;
			else
				username = _username;
			if(_password == "")
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
		~User(){
			pthread_rwlock_destroy(&query_priv_set_lock);
			pthread_rwlock_destroy(&update_priv_set_lock);
			pthread_rwlock_destroy(&load_priv_set_lock);
			pthread_rwlock_destroy(&unload_priv_set_lock);
			pthread_rwlock_destroy(&backup_priv_set_lock);
			pthread_rwlock_destroy(&restore_priv_set_lock);
			pthread_rwlock_destroy(&export_priv_set_lock);
		}
		std::string getPassword(){
			return password;
		}
		std::string getUsername(){
			return username;
		}
		/// <summary>
		/// @brief get the database list which the user can query/
		/// </summary>
		/// <returns></returns>
		std::string getQuery(){
			std::string query_db;
			if(username == ROOT_USERNAME)
			{
				query_db = "all";
				return query_db;
			}
			std::set<std::string>::iterator it = query_priv.begin();
			while(it != query_priv.end())
			{
				query_db = query_db + *it + ",";
				++it;
			}
			return query_db;
		}
		std::string getUpdate(){
			std::string update_db;
			if(username == ROOT_USERNAME)
			{
				update_db = "all";
				return update_db;
			}
			std::set<std::string>::iterator it = update_priv.begin();
			while(it != update_priv.end())
			{
				update_db = update_db + *it + ",";
				++it;
			}
			return update_db;
		}
		std::string getLoad(){
			std::string load_db;
			if(username == ROOT_USERNAME)
			{
				load_db = "all";
				return load_db;
			}

			std::set<std::string>::iterator it = load_priv.begin();
			while(it != load_priv.end())
			{
				load_db = load_db + *it + ",";
				++it;
			}
			return load_db;
		}
		std::string getUnload(){
			std::string unload_db;
			if(username == ROOT_USERNAME)
			{
				unload_db = "all";
				return unload_db;
			}

			std::set<std::string>::iterator it = unload_priv.begin();
			while(it != unload_priv.end())
			{
				unload_db = unload_db + *it + ",";
				++it;
			}
			return unload_db;
		}
		std::string getrestore(){
			std::string restore_db;
			if(username == ROOT_USERNAME)
			{
				restore_db = "all";
				return restore_db;
			}
			std::set<std::string>::iterator it = restore_priv.begin();
			while(it != restore_priv.end())
			{
				restore_db = restore_db + *it + ",";
				++it;
			}
			return restore_db;
		}
		std::string getbackup(){
			std::string backup_db;
			if(username == ROOT_USERNAME)
			{
				backup_db = "all";
				return backup_db;
			}
			std::set<std::string>::iterator it = backup_priv.begin();
			while(it != backup_priv.end())
			{
				backup_db = backup_db + *it + ",";
				++it;
			}
			return backup_db;
		}
		std::string getexport(){
			std::string export_db;
			if(username == ROOT_USERNAME)
			{
				export_db = "all";
				return export_db;
			}
			std::set<std::string>::iterator it = export_priv.begin();
			while(it != export_priv.end())
			{
				export_db = export_db + *it + ",";
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
std::map<std::string, struct DBInfo *> already_build;
std::map<std::string, struct User *> users;
//struct User root = User(ROOT_USERNAME, ROOT_PASSWORD);
//users.insert(pair<std::string, struct User*>(ROOT_USERNAME, &root));
//users[ROOT_USERNAME] = &root;
//struct User temp_user = User(username2, password2);

int connection_num = 0;

long next_backup = 0;
pthread_t scheduler = 0;

int query_num = 0;



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

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description: decode the url
 * @param {SRC} the encoded url
 * @return {string} the decode url
 */
string UrlDecode(string& SRC)
{
	string ret;
	char ch;
	int ii;
	for(size_t i = 0; i < SRC.length(); ++i)
	{
		if(int(SRC[i]) == 37)
		{
			sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			ret += ch;
			i = i + 2;
		}
		else if(SRC[i] == '+')
		{
			ret += ' ';
		}
		else
		{
			ret += SRC[i];
		}
	}
	return (ret);
}

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description: check the ip where is in the whiteList or in the blackList or not.
 * @param {*}
 * @return {*}
 */
bool ipCheck(const shared_ptr<HttpServer::Request>& request){
	//get the real IP of the client, because we use nginx here
	string remote_ip;
	unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals> m=request->header;
	string map_key = "X-Real-IP";
	//for (auto it = m.begin(); it != m.end(); it ++){
	pair<std::unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals>::iterator,std::unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals>::iterator> lu = m.equal_range(map_key);
	if(lu.first != lu.second)
		remote_ip = lu.first->second;
	else
		remote_ip = request->remote_endpoint_address;
	cout << "remote_ip: " << remote_ip << endl;
	//check if the ip is allow or denied
	if(whiteList == 1){
		return ipWhiteList->Check(remote_ip);
	}
	else if(blackList == 1){
		return ipBlackList->Check(remote_ip);
	}
	return true;
}

class Task
{
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
Task::Task(bool flag, string name, string ft, string query, const shared_ptr<HttpServer::Response>& res, const shared_ptr<HttpServer::Request>& req):response(res),request(req)
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
	//query_thread(update, db_name, format, db_query, response, request);
}

class Thread
{
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

void BackToFree(Thread *t)
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

class ThreadPool
{
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
	for (int i = 0; i < ThreadNum; i++)
	{
		Thread *p = new Thread();
		freethreads.push_back(p);
	}
}
ThreadPool::ThreadPool(int t)
{
	isclose = false;
	ThreadNum = t;
	busythreads.clear();
	freethreads.clear();
	for (int i = 0; i < t; i++)
	{
		Thread *p = new Thread();
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
	while (true)
	{
		if (isclose == true)
		{
			busy_mutex.lock();
			if (busythreads.size() != 0)
			{
				busy_mutex.unlock();
				continue;
			}
			busy_mutex.unlock();
			break;
		}

		free_mutex.lock();
		if (freethreads.size() == 0)
		{
			free_mutex.unlock();
			continue;
		}
		free_mutex.unlock();

		unique_lock<mutex> locker(task_mutex);
		while (tasklines.size() == 0)
			task_cond.wait(locker);

		Task *job = tasklines.front();
		tasklines.pop();
		locker.unlock();

		free_mutex.lock();
		Thread *t = freethreads.back();
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

int main(int argc, char *argv[])
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
		}

		else if (fpid > 0) {
			int status;
			waitpid(fpid, &status, 0);
			if (WIFEXITED(status)) {
				return 0;
			}
			else
			{
				if (Util::file_exist("system.db/port.txt"))
				{
					string cmd = "rm system.db/port.txt";
                    system(cmd.c_str());
				}
				cerr << "Server stopped abnormally, restarting server..." << endl;
				//break;
			}
		}

		else {
			cerr << "Failed to start server: deamon fork failure." << endl;
			return -1;
		}

	}

	return 0;
}

bool isNum(char *str)
{
	for(int i = 0; i < strlen(str); i++)
	{
		int tmp = (int)(str[i]);
		if(tmp < 48 || tmp > 57)
			return false;
	}
	return true;
}

int initialize(int argc, char *argv[])
{
    cout << "ghttp begin initialize..." << endl;
	//Server restarts to use the original database
	//current_database = NULL;
	cout << "init param..." << endl;
	 Util util;
	 Util::configure_new();

	 HttpServer server;
	 string db_name = "";
	 server.config.port = 9000;
	 bool loadCSR = 0;	// DO NOT load CSR by default

	 if (argc < 2)
	 {
		 /*cout << "please input the complete command:\t" << endl;
		 cout << "\t bin/gadd -h" << endl;*/
		 cout << "Use the default port:9000!" << endl;
		 cout << "Not load any database!" << endl;
		 server.config.port = 9000;
		 db_name = "";
		 loadCSR = 0;
		 whiteList = 0;
		 blackList = 0;

	 }
	 else if (argc == 2)
	 {
		 string command = argv[1];
		 if (command == "-h" || command == "--help")
		 {
			 cout << endl;
			 cout << "gStore HTTP Server(ghttp)" << endl;
			 cout << endl;
			 cout << "Usage:\tbin/ghttp -db [dbname] -p [port] -c [enable]" << endl;
			 cout << endl;
			 cout << "Options:" << endl;
			 cout << "\t-h,--help\t\tDisplay this message." << endl;
			 cout << "\t-db,--database[option],\t\t the database name.Default value is empty. Notice that the name can not end with .db" << endl;
			 cout << "\t-p,--port[option],\t\t the listen port. Default value is 9000." << endl;
			 cout << "\t-c,--csr[option],\t\t Enable CSR Struct or not. 0 denote that false, 1 denote that true. Default value is 0." << endl;

			 cout << endl;
			 return 0;
		 }
		 else
		 {
			 //cout << "the command is not complete." << endl;
			 cout << "Invalid arguments! Input \"bin/ghttp -h\" for help." << endl;
			 return 0;
		 }
	 }
	 else
	 {
		 db_name = Util::getArgValue(argc, argv, "db", "database");
		 
		 if (db_name.length() > 3 && db_name.substr(db_name.length() - 3, 3) == ".db")
		 {
			 cout << "Your db name to be built should not end with \".db\"." << endl;
			 return -1;
		 }
		 else if (db_name == "system")
		 {
			 cout << "You can not load system files." << endl;
			 return -1;
		 }
		 string port_str = Util::getArgValue(argc, argv, "p", "port", "9000");
		 server.config.port = Util::string2int(port_str);
		 port=server.config.port;
		 loadCSR = Util::string2int(Util::getArgValue(argc, argv, "c", "csr", "0"));
		 ipWhiteFile = Util::getConfigureValue("ip_allow_path");
		 ipBlackFile = Util::getConfigureValue("ip_deny_path");
		 if (ipWhiteFile.empty())
		 {
			 whiteList = 0;
		 }
		 else
		 {
			 whiteList = 1;
		 }
		 if (ipBlackFile.empty())
		 {
			 blackList = 0;
		 }
		 else
		 {
			 blackList = 1;
		 }

	 }
	 cout << "server port: " << server.config.port << " database name: " << db_name << endl;
	 if (whiteList) {
		 cout << "IP white List enabled." << endl;
		 ipWhiteList = new IPWhiteList();
		 ipWhiteList->Load(ipWhiteFile);
	 }
	 else if (blackList) {
		 cout << "IP black list enabled." << endl;
		 ipBlackList = new IPBlackList();
		 ipBlackList->Load(ipBlackFile);
	 }
	 
    if(Util::file_exist("system.db/port.txt"))
	{
		cout << "Port " << server.config.port << " is already in use." << endl;
	    return -1;
	}
	//users.insert(pair<std::string, struct User *>(ROOT_USERNAME, &root));

	//load system.db when initialize
	if(!Util::dir_exist("system.db"))
	{
		cout << "Can not find system.db."<<endl;
		return -1;
	}
	struct DBInfo *temp_db = new DBInfo("system");
	temp_db->setCreator("root");
	already_build.insert(pair<std::string, struct DBInfo *>("system", temp_db));
	std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find("system");
	if(pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0)
	{
		cout << "Unable to load the database system.db due to loss of lock."<<endl;
		return -1;
	}
	system_database = new Database("system");
	bool flag = system_database->load();
	if(!flag)
	{
		cout << "Failed to load the database system.db."<<endl;
			return -1;
	}
	databases.insert(pair<std::string, Database *>("system", system_database));
	pthread_rwlock_unlock(&(it_already_build->second->db_lock));



	//insert user from system.db to user map
	DB2Map();

	string database = db_name;
	Database* current_database = new Database(database);
	if (database.length() != 0)
	{
		if (Util::dir_exist(database + ".db")==false)
		{
			cout << "Database " << database << ".db has not been built." << endl;
			return -1;
		}
		if (Util::file_exist(database + ".db/success.txt")==false)
		{
			cout << "Database " << database << ".db has not been built successfully." << endl;
			string cmd = "rm -r " + database + ".db";
			system(cmd.c_str());
			return -1;
		}

		bool flag = current_database->load(loadCSR);
		if (!flag)
		{
			cout << "Failed to load the database." << endl;
			delete current_database;
			current_database = NULL;
			return -1;
		}
		shared_ptr<Txn_manager> txn_m = make_shared<Txn_manager>(current_database, database);
		pthread_rwlock_wrlock(&txn_m_lock);
		txn_managers.insert(pair<string, shared_ptr<Txn_manager>>(database, txn_m));
		pthread_rwlock_unlock(&txn_m_lock);
		//string success = db_name;
		//already_build.insert(db_name);
		databases.insert(pair<std::string, Database*>(db_name, current_database));
		//}
	}
	//init transaction log
	Util::init_transactionlog();
	//get the log name
	// string namelog_name = QUERYLOG_PATH + NAMELOG_PATH;
	// FILE* name_logfp = fopen(namelog_name.c_str(), "r+");
	// string querylog_name;
	// if (name_logfp == NULL)   //file not exist, create one
	// {
	// 	name_logfp = fopen(namelog_name.c_str(), "w");
	// 	querylog_name = Util::get_date_time();
	// 	int index_space = querylog_name.find(' ');
	// 	querylog_name = querylog_name.replace(index_space, 1, 1, '_');
	// 	fprintf(name_logfp, "%s", querylog_name.c_str());
	// }
	// else
	// {
	// 	char name_char[100];
	// 	fscanf(name_logfp, "%s", &name_char);
	// 	querylog_name = name_char;
	// }
	// fclose(name_logfp);
	string querylog_name=Util::get_date_day();
	//cout << "querylog_name: " << querylog_name << endl;
	//open the query log
	queryLog = QUERYLOG_PATH + querylog_name + ".log";
	if(Util::file_exist(queryLog)==false)
	{
	   cout<<"query log file is not exists, now create it."<<endl;
       Util::create_file(queryLog);
	}
	cout << "queryLog: " << queryLog << endl;
	query_logfp = fopen(queryLog.c_str(), "a");
	if (query_logfp == NULL)
	{
		cerr << "open query log error" << endl;
		return -1;
	}
	long querylog_size = ftell(query_logfp);

	
	
	// string cmd = "lsof -i:" + Util::int2string(server.config.port) + " > system.db/ep.txt";
	// system(cmd.c_str());
	// fstream ofp;
	// ofp.open("system.db/ep.txt", ios::in);
	// int ch = ofp.get();
	// if (!ofp.eof())
	// {
	// 	ofp.close();
	// 	cout << "Port " << server.config.port << " is already in use." << endl;
	// 	string cmd = "rm system.db/ep.txt";
	// 	system(cmd.c_str());
	// 	return -1;
	// }
	// ofp.close();
	// cmd = "rm system.db/ep.txt";
	// system(cmd.c_str());
    
    fstream ofp;
	system_password = Util::int2string(Util::getRandNum());
	ofp.open("system.db/password" + Util::int2string(server.config.port) + ".txt", ios::out);
	ofp << system_password;
	ofp.close();
	ofp.open("system.db/port.txt", ios::out);
	ofp << server.config.port;
	ofp.close();
	//time_t cur_time = time(NULL);
	//long time_backup = Util::read_backup_time();
	//long next_backup = cur_time - (cur_time - time_backup) % Util::gserver_backup_interval + Util::gserver_backup_interval;
	//NOTICE: no need to backup for endpoint
//TODO: we give up the backup function here
#ifndef ONLY_READ
	scheduler = start_thread(backup_scheduler);
#endif

	pool.create();


    //Default GET-example. If no other matches, this anonymous function will be called.
    //Will respond with content in the web/-directory, and its subdirectories.
    //Default file: index.html
    //Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    server.default_resource["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		//default_handler(server, response, request);
		request_handler(server, response, request,"GET");
		 
    };

	server.default_resource["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		request_handler(server, response, request,"POST");
	};

	

    thread server_thread([&server](){
			//handle the Ctrl+C signal
		    signal(SIGINT, signalHandler);
            //Start server
            server.start();
			cout<<"check: server started"<<endl;
            });

    //Wait for server to start so that the client can connect
    this_thread::sleep_for(chrono::seconds(1));

    

    server_thread.join();
	cout<<"check: server stoped"<<endl;
	//pthread_rwlock_destroy(&database_load_lock);
	pthread_rwlock_destroy(&databases_map_lock);
	pthread_rwlock_destroy(&already_build_map_lock);
	pthread_rwlock_destroy(&users_map_lock);
	pthread_rwlock_destroy(&txn_m_lock);
    return 0;
}

void signalHandler(int signum)
{
	pthread_rwlock_rdlock(&databases_map_lock);
	std::map<std::string, Database *>::iterator iter;
	Util::abort_transactionlog(Util::get_cur_time());
	for (iter = databases.begin(); iter != databases.end(); iter++)
	{
		string database_name = iter->first;
		if (database_name == "system")
			continue;
		db_checkpoint(database_name);
		Database *current_database = iter->second;
		pthread_rwlock_rdlock(&already_build_map_lock);
		std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(database_name);
		pthread_rwlock_unlock(&already_build_map_lock);
		if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0)
		{
			string error = "Unable to checkpoint due to loss of lock";
			cout << error << endl;
			pthread_rwlock_unlock(&databases_map_lock);
			cout << "Stop server failed." << endl;
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
	std::map<std::string, struct DBInfo *>::iterator it_already_build;
	for (it_already_build = already_build.begin(); it_already_build != already_build.end(); it_already_build++)
	{
		struct DBInfo *temp_db = it_already_build->second;
		delete temp_db;
		temp_db = NULL;
	}
	pthread_rwlock_unlock(&already_build_map_lock);

	//string cmd = "rm system.db/password" + Util::int2string(port) + ".txt";
	string cmd = "rm system.db/password" + Util::int2string(port) + ".txt";
	system(cmd.c_str());
	cmd = "rm system.db/port.txt";
        system(cmd.c_str());
	cout << "Server stopped." << endl;
	exit(signum);
}

//QUERY: can server.send() in default_resource_send run in parallism?

//TCP is slower than UDP, but more safely, and can support large data transfer well
//http://blog.csdn.net/javaniuniu/article/details/51419348
void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
        const shared_ptr<ifstream> &ifs) {

    //read and send 128 KB at a time
    static vector<char> buffer(131072); // Safe when server is running on one thread
    streamsize read_length;
	cout<<"int 0"<<endl;
    if((read_length=ifs->read(&buffer[0], buffer.size()).gcount())>0) {
        response->write(&buffer[0], read_length);
	cout<<"int 1"<<endl;
        if(read_length==static_cast<streamsize>(buffer.size())) {
	cout<<"int 2"<<endl;
            server.send(response, [&server, response, ifs](const boost::system::error_code &ec) {
	cout<<"int 3"<<endl;
                    if(!ec)
                    default_resource_send(server, response, ifs);
                    else
                    cerr << "Connection interrupted" << endl;
                    });
        }
    }
}

void backup_auto(int _backup_interval, string _backup_path){
	backup_interval = _backup_interval;
	backup_path = _backup_path;
	start_thread(backup_scheduler);
}

pthread_t start_thread(void *(*_function)(void*)) {
	pthread_t thread;
	if (pthread_create(&thread, NULL, _function, NULL) == 0) {
		return thread;
	}
	return 0;
}

bool stop_thread(pthread_t _thread) {
	return pthread_kill(_thread, SIGTERM) == 0;
}

//DEBUG+TODO: the whole process exits and need to reload the database
//(too slow: quit and restart)
void* func_timer(void* _args) {
	signal(SIGTERM, thread_sigterm_handler);
	sleep(Util::gserver_query_timeout);
	cerr << "Query out of time." << endl;
	//TODO: not use SIGTERM and abort() here, which will cause process to quit abnormally
	//here shoudl just end the timer thread
	abort();
}

void* backup_scheduler(void* _args) {
	signal(SIGTERM, thread_sigterm_handler);
	backup_path = BACKUP_PATH;
	cout << "backup path: " << backup_path << endl;
	time_t cur_time;
    while (true) {
		int timer = Util::string2int(Util::query_backuplog("system", "backup_timer"));
		sleep(timer);
        cur_time = time(NULL);
        //cout << "Time Now: " << cur_time << endl;
        vector<string> db_names;
        Util::search_backuplog(db_names, "is_backup", "true");
		for(int i = 0; i < db_names.size(); i++)
		{
			string db_name = db_names[i];
			int backup_interval = Util::string2int(Util::query_backuplog(db_name, "backup_interval"));
			long last_backup_time = Util::time_to_stamp(Util::query_backuplog(db_name, "last_backup_time"));
			if(last_backup_time + backup_interval > cur_time) continue;
			// db_name should be saved befor backup
			//save(reload) the loaded db and skip the unload db
			if(db_reload(db_name) != 0){
				cout << "Backup Failed" << endl;
				continue;
			}
			string db_path = db_name + ".db";
			int ret = copy(db_path, backup_path);
			if(ret == 1){
				cout << "DB: " << db_name << " backup failed due to loss of file" << endl;
				continue;
			}
			db_path = backup_path + '/' + db_path;
		
			string _db_path = db_path + '_' +  Util::get_timestamp();
			Util::update_backuplog(db_name, "last_backup_time", Util::get_date_time());
			string sys_cmd = "mv " + db_path + ' ' + _db_path;
			system(sys_cmd.c_str());
			cout << "DB: " << db_name << " backup success" << endl;
			cout << "Backup Time: " << Util::get_date_time() << endl;
		}
		//cout << "auto backup done!" << endl;
    }
}

void thread_sigterm_handler(int _signal_num) {
	pthread_exit(0);
}




/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description: build the database 
 * @param {string} db_name: the name of database that would build.
 * @param {string} db_path: the data file path
 * @param {string} username: username
 * @param {string} password: password
 * @return {*}
 */
void build_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string db_path,string username,string password)
{
	if (db_path == SYSTEM_PATH)
	{
		string error = "You have no rights to access system files";
		sendResponseMsg(1002, error, response);
		return;
	}
	string result = checkparamValue("db_name", db_name);
	if (result.empty() == false)
	{
		sendResponseMsg(1003, result, response);
		return;
	}
	
	//check if database named [db_name] is already built
	if (checkdbexist(db_name))
	{
		string error = "database already built.";
		sendResponseMsg(1004, error, response);
		return;
	}
	string dataset = db_path;
	string database = db_name;
	cout << "Import dataset to build database..." << endl;
	cout << "DB_store: " << database << "\tRDF_data: " << dataset << endl;
	int len = database.length();
	Database* current_database = new Database(database);
	bool flag = current_database->build(dataset);
	delete current_database;
	current_database = NULL;
	if (!flag)
	{
		string error = "Import RDF file to database failed.";
		string cmd = "rm -r " + database + ".db";
		system(cmd.c_str());
		sendResponseMsg(1005, error, response);
		return;
	}

	ofstream f;
	f.open("./" + database + ".db/success.txt");
	f.close();

	//by default, one can query or load or unload the database that is built by itself, so add the database name to the privilege set of the user
	if (addPrivilege(username, "query", db_name) == 0 || addPrivilege(username, "load", db_name) == 0 || addPrivilege(username, "unload", db_name) == 0 || addPrivilege(username, "backup", db_name) == 0 || addPrivilege(username, "restore", db_name) == 0 || addPrivilege(username, "export", db_name) == 0)
	{
		string error = "add query or load or unload privilege failed.";
		sendResponseMsg(1006, error, response);
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
	string update = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." +
		"<" + db_name + "> <built_by> <" + username + "> ." + "<" + db_name + "> <built_time> \"" + time + "\".}";
	updateSys(update);
	cout << "database add done." << endl;
	// string success = db_name + " " + db_path;
	string success = "Import RDF file to database done.";
	sendResponseMsg(0, success, response);
	Util::add_backuplog(db_name);
}

/**
 * @description: response message to client 
 * @Author: liwenjie
 * @param {int} code: StatusCode value
 * @param {string} msg:StatusMsg value
 * @param {const} shared_ptr
 */
void sendResponseMsg(int code, string msg, const shared_ptr<HttpServer::Response>& response)
{
	string resJson = CreateJson(code, msg, 0);
	cout<<"response result:"<<endl;
	cout<<resJson<<endl;
	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

}

/**
 * @description: check the param is legal or not.
 * @Author: liwenjie
 * @param {string} paramname: if paramname is db_name, then check the value is end with ".db" or not.
 * @param {string} value: the value of param.
 */
string checkparamValue(string paramname, string value)
{
	string result = "";
	if (value.empty()) 
	{
		result = "the value of " + paramname + " can not be empty!";
		return result;
	}
	if (paramname == "db_name")
	{
		string database = value;
		if (database == "system")
		{
			result = "you can not operate the system database";
			return result;
		}
		if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db")
		{
			//cout << "Your db name to be built should not end with \".db\"." << endl;
			result = "Your db name to be built should not end with \".db\".";
			return result;
		}
		
	}
	return "";
}

/**
 * @description: check the database is exist or not.
 * @Author: liwenjie
 * @param {string} db_name: the operation database name.
 */
bool checkdbexist(string db_name)
{
	bool result = true;
	pthread_rwlock_rdlock(&already_build_map_lock);
	
	std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
	if (it_already_build == already_build.end())
	{
		result=false;
	}
	pthread_rwlock_unlock(&already_build_map_lock);
	return result;
}

/**
 * @description: check the database has loaded or not.
 * @Author: liwenjie
 * @param {string} db_name
 */
bool checkdbload(string db_name)
{
	bool result = false;
	pthread_rwlock_rdlock(&databases_map_lock);
	if (databases.find(db_name) != databases.end())
	{
		result = true;
	}
	pthread_rwlock_unlock(&databases_map_lock);
	return result;
}

string getRemoteIp(const shared_ptr<HttpServer::Request>& request)
{
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
	return remote_ip;
}

/**
 * @description: try lock the write and read lock the database.
 * @Author: liwenjie
 * @param {struct} DBInfo
 */
bool trylockdb(std::map<std::string, struct DBInfo*>::iterator it_already_build)
{
	bool result = false;
	
	if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0)
	{
		//  pthread_rwlock_unlock(&(it_already_build->second->db_lock));
		//  cout<<"unlock the db_lock"<<endl;
		//  pthread_rwlock_wrlock(&(it_already_build->second->db_lock));
		// cout<<"wrlock the db_lock"<<endl;
		// result=true;
		return false;
		
	}
	else
	{
		result = true;
	}
	return result;
		
}
/**
 * @description: load the database to memory.
 * @Author: liwenjie
 * @param {const} shared_ptr
 * @param {string} db_name
 */
void load_thread_new(const shared_ptr<HttpServer::Response>& response, string db_name)
{
	string error = checkparamValue("db_name", db_name);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	if (checkdbexist(db_name)==false)
	{
		error = "the database ["+db_name+"] not built yet.";
		sendResponseMsg(1004, error, response);
		return;
	}
	if (checkdbload(db_name))
	{
		error = "the database already load yet.";
		sendResponseMsg(0, error, response);
		return;
	}
	
	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
	
	if (trylockdb(it_already_build)==false)
	{
		error = "the operation can not been excuted due to loss of lock.";
		sendResponseMsg(1007, error, response);
		pthread_rwlock_unlock(&already_build_map_lock);
		return;
	}
	else
	{
		pthread_rwlock_unlock(&already_build_map_lock);
		if(pthread_rwlock_trywrlock(&databases_map_lock)!=0)
	   {
		   error = "Unable to load due to loss of lock";
		
			sendResponseMsg(1007,error,response);
			return;

	   }
		cout<<"begin loading..."<<endl;
		string database = db_name;
		Database* current_database = new Database(database);
		bool flag = current_database->load();
		cout<<"end loading..."<<endl;
		shared_ptr<Txn_manager> txn_m = make_shared<Txn_manager>(current_database, database);
		pthread_rwlock_wrlock(&txn_m_lock);
		cout<<"begin txn manager..."<<endl;
		txn_managers.insert(pair<string, shared_ptr<Txn_manager>>(database, txn_m));
		pthread_rwlock_unlock(&txn_m_lock);
		cout<<"txt_managers:"<<txn_managers.size()<<endl;
		//delete current_database;
		//current_database = NULL;
		//cout << "load done." << endl;
		if (!flag)
		{
			error = "Failed to load the database.";
			sendResponseMsg(1005, error, response);
			return;
		}
	// 	if(pthread_rwlock_trywrlock(&databases_map_lock)!=0)
	//    {
	// 	//    cout<<"can not wrlock the databases map  lock!"<<endl;
    //     //    pthread_rwlock_unlock(&databases_map_lock);
	// 	//    cout<<"unlock the databases map  lock!"<<endl;
	// 	//    pthread_rwlock_wrlock(&databases_map_lock);
	// 	//    cout<<"relock the databases map  lock!"<<endl;
	// 	    error = "Unable to load due to loss of lock";
	// 		sendResponseMsg(1007,error,response);
	// 		return;
	//    }
		cout<<"wrlock the database map"<<endl;
		databases.insert(pair<std::string, Database*>(db_name, current_database));
		cout<<"insert the pair into the databases"<<endl;
		pthread_rwlock_unlock(&databases_map_lock);

		cout << "database insert done." << endl;
		string success = "Database loaded successfully.";
		sendResponseMsg(0, success, response);
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
	}




}

/**
 * @description: get the database monitor info
 * @Author:liwenjie
 * @param {const} shared_ptr response point
 * @param {string} db_name
 * @return {*}
 */
void monitor_thread_new(const shared_ptr<HttpServer::Response>& response, string db_name)
{
	//check the param value is legal or not.
	string error=checkparamValue("db_name",db_name);
	if(error.empty()==false)
	{
		sendResponseMsg(1003,error,response);
		return;
	}
	if (checkdbexist(db_name)==false)
	{
		error = "the database ["+db_name+"] not built yet.";
		sendResponseMsg(1004, error, response);
		return;
	}
	// if (checkdbload(db_name))
	// {
	// 	error = "the database already load yet.";
	// 	sendResponseMsg(0, error, response);
	// }
	pthread_rwlock_rdlock(&databases_map_lock);
	std::map<std::string, Database *>::iterator iter = databases.find(db_name);
	
	if(iter == databases.end())
	{
		//cout << "database not loaded yet." << endl;
		error = "Database not load yet.";
		sendResponseMsg(1004,error,response);
		pthread_rwlock_unlock(&databases_map_lock);
		return;
	}
	Database *_database = iter->second;
	pthread_rwlock_unlock(&databases_map_lock);

    pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(db_name);
	string creator = it_already_build->second->getCreator();
	string time = it_already_build->second->getTime();
	pthread_rwlock_unlock(&already_build_map_lock);

	if(pthread_rwlock_tryrdlock(&(it_already_build->second->db_lock)) != 0)
	{
		string error = "Unable to monitor due to loss of lock";
		string resJson = CreateJson(1007, error, 0);
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
		
		
		//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return;
	}
	//use JSON format to send message
	Document resDoc;
	resDoc.SetObject();
	Document::AllocatorType &allocator = resDoc.GetAllocator();
	resDoc.AddMember("StatusCode", 0, allocator);
	resDoc.AddMember("StatusMsg", "success", allocator);
	string name = _database->getName();
	resDoc.AddMember("database", StringRef(name.c_str()), allocator);
	resDoc.AddMember("creator", StringRef(creator.c_str()), allocator);
	resDoc.AddMember("built_time", StringRef(time.c_str()), allocator);
	char tripleNumString[128];

	//sprintf(tripleNumString, "%u", _database->getTripleNum());
	//resDoc.AddMember("triple num", StringRef( tripleNumString), allocator);
	/*不知道为什么要做这种处理？显示不下吗？*/
	/*resDoc.AddMember("triple num", _database->getTripleNum(), allocator);*/

	sprintf(tripleNumString, "%lld", _database->getTripleNum());
	//string triplenumstr = tripleNumString;
	//cout << "triple num:" << triplenumstr << endl;

	resDoc.AddMember("triple num", StringRef(tripleNumString), allocator);
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

	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
	
	//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
	pthread_rwlock_unlock(&(it_already_build->second->db_lock));
}

/**
 * @description: unload a database from memory 
 * @Author:liwenjie 
 * @param {const} shared_ptr response point
 * @param {string} db_name the database name
 * @return {*}
 */
void unload_thread_new(const shared_ptr<HttpServer::Response> &response, string db_name)
{

	string error = checkparamValue("db_name", db_name);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	if (checkdbexist(db_name) == false)
	{
		error = "the database [" + db_name + "] not built yet.";
		sendResponseMsg(1004, error, response);
		return;
	}
	if (checkdbload(db_name) == false)
	{
		error = "the database not load yet.";
		sendResponseMsg(0, error, response);
		return;
	}
	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(db_name);
	pthread_rwlock_unlock(&already_build_map_lock);
	if (trylockdb(it_already_build) == false)
	{
		error = "the operation can not been excuted due to loss of lock.";
		sendResponseMsg(1007, error, response);
		return;
	}
	else
	{
		pthread_rwlock_wrlock(&databases_map_lock);
		std::map<std::string, Database *>::iterator iter = databases.find(db_name);
		if (iter == databases.end())
		{
			string error = "Database not load yet.";
			sendResponseMsg(1004, error, response);
			pthread_rwlock_unlock(&databases_map_lock);
			pthread_rwlock_unlock(&(it_already_build->second->db_lock));
			return;
		}
		pthread_rwlock_unlock(&databases_map_lock);
        cout<<"unload txn_manager:"<<txn_managers.size()<<endl;
		pthread_rwlock_wrlock(&txn_m_lock);
		if (txn_managers.find(db_name) == txn_managers.end())
		{
			error = "transaction manager can not find the database";
			sendResponseMsg(1008, error, response);
			pthread_rwlock_unlock(&txn_m_lock);
            pthread_rwlock_unlock(&(it_already_build->second->db_lock));
			
			
			//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
			return;
		}
		pthread_rwlock_unlock(&txn_m_lock);
		db_checkpoint(db_name);
		txn_managers.erase(db_name);
		Database *current_database = iter->second;
		db_checkpoint(db_name);
		delete current_database;
		current_database = NULL;
		databases.erase(db_name);
		
		string success = "Database unloaded.";
		sendResponseMsg(0, success, response);
		//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
		//	pthread_rwlock_unlock(&database_load_lock);
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
	
	}
}

/**
 * @description:drop a database
 * @Author:liwenjie 
 * @param {const shared_ptr<HttpServer::Response>} &response
 * @param {string} db_name: the database name
 * @param {string} is_backup: true:logical drop,the file is not deleted; false:force drop, the file also is deleted.
 * @return {*}
 */
void drop_thread_new(const shared_ptr<HttpServer::Response> &response, string db_name, string is_backup)
{
	string error = checkparamValue("db_name", db_name);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	if (checkdbexist(db_name) == false)
	{
		error = "the database [" + db_name + "] not built yet.";
		sendResponseMsg(1004, error, response);
		return;
	}
	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(db_name);
	pthread_rwlock_unlock(&already_build_map_lock);
	if (trylockdb(it_already_build) == false)
	{
		error = "the operation can not been excuted due to loss of lock.";
		sendResponseMsg(1007, error, response);
	}
	else
	{
		pthread_rwlock_wrlock(&databases_map_lock);
		std::map<std::string, Database *>::iterator iter = databases.find(db_name);
		string time = "";
		if (iter != databases.end())
		{
			//@ the database has loaded, unload it firstly
			Database *current_database = iter->second;
			time = ""; //没有啥用
			delete current_database;
			current_database = NULL;
			databases.erase(db_name);
			cout<<"remove it from loaded database list"<<endl;
			//@ remove the database from the already build list
			already_build.erase(db_name);
			cout<<"remove the database from the already build database list"<<endl;
			pthread_rwlock_unlock(&databases_map_lock);
			pthread_rwlock_unlock(&(it_already_build->second->db_lock));
		}
		else
		{

			pthread_rwlock_unlock(&databases_map_lock);
			//drop database named [db_name]
			pthread_rwlock_rdlock(&already_build_map_lock);
			struct DBInfo *temp_db = it_already_build->second;
			time = temp_db->getTime(); //没有啥用
			delete temp_db;
			temp_db = NULL;
			already_build.erase(db_name);
			cout<<"remove the database from the already build database list"<<endl;
			pthread_rwlock_unlock(&already_build_map_lock);
		}
		//@ delete the database info from  the system database
		string update = "DELETE WHERE {<" + db_name + "> ?x ?y.}";
		updateSys(update);
		string cmd;
		
		if (is_backup == "false")
			cmd = "rm -r " + db_name + ".db";
		else if (is_backup == "true")
			cmd = "mv " + db_name + ".db " + db_name + ".bak";
		cout<<"delete the file: "<<cmd<<endl;
		system(cmd.c_str());
		
		Util::delete_backuplog(db_name);
		string success = "Database " + db_name + " dropped.";
		sendResponseMsg(0, success, response);
		return;
	}
}
/**
 * @description: show the all database list (except system database)
 * @Author:liwenjie
 * @param {const shared_ptr<HttpServer::Response>} &response
 * @return {*}
 */
void show_thread_new(const shared_ptr<HttpServer::Response> &response)
{
    pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build;
	string success;
	Document resDoc;
	resDoc.SetObject();
	Document::AllocatorType &allocator = resDoc.GetAllocator();

	resDoc.AddMember("StatusCode", 0, allocator);
	resDoc.AddMember("StatusMsg", "Get the database list successfully!", allocator);
	Value jsonArray(kArrayType);

	for (it_already_build = already_build.begin(); it_already_build != already_build.end(); it_already_build++)
	{
		string database_name = it_already_build->first;
		string creator = it_already_build->second->getCreator();
		string time = it_already_build->second->getTime();
		if ((database_name == "system"))
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

	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json" << "\r\n\r\n" << resJson;


	//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
	//return true;
	pthread_rwlock_unlock(&already_build_map_lock);
	return;
}

/**
 * @description: user manager (include add user,delete user,alter user),notice that  only support alter user's password
 * @Author:liwenjie
 * @param {const} shared_ptr
 * @param {string} username
 * @param {string} password
 * @param {string} type :the operation type, 1: addUser 2:deleteUser 3:alterUser
 * @return {*}
 */
void userManager_thread_new(const shared_ptr<HttpServer::Response>& response,string username,string password,string type)
{
	string error="";
     if(type=="1")
	 {
		 //add user
		 if(username.empty()||password.empty())
		 {
            error="the user name and password can not be empty while adding user.";
			sendResponseMsg(1003,error,response);
			return;
		 }
		 else{
			pthread_rwlock_wrlock(&users_map_lock);
			if(users.find(username) == users.end())
			{
				cout << "user ready to add." << endl;				
				struct User *temp_user = new User(username, password);
				users.insert(pair<std::string, struct User *>(username, temp_user));
				string update = "INSERT DATA {<" + username + "> <has_password> \"" + password + "\".}";
				updateSys(update);		
				error="user add done.";
				sendResponseMsg(0,error,response);
			}
			else
			{
				error = "username already existed, add user failed.";
			    sendResponseMsg(1004,error,response);
				
			}
			pthread_rwlock_unlock(&users_map_lock);
			return;
		 }
	 }
	 else if(type=="2")
	 {
        //delete user
		pthread_rwlock_wrlock(&users_map_lock);
		std::map<std::string, struct User *>::iterator iter;
		iter = users.find(username);
		if(iter != users.end() && username != ROOT_USERNAME)
		{
				delete iter->second;
				iter->second = NULL;
				users.erase(username);
				string update = "DELETE where {<" + username + "> ?p ?o.}";
				updateSys(update);
				sendResponseMsg(0,"delete user done.",response);
				
			}
			else
			{
				string error;
				int error_code;
				if(username == ROOT_USERNAME)
				{
					error = "you cannot delete root, delete user failed.";
					
				}
				else
				{
					error = "username not exist, delete user failed.";
					
				}
				sendResponseMsg(1004,error,response);
				pthread_rwlock_unlock(&users_map_lock);
				return;
			}
			pthread_rwlock_unlock(&users_map_lock);
			return;

	 }
	 else if(type=="3")
	 {
		 pthread_rwlock_wrlock(&users_map_lock);
			std::map<std::string, struct User *>::iterator iter;
			iter = users.find(username);
		
			if(iter == users.end())
			{
				error = "username not exist, change password failed.";
				sendResponseMsg(1004,error,response);
				pthread_rwlock_unlock(&users_map_lock);
				return;
				
			}
			else
			{
				iter->second->setPassword(password);
				string update = "DELETE WHERE {<" + username + "> <has_password> ?o.}";
				updateSys(update);
				string update2 = "INSERT DATA {<" + username + "> <has_password>  \"" + password + "\".}";
				updateSys(update2);
				sendResponseMsg(0,"change password done.",response);
				
			}
			pthread_rwlock_unlock(&users_map_lock);
			return;
	 }
	 else
	 {
		 error="the operation is not support.";
		 sendResponseMsg(1003,error,response);
		 return;
	 }
}

/**
 * @description: show all user list.
 * @Author:liwenjie
 * @param {const} shared_ptr
 * @return {*}
 */
void showuser_thread_new(const shared_ptr<HttpServer::Response>& response)
{
    pthread_rwlock_rdlock(&users_map_lock);
	if (users.empty())
	{
		string error = "No Users.";
		sendResponseMsg(0,error,response);
		//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		pthread_rwlock_unlock(&users_map_lock);
		return;
	}
	string success;

	Document resDoc;
	resDoc.SetObject();
	Document::AllocatorType &allocator = resDoc.GetAllocator();
	resDoc.AddMember("StatusCode", 0, allocator);
	resDoc.AddMember("StatusMsg", "success", allocator);
	Value json_array(kArrayType);
    std::map<std::string, struct User *>::iterator it;
	for (it = users.begin(); it != users.end(); it++)
	{
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

/**
 * @description: manage the user privilege, include add user privilege, delete user privilege
 * @Author:liwenjie
 * @param {const} shared_ptr
 * @param {string} username
 * @param {string} privilege 1:query,2:load,3:unload,4:update,5:backup,6:restore,7:export, you can set multi privileges by using , to split.
 * @param {string} type: the operation type,1: add privilege, 2: delete privilege
 * @param {string} db_name: the opration database
 * @return {*}
 */
void userPrivilegeManage_thread_new(const shared_ptr<HttpServer::Response> &response, string username,
									string privilege, string type, string db_name)
{
	string error = "";
	error = checkparamValue("db_name", db_name);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	error = checkparamValue("privilege", privilege);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	error=checkparamValue("type",type);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	if (username == ROOT_USERNAME)
	{
		string error = "you can't add privilege to root user.";
		sendResponseMsg(1004, error, response);
		//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return;
	}
	vector<string> privileges;
	if (privilege.substr(privilege.length() - 1, 1) != ",")
	{
		privilege = privilege + ",";
	}

	Util::split(privilege, ",", privileges);
	string result = "";
	for (int i = 0; i < privileges.size(); i++)
	{
		string temp_privilege_int = privileges[i];
		string temp_privilege = "";
		if (temp_privilege_int.empty())
		{
			continue;
		}
		if (temp_privilege_int == "1")
		{
			temp_privilege = "query";
		}
		else if (temp_privilege_int == "2")
		{
			temp_privilege = "load";
		}
		else if (temp_privilege_int == "3")
		{
			temp_privilege = "unload";
		}
		else if (temp_privilege_int == "4")
		{
			temp_privilege = "update";
		}
		else if (temp_privilege_int == "5")
		{
			temp_privilege = "backup";
		}
		else if (temp_privilege_int == "6")
		{
			temp_privilege = "restore";
		}
		else if (temp_privilege_int == "7")
		{
			temp_privilege = "export";
		}
		if (temp_privilege.empty() == false)
		{
			if (type == "1")
			{
				if (addPrivilege(username, temp_privilege, db_name) == 0)
				{
					result = result + "add privilege " + temp_privilege + " failed. \r\n";
				}
				else
				{
					result = result + "add privilege " + temp_privilege + " successfully. \r\n";
				}
			}
			else if (type == "2")
			{
				if (delPrivilege(username, temp_privilege, db_name) == 0)
				{
					result = result + "delete privilege " + temp_privilege + " failed. \r\n";
				}
				else
				{
					result = result + "delete privilege " + temp_privilege + " successfully. \r\n";
				}
			}
			else
			{
				result = "the operation type is not support.";
				sendResponseMsg(1003, result, response);
				return;
			}
		}
	}
	sendResponseMsg(0, result, response);
	return;
}

/**
 * @description: backup a database 
 * @Author:liwenjie
 * @param {const} shared_ptr
 * @param {string} db_name:the operation database name
 * @param {string} backup_path: the backup path
 * @return {*}
 */
void backup_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string backup_path)
{
	string error="";
	error=checkparamValue("db_name",db_name);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	if (checkdbexist(db_name) == false)
	{
		error = "the database [" + db_name + "] not built yet.";
		sendResponseMsg(1004, error, response);
		return;
	}
	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(db_name);
	pthread_rwlock_unlock(&already_build_map_lock);
	if (trylockdb(it_already_build) == false)
	{
		error = "the operation can not been excuted due to loss of lock.";
		sendResponseMsg(1007, error, response);
		return;
	}
	//begin backup database
	string path=backup_path;
	if(path == "") path = BACKUP_PATH;
	if(path == "." ){
		cout << "Backup Path Can not be root or empty, Backup Failed!" << endl;
		string error = "Failed to backup the database. Backup Path Can not be root or empty.";
		sendResponseMsg(1003,error,response);
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
		return;
	}
	if(path[0] == '.') path = path.substr(1, path.length() - 1);
	//if(path[0] == '/') path = path.substr(1, path.length() - 1);
	if(path[path.length() - 1] == '/') path = path.substr(0, path.length() - 1);
	string db_path = db_name + ".db";
	pthread_rwlock_wrlock(&databases_map_lock);
	int ret = copy(db_path, path);
	pthread_rwlock_unlock(&databases_map_lock);
	string timestamp="";
	if(ret == 1){
		cout << "Database Folder Misssing, Backup Failed!" << endl;
		string error = "Failed to backup the database. Database Folder Misssing.";
		sendResponseMsg(1005,error,response);
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
		return;
	}
	else{
		string time = Util::get_date_time();
		timestamp = Util::get_timestamp();
		cout << "Time:" + time << endl;
		cout << "DB:" + db_name + " Backup done!" << endl;
	}
	path = path + "/" + db_path;
	string _path = path + "_" + timestamp;
	string sys_cmd = "mv " + path + " " + _path;
	cout<<"backup command:"<<sys_cmd<<endl;
	system(sys_cmd.c_str());

	cout << "database backup done." << endl;
	string success = "Database backup successfully.";

	Document resDoc;
	resDoc.SetObject();
	Document::AllocatorType &allocator = resDoc.GetAllocator();
	resDoc.AddMember("StatusCode", 0, allocator);
	resDoc.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
	resDoc.AddMember("backupfilepath", StringRef(_path.c_str()), allocator);
	StringBuffer resBuffer;
	PrettyWriter<StringBuffer> resWriter(resBuffer);
	resDoc.Accept(resWriter);
	string resJson = resBuffer.GetString();

	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
	
	pthread_rwlock_unlock(&(it_already_build->second->db_lock));

}

/**
 * @description:restore the database
 * @Author:liwenjie
 * @param {const} shared_ptr
 * @param {string} db_name the operation database name
 * @param {string} backup_path the backup path
 * @param {string} username the operation username
 * @return {*}
 */
void restore_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string backup_path,string username)
{
	string error="";
	error=checkparamValue("db_name",db_name);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	string path=backup_path;
	//if(path[0] == '/') path = '.' + path;
	if(path[path.length() - 1] == '/') path = path.substr(0, path.length()-1);

	if(Util::dir_exist(path)==false){
		string error = "Backup Path not exist, Restore Failed";
		sendResponseMsg(1003,error,response);
		return;
	}
	string database=db_name;
	cout << "restore "<< database << endl;

	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(db_name);
	if(it_already_build == already_build.end())
	{
		string error = "Database not built yet. Rebuild Now";
		string time = Util::get_backup_time(path, db_name);
		if(time.size() == 0){
			string error = "Backup Path Does not Match DataBase Name, Restore Failed";
			sendResponseMsg(1003,error,response);
			pthread_rwlock_unlock(&already_build_map_lock);
			return;
		}
		if(addPrivilege(username, "query", db_name) == 0 || 
		addPrivilege(username, "load", db_name) == 0 || 
		addPrivilege(username, "unload", db_name) == 0 || 
		addPrivilege(username, "backup", db_name) == 0 || 
		addPrivilege(username, "restore", db_name) == 0 || 
		addPrivilege(username, "export", db_name) == 0)
		{
			string error = "add query or load or unload or backup or restore privilege failed.";

			sendResponseMsg(1005,error,response);
			
			//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
			//return false;
			pthread_rwlock_unlock(&already_build_map_lock);
			return;
		}

		cout << "add query and load and unload and backup and restore privilege succeed after build." << endl;
		struct DBInfo *temp_db = new DBInfo(db_name, username, time);
		already_build.insert(pair<std::string, struct DBInfo *>(db_name, temp_db));
		string update = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." +
		"<" + db_name + "> <built_by> <" + username + "> ." + "<" + db_name + "> <built_time> \"" + time + "\".}";
		updateSys(update);
		Util::add_backuplog(db_name);
		it_already_build = already_build.find(db_name);
	}
	pthread_rwlock_unlock(&already_build_map_lock);

	if(pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0)
	{
		string error = "Unable to restore due to loss of lock";

		sendResponseMsg(1007,error,response);
		
		//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return;
	}

	//restore
	string sys_cmd = "rm -rf " + db_name + ".db";
	system(sys_cmd.c_str());
	
	pthread_rwlock_wrlock(&databases_map_lock);
	int ret  = copy(path, DB_PATH);
	pthread_rwlock_unlock(&databases_map_lock);

	if(ret == 1){
		string error = "Failed to restore the database. Backup Path Error";
		sendResponseMsg(1005,error,response);
		cout << "Backup Path Error, Restore Failed!" << endl;
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
		return;
	}else{
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
	sendResponseMsg(0,success,response);
}

/**
 * @description: query the database.
 * @Author:liwenjie
 * @param {const} shared_ptr
 * @param {string} db_name: the operation database name.
 * @param {string} sparql: the sparql
 * @param {string} format: json/html/file
 * @param {string} file:the out file path
 * @return {*}
 */
void query_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string sparql,string format,
string update_flag,string remote_ip,string thread_id,string log_prefix)
{
    string error="";
	error=checkparamValue("db_name",db_name);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	error=checkparamValue("sparql",sparql);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	if(checkdbexist(db_name)==false)
	{
		error="Database not build yet.";
		sendResponseMsg(1004,error,response);
		return;
	}
	if(checkdbload(db_name)==false)
	{
		error="Database not load yet.";
		sendResponseMsg(1004,error,response);
		return;
	}
	bool update_flag_bool=true;
	if(update_flag=="0")
	{
       update_flag_bool=false;
	}
	string db_query=sparql;
	cout<<"check: "<<db_query<<endl;
	string str = db_query;



	pthread_rwlock_rdlock(&databases_map_lock);
	std::map<std::string, Database *>::iterator iter = databases.find(db_name);
	if(iter == databases.end())
	{
		string error = "Database not load yet.";
	//cout << error << endl;
		sendResponseMsg(1004,error,response);
		pthread_rwlock_unlock(&databases_map_lock);
		return;
	}
	cout<<"the database is loaded.."<<endl;
	Database *current_database;
	std::map<std::string, struct DBInfo *>::iterator it_already_build;
	try
	{
		/* code */
		current_database = iter->second;
		if (current_database == NULL)
		{
			cout << "current_database is null" << endl;
		}
		else
		{
			cout << "current_database:" << current_database->getName() << endl;
		}
		pthread_rwlock_unlock(&databases_map_lock);

		pthread_rwlock_rdlock(&already_build_map_lock);

		it_already_build = already_build.find(db_name);
		if (it_already_build == already_build.end())
		{
			cout << "not find the database " << db_name << endl;
		}
		else
		{
			cout << "find name:" << it_already_build->second->getName() << endl;
		}
		pthread_rwlock_unlock(&already_build_map_lock);

		pthread_rwlock_rdlock(&(it_already_build->second->db_lock));
		cout<<"lock the database"<<endl;
	}
	catch (const std::exception &e)
	{
		cout << e.what() << '\n';
		cout<<" focus the bug "<<endl;
	}
	catch (...)
	{
		cout << "unknow error!" << endl;
	}
	cout<<"test debug1..."<<endl;
	
	// Database *current_database = iter->second;
	// pthread_rwlock_unlock(&databases_map_lock);

	// pthread_rwlock_rdlock(&already_build_map_lock);
	// std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(db_name);
	// pthread_rwlock_unlock(&already_build_map_lock);
	
	// pthread_rwlock_rdlock(&(it_already_build->second->db_lock));


	FILE* output = NULL;

	ResultSet rs;
	int query_time = Util::get_cur_time();
	int ret_val;
	//catch exception when this is an update query and has no update privilege
	try{
		cout << "begin query..." << endl;
		ret_val = current_database->query(sparql, rs, output, update_flag_bool,false,nullptr);
	}catch(string exception_msg){
	
		string content=exception_msg;
		cout << exception_msg;
		sendResponseMsg(1005,content,response);
		return;	
	}
	catch(const std::runtime_error& e2)
	{
		string content =e2.what();
		cout<<"query failed:"<<content<<endl;
		sendResponseMsg(1005,content,response);
		return;	
	}
	catch (...)
	{
		string content = "unknow error";
		cout<<"query failed:"<<content<<endl;
		sendResponseMsg(1005,content,response);
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
	cout <<"ansNum_s: " << ansNum_s << endl;
	
	string filename = Util::getTimeString2()+"_"+Util::int2string(Util::getRandNum())+".txt";
	
	Util::create_dir("query_result/");
    string localname = "query_result/" + filename;
	if (ret)
	{
		cout << log_prefix << "search query returned successfully." << endl;

		//record each query operation, including the sparql and the answer number
		//accurate down to microseconds
		char time_str[100];
		struct timeval tv;
		gettimeofday(&tv, NULL);
		int s = tv.tv_usec / 1000;
		int y = tv.tv_usec % 1000;

		string query_start_time = Util::get_date_time() + ":" + Util::int2string(s) + "ms" + ":" + Util::int2string(y) + "microseconds";

		cout << "remote_ip: " << remote_ip << endl;

		//filter the IP from the test server
		if (remote_ip != TEST_IP)
		{
			Document doc;
			doc.SetObject();
			Document::AllocatorType &doc_allocator = doc.GetAllocator();
			doc.AddMember("QueryDateTime", StringRef(query_start_time.c_str()), doc_allocator);
			doc.AddMember("RemoteIP", StringRef(remote_ip.c_str()), doc_allocator);
			doc.AddMember("Sparql", StringRef(sparql.c_str()), doc_allocator);
			doc.AddMember("AnsNum", rs.ansNum, doc_allocator);
			doc.AddMember("Format", StringRef(format.c_str()), doc_allocator);
			doc.AddMember("FileName", StringRef(filename.c_str()), doc_allocator);
			// string QueryTime = Util::int2string(query_time) + "ms";
			doc.AddMember("QueryTime", query_time, doc_allocator);
			StringBuffer buffer;
			PrettyWriter<StringBuffer> writer(buffer);
			doc.Accept(writer);
			writeLog(query_logfp, buffer.GetString());
		}

		//string log_info = Util::get_date_time() + "\n" + sparql + "\n\nanswer num: " + Util::int2string(rs.ansNum)+"\nquery time: "+Util::int2string(query_time) +" ms\n-----------------------------------------------------------\n";

		//to void someone downloading all the data file by sparql query on purpose and to protect the data
		//if the ansNum too large, for example, larger than 100000, we limit the return ans.
		if (rs.ansNum > MAX_OUTPUT_SIZE)
		{
			if (rs.output_limit == -1 || rs.output_limit > MAX_OUTPUT_SIZE)
				rs.output_limit = MAX_OUTPUT_SIZE;
		}

		ofstream outfile;
		string ans = "";
		string success = "";
		//TODO: if result is stored in Stream instead of memory?  (if out of memory to use to_str)
		//BETTER: divide and transfer, in multiple times, getNext()
		if (format == "json")
		{
			success = rs.to_JSON();
			Document resDoc;
			Document::AllocatorType &allocator = resDoc.GetAllocator();
			resDoc.Parse(success.c_str());
			resDoc.AddMember("StatusCode", 0, allocator);
			resDoc.AddMember("StatusMsg", "success", allocator);
			resDoc.AddMember("AnsNum", rs.ansNum, allocator);
			resDoc.AddMember("OutputLimit", rs.output_limit, allocator);
	        resDoc.AddMember("QueryTime", StringRef(Util::int2string(query_time).c_str()), allocator);
			StringBuffer resBuffer;
			PrettyWriter<StringBuffer> resWriter(resBuffer);
			resDoc.Accept(resWriter);
			string resJson = resBuffer.GetString();
			//   localname = localname + "." + format;
			//   filename = filename + "." + format;
			//   filename = "";
			//   filename = "sparql." + format;
			//   cout << log_prefix << "filename: " << filename << endl;

			*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length();
			*response << "\r\nCache-Control: no-cache"
					  << "\r\nPragma: no-cache"
					  << "\r\nExpires: 0";
			*response << "\r\n\r\n"
					  << resJson;
			cout << "query complete! unlock the database " << endl;
			pthread_rwlock_unlock(&(it_already_build->second->db_lock));
			cout << "query complete! unlock the database successfully! " << endl;
			return;
			//return true;
			//
		}
		else if (format == "file")
		{
			success = rs.to_str();
			// 	localname = localname + ".txt";
			//    filename = filename + ".txt";
			outfile.open(localname);
			outfile << success;
			outfile.close();

			StringBuffer s;
			PrettyWriter<StringBuffer> writer(s);
			writer.StartObject();
			writer.Key("StatusCode");
			writer.Uint(0);
			writer.Key("StatusMsg");
			writer.String(StringRef("success"));
			writer.Key("AnsNum");
			writer.Uint(rs.ansNum);
			writer.Key("OutputLimit");
			writer.Uint(rs.output_limit);
			
			writer.Key("QueryTime");
			writer.String(StringRef(Util::int2string(query_time).c_str()));
			writer.Key("FileName");
			writer.String(StringRef(filename.c_str()));
			writer.EndObject();
			string resJson = s.GetString();

			*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length();
			*response << "\r\nCache-Control: no-cache"
					  << "\r\nPragma: no-cache"
					  << "\r\nExpires: 0";
			*response << "\r\n\r\n"
					  << resJson;

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
		}
		else if (format == "json+file" || format == "file+json")
		{
			success = rs.to_str();
			// 	localname = localname + ".txt";
			//    filename = filename + ".txt";
			outfile.open(localname);
			outfile << success;
			outfile.close();
			success = rs.to_JSON();
			Document resDoc;
			Document::AllocatorType &allocator = resDoc.GetAllocator();
			resDoc.Parse(success.c_str());
			resDoc.AddMember("StatusCode", 0, allocator);
			resDoc.AddMember("StatusMsg", "success", allocator);
			resDoc.AddMember("AnsNum", rs.ansNum, allocator);
			resDoc.AddMember("OutputLimit", rs.output_limit, allocator);
	        resDoc.AddMember("QueryTime", StringRef(Util::int2string(query_time).c_str()), allocator);
		
			resDoc.AddMember("FileName", StringRef(filename.c_str()), allocator);
			StringBuffer resBuffer;
			PrettyWriter<StringBuffer> resWriter(resBuffer);
			resDoc.Accept(resWriter);
			string resJson = resBuffer.GetString();
			*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length();
			*response << "\r\nCache-Control: no-cache"
					  << "\r\nPragma: no-cache"
					  << "\r\nExpires: 0";
			*response << "\r\n\r\n"
					  << resJson;
			cout << "query complete! unlock the database " << endl;
			pthread_rwlock_unlock(&(it_already_build->second->db_lock));
			cout << "query complete! unlock the database successfully! " << endl;
			return;
		}
		else if (format == "sparql-results+json")
		{

			success = rs.to_JSON(); // convert the result to json
			*response << "HTTP/1.1 200 OK\r\nContent-Type: application/sparql-results+json\r\nContent-Length: " << success.length();
			*response << "\r\nCache-Control: no-cache"
					  << "\r\nPragma: no-cache"
					  << "\r\nExpires: 0";
			*response << "\r\n\r\n"
					  << success; // success contains the json-encoded result

			pthread_rwlock_unlock(&(it_already_build->second->db_lock));
			return;
		}
		else
		{
			localname = localname + ".txt";
			filename = filename + ".txt";
			cout << log_prefix << "filename: " << filename << endl;
			filename = "";
			success = rs.to_str();
			filename = "sparql.txt";
			cout << log_prefix << "filename: " << filename << endl;
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length();
			*response << "\r\nContent-Type: application/octet-stream";
			*response << "\r\nContent-Disposition: attachment; filename=\"" << filename << '"';
			*response << "\r\n\r\n"
					  << success;

			pthread_rwlock_unlock(&(it_already_build->second->db_lock));
			return;
			//return true;
			//
		}
	}
	else
	{
		string error = "";
		int error_code;
		if (update)
		{
			cout << log_prefix << "update query returned correctly." << endl;
			error = "update query returns true.";
			error_code = 0;
		}
		else
		{
			cout << log_prefix << "search query returned error." << endl;
			error = "search query returns false.";
			error_code = 1005;
		}
		sendResponseMsg(error_code, error, response);
		//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
		//return false;
		return;
	}
}

/**
 * @description: export the database
 * @Author:liwenjie
 * @param {string} db_name
 * @param {string} db_path
 * @param {string} username
 * @return {*}
 */
void export_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string db_path,string username)
{
	string error="";
	error=checkparamValue("db_name",db_name);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	error=checkparamValue("db_path",db_path);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	if(checkdbexist(db_name)==false)
	{
		error="Database not build yet.";
		sendResponseMsg(1004,error,response);
		return;
	}
	if(db_path[db_path.length()-1] != '/')
		db_path = db_path + "/";
	if(Util::dir_exist(db_path)==false)
		Util::create_dir(db_path);
	db_path = db_path + db_name +"_"+Util::get_timestamp()+ ".nt";

	//check if database named [db_name] is already load
	Database *current_database;
	pthread_rwlock_rdlock(&databases_map_lock);
	std::map<std::string, Database *>::iterator iter = databases.find(db_name);
	if(iter == databases.end())
	{
		string error = "Database not load yet.";
	    sendResponseMsg(1004,error,response);
		pthread_rwlock_unlock(&databases_map_lock);
		return;
	}
	current_database = iter->second;
	pthread_rwlock_unlock(&databases_map_lock);
	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(db_name);
	pthread_rwlock_unlock(&already_build_map_lock);

	pthread_rwlock_rdlock(&(it_already_build->second->db_lock));

	string sparql = "select * where {?x ?y ?z.} ";
	ResultSet rs;
	cout<<"db_path:"<<db_path<<endl;
	FILE* ofp = fopen(db_path.c_str(), "w");
    int ret = current_database->query(sparql, rs, ofp, true, true);
    fflush(ofp);
	fclose(ofp);
	ofp = NULL;

	string success = "Export the database successfully.";


     Document resDoc;
	resDoc.SetObject();
	Document::AllocatorType &allocator = resDoc.GetAllocator();
	resDoc.AddMember("StatusCode", 0, allocator);
	resDoc.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
	resDoc.AddMember("filepath", StringRef(db_path.c_str()), allocator);
	StringBuffer resBuffer;
	PrettyWriter<StringBuffer> resWriter(resBuffer);
	resDoc.Accept(resWriter);
	string resJson = resBuffer.GetString();

	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
	
	pthread_rwlock_unlock(&(it_already_build->second->db_lock));

	// string resJson = CreateJson(0, success, 0);
	// *response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
	// sendResponseMsg(0,success,response);
	// pthread_rwlock_unlock(&(it_already_build->second->db_lock));
	return;
}

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description: login ghttp
 * @param {*}
 * @return {*}
 */
void login_thread_new(const shared_ptr<HttpServer::Response>& response)
{
	string success="login successfully.";
	 Document resDoc;
	resDoc.SetObject();
	Document::AllocatorType &allocator = resDoc.GetAllocator();
	resDoc.AddMember("StatusCode", 0, allocator);
	resDoc.AddMember("StatusMsg", "login successfully", allocator);
	string version=Util::getConfigureValue("version");
	resDoc.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
	string licensetype=Util::getConfigureValue("licensetype");
	resDoc.AddMember("licensetype",StringRef(licensetype.c_str()), allocator);

	StringBuffer resBuffer;
	PrettyWriter<StringBuffer> resWriter(resBuffer);
	resDoc.Accept(resWriter);
	string resJson = resBuffer.GetString();

	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
	
}
/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description: check the ghttp server activity 
 * @param {*}
 * @return {*}
 */
void check_thread_new(const shared_ptr<HttpServer::Response>& response)
{
    string success="the ghttp server is running...";
	sendResponseMsg(0,success,response);
}
/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description: begin a transaction
 * @param {string} db_name 
 * @param {string} isolevel : the Isolation level， 1:RC(read committed) 2:SI(snapshot isolation) 3:SR(seriablizable）
 * @return {*}
 */
void begin_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string isolevel,string username)
{
    string error="";
	error=checkparamValue("db_name",db_name);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	error=checkparamValue("isolevel",isolevel);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	int level=Util::string2int(isolevel);
	if(level<=0||level>3)
	{
		error="the Isolation level's value only can been 1/2/3";
		sendResponseMsg(1003,error,response);
		return;
	}
	if (checkdbexist(db_name) == false)
	{
		error = "the database [" + db_name + "] not built yet.";
		sendResponseMsg(1004, error, response);
		return;
	}
	
	pthread_rwlock_wrlock(&databases_map_lock);
	std::map<std::string, Database*>::iterator iter = databases.find(db_name);
	if (iter == databases.end())
	{
	    error = "Database not load yet.";
		sendResponseMsg(1004,error,response);
		pthread_rwlock_unlock(&databases_map_lock);
		return;
	}
	Database* current_database = iter->second;
	pthread_rwlock_unlock(&databases_map_lock);
	pthread_rwlock_rdlock(&txn_m_lock);
	if (txn_managers.find(db_name) == txn_managers.end())
	{
	     error = "Database transaction manager error.";
		sendResponseMsg(1004,error,response);
		return;
	}

	shared_ptr<Txn_manager> txn_m = txn_managers[db_name];
	pthread_rwlock_unlock(&txn_m_lock);
	
	cerr << "IsolationLevelType:" << level << endl;
	txn_id_t TID = txn_m->Begin(static_cast<IsolationLevelType>(level));
	cout <<"Transcation Id:"<< to_string(TID) << endl;
	cout << to_string(txn_m->Get_Transaction(TID)->GetStartTime()) << endl;
	Util::add_transactionlog(db_name, username, to_string(TID), to_string(txn_m->Get_Transaction(TID)->GetStartTime()), "RUNNING", "INF");
	if (TID == INVALID_ID)
	{
		error = "transaction begin failed.";
	    sendResponseMsg(1005,error,response);
		return;
	}
	//TODO: write the transaction log
	//string idx = db_name + "_" + username;
	//string idx = db_name + "_" + username;
	//running_txn.insert(pair<string, txn_id_t>(idx, TID));
	string TID_s = to_string(TID);

	StringBuffer s;
	PrettyWriter<StringBuffer> writer(s);
	writer.StartObject();
	writer.Key("StatusCode");
	writer.Uint(0);
	writer.Key("StatusMsg");
	writer.String(StringRef("transaction begin success"));
	writer.Key("TID");
	writer.Key(StringRef(TID_s.c_str()));
	writer.EndObject();
	string resJson =  s.GetString();
	//string resJson = CreateJson(992, success, 0);
	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
}

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description: query database with transcation 
 * @param {string} db_name 
 * @param {string} TID: the transcation id while begin transcation
 * @param {string} sparql:the query sparql (only support insert and delete)
 * @return {*}
 */
void tquery_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string TID_s,string sparql)
{
     string error="";
	 error=checkparamValue("db_name",db_name);
	 if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	error=checkparamValue("TID",TID_s);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	
	txn_id_t TID;
	
	if(Util::is_number(TID_s))
	{
		TID = strtoull(TID_s.c_str(), NULL, 0);
	}
	else
	{
		 error = "TID is not a pure number. TID: " + TID_s;
		 sendResponseMsg(1003,error,response);
		return;
	}
	error=checkparamValue("sparql",sparql);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	cout << "try to get databases_map_lock" << endl;
	pthread_rwlock_wrlock(&databases_map_lock);
	std::map<std::string, Database*>::iterator iter = databases.find(db_name);
	if (iter == databases.end())
	{
		error = "Database not load yet.";
		sendResponseMsg(1004,error,response);
		pthread_rwlock_unlock(&databases_map_lock);
		return;
	}
	pthread_rwlock_unlock(&databases_map_lock);
	pthread_rwlock_rdlock(&txn_m_lock);
	if (txn_managers.find(db_name) == txn_managers.end())
	{
		error = "Database transaction manager error.";
		sendResponseMsg(1004,error,response);
		return;
	}
	auto txn_m = txn_managers[db_name];
	pthread_rwlock_unlock(&txn_m_lock);
	cout << "sparql:    " << sparql << endl;
	string res;
	int ret = txn_m->Query(TID, sparql, res);
	if(ret == -1)
	{
		 error = "Transaction query failed due to wrong TID";
		sendResponseMsg(1005,error,response);
		return;
	}
	else if(ret == -10)
	{
		 error = "Transaction query failed due to wrong database status";
		sendResponseMsg(1005,error,response);
		return;
	}
	else if(ret == -99)
	{
		 error = "Transaction query failed. This transaction is not in running status!";
		sendResponseMsg(1005,error,response);
		return;
	}
	else if(ret == -100)
	{
		
		Document resDoc;
	
		cout<<"res:"<<res<<endl;
		Document::AllocatorType &allocator = resDoc.GetAllocator();
		resDoc.Parse(res.c_str());
		if(resDoc.HasParseError())
		{
			cout<<"error parse"<<endl;
			resDoc.Parse("{}");
			resDoc.AddMember("result",StringRef(res.c_str()),allocator);
		}
		resDoc.AddMember("StatusCode", 0, allocator);
		resDoc.AddMember("StatusMsg", "success", allocator);
		StringBuffer resBuffer;
		PrettyWriter<StringBuffer> resWriter(resBuffer);
		resDoc.Accept(resWriter);
		res = resBuffer.GetString();
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << res.length();
		*response << "\r\nContent-Type: application/octet-stream";
		*response << "\r\n\r\n" << res;
		return;
	}
	else if(ret == -20)
	{
		error = "Transaction query failed. This transaction is set abort due to conflict!";
		sendResponseMsg(1005,error,response);
		return;
	}
	else if(ret == -101)
	{
		error = "Transaction query failed. Unknown query error";
		sendResponseMsg(1005,error,response);
		return;
	}
	else
	{
		string success = "Transaction query success, update num: " + Util::int2string(ret);
		sendResponseMsg(0,success,response);
		return;
	}
}

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description: commit a transcation
 * @param {string} TID_s
 * @return {*}
 */
void commit_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string TID_s)
{
     string error="";
	 error=checkparamValue("db_name",db_name);
	 if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	error=checkparamValue("TID",TID_s);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	string res;
	txn_id_t TID;
	
	if(Util::is_number(TID_s))
	{
		TID = strtoull(TID_s.c_str(), NULL, 0);
	}
	else
	{
		 error = "TID is not a pure number. TID: " + TID_s;
		 sendResponseMsg(1003,error,response);
		return;
	}
	if(checkdbexist(db_name)==false)
	{
		error = "Database not built yet. ";
		 sendResponseMsg(1004,error,response);
		return;
	}

	if(checkdbload(db_name)==false)
	{
		error="Database not load yet.";
		 sendResponseMsg(1004,error,response);
		return;
	}


	pthread_rwlock_rdlock(&txn_m_lock);
	if (txn_managers.find(db_name) == txn_managers.end())
	{
		string error = "Database transaction manager error.";
		 sendResponseMsg(1004,error,response);
		return;
	}
	shared_ptr<Txn_manager> txn_m = txn_managers[db_name];
	pthread_rwlock_unlock(&txn_m_lock);

		
	int ret = txn_m->Commit(TID);
	//string idx = db_name + "_" + username;
	//running_txn.erase(idx);
	if (ret == 1)
	{
		 error = "transaction not in running state! commit failed. TID: " + TID_s;
		sendResponseMsg(1005,error,response);
		return;
	}
	else if (ret == -1)
	{
		error = "transaction not found, commit failed. TID: " + TID_s;
		sendResponseMsg(1005,error,response);
		return;
	}
	else
	{
		Util::update_transactionlog(to_string(TID), "COMMITED", to_string(txn_m->Get_Transaction(TID)->GetEndTime()));
		string success = "transaction commit success. TID: " + TID_s;
		sendResponseMsg(0,success,response);
		return;
	}

}

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description: rollback a transcation 
 * @param {string} db_name
 * @param {string} TID_s
 * @return {*}
 */
void rollback_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name,string TID_s)
{
    string error="";
	 error=checkparamValue("db_name",db_name);
	 if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	error=checkparamValue("TID",TID_s);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	string res;
	txn_id_t TID;
	
	if(Util::is_number(TID_s))
	{
		TID = strtoull(TID_s.c_str(), NULL, 0);
	}
	else
	{
		 error = "TID is not a pure number. TID: " + TID_s;
		 sendResponseMsg(1003,error,response);
		return;
	}
	if(checkdbexist(db_name)==false)
	{
		error = "Database not built yet. ";
		 sendResponseMsg(1004,error,response);
		return;
	}

	if(checkdbload(db_name)==false)
	{
		error="Database not load yet.";
		 sendResponseMsg(1004,error,response);
		return;
	}


	pthread_rwlock_rdlock(&txn_m_lock);
	if (txn_managers.find(db_name) == txn_managers.end())
	{
		string error = "Database transaction manager error.";
		 sendResponseMsg(1004,error,response);
		return;
	}
	shared_ptr<Txn_manager> txn_m = txn_managers[db_name];
	pthread_rwlock_unlock(&txn_m_lock);

		
	int ret = txn_m->Commit(TID);
	//string idx = db_name + "_" + username;
	//running_txn.erase(idx);
	if (ret == 1)
	{
		 error = "transaction not in running state! rollback failed. TID: " + TID_s;
		sendResponseMsg(1005,error,response);
		return;
	}
	else if (ret == -1)
	{
		error = "transaction not found, rollback failed. TID: " + TID_s;
		sendResponseMsg(1005,error,response);
		return;
	}
	else
	{
		Util::update_transactionlog(to_string(TID), "ROLLBACK", to_string(txn_m->Get_Transaction(TID)->GetEndTime()));
		string success = "transaction rollback success. TID: " + TID_s;
		sendResponseMsg(0,success,response);
		return;
	}
}


/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description: get transcation log  
 * @param {*}
 * @return {*}
 */
void txnlog_thread_new(const shared_ptr<HttpServer::Response>& response,string username)
{
    if(username==ROOT_USERNAME)
	{
		string resJson = Util::get_transactionlog();
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
	}
	else
	{
		string error = "Root User Only!";
		sendResponseMsg(1003,error,response);
	}
}

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved. 
 * email：liwenjiehn@pku.edu.cn
 * @description:this is used for checkpoint, we must ensure that modification is written to disk
 * @param {string} db_name
 * @return {*}
 */
void checkpoint_thread_new(const shared_ptr<HttpServer::Response>& response,string db_name)
{
    string error;
	 error=checkparamValue("db_name",db_name);
	 if (error.empty() == false)
	{
		sendResponseMsg(1003, error, response);
		return;
	}
	if(checkdbexist(db_name)==false)
	{
		error="database not built yet.";
		sendResponseMsg(1004,error,response);
		return;
	}
	
	pthread_rwlock_rdlock(&databases_map_lock);
	std::map<std::string, Database *>::iterator iter = databases.find(db_name);
	if(iter == databases.end())
	{
		error = "Database not load yet.";
	    sendResponseMsg(1004,error,response);
		pthread_rwlock_unlock(&databases_map_lock);
		return;
	}
	Database *current_database = iter->second;
	pthread_rwlock_unlock(&databases_map_lock);	

	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
	pthread_rwlock_unlock(&already_build_map_lock);
	if (trylockdb(it_already_build)==false)
	{
		error = "the operation can not been excuted due to loss of lock.";
		sendResponseMsg(1004, error, response);
		return;
	}
	else
	{
		
		current_database->save();
	//NOTICE: this info is in header
	string success = "Database saved successfully.";
	//header and content are split by an empty line
     sendResponseMsg(0,success,response);
	
	 pthread_rwlock_unlock(&(it_already_build->second->db_lock));
	 return;
	}


}

 void test_connect_thread_new(const shared_ptr<HttpServer::Response>& response)
 {
     Document resDoc;
	resDoc.SetObject();
	Document::AllocatorType &allocator = resDoc.GetAllocator();
	resDoc.AddMember("StatusCode", 0, allocator);
	resDoc.AddMember("StatusMsg", "success", allocator);
	string version=Util::getConfigureValue("version");
	resDoc.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
	string licensetype=Util::getConfigureValue("licensetype");
	resDoc.AddMember("licensetype",StringRef(licensetype.c_str()), allocator);

	StringBuffer resBuffer;
	PrettyWriter<StringBuffer> resWriter(resBuffer);
	resDoc.Accept(resWriter);
	string resJson = resBuffer.GetString();

	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
  
 }

 void getCoreVersion_thread_new(const shared_ptr<HttpServer::Response>& response)
 {

	 Document resDoc;
	resDoc.SetObject();
	Document::AllocatorType &allocator = resDoc.GetAllocator();
	resDoc.AddMember("StatusCode", 0, allocator);
	resDoc.AddMember("StatusMsg", "success", allocator);
	string version=Util::getConfigureValue("version");
	resDoc.AddMember("CoreVersion", StringRef(version.c_str()), allocator);

	StringBuffer resBuffer;
	PrettyWriter<StringBuffer> resWriter(resBuffer);
	resDoc.Accept(resWriter);
	string resJson = resBuffer.GetString();

	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
 }



string checkIdentity(string username, string password)
{
	//check identity.
	pthread_rwlock_rdlock(&users_map_lock);
	std::map<std::string, struct User*>::iterator it = users.find(username);
	if (it == users.end())
	{
		string error = "username not find.";
		pthread_rwlock_unlock(&users_map_lock);
		return error;
	}
	else if (it->second->getPassword() != password)
	{
		string error = "wrong password.";
		
		pthread_rwlock_unlock(&users_map_lock);
		return error;
	}
	pthread_rwlock_unlock(&users_map_lock);

	cout << "check identity successfully." << endl;

	return "";
}



void request_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
	if (!ipCheck(request)) {
		string error = "IP Blocked!";
		sendResponseMsg(1101, error, response);
		return;
	}
	string thread_id = Util::getThreadID();
	string log_prefix = "thread " + thread_id + " -- ";
	string username;
	string password;
	string operation;
	string db_name;
	Document document;
	string url;
	string remote_ip=getRemoteIp(request);
	cout << "request method:" << request->method << endl;
	cout << "request http_version:" << request->http_version << endl;
	cout << "request type:" << RequestType << endl;
	
	if (RequestType == "GET")
	{
		cout << "request path:" << request->path << endl;

		url = request->path;
		url=UrlDecode(url);
		cout << "request path:" << url << endl;
		operation=WebUrl::CutParam(url, "operation");
		username = WebUrl::CutParam(url, "username");
		password = WebUrl::CutParam(url, "password");
	
		username = UrlDecode(username);
		
		password = UrlDecode(password);
		db_name = WebUrl::CutParam(url, "db_name");
		
	}
	else if (RequestType == "POST")
	{
		auto strJson = request->content.string();
        cout<<"post content"<<endl;
		cout<<strJson<<endl;

		document.Parse(strJson.c_str());
		if (document.HasParseError())
		{
			string error = "the post content is not fit the json format,content=" + strJson;
			sendResponseMsg(1003, error, response);
			return;
		}

		operation="";
		db_name="";
		username="";
		password="";
		if(document.HasMember("operation")&&document["operation"].IsString())
		{
			operation=document["operation"].GetString();
		}
		if(document.HasMember("db_name")&&document["db_name"].IsString())
		{
			db_name=document["db_name"].GetString();
		}
		if(document.HasMember("username")&&document["username"].IsString())
		{
			username=document["username"].GetString();
		}
		if(document.HasMember("password")&&document["password"].IsString())
		{
			password=document["password"].GetString();
		}
			
	}
	else
	{
		string msg = "The method type " + request->method + " is not support";
		sendResponseMsg(1000, msg, response);
		return;
	}
	if(operation=="check")
	{
        check_thread_new(response);
		return;
	}

	string checkidentityresult = checkIdentity(username, password);
	if (checkidentityresult.empty() == false)
	{
		sendResponseMsg(1001, checkidentityresult, response);
		return;
	}
	if (checkPrivilege(username, operation, db_name) == 0)
	{
		string msg = "You have no " + operation + " privilege, operation failed";
		sendResponseMsg(1002, msg, response);
		return;
	}
	cout << log_prefix << "HTTP: this is " <<operation<< endl;

    // build database
	if (operation == "build")
	{
		string db_path="";
		if (RequestType == "GET")
		{
			db_path = WebUrl::CutParam(url, "db_path");
			db_name = UrlDecode(db_name);
			db_path = UrlDecode(db_path);
			
		}
		else if (RequestType == "POST")
		{
			try
			{
				if (document.HasMember("db_path")&&document["db_path"].IsString())
				{
					db_path = document["db_path"].GetString();
				}
			}
			catch (...)
			{
				string error = "the parameter has some error,please look up the api document.";
				sendResponseMsg(1003, error, response);
				return;
			}

			
		}
		build_thread_new(response, db_name, db_path, username, password);	
	}
	//load dababase
	else if (operation == "load")
	{
		load_thread_new(response, db_name);
	}
	//monitor database
	else if (operation == "monitor")
	{
        monitor_thread_new(response,db_name);
	}
	//unload database
	else if(operation=="unload")
	{
		cout<<"unload txn_manager:"<<txn_managers.size()<<endl;
        unload_thread_new(response,db_name);
	}
	//drop database
	else if(operation=="drop")
	{
	   string is_backup="true";
       if(RequestType=="GET")
	   {
          is_backup=WebUrl::CutParam(url,"is_backup");   
	   }
	   else{

		   try
		   {
			   if (document.HasMember("is_backup")&&document["is_backup"].IsString())
			   {
				   is_backup = document["is_backup"].GetString();
			   }
		   }
		   catch (...)
		   {
			   string error = "the parameter has some error,please look up the api document.";
			   sendResponseMsg(1003, error, response);
			   return;
		   }
	   }
	   if(is_backup.empty())
	   {
		   is_backup="true";
	   }
	   cout<<"is_backup:"<<is_backup<<endl;
	   drop_thread_new(response,db_name,is_backup);
	}
	//show all databases
	else if(operation=="show")
	{
        show_thread_new(response);
	}
	//manage the user list
	else if(operation=="usermanage")
	{
        string type="";
		string op_username="";
		string op_password="";
		if(RequestType=="GET")
		{
			type=WebUrl::CutParam(url,"type");
			op_username=WebUrl::CutParam(url,"op_username");
			op_username=UrlDecode(op_username);
			op_password=WebUrl::CutParam(url,"op_password");
			op_password=UrlDecode(op_password);
		}
		else 
		{
			try
			{
				if(document.HasMember("type")&&document["type"].IsString())
				{
					
                    type = document["type"].GetString();
				
				}
				if (document.HasMember("op_username")&&document["op_username"].IsString())
				{
					op_username = document["op_username"].GetString();
				}
				if (document.HasMember("op_password")&&document["op_password"].IsString())
				{
					op_password = document["op_password"].GetString();
				}
			}
			catch (...)
			{
				string error = "the parameter has some error,please look up the api document.";
				
				sendResponseMsg(1003, error, response);
				return;
			}
		}
		userManager_thread_new(response,op_username,op_password,type);
	}
	//show the user list
	else if(operation=="showuser")
	{
        showuser_thread_new(response);
	}
	//manage user privilege
	else if(operation=="userprivilegemanage")
	{
		string type="";
		string op_username="";
		string privileges="";
		if(RequestType=="GET")
		{
			type=WebUrl::CutParam(url,"type");
			op_username=WebUrl::CutParam(url,"op_username");
			privileges=WebUrl::CutParam(url,"privileges");
		}
		else 
		{
            type=document["type"].GetString();
			
			if (document.HasMember("op_username")&&document["op_username"].IsString())
			{
				op_username = document["op_username"].GetString();
			}
			if (document.HasMember("privileges")&&document["privileges"].IsString())
			{
				privileges = document["privileges"].GetString();
			}
		}
       userPrivilegeManage_thread_new(response,op_username,privileges,type,db_name);
	}
	//backup a database
	else if(operation=="backup")
	{
        string backup_path="";
		if(RequestType=="GET")
		{
			backup_path=WebUrl::CutParam(url,"backup_path");
			backup_path=UrlDecode(backup_path);
		}
		else 
		{
            
			if (document.HasMember("backup_path")&&document["backup_path"].IsString())
			{
				backup_path = document["backup_path"].GetString();
			}
			
		}
		backup_thread_new(response,db_name,backup_path);
	}
	//restore database
	else if(operation=="restore")
	{
		string backup_path="";
		if(RequestType=="GET")
		{
			backup_path=WebUrl::CutParam(url,"backup_path");
			backup_path=UrlDecode(backup_path);
		}
		else 
		{
           	if (document.HasMember("backup_path")&&document["backup_path"].IsString())
			{
				backup_path = document["backup_path"].GetString();
			}
		}
		restore_thread_new(response,db_name,backup_path,username);
		
	}
	//query database
	else if(operation=="query")
	{
        string format="";
		string sparql="";
	
		string querytype="0";
		if (checkPrivilege(username, "update", db_name) ==0)
		{
            querytype="0";
		}
		else
		{
			querytype="1";
		}
		if(RequestType=="GET")
		{
			format=WebUrl::CutParam(url,"format");
			sparql=WebUrl::CutParam(url,"sparql");
		
			sparql=UrlDecode(sparql);
		}
		else 
		{
       
			if (document.HasMember("format")&&document["format"].IsString())
			{
				format = document["format"].GetString();
			}
			if (document.HasMember("sparql")&&document["sparql"].IsString())
			{
				sparql = document["sparql"].GetString();
			}
			
		
		}
		if(format.empty())
		{
			format="json";
		}
		query_thread_new(response,db_name,sparql,format,querytype,remote_ip,thread_id,log_prefix);
	}
	else if(operation=="export")
	{
        string db_path="";
		if(RequestType=="GET")
		{
			db_path=WebUrl::CutParam(url,"db_path");
			db_path=UrlDecode(db_path);
		}
		else 
		{
            
			if (document.HasMember("db_path")&&document["db_path"].IsString())
			{
				db_path = document["db_path"].GetString();
			}
		}
		if(db_path.empty())
		{
			db_path=".";
		}
		export_thread_new(response,db_name,db_path,username);
	}
	else if(operation=="login")
	{
        login_thread_new(response);
	}
	else if(operation=="begin")
	{
         string isolevel="";
		if(RequestType=="GET")
		{
			isolevel=WebUrl::CutParam(url,"isolevel");
			isolevel=UrlDecode(isolevel);
		}
		else 
		{
            
			if (document.HasMember("isolevel")&&document["isolevel"].IsString())
			{
				isolevel = document["isolevel"].GetString();
			}
		}
		begin_thread_new(response,db_name,isolevel,username);
	}
	else if(operation=="tquery")
	{
       string TID="";
	   string sparql="";
		if(RequestType=="GET")
		{
			TID=WebUrl::CutParam(url,"tid");
			TID=UrlDecode(TID);
			sparql=WebUrl::CutParam(url,"sparql");
			sparql=UrlDecode(sparql);
		}
		else 
		{
           
			if (document.HasMember("tid")&&document["tid"].IsString())
			{
				TID = document["tid"].GetString();
			}
			if (document.HasMember("sparql")&&document["sparql"].IsString())
			{
				sparql = document["sparql"].GetString();
			}
		}
		tquery_thread_new(response,db_name,TID,sparql);

	}
	else if(operation=="commit")
	{
        string TID="";
	  
		if(RequestType=="GET")
		{
			TID=WebUrl::CutParam(url,"tid");
			TID=UrlDecode(TID);
			
		}
		else 
		{
           if (document.HasMember("tid")&&document["tid"].IsString())
			{
				TID = document["tid"].GetString();
			}
			
		}
		commit_thread_new(response,db_name,TID);
	}
	else if(operation=="rollback")
	{
		string TID="";
	  
		if(RequestType=="GET")
		{
			TID=WebUrl::CutParam(url,"tid");
			TID=UrlDecode(TID);
			
		}
		else 
		{
            if (document.HasMember("tid")&&document["tid"].IsString())
			{
				TID = document["tid"].GetString();
			}
			
		}
		rollback_thread_new(response,db_name,TID);
	}
	else if(operation=="txnlog")
	{
        txnlog_thread_new(response,username);
	}
	else if(operation=="checkpoint")
	{
         checkpoint_thread_new(response,db_name);

	}
	else if(operation=="testConnect")
	{
		test_connect_thread_new(response);
	}
	else if(operation=="getCoreVersion")
	{
		 getCoreVersion_thread_new(response);

	}
	
	else {
		string error="the operation "+operation +" has not match handler function";
		sendResponseMsg(1100, error, response);
	}
	
	
}
/**
 * @Author: liwenjie
 * @param {const} HttpServer
 * @param {const} shared_ptr
 * @param {const} shared_ptr
 * @param {string} RequestType
 */
bool request_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
	thread t(&request_thread, response, request, RequestType);
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
	if(logSize > MAX_QUERYLOG_size)
	{
		//close the old query log file
		fclose(query_logfp);
		//create and open a new query log file, then save the log name to name.log
		string querylog_name = Util::get_date_time();
		int index_space = querylog_name.find(' ');
		querylog_name = querylog_name.replace(index_space, 1, 1, '_');
		string namelog_name = QUERYLOG_PATH + NAMELOG_PATH;
		FILE *name_logfp = fopen(namelog_name.c_str(), "w");
		fprintf(name_logfp, "%s", querylog_name.c_str());

		queryLog = QUERYLOG_PATH + querylog_name + ".log";
		//queryLog = querylog_name;
		cout << "querLog: " << queryLog << endl;
		query_logfp = fopen(queryLog.c_str(), "a");
		if(query_logfp == NULL)
		{
			cerr << "open query log error"<<endl;
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


bool query_handler0_sparql_conform(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
    string thread_id = Util::getThreadID();
    string log_prefix = "thread " + thread_id + " -- ";
    cout << log_prefix << "HTTP: this is query_handler0_sparql_conform" << endl;
    cout << "request->path: " << request->path << endl;

    if (RequestType != "GET") {
        cout << log_prefix << "Implementation is currently limited to requests sent via GET." << endl;
        return false;
    }

    string db_name = request->path_match[1];
    db_name = UrlDecode(db_name);
    string db_query = request->path_match[2];
    db_query = UrlDecode(db_query);
    string format = "sparql-results+json";

    //check if the db_name is system
    if (db_name == "system") {
        string error = "no query privilege, operation failed.";
        string resJson = CreateJson(404, error, 0);
        *response << "HTTP/1.1 200 OK\r\nContent-Type: application/sparql-results+json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;
        return false;
    }

    query_num++;
    Task* task = new Task(0, db_name, format, db_query, response, request);
    pool.AddTask(task);
    return true;
}





//to stop the ghttp server




bool db_checkpoint(string db_name)
{
	pthread_rwlock_wrlock(&txn_m_lock);
	if (txn_managers.find(db_name) == txn_managers.end())
	{
		string error = "Database transaction manager error.";
		return false;
	}
	shared_ptr<Txn_manager> txn_m = txn_managers[db_name];
	txn_managers.erase(db_name);
	pthread_rwlock_unlock(&txn_m_lock);
	txn_m->abort_all_running();
	txn_m->Checkpoint();
	cout << "txn_m Checkpoint success!" << endl;
	return true;
}

bool checkall_thread(const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	if(!ipCheck(request)){
		cout<<"IP Blocked!"<<endl;
		string content="IP Blocked!";

		string resJson = CreateJson(916, content, 0);
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;
		return false;
	}
	string thread_id = Util::getThreadID();
	string log_prefix = "thread " + thread_id + " -- ";
	cout<<log_prefix<<"HTTP: this is checkall"<<endl;

	pthread_rwlock_rdlock(&databases_map_lock);
	cout << "databases_map_lock get" << endl;
	std::map<std::string, Database *>::iterator iter;
	string success;
	Util::abort_transactionlog(Util::get_cur_time());
	for(iter=databases.begin(); iter != databases.end(); iter++)
	{
		string database_name = iter->first;
		if (database_name == "system")
			continue;
		//abort all transaction
		db_checkpoint(database_name);
		Database *current_database = iter->second;
		pthread_rwlock_rdlock(&already_build_map_lock);
		std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(database_name);
		pthread_rwlock_unlock(&already_build_map_lock);
		if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0)
		{
			string error = "Unable to checkpoint due to loss of lock";
			string resJson = CreateJson(1007, error, 0);
			*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n" << resJson;

			//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
			pthread_rwlock_unlock(&databases_map_lock);
			return false;
		}
		current_database->save();
		delete current_database;
		current_database = NULL;
		cout<< "Database " << database_name << " saved successfully." <<endl;
		success = success + "Database "+ database_name +" saved successfully.\r\n";
		string resJson = CreateJson(0, success, 0);
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length()  << "\r\n\r\n" << resJson;

		//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
	}
	system_database->save();
	delete system_database;
	system_database = NULL;
	cout << "Database system saved successfully." << endl;
	pthread_rwlock_unlock(&databases_map_lock);

	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build;
	for (it_already_build = already_build.begin(); it_already_build != already_build.end(); it_already_build++)
	{
		struct DBInfo *temp_db = it_already_build->second;
		delete temp_db;
		temp_db = NULL;
	}
	pthread_rwlock_unlock(&already_build_map_lock);

	return true; 
}



bool addPrivilege(string username, string type, string db_name)
{
	if(username == ROOT_USERNAME)
	{
		return 1;
	}
	pthread_rwlock_rdlock(&users_map_lock);
	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(it != users.end() && db_name != "system")
	{
		pthread_rwlock_unlock(&users_map_lock);
		if(type == "query")
		{
			pthread_rwlock_wrlock(&(it->second->query_priv_set_lock));
			it->second->query_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_query_priv> <" + db_name + ">.}";
			updateSys(update);
				
			pthread_rwlock_unlock(&(it->second->query_priv_set_lock));

		}
		else if(type == "update")
		{
			pthread_rwlock_wrlock(&(it->second->update_priv_set_lock));
			it->second->update_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_update_priv> <" + db_name + ">.}";
			updateSys(update);
				
			pthread_rwlock_unlock(&(it->second->update_priv_set_lock));

		}
		else if(type == "load")
		{
			pthread_rwlock_wrlock(&(it->second->load_priv_set_lock));
			it->second->load_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_load_priv> <" + db_name + ">.}";
			updateSys(update);
			
			pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
		}
		else if(type == "unload")
		{
			pthread_rwlock_wrlock(&(it->second->unload_priv_set_lock));
			it->second->unload_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_unload_priv> <" + db_name + ">.}";
			updateSys(update);
			
			pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
		}
		else if(type == "restore")
		{
			pthread_rwlock_wrlock(&(it->second->restore_priv_set_lock));
			it->second->restore_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_restore_priv> <" + db_name + ">.}";
			updateSys(update);

			pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
		}
		else if(type == "backup")
		{
			pthread_rwlock_wrlock(&(it->second->backup_priv_set_lock));
			it->second->backup_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_backup_priv> <" + db_name + ">.}";
			updateSys(update);

			pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
		}
		else if(type == "export")
		{
			pthread_rwlock_wrlock(&(it->second->export_priv_set_lock));
			it->second->export_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_export_priv> <" + db_name + ">.}";
			updateSys(update);

			pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
		}
		return 1;
	}
	else
	{
		pthread_rwlock_unlock(&users_map_lock);
		return 0;
	}
}
bool delPrivilege(string username, string type, string db_name)
{
	pthread_rwlock_rdlock(&users_map_lock);
	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(it != users.end() && username != ROOT_USERNAME)
	{
		pthread_rwlock_unlock(&users_map_lock);
		if(type == "query" && it->second->query_priv.find(db_name) != it->second->query_priv.end())
		{
			pthread_rwlock_wrlock(&(it->second->query_priv_set_lock));
			it->second->query_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_query_priv> <" + db_name + ">.}";
			updateSys(update);
			
			pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
			return 1;
		}
		else if(type == "update" && it->second->update_priv.find(db_name) != it->second->update_priv.end())
		{
			pthread_rwlock_wrlock(&(it->second->update_priv_set_lock));
			it->second->update_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_update_priv> <" + db_name + ">.}";
			updateSys(update);
			
			pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
			return 1;
		}

		else if(type == "load" && it->second->load_priv.find(db_name) != it->second->load_priv.end())
		{
			pthread_rwlock_wrlock(&(it->second->load_priv_set_lock));
			it->second->load_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_load_priv> <" + db_name + ">.}";
			updateSys(update);
			
			pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
			return 1;
		}
		else if(type == "unload" && it->second->unload_priv.find(db_name) != it->second->unload_priv.end())
		{
			pthread_rwlock_wrlock(&(it->second->unload_priv_set_lock));
			it->second->unload_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_load_priv> <" + db_name + ">.}";
			updateSys(update);
			
			pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
			return 1;
		}
		else if(type == "backup" && it->second->backup_priv.find(db_name) != it->second->backup_priv.end())
		{
			pthread_rwlock_wrlock(&(it->second->backup_priv_set_lock));
			it->second->backup_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_backup_priv> <" + db_name + ">.}";
			updateSys(update);
			
			pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
			return 1;
		}
		else if(type == "restore" && it->second->restore_priv.find(db_name) != it->second->restore_priv.end())
		{
			pthread_rwlock_wrlock(&(it->second->restore_priv_set_lock));
			it->second->restore_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_restore_priv> <" + db_name + ">.}";
			updateSys(update);
			
			pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
			return 1;
		}
		else if(type == "export" && it->second->export_priv.find(db_name) != it->second->export_priv.end())
		{
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
	if(db_name == "system")
		return 0;

	if(username == ROOT_USERNAME)
		return 1;
	pthread_rwlock_rdlock(&users_map_lock);
	std::map<std::string, struct User *>::iterator it = users.find(username);
	//pthread_rwlock_unlock(&users_map_lock);
	if(type == "query")
	{
		pthread_rwlock_rdlock(&(it->second->query_priv_set_lock));
		if(it->second->query_priv.find(db_name) != it->second->query_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);	
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
	}
	else if(type == "update")
	{
		pthread_rwlock_rdlock(&(it->second->update_priv_set_lock));
		if(it->second->update_priv.find(db_name) != it->second->update_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);	
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
	}
	else if(type == "load")
	{
		pthread_rwlock_rdlock(&(it->second->load_priv_set_lock));
		if(it->second->load_priv.find(db_name) != it->second->load_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
	}
	else if(type == "unload")
	{
		pthread_rwlock_rdlock(&(it->second->unload_priv_set_lock));
		if(it->second->unload_priv.find(db_name) != it->second->unload_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
	}
	else if(type == "restore")
	{
		pthread_rwlock_rdlock(&(it->second->restore_priv_set_lock));
		if(it->second->restore_priv.find(db_name) != it->second->restore_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
	}
	else if(type == "backup")
	{
		pthread_rwlock_rdlock(&(it->second->backup_priv_set_lock));
		if(it->second->backup_priv.find(db_name) != it->second->backup_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
	}
	else if(type == "export")
	{
		pthread_rwlock_rdlock(&(it->second->export_priv_set_lock));
		if(it->second->export_priv.find(db_name) != it->second->export_priv.end())
		{
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
	if(body)
	{
		writer.Key("ResponseBody");
		writer.String(StringRef(ResponseBody.c_str()));
	}
	writer.EndObject();
	return s.GetString();

}
/*!
 * @brief		init the user map, alread_built map
 * @return 		Null.
*/
void DB2Map()
{	
	string sparql = "select ?x ?y where{?x <has_password> ?y.}";
	string strJson = querySys(sparql);
	//cout << "DDDDDDDDDDDDDDDB2Map: strJson : " << strJson << endl;
	Document document;
	document.Parse(strJson.c_str());
	Value &p1 = document["results"];
	Value &p2 = p1["bindings"]; 
	//int i = 0;
	for(int i = 0; i < p2.Size(); i++)
	{
		Value &pp = p2[i];
		Value &pp1 = pp["x"];
		Value &pp2 = pp["y"];
		string username = pp1["value"].GetString();
		string password = pp2["value"].GetString();
		//cout << "DDDDDDDDDDDDDDDDB2Map: username: " + username << " password: " << password << endl;
		struct User *user = new User(username, password);
			
		string sparql2 = "select ?x ?y where{<" + username + "> ?x ?y.}";
		string strJson2 = querySys(sparql2);
		//cout << "strJson2: " << strJson2 << endl;
		Document document2;
		document2.Parse(strJson2.c_str());

		Value &p12 = document2["results"];
		Value &p22 = p12["bindings"];
		for(int j = 0; j < p22.Size(); j++)
		{
			Value &ppj = p22[j];
			Value &pp12 = ppj["x"];
			Value &pp22 = ppj["y"];
			string type = pp12["value"].GetString();
			string db_name = pp22["value"].GetString();
			//cout << "DDDDDDDDDDDDDDDDDB2Map: type: " + type << " db_name: " << db_name << endl;
		
			if(type == "has_query_priv")
			{
				//cout << username << type << db_name << endl;
				user->query_priv.insert(db_name);
			}
			else if(type == "has_update_priv")
			{
				//cout << username << type << db_name << endl;
				user->update_priv.insert(db_name);
			}
			else if(type == "has_load_priv")
			{
				user->load_priv.insert(db_name);
			}
			else if(type == "has_unload_priv")
			{
				user->unload_priv.insert(db_name);
			}
			else if(type == "has_restore_priv")
			{
				user->restore_priv.insert(db_name);
			}else if(type == "has_backup_priv")
			{
				user->backup_priv.insert(db_name);
			}else if(type == "has_export_priv")
			{
				user->export_priv.insert(db_name);
			}
		}
		//users.insert(pair<std::string, struct User*>(username, &user));
		users.insert(pair<std::string, struct User *>(username, user));
	
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

	for(int i = 0; i < p2.Size(); i++)
	{
		Value &pp = p2[i];
		Value &pp1 = pp["x"];
		string db_name = pp1["value"].GetString();
		struct DBInfo *temp_db = new DBInfo(db_name);

		string sparql2 = "select ?x ?y where{<" + db_name + "> ?x ?y.}";
		string strJson2 = querySys(sparql2);
		Document document2;
		document2.Parse(strJson2.c_str());

		Value &p12 = document2["results"];
		Value &p22 = p12["bindings"];
	
		for(int j = 0; j < p22.Size(); j++)
		{
			Value &ppj = p22[j];
			Value &pp12 = ppj["x"];
			Value &pp22 = ppj["y"];
			string type = pp12["value"].GetString();
			string info = pp22["value"].GetString();
	
			if (type == "built_by")
				temp_db->setCreator(info);
			else if (type == "built_time")
				temp_db->setTime(info);
		}
		already_build.insert(pair<std::string, struct DBInfo *>(db_name, temp_db));		
	}	

	//add bulit_time of system.db to already_build map
	sparql = "select ?x where{<system> <built_time> ?x.}";
	strJson = querySys(sparql);
	document.Parse(strJson.c_str());
	p1 = document["results"];
	p2 = p1["bindings"]; 

	for (int i = 0; i < p2.Size(); i++)
	{
		Value &pp = p2[i];
		Value &pp1 = pp["x"];
		string built_time = pp1["value"].GetString();
		already_build.find("system")->second->setTime(built_time);
	}

	//get CoreVersion and APIVersion
	sparql = "select ?x where{<CoreVersion> <value> ?x.}";
	strJson = querySys(sparql);
	document.Parse(strJson.c_str());
	p1 = document["results"];
	p2 = p1["bindings"];
	for (int i = 0; i < p2.Size(); i++)
	{
		Value &pp = p2[i];
		Value &pp1 = pp["x"];
		CoreVersion = pp1["value"].GetString();
	}
	/*sparql = "select ?x where{<APIVersion> <value> ?x.}";
	strJson = querySys(sparql);
	document.Parse(strJson.c_str());
	p1 = document["results"];
	p2 = p1["bindings"];
	for (int i = 0; i < p2.Size(); i++)
	{
		Value &pp = p2[i];
		Value &pp1 = pp["x"];
		APIVersion = pp1["value"].GetString();
	}*/
}

string querySys(string sparql)
{
	string db_name = "system";
	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(db_name);
	pthread_rwlock_unlock(&already_build_map_lock);
	
	pthread_rwlock_rdlock(&(it_already_build->second->db_lock));
	ResultSet rs;
	FILE* output = NULL;

	int ret_val = system_database->query(sparql, rs, output);
	bool ret = false, update = false;
	if(ret_val < -1)   //non-update query
	{
		ret = (ret_val == -100);
	}
	else  //update query, -1 for error, non-negative for num of triples updated
	{
		update = true;
	}

	if(ret)
	{
		cout << "search query returned successfully." << endl;
		
		string success = rs.to_JSON();
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
		return success;
	}
	else
	{
		string error = "";
		int error_code;
		if(!update)
		{
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
		if (query.empty())
		{
			return 0;
		}
		printf("query is:\n%s\n\n", query.c_str());
		ResultSet _rs;
		FILE* ofp = stdout;
		
		string msg;
		int ret = system_database->query(query, _rs, ofp);
	//cout<<"gquery ret: "<<ret<<endl;
		if (ret <= -100)  //select query
		{
			if(ret == -100)
			{
				msg = _rs.to_str();
			}
			else //query error
			{
				msg = "query failed.";
			}

			return false;
		}
		else //update query
		{
			if(ret >= 0)
			{
				msg = "update num: " + Util::int2string(ret);
				cout << msg << endl;
				sysrefresh();
				//system_database->save();
				//delete system_database;
				//system_database=NULL;
				return true;
			}
			else //update error
			{
				msg = "update failed.";
				cout << msg << endl;
				return false;
			}
		}

}

bool initSys()
{
	//rebuild system.db

	system_database = new Database("system");
	bool flag = system_database->build(SYSTEM_PATH);
	if(flag)
	{
		cout << "import RDF file to database done." << endl;
		ofstream f;
		f.open("./system.db/success.txt");
		f.close();
	}
	else //if fails, drop system.db and return
	{
		cout << "import RDF file to database failed." << endl;
		string cmd = "rm -r system.db";
		system(cmd.c_str());
		delete system_database;
		system_database = NULL;
		return false;
	}
	delete system_database;
	system_database = new Database("system");
	system_database->load();
	cout << "system_database loaded" << endl;
	string cmd;
	cmd = "mv ./port.txt ./system.db/";
	system(cmd.c_str());
	cmd = "mv ./password"  + Util::int2string(port) + ".txt ./system.db/";
	system(cmd.c_str());
	return true;
}

/*
bool refreshSys()
{
	delete system_database;
	system_database = NULL;
	system_database = new Database("system");
	int flag = system_database->load();
	return flag;
}
*/

int copy(string src_path, string dest_path)
{
	string sys_cmd;
	if(Util::dir_exist(src_path)==false){
		//check the source path
		cout << "Source Path Error, Please Check It Again!" << endl;
		return 1;
	}
	if(Util::dir_exist(dest_path)==false){
		//check the destnation path 
		cout<<"the path ："<<dest_path<<" is not exist ,system will create it."<<endl;
		sys_cmd = "mkdir -p ./" + dest_path;
		system(sys_cmd.c_str());
	}
	
	sys_cmd = "cp -r " + src_path + ' ' +  dest_path ;
	system(sys_cmd.c_str());
    return 0;// success 
}





bool auto_backup_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string RequestType)
{
   return true;	

}

int db_reload(string db_name)
{
	pthread_rwlock_rdlock(&databases_map_lock);
	std::map<std::string, Database *>::iterator iter = databases.find(db_name);
	if(iter == databases.end())
	{
		cout << "Database not load yet." << endl;
		pthread_rwlock_unlock(&databases_map_lock);
		return -1;
	}
	pthread_rwlock_rdlock(&already_build_map_lock);
	std::map<std::string, struct DBInfo *>::iterator it_already_build = already_build.find(db_name);
	pthread_rwlock_unlock(&already_build_map_lock);
	if(pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0)
	{
		cout << "Unable to reload due to loss of lock";
		pthread_rwlock_unlock(&databases_map_lock);
		return -1;
	}
	db_checkpoint(db_name);
	Database *current_database = iter->second;
	delete current_database;
	cout << "Database unloaded." << endl;
	
	current_database = new Database(db_name);
	iter->second = current_database;
	bool flag = current_database->load();
	pthread_rwlock_wrlock(&txn_m_lock);
	if (txn_managers.find(db_name) == txn_managers.end())
	{
		string error = "Database transaction manager error.";
		return -1;
	}
	
	shared_ptr<Txn_manager> txn_m = make_shared<Txn_manager>(current_database, db_name);
	txn_managers[db_name] = txn_m; //new txn_manager
	pthread_rwlock_unlock(&txn_m_lock);
	if(!flag)
	{
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

txn_id_t get_txn_id(string db_name, string user)
{
	string idx = db_name + "_" + user;
	if (running_txn.find(idx) == running_txn.end()) {
		return 0;
	}
	else
		return running_txn[idx];
}

