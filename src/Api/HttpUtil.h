#pragma once
#include <curl/curl.h>
#include <cstring>
#include "HttpEntities.h"
#include "../Util/GlobalTypedef.h"

class HttpUtil
{
public:
	HttpUtil();
	~HttpUtil();
private:
	static bool m_bDebug;
	static size_t write_callback(void *contents, size_t size, size_t nmemb, std::string *s);

	template<typename TResponse>
	static TResponse response_parser(CURLcode& code, const std::string& body);
	static std::string get_file_ext(const std::string &file);
	static void url_encode(std::string& str);
	static void url_decode(std::string& str);
	static bool is_url_encode(const std::string &str);
public:
	/**
	* @brief: HTTP POST request
	* @param strUrl: the Url of the request, for example: http://api.gstore.cn
	* @param strPost: json string
	* @param strResponse: content returned
	* @return: returned value
	*/
	static CURLcode Post(const std::string& strUrl, const std::string& strPost, const std::string& filename);

	static CURLcode Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse);

	static CURLcode Post(const std::string& strUrl, const std::map<std::string, std::string>& headers, long timeOut, const std::string& strPost, std::string& strResponse);

	/**
	 * @brief: HTTP POST file request
	 * @param strUrl: the Url of the request, for example: http://api.gstore.cn
	 * @param headers: HTTP head
	 * @param timeOut: operation timeout (second)
	 * @param filePath: upload file
	 * @param params: form data params
	 * @param strResponse: content returned
	 */
	static CURLcode PostFile(const std::string& strUrl, const std::map<std::string, std::string>& headers, long timeOut, const std::string& filePath, const std::map<std::string, std::string>& params, std::string& strResponse);

	/**
	* @brief: HTTP GET request
	* @param strUrl: the Url of the request, for example: http://api.gstore.cn
	* @param strResponse: content returned
	* @return: returned value
	*/
	static CURLcode Get(const std::string& strUrl, const std::string& filename);

	static CURLcode Get(const std::string& strUrl, std::string& strResponse);

	static CURLcode Get(const std::string& strUrl, const std::map<std::string, std::string>& headers, std::string& strResponse);

	static CURLcode DownloadFile(const std::string& strUrl, std::string& filePath);

	static httpentities::ShutdownResponse shutdown(const std::string& url, httpentities::ShutdownRequest& request);

	static httpentities::CheckResponse check(const std::string& url, httpentities::CheckRequest& request);

	static httpentities::BaseResponse refreshConf(const std::string& url, const bool& inner, httpentities::RefreshconfRequest& request);

	static httpentities::TestConnectionResponse testConnection(const std::string& url, const bool& inner, httpentities::TestConnectionRequest& request);

	static httpentities::InitResponse init(const std::string& url, const bool& inner, httpentities::InitRequest& request);

	static httpentities::LoadResponse load(const std::string& url, const bool& inner, httpentities::LoadRequest& request);

	static httpentities::BaseResponse login(const std::string& url, httpentities::LoginRequest& request);

	static httpentities::BaseResponse unload(const std::string& url, const bool& inner, httpentities::UnloadRequest& request);

	static httpentities::BuildResponse build(const std::string& url, const bool& inner, httpentities::BuildRequest& request);

	static httpentities::BaseResponse drop(const std::string& url, const bool& inner, httpentities::DropRequest& request);

	static httpentities::ShowResponse show(const std::string& url, const bool& inner, httpentities::ShowRequest& request);

	static httpentities::MonitorResponse monitor(const std::string& url, const bool& inner, httpentities::MonitorRequest& request);

	static httpentities::QueryResponse query(const std::string& url, const bool& inner, httpentities::QueryRequest& request);

	static httpentities::BatchInsertResponse batchInsert(const std::string& url, const bool& inner, httpentities::BatchInsertRequest& request);

	static httpentities::BatchRemoveResponse batchRemove(const std::string& url, const bool& inner, httpentities::BatchRemoveRequest& request);

	static httpentities::ClusterResponse reply(const std::string& url, httpentities::ReplyRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse appendEntries(const std::string& url, httpentities::AppenEntriesRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse heartBeat(const std::string& url, httpentities::HeartBeatRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse clusterCheck(const std::string& url, httpentities::ClusterCheckRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse recoverFollower(const std::string& url, httpentities::RecoverRequest& request, const std::string& username, const std::string& password);
};
