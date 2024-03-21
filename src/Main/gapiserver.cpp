/*
  Copyright (c) 2019 Sogou, Inc.

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

	  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  Author: Xie Han (xiehan@sogou-inc.com;63350856@qq.com)
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include "workflow/HttpMessage.h"
#include "workflow/HttpUtil.h"
#include "workflow/WFServer.h"
#include "workflow/WFHttpServer.h"
#include "workflow/WFFacilities.h"
#include "../Util/WebUrl.h"
#include <iostream>
#include "../Database/Database.h"
#include "../Database/Txn_manager.h"
#include "../Util/Util.h"
#include "../tools/rapidjson/document.h"
#include "../tools/rapidjson/prettywriter.h"  
#include "../tools/rapidjson/writer.h"
#include "../tools/rapidjson/stringbuffer.h"
#include <fstream>
#include "../Util/IPWhiteList.h"
#include "../Util/IPBlackList.h"
//#include "../../Util/IPWhiteList.h"
//#include "../../Util/IPBlackList.h"

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
using namespace std;
int loadCSR = 0;
int blackList = 0;
int whiteList = 0;





//IPWhiteList* ipWhiteList;
//IPBlackList* ipBlackList;

string UrlDecode(string& SRC)
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

bool ipCheck(WFHttpTask* server_task, protocol::HttpRequest* req, protocol::HttpResponse* resp) {
	//get the real IP of the client, because we use nginx here
	char addrstr[128];
	struct sockaddr_storage addr;
	socklen_t l = sizeof addr;
	unsigned short port = 0;

	server_task->get_peer_addr((struct sockaddr*)&addr, &l);
	if (addr.ss_family == AF_INET)
	{
		struct sockaddr_in* sin = (struct sockaddr_in*)&addr;
		inet_ntop(AF_INET, &sin->sin_addr, addrstr, 128);
		port = ntohs(sin->sin_port);
	}
	else if (addr.ss_family == AF_INET6)
	{
		struct sockaddr_in6* sin6 = (struct sockaddr_in6*)&addr;
		inet_ntop(AF_INET6, &sin6->sin6_addr, addrstr, 128);
		port = ntohs(sin6->sin6_port);
	}
	else
		strcpy(addrstr, "Unknown");


	string remote_ip = addrstr;

	cout << "remote_ip: " << remote_ip << endl;
	//check if the ip is allow or denied
	/*if (whiteList == 1) {
		return ipWhiteList->Check(remote_ip);
	}
	else if (blackList == 1) {
		return ipBlackList->Check(remote_ip);
	}*/
	return true;
}



void handler_build(protocol::HttpRequest* req, protocol::HttpResponse* resp,
	string db_name,string db_path,string username,string password)
{
	cout << "HTTP: this is build" << endl;
	db_name = UrlDecode(db_name);
	db_path = UrlDecode(db_path);
	username = UrlDecode(username);
	password = UrlDecode(password);
	


}

void build_handler(string url)
{

}

string querySys(string sparql)
{
	string db_name = "system";
	//pthread_rwlock_rdlock(&already_build_map_lock);
	//std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find(db_name);
	//pthread_rwlock_unlock(&already_build_map_lock);

	//pthread_rwlock_rdlock(&(it_already_build->second->db_lock));
	ResultSet rs;
	FILE* output = NULL;
	Database* system_database;
	system_database = new Database("system");
	bool flag = system_database->load();
	int ret_val = system_database->query(sparql, rs, output);
	bool ret = false, update = false;
	if (ret_val < -1)   //non-update query
	{
		ret = (ret_val == -100);
	}
	else  //update query, -1 for error, non-negative for num of triples updated
	{
		update = true;
	}

	if (ret)
	{
		cout << "search query returned successfully." << endl;

		string success = rs.to_JSON();
		//pthread_rwlock_unlock(&(it_already_build->second->db_lock));
		return success;
	}
	else
	{
		string error = "";
		int error_code;
		if (!update)
		{
			cout << "search query returned error." << endl;
			error = "search query returns false.";
			error_code = 403;
		}

		//pthread_rwlock_unlock(&(it_already_build->second->db_lock));

		return error;
	}

}

