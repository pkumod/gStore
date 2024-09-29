#include <workflow/WFTaskFactory.h>
#include <workflow/WFFacilities.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include "multipart_parser.h"
#include "HttpEntities.h"


constexpr int REDIRECT_MAX = 3, RETRY_MAX = 5;

namespace {
/* struct for File resp */
struct FileData 
{
    FILE* _fw;
    WFFacilities::WaitGroup* _wait_group;
    FileData(FILE* fw, WFFacilities::WaitGroup* wait_group) : _fw(fw), _wait_group(wait_group) { }
};

/* struct for normal resp */
struct RespData
{
    std::string* _p_resp;
    WFFacilities::WaitGroup* _wait_group;
    RespData(std::string* resp, WFFacilities::WaitGroup* wait_group) : _p_resp(resp), _wait_group(wait_group) { }
};
} // anonymous namespace

class WorkFlowHttpUtil
{
public:
	WorkFlowHttpUtil() { };
	~WorkFlowHttpUtil() { };
private:
	// static bool m_bDebug;
	static void respwrite_callback(WFHttpTask *task)
    {
        protocol::HttpResponse *resp = task -> get_resp();
        RespData *data = (RespData*) task -> user_data;
        WFFacilities::WaitGroup *wait_group = data -> _wait_group;
        if (!ErrorHandler(task))
        {
            wait_group -> done();
            return;
        }
        
        const void *body;
        size_t body_len;

        resp -> get_parsed_body(&body, &body_len);
        std::string* target = data -> _p_resp;
        *target = (char*) body;

        wait_group -> done();
    }

    static void fwrite_callback(WFHttpTask *task)
    {
        protocol::HttpResponse *resp = task -> get_resp();
        FileData* data = (FileData*) task -> user_data;
        WFFacilities::WaitGroup *wait_group = data -> _wait_group;
        if (!ErrorHandler(task))
        {
            wait_group -> done();
            return;
        }

        const void *body;
        size_t body_len;
        
        FILE *fw = data -> _fw;
        
        resp -> get_parsed_body(&body, &body_len);
        
        fwrite(body, 1, body_len, fw);
        fclose(fw);

        wait_group -> done();
    }

    static bool ErrorHandler(WFHttpTask *task) {
        protocol::HttpRequest *req = task -> get_req();
        protocol::HttpResponse *resp = task -> get_resp();
        int state = task -> get_state();
        int error = task -> get_error();
        switch (state)
        {
        case WFT_STATE_SYS_ERROR:
            fprintf(stderr, "system error: %s\n", strerror(error));
            break;
        case WFT_STATE_DNS_ERROR:
            fprintf(stderr, "DNS error: %s\n", gai_strerror(error));
            break;
        case WFT_STATE_SSL_ERROR:
            fprintf(stderr, "SSL error: %d\n", error);
            break;
        case WFT_STATE_TASK_ERROR:
            fprintf(stderr, "Task error: %d\n", error);
            break;
        case WFT_STATE_SUCCESS:
            break;
        }
        if (state != WFT_STATE_SUCCESS)
        {
            fprintf(stderr, "Error Code: %d\n", error);
            return false;
        }
        return true;
    }

	template<typename TResponse>
	static TResponse response_parser(int& code, const std::string& body) 
    {
        if (code == WFT_STATE_SUCCESS)
            return TResponse(body);
        else
            return TResponse(code, strerror(code));
    }
public:
	/**
	* @brief: HTTP POST request
	* @param strUrl: the Url of the request, for example: http://api.gstore.cn
	* @param strPost: json string
	* @param strResponse: content returned
	* @return: returned value
	*/
	static int Post(const std::string& strUrl, const std::string& strPost, const std::string& filename)
    {
        WFFacilities::WaitGroup wait_group(1);
        FILE* fw = fopen(filename.c_str(), "wb");
        if (!fw)
        {
            std::cout << "open file failed" << std::endl;
            return -1;
        }
        WFHttpTask *task = WFTaskFactory::create_http_task(strUrl, REDIRECT_MAX, RETRY_MAX, fwrite_callback);
        protocol::HttpRequest *req = task -> get_req();
        protocol::HttpResponse *resp = task -> get_resp();
        req -> set_method("POST");
        req -> add_header_pair("Connection", "close");
        req -> add_header_pair("Timeout", "3");
        req -> add_header_pair("Content-Type", "application/json");
        req -> append_output_body(strPost);
        resp -> set_size_limit(40 * 1024 * 1024);
        FileData data = {fw, &wait_group};
        task -> user_data = (void*) &data;
        task -> start();
        
        wait_group.wait();

        return task -> get_state();

    }

