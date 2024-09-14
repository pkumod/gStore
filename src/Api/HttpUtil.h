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
	static const size_t OnReadFile(void* ptr, size_t size, size_t nmemb, void* stream);
public:
	/**
	* @brief: HTTP POST request
	* @param strUrl: the Url of the request, for example: http://api.gstore.cn
	* @param strPost: json string
	* @param strResponse: content returned
	* @return: returned value
	*/
	static int Post(const std::string& strUrl, const std::string& strPost, const std::string& filename);

	static int Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse);

	static int Post(const std::string& strUrl, const std::map<std::string, std::string>& headers, long timeOut, const std::string& strPost, std::string& strResponse);

	/**
	 * @brief: HTTP POST file request
	 * @param strUrl: the Url of the request, for example: http://api.gstore.cn
	 * @param headers: HTTP head
	 * @param timeOut: operation timeout
	 * @param filePath: upload file
	 * @param params: form data params
	 * @param strResponse: content returned
	 */
	static int PostFile(const std::string& strUrl, const std::map<std::string, std::string>& headers, long timeOut, const std::string& filePath, const std::map<std::string, std::string>& params, std::string& strResponse);

	/**
	* @brief: HTTP GET request
	* @param strUrl: the Url of the request, for example: http://api.gstore.cn
	* @param strResponse: content returned
	* @return: returned value
	*/
	static int Get(const std::string& strUrl, const std::string& filename);

	static int Get(const std::string& strUrl, std::string& strResponse);

	static int Get(const std::string& strUrl, const std::map<std::string, std::string>& headers, std::string& strResponse);

	static httpentities::ShutdownResponse shutdown(const std::string& url, httpentities::ShutdownRequest& request);

	static httpentities::CheckResponse check(const std::string& url, httpentities::CheckRequest& request);

	static httpentities::TestConnectionResponse testConnection(const std::string& url, const bool& inner, httpentities::TestConnectionRequest& request);

	static httpentities::LoadResponse load(const std::string& url, const bool& inner, httpentities::LoadRequest& request);

	static httpentities::ClusterResponse reply(const std::string& url, httpentities::ReplyRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse appendEntries(const std::string& url, httpentities::AppenEntriesRequest& request, const std::string& username, const std::string& password);
};