void DB2Map()
{
	//string sparql = "select ?x ?y where{?x <has_password> ?y.}";
	//string strJson = querySys(sparql);
	////cout << "DDDDDDDDDDDDDDDB2Map: strJson : " << strJson << endl;
	//Document document;
	//document.Parse(strJson.c_str());
	//Value& p1 = document["results"];
	//Value& p2 = p1["bindings"];
	////int i = 0;
	//for (int i = 0; i < p2.Size(); i++)
	//{
	//	Value& pp = p2[i];
	//	Value& pp1 = pp["x"];
	//	Value& pp2 = pp["y"];
	//	string username = pp1["value"].GetString();
	//	string password = pp2["value"].GetString();
	//	//cout << "DDDDDDDDDDDDDDDDB2Map: username: " + username << " password: " << password << endl;
	//	struct User* user = new User(username, password);

	//	string sparql2 = "select ?x ?y where{<" + username + "> ?x ?y.}";
	//	string strJson2 = querySys(sparql2);
	//	//cout << "strJson2: " << strJson2 << endl;
	//	Document document2;
	//	document2.Parse(strJson2.c_str());

	//	Value& p12 = document2["results"];
	//	Value& p22 = p12["bindings"];
	//	for (int j = 0; j < p22.Size(); j++)
	//	{
	//		Value& ppj = p22[j];
	//		Value& pp12 = ppj["x"];
	//		Value& pp22 = ppj["y"];
	//		string type = pp12["value"].GetString();
	//		string db_name = pp22["value"].GetString();
	//		//cout << "DDDDDDDDDDDDDDDDDB2Map: type: " + type << " db_name: " << db_name << endl;

	//		if (type == "has_query_priv")
	//		{
	//			//cout << username << type << db_name << endl;
	//			user->query_priv.insert(db_name);
	//		}
	//		else if (type == "has_update_priv")
	//		{
	//			//cout << username << type << db_name << endl;
	//			user->update_priv.insert(db_name);
	//		}
	//		else if (type == "has_load_priv")
	//		{
	//			user->load_priv.insert(db_name);
	//		}
	//		else if (type == "has_unload_priv")
	//		{
	//			user->unload_priv.insert(db_name);
	//		}
	//		else if (type == "has_restore_priv")
	//		{
	//			user->restore_priv.insert(db_name);
	//		}
	//		else if (type == "has_backup_priv")
	//		{
	//			user->backup_priv.insert(db_name);
	//		}
	//		else if (type == "has_export_priv")
	//		{
	//			user->export_priv.insert(db_name);
	//		}
	//	}
	//	//users.insert(pair<std::string, struct User*>(username, &user));
	//	users.insert(pair<std::string, struct User*>(username, user));

	//	//cout << ".................." << user->getUsername() << endl;
	//	//cout << ".................." << user->getPassword() << endl;
	//	//cout << ".................." << user->getLoad() << endl;
	//	//cout << ".................." << user->getQuery() << endl;
	//	//cout << ".................." << user->getUnload() << endl;
	//	//cout << "i: " << i << endl;
	//	//i++;
	//}
	////cout << "out of first ptree" << endl;

	////insert already_built database from system.db to already_build map
	//sparql = "select ?x where{?x <database_status> \"already_built\".}";
	//strJson = querySys(sparql);
	//document.Parse(strJson.c_str());
	//p1 = document["results"];
	//p2 = p1["bindings"];

	//for (int i = 0; i < p2.Size(); i++)
	//{
	//	Value& pp = p2[i];
	//	Value& pp1 = pp["x"];
	//	string db_name = pp1["value"].GetString();
	//	struct DBInfo* temp_db = new DBInfo(db_name);

	//	string sparql2 = "select ?x ?y where{<" + db_name + "> ?x ?y.}";
	//	string strJson2 = querySys(sparql2);
	//	Document document2;
	//	document2.Parse(strJson2.c_str());

	//	Value& p12 = document2["results"];
	//	Value& p22 = p12["bindings"];

	//	for (int j = 0; j < p22.Size(); j++)
	//	{
	//		Value& ppj = p22[j];
	//		Value& pp12 = ppj["x"];
	//		Value& pp22 = ppj["y"];
	//		string type = pp12["value"].GetString();
	//		string info = pp22["value"].GetString();

	//		if (type == "built_by")
	//			temp_db->setCreator(info);
	//		else if (type == "built_time")
	//			temp_db->setTime(info);
	//	}
	//	already_build.insert(pair<std::string, struct DBInfo*>(db_name, temp_db));
	//}

	////add bulit_time of system.db to already_build map
	//sparql = "select ?x where{<system> <built_time> ?x.}";
	//strJson = querySys(sparql);
	//document.Parse(strJson.c_str());
	//p1 = document["results"];
	//p2 = p1["bindings"];

	//for (int i = 0; i < p2.Size(); i++)
	//{
	//	Value& pp = p2[i];
	//	Value& pp1 = pp["x"];
	//	string built_time = pp1["value"].GetString();
	//	already_build.find("system")->second->setTime(built_time);
	//}

	////get CoreVersion and APIVersion
	//sparql = "select ?x where{<CoreVersion> <value> ?x.}";
	//strJson = querySys(sparql);
	//document.Parse(strJson.c_str());
	//p1 = document["results"];
	//p2 = p1["bindings"];
	//for (int i = 0; i < p2.Size(); i++)
	//{
	//	Value& pp = p2[i];
	//	Value& pp1 = pp["x"];
	//	CoreVersion = pp1["value"].GetString();
	//}
	//sparql = "select ?x where{<APIVersion> <value> ?x.}";
	//strJson = querySys(sparql);
	//document.Parse(strJson.c_str());
	//p1 = document["results"];
	//p2 = p1["bindings"];
	//for (int i = 0; i < p2.Size(); i++)
	//{
	//	Value& pp = p2[i];
	//	Value& pp1 = pp["x"];
	//	APIVersion = pp1["value"].GetString();
	//}
}
string show_handler(string url)
{
	string username = WebUrl::CutParam(url, "username");
	string password = WebUrl::CutParam(url, "password");
	username = UrlDecode(username);
	password = UrlDecode(password);
	string sparql = "select ?x where{<CoreVersion> <value> ?x.}";
	string result = querySys(sparql);
	return result;
	

}
void processGetMethod(WFHttpTask* server_task)
{
	protocol::HttpRequest* req = server_task->get_req();
	protocol::HttpResponse* resp = server_task->get_resp();
	string uri = req->get_request_uri();
	string operation = WebUrl::CutParam(uri, "operation");
	cout << "operation:" << operation << endl;
	string result = "";
	char buf[8192];
	int len;
	if (operation == "build")
	{

	}
	else if (operation == "showVersion")
	{
		cout << "begin show the version:" << endl;
		 result=show_handler(uri);
		 cout << "show Version result is:" << result << endl;
		 len = snprintf(buf, 8192, "<p>the version is=%s</p>", result.c_str());
		 resp->append_output_body(buf, len);
		
	}
	
	
}



