#include "HttpUtil.h"

bool HttpUtil::m_bDebug = false;

HttpUtil::HttpUtil()
{
	std::cout << "HttpUtil init" << std::endl;
}
HttpUtil::~HttpUtil()
{

}

static const std::string UrlEncode(const std::string& s)
{
	std::string ret;
	unsigned char* ptr = (unsigned char*)s.c_str();
	ret.reserve(s.length());

	for (int i = 0; i < s.length(); ++i)
	{
		if ((int(ptr[i]) == 42) || (int(ptr[i]) == 45) || (int(ptr[i]) == 46) || (int(ptr[i]) == 47) || (int(ptr[i]) == 58) || (int(ptr[i]) == 95))
			ret += ptr[i];
		else if ((int(ptr[i]) >= 48) && (int(ptr[i]) <= 57))
			ret += ptr[i];
		else if ((int(ptr[i]) >= 65) && (int(ptr[i]) <= 90))
			ret += ptr[i];
		else if ((int(ptr[i]) >= 97) && (int(ptr[i]) <= 122))
			ret += ptr[i];
		else if (int(ptr[i]) == 32)
			ret += '+';
		else if ((int(ptr[i]) != 9) && (int(ptr[i]) != 10) && (int(ptr[i]) != 13))
		{
			char buf[5];
			memset(buf, 0, 5);
			snprintf(buf, 5, "%%%X", ptr[i]);
			ret.append(buf);
		}
	}
	return ret;
}

static int OnDebug(CURL*, curl_infotype itype, char* pData, size_t size, void*)
{
	if (itype == CURLINFO_TEXT)
	{
		printf("[TEXT]%s\n", pData);
	}
	else if (itype == CURLINFO_HEADER_IN)
	{
		printf("[HEADER_IN]%s\n", pData);
	}
	else if (itype == CURLINFO_HEADER_OUT)
	{
		printf("[HEADER_OUT]%s\n", pData);
	}
	else if (itype == CURLINFO_DATA_IN)
	{
		printf("[DATA_IN]%s\n", pData);
	}
	else if (itype == CURLINFO_DATA_OUT)
	{
		printf("[DATA_OUT]%s\n", pData);
	}
	return 0;
}

size_t HttpUtil::write_callback(void *contents, size_t size, size_t nmemb, std::string *s)
{
	size_t newLength = size * nmemb;
    try {
        s->append((char*)contents, newLength);
    } catch(std::bad_alloc &e) {
        return 0;
    }
    return newLength;
}

template<typename TResponse>
TResponse HttpUtil::response_parser(CURLcode& code, const std::string& body)
{
	if (code == CURLE_OK)
		return TResponse(body);
	else
		return TResponse(code, curl_easy_strerror(code));
}

std::string HttpUtil::get_file_ext(const std::string& filename)
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

CURLcode HttpUtil::Get(const std::string& strUrl, std::string& strResponse)
{
	SLOG_CORE("url: " + strUrl);
	strResponse.clear();
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_URL, UrlEncode(strUrl).c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	SLOG_CORE("url: " + strUrl + ", code: " + std::to_string(res) + " (" + curl_easy_strerror(res) + ")");
	return res;
}

CURLcode HttpUtil::Get(const std::string& strUrl, const std::string& filename)
{
	SLOG_CORE("url: " + strUrl);
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 4096);
	curl_easy_setopt(curl, CURLOPT_URL, UrlEncode(strUrl).c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);

	FILE* fw = fopen(filename.c_str(), "wb");
	if (!fw)
	{
		std::cout << "open file failed" << std::endl;
		return CURLE_FILE_COULDNT_READ_FILE;
	}
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fw);

	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	fclose(fw);
	SLOG_CORE("url: " + strUrl + ", code: " + std::to_string(res) + " (" + curl_easy_strerror(res) + ")");
	return res;
}

