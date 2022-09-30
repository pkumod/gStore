#include "GstoreConnector.h"

GstoreConnector::GstoreConnector(void) :
	m_bDebug(false)
{

}
GstoreConnector::GstoreConnector(std::string _ip, int _port, std::string _http_type, std::string _user, std::string _passwd) :
	m_bDebug(false)
{
	if (_ip == "localhost")
		this->serverIP = defaultServerIP;
	else
		this->serverIP = _ip;
	this->serverPort = _port;
	this->Url = "http://" + this->serverIP + ":" + std::to_string(this->serverPort) + "/";
	if (_http_type == "grpc")
	{
		this->Url = this->Url + "grpc/api";
	}
	this->username = _user;
	this->password = _passwd;
}
GstoreConnector::~GstoreConnector(void)
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

int GstoreConnector::Get(const std::string& strUrl, std::string& strResponse)
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

int GstoreConnector::Get(const std::string& strUrl, const std::string& filename)
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

int GstoreConnector::Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse)
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

int GstoreConnector::Post(const std::string& strUrl, const std::string& strPost, const std::string& filename)
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

int GstoreConnector::Gets(const std::string& strUrl, std::string& strResponse, const char* pCaPath)
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
	if (NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	//curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

int GstoreConnector::Posts(const std::string& strUrl, const std::string& strPost, std::string& strResponse, const char* pCaPath)
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
	curl_easy_setopt(curl, CURLOPT_POST, 1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, strPost.c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&strResponse);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	if (NULL == pCaPath)
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, false);
	}
	else
	{
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
		curl_easy_setopt(curl, CURLOPT_CAINFO, pCaPath);
	}
	//curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	//curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	res = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