	static int Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse)
    {
        WFFacilities::WaitGroup wait_group(1);
        strResponse.clear();

        WFHttpTask *task = WFTaskFactory::create_http_task(strUrl, REDIRECT_MAX, RETRY_MAX, respwrite_callback);
        protocol::HttpRequest *req = task -> get_req();
        req -> set_method("POST");
        req -> add_header_pair("Connection", "close");
        req -> add_header_pair("Timeout", "3");
        req -> add_header_pair("Content-Type", "application/json");
        req -> append_output_body(strPost);
        RespData data = {&strResponse, &wait_group};
        task -> user_data = (void*) &data;
        task -> start();
        wait_group.wait();

        return task -> get_state();
    }

	static int Post(const std::string& strUrl, const std::map<std::string, std::string>& headers, long timeOut, const std::string& strPost, std::string& strResponse)
    {
        WFFacilities::WaitGroup wait_group(1);
        strResponse.clear();

        WFHttpTask *task = WFTaskFactory::create_http_task(strUrl, REDIRECT_MAX, RETRY_MAX, respwrite_callback);
        protocol::HttpRequest *req = task -> get_req();
        req -> set_method("POST");
        req -> add_header_pair("Connection", "close");
        req -> add_header_pair("Timeout", std::to_string(timeOut));
        req -> add_header_pair("Content-Type", "application/json");
        req -> append_output_body(strPost);
        for (auto &pair : headers) {
            req -> add_header_pair(pair.first, pair.second);
        }
        RespData data = {&strResponse, &wait_group};
        task -> user_data = (void*) &data;
        task -> start();
        wait_group.wait();

        return task -> get_state();
    }

	/**
	 * @brief: HTTP POST file request no bigger than 40MB
	 * @param strUrl: the Url of the request, for example: http://api.gstore.cn
	 * @param headers: HTTP head
	 * @param timeOut: operation timeout (second)
	 * @param filePath: upload file
	 * @param params: form data params
	 * @param strResponse: content returned
	 */
	static int PostFile(const std::string& strUrl, const std::map<std::string, std::string>& headers, long timeOut, const std::string& filePath, const std::map<std::string, std::string>& params, std::string& strResponse)
    {
        web::http::MultipartParser parser;
        parser.AddFile("file", filePath);
        for (const auto &pair : params) {
            parser.AddParameter(pair.first, pair.second);
        }
        std::string boundary = parser.boundary();
        std::string body = parser.GenBodyContent();
        WFFacilities::WaitGroup wait_group(1);
        WFHttpTask *http_task = WFTaskFactory::create_http_task(strUrl, REDIRECT_MAX, RETRY_MAX, respwrite_callback);
        protocol::HttpRequest *req = http_task -> get_req();
        protocol::HttpResponse *resp = http_task -> get_resp();
        for (const auto &pair : headers) {
            req -> add_header_pair(pair.first, pair.second);
        }
        RespData data = {&strResponse, &wait_group};
        http_task -> user_data = (void*) &data;
        req -> set_method("POST");
        req -> add_header_pair("Connection", "close");
        req -> add_header_pair("Timeout", std::to_string(timeOut));
        req -> add_header_pair("Content-Type", "multipart/form-data; boundary=" + boundary);
        req -> append_output_body_nocopy(body);
        http_task -> start();
        wait_group.wait();
        return http_task -> get_state();
    }

	/**
	* @brief: HTTP GET request
	* @param strUrl: the Url of the request, for example: http://api.gstore.cn
	* @param strResponse: content returned
	* @return: returned value
	*/
	static int Get(const std::string& strUrl, const std::string& filename)
    {
        WFFacilities::WaitGroup wait_group(1);
        FILE* fw = fopen(filename.c_str(), "wb");
        if (!fw)
        {
            std::cout << "open file failed" << std::endl;
            return -1;
        }
        WFHttpTask *task = WFTaskFactory::create_http_task(strUrl, REDIRECT_MAX, RETRY_MAX, fwrite_callback);
        protocol::HttpRequest *req = task -> get_req();
        protocol::HttpResponse *resp = task -> get_resp();
        req -> set_method("GET");
        req -> add_header_pair("Connection", "close");
        req -> add_header_pair("Timeout", "3");
        req -> add_header_pair("Content-Type", "application/json");
        resp -> set_size_limit(40 * 1024 * 1024);
        FileData data = {fw, &wait_group};
        task -> user_data = (void*) &data;
        task -> start();
        wait_group.wait();

        return task -> get_state();
    }

	static int Get(const std::string& strUrl, std::string& strResponse)
    {
        WFFacilities::WaitGroup wait_group(1);
        strResponse.clear();
        WFHttpTask *task = WFTaskFactory::create_http_task(strUrl, REDIRECT_MAX, RETRY_MAX, respwrite_callback);
        
        protocol::HttpRequest *req = task -> get_req();
        req -> set_method("GET");
        req -> add_header_pair("Connection", "close");
        req -> add_header_pair("Timeout", "3");
        req -> add_header_pair("Content-Type", "application/json");
        RespData data = {&strResponse, &wait_group};
        task -> user_data = (void*) &data;
        task -> start();
        wait_group.wait();

        return task -> get_state();
    }

	static int Get(const std::string& strUrl, const std::map<std::string, std::string>& headers, std::string& strResponse)
    {
        WFFacilities::WaitGroup wait_group(1);
        WFHttpTask *http_task = WFTaskFactory::create_http_task(strUrl, REDIRECT_MAX, RETRY_MAX, respwrite_callback);
        protocol::HttpRequest *req = http_task -> get_req();
        protocol::HttpResponse *resp = http_task -> get_resp();
        req -> set_method("GET");
        req -> add_header_pair("Connection", "close");
        req -> add_header_pair("Timeout", "3");
        req -> add_header_pair("Content-Type", "application/json");
        for (const auto &pair : headers) {
            req -> add_header_pair(pair.first, pair.second);
        }
        RespData data = {&strResponse, &wait_group};
        http_task -> user_data = (void*) &data;
        http_task -> start();
        wait_group.wait();

        return http_task -> get_state();
    }

