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
//#include "../../Util/IPWhiteList.h"
//#include "../../Util/IPBlackList.h"

using namespace std;
int loadCSR = 0;
int blackList = 0;
int whiteList = 0;

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

void handler_build(protocol::HttpRequest* req, protocol::HttpResponse* resp)
{

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
				return "the param has no value.";
			}
			else
			{
				return argv[i + 1];
			}

		}

	}
	printf("the param is not exist,using the default value:" + default_value);
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

	WFHttpServer server(process);
	port = atoi(portstr);
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