void process(WFHttpTask* server_task)
{
	protocol::HttpRequest* req = server_task->get_req();
	protocol::HttpResponse* resp = server_task->get_resp();
	long long seq = server_task->get_task_seq();
	protocol::HttpHeaderCursor cursor(req);
	std::string name;
	std::string value;
	char buf[8192];
	int len;
	cout << "method:"<<req->get_method() << endl;


	const char* method_char = req->get_method();
	string method(method_char);
	if (method == "GET")
	{
		cout << "handler the get request" << endl;
		processGetMethod(server_task);
	}

	/* Set response message body. */
	resp->append_output_body_nocopy("<html>", 6);
	len = snprintf(buf, 8192, "<p>%s %s %s</p>", req->get_method(),
		req->get_request_uri(), req->get_http_version());
	resp->append_output_body(buf, len);
	string uri = req->get_request_uri();


	string type = "dbname";
	len = snprintf(buf, 8192, "<p>dbname=%s</p>", WebUrl::CutParam(uri,"dbname").c_str());
	resp->append_output_body(buf, len);
	while (cursor.next(name, value))
	{
		len = snprintf(buf, 8192, "<p>%s: %s</p>", name.c_str(), value.c_str());
		resp->append_output_body(buf, len);
	}

	resp->append_output_body_nocopy("</html>", 7);

	/* Set status line if you like. */

	if (seq == 9) /* no more than 10 requests on the same connection. */
		resp->add_header_pair("Connection", "close");

	/* print some log */
	bool check = ipCheck(server_task, req, resp);

	resp->set_http_version("HTTP/1.1");
	resp->set_status_code("200");
	resp->set_reason_phrase("OK");

	resp->add_header_pair("Content-Type", "text/html");
	resp->add_header_pair("Server", "Sogou WFHttpServer");
	
	string operation = WebUrl::CutParam(uri, "operation");
	cout << "operation is :" << operation << endl;


}

