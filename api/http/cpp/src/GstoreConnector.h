/*
# Filename: GstoreConnector.h
# Author: suxunbin
# Last Modified: 2021-07-19 12:56
# Description: http api for C++
*/

#ifndef __HTTP_CURL_H__
#define __HTTP_CURL_H__

//REFERENCE: https://curl.haxx.se/
//libcurl is useful for developing http client, but not for server
//
//TODO: deal with cookie
//URL encode: http://www.ruanyifeng.com/blog/2010/02/url_encoding.html

#include <curl/curl.h>
#include <string>
#include <cstring>
#include <iostream>

#define defaultServerIP "127.0.0.1"
#define defaultServerPort 9000

class GstoreConnector
{
public:
	std::string serverIP;
	int serverPort;
	std::string Url;
	std::string username;
	std::string password;
	GstoreConnector(void);
	GstoreConnector(std::string _ip, int _port, std::string _http_type, std::string _user, std::string _passwd);
	~GstoreConnector(void);

public:
	/**
	* @brief: HTTP POST request
	* @param strUrl: the Url of the request, for example: http://www.baidu.com
	* @param strPost: input format: para1=val1?para2=val2&��
	* @param strResponse: content returned
	* @return: returned value
	*/

	int Post(const std::string& strUrl, const std::string& strPost, const std::string& filename);

	int Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse);

	/**
	* @brief: HTTP GET request
	* @param strUrl: the Url of the request, for example: http://www.baidu.com
	* @param strResponse: content returned
	* @return: returned value
	*/

	int Get(const std::string& strUrl, const std::string& filename);

	int Get(const std::string& strUrl, std::string& strResponse);

	/**
	* @brief: HTTPS POST request (uncertified version)
	* @param strUrl: the Url of the request, for example: https://www.alipay.com
	* @param strPost: input format: para1=val1?para2=val2&��
	* @param strResponse: content returned
	* @param pCaPath: the path to the CA certificate. If NULL, the server-side certificate is not validated.
	* @return: returned value
	*/

	int Posts(const std::string& strUrl, const std::string& strPost, std::string& strResponse, const char* pCaPath = NULL);

	/**
	* @brief: HTTPS GET request (uncertified version)
	* @param strUrl: the Url of the request, for example: https://www.alipay.com
	* @param strResponse: content returned
	* @param pCaPath: the path to the CA certificate. If NULL, the server-side certificate is not validated.
	* @return: returned value
	*/

	int Gets(const std::string& strUrl, std::string& strResponse, const char* pCaPath = NULL);

public:
	std::string build(std::string db_name, std::string db_path, std::string request_type = "GET");
	std::string load(std::string db_name, std::string csr = "0", std::string request_type = "GET");
	std::string unload(std::string db_name, std::string request_type = "GET");
	std::string monitor(std::string db_name, std::string request_type = "GET");
	std::string drop(std::string db_name, bool is_backup, std::string request_type = "GET");
	std::string show(std::string request_type = "GET");
	std::string usermanage(std::string type, std::string op_username, std::string op_password, std::string request_type = "GET");
	std::string showuser(std::string request_type = "GET");
	std::string userprivilegemanage(std::string type, std::string op_username, std::string privileges, std::string db_name, std::string request_type = "GET");
	std::string backup(std::string db_name, std::string backup_path, std::string request_type = "GET");
	std::string restore(std::string db_name, std::string backup_path, std::string request_type = "GET");
	std::string query(std::string db_name, std::string format, std::string sparql, std::string request_type = "GET");
	void fquery(std::string db_name, std::string format, std::string sparql, std::string filename, std::string request_type = "GET");
	std::string exportDB(std::string db_name, std::string db_path, std::string request_type = "GET");
	std::string login(std::string request_type = "GET");
	std::string check(std::string request_type = "GET");
	std::string begin(std::string db_name, std::string isolevel, std::string request_type = "GET");
	std::string tquery(std::string db_name, std::string tid, std::string sparql, std::string request_type = "GET");
	std::string commit(std::string db_name, std::string tid, std::string request_type = "GET");
	std::string rollback(std::string db_name, std::string tid, std::string request_type = "GET");
	std::string getTransLog(std::string request_type = "GET");
	std::string checkpoint(std::string db_name, std::string request_type = "GET");
	void SetDebug(bool bDebug);

private:
	bool m_bDebug;
};

#endif
