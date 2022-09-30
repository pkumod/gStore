/*
 * @Author: liwenjie
 * @Date: 2021-09-23 16:55:53
 * @LastEditTime: 2022-09-29 22:29:15
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: In User Settings Edit
 * @FilePath: /gstore/Main/ghttp.cpp
 */

// SPARQL Endpoint:  log files in logs/endpoint/
// operation.log: not used
// query.log: query string, result num, and time of answering

// TODO: to add db_name to all URLs, and change the index.js using /show to get name, save and set
// TODO: use gzip for network transfer, it is ok to download a gzip file instead of the original one

#include "../Server/server_http.hpp"
#include "../Server/client_http.hpp"
// db
#include "../Database/Database.h"
#include "../Database/Txn_manager.h"

#include <iostream>
#include <fstream>

#include "../GRPC/APIUtil.h"
#include "../Util/INIParser.h"
#include "../Util/WebUrl.h"

using namespace rapidjson;
using namespace std;
// Added for the json-example:
using namespace boost::property_tree;

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

#define TEST_IP ""
#define HTTP_TYPE "ghttp"
APIUtil *apiUtil = nullptr;
//! init the ghttp server
int initialize(int argc, char *argv[]);

// Added for the default_resource example
void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
						   const shared_ptr<ifstream> &ifs);

//! Create a json object to return the request's result.
std::string CreateJson(int StatusCode, string StatusMsg, bool body, string ResponseBody = "response");
//! start a thread
pthread_t start_thread(void *(*_function)(void *));
//！stop a thread
bool stop_thread(pthread_t _thread);
void *func_timer(void *_args);
void thread_sigterm_handler(int _signal_num);
//=============================================================================

void request_handler(const HttpServer &server, const shared_ptr<HttpServer::Response> &response, const shared_ptr<HttpServer::Request> &request, string request_type);

void shutdown_handler(const HttpServer &server, const shared_ptr<HttpServer::Response> &response, const shared_ptr<HttpServer::Request> &request, string request_type);

void signalHandler(int signum);

//=============================================================================

string getRemoteIp(const shared_ptr<HttpServer::Request> &request);

void sendResponseMsg(int code, string msg, std::string operation, const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void sendResponseMsg(rapidjson::Document &doc, std::string operation, const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void build_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string db_path, string username, string password);

void load_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, bool load_csr);

void monitor_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name);

void unload_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name);

void drop_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string is_backup);

void show_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username);

void userManager_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, string password, string type);

void showuser_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void userPrivilegeManage_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, string privilege,
									string type, string db_name);

void userPassword_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, string password);

void backup_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string backup_path);

void restore_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string backup_path, string username);

void query_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string sparql, string format,
					  string update_flag, string log_prefix, string username);

void export_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string db_path, string username);

void login_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void check_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void begin_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string isolevel, string username);

void tquery_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string TID_s, string sparql);

void commit_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string TID_s);

void rollback_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string TID_s);

void txnlog_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, int page_no, int page_size);

void checkpoint_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name);

void test_connect_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void getCoreVersion_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void batchInsert_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string file);

void batchRemove_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string file);

void querylog_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string date, int page_no, int page_size);

void querylogdate_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void accesslog_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string date, int page_no, int page_size);

void accesslogdate_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void ipmanage_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string type, string ips, string ip_type);

void fun_query_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, struct PFNInfo &pfn_info);

void fun_cudb_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, struct PFNInfo &pfn_info, string type);

void fun_review_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, struct PFNInfo &pfn_info);

void build_PFNInfo(rapidjson::Value &fun_info, struct PFNInfo &pfn_info);
pthread_t scheduler = 0;

int query_num = 0;

// DEBUG+TODO: why the result transfered to client has no \n \t??
// using json is a good way to avoid this problem

// BETTER: How about change HttpConnector into a console?
// TODO: we need a route
// JSON parser: http://www.tuicool.com/articles/yUJb6f
//(or use boost spirit to generate parser when compiling)
//
// NOTICE: no need to close connection here due to the usage of shared_ptr
// http://www.tuicool.com/articles/3Ub2y2
//
// TODO: the URL format is terrible, i.e. 127.0.0.1:9000/build/lubm/data/LUBM_10.n3
// we should define some keys like operation, database, dataset, query, path ...
// 127.0.0.1:9000?operation=build&database=lubm&dataset=data/LUBM_10.n3
//
// TODO: control the authority, check it if requesting for build/load/unload
// for sparql endpoint, just load database when starting, and comment out all functions except for query()

// REFERENCE: C++ URL encoder and decoder
// http://blog.csdn.net/nanjunxiao/article/details/9974593

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved.
 * email：liwenjiehn@pku.edu.cn
 * @description: decode the url
 * @param {SRC} the encoded url
 * @return {string} the decode url
 */
