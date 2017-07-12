/*=============================================================================
# Filename: ghttp.cpp
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2017-06-15 15:09
# Description: created by lvxin, improved by lijing
=============================================================================*/

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

int initialize();
//Added for the default_resource example
void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
        const shared_ptr<ifstream> &ifs);

pthread_t start_thread(void *(*_function)(void*));
bool stop_thread(pthread_t _thread);
void* func_timer(void* _args);
void* func_scheduler(void* _args);
void thread_sigterm_handler(int _signal_num);

Database *current_database = NULL;
int connection_num = 0;

long next_backup = 0;
pthread_t scheduler = 0;

//DEBUG+TODO: why the result transfered to client has no \n \t??
//using json is a good way to avoid this problem

//TODO+BETTER: port should be optional
//1. admin.html: build/load/query/unload
//2. index.html: only query (maybe load/unload if using multiple databases)
//3. ghttp: can add or not add a db as parameter
//BETTER: How about change HttpConnector into a console?
//
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

int main() {

	Util util;

	while (true) {
		pid_t fpid = fork();

		if (fpid == 0) {
			int ret = initialize();
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
			cerr << "Server stopped abnormally, restarting server..." << endl;
		}

		else {
			cerr << "Failed to start server: deamon fork failure." << endl;
			return -1;
		}

	}

	return 0;
}


