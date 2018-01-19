/*=============================================================================
# Filename: ghttp.cpp
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2017-06-15 15:09
# Description: created by lvxin, improved by lijing
=============================================================================*/

//SPARQL Endpoint:  log files in logs/endpoint/
//operation.log: not used
//query.log: query string, result num, and time of answering

//TODO: to add db_name to all URLs, and change the index.js using /show to get name, save and set
//TODO: use gzip for network transfer, it is ok to download a gzip file instead of the original one

//TODO: mutiple threads , multiple users and multiple databases
//How to acquire http connection ID? getSocket()  or use username to login?
//But when setting up endpoint, username may not be a good choice

//DEBUG: if port has already been occupied, the server will be restarted endlessly, here we should stop

//TODO: use response<< directly for query result, but for results too large to be placed in memory,
//file donwload and Stream should be considered, split-page getNext() as well
//Notice that read each block from disk and transfer to network may be very slow
//(and multi-page requests will bring high overhead for parallism)
//
//BETTER: define a format to return all information (header and body)
//
//TODO: backup() only preserve one version? is remove() and copy() updated to disk in time?
//how to avoid disk error itself? send to otherwhere?
//
//TODO+DEBUG?: write log in update_log in Database.cpp 
//is ostream.write() ok to update to disk at once? all add ofstream.flush()?
//Also the checkpoint function!!!
//http://bookug.cc/rwbuffer
//BETTER: add a sync function in Util to support FILE*, fd, and fstream
//In addition, query log in endpoint should also be synced!

//PARALLISM: now all requests are queued automatically, after one is finished , then another
//Hiw can we make some requests run in parallism

#include "../Server/server_http.hpp"
#include "../Server/client_http.hpp"
//db
#include "../Database/Database.h"
#include "../Util/Util.h"

using namespace std;
//Added for the json-example:
using namespace boost::property_tree;

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef SimpleWeb::Client<SimpleWeb::HTTP> HttpClient;

#define MAX_DATABASE_NUM 100
#define MAX_USER_NUM 1000
#define ROOT_USERNAME "root"
#define ROOT_PASSWORD "123456"
#define DEFAULT_USERNAME "a"
#define DEFAULT_PASSWORD "1"

//int initialize();
int initialize(int argc, char *argv[]);
//Added for the default_resource example
void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
        const shared_ptr<ifstream> &ifs);
void download_result(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string download, string filepath);

pthread_t start_thread(void *(*_function)(void*));
bool stop_thread(pthread_t _thread);
void* func_timer(void* _args);
void* func_scheduler(void* _args);
void thread_sigterm_handler(int _signal_num);
bool addPrivilege(string username, string type, string db_name);
bool delPrivilege(string username, string type, string db_name);
bool checkPrivilege(string username, string type, string db_name);
bool doQuery(string format, string db_query, const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

//=============================================================================
bool build_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool load_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool unload_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool query_handler0(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool query_handler1(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool monitor_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool checkpoint_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool show_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool delete_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool download_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool default_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool login_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool user_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool showUser_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool check_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);
//=============================================================================

Database *current_database = NULL;

std::map<std::string, Database *> databases;
std::set<std::string> already_build;

struct User{
	private:
		std::string username;
		std::string password;
	public:
		std::set<std::string> query_priv;
		std::set<std::string> load_priv;
		std::set<std::string> unload_priv;
		
		/*
		Database *build_priv[MAX_DATABASE_NUM];
		Database *load_priv[MAX_DATABASE_NUM];
		Database *unload_priv[MAX_DATABASE_NUM];
		*/

		User(){}
		User(string _username, string _password){
			if(_username == "")
				username = DEFAULT_USERNAME;
			else
				username = _username;
			if(_password == "")
				password = DEFAULT_PASSWORD;
			else
				password = _password;
		}
		std::string getPassword(){
			return password;
		}
		std::string getUsername(){
			return username;
		}
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
				query_db = query_db + *it + " ";
				++it;
			}
			return query_db;
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
				load_db = load_db + *it + " ";
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
				unload_db = unload_db + *it + " ";
				++it;
			}
			return unload_db;
		}
};
//struct User root = User(ROOT_USERNAME, ROOT_PASSWORD);

std::map<std::string, struct User *> users;
struct User root = User(ROOT_USERNAME, ROOT_PASSWORD);
//users.insert(pair<std::string, struct User*>(ROOT_USERNAME, &root));
//users[ROOT_USERNAME] = &root;
//struct User temp_user = User(username2, password2);
		
int connection_num = 0;

long next_backup = 0;
pthread_t scheduler = 0;

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