string UrlDecode(string &SRC)
{
	string ret;
	char ch;
	int ii;
	for (size_t i = 0; i < SRC.length(); ++i)
	{
		if (int(SRC[i]) == 37)
		{
			sscanf(SRC.substr(i + 1, 2).c_str(), "%x", &ii);
			ch = static_cast<char>(ii);
			ret += ch;
			i = i + 2;
		}
		else if (SRC[i] == '+')
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

class Task
{
public:
	bool update;
	string username;
	string db_name;
	string format;
	string db_query;
	string remote_ip;
	string querytype;
	string log_prefix;
	const shared_ptr<HttpServer::Response> response;
	const shared_ptr<HttpServer::Request> request;
	Task(bool flag, string name, string ft, string query, const shared_ptr<HttpServer::Response> &res, const shared_ptr<HttpServer::Request> &req);
	Task(string username, string name, string ft, string _remote_ip, string _log_prefix, string _querytype,
		 string query, const shared_ptr<HttpServer::Response> &res, const shared_ptr<HttpServer::Request> &req);
	~Task();
	void run();
};
Task::Task(bool flag, string name, string ft,
		   string query, const shared_ptr<HttpServer::Response> &res,
		   const shared_ptr<HttpServer::Request> &req) : response(res), request(req)
{
	update = flag;
	db_name = name;
	format = ft;
	db_query = query;
}

Task::Task(string _username, string name, string ft, string _remote_ip, string _log_prefix, string _querytype,
		   string query, const shared_ptr<HttpServer::Response> &res,
		   const shared_ptr<HttpServer::Request> &req) : response(res), request(req)
{
	// update = flag;
	username = _username;
	db_name = name;
	format = ft;
	db_query = query;
	querytype = _querytype;
	remote_ip = _remote_ip;
	log_prefix = _log_prefix;
}
Task::~Task()
{
}
void Task::run()
{
	// query_thread(update, db_name, format, db_query, response, request);
	query_thread_new(request, response, db_name, db_query, format, querytype, log_prefix, username);
}

class Thread
{
public:
	thread TD;
	int ID;
	static int threadnum;
	Task *task;
	Thread();
	~Thread();
	int GetThreadID();
	void assign(Task *t);
	void run();
	void start();
	friend bool operator==(Thread t1, Thread t2);
	friend bool operator!=(Thread t1, Thread t2);
};

list<Thread *> busythreads;
vector<Thread *> freethreads;
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
void Thread::assign(Task *t)
{
	task = t;
}
void Thread::run()
{
	SLOG_DEBUG("Thread: " + to_string(ID) + " run...");
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
	queue<Task *> tasklines;
	ThreadPool();
	~ThreadPool();
	void InitThreadPool(int t);
	void create();
	void SetThreadNum(int t);
	int GetThreadNum();
	void AddTask(Task *t);
	void start();
	void close();
};
ThreadPool::ThreadPool()
{

}
ThreadPool::~ThreadPool()
{
	for (vector<Thread *>::iterator i = freethreads.begin(); i != freethreads.end(); i++)
		delete *i;
}
void ThreadPool::InitThreadPool(int t)
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
void ThreadPool::AddTask(Task *t)
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

ThreadPool pool;

int main(int argc, char *argv[])
{
	srand(time(NULL));
	apiUtil = new APIUtil();

	// Notice that current_database is assigned in the child process, not in the father process
	// when used as endpoint, or when the database is indicated in command line, we can assign
	// current_database in father process(but this is resource consuming)
	// TODO+DEBUG: when will the restarting error?
	while (true)
	{
		// NOTICE: here we use 2 processes, father process is used for monitor and control(like, restart)
		// Child process is used to deal with web requests, can also has many threads
		pid_t fpid = fork();

		if (fpid == 0)
		{
			int ret = initialize(argc, argv);
			std::cout.flush();
			_exit(ret);
		}
		else if (fpid > 0)
		{
			int status;
			waitpid(fpid, &status, 0);
			if (WIFEXITED(status))
			{
				return 0;
			}
			else
			{
				if (Util::file_exist("system.db/port.txt"))
				{
					string cmd = "rm system.db/port.txt";
					system(cmd.c_str());
				}
				SLOG_WARN("Stopped abnormally, restarting server...");
			}
		}
		else
		{
			SLOG_ERROR("Failed to start server: deamon fork failure.");
			return -1;
		}
	}
	return 0;
}

int initialize(int argc, char *argv[])
{
	// Server restarts to use the original database
	// current_database = NULL;
	HttpServer server;
	string db_name = "";
	string port_str = apiUtil->get_default_port();
	int port;
	bool loadCSR = 0; // DO NOT load CSR by default

	if (argc < 2)
	{
		SLOG_DEBUG("Use the default port:" + port_str + "!");
		SLOG_DEBUG("Not load any database!");
		port = Util::string2int(port_str);
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
			cout << "\t-db,--database[option],\t\tthe database name.Default value is empty. Notice that the name can not end with .db" << endl;
			cout << "\t-p,--port[option],\t\tthe listen port. Default value is 9000." << endl;
			cout << "\t-c,--csr[option],\t\tEnable CSR Struct or not. 0 denote that false, 1 denote that true. Default value is 0." << endl;

			cout << endl;
			return 0;
		}
		else
		{
			cout << "Invalid arguments! Input \"bin/ghttp -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		db_name = Util::getArgValue(argc, argv, "db", "database");
		if (db_name.length() > 3 && db_name.substr(db_name.length() - 3, 3) == ".db")
		{
			SLOG_ERROR("Your db name to be built should not end with \".db\".");
			return -1;
		}
		else if (db_name == "system")
		{
			SLOG_ERROR("You can not load system files.");
			return -1;
		}
		port_str = Util::getArgValue(argc, argv, "p", "port", port_str);
		port = Util::string2int(port_str);
		loadCSR = Util::string2int(Util::getArgValue(argc, argv, "c", "csr", "0"));
	}
	// check port.txt exist
	if (Util::file_exist("system.db/port.txt"))
	{
		SLOG_ERROR("Server port " + port_str + " is already in use.");
		return -1;
	}
	// call apiUtil initialized
	if (apiUtil->initialize(HTTP_TYPE, port_str, db_name, loadCSR) == -1)
	{
		return -1;
	}

	server.config.port = port;
	server.config.thread_pool_size = apiUtil->get_thread_pool_num();
	
	pool.InitThreadPool(apiUtil->get_thread_pool_num());
	pool.create();
	// Default GET-example. If no other matches, this anonymous function will be called.
	// Will respond with content in the web/-directory, and its subdirectories.
	// Default file: index.html
	// Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server

	server.resource["/shutdown"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		shutdown_handler(server, response, request, "GET");
	};

	// POST-example for the path /build, responds with the matched string in path
	server.resource["/shutdown"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		shutdown_handler(server, response, request, "POST");
	};

	server.default_resource["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		// default_handler(server, response, request);
		request_handler(server, response, request, "GET");
	};

	server.default_resource["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		request_handler(server, response, request, "POST");
	};
	thread server_thread([&server]()
						 {
		//handle the Ctrl+C signal
		signal(SIGINT, signalHandler);
		//Start server
		SLOG_DEBUG("ghttp server port " + to_string(server.config.port));
		server.start(); });

	// Wait for server to start so that the client can connect
	this_thread::sleep_for(std::chrono::seconds(1));

	server_thread.join();
	SLOG_DEBUG("ghttp server stoped.");
	return 0;
}

void signalHandler(int signum)
{
	if (apiUtil)
	{
		delete apiUtil;
	}
	SLOG_DEBUG("ghttp server stopped.");
	std::cout.flush();
	_exit(signum);
}

// QUERY: can server.send() in default_resource_send run in parallism?

// TCP is slower than UDP, but more safely, and can support large data transfer well
// http://blog.csdn.net/javaniuniu/article/details/51419348
void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
						   const shared_ptr<ifstream> &ifs)
{

	// read and send 128 KB at a time
	static vector<char> buffer(131072); // Safe when server is running on one thread
	streamsize read_length;
	if ((read_length = ifs->read(&buffer[0], buffer.size()).gcount()) > 0)
	{
		response->write(&buffer[0], read_length);
		if (read_length == static_cast<streamsize>(buffer.size()))
		{
			server.send(response, [&server, response, ifs](const boost::system::error_code &ec)
						{
                    if(!ec)
                    default_resource_send(server, response, ifs);
                    else
                    cerr << "Connection interrupted" << endl; });
		}
	}
}

pthread_t start_thread(void *(*_function)(void *))
{
	pthread_t thread;
	if (pthread_create(&thread, NULL, _function, NULL) == 0)
	{
		return thread;
	}
	return 0;
}

bool stop_thread(pthread_t _thread)
{
	return pthread_kill(_thread, SIGTERM) == 0;
}

// DEBUG+TODO: the whole process exits and need to reload the database
//(too slow: quit and restart)
void *func_timer(void *_args)
{
	signal(SIGTERM, thread_sigterm_handler);
	sleep(Util::gserver_query_timeout);
	cerr << "Query out of time." << endl;
	// TODO: not use SIGTERM and abort() here, which will cause process to quit abnormally
	// here shoudl just end the timer thread
	abort();
}

void thread_sigterm_handler(int _signal_num)
{
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
void build_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string db_path, string username, string password)
{
	string operation = "build";
	try
	{
		string result = apiUtil->check_param_value("db_path", db_path);
		if (result.empty() == false)
		{
			sendResponseMsg(1003, result, operation, request, response);
			return;
		}
		if (db_path == apiUtil->get_system_path())
		{
			string error = "You have no rights to access system files.";
			sendResponseMsg(1002, error, operation, request, response);
			return;
		}
		if (Util::file_exist(db_path) == false)
		{
			string error = "RDF file not exist.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		result = apiUtil->check_param_value("db_name", db_name);
		if (result.empty() == false)
		{
			sendResponseMsg(1003, result, operation, request, response);
			return;
		}

		// check if database named [db_name] is already built
		if (apiUtil->check_db_exist(db_name))
		{
			string error = "database already built.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		// check databse number
		if (apiUtil->check_db_count() == false)
		{
			string error = "The total number of databases more than max_databse_num.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		
		string dataset = db_path;
		string database = db_name;
		SLOG_DEBUG("Import dataset to build database...");
		SLOG_DEBUG("DB_store: " + database + "\tRDF_data: " + dataset);
		// int len = database.length();
		Database *current_database = new Database(database);
		bool flag = current_database->build(dataset);
		delete current_database;
		current_database = NULL;
		if (!flag)
		{
			string error = "Import RDF file to database failed.";
			string cmd = "rm -r " + database + ".db";
			system(cmd.c_str());
			sendResponseMsg(1005, error, operation, request, response);
			return;
		}

		ofstream f;
		f.open("./" + database + ".db/success.txt");
		f.close();

		// by default, one can query or load or unload the database that is built by itself, so add the database name to the privilege set of the user
		if (apiUtil->add_privilege(username, "query", db_name) == 0 ||
			apiUtil->add_privilege(username, "load", db_name) == 0 ||
			apiUtil->add_privilege(username, "unload", db_name) == 0 ||
			apiUtil->add_privilege(username, "backup", db_name) == 0 ||
			apiUtil->add_privilege(username, "restore", db_name) == 0 ||
			apiUtil->add_privilege(username, "export", db_name) == 0)
		{
			string error = "add query or load or unload privilege failed.";
			sendResponseMsg(1006, error, operation, request, response);
			return;
		}
		SLOG_DEBUG("add query and load and unload privilege succeed after build.");

		// add database information to system.db
		if (apiUtil->build_db_user_privilege(db_name, username))
		{
			string success = "Import RDF file to database done.";
			sendResponseMsg(0, success, operation, request, response);
			Util::add_backuplog(db_name);
		}
		else
		{
			string error = "add database information to system failed.";
			sendResponseMsg(1006, error, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Build fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @description: response message to client
 * @Author: liwenjie
 * @param {int} code: StatusCode value
 * @param {string} msg:StatusMsg value
 * @param {const} shared_ptr
 */
void sendResponseMsg(int code, string msg, std::string operation, const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
{
	string resJson = CreateJson(code, msg, 0);
	if (code == 0)
	{
		SLOG_DEBUG("response result:\n" + resJson);
	}
	else
	{
		SLOG_ERROR("response result:\n" + resJson);
	}
	string remote_ip = getRemoteIp(request);
	apiUtil->write_access_log(operation, remote_ip, code, msg);
	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n"
			  << resJson;
}

void sendResponseMsg(rapidjson::Document &doc, std::string operation, const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
{
	int code;
	string msg;
	if (doc.HasMember("StatusCode") && doc["StatusCode"].IsInt())
	{
		code = doc["StatusCode"].GetInt();
	}
	else
	{
		code = 1005;
	}
	if (doc.HasMember("StatusMsg") && doc["StatusMsg"].IsString())
	{
		msg = doc["StatusMsg"].GetString();
	}
	else
	{
		msg = "";
	}
	string remote_ip = getRemoteIp(request);
	apiUtil->write_access_log(operation, remote_ip, code, msg);
	StringBuffer resBuffer;
	rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
	doc.Accept(resWriter);
	string json_str = resBuffer.GetString();

	SLOG_DEBUG("response result:\n" + json_str);
	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << json_str.length() << "\r\n\r\n"
			  << json_str;
}

string getRemoteIp(const shared_ptr<HttpServer::Request> &request)
{
	string remote_ip;
	// get the real IP of the client, because we use nginx here
	unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals> m = request->header;
	string map_key = "X-Real-IP";
	// for (auto it = m.begin(); it != m.end(); it ++){
	pair<std::unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals>::iterator, std::unordered_multimap<std::string, std::string, case_insensitive_hash, case_insensitive_equals>::iterator> lu = m.equal_range(map_key);
	if (lu.first != lu.second)
		remote_ip = lu.first->second;
	else
		remote_ip = request->remote_endpoint_address;
	// SLOG_DEBUG("remote_ip: " + remote_ip);
	return remote_ip;
}

/**
 * @description: load the database to memory.
 * @Author: liwenjie
 * @param {const} shared_ptr
 * @param {string} db_name
 */
void load_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, bool load_csr)
{
	string operation = "load";
	try
	{
		string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (!apiUtil->check_already_build(db_name))
		{
			error = "the database [" + db_name + "] not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}

		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			if (!apiUtil->trywrlock_database(db_name))
			{
				error = "Unable to load due to loss of lock.";
				sendResponseMsg(1005, error, operation, request, response);
				return;
			}

			Database *current_database = new Database(db_name);
			SLOG_DEBUG("begin loading...");
			bool rt = current_database->load(load_csr);
			SLOG_DEBUG("end loading...");
			if (rt)
			{
				apiUtil->add_database(db_name, current_database);
				// todo insert txn
				if (apiUtil->insert_txn_managers(current_database, db_name) == false)
				{
					SLOG_WARN("when load insert_txn_managers fail.");
				}
				string csr_str = "0";
				if (current_database->csr != NULL)
				{
					csr_str = "1";
				}
				rapidjson::Document doc;
				doc.SetObject();
				Document::AllocatorType &allocator = doc.GetAllocator();
				doc.AddMember("StatusCode", 0, allocator);
				doc.AddMember("StatusMsg", "database loaded successfully.", allocator);
				doc.AddMember("csr", StringRef(csr_str.c_str()), allocator);
				sendResponseMsg(doc, operation, request, response);
			}
			else
			{
				error = "load failed: unknow error.";
				sendResponseMsg(1005, error, operation, request, response);
			}
			rt = apiUtil->unlock_database(db_name);
		}
		else
		{
			string csr_str = "0";
			if (current_database->csr != NULL)
			{
				csr_str = "1";
			}
			rapidjson::Document doc;
			doc.SetObject();
			Document::AllocatorType &allocator = doc.GetAllocator();
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "the database already load yet.", allocator);
			doc.AddMember("csr", StringRef(csr_str.c_str()), allocator);
			sendResponseMsg(doc, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "load fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @description: get the database monitor info
 * @Author:liwenjie
 * @param {const} shared_ptr response point
 * @param {string} db_name
 * @return {*}
 */
void monitor_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name)
{
	string operation = "monitor";
	try
	{
		// check the param value is legal or not.
		string error = apiUtil->check_param_value("db_name", db_name);

		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		DatabaseInfo *database_info = apiUtil->get_databaseinfo(db_name);
		if (apiUtil->tryrdlock_databaseinfo(database_info) == false)
		{
			string error = "Unable to monitor due to loss of lock";
			sendResponseMsg(1007, error, operation, request, response);
			return;
		}
		string creator = database_info->getCreator();
		string time = database_info->getTime();
		apiUtil->unlock_databaseinfo(database_info);
		// /use JSON format to send message
		rapidjson::Document doc;
		doc.SetObject();
		Document::AllocatorType &allocator = doc.GetAllocator();
		doc.AddMember("StatusCode", 0, allocator);
		doc.AddMember("StatusMsg", "success", allocator);
		doc.AddMember("database", StringRef(db_name.c_str()), allocator);
		doc.AddMember("creator", StringRef(creator.c_str()), allocator);
		doc.AddMember("builtTime", StringRef(time.c_str()), allocator);
		char tripleNumString[128];
		sprintf(tripleNumString, "%lld", current_database->getTripleNum());
		doc.AddMember("tripleNum", StringRef(tripleNumString), allocator);
		doc.AddMember("entityNum", current_database->getEntityNum(), allocator);
		doc.AddMember("literalNum", current_database->getLiteralNum(), allocator);
		doc.AddMember("subjectNum", current_database->getSubNum(), allocator);
		doc.AddMember("predicateNum", current_database->getPreNum(), allocator);
		doc.AddMember("connectionNum", apiUtil->get_connection_num(), allocator);

		sendResponseMsg(doc, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "Monitor fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @description: unload a database from memory
 * @Author:liwenjie
 * @param {const} shared_ptr response point
 * @param {string} db_name the database name
 * @return {*}
 */
void unload_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name)
{
	string operation = "unload";
	try
	{
		string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "the database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		struct DatabaseInfo *db_info = apiUtil->get_databaseinfo(db_name);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			sendResponseMsg(1007, error, operation, request, response);
			return;
		}
		else
		{
			if (apiUtil->find_txn_managers(db_name) == false)
			{
				error = "transaction manager can not find the database";

				apiUtil->unlock_database_map();
				apiUtil->unlock_databaseinfo(db_info);
				sendResponseMsg(1008, error, operation, request, response);
				return;
			}
			apiUtil->db_checkpoint(db_name);
			apiUtil->delete_from_databases(db_name);
			apiUtil->unlock_databaseinfo(db_info);

			string success = "Database unloaded.";
			sendResponseMsg(0, success, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Unload fail" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
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
void drop_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string is_backup)
{
	string ip = getRemoteIp(request);
	string operation = "drop";
	try
	{
		string error = apiUtil->check_param_value("db_name", db_name);

		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		struct DatabaseInfo *db_info = apiUtil->get_databaseinfo(db_name);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			sendResponseMsg(1007, error, operation, request, response);
		}
		else
		{
			if (apiUtil->check_already_load(db_name))
			{
				//@ the database has loaded, unload it firstly
				apiUtil->delete_from_databases(db_name);
				SLOG_DEBUG("remove " + db_name + " from loaded database list");
			}
			apiUtil->unlock_databaseinfo(db_info);
			//@ delete the database info from the system database
			bool rt = apiUtil->delete_from_already_build(db_name);
			if (!rt)
			{
				SLOG_DEBUG("remove " + db_name + " from the already build database list fail.");
				error = "the operation can not been excuted due to loss of lock.";
				sendResponseMsg(1007, error, operation, request, response);
				return;
			}
			SLOG_DEBUG("remove " + db_name + " from the already build database list success.");
			string cmd;

			if (is_backup == "false")
				cmd = "rm -r " + db_name + ".db";
			else
				cmd = "mv " + db_name + ".db " + db_name + ".bak";
			SLOG_DEBUG("delete the file: " + cmd);
			system(cmd.c_str());
			Util::delete_backuplog(db_name);
			string success = "Database " + db_name + " dropped.";
			sendResponseMsg(0, success, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Drop fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @description: show the all database list (except system database)
 * @Author:liwenjie
 * @param {const shared_ptr<HttpServer::Response>} &response
 * @return {*}
 */
void show_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username)
{
	string ip = getRemoteIp(request);
	string operation = "show";
	try
	{

		stringstream str_stream;
		str_stream << "[";

		vector<struct DatabaseInfo *> array;
		apiUtil->get_already_builds(username, array);
		rapidjson::Document resDoc;
		resDoc.SetObject();
		Document::AllocatorType &allocator = resDoc.GetAllocator();
		size_t count = array.size();
		string line;
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			DatabaseInfo *dbInfo = array[i];
			line = dbInfo->toJSON();
			str_stream << line;
		}
		str_stream << "]";
		Document jsonArray;
		jsonArray.SetArray();
		line = str_stream.str();
		jsonArray.Parse(line.c_str());

		resDoc.AddMember("StatusCode", 0, allocator);
		resDoc.AddMember("StatusMsg", "Get the database list successfully!", allocator);
		resDoc.AddMember("ResponseBody", jsonArray, allocator);
		sendResponseMsg(resDoc, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "Show fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
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
void userManager_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, string password, string type)
{
	string error = "";
	string operation = "usermanage";
	try
	{
		error = apiUtil->check_param_value("op_username", username);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (type != "2")
		{
			error = apiUtil->check_param_value("op_password", password);
			if (error.empty() == false)
			{
				sendResponseMsg(1003, error, operation, request, response);
				return;
			}
		}
		if (type == "1") // add user
		{
			// check user number
			if (apiUtil->check_user_count() == false)
			{
				string error = "The total number of users more than max_user_num.";
				sendResponseMsg(1004, error, operation, request, response);
				return;
			}
			if (apiUtil->user_add(username, password))
			{
				sendResponseMsg(0, "User add done.", operation, request, response);
			}
			else
			{
				error = "Username already existed, add user failed.";
				sendResponseMsg(1004, error, operation, request, response);
			}
		}
		else if (type == "2") // delete user
		{
			if (username == apiUtil->get_root_username())
			{
				error = "You cannot delete root, delete user failed.";
				sendResponseMsg(1004, error, operation, request, response);
			}
			else if (apiUtil->user_delete(username))
			{
				sendResponseMsg(0, "Delete user done.", operation, request, response);
			}
			else
			{
				error = "Username not exist, delete user failed.";
				sendResponseMsg(1004, error, operation, request, response);
			}
		}
		else if (type == "3") // alert password
		{
			if (apiUtil->user_pwd_alert(username, password))
			{
				sendResponseMsg(0, "Change password done.", operation, request, response);
			}
			else
			{
				error = "Username not exist, change password failed.";
				sendResponseMsg(1004, error, operation, request, response);
			}
		}
		else
		{
			error = "The operation is not support.";
			sendResponseMsg(1003, error, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "User manager fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @description: show all user list.
 * @Author:liwenjie
 * @param {const} shared_ptr
 * @return {*}
 */
void showuser_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
{
	string ip = getRemoteIp(request);
	string operation = "showuser";
	try
	{
		vector<struct DBUserInfo *> userList;
		apiUtil->get_user_info(&userList);
		if (userList.empty())
		{
			sendResponseMsg(0, "No Users", operation, request, response);
			return;
		}
		size_t count = userList.size();
		stringstream str_stream;
		str_stream << "[";
		string line;
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			line = userList[i]->toJSON();
			// cout<< "user[" << i << "]: " << line << endl;
			str_stream << line;
		}
		str_stream << "]";
		Document jsonArray;
		jsonArray.SetArray();
		line = str_stream.str();
		jsonArray.Parse(line.c_str());

		rapidjson::Document resDoc;
		rapidjson::Document::AllocatorType &allocator = resDoc.GetAllocator();
		resDoc.SetObject();
		resDoc.AddMember("StatusCode", 0, allocator);
		resDoc.AddMember("StatusMsg", "success", allocator);
		resDoc.AddMember("ResponseBody", jsonArray, allocator);
		sendResponseMsg(resDoc, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "Show user fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
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
void userPrivilegeManage_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username,
									string privilege, string type, string db_name)
{
	string error = "";
	string operation = "userprivilegemanage";
	try
	{
		error = apiUtil->check_param_value("type", type);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		else if (type != "1" && type != "2" && type != "3")
		{
			error = "The type " + type + " is not support.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		error = apiUtil->check_param_value("op_username", username);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		else if (apiUtil->check_user_exist(username) == false)
		{
			error = "The username is not exists.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		else if (username == apiUtil->get_root_username())
		{
			string error = "You can't change privileges for root user.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		if (type != "3")
		{
			error = apiUtil->check_param_value("db_name", db_name);
			if (error.empty() == false)
			{
				sendResponseMsg(1003, error, operation, request, response);
				return;
			}
			// check database exist
			if (apiUtil->check_db_exist(db_name) == false)
			{
				error = "Database not build yet.";
				sendResponseMsg(1004, error, operation, request, response);
				return;
			}
			error = apiUtil->check_param_value("privileges", privilege);
			if (error.empty() == false)
			{
				sendResponseMsg(1003, error, operation, request, response);
				return;
			}
		}
		string result = "";
		if (type == "3")
		{
			// clear the user all privileges
			int resultint = apiUtil->clear_user_privilege(username);
			if (resultint == 1)
			{
				result = "Clear the all privileges for the user successfully!";
				sendResponseMsg(0, result, operation, request, response);
			}
			else
			{
				error = "Clear the all privileges for the user fail.";
				sendResponseMsg(1004, error, operation, request, response);
			}
		}
		else
		{
			vector<string> privileges;
			if (privilege.substr(privilege.length() - 1, 1) != ",")
			{
				privilege = privilege + ",";
			}

			Util::split(privilege, ",", privileges);
			for (unsigned i = 0; i < privileges.size(); i++)
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
						if (apiUtil->add_privilege(username, temp_privilege, db_name) == 0)
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
						if (apiUtil->del_privilege(username, temp_privilege, db_name) == 0)
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
						sendResponseMsg(1003, result, operation, request, response);
						return;
					}
				}
			}
			sendResponseMsg(0, result, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "User privilege manage fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @description change user password
 *
 * @param request
 * @param response
 * @param username
 * @param password
 */
void userPassword_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, string password)
{
	string error = "";
	string operation = "userpassword";
	try
	{
		std::string error = apiUtil->check_param_value("op_password", password);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->user_pwd_alert(username, password))
		{
			sendResponseMsg(0, "Change password done. ", operation, request, response);
		}
		else
		{
			error = "Username not exist.";
			sendResponseMsg(1004, error, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Alert password fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @description: backup a database
 * @Author:liwenjie
 * @param {const} shared_ptr
 * @param {string} db_name:the operation database name
 * @param {string} backup_path: the backup path
 * @return {*}
 */
void backup_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string backup_path)
{
	string operation = "backup";
	try
	{
		string error = "";
		error = apiUtil->check_param_value("db_name", db_name);

		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "the database [" + db_name + "] not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		struct DatabaseInfo *db_info = apiUtil->get_databaseinfo(db_name);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			sendResponseMsg(1007, error, operation, request, response);
			return;
		}
		// begin backup database
		string path = backup_path;
		if (path.empty())
		{
			path = apiUtil->get_backup_path();
			SLOG_DEBUG("backup_path is empty, set to default path: " + path);
		}
		if (path == ".")
		{
			string error = "Failed to backup the database. Backup Path Can not be root or empty.";
			apiUtil->unlock_databaseinfo(db_info);
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (path[path.length() - 1] == '/')
		{
			path = path.substr(0, path.length() - 1);
		}
		SLOG_DEBUG("backup path: " + path);
		string db_path = db_name + ".db";
		apiUtil->rw_wrlock_database_map();
		int ret = apiUtil->db_copy(db_path, path);
		apiUtil->unlock_database_map();
		string timestamp = "";
		if (ret == 1)
		{
			string error = "Failed to backup the database. Database Folder Misssing.";
			apiUtil->unlock_databaseinfo(db_info);
			sendResponseMsg(1005, error, operation, request, response);
		}
		else
		{
			timestamp = Util::get_timestamp();
			path = path + "/" + db_path;
			string _path = path + "_" + timestamp;
			string sys_cmd = "mv " + path + " " + _path;
			system(sys_cmd.c_str());

			SLOG_DEBUG("database backup done: " + db_name);
			string success = "Database backup successfully.";
			apiUtil->unlock_databaseinfo(db_info);

			Document resDoc;
			resDoc.SetObject();
			Document::AllocatorType &allocator = resDoc.GetAllocator();
			resDoc.AddMember("StatusCode", 0, allocator);
			resDoc.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
			resDoc.AddMember("backupfilepath", StringRef(_path.c_str()), allocator);
			sendResponseMsg(resDoc, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Backup fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
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
void restore_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string backup_path, string username)
{
	string error = "";
	string operation = "restore";
	try
	{
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		string path = backup_path;
		if (path[path.length() - 1] == '/')
		{
			path = path.substr(0, path.length() - 1);
		}
		SLOG_DEBUG("backup path:" + path);
		if (Util::dir_exist(path) == false)
		{
			string error = "Backup path not exist, restore failed.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		string database = db_name;
		SLOG_DEBUG("restore database:" + database);
		if (apiUtil->check_already_build(db_name) == false)
		{
			error = "Database not built yet. Rebuild Now";
			string time = Util::get_backup_time(path, db_name);
			if (time.size() == 0)
			{
				string error = "Backup path does not match database name, restore failed";
				sendResponseMsg(1003, error, operation, request, response);
				return;
			}
			if (apiUtil->add_privilege(username, "query", db_name) == 0 ||
				apiUtil->add_privilege(username, "load", db_name) == 0 ||
				apiUtil->add_privilege(username, "unload", db_name) == 0 ||
				apiUtil->add_privilege(username, "backup", db_name) == 0 ||
				apiUtil->add_privilege(username, "restore", db_name) == 0 ||
				apiUtil->add_privilege(username, "export", db_name) == 0)
			{
				error = "add query or load or unload or backup or restore or export privilege failed.";
				sendResponseMsg(1006, error, operation, request, response);
				return;
			}
			if (apiUtil->build_db_user_privilege(db_name, username))
			{
				Util::add_backuplog(db_name);
			}
			else
			{
				error = "Database not built yet. Rebuild failed.";
				sendResponseMsg(1005, error, operation, request, response);
				return;
			}
		}
		struct DatabaseInfo *db_info = apiUtil->get_databaseinfo(db_name);

		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			string error = "Unable to restore due to loss of lock";
			sendResponseMsg(1007, error, operation, request, response);
			return;
		}

		// restore
		string sys_cmd = "rm -rf " + db_name + ".db";
		std::system(sys_cmd.c_str());

		// TODO why need lock the database_map?
		// apiUtil->trywrlock_database_map();
		int ret = apiUtil->db_copy(path, apiUtil->get_Db_path());
		// apiUtil->unlock_database_map();

		if (ret == 1)
		{
			string error = "Failed to restore the database. Backup path error";
			apiUtil->unlock_databaseinfo(db_info);
			sendResponseMsg(1005, error, operation, request, response);
		}
		else
		{
			// TODO update the in system.db
			path = Util::get_folder_name(path, db_name);
			sys_cmd = "mv " + path + " " + db_name + ".db";
			std::system(sys_cmd.c_str());
			apiUtil->unlock_databaseinfo(db_info);
			string success = "Database " + db_name + " restore successfully.";
			sendResponseMsg(0, success, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Restore fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
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
void query_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response,
					  string db_name, string sparql, string format,
					  string update_flag, string log_prefix, string username)
{
	string error = "";
	string operation = "query";
	try
	{
		// check db_name paramter
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		// check sparql paramter
		error = apiUtil->check_param_value("sparql", sparql);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		string thread_id = Util::getThreadID();
		Database *current_database;
		bool update_flag_bool = true;
		if (update_flag == "0")
		{
			update_flag_bool = false;
		}
		try
		{
			// check database exist
			if (apiUtil->check_db_exist(db_name) == false)
			{
				throw runtime_error("Database not build yet.");
			}
			// check database load status
			current_database = apiUtil->get_database(db_name);
			if (current_database == NULL)
			{
				throw runtime_error("Database not load yet.");
			}
			bool lock_rt = apiUtil->rdlock_database(db_name);
			if (lock_rt)
			{
				SLOG_DEBUG("get current database read lock success: " + db_name);
			}
			else
			{
				throw runtime_error("get current database read lock fail.");
			}
		}
		catch (const std::exception &e)
		{
			error = string(e.what());
			sendResponseMsg(1005, error, operation, request, response);
			return;
		}

		FILE *output = NULL;

		ResultSet rs;
		int ret_val;
		int query_time = Util::get_cur_time();

		// set query_start_time
		std::string query_start_time;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		int s = tv.tv_usec / 1000;
		int y = tv.tv_usec % 1000;
		query_start_time = Util::get_date_time() + ":" + Util::int2string(s) + "ms" + ":" + Util::int2string(y) + "microseconds";
		try
		{
			SLOG_DEBUG("begin query...");
			rs.setUsername(username);
			ret_val = current_database->query(sparql, rs, output, update_flag_bool, false, nullptr);
			query_time = Util::get_cur_time() - query_time;
		}
		catch (string exception_msg)
		{
			string content = exception_msg;
			apiUtil->unlock_database(db_name);
			sendResponseMsg(1005, content, operation, request, response);
			return;
		}
		catch (const std::runtime_error &e2)
		{
			string content = e2.what();
			apiUtil->unlock_database(db_name);
			sendResponseMsg(1005, content, operation, request, response);
			return;
		}
		catch (...)
		{
			string content = "unknow error";
			apiUtil->unlock_database(db_name);
			sendResponseMsg(1005, content, operation, request, response);
			return;
		}

		bool ret = false, update = false;
		if (ret_val < -1) // non-update query
		{
			ret = (ret_val == -100);
		}
		else // update query, -1 for error, non-negative for num of triples updated
		{
			update = true;
		}

		string filename = thread_id + "_" + Util::getTimeString2() + "_" + Util::int2string(Util::getRandNum()) + ".txt";
		string localname = apiUtil->get_query_result_path() + filename;
		if (ret)
		{
			// SLOG_DEBUG(thread_id + ":search query returned successfully.");

			// record each query operation, including the sparql and the answer number
			// accurate down to microseconds
			// filter the IP from the test server
			string remote_ip = getRemoteIp(request);
			long rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
			long rs_outputlimit = (long)rs.output_limit;
			if (rs_outputlimit != -1)
				rs_ansNum = min(rs_ansNum, rs_outputlimit);
			if (remote_ip != TEST_IP)
			{
				int status_code = 0;
				string file_name = "";
				if (format.find("file") != string::npos)
				{
					file_name = string(filename.c_str());
				}
				struct DBQueryLogInfo queryLogInfo(query_start_time, remote_ip, sparql, rs_ansNum, format, file_name, status_code, query_time, db_name);
				apiUtil->write_query_log(&queryLogInfo);
			}

			// to void someone downloading all the data file by sparql query on purpose and to protect the data
			// if the ansNum too large, for example, larger than 100000, we limit the return ans.
			if (rs_ansNum > apiUtil->get_max_output_size())
			{
				if (rs_outputlimit == -1 || rs_outputlimit > apiUtil->get_max_output_size())
				{
					rs_outputlimit = apiUtil->get_max_output_size();
				}
			}
			string query_time_s = Util::int2string(query_time);

			ofstream outfile;
			string ans = "";
			string success = rs.to_JSON();
			// TODO: if result is stored in Stream instead of memory?  (if out of memory to use to_str)
			// BETTER: divide and transfer, in multiple times, getNext()
			if (format == "json")
			{
				Document resDoc;
				Document::AllocatorType &allocator = resDoc.GetAllocator();
				/* code */
				resDoc.Parse(success.c_str());
				if (resDoc.HasParseError())
				{
					SLOG_ERROR("result parse error:\n" + success);
					error = "parse error";
					string filename2 = "error_" + filename;
					string localname2 = apiUtil->get_query_result_path() + filename2;
					outfile.open(localname2);
					outfile << success;
					outfile.close();
					sendResponseMsg(1005, error, operation, request, response);
				}
				else
				{
					resDoc.AddMember("StatusCode", 0, allocator);
					resDoc.AddMember("StatusMsg", "success", allocator);
					resDoc.AddMember("AnsNum", rs_ansNum, allocator);
					resDoc.AddMember("OutputLimit", rs_outputlimit, allocator);
					resDoc.AddMember("ThreadId", StringRef(thread_id.c_str()), allocator);
					resDoc.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);
					rapidjson::StringBuffer resBuffer;
					rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
					resDoc.Accept(resWriter);
					string resJson = resBuffer.GetString();

					*response << "HTTP/1.1 200 OK"
							  << "\r\nContent-Type: application/json"
							  << "\r\nContent-Length: " << resJson.length()
							  << "\r\nCache-Control: no-cache"
							  << "\r\nPragma: no-cache"
							  << "\r\nExpires: 0"
							  << "\r\n\r\n"
							  << resJson;
				}
			}
			else if (format == "file")
			{
				outfile.open(localname);
				outfile << success;
				outfile.close();

				rapidjson::StringBuffer s;
				rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(s);
				writer.StartObject();
				writer.Key("StatusCode");
				writer.Uint(0);
				writer.Key("StatusMsg");
				writer.String(StringRef("success"));
				writer.Key("AnsNum");
				writer.Uint(rs_ansNum);
				writer.Key("OutputLimit");
				writer.Uint(rs_outputlimit);

				writer.Key("QueryTime");
				writer.String(StringRef(query_time_s.c_str()));
				writer.Key("FileName");
				writer.String(StringRef(filename.c_str()));
				writer.EndObject();
				string resJson = s.GetString();

				//! Notice: remember to set no-cache in the response of query, Firefox and chrome works well even if you don't set, but IE will act strange if you don't set
				// beacause IE will defaultly cache the query result after first query request, so the following query request of the same url will not be send if the result in cache isn't expired.
				// then the following query will show the same result without sending a request to let the service run query
				// so the download function will go wrong because there is no file in the service.
				*response << "HTTP/1.1 200 OK"
						  << "\r\nContent-Type: application/json"
						  << "\r\nContent-Length: " << resJson.length()
						  << "\r\nCache-Control: no-cache"
						  << "\r\nPragma: no-cache"
						  << "\r\nExpires: 0"
						  << "\r\n\r\n"
						  << resJson;
			}
			else if (format == "json+file" || format == "file+json")
			{
				outfile.open(localname);
				outfile << success;
				outfile.close();

				Document resDoc;
				Document::AllocatorType &allocator = resDoc.GetAllocator();
				/* code */
				resDoc.Parse(success.c_str());
				if (resDoc.HasParseError())
				{
					SLOG_ERROR("result parse error:\n" + success);
					error = "parse error";
					string filename2 = "error_" + filename;
					string localname2 = apiUtil->get_query_result_path() + filename2;
					outfile.open(localname2);
					outfile << success;
					outfile.close();
					sendResponseMsg(1005, error, operation, request, response);
				}
				else
				{
					resDoc.AddMember("StatusCode", 0, allocator);
					resDoc.AddMember("StatusMsg", "success", allocator);
					resDoc.AddMember("AnsNum", rs_ansNum, allocator);
					resDoc.AddMember("OutputLimit", rs_outputlimit, allocator);
					resDoc.AddMember("QueryTime", StringRef(query_time_s.c_str()), allocator);
					resDoc.AddMember("FileName", StringRef(filename.c_str()), allocator);
					StringBuffer resBuffer;
					PrettyWriter<StringBuffer> resWriter(resBuffer);
					resDoc.Accept(resWriter);
					string resJson = resBuffer.GetString();
					*response << "HTTP/1.1 200 OK"
							  << "\r\nContent-Type: application/json"
							  << "\r\nContent-Length: " << resJson.length()
					          << "\r\nCache-Control: no-cache"
							  << "\r\nPragma: no-cache"
							  << "\r\nExpires: 0"
					          << "\r\n\r\n"
							  << resJson;
				}
			}
			else if (format == "sparql-results+json")
			{
				*response << "HTTP/1.1 200 OK"
						  << "\r\nContent-Type: application/sparql-results+json"
						  << "\r\nContent-Length: " << success.length()
						  << "\r\nCache-Control: no-cache"
						  << "\r\nPragma: no-cache"
						  << "\r\nExpires: 0"
						  << "\r\n\r\n"
						  << success; // success contains the json-encoded result
			}
			else
			{
				error = "Unkown result format.";
				sendResponseMsg(1005, error, operation, request, response);
			}
		}
		else
		{
			string error = "";
			int error_code;
			if (update)
			{
				SLOG_DEBUG(log_prefix + "update query returned correctly.");
				error = "update query returns true.";
				error_code = 0;
			}
			else
			{
				SLOG_DEBUG(log_prefix + "search query returned error.");
				error = "search query returns false.";
				error_code = 1005;
			}
			sendResponseMsg(error_code, error, operation, request, response);
		}
		apiUtil->unlock_database(db_name);
		SLOG_DEBUG("query complete!");
	}
	catch (const std::exception &e)
	{
		string error = "Query fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
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
void export_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string db_path, string username)
{
	string error = "";
	string operation = "export";
	try
	{
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		error = apiUtil->check_param_value("db_path", db_path);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not build yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		if (db_path[db_path.length() - 1] != '/')
		{
			db_path = db_path + "/";
		}
		if (Util::dir_exist(db_path) == false)
		{
			Util::create_dirs(db_path);
		}
		db_path = db_path + db_name + "_" + Util::get_timestamp() + ".nt";
		// check if database named [db_name] is already load
		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			string error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		apiUtil->rdlock_database(db_name); // lock database
		SLOG_DEBUG("export_path: " + db_path);
		FILE *ofp = fopen(db_path.c_str(), "w");
		current_database->export_db(ofp);
		fflush(ofp);
		fclose(ofp);
		ofp = NULL;
		current_database = NULL;
		apiUtil->unlock_database(db_name); // unlock
		string success = "Export the database successfully.";

		Document resDoc;
		resDoc.SetObject();
		Document::AllocatorType &allocator = resDoc.GetAllocator();
		resDoc.AddMember("StatusCode", 0, allocator);
		resDoc.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
		resDoc.AddMember("filepath", StringRef(db_path.c_str()), allocator);
		sendResponseMsg(resDoc, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "Export fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved.
 * email：liwenjiehn@pku.edu.cn
 * @description: login ghttp
 * @param {*}
 * @return {*}
 */
void login_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
{
	string operation = "login";
	try
	{
		Document resDoc;
		resDoc.SetObject();
		Document::AllocatorType &allocator = resDoc.GetAllocator();
		resDoc.AddMember("StatusCode", 0, allocator);
		resDoc.AddMember("StatusMsg", "login successfully", allocator);
		string version = Util::getConfigureValue("version");
		resDoc.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
		string licensetype = Util::getConfigureValue("licensetype");
		resDoc.AddMember("licensetype", StringRef(licensetype.c_str()), allocator);
		string cur_path = Util::get_cur_path();
		resDoc.AddMember("RootPath", StringRef(cur_path.c_str()), allocator);
		resDoc.AddMember("type", HTTP_TYPE, allocator);
		sendResponseMsg(resDoc, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "login fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}
/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved.
 * email：liwenjiehn@pku.edu.cn
 * @description: check the ghttp server activity
 * @param {*}
 * @return {*}
 */
void check_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
{
	string success = "the ghttp server is running...";
	string operation = "check";
	string ip = getRemoteIp(request);
	sendResponseMsg(0, success, operation, request, response);
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
void begin_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string isolevel, string username)
{
	string error = "";
	string operation = "begin";
	try
	{
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		error = apiUtil->check_param_value("isolevel", isolevel);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		int level = Util::string2int(isolevel);
		if (level <= 0 || level > 3)
		{
			error = "the Isolation level's value only can been 1/2/3";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		if (apiUtil->get_Txn_ptr(db_name) == NULL)
		{
			error = "Database transaction manager error.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		string TID_s = apiUtil->begin_process(db_name, level, username);
		if (TID_s.empty())
		{
			error = "transaction begin failed.";
			sendResponseMsg(1008, error, operation, request, response);
			return;
		}
		StringBuffer s;
		Document doc;
		doc.SetObject();
		Document::AllocatorType &allocator = doc.GetAllocator();
		doc.AddMember("StatusCode", 0, allocator);
		doc.AddMember("StatusMsg", "transaction begin success", allocator);
		doc.AddMember("TID", StringRef(TID_s.c_str()), allocator);
		sendResponseMsg(doc, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "Transaction begin fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
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
void tquery_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string TID_s, string sparql)
{
	string error = "";
	string operation = "tquery";
	try
	{
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		error = apiUtil->check_param_value("TID", TID_s);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}

		txn_id_t TID;
		TID = apiUtil->check_txn_id(TID_s);
		if (TID == (unsigned long long)0)
		{
			error = "TID "+TID_s+" is not a pure number.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		error = apiUtil->check_param_value("sparql", sparql);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		auto txn_m = apiUtil->get_Txn_ptr(db_name);
		if (txn_m == NULL)
		{
			error = "Get database transaction manager error.";
			sendResponseMsg(1008, error, operation, request, response);
			return;
		}
		SLOG_DEBUG("tquery sparql: " + sparql);
		string res;
		int ret = txn_m->Query(TID, sparql, res);
		if (ret == -1)
		{
			error = "Transaction query failed due to wrong TID";
			sendResponseMsg(1005, error, operation, request, response);
		}
		else if (ret == -10)
		{
			error = "Transaction query failed due to wrong database status";
			apiUtil->rollback_process(txn_m, TID);
			sendResponseMsg(1005, error, operation, request, response);
		}
		else if (ret == -99)
		{
			error = "Transaction query failed. This transaction is not in running status!";
			apiUtil->rollback_process(txn_m, TID);
			sendResponseMsg(1005, error, operation, request, response);
		}
		else if (ret == -100)
		{
			Document resDoc;
			Document::AllocatorType &allocator = resDoc.GetAllocator();
			resDoc.Parse(res.c_str());
			if (resDoc.HasParseError())
			{
				SLOG_ERROR("parse error:" + res);
				resDoc.Parse("{}");
				resDoc.AddMember("result", StringRef(res.c_str()), allocator);
			}
			resDoc.AddMember("StatusCode", 0, allocator);
			resDoc.AddMember("StatusMsg", "success", allocator);
			sendResponseMsg(resDoc, operation, request, response);
		}
		else if (ret == -20)
		{
			error = "Transaction query failed. This transaction is set abort due to conflict!";
			apiUtil->rollback_process(txn_m, TID);
			sendResponseMsg(1005, error, operation, request, response);
		}
		else if (ret == -101)
		{
			error = "Transaction query failed. Unknown query error";
			sendResponseMsg(1005, error, operation, request, response);
		}
		else
		{
			string success = "Transaction query success, update num: " + Util::int2string(ret);
			sendResponseMsg(0, success, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Transaction query fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
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
void commit_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string TID_s)
{
	string error = "";
	string operation = "commit";
	try
	{
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		error = apiUtil->check_param_value("TID", TID_s);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		string res;
		auto TID = apiUtil->check_txn_id(TID_s);
		if (TID == (unsigned long long)0)
		{
			error = "TID " + TID_s + " is not a pure number.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		auto txn_m = apiUtil->get_Txn_ptr(db_name);
		if (txn_m == NULL)
		{
			error = "Get database transaction manager error.";
			sendResponseMsg(1008, error, operation, request, response);
			return;
		}
		int ret = txn_m->Commit(TID);
		if (ret == -1)
		{
			error = "Transaction not found, commit failed. TID: " + TID_s;
			sendResponseMsg(1005, error, operation, request, response);
		}
		else if (ret == 1)
		{
			error = "Transaction not in running state! commit failed. TID: " + TID_s;
			apiUtil->rollback_process(txn_m, TID);
			sendResponseMsg(1005, error, operation, request, response);
		}
		else
		{
			apiUtil->commit_process(txn_m, TID);
			// TODO auto checkpoint are sometimes blocked
			// auto latest_tid = txn_m->find_latest_txn();
			// SLOG_DEBUG("latest TID: "+ to_string(latest_tid));
			// if (latest_tid == 0)
			// {
			// 	SLOG_DEBUG("This is latest TID, auto checkpoint and save.");
			// 	txn_m->Checkpoint();
			// 	SLOG_DEBUG("Transaction checkpoint done.");
			// 	if (apiUtil->trywrlock_database(db_name))
			// 	{
			// 		current_database->save();
			// 		apiUtil->unlock_database(db_name);
			// 	}
			// 	else
			// 	{
			// 		SLOG_ERROR("The save operation can not been excuted due to loss of lock.");
			// 	}
			// }
			string success = "Transaction commit success. TID: " + TID_s;
			sendResponseMsg(0, success, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Transaction commit fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
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
void rollback_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string TID_s)
{
	string error = "";
	string operation = "rollback";
	try
	{
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		error = apiUtil->check_param_value("TID", TID_s);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		auto TID = apiUtil->check_txn_id(TID_s);
		if (TID == (unsigned long long)0)
		{
			error = "TID is not a pure number. TID: " + TID_s;
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet. ";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		auto txn_m = apiUtil->get_Txn_ptr(db_name);
		if (txn_m == NULL)
		{
			string error = "Get database transaction manager error.";
			sendResponseMsg(1008, error, operation, request, response);
			return;
		}
		int ret = txn_m->Rollback(TID);
		if (ret == 1)
		{
			error = "Transaction not in running state! rollback failed. TID: " + TID_s;
			sendResponseMsg(1005, error, operation, request, response);
		}
		else if (ret == -1)
		{
			error = "Transaction not found, rollback failed. TID: " + TID_s;
			sendResponseMsg(1005, error, operation, request, response);
		}
		else
		{
			apiUtil->rollback_process(txn_m, TID);
			string success = "Transaction rollback success. TID: " + TID_s;
			sendResponseMsg(0, success, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Transaction rollback fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
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
void txnlog_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, int page_no, int page_size)
{
	string operation = "txnlog";
	try
	{
		// if (username != apiUtil->get_root_username())
		// {
		// 	string error = "Root User Only!";
		// 	sendResponseMsg(1004, error, operation, request, response);
		// 	return;
		// }

		struct TransactionLogs transactionLogs;
		apiUtil->get_transactionlog(page_no, page_size, &transactionLogs);
		vector<struct TransactionLogInfo> logList = transactionLogs.getTransactionLogInfoList();
		size_t count = logList.size();
		string line = "";
		stringstream str_stream;
		str_stream << "[";
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			TransactionLogInfo log_info = logList[i];
			line = log_info.toJSON();
			str_stream << line;
		}
		str_stream << "]";

		Document all;
		Document jsonArray;
		Document::AllocatorType &allocator = all.GetAllocator();
		all.SetObject();
		jsonArray.SetArray();
		line = str_stream.str();
		jsonArray.Parse(line.c_str());

		int totalSize = transactionLogs.getTotalSize();
		int totalPage = transactionLogs.getTotalPage();
		all.AddMember("StatusCode", 0, allocator);
		all.AddMember("StatusMsg", "Get transaction log success.", allocator);
		all.AddMember("totalSize", totalSize, allocator);
		all.AddMember("totalPage", totalPage, allocator);
		all.AddMember("pageNo", page_no, allocator);
		all.AddMember("pageSize", page_size, allocator);
		all.AddMember("list", jsonArray, allocator);

		sendResponseMsg(all, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "Query fail: " + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
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
void checkpoint_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name)
{
	string error;
	string operation = "checkpoint";
	try
	{
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			sendResponseMsg(1007, error, operation, request, response);
		}
		else
		{
			auto txn_m = apiUtil->get_Txn_ptr(db_name);
			if (txn_m == NULL)
			{
				error = "Get database transaction manager error.";
				apiUtil->unlock_database(db_name);
				sendResponseMsg(1008, error, operation, request, response);
			}
			else
			{
				txn_m->Checkpoint();
				current_database->save();
				apiUtil->unlock_database(db_name);
				string success = "Database saved successfully.";
				sendResponseMsg(0, success, operation, request, response);
			}
		}
	}
	catch (const std::exception &e)
	{
		string error = "Checkpoint fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

void test_connect_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
{
	string operation = "testconnect";
	try
	{
		Document resDoc;
		resDoc.SetObject();
		Document::AllocatorType &allocator = resDoc.GetAllocator();
		resDoc.AddMember("StatusCode", 0, allocator);
		resDoc.AddMember("StatusMsg", "success", allocator);
		string version = Util::getConfigureValue("version");
		resDoc.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
		string licensetype = Util::getConfigureValue("licensetype");
		resDoc.AddMember("licensetype", StringRef(licensetype.c_str()), allocator);
		resDoc.AddMember("type", HTTP_TYPE, allocator);
		sendResponseMsg(resDoc, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "Test connect fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

void getCoreVersion_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
{
	string operation = "getcoreversion";
	try
	{
		Document resDoc;
		resDoc.SetObject();
		Document::AllocatorType &allocator = resDoc.GetAllocator();
		resDoc.AddMember("StatusCode", 0, allocator);
		resDoc.AddMember("StatusMsg", "success", allocator);
		string version = Util::getConfigureValue("version");
		resDoc.AddMember("CoreVersion", StringRef(version.c_str()), allocator);
		resDoc.AddMember("type", HTTP_TYPE, allocator);
		sendResponseMsg(resDoc, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "Get core version fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved.
 * email：liwenjiehn@pku.edu.cn
 * @description:
 * @param {string} db_name: the name of target database
 * @param {string} file: the insert data file
 * @return {*}
 */
void batchInsert_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string file, string IP)
{
	string error;
	string operation = "batchInsert";
	try
	{
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		error = apiUtil->check_param_value("file", file);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (Util::file_exist(file) == false)
		{
			error = "The data file is not exist";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			sendResponseMsg(1004, error, operation, request, response);
		}
		else
		{
			string success = "Batch insert data successfully.";
			unsigned success_num = current_database->batch_insert(file, false, nullptr);
			current_database->save();
			apiUtil->unlock_database(db_name);

			Document resDoc;
			resDoc.SetObject();
			Document::AllocatorType &allocator = resDoc.GetAllocator();
			resDoc.AddMember("StatusCode", 0, allocator);
			resDoc.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
			resDoc.AddMember("success_num", StringRef(Util::int2string(success_num).c_str()), allocator);
			sendResponseMsg(resDoc, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Batch insert fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @Author: liwenjie
 * Copyright 2021 gStore, All Rights Reserved.
 * email：liwenjiehn@pku.edu.cn
 * @description:
 * @param {string} db_name: the target database's name
 * @param {string} file: the remove data file
 * @return {*}
 */
void batchRemove_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string file, string IP)
{
	string error;
	string operation = "batchremove";
	try
	{
		error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		error = apiUtil->check_param_value("file", file);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (Util::file_exist(file) == false)
		{
			error = "the data file is not exist";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (apiUtil->check_db_exist(db_name) == false)
		{
			error = "Database not built yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		if (apiUtil->check_already_load(db_name) == false)
		{
			error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		Database *current_database = apiUtil->get_database(db_name);
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			sendResponseMsg(1007, error, operation, request, response);
		}
		else
		{
			string success = "Batch remove data successfully.";
			unsigned success_num = current_database->batch_remove(file, false, nullptr);
			current_database->save();
			apiUtil->unlock_database(db_name);

			Document resDoc;
			resDoc.SetObject();
			Document::AllocatorType &allocator = resDoc.GetAllocator();
			resDoc.AddMember("StatusCode", 0, allocator);
			resDoc.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
			resDoc.AddMember("success_num", StringRef(Util::int2string(success_num).c_str()), allocator);
			sendResponseMsg(resDoc, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Batch remove fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

void request_thread(const shared_ptr<HttpServer::Response> &response,
					const shared_ptr<HttpServer::Request> &request, string request_type)
{
	string remote_ip = getRemoteIp(request);
	string ipCheckResult = apiUtil->check_access_ip(remote_ip);
	if (!ipCheckResult.empty())
	{
		sendResponseMsg(1101, ipCheckResult, "ipcheck", request, response);
		return;
	}
	string thread_id = Util::getThreadID();
	string log_prefix = "thread " + thread_id + " -- ";
	string username;
	string password;
	string encryption;
	string operation;
	string db_name;
	Document document;
	string url;

	if (request_type == "GET")
	{
		url = request->path;
		url = UrlDecode(url);
		SLOG_DEBUG("get url: \n" + url);

		operation = WebUrl::CutParam(url, "operation");
		username = WebUrl::CutParam(url, "username");
		password = WebUrl::CutParam(url, "password");
		db_name = WebUrl::CutParam(url, "db_name");
		encryption = WebUrl::CutParam(url, "encryption");

		username = UrlDecode(username);
		password = UrlDecode(password);
		db_name = UrlDecode(db_name);
	}
	else if (request_type == "POST")
	{
		auto strJson = request->content.string();
		SLOG_DEBUG("post content: \n" + strJson);

		document.Parse(strJson.c_str());
		if (document.HasParseError())
		{
			string error = "the post content is not fit the json format, content=" + strJson;
			sendResponseMsg(1004, error, "paramcheck ", request, response);
			return;
		}
		operation = "";
		db_name = "";
		username = "";
		password = "";
		if (document.HasMember("operation") && document["operation"].IsString())
		{
			operation = document["operation"].GetString();
		}
		if (document.HasMember("db_name") && document["db_name"].IsString())
		{
			db_name = document["db_name"].GetString();
		}
		if (document.HasMember("username") && document["username"].IsString())
		{
			username = document["username"].GetString();
		}
		if (document.HasMember("password") && document["password"].IsString())
		{
			password = document["password"].GetString();
		}
		if (document.HasMember("encryption"))
		{
			encryption = document["encryption"].GetString();
		}
		else
		{
			encryption = "0";
		}
	}
	else
	{
		string msg = "The method type " + request_type + " is not support";
		sendResponseMsg(1004, msg, "methodcheck", request, response);
		return;
	}
	stringstream ss;
	ss << "\n------------------------ ghttp-api ------------------------";
	ss << "\nthread_id: " << thread_id;
	ss << "\nremote_ip: " << remote_ip;
	ss << "\noperation: " << operation;
	ss << "\nmethod: " << request_type;
	ss << "\nrequest_path: " << request->path;
	ss << "\nhttp_version: " << request->http_version;
	ss << "\nrequest_time: " << Util::get_date_time();
	ss << "\n----------------------------------------------------------";
	SLOG_DEBUG(ss.str());
	if (operation == "check")
	{
		check_thread_new(request, response);
		return;
	}

	string checkidentityresult = apiUtil->check_indentity(username, password, encryption);
	if (checkidentityresult.empty() == false)
	{
		apiUtil->update_access_ip_error_num(remote_ip);
		sendResponseMsg(1001, checkidentityresult, "authcheck", request, response);
		return;
	}
	if (apiUtil->check_privilege(username, operation, db_name) == 0)
	{
		string msg = "You have no " + operation + " privilege, operation failed";
		sendResponseMsg(1004, msg, "privilegecheck", request, response);
		return;
	}
	// build database
	if (operation == "build")
	{
		string db_path = "";
		string port;
		try
		{
			if (request_type == "GET")
			{
				db_path = WebUrl::CutParam(url, "db_path");
				db_path = UrlDecode(db_path);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("db_path") && document["db_path"].IsString())
				{
					db_path = document["db_path"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		build_thread_new(request, response, db_name, db_path, username, password);
	}
	// load dababase
	else if (operation == "load")
	{
		bool load_csr = false;
		string loadCSRStr = "";
		string port;
		try
		{
			if (request_type == "GET")
			{
				loadCSRStr = WebUrl::CutParam(url, "csr");
				loadCSRStr = UrlDecode(loadCSRStr);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("csr") && document["csr"].IsString())
				{
					loadCSRStr = document["csr"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (loadCSRStr == "1")
			load_csr = true;
		load_thread_new(request, response, db_name, load_csr);
	}
	// monitor database
	else if (operation == "monitor")
	{
		monitor_thread_new(request, response, db_name);
	}
	// unload database
	else if (operation == "unload")
	{
		unload_thread_new(request, response, db_name);
	}
	// drop database
	else if (operation == "drop")
	{
		string is_backup = "true";
		try
		{
			if (request_type == "GET")
			{
				is_backup = WebUrl::CutParam(url, "is_backup");
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("is_backup") && document["is_backup"].IsString())
				{
					is_backup = document["is_backup"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error, please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (is_backup.empty())
		{
			is_backup = "true";
		}

		drop_thread_new(request, response, db_name, is_backup);
	}
	// show all databases
	else if (operation == "show")
	{
		show_thread_new(request, response, username);
	}
	// manage the user list
	else if (operation == "usermanage")
	{
		string type = "";
		string op_username = "";
		string op_password = "";
		try
		{
			if (request_type == "GET")
			{
				type = WebUrl::CutParam(url, "type");
				op_username = WebUrl::CutParam(url, "op_username");
				op_username = UrlDecode(op_username);
				op_password = WebUrl::CutParam(url, "op_password");
				op_password = UrlDecode(op_password);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("type") && document["type"].IsString())
				{
					type = document["type"].GetString();
				}
				if (document.HasMember("op_username") && document["op_username"].IsString())
				{
					op_username = document["op_username"].GetString();
				}
				if (document.HasMember("op_password") && document["op_password"].IsString())
				{
					op_password = document["op_password"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		userManager_thread_new(request, response, op_username, op_password, type);
	}
	// show the user list
	else if (operation == "showuser")
	{
		showuser_thread_new(request, response);
	}
	// manage user privilege
	else if (operation == "userprivilegemanage")
	{
		string type = "";
		string op_username = "";
		string privileges = "";
		try
		{
			if (request_type == "GET")
			{
				type = WebUrl::CutParam(url, "type");
				op_username = WebUrl::CutParam(url, "op_username");
				privileges = WebUrl::CutParam(url, "privileges");
			}
			else if (request_type == "POST")
			{
				type = document["type"].GetString();

				if (document.HasMember("op_username") && document["op_username"].IsString())
				{
					op_username = document["op_username"].GetString();
				}
				if (document.HasMember("privileges") && document["privileges"].IsString())
				{
					privileges = document["privileges"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		userPrivilegeManage_thread_new(request, response, op_username, privileges, type, db_name);
	}
	// change user password
	else if (operation == "userpassword")
	{
		string op_password = "";
		try
		{
			if (request_type == "GET")
			{
				op_password = WebUrl::CutParam(url, "op_password");
				op_password = UrlDecode(op_password);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("op_password") && document["op_password"].IsString())
				{
					op_password = document["op_password"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		userPassword_thread_new(request, response, username, op_password);
	}
	// backup a database
	else if (operation == "backup")
	{
		string backup_path = "";
		try
		{
			if (request_type == "GET")
			{
				backup_path = WebUrl::CutParam(url, "backup_path");
				backup_path = UrlDecode(backup_path);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("backup_path") && document["backup_path"].IsString())
				{
					backup_path = document["backup_path"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		backup_thread_new(request, response, db_name, backup_path);
	}
	// restore database
	else if (operation == "restore")
	{
		string backup_path = "";
		try
		{
			if (request_type == "GET")
			{
				backup_path = WebUrl::CutParam(url, "backup_path");
				backup_path = UrlDecode(backup_path);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("backup_path") && document["backup_path"].IsString())
				{
					backup_path = document["backup_path"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		restore_thread_new(request, response, db_name, backup_path, username);
	}
	// query database
	else if (operation == "query")
	{
		string format = "";
		string sparql = "";

		string querytype = "0";
		if (apiUtil->check_privilege(username, "update", db_name) == 0)
		{
			querytype = "0";
		}
		else
		{
			querytype = "1";
		}
		try
		{
			if (request_type == "GET")
			{
				format = WebUrl::CutParam(url, "format");
				format = UrlDecode(format);
				sparql = WebUrl::CutParam(url, "sparql");
				sparql = UrlDecode(sparql);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("format") && document["format"].IsString())
				{
					format = document["format"].GetString();
				}
				if (document.HasMember("sparql") && document["sparql"].IsString())
				{
					sparql = document["sparql"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}

		if (format.empty())
		{
			format = "json";
		}

		query_num++;
		Task *task = new Task(username, db_name, format, remote_ip, log_prefix, querytype, sparql, response, request);
		pool.AddTask(task);
	}
	else if (operation == "export")
	{
		string db_path = "";
		try
		{
			if (request_type == "GET")
			{
				db_path = WebUrl::CutParam(url, "db_path");
				db_path = UrlDecode(db_path);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("db_path") && document["db_path"].IsString())
				{
					db_path = document["db_path"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		export_thread_new(request, response, db_name, db_path, username);
	}
	else if (operation == "login")
	{
		login_thread_new(request, response);
	}
	else if (operation == "begin")
	{
		string isolevel = "";
		try
		{
			if (request_type == "GET")
			{
				isolevel = WebUrl::CutParam(url, "isolevel");
				isolevel = UrlDecode(isolevel);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("isolevel") && document["isolevel"].IsString())
				{
					isolevel = document["isolevel"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		begin_thread_new(request, response, db_name, isolevel, username);
	}
	else if (operation == "tquery")
	{
		string TID = "";
		string sparql = "";
		try
		{
			if (request_type == "GET")
			{
				TID = WebUrl::CutParam(url, "tid");
				TID = UrlDecode(TID);
				sparql = WebUrl::CutParam(url, "sparql");
				sparql = UrlDecode(sparql);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("tid") && document["tid"].IsString())
				{
					TID = document["tid"].GetString();
				}
				if (document.HasMember("sparql") && document["sparql"].IsString())
				{
					sparql = document["sparql"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		tquery_thread_new(request, response, db_name, TID, sparql);
	}
	else if (operation == "commit")
	{
		string TID = "";
		try
		{
			if (request_type == "GET")
			{
				TID = WebUrl::CutParam(url, "tid");
				TID = UrlDecode(TID);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("tid") && document["tid"].IsString())
				{
					TID = document["tid"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		commit_thread_new(request, response, db_name, TID);
	}
	else if (operation == "rollback")
	{
		string TID = "";
		try
		{
			if (request_type == "GET")
			{
				TID = WebUrl::CutParam(url, "tid");
				TID = UrlDecode(TID);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("tid") && document["tid"].IsString())
				{
					TID = document["tid"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		rollback_thread_new(request, response, db_name, TID);
	}
	else if (operation == "txnlog")
	{

		int page_no = 1;
		int page_size = 10;
		try
		{
			if (request_type == "GET")
			{
				string str_page_no = WebUrl::CutParam(url, "pageNo");
				string str_page_size = WebUrl::CutParam(url, "pageSize");
				page_no = Util::string2int(str_page_no);
				page_size = Util::string2int(str_page_size);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("pageNo") && document["pageNo"].IsInt())
				{
					page_no = document["pageNo"].GetInt();
				}
				if (document.HasMember("pageSize") && document["pageSize"].IsInt())
				{
					page_size = document["pageSize"].GetInt();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		txnlog_thread_new(request, response, username, page_no, page_size);
	}
	else if (operation == "checkpoint")
	{
		checkpoint_thread_new(request, response, db_name);
	}
	else if (operation == "testConnect")
	{
		test_connect_thread_new(request, response);
	}
	else if (operation == "getCoreVersion")
	{
		getCoreVersion_thread_new(request, response);
	}
	else if (operation == "batchInsert")
	{
		string file = "";
		try
		{
			if (request_type == "GET")
			{
				file = WebUrl::CutParam(url, "file");
				file = UrlDecode(file);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("file") && document["file"].IsString())
				{
					file = document["file"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		batchInsert_thread_new(request, response, db_name, file, remote_ip);
	}
	else if (operation == "batchRemove")
	{
		string file = "";
		try
		{
			if (request_type == "GET")
			{
				file = WebUrl::CutParam(url, "file");
				file = UrlDecode(file);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("file") && document["file"].IsString())
				{
					file = document["file"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		batchRemove_thread_new(request, response, db_name, file, remote_ip);
	}
	else if (operation == "querylog")
	{
		string date = "";
		int page_no = 1;
		int page_size = 10;
		try
		{
			if (request_type == "GET")
			{
				date = WebUrl::CutParam(url, "date");
				string str_page_no = WebUrl::CutParam(url, "pageNo");
				string str_page_size = WebUrl::CutParam(url, "pageSize");
				page_no = Util::string2int(str_page_no);
				page_size = Util::string2int(str_page_size);
				date = UrlDecode(date);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("date") && document["date"].IsString())
				{
					date = document["date"].GetString();
				}
				if (document.HasMember("pageNo") && document["pageNo"].IsInt())
				{
					page_no = document["pageNo"].GetInt();
				}
				if (document.HasMember("pageSize") && document["pageSize"].IsInt())
				{
					page_size = document["pageSize"].GetInt();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		querylog_thread_new(request, response, date, page_no, page_size);
	}
	else if (operation == "querylogdate")
	{
		querylogdate_thread_new(request, response);
	}
	else if (operation == "accesslog")
	{
		string date = "";
		int page_no = 1;
		int page_size = 10;
		try
		{
			if (request_type == "GET")
			{
				date = WebUrl::CutParam(url, "date");
				string str_page_no = WebUrl::CutParam(url, "pageNo");
				string str_page_size = WebUrl::CutParam(url, "pageSize");
				page_no = Util::string2int(str_page_no);
				page_size = Util::string2int(str_page_size);
				date = UrlDecode(date);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("date") && document["date"].IsString())
				{
					date = document["date"].GetString();
				}
				if (document.HasMember("pageNo") && document["pageNo"].IsInt())
				{
					page_no = document["pageNo"].GetInt();
				}
				if (document.HasMember("pageSize") && document["pageSize"].IsInt())
				{
					page_size = document["pageSize"].GetInt();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		accesslog_thread_new(request, response, date, page_no, page_size);
	}
	else if (operation == "accesslogdate")
	{
		accesslogdate_thread_new(request, response);
	}
	else if (operation == "ipmanage")
	{
		string type = "";
		string ips = "";
		string ip_type = "";
		try
		{
			if (request_type == "GET")
			{
				type = WebUrl::CutParam(url, "type");
				if (type == "2")
				{
					ips = WebUrl::CutParam(url, "ips");
					ip_type = WebUrl::CutParam(url, "ip_type");
				}
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("type") && document["type"].IsString())
				{
					type = document["type"].GetString();
				}
				if (type == "2")
				{
					if (document.HasMember("ips") && document["ips"].IsString())
					{
						ips = document["ips"].GetString();
					}
					if (document.HasMember("ip_type") && document["ip_type"].IsString())
					{
						ip_type = document["ip_type"].GetString();
					}
				}
			}
			ipmanage_thread_new(request, response, type, ips, ip_type);
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
	}
	else if (operation == "funquery")
	{
		struct PFNInfo pfn_info;
		try
		{
			if (request_type == "POST")
			{
				if (document.HasMember("funInfo"))
				{
					Value &fun_info = document["funInfo"];
					build_PFNInfo(fun_info, pfn_info);
				}
			}
			else
			{
				string error = "please use POST";
				sendResponseMsg(1003, error, operation, request, response);
				return;
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		fun_query_thread_new(request, response, username, pfn_info);
	}
	else if (operation == "funcudb")
	{
		struct PFNInfo pfn_info;
		string type = "";
		try
		{
			if (request_type == "POST")
			{
				if (document.HasMember("funInfo") && document["funInfo"].IsObject())
				{
					Value &fun_info = document["funInfo"];
					build_PFNInfo(fun_info, pfn_info);
				}
				if (document.HasMember("type") && document["type"].IsString())
				{
					type = document["type"].GetString();
				}
			}
			else
			{
				string error = "please use POST";
				sendResponseMsg(1003, error, operation, request, response);
				return;
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		fun_cudb_thread_new(request, response, username, pfn_info, type);
	}
	else if (operation == "funreview")
	{
		struct PFNInfo pfn_info;
		try
		{
			if (request_type == "POST")
			{
				if (document.HasMember("funInfo"))
				{
					Value &fun_info = document["funInfo"];
					build_PFNInfo(fun_info, pfn_info);
				}
			}
			else
			{
				string error = "please use POST";
				sendResponseMsg(1003, error, operation, request, response);
				return;
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		fun_review_thread_new(request, response, username, pfn_info);
	}
	else
	{
		string error = "the operation " + operation + " has not match handler function";
		sendResponseMsg(1100, error, "operationcheck", request, response);
	}
}
/**
 * @Author: liwenjie
 * @param {const} HttpServer
 * @param {const} shared_ptr
 * @param {const} shared_ptr
 * @param {string} request_type
 */
void request_handler(const HttpServer &server, const shared_ptr<HttpServer::Response> &response, const shared_ptr<HttpServer::Request> &request, string request_type)
{

	// string operation;
	// Document document;
	// string url;
	// string postcontent;
	// if (request_type == "GET")
	// {
	// 	cout << "request path:" << request->path << endl;

	// 	url = request->path;
	// 	url=UrlDecode(url);
	// 	cout << "request path:" << url << endl;
	// 	operation=WebUrl::CutParam(url, "operation");

	// }
	// else if (request_type == "POST")
	// {
	// 	auto strJson = request->content.string();
	//     cout<<"post content:"<<endl;
	// 	cout<<strJson<<endl;
	//     postcontent=strJson.c_str();

	//     document.Parse(strJson.c_str());
	// 	if(document.HasParseError())
	// 	{
	// 		string error = "the post content is not fit the json format,content=" + strJson;
	// 		sendResponseMsg(1003, error, response);
	// 		return ;
	// 	}

	// 	operation="";

	// 	if(document.HasMember("operation")&&document["operation"].IsString())
	// 	{
	// 		operation=document["operation"].GetString();
	// 	}

	// }
	// if(operation=="shutdown")
	// {
	//    bool flag = stop_handler(server, response, request,request_type);
	// 	if (flag)
	// 	{

	// 		cout<<"the Server is stopped！"<<endl;
	//         exit(1);
	// 		return;
	// 	}

	// }
	// else{
	thread t(&request_thread, response, request, request_type);
	t.detach();
	return;
	// }
}

void shutdown_handler(const HttpServer &server, const shared_ptr<HttpServer::Response> &response, const shared_ptr<HttpServer::Request> &request, string request_type)
{
	string operation = "shutdown";
	string remote_ip = getRemoteIp(request);
	string ipCheckResult = apiUtil->check_access_ip(remote_ip);
	if (!ipCheckResult.empty())
	{
		sendResponseMsg(1101, ipCheckResult, operation, request, response);
		return;
	}
	string username;
	string password;

	Document document;
	string url;
	if (request_type == "GET")
	{
		url = request->path;
		url = UrlDecode(url);

		username = WebUrl::CutParam(url, "username");
		password = WebUrl::CutParam(url, "password");

		username = UrlDecode(username);
		password = UrlDecode(password);
	}
	else if (request_type == "POST")
	{
		auto strJson = request->content.string();
		SLOG_DEBUG("shutdown post content: \n" + strJson);

		document.Parse(strJson.c_str());
		if (document.HasParseError())
		{
			string error = "the post content is not fit the json format,content=" + strJson;
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}

		username = "";
		password = "";

		if (document.HasMember("username") && document["username"].IsString())
		{
			username = document["username"].GetString();
		}
		if (document.HasMember("password") && document["password"].IsString())
		{
			password = document["password"].GetString();
		}
	}
	else
	{
		string msg = "The method type " + request_type + " is not support";
		sendResponseMsg(1000, msg, operation, request, response);
		return;
	}
	// check identity.
	if (username != apiUtil->get_system_username())
	{
		string error = "You have no rights to stop the server.";
		sendResponseMsg(1001, error, operation, request, response);
		return;
	}
	string checkidentityresult = apiUtil->check_server_indentity(password);
	if (checkidentityresult.empty() == false)
	{
		apiUtil->update_access_ip_error_num(remote_ip);
		sendResponseMsg(1001, checkidentityresult, operation, request, response);
		return;
	}
	bool flag = apiUtil->db_checkpoint_all();
	if (flag)
	{
		string msg = "Server stopped successfully.";
		string resJson = CreateJson(0, msg, 0);
		SLOG_DEBUG("response result:\n" + resJson);
		apiUtil->write_access_log(operation, remote_ip, 0, msg);
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n"
				  << resJson;
		delete apiUtil;
		std::cout.flush();
		// TODO exit synchlized
		_exit(1);
	}
	else
	{
		string msg = "Server stopped failed.";
		string resJson = CreateJson(1005, msg, 0);
		SLOG_DEBUG("response result:\n" + resJson);
		apiUtil->write_access_log(operation, remote_ip, 1005, msg);
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n"
				  << resJson;
	}
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
	if (body)
	{
		writer.Key("ResponseBody");
		writer.String(StringRef(ResponseBody.c_str()));
	}
	writer.EndObject();
	return s.GetString();
}

/**
 * query log thread
 *
 * @param response
 * @param date
 * @param page_no
 * @param page_size
 */
void querylog_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string date, int page_no, int page_size)
{
	string ip = getRemoteIp(request);
	string operation = "querylog";
	try
	{
		std::string error = apiUtil->check_param_value("date", date);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		struct DBQueryLogs dbQueryLogs;
		apiUtil->get_query_log(date, page_no, page_size, &dbQueryLogs);
		vector<struct DBQueryLogInfo> logList = dbQueryLogs.getQueryLogInfoList();
		size_t count = logList.size();
		string line = "";
		stringstream str_stream;
		str_stream << "[";
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			DBQueryLogInfo log_info = logList[i];
			line = log_info.toJSON();
			str_stream << line;
		}
		str_stream << "]";
		Document all;
		Document jsonArray;
		Document::AllocatorType &allocator = all.GetAllocator();
		all.SetObject();
		jsonArray.SetArray();
		line = str_stream.str();
		jsonArray.Parse(line.c_str());
		int totalSize = dbQueryLogs.getTotalSize();
		int totalPage = dbQueryLogs.getTotalPage();
		all.AddMember("StatusCode", 0, allocator);
		all.AddMember("StatusMsg", "Get query log success", allocator);
		all.AddMember("totalSize", totalSize, allocator);
		all.AddMember("totalPage", totalPage, allocator);
		all.AddMember("pageNo", page_no, allocator);
		all.AddMember("pageSize", page_size, allocator);
		all.AddMember("list", jsonArray, allocator);
		sendResponseMsg(all, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "Get query log fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * query log date thread
 * 
 * @param request 
 * @param response 
 */
void querylogdate_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
{
	string operation = "querylogdate";
	try
	{
		vector<string> logfiles;
		apiUtil->get_query_log_files(logfiles);
		sort(logfiles.begin(), logfiles.end(), [](const string& a, const string& b) {
			return a > b;
		});
		std::stringstream str_stream;
		str_stream << "[";
		size_t count = logfiles.size();
		std::string item;
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			item = logfiles[i];
			item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
			str_stream << "\"" << item << "\"";
		}
		str_stream << "]";
		rapidjson::Document resp_data;
		rapidjson::Document array_data;
		resp_data.SetObject();
		array_data.SetArray();
		rapidjson::Document::AllocatorType &allocator = resp_data.GetAllocator();
		array_data.Parse(str_stream.str().c_str());
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get query log date success", allocator);
		resp_data.AddMember("list", array_data, allocator);
		sendResponseMsg(resp_data, operation, request, response);
	}
	catch(const std::exception& e)
	{
		string error = "Get query log date fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * access log thread
 *
 * @param response
 * @param date
 * @param page_no
 * @param page_size
 */
void accesslog_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string date, int page_no, int page_size)
{
	string ip = getRemoteIp(request);
	string operation = "accesslog";
	try
	{
		struct DBAccessLogs dbAccessLogs;
		apiUtil->get_access_log(date, page_no, page_size, &dbAccessLogs);
		vector<struct DBAccessLogInfo> logList = dbAccessLogs.getAccessLogInfoList();
		size_t count = logList.size();
		string line = "";
		stringstream str_stream;
		str_stream << "[";
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			DBAccessLogInfo log_info = logList[i];
			line = log_info.toJSON();
			str_stream << line;
		}
		str_stream << "]";

		Document all;
		Document jsonArray;
		Document::AllocatorType &allocator = all.GetAllocator();
		all.SetObject();
		jsonArray.SetArray();
		line = str_stream.str();
		jsonArray.Parse(line.c_str());

		int totalSize = dbAccessLogs.getTotalSize();
		int totalPage = dbAccessLogs.getTotalPage();
		all.AddMember("StatusCode", 0, allocator);
		all.AddMember("StatusMsg", "Get access log success", allocator);
		all.AddMember("totalSize", totalSize, allocator);
		all.AddMember("totalPage", totalPage, allocator);
		all.AddMember("pageNo", page_no, allocator);
		all.AddMember("pageSize", page_size, allocator);
		all.AddMember("list", jsonArray, allocator);
		sendResponseMsg(all, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "Get access log" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * access log date thread
 * 
 * @param request 
 * @param response 
 */
void accesslogdate_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
{
	string operation = "accesslogdate";
	try
	{
		vector<string> logfiles;
		apiUtil->get_access_log_files(logfiles);
		sort(logfiles.begin(), logfiles.end(), [](const string& a, const string& b) {
			return a > b;
		});
		std::stringstream str_stream;
		str_stream << "[";
		size_t count = logfiles.size();
		std::string item;
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			item = logfiles[i];
			item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
			str_stream << "\"" << item << "\"";
		}
		str_stream << "]";
		rapidjson::Document resp_data;
		rapidjson::Document array_data;
		resp_data.SetObject();
		array_data.SetArray();
		rapidjson::Document::AllocatorType &allocator = resp_data.GetAllocator();
		array_data.Parse(str_stream.str().c_str());
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "Get access log date success", allocator);
		resp_data.AddMember("list", array_data, allocator);
		sendResponseMsg(resp_data, operation, request, response);
	}
	catch(const std::exception& e)
	{
		string error = "Get access log date fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}
/**
 * @brief IP manager
 *
 * @param response
 * @param ips ip string
 * @param ip_type 1-black ip 2-white ip
 * @param type  1-query 2-save
 */
void ipmanage_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string type, string ips, string ip_type)
{
	string ip = getRemoteIp(request);
	string operation = "ipmanage";
	try
	{
		Document all;
		Document::AllocatorType &allocator = all.GetAllocator();
		all.SetObject();
		if (type == "1")
		{
			string IPtype = apiUtil->ip_enabled_type();
			if (IPtype == "3")
			{
				sendResponseMsg(1005, "please configure ip_deny_path or ip_allow_path in the conf.ini file first.", operation, request, response);
				return;
			}
			vector<string> ip_list = apiUtil->ip_list(IPtype);
			size_t count = ip_list.size();
			stringstream str_stream;
			str_stream << "[";
			for (size_t i = 0; i < count; i++)
			{
				if (i > 0)
				{
					str_stream << ",";
				}
				str_stream << "\"" << ip_list[i] << "\"";
			}
			str_stream << "]";
			Document responseBody;
			Document listDoc;
			responseBody.SetObject();
			listDoc.SetArray();
			listDoc.Parse(str_stream.str().c_str());
			responseBody.AddMember("ip_type", StringRef(IPtype.c_str()), allocator);
			responseBody.AddMember("ips", listDoc, allocator);

			all.AddMember("StatusCode", 0, allocator);
			all.AddMember("StatusMsg", "success", allocator);
			all.AddMember("ResponseBody", responseBody, allocator);
			sendResponseMsg(all, operation, request, response);
		}
		else if (type == "2")
		{
			if (ips.empty())
			{
				sendResponseMsg(1003, "the ips can't be empty", operation, request, response);
				return;
			}
			vector<string> ipVector;
			Util::split(ips, ",", ipVector);
			if (ip_type == "1" || ip_type == "2")
			{
				bool rt = apiUtil->ip_save(ip_type, ipVector);
				if (rt)
				{
					sendResponseMsg(0, "success", operation, request, response);
				}
				else
				{
					if (ip_type == "1")
					{
						sendResponseMsg(1005, "ip_deny_path is not configured, please configure it in the conf.ini file first.", operation, request, response);
					}
					else
					{
						sendResponseMsg(1005, "ip_allow_path is not configured, please configure it in the conf.ini file first.", operation, request, response);
					}
				}
			}
			else
			{
				sendResponseMsg(1003, "ip_type is invalid, please look up the api document.", operation, request, response);
			}
		}
		else
		{
			sendResponseMsg(1003, "type is invalid, please look up the api document.", operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "IP manger fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

void fun_query_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, struct PFNInfo &pfn_info)
{
	string ip = getRemoteIp(request);
	string operation = "funquery";
	try
	{
		struct PFNInfos *pfn_infos = new PFNInfos();
		apiUtil->fun_query(pfn_info.getFunName(), pfn_info.getFunStatus(), username, pfn_infos);
		vector<struct PFNInfo> list = pfn_infos->getPFNInfoList();
		size_t count = list.size();
		string line = "";
		stringstream str_stream;
		str_stream << "[";
		for (size_t i = 0; i < count; i++)
		{
			if (i > 0)
			{
				str_stream << ",";
			}
			PFNInfo pfn_info = list[i];
			line = pfn_info.toJSON();
			str_stream << line;
		}
		str_stream << "]";

		Document all;
		Document jsonArray;
		Document::AllocatorType &allocator = all.GetAllocator();
		all.SetObject();
		jsonArray.SetArray();
		line = str_stream.str();
		jsonArray.Parse(line.c_str());

		all.AddMember("StatusCode", 0, allocator);
		all.AddMember("StatusMsg", "success", allocator);
		all.AddMember("list", jsonArray, allocator);

		sendResponseMsg(all, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string msg = "Function query fail:" + string(e.what());
		sendResponseMsg(1005, msg, operation, request, response);
	}
}

void fun_cudb_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, struct PFNInfo &pfn_info, string type)
{
	string operation = "funcudb";
	std::string error = apiUtil->check_param_value("type", type);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, operation, request, response);
		return;
	}
	if (type == "1")
	{
		operation = "funcreate";
		try
		{
			apiUtil->fun_create(username, &pfn_info);
			sendResponseMsg(0, "success", operation, request, response);
		}
		catch (const std::exception &e)
		{
			string msg = "Function create fail:" + string(e.what());
			sendResponseMsg(1005, msg, operation, request, response);
		}
	}
	else if (type == "2")
	{
		operation = "funupdate";
		try
		{
			apiUtil->fun_update(username, &pfn_info);
			sendResponseMsg(0, "success", operation, request, response);
		}
		catch (const std::exception &e)
		{
			string msg = "Function update fail:" + string(e.what());
			sendResponseMsg(1005, msg, operation, request, response);
		}
	}
	else if (type == "3")
	{
		operation = "fundelete";
		try
		{
			apiUtil->fun_delete(username, &pfn_info);
			sendResponseMsg(0, "success", operation, request, response);
		}
		catch (const std::exception &e)
		{
			string msg = "Function delete fail:" + string(e.what());
			sendResponseMsg(1005, msg, operation, request, response);
		}
	}
	else if (type == "4")
	{
		operation = "funbuild";
		try
		{
			string result = apiUtil->fun_build(username, pfn_info.getFunName());
			if (result == "")
			{
				sendResponseMsg(0, "success", operation, request, response);
			}
			else
			{
				sendResponseMsg(1005, result, operation, request, response);
			}
		}
		catch (const std::exception &e)
		{
			string msg = "Function build fail:" + string(e.what());
			sendResponseMsg(1005, msg, operation, request, response);
		}
	}
	else
	{
		sendResponseMsg(1003, "Type is invalid, please look up the api document.", operation, request, response);
	}
}

void fun_review_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, struct PFNInfo &pfn_info)
{
	string operation = "funreview";
	try
	{
		apiUtil->fun_review(username, &pfn_info);
		string content = pfn_info.getFunBody();
		content = Util::urlEncode(content);
		Document all;
		all.SetObject();
		all.AddMember("StatusCode", 0, all.GetAllocator());
		all.AddMember("StatusMsg", "success", all.GetAllocator());
		all.AddMember("Result", StringRef(content.c_str()), all.GetAllocator());
		sendResponseMsg(all, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string msg = "Function review fail:" + string(e.what());
		sendResponseMsg(1005, msg, operation, request, response);
	}
}

void build_PFNInfo(rapidjson::Value &fun_info, struct PFNInfo &pfn_info)
{
	if (fun_info.HasMember("funName") && fun_info["funName"].IsString())
	{
		pfn_info.setFunName(fun_info["funName"].GetString());
	}
	if (fun_info.HasMember("funDesc") && fun_info["funDesc"].IsString())
	{
		pfn_info.setFunDesc(fun_info["funDesc"].GetString());
	}
	if (fun_info.HasMember("funArgs") && fun_info["funArgs"].IsString())
	{
		pfn_info.setFunArgs(fun_info["funArgs"].GetString());
	}
	if (fun_info.HasMember("funBody") && fun_info["funBody"].IsString())
	{
		pfn_info.setFunBody(fun_info["funBody"].GetString());
	}
	if (fun_info.HasMember("funSubs") && fun_info["funSubs"].IsString())
	{
		pfn_info.setFunSubs(fun_info["funSubs"].GetString());
	}
	if (fun_info.HasMember("funStatus") && fun_info["funStatus"].IsString())
	{
		pfn_info.setFunStatus(fun_info["funStatus"].GetString());
	}
	if (fun_info.HasMember("funReturn") && fun_info["funReturn"].IsString())
	{
		pfn_info.setFunReturn(fun_info["funReturn"].GetString());
	}
}