CURLcode HttpUtil::Get(const std::string& strUrl, const std::map<std::string, std::string>& headers, std::string& strResponse)
{
	SLOG_CORE("url: " + strUrl);
	strResponse.clear();
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	struct curl_slist* headerlist = nullptr;
	std::string header_str;
	for (const auto& pair : headers)
	{
		if (pair.first.empty())
		{
			continue;
		}
		header_str = pair.first + ":" + pair.second;
		headerlist = curl_slist_append(headerlist, header_str.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);  
	curl_easy_setopt(curl, CURLOPT_URL, UrlEncode(strUrl).c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	SLOG_CORE("url: " + strUrl + ", code: " + std::to_string(res) + " (" + curl_easy_strerror(res) + ")");
	return res;
}

CURLcode HttpUtil::Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse)
{
	SLOG_CORE("url: " + strUrl + ", requestBody: " + strPost);
	strResponse.clear();
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	struct curl_slist* headerlist = NULL;
	std::string content_type = "Content-Type:application/json";
	headerlist = curl_slist_append(headerlist, content_type.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);  
	curl_easy_setopt(curl, CURLOPT_URL, UrlEncode(strUrl).c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	SLOG_CORE("url: " + strUrl + ", code: " + std::to_string(res) + " (" + curl_easy_strerror(res) + ")");
	return res;
}

CURLcode HttpUtil::Post(const std::string& strUrl, const std::string& strPost, const std::string& filename)
{
	SLOG_CORE("url: " + strUrl + ", requestBody: " + strPost);
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	struct curl_slist* headerlist = NULL;
	std::string content_type = "Content-Type:application/json";
	headerlist = curl_slist_append(headerlist, content_type.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);  
	curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, 4096);
	curl_easy_setopt(curl, CURLOPT_URL, UrlEncode(strUrl).c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
	FILE* fw = fopen(filename.c_str(), "wb");
	if (!fw)
	{
		std::cout << "open file failed" << std::endl;
		return CURLE_FILE_COULDNT_READ_FILE;
	}
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fw);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	fclose(fw);
	SLOG_CORE("url: " + strUrl + ", code: " + std::to_string(res) + " (" + curl_easy_strerror(res) + ")");
	return res;
}

CURLcode HttpUtil::Post(const std::string& strUrl, const std::map<std::string, std::string>& headers, long timeOut, const std::string& strPost, std::string& strResponse)
{
	SLOG_CORE("url: " + strUrl + ", requestBody: " + strPost);
	strResponse.clear();
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	struct curl_slist* headerlist = nullptr;
	std::string header_str;
	if (headers.find("Content-Type") == headers.end())
	{
		header_str = "Content-Type:application/json";
		headerlist = curl_slist_append(headerlist, header_str.c_str());
	}
	for (const auto& pair : headers)
	{
		if (pair.first.empty())
		{
			continue;
		}
		header_str = pair.first + ":" + pair.second;
		headerlist = curl_slist_append(headerlist, header_str.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);  
	curl_easy_setopt(curl, CURLOPT_URL, UrlEncode(strUrl).c_str());
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeOut);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	SLOG_CORE("url: " + strUrl + ", code: " + std::to_string(res) + " (" + curl_easy_strerror(res) + ")");
	return res;
}

CURLcode HttpUtil::PostFile(const std::string& strUrl, const std::map<std::string, std::string>& headers, long timeOut, const std::string& filePath, const std::map<std::string, std::string>& params, std::string& strResponse)
{
	SLOG_CORE("url: " + strUrl + ", filePath: " + filePath);
    strResponse.clear();
    CURLcode res;
    CURL* curl = curl_easy_init();
    if (NULL == curl)
    {
        return CURLE_FAILED_INIT;
    }
    if (m_bDebug)
    {
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
    }
	// 设置目标URL
    curl_easy_setopt(curl, CURLOPT_URL, UrlEncode(strUrl).c_str());
    // 设置POST请求
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	
	// 设置上传文件
	struct curl_httppost* formpost = NULL;
	struct curl_httppost* lastptr = NULL;
	curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, "file", CURLFORM_FILE, filePath.c_str(), CURLFORM_END);
	
	// 设置其他参数
    for (const auto& pair : params)
    {
		curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, pair.first.c_str(), CURLFORM_COPYCONTENTS, pair.second.c_str(), CURLFORM_END);
    }

	struct curl_slist* headerlist = nullptr;
	std::string header_str;
	for (const auto& pair : headers)
	{
		if (pair.first.empty())
		{
			continue;
		}
		header_str = pair.first + ":" + pair.second;
		headerlist = curl_slist_append(headerlist, header_str.c_str());
	}
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
	
	// 设置表单数据
	curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);

	// 设置回调函数接收响应数据
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	
	// 执行请求
    res = curl_easy_perform(curl);
    
	// 检测错误
	if (res != CURLE_OK)
	{
		SLOG_CORE("post file failed: " << curl_easy_strerror(res));
	}
	else
	{
		SLOG_CORE("post file response: " << strResponse);
	}
	SLOG_CORE("url: " + strUrl + ", responseBody: " + strResponse);
	// 清理
	curl_formfree(formpost);
	curl_easy_cleanup(curl);
	curl_slist_free_all(headerlist);
	SLOG_CORE("url: " + strUrl + ", code: " + std::to_string(res) + " (" + curl_easy_strerror(res) + ")");
    return res;
}


