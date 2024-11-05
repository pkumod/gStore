#include "WFHttpUtil.h"

void WFHttpUtil::respwrite_callback(WFHttpTask *task)
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

void WFHttpUtil::fwrite_callback(WFHttpTask *task)
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

bool WFHttpUtil::ErrorHandler(WFHttpTask *task) {
    protocol::HttpRequest *req = task -> get_req();
    protocol::HttpResponse *resp = task -> get_resp();
    int state = task -> get_state();
    int error = task -> get_error();
    
    std::stringstream error_msg;
    switch (state)
    {
    case WFT_STATE_SYS_ERROR:
        error_msg << "system error: " << strerror(error) << std::endl;
        break;
    case WFT_STATE_DNS_ERROR:
        error_msg << "DNS error: " << strerror(error) << std::endl;
        break;
    case WFT_STATE_SSL_ERROR:
        error_msg << "SSL error: " << strerror(error) << std::endl;
        break;
    case WFT_STATE_TASK_ERROR:
        error_msg << "task error: " << strerror(error) << std::endl;
        break;
    case WFT_STATE_SUCCESS:
        break;
    }
    if (state != WFT_STATE_SUCCESS)
    {
        if (error_msg.str().empty()) error_msg << "Error Code: " << error << std::endl;
        SLOG_CORE(error_msg.str());
        return false;
    }
    return true;
}


/**
* @brief: HTTP POST request
* @param strUrl: the Url of the request, for example: http://api.gstore.cn
* @param strPost: json string
* @param strResponse: content returned
* @return: returned value
*/
int WFHttpUtil::Post(const std::string& strUrl, const std::string& strPost, const std::string& filename)
{
    struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    settings.endpoint_params.response_timeout = -1;
    WORKFLOW_library_init(&settings);

    WFFacilities::WaitGroup wait_group(1);
    FILE* fw = fopen(filename.c_str(), "wb");
    if (!fw)
    {
        SLOG_CORE("open file failed\n");
        return -1;
    }

    WFHttpTask *task = WFTaskFactory::create_http_task(http_wrapper(strUrl), REDIRECT_MAX, RETRY_MAX, fwrite_callback);
    protocol::HttpRequest *req = task -> get_req();
    protocol::HttpResponse *resp = task -> get_resp();
    req -> set_method("POST");
    req -> add_header_pair("Connection", "close");
    // // // req -> add_header_pair("Timeout", "3");
    req -> add_header_pair("Content-Type", "application/json");
    req -> append_output_body(strPost);
    resp -> set_size_limit(40 * 1024 * 1024);
    FileData data = {fw, &wait_group};
    task -> user_data = (void*) &data;
    task -> start();
    
    wait_group.wait();

    return task -> get_state();

}

int WFHttpUtil::Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse)
{

    struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    settings.endpoint_params.response_timeout = -1;
    WORKFLOW_library_init(&settings);

    WFFacilities::WaitGroup wait_group(1);
    strResponse.clear();

    WFHttpTask *task = WFTaskFactory::create_http_task(http_wrapper(strUrl), REDIRECT_MAX, RETRY_MAX, respwrite_callback);
    protocol::HttpRequest *req = task -> get_req();
    req -> set_method("POST");
    req -> add_header_pair("Connection", "close");
    // req -> add_header_pair("Timeout", "3");
    req -> add_header_pair("Content-Type", "application/json");
    req -> append_output_body(strPost);
    RespData data = {&strResponse, &wait_group};
    task -> user_data = (void*) &data;
    task -> start();
    wait_group.wait();

    return task -> get_state();
}

int WFHttpUtil::Post(const std::string& strUrl, const std::map<std::string, std::string>& headers, long timeOut, const std::string& strPost, std::string& strResponse)
{
    struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    settings.endpoint_params.response_timeout = -1;
    WORKFLOW_library_init(&settings);

    WFFacilities::WaitGroup wait_group(1);
    strResponse.clear();

    WFHttpTask *task = WFTaskFactory::create_http_task(http_wrapper(strUrl), REDIRECT_MAX, RETRY_MAX, respwrite_callback);
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
int WFHttpUtil::PostFile(const std::string& strUrl, const std::map<std::string, std::string>& headers, long timeOut, const std::string& filePath, const std::map<std::string, std::string>& params, std::string& strResponse)
{

    struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    settings.endpoint_params.response_timeout = -1;
    WORKFLOW_library_init(&settings);

    MultipartParser parser;
    parser.AddFile("file", filePath);
    for (const auto &pair : params) {
        parser.AddParameter(pair.first, pair.second);
    }
    std::string boundary = parser.boundary();
    std::string body = parser.GenBodyContent();
    WFFacilities::WaitGroup wait_group(1);
    WFHttpTask *http_task = WFTaskFactory::create_http_task(http_wrapper(strUrl), REDIRECT_MAX, RETRY_MAX, respwrite_callback);
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
int WFHttpUtil::Get(const std::string& strUrl, const std::string& filename)
{

    struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    settings.endpoint_params.response_timeout = -1;
    WORKFLOW_library_init(&settings);

    WFFacilities::WaitGroup wait_group(1);
    FILE* fw = fopen(filename.c_str(), "wb");
    if (!fw)
    {
        SLOG_CORE("open file failed\n");
        return -1;
    }
    WFHttpTask *task = WFTaskFactory::create_http_task(http_wrapper(strUrl), REDIRECT_MAX, RETRY_MAX, fwrite_callback);
    protocol::HttpRequest *req = task -> get_req();
    protocol::HttpResponse *resp = task -> get_resp();
    req -> set_method("GET");
    req -> add_header_pair("Connection", "close");
    // // req -> add_header_pair("Timeout", "3");
    req -> add_header_pair("Content-Type", "application/json");
    resp -> set_size_limit(40 * 1024 * 1024);
    FileData data = {fw, &wait_group};
    task -> user_data = (void*) &data;
    task -> start();
    wait_group.wait();

    return task -> get_state();
}

int WFHttpUtil::Get(const std::string& strUrl, std::string& strResponse)
{

    struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    settings.endpoint_params.response_timeout = -1;
    WORKFLOW_library_init(&settings);

    WFFacilities::WaitGroup wait_group(1);
    strResponse.clear();
    WFHttpTask *task = WFTaskFactory::create_http_task(http_wrapper(strUrl), REDIRECT_MAX, RETRY_MAX, respwrite_callback);
    
    protocol::HttpRequest *req = task -> get_req();
    req -> set_method("GET");
    req -> add_header_pair("Connection", "close");
    // // req -> add_header_pair("Timeout", "3");
    req -> add_header_pair("Content-Type", "application/json");
    RespData data = {&strResponse, &wait_group};
    task -> user_data = (void*) &data;
    task -> start();
    wait_group.wait();

    return task -> get_state();
}

int WFHttpUtil::Get(const std::string& strUrl, const std::map<std::string, std::string>& headers, std::string& strResponse)
{

    struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    settings.endpoint_params.response_timeout = -1;
    WORKFLOW_library_init(&settings);
    
    WFFacilities::WaitGroup wait_group(1);
    WFHttpTask *http_task = WFTaskFactory::create_http_task(http_wrapper(strUrl), REDIRECT_MAX, RETRY_MAX, respwrite_callback);
    protocol::HttpRequest *req = http_task -> get_req();
    protocol::HttpResponse *resp = http_task -> get_resp();
    req -> set_method("GET");
    req -> add_header_pair("Connection", "close");
    // // req -> add_header_pair("Timeout", "3");
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

