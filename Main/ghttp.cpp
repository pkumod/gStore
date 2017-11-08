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

//=============================================================================
bool build_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool load_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool unload_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool query_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool monitor_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool checkpoint_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool show_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool delete_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool download_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);

bool default_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request);
//=============================================================================

Database *current_database = NULL;
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
	}

	time_t cur_time = time(NULL);
	long time_backup = Util::read_backup_time();
	long next_backup = cur_time - (cur_time - time_backup) % Util::gserver_backup_interval + Util::gserver_backup_interval;
	//NOTICE: no need to backup for endpoint
#ifndef ONLY_READ
	scheduler = start_thread(func_scheduler);
#endif



#ifndef SPARQL_ENDPOINT
	//GET-example for the path /?operation=build&db_name=[db_name]&ds_path=[ds_path], responds with the matched string in path
	//i.e. database name and dataset path
	server.resource["^/%3[F|f]operation%3[D|d]build%26db_name%3[D|d](.*)%26ds_path%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		build_handler(server, response, request);
    };

	//GET-example for the path /?operation=load&db_name=[db_name], responds with the matched string in path
    server.resource["^/%3[F|f]operation%3[D|d]load%26db_name%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		load_handler(server, response, request);
	};

    //GET-example for the path /?operation=unload&db_name=[db_name], responds with the matched string in path
    server.resource["^/%3[F|f]operation%3[D|d]unload%26db_name%3[D|d](.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		unload_handler(server, response, request);
    }; 

	//GET-example for the path /?operation=build&db_name=[db_name]&ds_path=[ds_path], responds with the matched string in path
	//i.e. database name and dataset path
	server.resource["^/?operation=build&db_name=(.*)&ds_path=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		build_handler(server, response, request);
    };

	//GET-example for the path /?operation=load&db_name=[db_name], responds with the matched string in path
    server.resource["^/?operation=load&db_name=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		load_handler(server, response, request);
	};

    //GET-example for the path /?operation=unload&db_name=[db_name], responds with the matched string in path
    server.resource["^/?operation=unload&db_name=(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		unload_handler(server, response, request);
    };