CURLcode HttpUtil::DownloadFile(const std::string& strUrl, std::string& filePath)
{
	SLOG_CORE("url: " + strUrl + ", savePath: " + filePath);
	CURLcode res;
	CURL* curl = curl_easy_init();
	if (NULL == curl)
	{
		return CURLE_FAILED_INIT;
	}
	if (m_bDebug)
	{
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
		curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, OnDebug);
	}
	std::string readBuffer;
	std::string headerBuffer;
	curl_easy_setopt(curl, CURLOPT_URL, UrlEncode(strUrl).c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
	curl_easy_setopt(curl, CURLOPT_HEADER, 0);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerBuffer);
	res = curl_easy_perform(curl);
	if (res != CURLE_OK)
	{
		SLOG_ERROR("download file failed: " << curl_easy_strerror(res));
		curl_easy_cleanup(curl);
		filePath = "";
		return res;
	}
	// curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	// curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, NULL);
	// curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerBuffer);
	// res = curl_easy_perform(curl);
	// if (res != CURLE_OK)
	// {
	// 	SLOG_ERROR("download file failed: " << curl_easy_strerror(res));
	// 	curl_easy_cleanup(curl);
	// 	filePath = "";
	// 	return res;
	// }
	std::stringstream sshb(headerBuffer);
	std::string item;
	std::string filename;
	std::string pattern = "filename=\"(.*?)\"";
	std::regex regex = std::regex(pattern);
	std::smatch match;
	while (std::getline(sshb, item, '\n')) {
		SLOG_CORE(item);
		if (std::regex_search(item, match, regex)) {
			filename = match[1];
			break;
		}
	}
	// 校验文件类型
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
		SLOG_ERROR("download file type not allowed: " + filename);
		filePath = "";
		return CURLE_WRITE_ERROR;
	}
	filePath = filePath + filename;
	// 判断filePath文件如果存在，先删除
	std::filesystem::path _path(filePath);
	if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath)) {
		std::filesystem::remove(_path);
	}
	FILE* fw = fopen(filePath.c_str(), "wb");
	if (!fw)
	{
		SLOG_ERROR("open file failed: " + filePath);
		filePath = "";
		return CURLE_WRITE_ERROR;
	}
	fwrite(readBuffer.c_str(), 1, readBuffer.size(), fw);
	fclose(fw);
	curl_easy_cleanup(curl);
	SLOG_CORE("download url: " + strUrl + ", savePath: " + filePath + ", code: " + std::to_string(res) + " (" + curl_easy_strerror(res) + ")");
	return res;
}

httpentities::ShutdownResponse HttpUtil::shutdown(const std::string& url, httpentities::ShutdownRequest& request)
{
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", request.username));
	headers.insert(std::pair<std::string, std::string>("password", request.password));
	CURLcode status = Post(url, headers, 60, "", body_str);
	return response_parser<httpentities::ShutdownResponse>(status, body_str);
}

httpentities::CheckResponse HttpUtil::check(const std::string& url, httpentities::CheckRequest& request)
{
	std::string param_str = request.to_params();
	std::string body_str;
	std::string strUrl = url + "?" + param_str;
	CURLcode status = Get(strUrl, body_str);
	return response_parser<httpentities::CheckResponse>(status, body_str);
}