static WFFacilities::WaitGroup wait_group(1);

void sig_handler(int signo)
{
	wait_group.done();
}


string getArgValue(int argc, char* argv[], string argname, string default_value)
{
	for (int i = 0; i < argc; i++)
	{
		if (argv[i] == "-" + argname)
		{
			if (i + 1 >= argc)
			{
				return argname+" has no value.";
			}
			else
			{
				return argv[i + 1];
			}

		}

	}
	cout<<argname<<" is not exist,using the default value:"<<default_value<<endl;
	return default_value;

}

/// <summary>
/// ����API Server �������� apiserver -p 9999 -ipallow y -ipdeny y -db lubm -advanced y
/// ���в������£�
///    -p������)���˿�
///    -db(��ѡ):Ĭ��Ϊsystem
///    -ipallow(��ѡ):ip������
///    -ipdeny(��ѡ):ip������
///    -advanced(��ѡ):�Ƿ����ø߼����ܣ�load CSR)
///    
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, char* argv[])
{
	unsigned short port;

	if (argc < 2)
	{
		fprintf(stderr, "USAGE: %s <port>\n", argv[0]);
		exit(1);
	}

	string portstr = getArgValue(argc, argv, "p", "9999");

	string ipallow = getArgValue(argc, argv, "ipallow", "n");

	string ipdeny = getArgValue(argc, argv, "ipdeny", "n");

	string advanced = getArgValue(argc, argv, "advanced", "n");

	cout<< "  __ _ ___| |_ ___  _ __ ___" << endl;
	cout<< " / _` / __| __/ _ \| '__/ _" << endl;
	cout<< " | (_| \__ \ || (_) | | |  __/" << endl;
	cout << " \__, |___/\__\___/|_|  \___|" << endl;
	cout << " |___/" << endl;
	



	/*if (ipallow != "n")
	{
		whiteList = 1;
		cout << "IP white List enabled." << endl;
		ipWhiteList = new IPWhiteList();
		ipWhiteList->Load(ipallow);
	}
	if (ipdeny != "n")
	{
		blackList = 1;
		cout << "IP black list enabled." << endl;
		ipBlackList = new IPBlackList();
		ipBlackList->Load(ipdeny);
	}*/

	if (advanced == "y")
	{
		loadCSR = 1;

	}

	signal(SIGINT, sig_handler);
	//initialize(argc, argv);
	WFHttpServer server(process);
	port = atoi(portstr.c_str());
	
	if (server.start(port) == 0)
	{
		wait_group.wait();
		server.stop();
	}
	else
	{
		perror("Cannot start server");
		exit(1);
	}

	return 0;
}

