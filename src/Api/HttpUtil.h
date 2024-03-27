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

class HttpUtil
{
public:
	std::string serverIP;
	int serverPort;
	std::string Url;
	std::string username;
	std::string password;
	HttpUtil(void);
	HttpUtil(std::string _ip, int _port, std::string _http_type, std::string _user, std::string _passwd);
	~HttpUtil(void);
private:
	static bool m_bDebug;

public:
	/**
	* @brief: HTTP POST request
	* @param strUrl: the Url of the request, for example: http://www.baidu.com
	* @param strPost: input format: para1=val1?para2=val2&��
	* @param strResponse: content returned
	* @return: returned value
	*/

	static int Post(const std::string& strUrl, const std::string& strPost, const std::string& filename);

	static int Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse);

	/**
	* @brief: HTTP GET request
	* @param strUrl: the Url of the request, for example: http://www.baidu.com
	* @param strResponse: content returned
	* @return: returned value
	*/

	static int Get(const std::string& strUrl, const std::string& filename);

	static int Get(const std::string& strUrl, std::string& strResponse);
};

#endif