std::string GstoreConnector::build(std::string db_name, std::string db_path, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=build&db_name=" + db_name + "&db_path=" + db_path + "&username=" + this->username + "&password=" + this->password;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"build\", \"db_name\": \"" + db_name + "\", \"db_path\": \"" + db_path + "\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::check(std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=check";
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"check\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::load(std::string db_name, std::string csr, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=load&db_name=" + db_name + "&csr=" + csr + "&username=" + this->username + "&password=" + this->password;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"load\", \"db_name\": \"" + db_name + "\", \"csr\": \"" + csr + "\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::monitor(std::string db_name, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=monitor&db_name=" + db_name + "&username=" + this->username + "&password=" + this->password;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"monitor\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::unload(std::string db_name, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=unload&db_name=" + db_name + "&username=" + this->username + "&password=" + this->password;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"unload\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::drop(std::string db_name, bool is_backup, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl;
		if (is_backup)
			strUrl = this->Url + "?operation=drop&db_name=" + db_name + "&username=" + this->username + "&password=" + this->password + "&is_backup=true";
		else
			strUrl = this->Url + "?operation=drop&db_name=" + db_name + "&username=" + this->username + "&password=" + this->password + "&is_backup=false";
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost;
		if (is_backup)
			strPost = "{\"operation\": \"drop\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"is_backup\": \"true\"}";
		else
			strPost = "{\"operation\": \"drop\", \"db_name\": \"" + db_name + "\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"is_backup\": \"false\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::show(std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=show&username=" + this->username + "&password=" + this->password;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"show\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::usermanage(std::string type, std::string op_username, std::string op_password, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=usermanage&type=" + type + "&username=" + this->username + "&password=" + this->password + "&op_username=" + op_username + "&op_password=" + op_password;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"usermanage\", \"type\": \"" + type + "\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"op_username\": \"" + op_username + "\", \"op_password\": \"" + op_password + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::showuser(std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=showuser&username=" + this->username + "&password=" + this->password;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"showuser\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::userprivilegemanage(std::string type, std::string op_username, std::string privileges, std::string db_name, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "operation=userprivilegemanage&type=" + type + "&username=" + this->username + "&password=" + this->password + "&op_username=" + op_username + "&privileges=" + privileges + "&db_name=" + db_name;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"userprivilegemanage\", \"type\": \"" + type + "\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"op_username\": \"" + op_username + "\", \"privileges\": \"" + privileges + "\", \"db_name\": \"" + db_name + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::backup(std::string db_name, std::string backup_path, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=backup&username=" + this->username + "&password=" + this->password + "&db_name=" + db_name + "&backup_path=" + backup_path;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"backup\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"db_name\": \"" + db_name + "\", \"backup_path\": \"" + backup_path + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::restore(std::string db_name, std::string backup_path, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=restore&username=" + this->username + "&password=" + this->password + "&db_name=" + db_name + "&backup_path=" + backup_path;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"restore\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"db_name\": \"" + db_name + "\", \"backup_path\": \"" + backup_path + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::query(std::string db_name, std::string format, std::string sparql, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=query&username=" + this->username + "&password=" + this->password + "&db_name=" + db_name + "&format=" + format + "&sparql=" + sparql;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"query\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"db_name\": \"" + db_name + "\", \"format\": \"" + format + "\", \"sparql\": \"" + sparql + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

void GstoreConnector::fquery(std::string db_name, std::string format, std::string sparql, std::string filename, std::string request_type)
{
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=query&username=" + this->username + "&password=" + this->password + "&db_name=" + db_name + "&format=" + format + "&sparql=" + sparql;
		int ret = this->Get(strUrl, filename);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"query\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"db_name\": \"" + db_name + "\", \"format\": \"" + format + "\", \"sparql\": \"" + sparql + "\"}";
		int ret = this->Post(this->Url, strPost, filename);
	}
	return;
}

std::string GstoreConnector::exportDB(std::string db_name, std::string db_path, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=export&username=" + this->username + "&password=" + this->password + "&db_name=" + db_name + "&db_path=" + db_path;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"export\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"db_name\": \"" + db_name + "\", \"db_path\": \"" + db_path + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::login(std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=login&username=" + this->username + "&password=" + this->password;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"login\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::begin(std::string db_name, std::string isolevel, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=begin&username=" + this->username + "&password=" + this->password + "&db_name=" + db_name + "&isolevel=" + isolevel;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"begin\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"db_name\": \"" + db_name + "\", \"isolevel\": \"" + isolevel + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}

	return res;
}

std::string GstoreConnector::tquery(std::string db_name, std::string tid, std::string sparql, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=tquery&username=" + this->username + "&password=" + this->password + "&db_name=" + db_name + "&tid=" + tid + "&sparql=" + sparql;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"tquery\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"db_name\": \"" + db_name + "\", \"tid\": \"" + tid + "\", \"sparql\": \"" + sparql + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::commit(std::string db_name, std::string tid, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=commit&username=" + this->username + "&password=" + this->password + "&db_name=" + db_name + "&tid=" + tid;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"commit\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"db_name\": \"" + db_name + "\", \"tid\": \"" + tid + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::rollback(std::string db_name, std::string tid, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=rollback&username=" + this->username + "&password=" + this->password + "&db_name=" + db_name + "&tid=" + tid;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"rollback\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"db_name\": \"" + db_name + "\", \"tid\": \"" + tid + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::getTransLog(std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=txnlog&username=" + this->username + "&password=" + this->password;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"txnlog\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

std::string GstoreConnector::checkpoint(std::string db_name, std::string request_type)
{
	std::string res;
	if (request_type == "GET")
	{
		std::string strUrl = this->Url + "?operation=checkpoint&username=" + this->username + "&password=" + this->password + "&db_name=" + db_name;
		int ret = this->Get(strUrl, res);
	}
	else if (request_type == "POST")
	{
		std::string strPost = "{\"operation\": \"checkpoint\", \"username\": \"" + this->username + "\", \"password\": \"" + this->password + "\", \"db_name\": \"" + db_name + "\"}";
		int ret = this->Post(this->Url, strPost, res);
	}
	return res;
}

void GstoreConnector::SetDebug(bool bDebug)
{
	m_bDebug = bDebug;
}
