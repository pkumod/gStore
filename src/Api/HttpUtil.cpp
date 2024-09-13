#include "HttpUtil.h"

bool HttpUtil::m_bDebug = false;

HttpUtil::HttpUtil()
{

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

static size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	std::string* str = dynamic_cast<std::string*>((std::string*)lpVoid);
	if (NULL == str || NULL == buffer)
	{
		return -1;
	}

	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}

int HttpUtil::Get(const std::string& strUrl, std::string& strResponse)
{
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
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int HttpUtil::Get(const std::string& strUrl, const std::string& filename)
{
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
		return -1;
	}
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fw);

	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	res = curl_easy_perform(curl);

	curl_easy_cleanup(curl);
	fclose(fw);
	return res;
}

int HttpUtil::Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse)
{
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
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int HttpUtil::Post(const std::string& strUrl, const std::string& strPost, const std::string& filename)
{
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
		return -1;
	}
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fw);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	fclose(fw);
	return res;
}

httpentities::ShutdownResponse HttpUtil::shutdown(const std::string& url, httpentities::ShutdownRequest& request)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	if (status == CURLE_OK)
	{
		return httpentities::ShutdownResponse(body_str);
	}
	else
	{
		return httpentities::ShutdownResponse(-1, "curl error");
	}
}

httpentities::CheckResponse HttpUtil::check(const std::string& url, httpentities::CheckRequest& request)
{
	std::string param_str = request.to_params();
	std::string body_str;
	std::string strUrl = url + "?" + param_str;
	int status = Get(strUrl, body_str);
	if (status == CURLE_OK)
	{
		return httpentities::CheckResponse(body_str);
	}
	else
	{
		return httpentities::CheckResponse(-1, "curl error");
	}
}

httpentities::TestConnectionResponse HttpUtil::testConnection(const std::string& url, const bool& inner, httpentities::TestConnectionRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	if (status == CURLE_OK)
	{
		return httpentities::TestConnectionResponse(body_str);
	}
	else
	{
		return httpentities::TestConnectionResponse(-1, "curl error");
	}
}

httpentities::LoadResponse HttpUtil::load(const std::string& url, const bool& inner, httpentities::LoadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	int status = Post(url, json_str, body_str);
	if (status == CURLE_OK)
	{
		return httpentities::LoadResponse(body_str);
	}
	else
	{
		return httpentities::LoadResponse(-1, "curl error");
	}
}

httpentities::ClusterResponse HttpUtil::reply(const std::string& url, httpentities::ReplyRequest& request)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	int status = Post(url, json_str, body_str);
	if (status == CURLE_OK)
	{
		return httpentities::ClusterResponse(body_str);
	}
	else
	{
		return httpentities::ClusterResponse(-1, "curl error");
	}
}