httpentities::ShutdownResponse shutdown(const std::string& url, httpentities::ShutdownRequest& request)
{
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", request.username));
	headers.insert(std::pair<std::string, std::string>("password", request.password));
	int status = Post(url, headers, 60, "", body_str);
	return response_parser<httpentities::ShutdownResponse>(status, body_str);
}

httpentities::CheckResponse check(const std::string& url, httpentities::CheckRequest& request)
{
	std::string param_str = request.to_params();
	std::string body_str;
	std::string strUrl = url + "?" + param_str;
	int status = Get(strUrl, body_str);
	return response_parser<httpentities::CheckResponse>(status, body_str);
}

httpentities::BaseResponse refreshConf(const std::string& url, const bool& inner, httpentities::RefreshconfRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::TestConnectionResponse testConnection(const std::string& url, const bool& inner, httpentities::TestConnectionRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::TestConnectionResponse>(status, body_str);
}

httpentities::InitResponse init(const std::string& url, const bool& inner, httpentities::InitRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::InitResponse>(status, body_str);
}

httpentities::LoadResponse load(const std::string& url, const bool& inner, httpentities::LoadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::LoadResponse>(status, body_str);
}

httpentities::BaseResponse login(const std::string& url, httpentities::LoginRequest& request)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::BaseResponse unload(const std::string& url, const bool& inner, httpentities::UnloadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::BuildResponse build(const std::string& url, const bool& inner, httpentities::BuildRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::BuildResponse>(status, body_str);
}

httpentities::BaseResponse drop(const std::string& url, const bool& inner, httpentities::DropRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::ShowResponse show(const std::string& url, const bool& inner, httpentities::ShowRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::ShowResponse>(status, body_str);
}

httpentities::MonitorResponse monitor(const std::string& url, const bool& inner, httpentities::MonitorRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::MonitorResponse>(status, body_str);
}

httpentities::QueryResponse query(const std::string& url, const bool& inner, httpentities::QueryRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::QueryResponse>(status, body_str);
}


httpentities::BatchInsertResponse batchInsert(const std::string& url, const bool& inner, httpentities::BatchInsertRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::BatchInsertResponse>(status, body_str);
}

httpentities::BatchRemoveResponse batchRemove(const std::string& url, const bool& inner, httpentities::BatchRemoveRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	return response_parser<httpentities::BatchRemoveResponse>(status, body_str);
}

httpentities::ClusterResponse reply(const std::string& url, httpentities::ReplyRequest& request, const std::string& username, const std::string& password)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	int status = Post(url, headers, 60, json_str, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

httpentities::ClusterResponse appendEntries(const std::string& url, httpentities::AppenEntriesRequest& request, const std::string& username, const std::string& password)
{
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	std::map<std::string, std::string> params;
	params.insert(std::pair<std::string, std::string>("db_name", request.db_name));
	params.insert(std::pair<std::string, std::string>("term", std::to_string(request.term)));
	params.insert(std::pair<std::string, std::string>("index", std::to_string(request.index)));
	params.insert(std::pair<std::string, std::string>("operation", request.operation));
	std::string body_str;
	int status = PostFile(url, headers, 3600, request.file_path, params, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

httpentities::ClusterResponse heartBeat(const std::string& url, httpentities::HeartBeatRequest& request, const std::string& username, const std::string& password)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	int status = Post(url, headers, 60, json_str, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

httpentities::ClusterResponse cancel(const std::string& url, httpentities::CancelRequest& request, const std::string& username, const std::string& password)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	int status = Post(url, headers, 60, json_str, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

httpentities::ClusterResponse clusterCheck(const std::string& url, httpentities::ClusterCheckRequest& request, const std::string& username, const std::string& password)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	int status = Post(url, headers, 60, json_str, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

};
