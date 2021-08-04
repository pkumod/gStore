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
#include "WebUrl.h"
#include <iostream>
#include "../Util/Util_New.h"
#include "../Database/Database.h"
#include "../Database/Txn_manager.h"
//#include "../../Util/IPWhiteList.h"
//#include "../../Util/IPBlackList.h"

using namespace std;
int loadCSR = 0;
int blackList = 0;
int whiteList = 0;

struct DBInfo {
private:
	std::string db_name;
	std::string creator;
	std::string built_time;
public:
	pthread_rwlock_t db_lock;

	DBInfo() {
		pthread_rwlock_init(&db_lock, NULL);
	}
	DBInfo(string _db_name, string _creator, string _time) {
		db_name = _db_name;
		creator = _creator;
		built_time = _time;
		pthread_rwlock_init(&db_lock, NULL);
	}
	DBInfo(string _db_name) {
		db_name = _db_name;
		pthread_rwlock_init(&db_lock, NULL);
	}
	~DBInfo() {
		pthread_rwlock_destroy(&db_lock);
	}
	std::string getName() {
		return db_name;
	}
	std::string getCreator() {
		return creator;
	}
	void setCreator(string _creator) {
		creator = _creator;
	}
	std::string getTime() {
		return built_time;
	}
	void setTime(string _time) {
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

	User() {
		pthread_rwlock_init(&query_priv_set_lock, NULL);
		pthread_rwlock_init(&update_priv_set_lock, NULL);
		pthread_rwlock_init(&load_priv_set_lock, NULL);
		pthread_rwlock_init(&unload_priv_set_lock, NULL);
		pthread_rwlock_init(&backup_priv_set_lock, NULL);
		pthread_rwlock_init(&restore_priv_set_lock, NULL);
		pthread_rwlock_init(&export_priv_set_lock, NULL);
	}
	User(string _username, string _password) {
		if (_username == "")
			username = "root";
		else
			username = _username;
		if (_password == "")
			password = "123456";
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
	~User() {
		pthread_rwlock_destroy(&query_priv_set_lock);
		pthread_rwlock_destroy(&update_priv_set_lock);
		pthread_rwlock_destroy(&load_priv_set_lock);
		pthread_rwlock_destroy(&unload_priv_set_lock);
		pthread_rwlock_destroy(&backup_priv_set_lock);
		pthread_rwlock_destroy(&restore_priv_set_lock);
		pthread_rwlock_destroy(&export_priv_set_lock);
	}
	std::string getPassword() {
		return password;
	}
	std::string getUsername() {
		return username;
	}
	std::string getQuery() {
		std::string query_db;
		if (username == ROOT_USERNAME)
		{
			query_db = "all";
			return query_db;
		}
		std::set<std::string>::iterator it = query_priv.begin();
		while (it != query_priv.end())
		{
			query_db = query_db + *it + " ";
			++it;
		}
		return query_db;
	}
	std::string getUpdate() {
		std::string update_db;
		if (username == ROOT_USERNAME)
		{
			update_db = "all";
			return update_db;
		}
		std::set<std::string>::iterator it = update_priv.begin();
		while (it != update_priv.end())
		{
			update_db = update_db + *it + " ";
			++it;
		}
		return update_db;
	}
	std::string getLoad() {
		std::string load_db;
		if (username == ROOT_USERNAME)
		{
			load_db = "all";
			return load_db;
		}

		std::set<std::string>::iterator it = load_priv.begin();
		while (it != load_priv.end())
		{
			load_db = load_db + *it + " ";
			++it;
		}
		return load_db;
	}
	std::string getUnload() {
		std::string unload_db;
		if (username == ROOT_USERNAME)
		{
			unload_db = "all";
			return unload_db;
		}

		std::set<std::string>::iterator it = unload_priv.begin();
		while (it != unload_priv.end())
		{
			unload_db = unload_db + *it + " ";
			++it;
		}
		return unload_db;
	}
	std::string getrestore() {
		std::string restore_db;
		if (username == ROOT_USERNAME)
		{
			restore_db = "all";
			return restore_db;
		}
		std::set<std::string>::iterator it = restore_priv.begin();
		while (it != restore_priv.end())
		{
			restore_db = restore_db + *it + " ";
			++it;
		}
		return restore_db;
	}
	std::string getbackup() {
		std::string backup_db;
		if (username == ROOT_USERNAME)
		{
			backup_db = "all";
			return backup_db;
		}
		std::set<std::string>::iterator it = backup_priv.begin();
		while (it != backup_priv.end())
		{
			backup_db = backup_db + *it + " ";
			++it;
		}
		return backup_db;
	}
	std::string getexport() {
		std::string export_db;
		if (username == ROOT_USERNAME)
		{
			export_db = "all";
			return export_db;
		}
		std::set<std::string>::iterator it = export_priv.begin();
		while (it != export_priv.end())
		{
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

//IPWhiteList* ipWhiteList;
//IPBlackList* ipBlackList;

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

void handler_build(protocol::HttpRequest* req, protocol::HttpResponse* resp,
	string db_name,string db_path,string username,string password)
{
	cout << "HTTP: this is build" << endl;
	db_name = UrlDecode(db_name);
	db_path = UrlDecode(db_path);
	username = UrlDecode(username);
	password = UrlDecode(password);
	


}

int initialize(int argc, char* argv[])
{
	cout << "enter initialize." << endl;
	//Server restarts to use the original database
	//current_database = NULL;

	//users.insert(pair<std::string, struct User *>(ROOT_USERNAME, &root));

	//load system.db when initialize
	string current_path = Util_New::getCurrentRootPath();
	cout << "the current path is:" << current_path << endl;
	if (Util_New::checkFileOrDirIsExist("system.db")==false)
	{
		cout << "Can not find system.db." << endl;
		return -1;
	}
	struct DBInfo* temp_db = new DBInfo("system");
	temp_db->setCreator("root");
	already_build.insert(pair<std::string, struct DBInfo*>("system", temp_db));
	std::map<std::string, struct DBInfo*>::iterator it_already_build = already_build.find("system");
	if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0)
	{
		cout << "Unable to load the database system.db due to loss of lock." << endl;
		return -1;
	}
	system_database = new Database("system");
	bool flag = system_database->load();
	if (!flag)
	{
		cout << "Failed to load the database system.db." << endl;


		return -1;
	}
	databases.insert(pair<std::string, Database*>("system", system_database));
	pthread_rwlock_unlock(&(it_already_build->second->db_lock));

	
	//DB2Map();
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

	/* Set response message body. */
	resp->append_output_body_nocopy("<html>", 6);
	len = snprintf(buf, 8192, "<p>%s %s %s</p>", req->get_method(),
		req->get_request_uri(), req->get_http_version());
	resp->append_output_body(buf, len);
	string uri = req->get_request_uri();
	WebUrl web(uri);


	len = snprintf(buf, 8192, "<p>dbname=%s</p>", web.Request("dbname").c_str());
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

	string operation = web.Request("operation");
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
/// 启动API Server 启动命令 apiserver -p 9999 -ipallow y -ipdeny y -db lubm -advanced y
/// 其中参数如下：
///    -p（必须)：端口
///    -db(可选):默认为system
///    -ipallow(可选):ip白名单
///    -ipdeny(可选):ip黑名单
///    -advanced(可选):是否启用高级功能（load CSR)
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
	initialize(argc, argv);
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

