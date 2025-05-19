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
    // protocol::HttpRequest *req = task -> get_req();
    // protocol::HttpResponse *resp = task -> get_resp();
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

std::string WFHttpUtil::get_file_ext(const std::string& filename)
{
    std::string::size_type pos1 = filename.find_last_of("/");
    if (pos1 == std::string::npos)
        pos1 = 0;
    else
        pos1++;
    std::string file = filename.substr(pos1, -1);
    std::string::size_type pos2 = file.find_last_of(".");
    if (pos2 == std::string::npos)
        return "";
    else
        return file.substr(pos2 + 1, -1);
}

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

    return Post(strUrl, {}, -1, strPost, strResponse);
}

int WFHttpUtil::Post(const std::string& strUrl, const std::map<std::string, std::string>& headers, const int& timeOut, const std::string& strPost, std::string& strResponse)
{
    int _timeout = timeOut;
    if (timeOut > 0)
    {
        _timeout = timeOut * 1000;
    }
    struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    settings.endpoint_params.response_timeout = _timeout;
    WORKFLOW_library_init(&settings);

    WFFacilities::WaitGroup wait_group(1);
    strResponse.clear();

    WFHttpTask *task = WFTaskFactory::create_http_task(http_wrapper(strUrl), REDIRECT_MAX, RETRY_MAX, respwrite_callback);
    protocol::HttpRequest *req = task -> get_req();
    req -> set_method("POST");
    req -> add_header_pair("Connection", "close");
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

int WFHttpUtil::PostFile(const std::string& strUrl, const std::map<std::string, std::string>& headers, const int& timeOut, const std::string& filePath, const std::map<std::string, std::string>& params, std::string& strResponse)
{
    int _timeout = timeOut;
    if (timeOut > 0)
    {
        _timeout = timeOut * 1000;
    }
    struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    settings.endpoint_params.response_timeout = _timeout;
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
    req -> add_header_pair("Content-Type", "multipart/form-data; boundary=" + boundary);
    req -> append_output_body_nocopy(body);
    http_task -> start();
    wait_group.wait();
    return http_task -> get_state();
}


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
    resp -> set_size_limit(40 * 1024 * 1024);
    FileData data = {fw, &wait_group};
    task -> user_data = (void*) &data;
    task -> start();
    wait_group.wait();

    return task -> get_state();
}

int WFHttpUtil::Get(const std::string& strUrl, std::string& strResponse)
{
    return Get(strUrl, {}, -1, strResponse);
}

int WFHttpUtil::Get(const std::string& strUrl, const std::map<std::string, std::string>& headers, const int& timeOut, std::string& strResponse)
{
    int _timeout = timeOut;
    if (timeOut > 0)
    {
        _timeout = timeOut * 1000;
    }
    struct WFGlobalSettings settings = GLOBAL_SETTINGS_DEFAULT;
    settings.endpoint_params.response_timeout = _timeout;
    WORKFLOW_library_init(&settings);
    
    WFFacilities::WaitGroup wait_group(1);
    WFHttpTask *http_task = WFTaskFactory::create_http_task(http_wrapper(strUrl), REDIRECT_MAX, RETRY_MAX, respwrite_callback);
    protocol::HttpRequest *req = http_task -> get_req();
    protocol::HttpResponse *resp = http_task -> get_resp();
    req -> set_method("GET");
    req -> add_header_pair("Connection", "close");
    for (const auto &pair : headers) {
        req -> add_header_pair(pair.first, pair.second);
    }
    RespData data = {&strResponse, &wait_group};
    http_task -> user_data = (void*) &data;
    http_task -> start();
    wait_group.wait();

    return http_task -> get_state();
}

int WFHttpUtil::DownloadFile(const std::string& strUrl, std::string& filePath)
{
    WFHttpTask* httptask = WFTaskFactory::create_http_task(http_wrapper(strUrl), REDIRECT_MAX, RETRY_MAX, fwrite_callback);
    httptask->set_callback([httptask, strUrl, &filePath](WFHttpTask* task) {
        FileData* data = (FileData*) task -> user_data;
        WFFacilities::WaitGroup *wait_group = data -> _wait_group;
        if (!ErrorHandler(task))
        {
            filePath = "";
            wait_group -> done();
            return;
        }
        protocol::HttpResponse *resp = task ->get_resp();
        protocol::HttpHeaderCursor resp_cursor(resp);
        std::string name;
        std::string value;
        std::string filename = "";
        while (resp_cursor.next(name, value))
        {
            SLOG_CORE(name + ": " + value);
            if (name == "Content-Disposition")
            {
                std::regex pattern(R"(filename=(.*?)(;|$))");
                std::smatch matches;
                if (std::regex_search(value, matches, pattern)) {
                    filename = matches[1];
                    filename.erase(remove(filename.begin(), filename.end(), '"'), filename.end());
                    break;
                }
            }
        }
        if (filename.empty())
        {
            filename = strUrl.substr(strUrl.find_last_of("/") + 1);
        }
        std::string file_suffix = get_file_ext(filename);
        SLOG_CORE("filename: " + filename + ", extname: " + file_suffix);
        // 获取允许的文件格式
        std::set<std::string> extensions;
        GlobalTypedef::upload_allow_extensions(extensions);
        GlobalTypedef::upload_allow_compress_packages(extensions);
        if (std::find(extensions.begin(), extensions.end(), file_suffix) == extensions.end()) {
            wait_group -> done();
            SLOG_ERROR("Download File type not allowed: " + filename);
            filePath = "";
            return;
        }
        filePath = filePath + filename;
        // 判断filePath文件如果存在，先删除
        std::filesystem::path _path(filePath);
        if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath)) {
            std::filesystem::remove(_path);
        }
        const void *body;
        size_t body_len;
        std::ofstream file(filePath, std::ios::binary);
        resp->get_parsed_body(&body, &body_len);
        if (file.is_open()) {
            file.write((const char*)body, body_len);
            file.close();
            SLOG_CORE("Download File success: " + filePath);
        } else {
            filePath = "";
            SLOG_ERROR("Donwload File failed: open " + filePath+ " failed");
        }
        wait_group->done();
    });
    WFFacilities::WaitGroup wait_group(1);
    FileData data = {nullptr, &wait_group};
    httptask->user_data = (void*) &data;
    httptask->start();
    wait_group.wait();

    return httptask->get_state();
}
