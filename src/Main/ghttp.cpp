/*
 * @Author: liwenjie
 * @Date: 2021-09-23 16:55:53
 * @LastEditTime: 2023-02-16 15:40:12
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
#include "../Server/MultipartParser.hpp"
// db
#include "../Database/Database.h"
#include "../Database/Txn_manager.h"

#include <iostream>
#include <fstream>

#include "../Api/APIUtil.h"
#include "../Api/PFNUtil.h"
#include "../Util/INIParser.h"
#include "../Util/WebUrl.h"
#include "../Util/CompressFileUtil.h"
#include "../Reason/Reason.h"
#include "../Api/HttpUtil.h"

using namespace rapidjson;
using namespace std;
// Added for the json-example:
// using namespace boost::property_tree;

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

#define TEST_IP ""
#define HTTP_TYPE "ghttp"
APIUtil *apiUtil = nullptr;
PFNUtil *pfnUtil = nullptr;
Latch latch;
//! init the ghttp server
int initialize(unsigned short port, std::string db_name, bool load_src);

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

void upload_handler(const HttpServer &server, const shared_ptr<HttpServer::Response> &response, const shared_ptr<HttpServer::Request> &request);

void download_handler(const HttpServer &server, const shared_ptr<HttpServer::Response> &response, const shared_ptr<HttpServer::Request> &request, string request_type);

void signalHandler(int signum);

//=============================================================================

string getRemoteIp(const shared_ptr<HttpServer::Request> &request);

void sendResponseMsg(int code, string msg, std::string operation, const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void sendResponseMsg(rapidjson::Document &doc, std::string operation, const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void build_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string db_path, string remote_ip, string port, string username, string password, string async, string callback);

void load_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string remote_ip, string port, bool load_csr);

void monitor_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string disk);

void unload_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name);

void drop_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string is_backup);

void show_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username);

void userManager_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, string password, string type);

void showuser_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void userPrivilegeManage_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, string privilege,
									string type, string db_name);

void userPassword_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, string password);

void backup_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string backup_path, bool backup_zip, string async, string callback);

void backup_path_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name);

void restore_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string backup_path, string username, string async, string callback);

void query_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string sparql, string format,
					  string update_flag, string log_prefix, string username, string remote_ip);

void export_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string db_path, string username, string compress);

void login_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string remote_ip);

void check_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void begin_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string isolevel, string username);

void tquery_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string TID_s, string sparql);

void commit_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string TID_s);

void rollback_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string TID_s);

void txnlog_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, int page_no, int page_size);

void checkpoint_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name);

void test_connect_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void getCoreVersion_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void batchInsert_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string file, string dir, string async, string callback);

void batchRemove_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string file, string async, string callback);

void querylog_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string date, int page_no, int page_size);

void accesslog_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string date, int page_no, int page_size);

void querylogdate_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void accesslogdate_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void ipmanage_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string type, string ips, string ip_type);

void fun_query_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, struct PFNInfo &pfn_info);

void fun_cudb_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, struct PFNInfo &pfn_info, string type);

void fun_review_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string username, struct PFNInfo &pfn_info);

void rename_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string new_name);

void stat_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response);

void checkOperationState_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string opt_id);


void reasonManage_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string type,
string db_name,Document &document);

std::map<std::string, std::string> parse_post_body(const std::string &body);

int fileSize(const std::string &filepath, size_t *size);

bool start_with(const std::string& str, const std::string& prefix);

std::string fileSuffix(const std::string &filepath);

std::string fileName(const std::string &filepath);

pthread_t scheduler = 0;

int query_num = 0;

std::string _db_home;

std::string _db_suffix;
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
	query_thread_new(request, response, db_name, db_query, format, querytype, log_prefix, username, remote_ip);
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
	task = NULL;
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
	{
		delete *i;
		*i = NULL;
	}	
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
	pfnUtil = new PFNUtil();
 	_db_home = apiUtil->get_Db_path();
	_db_suffix = apiUtil->get_Db_suffix();
	size_t _len_suffix = _db_suffix.length();
	string db_name = "";
	string port_str = apiUtil->get_default_port();
	unsigned short port = 9000;
	bool loadCSR = 0; // DO NOT load CSR by default

	if (argc < 2)
	{
		SLOG_INFO("Use the default port:" + port_str + "!");
		SLOG_INFO("Not load any database!");
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
			cout << "Usage:\tbin/ghttp -p [port] -db [dbname] -c [enable]" << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-db,--database[option],\t\tthe database name.Default value is empty. Notice that the name can not end with "<< _db_suffix<< endl;
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
		if (db_name.length() > _len_suffix && db_name.substr(db_name.length() - _len_suffix, _len_suffix) == _db_suffix)
		{
			cout<<"The database name can not end with " + _db_suffix + "! Input \"bin/ghttp -h\" for help." << endl;
			return -1;
		}
		else if (db_name == "system")
		{
			SLOG_ERROR("The database name can not be system.");
			return -1;
		}
		port_str = Util::getArgValue(argc, argv, "p", "port", port_str);
		port = Util::string2int(port_str);
		loadCSR = Util::string2int(Util::getArgValue(argc, argv, "c", "csr", "0"));
	}
	// check ghttp thread
	std::string processPath = Util::getExactPath(argv[0]);
	std::string currPid = to_string(getpid());
	if (Util::checkProcessExist(processPath, currPid))
	{
		SLOG_INFO("ghttp server already running.");
		return 0;
	}
	// check port
	int max_try = 20;
	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	int bind_return = bind(sock, (struct sockaddr*) &addr,sizeof(addr));
	if (bind_return == -1)
	{
		cout<<"waiting"<<std::flush;
		while (bind_return == -1 && max_try > 0) {
			cout<<"."<<std::flush;
			sleep(3);
			bind_return = bind(sock, (struct sockaddr*) &addr,sizeof(addr));
			max_try --;
		}
		cout<<endl;
		if (bind_return == -1)
		{			
			SLOG_INFO("Server port " + port_str + " is already in use.");
			return -1;
		}
	} 
	max_try = 20;
	// relase bind
	int close_status = close(sock);
	while (close_status != 0 && max_try > 0) {
		close_status = close(sock);
		// shutdown_status = shutdown(sock, SHUT_RDWR);
		if (close_status != 0)
		{
			SLOG_DEBUG("close I/O result:" + to_string(close_status));
			sleep(3);
		}
		max_try --;
	}
	sock = -1;
	std::memset(&addr, 0, sizeof(addr));

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
			int ret = initialize(port, db_name, loadCSR);
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
				string system_port_path = _db_home + "/system" + _db_suffix + "/port.txt";
				if (Util::file_exist(system_port_path))
				{
					Util::remove_path(system_port_path);
				}
				SLOG_WARN("Stopped abnormally, restarting server...");
				latch.lockExclusive();
				if (apiUtil)
				{
					delete apiUtil;
					apiUtil = NULL;
					apiUtil = new APIUtil();
				}
				latch.unlock();
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

int initialize(unsigned short port, std::string db_name, bool load_src)
{	
	// call apiUtil initialized
	if (apiUtil->initialize(HTTP_TYPE, to_string(port), db_name, load_src) == -1)
	{
		return -1;
	}
	// Server restarts to use the original database
	// current_database = NULL;
	HttpServer server;

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

	server.resource["/file/upload"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		upload_handler(server, response, request);
	};

	server.resource["/file/upload"]["OPTIONS"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		cout << "fileupload options request" << endl;
		std::string resJson = "ok";
		*response << "HTTP/1.1 200 OK\r\nContent-Type: text/plain"
			  <<"\r\nAccess-Control-Allow-Methods: POST\r\nAccess-Control-Allow-Origin: *"
			  <<"\r\nContent-Length: " << resJson.length() << "\r\n\r\n"
			  << resJson;
	};

	server.resource["/file/download"]["POST"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		download_handler(server, response, request, "POST");
	};

	server.resource["/file/download"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		download_handler(server, response, request, "GET");
	};

	server.resource["/file/download"]["OPTIONS"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request)
	{
		cout << "filedownload options request" << endl;
		std::string resJson = "ok";
		*response << "HTTP/1.1 200 OK\r\nContent-Type: text/plain"
			  <<"\r\nAccess-Control-Allow-Methods: POST\r\nAccess-Control-Allow-Origin: *"
			  <<"\r\nContent-Length: " << resJson.length() << "\r\n\r\n"
			  << resJson;
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
		SLOG_INFO("ghttp server port " + to_string(server.config.port));
		server.start(); });

	// Wait for server to start so that the client can connect
	this_thread::sleep_for(std::chrono::seconds(1));

	server_thread.join();
	SLOG_INFO("ghttp server stoped.");
	return 0;
}

void signalHandler(int signum)
{
	if (apiUtil)
	{
		delete apiUtil;
		apiUtil = NULL;
	}
	if (pfnUtil)
	{
		delete pfnUtil;
		pfnUtil = NULL;
	}
	SLOG_INFO("ghttp server stopped.");
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
void build_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string db_path, string remote_ip, string port, string username, string password, string async, string callback)
{
	string operation = "build";
	try
	{
		// string result = apiUtil->check_param_value("db_path", db_path);
		// if (result.empty() == false)
		// {
		// 	sendResponseMsg(1003, result, operation, request, response);
		// 	return;
		// }
		if (!db_path.empty()) 
		{
			if (db_path == Util::system_path)
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
		}
		string result = apiUtil->check_param_value("db_name", db_name);
		if (result.empty() == false)
		{
			sendResponseMsg(1003, result, operation, request, response);
			return;
		}
		// check if database named [db_name] is already built
		if (apiUtil->check_db_exist(db_name))
		{
			string error = "Database already built.";
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

		std::vector<std::string> zip_files;
		std::string unz_dir_path;
		std::string file_suffix = fileSuffix(db_path);
		bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
		if (is_zip)
		{
			auto code = CompressUtil::FileHelper::foreachZip(db_path,[](std::string filename)->bool
				{
					if (apiUtil->check_upload_allow_extensions(fileSuffix(filename)) == false)
						return false;
					return true;
				});
			if (code != CompressUtil::UnZipOK)
			{
				string error = "uncompress is failed error.";
				sendResponseMsg(code, error, operation, request, response);
				return;
			}
			std::string file_name = fileName(db_path);
			size_t pos = file_name.size() - file_suffix.size() - 1;
            unz_dir_path = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2();
			mkdir(unz_dir_path.c_str(), 0775);
			CompressUtil::UnCompressZip upfile(db_path, unz_dir_path);
			code = upfile.unCompress();
			if (code != CompressUtil::UnZipOK)
			{
				Util::remove_path(unz_dir_path);
				string error = "uncompress is failed error.";
				sendResponseMsg(code, error, operation, request, response);
				return;
			}
			db_path = upfile.getMaxFilePath();
			upfile.getFileList(zip_files, db_path);
		}
		std::string opt_id = apiUtil->generateUid();
		string remote_ip = getRemoteIp(request);
		string msg = "Operation Success.";
		apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
		auto build_helper = [db_name,username,unz_dir_path,is_zip,zip_files,db_path,operation,opt_id,async,callback]
				(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
				{
					string _db_path = _db_home + "/" + db_name + _db_suffix;
					string dataset = db_path;
					string database = db_name;
					SLOG_DEBUG("Import dataset to build database...");
					SLOG_DEBUG("DB_store: " + database + "\tRDF_data: " + dataset);
					string result;
					shared_ptr<Database> current_database = make_shared<Database>(database);
					bool flag = false;
					if (!dataset.empty())
						flag = current_database->build(dataset);
					else
						flag = current_database->BuildEmptyDB();
					int success_num = current_database->getTripleNum();
					current_database.reset();
					if (flag) 
					{
						// if zip file then excuse batchInsert
						if (is_zip && zip_files.size() > 0)
						{
							current_database = make_shared<Database>(db_name);
							bool rt  = current_database->load(false);
							if (!rt)
							{
								result = "Import RDF file to database failed: load error.";
								Util::remove_path(_db_path);
								if (!unz_dir_path.empty())
								{
									Util::remove_path(unz_dir_path);
								}
								apiUtil->update_access_log(1005, result, opt_id, -1, 0, 0);
								if (async != "true")
									sendResponseMsg(1005, result, operation, request, response);
								current_database.reset();
								return;
							}
							for (std::string rdf_zip : zip_files)
							{
								current_database->batch_insert(rdf_zip, false, nullptr);
							}
							current_database->save();
							success_num = current_database->getTripleNum();

							current_database.reset();
						}
						// init database info and privilege
						if (apiUtil->build_db_user_privilege(db_name, username) 
							&& apiUtil->init_privilege(username, db_name))
						{
							// add success.txt
							ofstream f;
							f.open(_db_path + "/success.txt");
							f.close();
							// add backup.log
							Util::add_backuplog(db_name);
							// build response result
							result = "Import RDF file to database done.";
							string error_log = _db_path + "/parse_error.log";
							size_t parse_error_num = Util::count_lines(error_log);
							// exclude Info line
							if (parse_error_num > 0)
								parse_error_num = parse_error_num - 1;
							if (zip_files.size() > 0)
								parse_error_num = parse_error_num - zip_files.size();
							if (parse_error_num > 0)
							{
								SLOG_ERROR("RDF parse error num " + to_string(parse_error_num));
								SLOG_ERROR("See log file for details " + error_log);
							}
							// remove unzip dir
							if (!unz_dir_path.empty())
							{
								Util::remove_path(unz_dir_path);
							}
							Util::add_backuplog(db_name);
							apiUtil->update_access_log(0, result, opt_id, 1, success_num, parse_error_num);
							if (async != "true")
							{
								rapidjson::Document resp_data;
								resp_data.SetObject();
								rapidjson::Document::AllocatorType &allocator = resp_data.GetAllocator();
								resp_data.AddMember("StatusCode", 0, allocator);
								resp_data.AddMember("StatusMsg", StringRef(result.c_str()), allocator);
								resp_data.AddMember("failed_num", parse_error_num, allocator);
								resp_data.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
								sendResponseMsg(resp_data, operation, request, response);
							}
							if (!callback.empty())
							{
								string postdata;
								string res;
								postdata += "{\"StatusCode\":\"0\",";
								postdata += "\"StatusMsg\":\"" + result + "\",";
								postdata += "\"failed_num\":\"" + std::to_string(parse_error_num) + "\",";
								postdata += "\"opt_id\":\"" + opt_id + "\"}";
								HttpUtil::Post(callback, postdata, res);
							}
							return;
						}
					}
					else
					{
						result = "Import RDF file to database failed.";
						rmdir(_db_path.c_str());
						Util::remove_path(_db_path);
						if (!unz_dir_path.empty())
						{
							Util::remove_path(unz_dir_path);
						}
						apiUtil->update_access_log(1005, result, opt_id, -1, 0, 0);
						if (async != "true")
							sendResponseMsg(1005, result, operation, request, response);
					}
				};
		if (async == "true")
		{
			Document resDoc;
			resDoc.SetObject();
			Document::AllocatorType &allocator = resDoc.GetAllocator();
			resDoc.AddMember("StatusCode", 0, allocator);
			resDoc.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			resDoc.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
			sendResponseMsg(resDoc, operation, request, response);
			thread t(build_helper, nullptr, nullptr);
			t.detach();
		}
		else
		{
			build_helper(request, response);
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
		SLOG_DEBUG("response result:" + resJson);
	}
	else
	{
		SLOG_ERROR("response result:" + resJson);
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
	if (operation != "build" && operation != "batchInsert" && operation != "batchRemove" && operation != "backup" && operation != "restore")
		apiUtil->write_access_log(operation, remote_ip, code, msg);
	StringBuffer resBuffer;
	rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
	doc.Accept(resWriter);
	string json_str = resBuffer.GetString();

	SLOG_DEBUG("response result: " + json_str);
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
void load_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string remote_ip, string port, bool load_csr)
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

		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (current_database == NULL)
		{
			if (!apiUtil->trywrlock_database(db_name))
			{
				error = "Unable to load due to loss of lock.";
				sendResponseMsg(1005, error, operation, request, response);
				return;
			}

			Socket socket;
			shared_ptr<Database> current_database = make_shared<Database>(db_name);
			SLOG_DEBUG("begin loading...");
			bool rt;
			if (!port.empty())
			{
				socket.create();
				socket.connect(remote_ip, Util::string2int(port));
				rt = current_database->load(socket, load_csr);
				string msg = "Load database done.";
				string resJson = CreateJson(0, msg, 0);
				socket.send(resJson);
				socket.close();
			}
			else
			{
				rt = current_database->load(load_csr);
			}
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
void monitor_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string disk)
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
		shared_ptr<DatabaseInfo> database_info;
		apiUtil->get_databaseinfo(db_name, database_info);
		if (apiUtil->rdlock_databaseinfo(database_info) == false)
		{
			string error = "Unable to monitor due to loss of lock";
			sendResponseMsg(1007, error, operation, request, response);
			return;
		}
		string creator = database_info->getCreator();
		string time = database_info->getTime();
		apiUtil->unlock_databaseinfo(database_info);
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (current_database == NULL) {
			current_database = make_shared<Database>(db_name);
			current_database->loadDBInfoFile();
			current_database->loadStatisticsInfoFile();
		}
		unordered_map<string, unsigned long long> umap = current_database->getStatisticsInfo();
		rapidjson::Document doc;
		doc.SetObject();
		rapidjson::Document::AllocatorType &allocator = doc.GetAllocator();
		rapidjson::Value subjectList(kArrayType);
		for (auto &kv : umap)
		{
			rapidjson::Value item(kObjectType);
			item.AddMember("name", rapidjson::Value().SetString(Util::clear_angle_brackets(kv.first).c_str(), allocator), allocator);
			item.AddMember("value", rapidjson::Value().SetUint64(kv.second), allocator);
			subjectList.PushBack(item.Move(), allocator);
		}
		// /use JSON format to send message
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
		unsigned diskUsed = 0;
		if (disk != "0") {
			string db_path = _db_home + "/" + db_name + _db_suffix;
			db_path = Util::getExactPath(db_path.c_str());
			string real_path = Util::getExactPath(db_path.c_str());
			if (!real_path.empty()) {
				long long unsigned count_size_byte = Util::count_dir_size(real_path.c_str());
				// byte to MB
				diskUsed = count_size_byte>>20;
			}
		}
		doc.AddMember("diskUsed", diskUsed, allocator);
		doc.AddMember("subjectList", subjectList, allocator);
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
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			sendResponseMsg(1007, error, operation, request, response);
			return;
		}
		else
		{
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
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			sendResponseMsg(1007, error, operation, request, response);
		}
		else
		{
			if (apiUtil->check_already_load(db_name))
			{
				bool rt = apiUtil->remove_txn_managers(db_name);
				if (!rt)
				{
					apiUtil->unlock_databaseinfo(db_info);
					SLOG_DEBUG("remove " + db_name + " from the txn managers fail.");
					error = "the operation can not been excuted due to can not release txn manager.";
					sendResponseMsg(1005, error, operation, request, response);
					return;
				}
				SLOG_DEBUG("remove " + db_name + " from the txn managers.");
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
			string db_path = _db_home + "/" + db_name + _db_suffix;
			if (is_backup == "false")
			{
				Util::remove_path(db_path);	
				SLOG_DEBUG("delete the file: " + db_path);
			}
			else
			{
				string cmd = "mv " + db_path + " " + _db_home + "/" + db_name + ".bak";
				SLOG_DEBUG("delete the file: " + cmd);
				system(cmd.c_str());
			}
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
		vector<shared_ptr<DatabaseInfo>> array;
		apiUtil->get_already_builds(username, array);
		rapidjson::Document resDoc;
		resDoc.SetObject();
		Document::AllocatorType &allocator = resDoc.GetAllocator();
		size_t count = array.size();
		// string line;
		rapidjson::Value jsonArray(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			shared_ptr<DatabaseInfo> dbInfo = array[i];
			jsonArray.PushBack(dbInfo->toJSON(allocator).Move(), allocator);
		}

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
		vector<shared_ptr<struct DBUserInfo>> userList;
		apiUtil->get_user_info(userList);
		if (userList.empty())
		{
			sendResponseMsg(0, "No Users", operation, request, response);
			return;
		}
		size_t count = userList.size();
		rapidjson::Document resDoc;
		rapidjson::Document::AllocatorType &allocator = resDoc.GetAllocator();
		resDoc.SetObject();
		rapidjson::Value jsonArray(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			shared_ptr<struct DBUserInfo> useInfo = userList[i];
			jsonArray.PushBack(useInfo->toJSON(allocator).Move(), allocator);
		}	
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
			vector<string> privilegeTypes;
			for (unsigned i = 0; i < privileges.size(); i++)
			{
				string temp_privilege_int = privileges[i];
				if (temp_privilege_int.empty())
				{
					continue;
				}
				if (temp_privilege_int == "1")
				{
					privilegeTypes.push_back("query");
				}
				else if (temp_privilege_int == "2")
				{
					privilegeTypes.push_back("load");
				}
				else if (temp_privilege_int == "3")
				{
					privilegeTypes.push_back("unload");
				}
				else if (temp_privilege_int == "4")
				{
					privilegeTypes.push_back("update");
				}
				else if (temp_privilege_int == "5")
				{
					privilegeTypes.push_back("backup");
				}
				else if (temp_privilege_int == "6")
				{
					privilegeTypes.push_back("restore");
				}
				else if (temp_privilege_int == "7")
				{
					privilegeTypes.push_back("export");
				} 
				else
				{
					SLOG_DEBUG("The privilege " + temp_privilege_int + " undefined.");
					continue;
				} 
			}
			if (privilegeTypes.size() > 0)
			{
				string privilegeNames="";
				for (size_t i = 0; i < privilegeTypes.size(); i++)
				{
					if (i > 0) 
					{
						privilegeNames = privilegeNames + ",";
					}
					privilegeNames = privilegeNames + privilegeTypes[i];
				}
				if (type == "1")
				{	
					if (apiUtil->add_privilege(username, privilegeTypes, db_name) == 0)
					{
						result = result + "add privilege " + privilegeNames + " failed.";
						sendResponseMsg(1005, result, operation, request, response);
					}
					else
					{
						result = result + "add privilege " + privilegeNames + " successfully.";
						sendResponseMsg(0, result, operation, request, response);
					}
				}
				else if (type == "2")
				{
					if (apiUtil->del_privilege(username, privilegeTypes, db_name) == 0)
					{
						result = result + "delete privilege " + privilegeNames + " failed.";
						sendResponseMsg(1005, result, operation, request, response);
					}
					else
					{
						result = result + "delete privilege " + privilegeNames + " successfully.";
						sendResponseMsg(0, result, operation, request, response);
					}
				}
				else
				{
					result = "the operation type is not support.";
					sendResponseMsg(1003, result, operation, request, response);
				}
			} 
			else
			{
				result = "not match any valid privilege, valid values between 1 and 7.";
				sendResponseMsg(1003, result, operation, request, response);
			} 
		}
	}
	catch (const std::exception &e)
	{
		string error = "User privilege manage fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * 1:addReason 2:listReason 3:compileReason 4:executeReason 5:disableReason 6.showReason 7.deleteReason
*/
void reasonManage_thread_new(const shared_ptr<HttpServer::Request> &request, 
const shared_ptr<HttpServer::Response> &response, int type, string db_name,Document &document)
{
	string error = "";
	string operation = "reasonManage";
	try
	{
		bool checkresult=apiUtil->check_db_exist(db_name);
		if(checkresult==false)
		{
			error="the database is not exist!";
			SLOG_DEBUG("dbname:"+db_name+",error:"+error);
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		int typeint=type;
	
		if(typeint==1)
		{
			//add reason
		   operation = "AddReasonRule";
		   if(document.HasMember("ruleinfo")==false)
		   {
			 string error2="the data has not the rule information";
			 cout<<"error2:"<<error2<<endl;
			 sendResponseMsg(1005, error2, operation, request, response);
			return;
		   }
           Value reasonInfo=document["ruleinfo"].GetObject();
		   Document::AllocatorType &allocator = document.GetAllocator();
		   std::string createtime = Util::get_date_time();
		   reasonInfo.AddMember("status","新建",allocator);
		   reasonInfo.AddMember("createtime",StringRef(createtime.c_str()),allocator);
		   ReasonOperationResult resultInfo= ReasonHelper::saveReasonRuleInfo(reasonInfo,db_name,_db_home,_db_suffix);
		   if(resultInfo.issuccess==1)
		   {
		   sendResponseMsg(0, resultInfo.error_message, operation, request, response);
		   return;
		   }
		   else
		   {
			sendResponseMsg(1005, resultInfo.error_message, operation, request, response);
			return;
		   }

		}
		else if(typeint==2)
		{
			//listReason
			operation = "listReasonRules";
            string db_path=_db_home+db_name+_db_suffix;
			vector<string> liststr = ReasonHelper::getReasonRuleList(db_path);
			stringstream str_stream;
			str_stream << "[";
            for(int i=0;i<liststr.size();i++)
			{
				if (i > 0) 
					str_stream << ",";
				str_stream << liststr[i];
			}
			str_stream << "]";
			string arrayStr = str_stream.str();
			Document list;
			list.SetArray();
			list.Parse(arrayStr.c_str());
			if (list.HasParseError()) 
			{
				SLOG_ERROR("pasrse rulefiles error:" + arrayStr);
				sendResponseMsg(1005, "pasrse rulefiles error", operation, request, response);
			} 
			else 
			{
				Document doc;
				Document::AllocatorType &allocator = doc.GetAllocator();
				unsigned num = list.Size();
				doc.SetObject();
				doc.AddMember("StatusCode", 0, allocator);
				doc.AddMember("StatusMsg", "ok", allocator);
				doc.AddMember("list", list.Move(), allocator);
				doc.AddMember("num",  num, allocator);
           		sendResponseMsg(doc, operation, request, response);
			}
		}
		else if(typeint==3)
		{
			//compileReason
			operation = "compileReasonRule";
			string rulename = document["rulename"].GetString();
			ReasonSparql resultInfo = ReasonHelper::compileReasonRule(rulename, db_name, _db_home, _db_suffix);
			Document doc;
			doc.SetObject();
			Document::AllocatorType &allocator = doc.GetAllocator();
			if (resultInfo.issuccess == 0)
			{
				sendResponseMsg(1005, resultInfo.error_message, operation, request, response);
				return;
			}

			doc.AddMember("insert_sparql", StringRef(resultInfo.insert_sparql.c_str()), allocator);
			doc.AddMember("delete_sparql", StringRef(resultInfo.delete_sparql.c_str()), allocator);
			doc.AddMember("check_sparql",StringRef(resultInfo.check_sparql.c_str()), allocator);
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "ok", allocator);
			sendResponseMsg(doc, operation, request, response);
			return;
			// // rapidjson::StringBuffer buffer;
			// // rapidjson::Writer<rapidj son::StringBuffer> writer(buffer);

			// // 将Document对象写入StringBuffer，使用PrettyWriter进行格式化
			// if (doc.Accept(writer))
			// {
			// 	// 输出格式化的JSON
				
			// }
			// else
			// {
			// 	error="the result is not json format!";
			// 	sendResponseMsg(1005, error, operation, request, response);
			//     return;
			// }
		}
		else if(typeint==4)
		{
			//executeReason
			operation = "executeReason";
			string rulename=document["rulename"].GetString();
			string username=document["username"].GetString();
			ReasonSparql resultInfo= ReasonHelper::executeReasonRule(rulename,db_name,_db_home,_db_suffix);
			Document doc;
			doc.SetObject();
			bool update_flag_bool = true;
			Document::AllocatorType &allocator = doc.GetAllocator();
			if(resultInfo.issuccess==0)
			{
				sendResponseMsg(1005, resultInfo.error_message, operation, request, response);
			    return;
			}
			//SLOG_DEBUG("insert_sparql:"+resultInfo.insert_sparql);
			doc.AddMember("insert_sparql",StringRef(resultInfo.insert_sparql.c_str()),allocator);
			if(apiUtil->check_db_exist(db_name)==false)
			{
				error="the database is not exist!";
				sendResponseMsg(1005, error, operation, request, response);
			    return;
			}
            shared_ptr<Database> current_database;
			
			// check database load status
			apiUtil->get_database(db_name, current_database);
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
				//throw runtime_error("get current database read lock fail.");
				error="get current database read lock fail.";
				sendResponseMsg(1005, error, operation, request, response);
			    return;
			}
			ResultSet rs;
			int ret_val;
			FILE *output = NULL;
			string sparql = resultInfo.insert_sparql;
			try
			{
				// SLOG_DEBUG("begin query...");
				rs.setUsername(username);
				ret_val = current_database->query(sparql, rs, output, update_flag_bool, false, nullptr);
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
            // cout<<"rs.number:"<<rs.ansNum<<",rs.answer:"<<rs.answer<<endl;
			
			// cout << "ans:" << rs_ansNum << endl;
			doc.AddMember("AnsNum", ret_val, allocator);
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "ok", allocator);
			current_database->save();
			apiUtil->unlock_database(db_name);
			
			//_db.unload();
			//_db.save();
			ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已执行", _db_home, _db_suffix);
			// ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,ret_val,_db_home, _db_suffix,"ok");
			sendResponseMsg(doc, operation, request, response);
			return;
			// rapidjson::StringBuffer buffer;
			// rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

			// // 将Document对象写入StringBuffer，使用PrettyWriter进行格式化
			// if (doc.Accept(writer))
			// {
			// 	// 输出格式化的JSON
			// 	sendResponseMsg(0, buffer.GetString(), operation, request, response);
			// 	return;
			// }
			// else
			// {
			// 	error="the result is not json format!";
			// 	sendResponseMsg(1005, error, operation, request, response);
			//     return;
			// }

		}
		else if(typeint==5)
		{
			operation = "disableReason";
			string rulename=document["rulename"].GetString();
			string username=document["username"].GetString();
			ReasonSparql resultInfo= ReasonHelper::disableReasonRule(rulename,db_name,_db_home,_db_suffix);
			Document doc;
			bool update_flag_bool=true;
			doc.SetObject();
			Document::AllocatorType &allocator = doc.GetAllocator();
			if(resultInfo.issuccess==0)
			{
				sendResponseMsg(1005, resultInfo.error_message, operation, request, response);
			    return;
			}
			
			doc.AddMember("delete_sparql",StringRef(resultInfo.delete_sparql.c_str()),allocator);
			if(apiUtil->check_db_exist(db_name)==false)
			{
				error="the database is not exist!";
				sendResponseMsg(1005, error, operation, request, response);
			    return;
			}
            shared_ptr<Database> current_database;
			
			// check database load status
			apiUtil->get_database(db_name, current_database);
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
				//throw runtime_error("get current database read lock fail.");
				error="get current database read lock fail.";
				sendResponseMsg(1005, error, operation, request, response);
			    return;
			}
			ResultSet rs;
			int ret_val;
			FILE *output = NULL;
			string sparql = resultInfo.delete_sparql;
			try
			{
				// SLOG_DEBUG("begin query...");
				rs.setUsername(username);
				ret_val = current_database->query(sparql, rs, output, update_flag_bool, false, nullptr);
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
			
            // long rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
            //cout << "ans:" << ret_val << endl;
			doc.AddMember("AnsNum",ret_val,allocator);
			
            ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已失效",_db_home,_db_suffix);
			// int effectNum=0-ret_val;
			// ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,effectNum,_db_home,_db_suffix,"ok");
		
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "ok", allocator);
			current_database->save();
			apiUtil->unlock_database(db_name);
			sendResponseMsg(doc, operation, request, response);
			return;

		}
		else if(typeint==6)
		{
		     operation = "showReason";
			string rulename=document["rulename"].GetString();
			ReasonOperationResult resultInfo= ReasonHelper::getReasonInfo(rulename,db_name,_db_home,_db_suffix);
		
			
			if(resultInfo.issuccess==0)
			{
				sendResponseMsg(1005, resultInfo.error_message, operation, request, response);
			    return;
			}
			
			
			else
			{
				// 输出格式化的JSON
				std::string ruleinfo = "{\"ruleinfo\":" + resultInfo.error_message + "}";
				Document doc;
				doc.SetObject();
				doc.Parse(ruleinfo.c_str());
				Document::AllocatorType &allocator = doc.GetAllocator();
				doc.AddMember("StatusCode",0,allocator);
				doc.AddMember("StatusMsg","ok",allocator);
				sendResponseMsg(doc, operation, request, response);
				return;
			}
			
		}
		else if (typeint == 7)
		{
			operation = "deleteReasonRule";
			string rulename = document["rulename"].GetString();
			ReasonOperationResult resultInfo = ReasonHelper::getReasonInfo(rulename, db_name, _db_home, _db_suffix);
			if (resultInfo.issuccess == 0)
			{
				sendResponseMsg(1005, resultInfo.error_message, operation, request, response);
				return;
			}

			else
			{
				// 输出格式化的JSON
				Document doc;
				doc.SetObject();
				doc.Parse(resultInfo.error_message.c_str());
				Document::AllocatorType &allocator = doc.GetAllocator();
				if (doc.HasMember("status")&&doc["status"].GetString() == "已执行")
				{
					string error_msg = "该规则已经执行，请先让该规则失效后再执行";
					sendResponseMsg(1005, error_msg, operation, request, response);
					return;
				}
				
				ReasonOperationResult resultInfo2 = ReasonHelper::removeReasonRule(rulename, db_name, _db_home, _db_suffix);
				if (resultInfo2.issuccess == 1)
				{
					Document doc2;
				    doc2.SetObject();
					doc2.AddMember("StatusCode", 0, allocator);
					doc2.AddMember("StatusMsg", StringRef(resultInfo2.error_message.c_str()), allocator);
					sendResponseMsg(doc2, operation, request, response);
					return;
				}
				else
				{

					sendResponseMsg(1005, resultInfo2.error_message, operation, request, response);
					return;
				}
			}
		}
		else if (typeint == 8)
		{
			operation = "checkReasonRule";
			string rulename=document["rulename"].GetString();
			string username=document["username"].GetString();
			ReasonSparql resultInfo= ReasonHelper::getCheckSparql(rulename,db_name,_db_home,_db_suffix);
			Document doc;
			bool update_flag_bool=true;
			doc.SetObject();
			int effectNum=0;
			string checkMsg="ok";
			Document::AllocatorType &allocator = doc.GetAllocator();
			if(resultInfo.issuccess==0)
			{
				sendResponseMsg(1005, resultInfo.error_message, operation, request, response);
			    return;
			}
			
			doc.AddMember("check_sparql",StringRef(resultInfo.check_sparql.c_str()),allocator);
			if(apiUtil->check_db_exist(db_name)==false)
			{
				error="the database is not exist!";
				sendResponseMsg(1005, error, operation, request, response);
			    return;
			}
            shared_ptr<Database> current_database;
			
			// check database load status
			apiUtil->get_database(db_name, current_database);
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
				//throw runtime_error("get current database read lock fail.");
				error="get current database read lock fail.";
				sendResponseMsg(1005, error, operation, request, response);
			    return;
			}
			ResultSet rs;
			int ret_val;
			FILE *output = NULL;
			string sparql = resultInfo.check_sparql;
			
			try
			{
				// SLOG_DEBUG("begin query...");
				rs.setUsername(username);
				ret_val = current_database->query(sparql, rs, output, update_flag_bool, false, nullptr);
				
			}
			catch (string exception_msg)
			{
				string content = exception_msg;
				apiUtil->unlock_database(db_name);
				sendResponseMsg(1005, content, operation, request, response);
			
				ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
				return;
			}
			catch (const std::runtime_error &e2)
			{
				string content = e2.what();
				apiUtil->unlock_database(db_name);
				sendResponseMsg(1005, content, operation, request, response);
				ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
				return;
			}
			catch (...)
			{
				string content = "unknow error";
				apiUtil->unlock_database(db_name);
				sendResponseMsg(1005, content, operation, request, response);
				ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,0,_db_home,_db_suffix,content);
				return;
			}
			
            // long rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
            string json=rs.to_JSON();
			// SLOG_INFO(json);
			Document doc2;
			doc2.SetObject();
			doc2.Parse(json.c_str());
		
			if(doc2.HasParseError())
			{
				checkMsg="query result is not json format!";
				effectNum=0;
				doc.AddMember("effectNum",effectNum,allocator);
				doc.AddMember("checkMsg",StringRef(checkMsg.c_str()),allocator);
				
			}
			else if(doc2.HasMember("results"))
			{
				Value results=doc2["results"].GetObject();
				if(results.HasMember("bindings"))
				{
					Value bindings=results["bindings"].GetArray();
					if(bindings.Size()>0)
					{
						Value resultobj=bindings[0]["result"].GetObject();
						if(resultobj.HasMember("value"))
						{
							string result_value=resultobj["value"].GetString();
						
							effectNum=Util::string2int(result_value);
							doc.AddMember("effectNum",effectNum,allocator);
							
							
						}
						else
						{
							// effectNum="0";
							effectNum=0;
							doc.AddMember("effectNum",effectNum,allocator);

						}
					}
					else
					{
						effectNum=0;
						doc.AddMember("effectNum",effectNum,allocator);
					}
				}
				doc.AddMember("checkMsg",StringRef(checkMsg.c_str()),allocator);
			}
            // cout << "ans:" << ret_val << endl;
		
			
             ReasonHelper::updateReasonRuleStatus(rulename, db_name, "已校验",_db_home,_db_suffix);
			// int effectNum=0-ret_val;
            ReasonHelper::updateReasonRuleEffectNum(rulename,db_name,effectNum,_db_home,_db_suffix,checkMsg);
		
			doc.AddMember("StatusCode", 0, allocator);
			doc.AddMember("StatusMsg", "ok", allocator);
			current_database->save();
			apiUtil->unlock_database(db_name);
			sendResponseMsg(doc, operation, request, response);
			return;
		}
		else
		{
           error = "the operation type is not support!";
		  sendResponseMsg(1005, error, operation, request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Reason manage fail:" + string(e.what());
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
void backup_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string backup_path, bool backup_zip, string async, string callback)
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
		shared_ptr<Database> current_db;
		apiUtil->get_database(db_name, current_db);
		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "the operation can not been excuted due to loss of lock.";
			sendResponseMsg(1007, error, operation, request, response);
			return;
		}
		// begin backup database
		string path = backup_path;
		string default_backup_path = Util::backup_path;
		if (path.empty())
		{
			path = default_backup_path;
			SLOG_DEBUG("backup_path is empty, set to default path: " + path);
		}
		if (path == "." || Util::getExactPath(path.c_str()) == Util::getExactPath(_db_home.c_str()))
		{
			error = "Failed to backup the database. Backup path can not be root or \"" + _db_home + "\".";
			apiUtil->unlock_databaseinfo(db_info);
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}

		std::string opt_id = apiUtil->generateUid();
		string remote_ip = getRemoteIp(request);
		string msg = "Operation Success.";
		apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
		string temp_path = path;
		auto backup_helper = [db_name,operation,opt_id,&current_db,&db_info,temp_path,default_backup_path,backup_zip,async,callback]
			(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
			{
				string path = temp_path;
				std::string error = "";
				bool flag = false;
				if (current_db)
				{
					flag = current_db->backup();
				}
				else
				{
					Database _db(db_name);
					flag = _db.backup();
				}
				if(flag == false)
				{
					error = "Failed to backup the database.";
					apiUtil->unlock_databaseinfo(db_info);
					apiUtil->update_access_log(1005, error, opt_id, -1, 0, 0);
					if (async != "true")
						sendResponseMsg(1005, error, operation, request, response);
					return;
				}
				else
				{
					string timestamp = Util::get_timestamp();
					string new_folder =  db_name + _db_suffix + "_" + timestamp;
					string sys_cmd, _path, backup_store_path;
					Util::string_suffix(path, '/');
					_path = path + new_folder;
					backup_store_path = default_backup_path + "/" + db_name + _db_suffix;
					if (backup_zip)
					{
						_path = _path + ".zip";
						CompressUtil::CompressZip compress_dir;
						if (!compress_dir.compressDirExportZip(backup_store_path, _path))
						{
							error = "Failed to backup compress the database.";
							apiUtil->unlock_databaseinfo(db_info);
							apiUtil->update_access_log(1005, error, opt_id, -1, 0, 0);
							if (async != "true")
								sendResponseMsg(1005, error, operation, request, response);
							return;
						}
						Util::remove_path(backup_store_path);
					}
					else
					{
						sys_cmd = "mv " + backup_store_path + " " + _path;
						system(sys_cmd.c_str());
					}
					vector<string> files;
					Util::dir_files(path, "", files);
					int max_backups = atoi(Util::getConfigureValue("max_backups").c_str());
					std::string db_file_suffix = db_name + _db_suffix;
					int db_file_suffix_size = db_file_suffix.size();
					std::string db_file_min;
					unsigned int backup_num = 0;
					for (auto& file_name : files)
					{
						if (file_name.substr(0, db_file_suffix_size) == db_file_suffix)
						{
							if (db_file_min.empty() || file_name < db_file_min)
								db_file_min = file_name;
							backup_num++;
						}
					}
					if (backup_num > max_backups)
					{
						Util::remove_path(path + db_file_min);
					}

					SLOG_DEBUG("database backup done: " + db_name);
					string success = "Database backup successfully.";
					// current_db = NULL;
					apiUtil->unlock_databaseinfo(db_info);
					apiUtil->update_access_log(0, success, opt_id, 1, 0, 0, _path);

					if (async != "true")
					{
						Document resDoc;
						resDoc.SetObject();
						Document::AllocatorType &allocator = resDoc.GetAllocator();
						resDoc.AddMember("StatusCode", 0, allocator);
						resDoc.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
						resDoc.AddMember("backupfilepath", StringRef(_path.c_str()), allocator);
						resDoc.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
						sendResponseMsg(resDoc, operation, request, response);
					}
					if (!callback.empty())
					{
						string postdata;
						string res;
						postdata += "{\"StatusCode\":\"0\",";
						postdata += "\"StatusMsg\":\"" + success + "\",";
						postdata += "\"backupfilepath\":\"" + _path + "\",";
						postdata += "\"opt_id\":\"" + opt_id + "\"}";
						HttpUtil::Post(callback, postdata, res);
					}
				}
			};
		if (async == "true")
		{
			Document resDoc;
			resDoc.SetObject();
			Document::AllocatorType &allocator = resDoc.GetAllocator();
			resDoc.AddMember("StatusCode", 0, allocator);
			resDoc.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			resDoc.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
			sendResponseMsg(resDoc, operation, request, response);

			thread t(backup_helper, nullptr, nullptr);
			t.detach();
		}
		else
		{
			backup_helper(request, response);
		}
	}
	catch (const std::exception &e)
	{
		string error = "Backup fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}

/**
 * @description:query backup path
 * @Author:wangjian
 * @param {const} shared_ptr
 * @param {string} db_name the operation database name
 */
void backup_path_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name)
{
	string operation = "backuppath";
	try
	{
		string error = "";
		error = apiUtil->check_param_value("db_name", db_name);

		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		std::vector<std::string> file_list;
		string backup_path = Util::backup_path;
		Util::dir_files(backup_path, db_name, file_list);
		Document resDoc;
		Document pathsDoc;
		resDoc.SetObject();
		pathsDoc.SetArray();
		Document::AllocatorType &allocator = resDoc.GetAllocator();
		for (size_t i = 0; i < file_list.size(); i++)
		{
			pathsDoc.PushBack(Value().SetString((backup_path + file_list[i]).c_str(), allocator), allocator);
		}
		resDoc.AddMember("StatusCode", 0, allocator);
		resDoc.AddMember("StatusMsg", "success", allocator);
		resDoc.AddMember("paths", pathsDoc, allocator);
		sendResponseMsg(resDoc, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "query backup path fail:" + string(e.what());
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
void restore_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string backup_path, string username, string async, string callback)
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
		bool is_zip = false;
		string path = backup_path;
		if (fileSuffix(backup_path) == "zip")
		{
			is_zip = true;
			if (Util::file_exist(path) == false)
			{
				string error = "Backup path not exist, restore failed.";
				sendResponseMsg(1003, error, operation, request, response);
				return;
			}
			path = path.substr(0, path.length() - 4);
		}
		else
		{
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
		}
		// check load status: need unload if already load
		if (apiUtil->check_already_load(db_name))
		{
			string error = "Database alreay load, need unload it first.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		SLOG_DEBUG("restore database:" + db_name);
		if (apiUtil->check_already_build(db_name) == false)
		{
			SLOG_DEBUG("Database not built yet, rebuild now.");
			string time = Util::get_backup_time(path, db_name);
			if (time.size() == 0)
			{
				error = "Backup path does not match database name, restore failed";
				sendResponseMsg(1003, error, operation, request, response);
				return;
			}
			if (apiUtil->init_privilege(username, db_name) == 0)
			{
				error = "init privilege failed.";
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

		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			string error = "Unable to restore due to loss of lock";
			sendResponseMsg(1007, error, operation, request, response);
			return;
		}

		std::string opt_id = apiUtil->generateUid();
		string remote_ip = getRemoteIp(request);
		string msg = "Operation Success.";
		apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
		auto restore_helper = [db_name,operation,opt_id,is_zip,backup_path,&db_info,path,async,callback]
			(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
			{
				// TODO why need lock the database_map?
				// apiUtil->trywrlock_database_map();
				int ret = 0;
				if (is_zip)
				{
					if (Util::dir_exist(path))
						Util::remove_path(path);
					mkdir(path.c_str(), 0775);
					CompressUtil::UnCompressZip unzip(backup_path, path);
					if (unzip.unCompress() != CompressUtil::UnZipOK)
					{
						Util::remove_path(path);
						string error = "backup compress fail";
						apiUtil->update_access_log(1003, error, opt_id, -1, 0, 0);
						if (async != "true")
							sendResponseMsg(1003, error, operation, request, response);
						return;
					}
					ret = apiUtil->db_copy(path, _db_home);
					Util::remove_path(path);
				}
				else
				{
					ret = apiUtil->db_copy(path, _db_home);
				}
				// apiUtil->unlock_database_map();
				// copy failed
				if (ret == 1)
				{
					string error = "Failed to restore the database. Backup path error";
					apiUtil->unlock_databaseinfo(db_info);
					apiUtil->update_access_log(1005, error, opt_id, -1, 0, 0);
					if (async != "true")
						sendResponseMsg(1005, error, operation, request, response);
				}
				else
				{
					// remove old folder
					string db_path = _db_home + "/" + db_name + _db_suffix;
					Util::remove_path(db_path);
					// mv backup folder to database folder
					string folder_name = Util::get_folder_name(path, db_name);
					string sys_cmd = "mv " + _db_home + "/" + folder_name + " " + db_path;
					std::system(sys_cmd.c_str());
					apiUtil->unlock_databaseinfo(db_info);

					string success = "Database " + db_name + " restore successfully.";
					apiUtil->update_access_log(0, success, opt_id, 1, 0, 0);
					if (async != "true")
					{
						Document resDoc;
						resDoc.SetObject();
						Document::AllocatorType &allocator = resDoc.GetAllocator();
						resDoc.AddMember("StatusCode", 0, allocator);
						resDoc.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
						resDoc.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
						sendResponseMsg(resDoc, operation, request, response);
					}
					if (!callback.empty())
					{
						string postdata;
						string res;
						postdata += "{\"StatusCode\":\"0\",";
						postdata += "\"StatusMsg\":\"" + success + "\",";
						postdata += "\"opt_id\":\"" + opt_id + "\"}";
						HttpUtil::Post(callback, postdata, res);
					}
				}
			};
		if (async == "true")
		{
			Document resDoc;
			resDoc.SetObject();
			Document::AllocatorType &allocator = resDoc.GetAllocator();
			resDoc.AddMember("StatusCode", 0, allocator);
			resDoc.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
			resDoc.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
			sendResponseMsg(resDoc, operation, request, response);

			thread t(restore_helper, nullptr, nullptr);
			t.detach();
		}
		else
		{
			restore_helper(request, response);
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
					  string update_flag, string log_prefix, string username, string remote_ip)
{
	string error = "";
	string operation = "query";
	try
	{
		std::string min_memory = Util::getConfigureValue("min_memory").c_str();
		int memoryLeft = Util::memoryLeft();
		if (memoryLeft < atoi(min_memory.c_str()))
		{
			error = "memory not enough, available:" + std::to_string(memoryLeft) + "GB, need minimum:" + min_memory + "GB";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
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
		shared_ptr<Database> current_database;
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
			apiUtil->get_database(db_name, current_database);
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
					error = "parse error";
					string filename2 = "error_" + filename;
					string localname2 = apiUtil->get_query_result_path() + filename2;
					outfile.open(localname2);
					outfile << success;
					outfile.close();
					SLOG_ERROR("result parse error: ErrorCode=" + to_string(resDoc.GetParseError()) 
							+ ", ErrorPosition=" + to_string(resDoc.GetErrorOffset()) + ", ResultFile=" + localname2);
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
					SLOG_DEBUG("response result:\n" << resJson);
					auto content = request->header.find("Accept-Encoding");
					if (content != request->header.end())
					{
						std::string accept_encoding = content->second;
						if (accept_encoding.find("gzip") != std::string::npos)
						{
							char* compress_ = (char*)malloc(resJson.size());
							if (compress_ != nullptr)
							{
								size_t compress_size = 0;
								int status = CompressUtil::GzipHelper::compress(&resJson, compress_, compress_size);
								if (status == 0)
								{
									*response << "HTTP/1.1 200 OK"
										<< "\r\nContent-Type: application/json"
										<< "\r\nContent-Length: " << compress_size
										<< "\r\nCache-Control: no-cache"
										<< "\r\nPragma: no-cache"
										<< "\r\nExpires: 0"
										<< "\r\nContent-Encoding: gzip"
										<< "\r\n\r\n";
									char buffer;
									for (size_t i = 0; i < compress_size; ++i)
									{
										buffer = compress_[i];
										*response << buffer;
									}
									free(compress_);
									apiUtil->unlock_database(db_name);
									SLOG_DEBUG("query complete!");
									return;
								}
								else
								{
									free(compress_);
								}
							}
						}
					}
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
				rapidjson::Writer<rapidjson::StringBuffer> writer(s);
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
				SLOG_DEBUG("response result:\n" << resJson);
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
				resDoc.SetObject();
				Document::AllocatorType &allocator = resDoc.GetAllocator();
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
					Writer<StringBuffer> resWriter(resBuffer);
					resDoc.Accept(resWriter);
					string resJson = resBuffer.GetString();
					SLOG_DEBUG("response result:\n" << resJson);
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
			if (update)
			{
				SLOG_DEBUG("update query returns true. update num " + to_string(ret_val));
				Document resDoc;
				resDoc.SetObject();
				Document::AllocatorType &allocator = resDoc.GetAllocator();
				resDoc.AddMember("StatusCode", 0, allocator);
				resDoc.AddMember("StatusMsg", "update query returns true.", allocator);
				resDoc.AddMember("AnsNum", ret_val, allocator);
				resDoc.AddMember("QueryTime", query_time, allocator);
				rapidjson::StringBuffer resBuffer;
				rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
				resDoc.Accept(resWriter);
				string resJson = resBuffer.GetString();
				SLOG_DEBUG("response result:\n" << resJson);
				*response << "HTTP/1.1 200 OK"
							<< "\r\nContent-Type: application/json"
							<< "\r\nContent-Length: " << resJson.length()
							<< "\r\nCache-Control: no-cache"
							<< "\r\nPragma: no-cache"
							<< "\r\nExpires: 0"
							<< "\r\n\r\n"
							<< resJson;
			}
			else
			{
				SLOG_DEBUG(log_prefix + "search query returned error.");
				std::string error = "search query returns false.";
				sendResponseMsg(1005, error, operation, request, response);
			}
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
void export_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string db_path, string username, string compress)
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
		// check if database named [db_name] is already load
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (current_database == NULL)
		{
			string error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		Util::string_suffix(db_path, '/');
		if (Util::dir_exist(db_path) == false)
		{
			Util::create_dirs(db_path);
		}
		std::string zip_path;
		if (compress=="0")
			db_path = db_path + db_name + "_" + Util::get_timestamp() + ".nt";
		else
		{
			zip_path = db_path + db_name + "_" + Util::get_timestamp() + ".zip";
			db_path = db_name + "_" + Util::get_timestamp() + ".nt";
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
		if (compress=="0")
		{
			resDoc.AddMember("filepath", StringRef(db_path.c_str()), allocator);
			sendResponseMsg(resDoc, operation, request, response);
		}
		else
		{
			if (!CompressUtil::FileHelper::compressExportZip(db_path, zip_path))
			{
				error = "export compress fail.";
				sendResponseMsg(1005, error, operation, request, response);
				Util::remove_path(zip_path);
				return;
			}
			resDoc.AddMember("filepath", StringRef(zip_path.c_str()), allocator);
			sendResponseMsg(resDoc, operation, request, response);
			Util::remove_path(db_path);
		}
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
void login_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string remote_ip)
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
		string product_name = Util::getConfigureValue("product_name");
		std::transform(product_name.begin(), product_name.end(), product_name.begin(), ::tolower);
		if (product_name != "gstore") {
			licensetype = "";
		}
		resDoc.AddMember("licensetype", StringRef(licensetype.c_str()), allocator);
		string cur_path = Util::get_cur_path();
		resDoc.AddMember("RootPath", StringRef(cur_path.c_str()), allocator);
		resDoc.AddMember("type", HTTP_TYPE, allocator);
		apiUtil->reset_access_ip_error_num(remote_ip);
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
		shared_ptr<Txn_manager> txn_m;
		apiUtil->get_Txn_ptr(db_name, txn_m);
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
			apiUtil->aborted_process(txn_m, TID);
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
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (current_database == NULL)
		{
			error = "Database not load yet.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		shared_ptr<Txn_manager> txn_m;
		apiUtil->get_Txn_ptr(db_name, txn_m);
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
		shared_ptr<Txn_manager> txn_m;
		apiUtil->get_Txn_ptr(db_name, txn_m);
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
		rapidjson::Document all;
		all.SetObject();
		rapidjson::Document::AllocatorType &allocator = all.GetAllocator();
		rapidjson::Value jsonArray(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			TransactionLogInfo log_info = logList[i];
			jsonArray.PushBack(log_info.toJSON(allocator).Move(), allocator);
		}

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
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
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
			shared_ptr<Txn_manager> txn_m;
			apiUtil->get_Txn_ptr(db_name, txn_m);
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
		string product_name = Util::getConfigureValue("product_name");
		std::transform(product_name.begin(), product_name.end(), product_name.begin(), ::tolower);
		if (product_name != "gstore") {
			licensetype = "";
		}
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
void batchInsert_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string file, string dir, string async, string callback)
{
	string error;
	string operation = "batchInsert";
	bool is_file = true;
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
			is_file = false;
			error = apiUtil->check_param_value("dir", dir);
			if (error.empty() == false)
			{
				error = "file and dir cannot be empty at the same time!";
				sendResponseMsg(1003, error, operation, request, response);
				return;
			}
		}
		if (is_file && Util::file_exist(file) == false)
		{
			error = "The data file is not exist";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		if (!is_file && Util::dir_exist(dir) == false )
		{
			error = "The data directory is not exist";
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
		std::vector<std::string> zip_files;
		std::string unz_dir_path;
		std::string file_suffix = fileSuffix(file);
		bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
		if (is_zip)
		{
			auto code = CompressUtil::FileHelper::foreachZip(file,[](std::string filename)->bool
				{
					if (apiUtil->check_upload_allow_extensions(fileSuffix(filename)) == false)
						return false;
					return true;
				});
			if (code != CompressUtil::UnZipOK)
			{
				string error = "uncompress is failed error.";
				sendResponseMsg(code, error, operation, request, response);
				return;
			}
			std::string file_name = fileName(file);
			size_t pos = file_name.size() - file_suffix.size() - 1;
            unz_dir_path = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2();
			mkdir(unz_dir_path.c_str(), 0775);
			CompressUtil::UnCompressZip upfile(file, unz_dir_path);
			code = upfile.unCompress();
			if (code != CompressUtil::UnZipOK)
			{
				Util::remove_path(unz_dir_path);
				string error = "uncompress is failed error.";
				sendResponseMsg(code, error, operation, request, response);
				return;
			}
			upfile.getFileList(zip_files, "");
		}
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			sendResponseMsg(1004, error, operation, request, response);
			if (!unz_dir_path.empty())
			{
				Util::remove_path(unz_dir_path);
			}
		}
		else
		{
			std::string opt_id = apiUtil->generateUid();
			string remote_ip = getRemoteIp(request);
			string msg = "Operation Success.";
			apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
			string _dir = dir;
			auto insert_helper = [operation,opt_id,db_name,file,_dir,is_file,is_zip,zip_files,unz_dir_path,async,callback](const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
				{
					shared_ptr<Database> current_database;
					apiUtil->get_database(db_name, current_database);
					unsigned success_num = 0;
					unsigned total_num = 0;
					unsigned parse_error_num = 0;
					string error_log = _db_home +  "/" + db_name + _db_suffix + "/parse_error.log";
					if (is_file)
					{
						if (!is_zip)
						{
							total_num = Util::count_lines(error_log);
							success_num = current_database->batch_insert(file, false, nullptr);
							// exclude Info line
							parse_error_num = Util::count_lines(error_log) - total_num - 1;
						}
						else
						{
							total_num = Util::count_lines(error_log);
							for (std::string rdf_zip : zip_files)
							{
								SLOG_DEBUG("begin insert data from " + rdf_zip);
								success_num += current_database->batch_insert(rdf_zip, false, nullptr);
							}
							parse_error_num = Util::count_lines(error_log) - total_num - zip_files.size();
						}
					}
					else
					{
						vector<string> files;
						string dir = _dir;
						Util::string_suffix(dir, '/');
						Util::dir_files(dir, "", files);
						total_num = Util::count_lines(error_log);
						for (string rdf_file : files)
						{
							SLOG_DEBUG("begin insert data from " + dir + rdf_file);
							success_num += current_database->batch_insert(dir + rdf_file, false, nullptr);
						}
						// exclude Info line
						parse_error_num = Util::count_lines(error_log) - total_num - files.size();
					}
					current_database->save();
					string success = "Batch insert data successfully.";
					apiUtil->unlock_database(db_name);
					if (!unz_dir_path.empty())
					{
						Util::remove_path(unz_dir_path);
					}
					apiUtil->update_access_log(0, success, opt_id, 1, success_num, parse_error_num);
					if (async != "true")
					{
						Document resDoc;
						resDoc.SetObject();
						Document::AllocatorType &allocator = resDoc.GetAllocator();
						resDoc.AddMember("StatusCode", 0, allocator);
						resDoc.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
						resDoc.AddMember("success_num", success_num, allocator);
						resDoc.AddMember("failed_num", parse_error_num, allocator);
						resDoc.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
						sendResponseMsg(resDoc, operation, request, response);
					}
					if (!callback.empty())
					{
						string postdata;
						string res;
						postdata += "{\"StatusCode\":\"0\",";
						postdata += "\"StatusMsg\":\"" + success + "\",";
						postdata += "\"success_num\":\"" + std::to_string(success_num) + "\",";
						postdata += "\"failed_num\":\"" + std::to_string(parse_error_num) + "\",";
						postdata += "\"opt_id\":\"" + opt_id + "\"}";
						HttpUtil::Post(callback, postdata, res);
					}
				};
			if (async == "true")
			{
				Document resDoc;
				resDoc.SetObject();
				Document::AllocatorType &allocator = resDoc.GetAllocator();
				resDoc.AddMember("StatusCode", 0, allocator);
				resDoc.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
				resDoc.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
				sendResponseMsg(resDoc, operation, request, response);
				thread t(insert_helper, nullptr, nullptr);
				t.detach();
			}
			else
			{
				insert_helper(request, response);
			}
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
void batchRemove_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string file, string async, string callback)
{
	string error;
	string operation = "batchRemove";
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
		shared_ptr<Database> current_database;
		apiUtil->get_database(db_name, current_database);
		if (apiUtil->trywrlock_database(db_name) == false)
		{
			error = "The operation can not been excuted due to loss of lock.";
			sendResponseMsg(1007, error, operation, request, response);
		}
		else
		{
			std::string opt_id = apiUtil->generateUid();
			string remote_ip = getRemoteIp(request);
			string msg = "Operation Success.";
			apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
			auto remove_helper = [db_name,operation,file,opt_id,async,callback]
				(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
				{
					shared_ptr<Database> current_database;
					apiUtil->get_database(db_name, current_database);
					string success = "Batch remove data successfully.";
					int success_num = current_database->batch_remove(file, false, nullptr);
					current_database->save();
					apiUtil->unlock_database(db_name);
					apiUtil->update_access_log(0, success, opt_id, 1, success_num, 0);
					if (async != "true")
					{
						Document resDoc;
						resDoc.SetObject();
						Document::AllocatorType &allocator = resDoc.GetAllocator();
						resDoc.AddMember("StatusCode", 0, allocator);
						resDoc.AddMember("StatusMsg", StringRef(success.c_str()), allocator);
						resDoc.AddMember("success_num", success_num, allocator);
						resDoc.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
						sendResponseMsg(resDoc, operation, request, response);
					}
					if (!callback.empty())
					{
						string postdata;
						string res;
						postdata += "{\"StatusCode\":\"0\",";
						postdata += "\"StatusMsg\":\"" + success + "\",";
						postdata += "\"success_num\":\"" + std::to_string(success_num) + "\",";
						postdata += "\"opt_id\":\"" + opt_id + "\"}";
						HttpUtil::Post(callback, postdata, res);
					}
				};
			if (async == "true")
			{
				Document resDoc;
				resDoc.SetObject();
				Document::AllocatorType &allocator = resDoc.GetAllocator();
				resDoc.AddMember("StatusCode", 0, allocator);
				resDoc.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
				resDoc.AddMember("opt_id", StringRef(opt_id.c_str()), allocator);
				sendResponseMsg(resDoc, operation, request, response);
				thread t(remove_helper, nullptr, nullptr);
				t.detach();
			}
			else
			{
				remove_helper(request, response);
			}
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
	string ipCheckResult = apiUtil->check_access_ip(remote_ip, 1);
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
		if (url.find("operation") == string::npos)
		{
			sendResponseMsg(14, "GET " + url, "unknown", request, response);
			return;
		}
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
	if (operation != "login" && operation != "testConnect")
	{	
		ipCheckResult = apiUtil->check_access_ip(remote_ip, 2);
		if (!ipCheckResult.empty())
		{
			sendResponseMsg(1101, ipCheckResult, "ipcheck", request, response);
			return;
		}
	}
	string ss;
	ss += "\n------------------------ ghttp-api ------------------------";
	ss += "\nthread_id: " + thread_id;
	ss += "\nremote_ip: " + remote_ip;
	ss += "\noperation: " + operation;
	ss += "\nmethod: " + request_type;
	ss += "\nrequest_path: " + request->path;
	ss += "\nhttp_version: " + request->http_version;
	ss += "\nrequest_time: " + Util::get_date_time();
	ss += "\n----------------------------------------------------------";
	SLOG_DEBUG(ss);
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
		string async = "";
		string callback = "";
		try
		{
			if (request_type == "GET")
			{
				db_path = WebUrl::CutParam(url, "db_path");
				port = WebUrl::CutParam(url, "port");
				db_path = UrlDecode(db_path);
				port = UrlDecode(port);
				async = WebUrl::CutParam(url, "async");
				async = UrlDecode(async);
				callback = WebUrl::CutParam(url, "callback");
				callback = UrlDecode(callback);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("db_path") && document["db_path"].IsString())
				{
					db_path = document["db_path"].GetString();
				}
				if (document.HasMember("port") && document["port"].IsString())
				{
					port = document["port"].GetString();
				}
				if (document.HasMember("async") && document["async"].IsString())
				{
					async = document["async"].GetString();
				}
				if (document.HasMember("callback") && document["callback"].IsString())
				{
					callback = document["callback"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		build_thread_new(request, response, db_name, db_path, remote_ip, port, username, password, async, callback);
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
				port = WebUrl::CutParam(url, "port");
				loadCSRStr = UrlDecode(loadCSRStr);
				port = UrlDecode(port);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("csr") && document["csr"].IsString())
				{
					loadCSRStr = document["csr"].GetString();
				}
				if (document.HasMember("port") && document["port"].IsString())
				{
					port = document["port"].GetString();
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
		load_thread_new(request, response, db_name, remote_ip, port, load_csr);
	}
	// monitor database
	else if (operation == "monitor")
	{
		string disk = "1";
		try
		{
			if (request_type == "GET")
			{
				disk = WebUrl::CutParam(url, "disk");
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("disk") && document["disk"].IsString())
				{
					disk = document["disk"].GetString();
				}
			}
		}
		catch (...)
		{

		}
		monitor_thread_new(request, response, db_name, disk);
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
		string compress_zip = "";
		string async = "";
		string callback = "";
		try
		{
			if (request_type == "GET")
			{
				backup_path = WebUrl::CutParam(url, "backup_path");
				backup_path = UrlDecode(backup_path);
				compress_zip = WebUrl::CutParam(url, "backup_zip");
				compress_zip = UrlDecode(compress_zip);
				async = WebUrl::CutParam(url, "async");
				async = UrlDecode(async);
				callback = WebUrl::CutParam(url, "callback");
				callback = UrlDecode(callback);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("backup_path") && document["backup_path"].IsString())
				{
					backup_path = document["backup_path"].GetString();
				}
				if (document.HasMember("backup_zip") && document["backup_zip"].IsString())
				{
					compress_zip = document["backup_zip"].GetString();
				}
				if (document.HasMember("async") && document["async"].IsString())
				{
					async = document["async"].GetString();
				}
				if (document.HasMember("callback") && document["callback"].IsString())
				{
					callback = document["callback"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		bool backup_zip = false;
		if (compress_zip == "true")
			backup_zip = true;
		backup_thread_new(request, response, db_name, backup_path, backup_zip, async, callback);
	}
	// query backup path
	else if (operation == "backuppath")
	{
		backup_path_thread_new(request, response, db_name);
	}
	// restore database
	else if (operation == "restore")
	{
		string backup_path = "";
		string async = "";
		string callback = "";
		try
		{
			if (request_type == "GET")
			{
				backup_path = WebUrl::CutParam(url, "backup_path");
				backup_path = UrlDecode(backup_path);
				async = WebUrl::CutParam(url, "async");
				async = UrlDecode(async);
				callback = WebUrl::CutParam(url, "callback");
				callback = UrlDecode(callback);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("backup_path") && document["backup_path"].IsString())
				{
					backup_path = document["backup_path"].GetString();
				}
				if (document.HasMember("async") && document["async"].IsString())
				{
					async = document["async"].GetString();
				}
				if (document.HasMember("callback") && document["callback"].IsString())
				{
					callback = document["callback"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		restore_thread_new(request, response, db_name, backup_path, username, async, callback);
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
		string compress = "0";
		try
		{
			if (request_type == "GET")
			{
				db_path = WebUrl::CutParam(url, "db_path");
				db_path = UrlDecode(db_path);
				compress = WebUrl::CutParam(url, "compress");
				compress = UrlDecode(compress);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("db_path") && document["db_path"].IsString())
				{
					db_path = document["db_path"].GetString();
				}
				if (document.HasMember("compress") && document["compress"].IsString())
				{
					compress = document["compress"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		export_thread_new(request, response, db_name, db_path, username, compress);
	}
	else if (operation == "login")
	{
		login_thread_new(request, response, remote_ip);
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
		string dir = "";
		string async = "";
		string callback = "";
		try
		{
			if (request_type == "GET")
			{
				file = WebUrl::CutParam(url, "file");
				file = UrlDecode(file);
				dir = WebUrl::CutParam(url, "dir");
				dir = UrlDecode(dir);
				async = WebUrl::CutParam(url, "async");
				async = UrlDecode(async);
				callback = WebUrl::CutParam(url, "callback");
				callback = UrlDecode(callback);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("file") && document["file"].IsString())
				{
					file = document["file"].GetString();
				}
				if (document.HasMember("dir") && document["dir"].IsString())
				{
					dir = document["dir"].GetString();
				}
				if (document.HasMember("async") && document["async"].IsString())
				{
					async = document["async"].GetString();
				}
				if (document.HasMember("callback") && document["callback"].IsString())
				{
					callback = document["callback"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		batchInsert_thread_new(request, response, db_name, file, dir, async, callback);
	}
	else if (operation == "batchRemove")
	{
		string file = "";
		string async = "";
		string callback = "";
		try
		{
			if (request_type == "GET")
			{
				file = WebUrl::CutParam(url, "file");
				file = UrlDecode(file);
				async = WebUrl::CutParam(url, "async");
				async = UrlDecode(async);
				callback = WebUrl::CutParam(url, "callback");
				callback = UrlDecode(callback);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("file") && document["file"].IsString())
				{
					file = document["file"].GetString();
				}
				if (document.HasMember("async") && document["async"].IsString())
				{
					async = document["async"].GetString();
				}
				if (document.HasMember("callback") && document["callback"].IsString())
				{
					callback = document["callback"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		batchRemove_thread_new(request, response, db_name, file, async, callback);
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
					pfnUtil->build_PFNInfo(fun_info, &pfn_info);
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
					pfnUtil->build_PFNInfo(fun_info, &pfn_info);
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
					pfnUtil->build_PFNInfo(fun_info, &pfn_info);
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
	else if (operation == "rename")
	{
		string new_name = "";
		try
		{
			if (request_type == "GET")
			{
				new_name = WebUrl::CutParam(url, "new_name");
				new_name = UrlDecode(new_name);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("new_name") && document["new_name"].IsString())
				{
					new_name = document["new_name"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		rename_thread_new(request, response, db_name, new_name);
	}
	else if (operation == "stat")
	{
		stat_thread_new(request, response);
	}
	else if (operation == "checkOperationState")
	{
		string opt_id = "";
		try
		{
			if (request_type == "GET")
			{
				opt_id = WebUrl::CutParam(url, "opt_id");
				opt_id = UrlDecode(opt_id);
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("opt_id") && document["opt_id"].IsString())
				{
					opt_id = document["opt_id"].GetString();
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please look up the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		checkOperationState_thread_new(request, response, opt_id);
	}
	else if(operation=="reasonManage")
	{
		int type =0;
		SLOG_DEBUG("begin reasonManage,");
		// cout<<"begin reasonManage...."<<endl;
		try
		{
			if (request_type == "GET")
			{
				
			string error = "the parameter has some error,please consult the api document.";
			sendResponseMsg(1003, error, operation, request, response);
			return;
			}
			else if (request_type == "POST")
			{
				if (document.HasMember("type")==true)
				{
					if (document["type"].IsInt())
					{
						type = document["type"].GetInt();
					}
					else if (document["type"].IsString())
					{
						type = stoi(document["type"].GetString());
					}
					else
					{
						string error = "the type parameter not exist or has some error,please consult the api document.";
						sendResponseMsg(1003, error, operation, request, response);
						return;
					}
					SLOG_DEBUG("begin reasonManage,type:" + std::to_string(type));
					reasonManage_thread_new(request, response, type, db_name, document);
				}
				else
				{
					string error = "the type parameter not exist or has some error,please consult the api document.";
					sendResponseMsg(1003, error, operation, request, response);
					return;
				}
			}
		}
		catch (...)
		{
			string error = "the parameter has some error,please consult the api document.";
			cout<<"error:"<<error<<endl;
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		
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
	string ipCheckResult = apiUtil->check_access_ip(remote_ip, 0);
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
	string msg = "Server stopped successfully.";
	string resJson = CreateJson(0, msg, 0);
	// SLOG_DEBUG("response result:" + resJson);
	// register callback for exit
	response->register_callback([](std::ios_base::event __e, ios_base& __b, int __i){
		SLOG_DEBUG("Server stopped successfully.");
		delete apiUtil;
		apiUtil = NULL;
		std::cout.flush();
		_exit(1);
	}, 0);
	apiUtil->write_access_log(operation, remote_ip, 0, msg);
	*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: " << resJson.length() << "\r\n\r\n"
				<< resJson;
}

void upload_handler(const HttpServer &server, const shared_ptr<HttpServer::Response> &response, const shared_ptr<HttpServer::Request> &request)
{
	string operation = "uploadfile";
	string thread_id = Util::getThreadID();
	string remote_ip = getRemoteIp(request);
	string ipCheckResult = apiUtil->check_access_ip(remote_ip, 0);
	if (!ipCheckResult.empty())
	{
		sendResponseMsg(1101, ipCheckResult, operation, request, response);
		return;
	}
	string ss;
	ss += "\n------------------------ ghttp-api ------------------------";
	ss += "\nthread_id: " + thread_id;
	ss += "\nremote_ip: " + remote_ip;
	ss += "\noperation: " + operation;
	ss += "\nmethod: POST";
	ss += "\nrequest_path: " + request->path;
	ss += "\nhttp_version: " + request->http_version;
	ss += "\nrequest_time: " + Util::get_date_time();
	ss += "\n----------------------------------------------------------";
	SLOG_DEBUG(ss);
	// "Content-Type:multipart/form-data; boundary=--------------------------617568955343916342854790"
	auto contentType = request->header.find("Content-Type");
	std::string boundary;
	if (contentType != request->header.end())
	{
		size_t idx = contentType->second.find("boundary=");
		if (idx != string::npos)
		{
			idx += 9;
			size_t len = contentType->second.length()-idx;
			boundary = "--" + contentType->second.substr(idx, len);
		}
	} 
	else 
	{
		sendResponseMsg(9, "Content-Type not match", operation, request, response);
		return;
	}
	if (boundary.empty())
	{
		sendResponseMsg(1005, "boundary parse error", operation, request, response);
		return;
	}
	auto data = std::make_shared<std::string>(request->content.string());

	#if defined(MULTIPART_PARSER_DEBUG)	
	cout << contentType->second << endl;
	cout << "parse boundary: " << boundary << endl;
	cout << *data << endl;
	#endif // MULTIPART_PARSER_DEBUG

	MultipartParser multipartParser(data, 0, boundary);
	std::unique_ptr<MultipartFormData> formPtr = multipartParser.parse();

	#if defined(MULTIPART_PARSER_DEBUG)	
	MultipartFormData::iterator formItem;
	for (formItem = formPtr->begin(); formItem != formPtr->end(); formItem++)
	{
		if (formItem->first == "file")
		{
			cout << "filename=" << formItem->second.first << endl;
		} 
		else 
		{
			cout << formItem->second.first << "=" << formItem->second.second << endl;
		}
	}
	#endif // MULTIPART_PARSER_DEBUG
	
	if (formPtr->empty())
	{   
		sendResponseMsg(9, "Form data is empty", operation, request, response);
		formPtr.release();
		return;
	}
	std::string error;
	if (formPtr->find("username") == formPtr->end() || formPtr->find("password") == formPtr->end())
	{
		error = "username or password is empty";
		sendResponseMsg(1003, error, operation, request, response);
		formPtr.release();
		return;
	}
	std::string username = formPtr->at("username").second;
	std::string password = formPtr->at("password").second;
	error = apiUtil->check_param_value("username", username);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, operation, request, response);
		formPtr.release();
		return;
	}
	error = apiUtil->check_param_value("password", password);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, operation, request, response);
		formPtr.release();
		return;
	}
	// filename : filecontent
	std::pair<std::string, std::string>& fileinfo = formPtr->at("file");
	if(fileinfo.first.empty())
	{
		error = "Upload file can not be empty!";
		sendResponseMsg(1003, error, operation, request, response);
		formPtr.release();
		return;
	}
	if(request->header.find("Content-Length") != request->header.end())
	{
		size_t content_length = stoul(request->header.find("Content-Length")->second, nullptr, 0);
		size_t max_body_size = apiUtil->get_upload_max_body_size();
		if (content_length > max_body_size)
		{
			SLOG_DEBUG("File size is " + to_string(content_length) + " byte, allowed max size " + to_string(max_body_size) + " byte!");
			error = "Upload file more than max_body_size!";
			sendResponseMsg(1005, error, operation, request, response);
			formPtr.release();
			return;
		}
	}
	std::string file_suffix = fileSuffix(fileinfo.first);
	if (!apiUtil->check_upload_allow_compress_packages(file_suffix) && apiUtil->check_upload_allow_extensions(file_suffix) == false)
	{
		error = "The type of upload file is not supported!";
		sendResponseMsg(1005, error, operation, request, response);
		formPtr.release();
		return;
	}
	
	// remove path info, only return base filename
	std::string file_name = fileName(fileinfo.first);
	size_t pos = file_name.size() - file_suffix.size() - 1;
	std::string file_dst = apiUtil->get_upload_path() + file_name.substr(0, pos) + "_" + Util::getTimeString2() + "." + file_suffix;
	// write file
	ofstream fout(file_dst.c_str());
	if (fout)
	{
		fout << fileinfo.second;
		fout.close();
		formPtr.release();
		std::string msg = "upload file success, file path: " + file_dst;
		std::string resJson = "{\"StatusCode\":0, \"StatusMsg\":\"success\", \"filepath\": \""+file_dst+"\"}";
		apiUtil->write_access_log(operation, remote_ip, 0, msg);
		
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " 
				  << resJson.length() << "\r\n\r\n"
				  << resJson;
	}
	else
	{
		formPtr.release();
		SLOG_ERROR("open file error: " + file_dst);
		error = "Upload file fail: can not write file to dest path!";
		sendResponseMsg(1005, error, operation, request, response);
	}
}

void download_handler(const HttpServer &server, const shared_ptr<HttpServer::Response> &response, const shared_ptr<HttpServer::Request> &request, string request_type)
{
	string operation = "downloadfile";
	string thread_id = Util::getThreadID();
	string remote_ip = getRemoteIp(request);
	string ipCheckResult = apiUtil->check_access_ip(remote_ip, 0);
	if (!ipCheckResult.empty())
	{
		sendResponseMsg(1101, ipCheckResult, operation, request, response);
		return;
	}
	string username;
	string password;
	string encryption;
	string filepath;
	string url;
	if (request_type == "GET")
	{
		url = request->path;
		url = UrlDecode(url);
		string ss;
		ss += "\n------------------------ ghttp-api ------------------------";
		ss += "\nthread_id: " + thread_id;
		ss += "\nremote_ip: " + remote_ip;
		ss += "\noperation: " + operation;
		ss += "\nmethod: GET";
		ss += "\nrequest_path: " + request->path;
		ss += "\nhttp_version: " + request->http_version;
		ss += "\nrequest_time: " + Util::get_date_time();
		ss += "\n----------------------------------------------------------";
		SLOG_DEBUG(ss);

		username = WebUrl::CutParam(url, "username");
		password = WebUrl::CutParam(url, "password");
		encryption = WebUrl::CutParam(url, "encryption");
		filepath = WebUrl::CutParam(url, "filepath");
		if (encryption.empty()) {
			encryption = "0";
		}

		username = UrlDecode(username);
		password = UrlDecode(password);
		filepath = UrlDecode(filepath);
	}
	else if (request_type == "POST")
	{
		auto strParams = request->content.string();
		string ss;
		ss += "\n------------------------ ghttp-api ------------------------";
		ss += "\nthread_id: " + thread_id;
		ss += "\nremote_ip: " + remote_ip;
		ss += "\noperation: " + operation;
		ss += "\nmethod: POST";
		ss += "\nrequest_path: " + request->path;
		ss += "\nhttp_version: " + request->http_version;
		ss += "\nrequest_time: " + Util::get_date_time();
		ss += "\nrequest_body: \n" + strParams;
		ss += "\n----------------------------------------------------------";
		SLOG_DEBUG(ss);
		std::map<std::string, std::string> form_data = parse_post_body(strParams);
		username = "";
		password = "";
		if (form_data.find("username") != form_data.end())
		{
			username = UrlDecode(form_data.at("username"));
		}
		if (form_data.find("password") != form_data.end())
		{
			password =  UrlDecode(form_data.at("password"));
		}
		if (form_data.find("encryption") != form_data.end())
		{
			encryption = UrlDecode(form_data.at("encryption"));
		}
		else
		{
			encryption = "0";
		}
		if (form_data.find("filepath") != form_data.end())
		{
			filepath = UrlDecode(form_data.at("filepath"));
		}
	}

	string error="";
	error = apiUtil->check_param_value("username", username);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, operation, request, response);
		return;
	}
	error = apiUtil->check_param_value("password", password);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, operation, request, response);
		return;
	}
	error = apiUtil->check_param_value("filepath", filepath);
	if (error.empty() == false)
	{
		sendResponseMsg(1003, error, operation, request, response);
		return;
	}
	if (Util::is_file(filepath))
	{
		// the file must in the gstore home dir
		std::string exact_path = Util::getExactPath(filepath.c_str());
		std::string cur_path = Util::get_cur_path();
		SLOG_DEBUG("download file path: " + filepath);
		SLOG_DEBUG("file exact path: " + exact_path);
		if (start_with(exact_path, cur_path) == false)
		{
			error = "Download file must in the gstore home dir";
			sendResponseMsg(1005, error, operation, request, response);
			return;
		}
		size_t file_size;
		int start = 0;
	    int end = -1;
        int ret = fileSize(exact_path, &file_size);

        if (ret != 0)
        {
			error = "Read file error!";
            sendResponseMsg(1005, error, operation, request, response);
			return;
        }
       	end = file_size;

		ifstream ifs;
		ifs.open(exact_path.c_str(), ios::binary);
		char buffer;
		*response << "HTTP/1.1 200 OK\r\nContent-Type: application/octet-stream\r\nAccess-Control-Allow-Origin: *" 
				  << "\r\nContent-Range: bytes "+ std::to_string(start) + "-" + std::to_string(end) + "/" + std::to_string(file_size)
				  << "\r\nContent-Length: " + std::to_string(file_size) << "\r\n\r\n";
		while(ifs.get(buffer))
		{
			*response << buffer;
		}
		ifs.close();
	}
	else
	{
		error = "Download file not exists";
		sendResponseMsg(1005, error, operation, request, response);
	}
}

std::map<std::string, std::string> parse_post_body(const std::string &body)
{
    std::map<std::string, std::string> map;

    if (body.empty())
        return map;

    std::vector<std::string> arr;
	Util::split(body, "&", arr);

    if (arr.empty())
        return map;

    for (const auto &ele: arr)
    {
        if (ele.empty())
            continue;

        std::vector<std::string> kv;
		Util::split(ele, "=", kv);
        size_t kv_size = kv.size();
        std::string &key = kv[0];

        if (key.empty() || map.count(key) > 0)
            continue;

        if (kv_size == 1)
        {
            map.emplace(std::move(key), "");
            continue;
        }

        std::string &val = kv[1];

        if (val.empty())
            map.emplace(std::move(key), "");
        else
            map.emplace(std::move(key), std::move(val));
    }
    return map;
}

int fileSize(const std::string &filepath, size_t *size)
{
    // https://linux.die.net/man/2/stat
    struct stat st;
    memset(&st, 0, sizeof st);
    int ret = stat(filepath.c_str(), &st);
    int status = 0;
    if (ret == -1)
    {
        *size = 0;
        status = 1;
    }
    else
    {
        *size = st.st_size;
    }
    return status;
}

bool start_with(const std::string& str, const std::string& prefix)
{
	size_t prefix_len = prefix.size();

	if (str.size() < prefix_len)
		return false;

	for (size_t i = 0; i < prefix_len; i++)
	{
		if (str[i] != prefix[i])
			return false;
	}

	return true;
}

std::string fileSuffix(const std::string &filepath)
{
    std::string::size_type pos1 = filepath.find_last_of("/");
    if (pos1 == std::string::npos)
    {
        pos1 = 0;
    }
    else
    {
        pos1++;
    }
    std::string file = filepath.substr(pos1, -1);

    std::string::size_type pos2 = file.find_last_of(".");
    if (pos2 == std::string::npos)
    {
        return "";
    }
    return file.substr(pos2 + 1, -1);
}

std::string fileName(const std::string &filepath)
{
    std::string::size_type pos1 = filepath.find_last_not_of("/");
    if (pos1 == std::string::npos)
    {
        return "/";
    }
    std::string::size_type pos2 = filepath.find_last_of("/", pos1);
    if (pos2 == std::string::npos)
    {
        pos2 = 0;
    } else
    {
        pos2++;
    }

    return filepath.substr(pos2, pos1 - pos2 + 1);
}

std::string CreateJson(int StatusCode, string StatusMsg, bool body, string ResponseBody)
{
	rapidjson::StringBuffer s;
	rapidjson::Writer<StringBuffer> writer(s);
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
		rapidjson::Document all;
		rapidjson::Document::AllocatorType &allocator = all.GetAllocator();
		all.SetObject();
		rapidjson::Value jsonArray(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			DBQueryLogInfo log_info = logList[i];
			jsonArray.PushBack(log_info.toJSON(allocator).Move(), allocator);
		}
		
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
		size_t count = logfiles.size();
		std::string item;
		rapidjson::Document all;
		rapidjson::Value jsonArray(rapidjson::kArrayType);
		all.SetObject();
		rapidjson::Document::AllocatorType &allocator = all.GetAllocator();
		for (size_t i = 0; i < count; i++)
		{
			item = logfiles[i];
			item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
			jsonArray.PushBack(rapidjson::Value().SetString(item.c_str(), allocator).Move(), allocator);
		}
		all.AddMember("StatusCode", 0, allocator);
		all.AddMember("StatusMsg", "Get query log date success", allocator);
		all.AddMember("list", jsonArray, allocator);
		sendResponseMsg(all, operation, request, response);
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
		rapidjson::Document all;
		rapidjson::Value jsonArray(rapidjson::kArrayType);
		Document::AllocatorType &allocator = all.GetAllocator();
		for (size_t i = 0; i < count; i++)
		{
			DBAccessLogInfo log_info = logList[i];
			jsonArray.PushBack(log_info.toJSON(allocator).Move(), allocator);
		}
		int totalSize = dbAccessLogs.getTotalSize();
		int totalPage = dbAccessLogs.getTotalPage();
		all.SetObject();
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
		size_t count = logfiles.size();
		std::string item;
		rapidjson::Document all;
		rapidjson::Value jsonArray(rapidjson::kArrayType);
		all.SetObject();
		rapidjson::Document::AllocatorType &allocator = all.GetAllocator();
		for (size_t i = 0; i < count; i++)
		{
			item = logfiles[i];
			item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
			jsonArray.PushBack(rapidjson::Value().SetString(item.c_str(), allocator).Move(), allocator);
		}
		all.AddMember("StatusCode", 0, allocator);
		all.AddMember("StatusMsg", "Get access log date success", allocator);
		all.AddMember("list", jsonArray, allocator);
		sendResponseMsg(all, operation, request, response);
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
			rapidjson::Document all;
			rapidjson::Document::AllocatorType &allocator = all.GetAllocator();
			rapidjson::Value responseBody(kObjectType);
			rapidjson::Value ips(kArrayType);
			for (size_t i = 0; i < count; i++)
			{
				ips.PushBack(rapidjson::Value().SetString(ip_list[i].c_str(), allocator).Move(), allocator);
			}

			responseBody.AddMember("ip_type", rapidjson::Value().SetString(IPtype.c_str(), allocator).Move(), allocator);
			responseBody.AddMember("ips", ips, allocator);

			all.SetObject();
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
		pfnUtil->fun_query(pfn_info.getFunName(), pfn_info.getFunStatus(), username, pfn_infos);
		vector<struct PFNInfo> list = pfn_infos->getPFNInfoList();
		size_t count = list.size();
		rapidjson::Document all;
		rapidjson::Document::AllocatorType &allocator = all.GetAllocator();
		rapidjson::Value jsonArray(rapidjson::kArrayType);
		for (size_t i = 0; i < count; i++)
		{
			jsonArray.PushBack(list[i].toJSON(allocator).Move(), allocator);
		}
		all.SetObject();
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
			pfnUtil->fun_create(username, &pfn_info);
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
			pfnUtil->fun_update(username, &pfn_info);
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
			pfnUtil->fun_delete(username, &pfn_info);
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
			string result = pfnUtil->fun_build(username, pfn_info.getFunName());
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
		pfnUtil->fun_review(username, &pfn_info);
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

void rename_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string db_name, string new_name)
{
	string operation = "rename";
	try
	{
		std::string error = apiUtil->check_param_value("db_name", db_name);
		if (error.empty() == false)
		{
			sendResponseMsg(1003, error, operation, request, response);
			return;
		}
		error = apiUtil->check_param_value("new_name", new_name);
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
		if (apiUtil->check_already_load(db_name) == true)
		{
			error = "Database is loaded, need unload first.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		// check new_name available
		if (apiUtil->check_db_exist(new_name) == true)
		{
			error = "Database name " + new_name + " already exists.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}

		shared_ptr<DatabaseInfo> db_info;
		apiUtil->get_databaseinfo(db_name, db_info);
		if (apiUtil->trywrlock_databaseinfo(db_info) == false)
		{
			error = "Unable to rename due to loss of lock";
			sendResponseMsg(1007, error, operation, request, response);
			return;
		}
		string new_db_path = _db_home + "/" + new_name + _db_suffix;
		// check new_db_path.
		if (Util::dir_exist(new_db_path))
		{
			error = "Database local path " + new_db_path + " already exists.";
			sendResponseMsg(1004, error, operation, request, response);
			apiUtil->unlock_databaseinfo(db_info);
			return;
		}
		
		// mv old folder to new folder
		string db_path = _db_home + "/" + db_name + _db_suffix;
		string sys_cmd = "mv " + db_path + " " + new_db_path;
		std::system(sys_cmd.c_str());
		apiUtil->unlock_databaseinfo(db_info);
		// insert new_name
		apiUtil->add_already_build(new_name, db_info->getCreator(), db_info->getTime());
		// copy privileges
		apiUtil->copy_privilege(db_name, new_name);
		// add backuplog
		Util::add_backuplog(new_name);

		// remove old_name
		apiUtil->delete_from_already_build(db_name);
		// remove backuplog
		Util::delete_backuplog(db_name);

		std::string success = "Database rename successfully.";
		sendResponseMsg(0, success, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string msg = "rename fail:" + string(e.what());
		sendResponseMsg(1005, msg, operation, request, response);
	}
}

void stat_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response)
{
	string operation = "stat";
	try
	{
		int pid = getpid();
		float cup_usage = Util::get_cpu_usage(pid) * 100; // %
		char cup_usage_char[32];
		sprintf(cup_usage_char, "%f", cup_usage);
		float mem_usage = Util::get_memory_usage(pid); // MB
		char mem_usage_char[32];
		sprintf(mem_usage_char, "%f", mem_usage);
		unsigned long long disk_available = Util::get_disk_free(); // MB
		char disk_available_char[32];
		sprintf(disk_available_char, "%llu", disk_available);
		Document resp_data;
		Document::AllocatorType &allocator = resp_data.GetAllocator();
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", 0, allocator);
		resp_data.AddMember("StatusMsg", "success", allocator);
		resp_data.AddMember("cup_usage", StringRef(cup_usage_char), allocator);
		resp_data.AddMember("mem_usage", StringRef(mem_usage_char), allocator);
		resp_data.AddMember("disk_available", StringRef(disk_available_char), allocator);
		sendResponseMsg(resp_data, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string msg = "stat fail:" + string(e.what());
		sendResponseMsg(1005, msg, operation, request, response);
	}
}

void checkOperationState_thread_new(const shared_ptr<HttpServer::Request> &request, const shared_ptr<HttpServer::Response> &response, string opt_id)
{
	string error;
	string operation = "checkOperationState";
	try
	{
		if (opt_id.empty())
		{
			error = "opt_id is empty.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		struct DBAccessLogInfo log;
		if (!apiUtil->getAccessLogByOptId(opt_id, log))
		{
			error = "opt_id not found.";
			sendResponseMsg(1004, error, operation, request, response);
			return;
		}
		Document resp_data;
		Document::AllocatorType &allocator = resp_data.GetAllocator();
		string msg = log.getMsg();
		std::string backupfilepath = log.getBackupfilepath();
		resp_data.SetObject();
		resp_data.AddMember("StatusCode", log.getCode(), allocator);
		resp_data.AddMember("StatusMsg", StringRef(msg.c_str()), allocator);
		resp_data.AddMember("state", log.getState(), allocator);
		std::string log_operation = log.getOperation();
		if (log_operation == "backup")
		{
			resp_data.AddMember("backupfilepath", StringRef(backupfilepath.c_str()), allocator);
		}
		else if(log_operation != "restore")
		{
			resp_data.AddMember("success_num", log.getNum(), allocator);
			resp_data.AddMember("failed_num", log.getFailNum(), allocator);
		}
		sendResponseMsg(resp_data, operation, request, response);
	}
	catch (const std::exception &e)
	{
		string error = "checkbatchInsertUid fail:" + string(e.what());
		sendResponseMsg(1005, error, operation, request, response);
	}
}