int main(int argc, char *argv[])
{
	Util util;
	srand(time(NULL));

	while (true) {
		//NOTICE: here we use 2 processes, father process is used for monitor and control(like, restart)
		//Child process is used to deal with web requests, can also has many threads
		pid_t fpid = fork();

		if (fpid == 0) {
			//int ret = initialize();
			int ret = initialize(argc, argv);
			exit(ret);
			return ret;
		}

		else if (fpid > 0) {
			int status;
			waitpid(fpid, &status, 0);
			if (WIFEXITED(status)) {
				exit(0);
				return 0;
			}
			else
			{
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
    cout << "enter initialize." << endl;
	//Server restarts to use the original database
	//current_database = NULL;

	users.insert(pair<std::string, struct User *>(ROOT_USERNAME, &root));

	HttpServer server;
	string db_name;
	if(argc == 1)
	{
		server.config.port = 9000;
		db_name = "";
	}
	else if(argc == 2)
	{
		if(isNum(argv[1]))
		{
			server.config.port = atoi(argv[1]);
			db_name = "";
		}
		else
		{
			server.config.port = 9000;
			db_name = argv[1];
		}
	}
	else
	{
		if(isNum(argv[1]))
		{
			server.config.port = atoi(argv[1]);
			db_name = argv[2];
		}
		else if(isNum(argv[2]))
		{
			server.config.port = atoi(argv[2]);
			db_name = argv[1];
		}
		else
		{
			cout << "wrong format of parameters, please input the server port and the database." << endl;
			return -1;
		}
	}
	cout << "server port: " << server.config.port << " database name: " << db_name << endl;
	//USAGE: then user can use http://localhost:port/ to visit the server or coding with RESTful API
    //HTTP-server at port 9000 using 1 thread
    //Unless you do more heavy non-threaded processing in the resources,
    //1 thread is usually faster than several threads

    //GET-example for the path /load/[db_name], responds with the matched string in path
    //For instance a request GET /load/db123 will receive: db123
    //server.resource["^/load/(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {

	string database = db_name;
	if(current_database == NULL && database != "")
	{
		if(database.length() > 3 && database.substr(database.length()-3, 3) == ".db")
		{
			cout << "Your db name to be built should not end with \".db\"." << endl;
			return -1;
		}
	
		cout << database << endl;
		current_database = new Database(database);
		bool flag = current_database->load();
		if (!flag)
		{
			cout << "Failed to load the database."<<endl;
			delete current_database;
			current_database = NULL;
			return -1;
		}
		//string success = db_name;
		cout << "Database loaded successfully."<<endl;
		already_build.insert(db_name);
		databases.insert(pair<std::string, Database *>(db_name, current_database));
	}

	time_t cur_time = time(NULL);
	long time_backup = Util::read_backup_time();
	long next_backup = cur_time - (cur_time - time_backup) % Util::gserver_backup_interval + Util::gserver_backup_interval;
	//NOTICE: no need to backup for endpoint
#ifndef ONLY_READ
	scheduler = start_thread(func_scheduler);
#endif



#ifndef SPARQL_ENDPOINT
	//GET-example for the path /?operation=build&db_name=[db_name]&ds_path=[ds_path]&username=[username]&password=[password], responds with the matched string in path
	//i.e. database name and dataset path
	server.resource["^/%3[F|f]operation%3[D|d]build%26db_name%3[D|d](.*)%26ds_path%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		build_handler(server, response, request);
    };

	//GET-example for the path /?operation=load&db_name=[db_name]&username=[username]&password=[password], responds with the matched string in path
    server.resource["^/%3[F|f]operation%3[D|d]load%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		load_handler(server, response, request);
	};

    //GET-example for the path /?operation=unload&db_name=[db_name]&username=[username]&password=[password], responds with the matched string in path
    server.resource["^/%3[F|f]operation%3[D|d]unload%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		unload_handler(server, response, request);
    }; 

    //GET-example for the path /?operation=login&username=[username]&password=[password], responds with the matched string in path
    server.resource["^/%3[F|f]operation%3[D|d]login%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		login_handler(server, response, request);
    };

	//GET-example for the path /?operation=user&type=[type]&username1=[username1]&password1=[password1]&username2=[username2]&addtion=[password2 || db_name], responds with the matched string in path
	server.resource["^/%3[F|f]operation%3[D|d]user%26type%3[D|d](.*)%26username1%3[D|d](.*)%26password1%3[D|d](.*)%26username2%3[D|d](.*)%26addtion%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		user_handler(server, response, request);
    }; 

	//GET-example for the path /?operation=user&type=[type]&username1=[username1]&password1=[password1]&username2=[username2]&addtion=[password2 || db_name], responds with the matched string in path
	server.resource["^/?operation=user&type=(.*)&username1=(.*)&password1=(.*)&username2=(.*)&addtion=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		user_handler(server, response, request);
    };

	//GET-example for the path /?operation=login&username=[username]&password=[password], responds with the matched string in path
	server.resource["^/?operation=login&username=(.*)&password=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		login_handler(server, response, request);
    };

	//GET-example for the path /?operation=build&db_name=[db_name]&ds_path=[ds_path]&username=[username]&password=[password], responds with the matched string in path
	//i.e. database name and dataset path
	server.resource["^/?operation=build&db_name=(.*)&ds_path=(.*)&username=(.*)&password=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		build_handler(server, response, request);
    };

	//GET-example for the path /?operation=load&db_name=[db_name]&username=[username]&password=[password], responds with the matched string in path
    server.resource["^/?operation=load&db_name=(.*)&username=(.*)&password=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		load_handler(server, response, request);
	};

    //GET-example for the path /?operation=unload&db_name=[db_name]&username=[username]&password=[password], responds with the matched string in path
    server.resource["^/?operation=unload&db_name=(.*)&username=(.*)&password=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		unload_handler(server, response, request);
    };
   
	//NOTICE:this may not be visited by browser directly if the browser does not do URL encode automatically!
	//In programming language, do URL encode first and then call server, then all is ok
	server.resource["^/%3[F|f]operation%3[D|d]showUser(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
	 //server.resource["^/monitor$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		showUser_handler(server, response, request);
    };
 
    //NOTICE:this may not be visited by browser directly if the browser does not do URL encode automatically!
	//In programming language, do URL encode first and then call server, then all is ok
	server.resource["^/?operation=showUser$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
	 //server.resource["^/monitor$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		showUser_handler(server, response, request);
    };
   
	//GET-example for the path /?operation=query&db_name=[db_name]&username=[username]&password=[password]&format=[format]&sparql=[sparql], responds with the matched string in path
	 server.resource["^/%3[F|f]operation%3[D|d]query%26db_name%3[D|d](.*)%26username%3[D|d](.*)%26password%3[D|d](.*)%26format%3[D|d](.*)%26sparql%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	 {
		query_handler1(server, response, request);
		// server.resource["^/query/(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    };

	 //GET-example for the path /?operation=query&db_name=[db_name]&username=[username]&password=[password]&format=[format]&sparql=[sparql], responds with the matched string in path
	 server.resource["^/?operation=query&db_name=(.*)&username=(.*)&password=(.*)&format=(.*)&sparql=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	 {
		query_handler1(server, response, request);
		// server.resource["^/query/(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    };
	//GET-example for the path /?operation=check&username=[username]&password=[password], responds with the matched string in path
    server.resource["^/%3[F|f]operation%3[D|d]check%26username%3[D|d](.*)%26password%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		check_handler(server, response, request);
    };
	//GET-example for the path /?operation=check&username=[username]&password=[password], responds with the matched string in path
	server.resource["^/?operation=check&username=(.*)&password=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		check_handler(server, response, request);
    };

#endif
	
	
	//GET-example for the path /?operation=query&&format=[format]&sparql=[sparql], responds with the matched string in path
	 server.resource["^/%3[F|f]operation%3[D|d]query%26format%3[D|d](.*)%26sparql%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	 {
		query_handler0(server, response, request);
		// server.resource["^/query/(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    };

	 //GET-example for the path /?operation=query&format=[format]&sparql=[sparql], responds with the matched string in path
	 server.resource["^/?operation=query&format=(.*)&sparql=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	 {
		query_handler0(server, response, request);
		// server.resource["^/query/(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    };

    //NOTICE:this may not be visited by browser directly if the browser does not do URL encode automatically!
	//In programming language, do URL encode first and then call server, then all is ok
	server.resource["^/%3[F|f]operation%3[D|d]monitor%26db_name%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
	 //server.resource["^/monitor$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		monitor_handler(server, response, request);
    };
 
    //NOTICE:this may not be visited by browser directly if the browser does not do URL encode automatically!
	//In programming language, do URL encode first and then call server, then all is ok
	server.resource["^/?operation=monitor&db_name=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
	 //server.resource["^/monitor$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		monitor_handler(server, response, request);
    };
   
    // server.resource["^/json$"]["POST"]=[](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    //     try {
    //         ptree pt;
    //         read_json(request->content, pt);

    //         string sparql=pt.get<string>("queryContent");

    //         *response << "HTTP/1.1 200 OK\r\n"
    //             << "Content-Type: application/json\r\n"
    //             << "Content-Length: " << name.length() << "\r\n\r\n"
    //             << name;
    //     }
    //     catch(exception& e) {
    //         *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
    //     }
    // };

	//TODO: use db_name if multiple databases
    server.resource["^/%3[F|f]operation%3[D|d]checkpoint%26db_name%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		checkpoint_handler(server, response, request);
    };
	//TODO: use db_name if multiple databases
	server.resource["^/?operation=checkpoint&db_name=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		checkpoint_handler(server, response, request);
    };

	//TODO: add user name as parameter, current using or all databases availiable
    server.resource["^/%3[F|f]operation%3[D|d]show%26type%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		show_handler(server, response, request);
    };

	//TODO: add user name as parameter, current using or all databases availiable
    server.resource["^/?operation=show&type=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		show_handler(server, response, request);
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
	//
	//GET-example for the path /?operation=delete&filepath=[filepath], responds with the matched string in path
	server.resource["^/%3[F|f]operation%3[D|d]delete%26filepath%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		delete_handler(server, response, request);
    };
	//GET-example for the path /?operation=delete&filepath=[filepath], responds with the matched string in path
	server.resource["^/%3[F|f]operation%3[D|d]download%26filepath%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		download_handler(server, response, request);
    };
    //Default GET-example. If no other matches, this anonymous function will be called.
    //Will respond with content in the web/-directory, and its subdirectories.
    //Default file: index.html
    //Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    server.default_resource["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		default_handler(server, response, request);
    };

    thread server_thread([&server](){
            //Start server
            server.start();
			cout<<"check: server started"<<endl;
            });

    //Wait for server to start so that the client can connect
    this_thread::sleep_for(chrono::seconds(1));

    // //Client examples
    // HttpClient client("localhost:9000");
    // auto r1=client.request("GET", "/match/123");
    // cout << r1->content.rdbuf() << endl;

    // string json_string="{\"firstName\": \"John\",\"lastName\": \"Smith\",\"age\": 25}";
    // auto r2=client.request("POST", "/string", json_string);
    // cout << r2->content.rdbuf() << endl;

    // auto r3=client.request("POST", "/json", json_string);
    // cout << r3->content.rdbuf() << endl;

    server_thread.join();
	cout<<"check: server stoped"<<endl;

    return 0;
}

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
	abort();
}

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

void thread_sigterm_handler(int _signal_num) {
	pthread_exit(0);
}

void delete_result(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string filepath)
{
	cout<<"to delete result"<<endl;
	try {
		//set the home directory of the web server
		//NOTICE: we use .tmp/web instead of . to avoid attack: delete other files rather than the download one
		auto root_path=boost::filesystem::canonical(".tmp/web");
		auto path=boost::filesystem::canonical(root_path/filepath);
		cout << "path: " << path << endl;
		//Check if path is within root_path
		if(distance(root_path.begin(), root_path.end())>distance(path.begin(), path.end()) ||
				!equal(root_path.begin(), root_path.end(), path.begin()))
			throw invalid_argument("path must be within root path.");
		/*
		if(download == "true")
		{
			std::string cache_control, etag;

			// Uncomment the following line to enable Cache-Control
			// cache_control="Cache-Control: max-age=86400\r\n";
			
			//return file in path
			auto ifs=make_shared<ifstream>();
			ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

			if(*ifs) {
		//cout<<"open file!!!"<<endl;
				auto length=ifs->tellg();
				ifs->seekg(0, ios::beg);

				*response << "HTTP/1.1 200 OK\r\n" << cache_control << etag << "Content-Length: " << length << "\r\n\r\n";
				default_resource_send(server, response, ifs);
				//DEBUG: in fact this make no sense, result will be received by clients even if server not wait for the network transfer!  (but very slow if results are large)
				//sleep(10);  //seconds
				//cout<<"sleep ends"<<endl;
				//DEBUG: this will close the stream, and the info received by client may be incomplete
				//ifs->close();
			}
			else
				throw invalid_argument("could not read file.");
		}
		*/
		if((boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)))
		{
			//delete file in delpath.
			char name[60];
			strcpy(name, path.c_str());
			cout << "name: " << name << endl;
			if(remove(name) == -1)
			{
				cout << "could not delete file." << endl;
				perror("remove");
			}
			else
			{
				cout << "file delete." << endl;
				//Notice: if file deleted successfully, service need to response to the browser, if not the browser will not execute the call-back function.
				string success = "file delete successfully.";
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
			}
		}	
	}
	catch(const exception &e) {
		//cout<<"can not open file!!!"<<endl;
		string content="Could not open path "+request->path+": "+e.what();
		*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
	}
}

void download_result(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request, string filepath)
{
	cout<<"to download result"<<endl;
	try {
		//set the home directory of the web server
		//NOTICE: we use .tmp/web instead of . to avoid attack: delete other files rather than the download one
		cout << "filepath: " << filepath << endl;
		auto root_path=boost::filesystem::canonical(".tmp/web");
		cout << "root_path: " << root_path << endl;
		auto path=boost::filesystem::canonical(root_path/filepath);
		cout << "path: " << path << endl;
		//Check if path is within root_path
		if(distance(root_path.begin(), root_path.end())>distance(path.begin(), path.end()) ||
				!equal(root_path.begin(), root_path.end(), path.begin()))
			throw invalid_argument("path must be within root path.");

		std::string cache_control, etag;

		// Uncomment the following line to enable Cache-Control
		// cache_control="Cache-Control: max-age=86400\r\n";
			
		//return file in path
		auto ifs=make_shared<ifstream>();
		ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);

		if(*ifs) {
			//cout<<"open file!!!"<<endl;
			auto length=ifs->tellg();
			ifs->seekg(0, ios::beg);
			//!Notice: remember to set the Content-Disposition and the Content-type to let the browser to download.
			*response << "HTTP/1.1 200 OK\r\n" << "Content-Length: " << length << "\r\n";
			*response << "Content-Disposition: attachment; filename=sparql.txt" << "\r\n";
			*response << "Content-Type: application/octet-stream" << "\r\n\r\n";
			default_resource_send(server, response, ifs);
		}
		else
			throw invalid_argument("could not read file.");
		/*
		if((boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)))
		{
			//delete file in delpath.
			char name[60];
			strcpy(name, path.c_str());
			cout << "name: " << name << endl;
			if(remove(name) == -1)
			{
				cout << "could not delete file." << endl;
				perror("remove");
			}
			else
				cout << "file delete." << endl;
		}
		*/
	}
	catch(const exception &e) {
		//cout<<"can not open file!!!"<<endl;
		string content="Could not open path "+request->path+": "+e.what();
		*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
	}
}

bool build_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is build"<<endl;
	string db_name=request->path_match[1];
	string db_path=request->path_match[2];
	string username = request->path_match[3];
	string password = request->path_match[4];
	db_name = UrlDecode(db_name);
	db_path = UrlDecode(db_path);
	username = UrlDecode(username);
	password = UrlDecode(password);

	//check if database named [db_name] is already built
	if(already_build.find(db_name) != already_build.end())
	{
		string error = "database already built.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	//check identity.
	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(it == users.end())
	{
		string error = "username not find.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	else if(it->second->getPassword() != password)
	{
		string error = "wrong password.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	cout << "check identity successfully." << endl;

	if(db_name=="" || db_path=="")
	{
		string error = "Exactly 2 arguments required!";
		// error = db_name + " " + db_path;
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	string database = db_name;
	if(database.length() > 3 && database.substr(database.length()-3, 3) == ".db")
	{
		string error = "Your db name to be built should not end with \".db\".";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	//database += ".db";
	string dataset = db_path;

	cout << "Import dataset to build database..." << endl;
	cout << "DB_store: " << database << "\tRDF_data: " << dataset << endl;
	int len = database.length();

	current_database = new Database(database);
	bool flag = current_database->build(dataset);
	delete current_database;
	current_database = NULL;

	
	if(!flag)
	{
		string error = "Import RDF file to database failed.";
		string cmd = "rm -r " + database;
		system(cmd.c_str());
		return false;
	}

	//by default, one can query or load or unload the database that is built by itself, so add the database name to the privilege set of the user
	if(addPrivilege(username, "query", db_name) == 0 || addPrivilege(username, "load", db_name) == 0 || addPrivilege(username, "unload", db_name) == 0)
	{
		string error = "add query or load or unload privilege failed.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	//add database name to already_build set.
	already_build.insert(db_name);

	// string success = db_name + " " + db_path;
	string success = "Import RDF file to database done.";
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

	return true;
}

bool load_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is load"<<endl;
	string db_name = request->path_match[1];
	string username = request->path_match[2];
	string password = request->path_match[3];
	db_name = UrlDecode(db_name);
	username = UrlDecode(username);
	password = UrlDecode(password);

	//check if database named [db_name] is already load
	if(databases.find(db_name) != databases.end())
	{
		string error = "database already load.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	//check identity.
	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(it == users.end())
	{
		string error = "username not find.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	else if(it->second->getPassword() != password)
	{
		string error = "wrong password.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	cout << "check identity successfully." << endl;

	//check privilege
	if(checkPrivilege(username, "load", db_name) == 0)
	{
		string error = "no load privilege, operation failed.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	cout << "check privilege successfully." << endl;

	//if database already loaded.
	if(databases.find(db_name) != databases.end())
	{
		string error = "Database already loaded.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	//if database haven't been built.
	if(already_build.find(db_name) == already_build.end())
	{
		string error = "Database not built yet.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
   //	string db_name = argv[1];
	if(db_name=="")
	{
		string error = "Exactly 1 argument is required!";
		*response << "HTTP/1.1 200 ok\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	string database = db_name;
	if(database.length() > 3 && database.substr(database.length()-3, 3) == ".db")
	{
		//cout << "Your db name to be built should not end with \".db\"." << endl;
	   string error = "Your db name to be built should not end with \".db\".";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	//database += ".db";
/*
	if(current_database != NULL)
	{
		//cout << "Please unload your current database first." <<endl;
		string error = "Please unload your current database first.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
*/
	cout << database << endl;
	current_database = new Database(database);
	bool flag = current_database->load();
	cout << "load done." << endl;
	if (!flag)
	{
		string error = "Failed to load the database.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		delete current_database;
		current_database = NULL;
		return false;
	}

	time_t cur_time = time(NULL);
	long time_backup = Util::read_backup_time();
	long next_backup = cur_time - (cur_time - time_backup) % Util::gserver_backup_interval + Util::gserver_backup_interval;
	scheduler = start_thread(func_scheduler);
//string success = db_name;
//cout << "Database loaded successfully." << endl;
	
	databases.insert(pair<std::string, Database *>(db_name, current_database));

	cout << "database insert done." << endl;
	string success = "Database loaded successfully.";
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

	return true;
}

bool unload_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is unload"<<endl;

	string db_name = request->path_match[1];
	string username = request->path_match[2];
	string password = request->path_match[3];
	db_name = UrlDecode(db_name);
	username = UrlDecode(username);
	password = UrlDecode(password);

	//check identity.
	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(it == users.end())
	{
		string error = "username not find.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	else if(it->second->getPassword() != password)
	{
		string error = "wrong password.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	cout << "check identity successfully." << endl;

	//check privilege
	if(checkPrivilege(username, "unload", db_name) == 0)
	{
		string error = "no unload privilege, operation failed.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	cout << "check privilege successfully." << endl;
/*
	if(current_database == NULL)
	{
		string error = "No database used now.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	if(current_database->getName() != db_name)
	{
		string error = "Database Name not matched.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
*/
	std::map<std::string, Database *>::iterator iter = databases.find(db_name);
	if(iter == databases.end())
	{
		string error = "Database not load yet.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	delete current_database;
	current_database = NULL;
	databases.erase(db_name);
	string success = "Database unloaded.";
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

	return true;
}
bool doQuery(string format, string db_query, const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	string sparql;
	sparql = db_query;
	if (sparql.empty()) {
		cerr << "Empty SPARQL." << endl;
		return false;
	}
	FILE* output = NULL;


	pthread_t timer = start_thread(func_timer);
	if (timer == 0) {
		cerr << "Failed to start timer." << endl;
	}

	ResultSet rs;
	int query_time = Util::get_cur_time();
	bool ret = current_database->query(sparql, rs, output);
	query_time = Util::get_cur_time() - query_time;
	string query_time_s = Util::int2string(query_time);
	if (timer != 0 && !stop_thread(timer)) {
		cerr << "Failed to stop timer." << endl;
	}
	ostringstream stream;
	stream << rs.ansNum;
	string ansNum_s = stream.str();
	cout << "ansNum_s: " << ansNum_s << endl;
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	string tempTime = asctime(timeinfo);
	for(int i = 0; i < tempTime.length(); i++)
	{
		if(tempTime[i] == ' ')
			tempTime[i] = '_';
	}
	string myTime = tempTime.substr(0, tempTime.length()-1);
	myTime = myTime + "_" + Util::int2string(rand() % 10000);
	//TODO+BETTER: attach the thread's ID in fileName when running in parallism
	string localname = ".tmp/web/" + myTime + "_query";
	//string filename = ".tmp/web/" + myTime + "_query";
	string filename = myTime + "_query";

	//TODO: if result is stored in Stream instead of memory?  (if out of memory to use to_str)
	//BETTER: divide and transfer, in multiple times, getNext()
	if(ret)
	{
	//	cout << "query returned successfully." << endl;
		
		//record each query operation, including the sparql and the answer number
		//TODO: get the query time, and also record it to the log and response to the browser.
		ofstream outlog;
		string queryLog = "logs/endpoint/query.log";
		//BETTER: only open once and close when server stops, use C read/write
		outlog.open(queryLog, ios::app);
		if(!outlog)
		{
			cout << queryLog << "can't open." << endl;
			return false;
		}
		outlog << Util::get_date_time() << endl;
		outlog << sparql << endl << endl;
		outlog << "answer num: "<<rs.ansNum << endl;
		outlog << "query time: "<<query_time <<" ms"<< endl;
		outlog << "-----------------------------------------------------------" << endl;
		outlog.close();


		ofstream outfile;
		string ans = "";
		string success = "";
		if(format == "json")
		{
		//	cout << "query success, transfer to json." << endl;
			success = rs.to_JSON();
		}
		else
		{
		//	cout << "query success, transfer to str." << endl;
			success = rs.to_str();
		}
		if(format == "html")
		{
			localname = localname + ".txt";
			filename = filename + ".txt";
		}
		else
		{
			localname = localname + "." + format;
			filename = filename + "." + format;
		}
		cout << "filename: " << filename << endl;
		if(format == "html")
		{
			outfile.open(localname);
			outfile << success;
			outfile.close();
			if(rs.ansNum <= 100)
			{
				//!Notice: remember to set no-cache in the response of query, Firefox and chrome works well even if you don't set, but IE will act strange if you don't set
				//beacause IE will defaultly cache the query result after first query request, so the following query request of the same url will not be send if the result in cache isn't expired.
				//then the following query will show the same result without sending a request to let the service run query
				//so the download function will go wrong because there is no file in the service.
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << query_time_s.length()+ansNum_s.length()+filename.length()+success.length()+4;
				*response << "\r\nContent-Type: text/plain";
				*response << "\r\nCache-Control: no-cache" << "\r\nPragma: no-cache" << "\r\nExpires: 0";
				*response  << "\r\n\r\n" << "0+" << query_time_s << '+' << rs.ansNum << '+' << filename << '+' << success;
				return true;
			}
			else
			{
				rs.output_limit = 100;
				success = "";
				success = rs.to_str();
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << query_time_s.length()+ansNum_s.length()+filename.length()+success.length()+4;
				*response << "\r\nContent-Type: text/plain";
				*response << "\r\nCache-Control: no-cache" << "\r\nPragma: no-cache" << "\r\nExpires: 0";
				*response << "\r\n\r\n" << "1+" << query_time_s << '+' << rs.ansNum << '+' << filename << '+' << success;
				return true;
			}
		}
		else
		{
			
			string filename = "";
			filename = "sparql." + format;
			cout << "filename: " << filename << endl;
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length();
			*response << "\r\nContent-Type: application/octet-stream";
			*response << "\r\nContent-Disposition: attachment; filename=\"" << filename << '"';
			*response << "\r\n\r\n" << success;
			
			//outfile.open(localname);
			//outfile << success;
			//outfile.close();

			//download_result(server, response, request, "true", filename);
			//
			//*response << "HTTP/1.1 200 OK\r\nContent-Length: " << ansNum_s.length()+filename.length()+3;
			//*response << "\r\n\r\n" << "2+" << rs.ansNum << '+' << filename;
			return true;
		}
	}
	else
	{
		//TODO: if error, browser should give some prompt
		string error = "query() returns false.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	return true;
}
bool query_handler0(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is query"<<endl;

	string format = request->path_match[1];
	//string format = "html";
	string db_query=request->path_match[2];
	format = UrlDecode(format);
	db_query = UrlDecode(db_query);
	cout<<"check: "<<db_query<<endl;
	string str = db_query;

	if(current_database == NULL)
	{
		string error = "No database in use!";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	doQuery(format, db_query, server, response, request);

}

bool query_handler1(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is query"<<endl;

	string db_name = request->path_match[1];
	string username = request->path_match[2];
	string password = request->path_match[3];
	string format = request->path_match[4];
	//string format = "html";
	string db_query=request->path_match[5];
	db_name = UrlDecode(db_name);
	username = UrlDecode(username);
	password = UrlDecode(password);
	format = UrlDecode(format);
	db_query = UrlDecode(db_query);
	cout<<"check: "<<db_query<<endl;
	string str = db_query;

	//check identity.
	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(it == users.end())
	{
		string error = "username not find.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	else if(it->second->getPassword() != password)
	{
		string error = "wrong password.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	cout << "check identity successfully." << endl;

	//check privilege
	if(checkPrivilege(username, "query", db_name) == 0)
	{
		string error = "no query privilege, operation failed.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	cout << "check privilege successfully." << endl;

	std::map<std::string, Database *>::iterator iter = databases.find(db_name);
	if(iter == databases.end())
	{
		string error = "Database not load yet.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	current_database = iter->second;
	doQuery(format, db_query, server, response, request);

}

bool monitor_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is monitor"<<endl;

	string db_name = request->path_match[1];
	db_name = UrlDecode(db_name);

	std::map<std::string, Database *>::iterator iter = databases.find(db_name);
	if(iter == databases.end())
	{
		//cout << "database not loaded yet." << endl;
		string error = "Database not load yet.\r\n";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	Database * _database = iter->second;

	//BETTER: use JSON format to send/receive messages
	//C++ can not deal with JSON directly, JSON2string string2JSON
	string success;
	string name = _database->getName();
	success = success + "database: " + name + "\r\n";
	TYPE_TRIPLE_NUM triple_num = _database->getTripleNum();
	success = success + "triple num: " + Util::int2string(triple_num) + "\r\n";
	TYPE_ENTITY_LITERAL_ID entity_num = _database->getEntityNum();
	success = success + "entity num: " + Util::int2string(entity_num) + "\r\n";
	TYPE_ENTITY_LITERAL_ID literal_num = _database->getLiteralNum();
	success = success + "literal num: " + Util::int2string(literal_num) + "\r\n";
	TYPE_ENTITY_LITERAL_ID sub_num = _database->getSubNum();
	success = success + "subject num: " + Util::int2string(sub_num) + "\r\n";
	TYPE_PREDICATE_ID pre_num = _database->getPreNum();
	success = success + "predicate num: " + Util::int2string(pre_num) + "\r\n";
	//BETTER: how to compute the connection num in Boost::asio?
	int conn_num = connection_num / 2;
	//int conn_num = 3;    //currectly connected sessions
	//this connection num is countint the total(no break)
	success = success + "connection num: " + Util::int2string(conn_num) + "\r\n";
	//TODO: add the info of memory and thread, operation num and IO frequency

	//success = "<p>" + success + "</p>";
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

	return true;
}

bool delete_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout << "HTTP: this is delete" << endl;

	/*
	string download = request->path_match[1];
	download = UrlDecode(download);
	cout << "download: " << download << endl;
	*/
	string filepath = request->path_match[1];
	filepath = UrlDecode(filepath);

	delete_result(server, response, request, filepath);

	return true;
}

bool download_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout << "HTTP: this is download" << endl;

	/*
	string download = request->path_match[1];
	download = UrlDecode(download);
	cout << "download: " << download << endl;
	*/
	string filepath = request->path_match[1];
	filepath = UrlDecode(filepath);

	download_result(server, response, request, filepath);
	//sleep(1);  //seconds
	//cout<<"sleep ends"<<endl;

	return true;
}

bool default_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is default"<<endl;
	string req_url = request->path;
	cout << "request url: " << req_url << endl;
	//if request matches /?operation=monitor, then do monitor_handler
	//because if the user visit through the browser by using url /?operation=monitor
	//it can not match directly to monitor_handler, and will match this default get
	//so we need to check here to do monitor_handler, although the implementation is not perfect enough.
	//it is also used in /?operation=show    /?operation=checkpoint
	if(req_url == "/?operation=monitor")
		monitor_handler(server, response, request);
	else if(req_url == "/?operation=show")
		show_handler(server, response, request);
	else if(req_url == "/?operation=checkpoint")
		checkpoint_handler(server, response, request);
	//BETTER: use lock to ensure thread safe
	if(request->path == "/admin.html" || request->path == "/admin_root.html")
	{
		cout << "request->path is admin.html or admin_root.html" << endl;
		request->path = "/login.html";
		cout << "request->path changed to: " << request->path << endl;
	}

	connection_num++;
	//NOTICE: it seems a visit will output twice times
	//And different pages in a browser is viewed as two connections here
	//cout<<"new connection"<<endl;
	try {
		auto web_root_path=boost::filesystem::canonical("./Server/web");
		auto path=boost::filesystem::canonical(web_root_path/request->path);
		//Check if path is within web_root_path
		if(distance(web_root_path.begin(), web_root_path.end())>distance(path.begin(), path.end()) ||
				!equal(web_root_path.begin(), web_root_path.end(), path.begin()))
			throw invalid_argument("path must be within root path");
		if(boost::filesystem::is_directory(path))
			path/="index.html";
		if(!(boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)))
			throw invalid_argument("file does not exist");

		std::string cache_control, etag;

		// Uncomment the following line to enable Cache-Control
		// cache_control="Cache-Control: max-age=86400\r\n";

		auto ifs=make_shared<ifstream>();
		ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);
		string extName = path.extension().string();
		cout << "extName: " << extName << endl;
		
		if(*ifs) {
			auto length=ifs->tellg();
			ifs->seekg(0, ios::beg);

			*response << "HTTP/1.1 200 OK\r\n" << cache_control << etag << "Content-Length: " << length << "\r\n";
			//!Notice: IE need to declare the Content-type of the response, if not the browser will not show the result with an unknow format.
			//it works well in Firefox and Chrome if you don't declare the Content-type, but in IE the css file will be ignored because of unmatched MIME type, then the web page will show in a form without css style and it looks ugly.
			if(extName == ".html")
				*response << "Content-Type: text/html" << "\r\n\r\n";
			else if(extName == ".js")
				*response << "Content-Type: application/x-javascript" << "\r\n\r\n";
			else if(extName == ".css")
				*response << "Content-Type: text/css" << "\r\n\r\n";
			else if(extName == ".png")
				*response << "Content-Type: image/png" << "\r\n\r\n";
			else if(extName == ".jpg")
				*response << "Content-Type: image/jpeg" << "\r\n\r\n";
			default_resource_send(server, response, ifs);
		}
		else
			throw invalid_argument("could not read file");
	}
	catch(const exception &e) {
		string content="Could not open path "+request->path+": "+e.what();
		*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
	}

	return true;
}
bool check_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is check"<<endl;
	string req_url = request->path;
	cout << "request url: " << req_url << endl;
	string username = request->path_match[1];
	username = UrlDecode(username);
	string password = request->path_match[2];
	password = UrlDecode(password);

	cout << "username = " << username << endl;
	cout << "password = " << password << endl;

	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(it == users.end())
	{
		string error = "wrong username.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	else if(it->second->getPassword() != password)
	{
		string error = "wrong password.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	else
	{
		cout << "login successfully." << endl;
		string success = "check identity successfully.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
		return true;
	}
}
bool login_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is login"<<endl;
	string req_url = request->path;
	cout << "request url: " << req_url << endl;
	string username = request->path_match[1];
	username = UrlDecode(username);
	string password = request->path_match[2];
	password = UrlDecode(password);

	cout << "username = " << username << endl;
	cout << "password = " << password << endl;

	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(it == users.end())
	{
		string error = "wrong username.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	else if(it->second->getPassword() != password)
	{
		string error = "wrong password.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	else
	{
		cout << "login successfully." << endl;
	//NOTICE: it seems a visit will output twice times
	//And different pages in a browser is viewed as two connections here
	//cout<<"new connection"<<endl;
/*
	try {
		auto web_root_path=boost::filesystem::canonical("./Server/web");
		request->path = "/admin.html";
		auto path=boost::filesystem::canonical(web_root_path/request->path);
		cout << "path: " << path << endl;

		std::string cache_control, etag;

		// Uncomment the following line to enable Cache-Control
		// cache_control="Cache-Control: max-age=86400\r\n";

		auto ifs=make_shared<ifstream>();
		ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);
		if(*ifs) {
			cout << "in login 1" << endl;
			auto length=ifs->tellg();
			ifs->seekg(0, ios::beg);

			*response << "HTTP/1.1 200 OK\r\n" << cache_control << etag << "Content-Length: " << length << "\r\n";
			
			*response << "Content-Type: text/html" << "\r\n\r\n";

			default_resource_send(server, response, ifs);
			cout << "in login 2" << endl;
		}
		else
			throw invalid_argument("could not read file");
	}
	catch(const exception &e) {
		cout << "in login 3" << endl;
		string content="Could not open path "+request->path+": "+e.what();
		*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
	}

	return true;
*/
	try {
		auto web_root_path=boost::filesystem::canonical("./Server/web");
		if(username == ROOT_USERNAME)
			request->path = "/admin_root.html";
		else
			request->path = "/admin.html";
		auto path=boost::filesystem::canonical(web_root_path/request->path);
		//Check if path is within web_root_path
		if(distance(web_root_path.begin(), web_root_path.end())>distance(path.begin(), path.end()) ||
				!equal(web_root_path.begin(), web_root_path.end(), path.begin()))
			throw invalid_argument("path must be within root path");
		if(boost::filesystem::is_directory(path))
			path/="login.html";
		if(!(boost::filesystem::exists(path) && boost::filesystem::is_regular_file(path)))
			throw invalid_argument("file does not exist");

		std::string cache_control, etag;

		// Uncomment the following line to enable Cache-Control
		// cache_control="Cache-Control: max-age=86400\r\n";

		auto ifs=make_shared<ifstream>();
		ifs->open(path.string(), ifstream::in | ios::binary | ios::ate);
		string extName = path.extension().string();
		cout << "extName: " << extName << endl;
		
		if(*ifs) {
			auto length=ifs->tellg();
			ifs->seekg(0, ios::beg);

			*response << "HTTP/1.1 200 OK\r\n" << cache_control << etag << "Content-Length: " << length << "\r\n";
			//!Notice: IE need to declare the Content-type of the response, if not the browser will not show the result with an unknow format.
			//it works well in Firefox and Chrome if you don't declare the Content-type, but in IE the css file will be ignored because of unmatched MIME type, then the web page will show in a form without css style and it looks ugly.
			if(extName == ".html")
				*response << "Content-Type: text/html" << "\r\n\r\n";
			else if(extName == ".js")
				*response << "Content-Type: application/x-javascript" << "\r\n\r\n";
			else if(extName == ".css")
				*response << "Content-Type: text/css" << "\r\n\r\n";
			else if(extName == ".png")
				*response << "Content-Type: image/png" << "\r\n\r\n";
			else if(extName == ".jpg")
				*response << "Content-Type: image/jpeg" << "\r\n\r\n";
			default_resource_send(server, response, ifs);
		}
		else
			throw invalid_argument("could not read file");
	}
	catch(const exception &e) {
		string content="Could not open path "+request->path+": "+e.what();
		*response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
	}

	return true;
	}
}
//TODO+BETTER: server choose to save a database when system is not busy
//If user send this command too frequently, the performance may be awful if updates are large
bool checkpoint_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is checkpoint"<<endl;

	string db_name = request->path_match[1];
	db_name = UrlDecode(db_name);

	std::map<std::string, Database *>::iterator iter = databases.find(db_name);
	if(iter == databases.end())
	{
		string error = "Database named " + db_name + "haven't loaded yet.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	current_database = iter->second;

	//For database checkpoint or log/transaction:
	//http://www.newsmth.net/nForum/#!article/LinuxDev/37802?p=2
	//http://blog.csdn.net/cywosp/article/details/8767327
	//http://www.fx114.net/qa-63-143449.aspx
	current_database->save();
	//NOTICE: this info is in header
	string success = "Database saveed successfully.";
	//header and content are split by an empty line
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

	return true;
}

//BETTER+TODO: indicate the db_name when query
bool show_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is show"<<endl;

	string type = request->path_match[1];
	type = UrlDecode(type);
	if(type == "current")
	{
		if(current_database == NULL)
		{
			string error = "No database used.\r\n";
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
			return false;
		}

		//NOTICE: this info is in header
		string success = current_database->getName();
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

		return true;
	}
	else if(type == "all")
	{
		if(databases.empty())
		{
			string error = "No database.\r\n";
			*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
			return false;
		}
		std::map<std::string, Database *>::iterator it;
		string success;
		for(it=databases.begin(); it != databases.end(); it++)
		{
			string database_name = it->first;
			success = success + database_name + "\r\n";
		}
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
		return true;

	}
	else
	{
		string error = "show current database or all databases, please choose current type or all type.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
}
// to add, delete users or modify the privilege of a user, operation be done by the root user
bool user_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout << "HTTP: this is user" << endl;

	string req_url = request->path;
	cout << "request url: " << req_url << endl;
	string type = request->path_match[1];
	type = UrlDecode(type);
	string username1 = request->path_match[2];
	username1 = UrlDecode(username1);
	string password1 = request->path_match[3];
	password1 = UrlDecode(password1);

	cout << "type = " << type << endl;
	cout << "username1 = " << username1 << endl;
	cout << "password1 = " << password1 << endl;
	
	//if root user
	if(username1 == ROOT_USERNAME & password1 == ROOT_PASSWORD)
	{
		string username2 = request->path_match[4];
		username2 = UrlDecode(username2);
		string addtion = request->path_match[5];
		addtion = UrlDecode(addtion);
		string password2 = addtion;
			
		cout << "username2 = " << username2 << endl;
		cout << "password2 = " << password2 << endl;

		if(type == "add_user")
		{
			if(users.find(username2) == users.end())
			{
				cout << "user ready to add." << endl;				
				struct User *temp_user = new User(username2, password2);
				users.insert(pair<std::string, struct User *>(username2, temp_user));
				cout << "user add done." << endl;

			}
			else
			{
				string error = "username already existed, add user failed.";
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
				return false;
			}

		}
		else if(type == "delete_user")
		{	
			std::map<std::string, struct User *>::iterator iter;
			iter = users.find(username2);
			if(iter != users.end() && username2 != ROOT_USERNAME)
			{
				delete iter->second;
				iter->second = NULL;
				users.erase(username2);
			}
			else
			{
				string error;
				if(username2 == ROOT_USERNAME)
					error = "you cannot delete root, delete user failed.";
				else
					error = "username not exist, delete user failed.";
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
				return false;
			}

		}
		else if(type == "add_query" || type == "add_load" || type == "add_unload")
		{
			if(username2 == ROOT_USERNAME)
			{
				string error = "you can't add privilege to root user.";
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
				return false;
			}
			int len = type.length();
			string subType = type.substr(4, len-4);
			cout << "subType: " << subType << endl;
			string db_name = addtion;
			if(addPrivilege(username2, subType, db_name) == 0)
			{
				string error = "add privilege failed.";
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
				return false;
			}
		}
		else if(type == "delete_query" || type == "delete_load" || type == "delete_unload")
		{
			if(username2 == ROOT_USERNAME)
			{
				string error = "you can't delete privilege of root user.";
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
				return false;
			}
			int len = type.length();
			string subType = type.substr(7, len-7);
			cout << "subType: " << subType << endl;
			string db_name = addtion;
			if(delPrivilege(username2, subType, db_name) == 0)
			{
				string error = "delete privilege failed.";
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
				return false;
			}
		}
		string success = "operation succeeded.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
		return true;
	}
	
	//if not root user, no privilege to perform this operation
	string error = "Not root user, no privilege to perform this operation.";
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
	return false;
}

bool showUser_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is showUser"<<endl;

	if(users.empty())
	{
		string error = "No Users.\r\n";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	std::map<std::string, struct User *>::iterator it;
	string success;
	success = "username\tpassword\tquery privilege\tload privilege\tunload privilege\n";

	int i = 0;
	for(it=users.begin(); it != users.end(); it++)
	{
		cout << i << endl;
		i++;
		string username = it->second->getUsername();
		//cout << username << endl;
		success = success + username + "\t";
		//cout << success << endl;
		string password = it->second->getPassword();
		//cout << password << endl;
		success = success + password + "\t";
		//cout << success << endl;
		string query_db = it->second->getQuery();
		//cout << query_db << endl;
		success = success + query_db + "\t";
		//cout << success << endl;
		string load_db = it->second->getLoad();
		//cout << load_db << endl;
		success = success + load_db + "\t";
		//cout << success << endl;
		string unload_db = it->second->getUnload();
		//cout << unload_db << endl;
		success = success + unload_db + "\n";
		//cout << success << endl;
	}

	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
	return true;
}

bool addPrivilege(string username, string type, string db_name)
{
	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(it != users.end())
	{
		if(type == "query")
			it->second->query_priv.insert(db_name);
		else if(type == "load")
			it->second->load_priv.insert(db_name);
		else if(type == "unload")
			it->second->unload_priv.insert(db_name);
		return 1;
	}
	else
		return 0;
}
bool delPrivilege(string username, string type, string db_name)
{
	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(it != users.end())
	{
		if(type == "query" && it->second->query_priv.find(db_name) != it->second->query_priv.end())
		{
			it->second->query_priv.erase(db_name);
			return 1;
		}
		else if(type == "load" && it->second->load_priv.find(db_name) != it->second->load_priv.end())
		{
			it->second->load_priv.erase(db_name);
			return 1;
		}
		else if(type == "unload" && it->second->unload_priv.find(db_name) != it->second->unload_priv.end())
		{
			it->second->unload_priv.erase(db_name);
			return 1;
		}
	}
	return 0;
}
bool checkPrivilege(string username, string type, string db_name)
{
	if(username == ROOT_USERNAME)
		return 1;

	std::map<std::string, struct User *>::iterator it = users.find(username);
	if(type == "query" && it->second->query_priv.find(db_name) != it->second->query_priv.end())
		return 1;
	else if(type == "load" && it->second->load_priv.find(db_name) != it->second->load_priv.end())
		return 1;
	else if(type == "unload" && it->second->unload_priv.find(db_name) != it->second->unload_priv.end())
		return 1;
	else
		return 0;
}