#endif
	
	
	//GET-example for the path /?operation=query&format=[format]&sparql=[sparql], responds with the matched string in path
	 server.resource["^/%3[F|f]operation%3[D|d]query%26format%3[D|d](.*)%26sparql%3[D|d](.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	 {
		query_handler(server, response, request);
		// server.resource["^/query/(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    };

	 //GET-example for the path /?operation=query&format=[format]&sparql=[sparql], responds with the matched string in path
	 server.resource["^/?operation=query&format=(.*)&sparql=(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	 {
		query_handler(server, response, request);
		// server.resource["^/query/(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    };

    //NOTICE:this may not be visited by browser directly if the browser does not do URL encode automatically!
	//In programming language, do URL encode first and then call server, then all is ok
	server.resource["^/%3[F|f]operation%3[D|d]monitor$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
	 //server.resource["^/monitor$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		monitor_handler(server, response, request);
    };
 
    //NOTICE:this may not be visited by browser directly if the browser does not do URL encode automatically!
	//In programming language, do URL encode first and then call server, then all is ok
	server.resource["^/?operation=monitor$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
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
    server.resource["^/%3[F|f]operation%3[D|d]checkpoint$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		checkpoint_handler(server, response, request);
    };
	//TODO: use db_name if multiple databases
	server.resource["^/?operation=checkpoint$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		checkpoint_handler(server, response, request);
    };

	//TODO: add user name as parameter, current using or all databases availiable
    server.resource["^/%3[F|f]operation%3[D|d]show$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
	{
		show_handler(server, response, request);
    };

	//TODO: add user name as parameter, current using or all databases availiable
    server.resource["^/?operation=show$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) 
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
	//cout<<"int 0"<<endl;
    if((read_length=ifs->read(&buffer[0], buffer.size()).gcount())>0) {
        response->write(&buffer[0], read_length);
	//cout<<"int 1"<<endl;
        if(read_length==static_cast<streamsize>(buffer.size())) {
	//cout<<"int 2"<<endl;
            server.send(response, [&server, response, ifs](const boost::system::error_code &ec) {
	//cout<<"int 3"<<endl;
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
	db_name = UrlDecode(db_name);
	db_path = UrlDecode(db_path);
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

	if(current_database != NULL)
	{
		string error = "Please unload your database first.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

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

	// string success = db_name + " " + db_path;
	string success = "Import RDF file to database done.";
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

	return true;
}

bool load_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is load"<<endl;
	string db_name = request->path_match[1];
	db_name = UrlDecode(db_name);

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
	if(current_database != NULL)
	{
		//cout << "Please unload your current database first." <<endl;
		string error = "Please unload your current database first.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}
	cout << database << endl;
	current_database = new Database(database);
	bool flag = current_database->load();
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
	string success = "Database loaded successfully.";
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

	return true;
}

bool unload_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is unload"<<endl;
	string db_name = request->path_match[1];
	db_name = UrlDecode(db_name);

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

	delete current_database;
	current_database = NULL;
	string success = "Database unloaded.";
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

	return true;
}

bool query_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
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
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << ansNum_s.length()+filename.length()+success.length()+4;
				*response << "\r\nContent-Type: text/plain";
				*response << "\r\nCache-Control: no-cache" << "\r\nPragma: no-cache" << "\r\nExpires: 0";
				*response  << "\r\n\r\n" << "0+" << rs.ansNum << '+' << filename << '+' << success;
				return true;
			}
			else
			{
				rs.output_limit = 100;
				success = "";
				success = rs.to_str();
				*response << "HTTP/1.1 200 OK\r\nContent-Length: " << ansNum_s.length()+filename.length()+success.length()+4;
				*response << "\r\nContent-Type: text/plain";
				*response << "\r\nCache-Control: no-cache" << "\r\nPragma: no-cache" << "\r\nExpires: 0";
				*response << "\r\n\r\n" << "1+" << rs.ansNum << '+' << filename << '+' << success;
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

bool monitor_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is monitor"<<endl;

	if(current_database == NULL)
	{
		string error = "No database used now.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	//BETTER: use JSON format to send/receive messages
	//C++ can not deal with JSON directly, JSON2string string2JSON
	string success;
	string name = current_database->getName();
	success = success + "database: " + name + "\n";
	TYPE_TRIPLE_NUM triple_num = current_database->getTripleNum();
	success = success + "triple num: " + Util::int2string(triple_num) + "\r\n";
	TYPE_ENTITY_LITERAL_ID entity_num = current_database->getEntityNum();
	success = success + "entity num: " + Util::int2string(entity_num) + "\r\n";
	TYPE_ENTITY_LITERAL_ID literal_num = current_database->getLiteralNum();
	success = success + "literal num: " + Util::int2string(literal_num) + "\r\n";
	TYPE_ENTITY_LITERAL_ID sub_num = current_database->getSubNum();
	success = success + "subject num: " + Util::int2string(sub_num) + "\r\n";
	TYPE_PREDICATE_ID pre_num = current_database->getPreNum();
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

//TODO+BETTER: server choose to save a database when system is not busy
//If user send this command too frequently, the performance may be awful if updates are large
bool checkpoint_handler(const HttpServer& server, const shared_ptr<HttpServer::Response>& response, const shared_ptr<HttpServer::Request>& request)
{
	cout<<"HTTP: this is checkpoint"<<endl;

	if(current_database == NULL)
	{
		string error = "No database used.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

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

	if(current_database == NULL)
	{
		string error = "No database used.";
		*response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		return false;
	}

	//NOTICE: this info is in header
	string success = current_database->getName();
	*response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

	return true;
}


