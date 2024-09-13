#pragma once
#include <curl/curl.h>
#include <string>
#include <cstring>
#include <iostream>
#include "HttpEntities.h"

class HttpUtil
{
public:
	HttpUtil();
	~HttpUtil();
private:
	static bool m_bDebug;

public:
	/**
	* @brief: HTTP POST request
	* @param strUrl: the Url of the request, for example: http://www.baidu.com
	* @param strPost: json string
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

	static httpentities::ShutdownResponse shutdown(const std::string& url, httpentities::ShutdownRequest& request);

	static httpentities::CheckResponse check(const std::string& url, httpentities::CheckRequest& request);

	static httpentities::TestConnectionResponse testConnection(const std::string& url, const bool& inner, httpentities::TestConnectionRequest& request);

	static httpentities::LoadResponse load(const std::string& url, const bool& inner, httpentities::LoadRequest& request);

	static httpentities::ClusterResponse reply(const std::string& url, httpentities::ReplyRequest& request);
};