int initialize() {
    //HTTP-server at port 9000 using 1 thread
    //Unless you do more heavy non-threaded processing in the resources,
    //1 thread is usually faster than several threads
    HttpServer server;
    //server.config.port=8080;
	server.config.port=9000;
	//cout<<"after server built"<<endl;

    //GET-example for the path /build/[db_name]/[db_path], responds with the matched string in path
    //For instance a request GET /build/db/123 will receive: db 123
    //server.resource["^/build/([a-zA-Z]+[0-9]*)/([a-zA-Z]+/*[a-zA-Z]+[0-9]*.n[a-zA-Z]*[0-9]*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    // server.resource["^/build/([a-zA-Z0-9]+)/([a-zA-Z0-9]+)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    server.resource["^/build/([a-zA-Z0-9]*)/(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		cout<<"HTTP: this is build"<<endl;
        string db_name=request->path_match[1];
        string db_path=request->path_match[2];
        if(db_name=="" || db_path=="")
        {
            string error = "Exactly 2 arguments required!";
            // error = db_name + " " + db_path;
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
            return 0;
        }

        string database = db_name;
        if(database.length() > 3 && database.substr(database.length()-3, 3) == ".db")
        {
            string error = "Your db name to be built should not end with \".db\".";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
            return 0;
        }
	
	//database += ".db";
        string dataset = db_path;

        if(current_database != NULL)
        {
            string error = "Please unload your database first.";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
            return 0;
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
            return 0;
        }

        // string success = db_name + " " + db_path;
        string success = "Import RDF file to database done.";
        *response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
        return 0;
    };

    //GET-example for the path /load/[db_name], responds with the matched string in path
    //For instance a request GET /load/db123 will receive: db123
    server.resource["^/load/(.*)$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		cout<<"HTTP: this is load"<<endl;
        string db_name=request->path_match[1];
	
	

        if(db_name=="")
        {
            string error = "Exactly 1 argument is required!";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
            return 0;
        }

        string database = db_name;
        if(database.length() > 3 && database.substr(database.length()-3, 3) == ".db")
        {
            string error = "Your db name to be built should not end with \".db\".";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
            return 0;
        }
	
	//database += ".db";

        if(current_database != NULL)
        {
            string error = "Please unload your current database first.";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
            return 0;
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
            return 0;
        }
	
		time_t cur_time = time(NULL);
		long time_backup = Util::read_backup_time();
		long next_backup = cur_time - (cur_time - time_backup) % Util::gserver_backup_interval + Util::gserver_backup_interval;
		scheduler = start_thread(func_scheduler);

	//string success = db_name;
       string success = "Database loaded successfully.";
        *response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;

        return 0;
    };

    //GET-example for the path /query/[query_file_path], responds with the matched string in path
    //For instance a request GET /query/db123 will receive: db123
    server.resource["^/query/(.*)$"]["GET"] = [&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		//TODO: add a format parameter, better to change tp "?format=json"
		string format = "json";
		cout<<"HTTP: this is query"<<endl;
        string db_query=request->path_match[1];
		cout<<"check: "<<db_query<<endl;
		db_query = UrlDecode(db_query);
		cout<<"check: "<<db_query<<endl;
        string str = db_query;

        if(current_database == NULL)
        {
            string error = "No database in use!";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
            return 0;
        };

        string sparql;

        if(db_query[0]=='\"')
        {
            sparql = db_query.substr(1, db_query.length()-2);
			cout<<"check: this is string "<<sparql<<endl;
        }
        else
        {
            string ret = Util::getExactPath(db_query.c_str());
            const char *path = ret.c_str();
            if(path == NULL)
            {
                string error = "Invalid path of query.";
                *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
                return 0;
            }
            sparql = Util::getQueryFromFile(path);
			cout<<"check: this is path "<<sparql<<endl;
        }

        if (sparql.empty()) {
            cerr << "Empty SPARQL." << endl;
            return 0;
        }

        //FILE* output = stdout;
        FILE* output = NULL; //not update result on the screen

		pthread_t timer = start_thread(func_timer);
		if (timer == 0) {
			cerr << "Failed to start timer." << endl;
		}

        ResultSet rs;
        bool ret = current_database->query(sparql, rs, output);
		if (timer != 0 && !stop_thread(timer)) {
			cerr << "Failed to stop timer." << endl;
		}

        if(ret)
        {
			//TODO: if the result is too large? or if the result is placed in Stream?
			//Should use file donload
            string success = "";
			if(format == "json")
			{
				success = rs.to_JSON();
			}
			else
			{
				success = rs.to_str();
			}
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
            return 0;
        }
        else
        {
			//TODO: if error, browser should give some prompt
            string error = "query() returns false.";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
            return 0;
        }
    };

    //GET-example for the path /unload/[db_name], responds with the matched string in path
    //For instance a request GET /unload/db123 will receive: db123
    server.resource["^/unload$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		cout<<"HTTP: this is unload"<<endl;
        if(current_database == NULL)
        {
            string error = "No database used now.";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
            return 0;
        }

		if (scheduler != 0 && !stop_thread(scheduler)) {
			cerr << "Failed to stop scheduler." << endl;
		}
		else {
			scheduler = 0;
			next_backup = 0;
		}

        delete current_database;
        current_database = NULL;
        string success = "Database unloaded.";
        *response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
        return 0;
    };

    server.resource["^/monitor$"]["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		cout<<"HTTP: this is monitor"<<endl;
        if(current_database == NULL)
        {
            string error = "No database used now.";
            *response << "HTTP/1.1 200 OK\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
            return 0;
        }

		//BETTER: use JSON format to send/receive messages
		//C++ can not deal with JSON directly, JSON2string string2JSON
        string success;
		string name = current_database->getName();
		success = success + "database: " + name + "\n";
		TYPE_TRIPLE_NUM triple_num = current_database->getTripleNum();
		success = success + "triple num: " + Util::int2string(triple_num) + "\n";
		TYPE_ENTITY_LITERAL_ID entity_num = current_database->getEntityNum();
		success = success + "entity num: " + Util::int2string(entity_num) + "\n";
		TYPE_ENTITY_LITERAL_ID literal_num = current_database->getLiteralNum();
		success = success + "literal num: " + Util::int2string(literal_num) + "\n";
		TYPE_ENTITY_LITERAL_ID sub_num = current_database->getSubNum();
		success = success + "subject num: " + Util::int2string(sub_num) + "\n";
		TYPE_PREDICATE_ID pre_num = current_database->getPreNum();
		success = success + "predicate num: " + Util::int2string(pre_num) + "\n";
		//BETTER: how to compute the connection num in Boost::asio?
		int conn_num = connection_num / 2;
		//int conn_num = 3;    //currectly connected sessions
		//this connection num is countint the total(no break)
		success = success + "connection num: " + Util::int2string(conn_num) + "\n";
		//TODO: add the info of memory and thread, operation num and IO frequency

		//success = "<p>" + success + "</p>";
        *response << "HTTP/1.1 200 OK\r\nContent-Length: " << success.length() << "\r\n\r\n" << success;
        return 0;
    };
    
    // server.resource["^/json$"]["POST"]=[](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
    //     try {
    //         ptree pt;
    //         read_json(request->content, pt);

    //         string name=pt.get<string>("firstName")+" "+pt.get<string>("lastName");

    //         *response << "HTTP/1.1 200 OK\r\n"
    //             << "Content-Type: application/json\r\n"
    //             << "Content-Length: " << name.length() << "\r\n\r\n"
    //             << name;
    //     }
    //     catch(exception& e) {
    //         *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << strlen(e.what()) << "\r\n\r\n" << e.what();
    //     }
    // };

    //Default GET-example. If no other matches, this anonymous function will be called.
    //Will respond with content in the web/-directory, and its subdirectories.
    //Default file: index.html
    //Can for instance be used to retrieve an HTML 5 client that uses REST-resources on this server
    server.default_resource["GET"]=[&server](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> request) {
		//BETTER: use lock to ensure thread safe
		cout<<"HTTP: this is default"<<endl;
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

            if(*ifs) {
                auto length=ifs->tellg();
                ifs->seekg(0, ios::beg);

                *response << "HTTP/1.1 200 OK\r\n" << cache_control << etag << "Content-Length: " << length << "\r\n\r\n";
                default_resource_send(server, response, ifs);
            }
            else
                throw invalid_argument("could not read file");
        }
        catch(const exception &e) {
            string content="Could not open path "+request->path+": "+e.what();
            *response << "HTTP/1.1 400 Bad Request\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
        }
    };

    thread server_thread([&server](){
            //Start server
            server.start();
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

    return 0;
}

void default_resource_send(const HttpServer &server, const shared_ptr<HttpServer::Response> &response,
        const shared_ptr<ifstream> &ifs) {
    //read and send 128 KB at a time
    static vector<char> buffer(131072); // Safe when server is running on one thread
    streamsize read_length;
    if((read_length=ifs->read(&buffer[0], buffer.size()).gcount())>0) {
        response->write(&buffer[0], read_length);
        if(read_length==static_cast<streamsize>(buffer.size())) {
            server.send(response, [&server, response, ifs](const boost::system::error_code &ec) {
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