httpentities::BaseResponse HttpUtil::refreshConf(const std::string& url, const bool& inner, httpentities::RefreshconfRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::TestConnectionResponse HttpUtil::testConnection(const std::string& url, const bool& inner, httpentities::TestConnectionRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::TestConnectionResponse>(status, body_str);
}

httpentities::InitResponse HttpUtil::init(const std::string& url, const bool& inner, httpentities::InitRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::InitResponse>(status, body_str);
}

httpentities::LoadResponse HttpUtil::load(const std::string& url, const bool& inner, httpentities::LoadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::LoadResponse>(status, body_str);
}

httpentities::BaseResponse HttpUtil::login(const std::string& url, httpentities::LoginRequest& request)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::BaseResponse HttpUtil::unload(const std::string& url, const bool& inner, httpentities::UnloadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::BuildResponse HttpUtil::build(const std::string& url, const bool& inner, httpentities::BuildRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::BuildResponse>(status, body_str);
}

httpentities::BaseResponse HttpUtil::drop(const std::string& url, const bool& inner, httpentities::DropRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::ShowResponse HttpUtil::show(const std::string& url, const bool& inner, httpentities::ShowRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::ShowResponse>(status, body_str);
}

httpentities::MonitorResponse HttpUtil::monitor(const std::string& url, const bool& inner, httpentities::MonitorRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::MonitorResponse>(status, body_str);
}

httpentities::QueryResponse HttpUtil::query(const std::string& url, const bool& inner, httpentities::QueryRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::QueryResponse>(status, body_str);
}


httpentities::BatchInsertResponse HttpUtil::batchInsert(const std::string& url, const bool& inner, httpentities::BatchInsertRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::BatchInsertResponse>(status, body_str);
}

httpentities::BatchRemoveResponse HttpUtil::batchRemove(const std::string& url, const bool& inner, httpentities::BatchRemoveRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	CURLcode status = Post(url, json_str, body_str);
	return response_parser<httpentities::BatchRemoveResponse>(status, body_str);
}

httpentities::ClusterResponse HttpUtil::reply(const std::string& url, httpentities::ReplyRequest& request, const std::string& username, const std::string& password)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	CURLcode status = Post(url, headers, 60, json_str, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

httpentities::ClusterResponse HttpUtil::appendEntries(const std::string& url, httpentities::AppenEntriesRequest& request, const std::string& username, const std::string& password)
{
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	std::map<std::string, std::string> params;
	params.insert(std::pair<std::string, std::string>("db_name", request.db_name));
	params.insert(std::pair<std::string, std::string>("term", std::to_string(request.term)));
	params.insert(std::pair<std::string, std::string>("index", std::to_string(request.index)));
	params.insert(std::pair<std::string, std::string>("nextIndex", std::to_string(request.nextIndex)));
	params.insert(std::pair<std::string, std::string>("uid", std::to_string(request.uid)));
	params.insert(std::pair<std::string, std::string>("updateType", request.updateType));
	std::string body_str;
	CURLcode status = PostFile(url, headers, 3600, request.file_path, params, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

httpentities::ClusterResponse HttpUtil::heartBeat(const std::string& url, httpentities::HeartBeatRequest& request, const std::string& username, const std::string& password)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	CURLcode status = Post(url, headers, 60, json_str, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

httpentities::ClusterResponse HttpUtil::clusterCheck(const std::string& url, httpentities::ClusterCheckRequest& request, const std::string& username, const std::string& password)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	CURLcode status = Post(url, headers, 60, json_str, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

httpentities::ClusterResponse HttpUtil::recoverFollower(const std::string& url, httpentities::RecoverRequest& request, const std::string& username, const std::string& password)
{
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	std::map<std::string, std::string> params;
	params.insert(std::pair<std::string, std::string>("db_name", request.db_name));
	params.insert(std::pair<std::string, std::string>("term", std::to_string(request.term)));
	params.insert(std::pair<std::string, std::string>("index", std::to_string(request.index)));
	params.insert(std::pair<std::string, std::string>("nextIndex", std::to_string(request.nextIndex)));
	params.insert(std::pair<std::string, std::string>("uid", std::to_string(request.uid)));
	params.insert(std::pair<std::string, std::string>("updateType", request.updateType));
	params.insert(std::pair<std::string, std::string>("recoverIndex", std::to_string(request.recoverIndex)));
	std::string body_str;
	CURLcode status = PostFile(url, headers, 3600, request.file_path, params, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}