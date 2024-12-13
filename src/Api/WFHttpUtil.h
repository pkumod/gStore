#pragma once

#include <workflow/WFTaskFactory.h>
#include <workflow/WFFacilities.h>
#include "../Util/GlobalTypedef.h"

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

static const std::string boundary_prefix_ = "----CppRestSdkClient";
static const std::string rand_chars_ = "0123456789"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
class MultipartParser
{
public:
  MultipartParser() {
    int i = 0;
    int len = rand_chars_.size();
    boundary_ = boundary_prefix_;
    while(i < 16)
    {
        int idx = rand() % len;
        boundary_.push_back(rand_chars_[idx]);
        ++i;
    }
  }
  inline const std::string &body_content()
  {
    return body_content_;
  }

  inline const std::string &boundary()
  {
    return boundary_;
  }

  inline void AddParameter(const std::string &name, const std::string &value)
  {
    params_.push_back(std::move(std::pair<std::string, std::string>(name, value)));
  }

  inline void AddFile(const std::string &name, const std::string &value)
  {
    files_.push_back(std::move(std::pair<std::string, std::string>(name, value)));
  }

  const std::string &GenBodyContent() {
    std::vector<std::future<std::string> > futures;
    body_content_.clear();
    for(auto &file:files_)
    {
        std::future<std::string> content_futures = std::async(std::launch::async, [&file]()
        {
        std::ifstream ifile(file.second, std::ios::binary | std::ios::ate);
        std::streamsize size = ifile.tellg();
        ifile.seekg(0, std::ios::beg);
        char *buff = new char[size];
        ifile.read(buff, size);
        ifile.close();
        std::string ret(buff, size);
        delete[] buff;
        return ret;
        });
        futures.push_back(std::move(content_futures));
    }

    for(auto &param:params_)
    {
        body_content_ += "\r\n--";
        body_content_ += boundary_;
        body_content_ += "\r\nContent-Disposition: form-data; name=\"";
        body_content_ += param.first;
        body_content_ += "\"\r\n\r\n";
        body_content_ += param.second;
    }

    for(size_t i = 0; i < files_.size(); ++i)
    {
        std::string filename;
        std::string content_type;
        std::string file_content = futures[i].get();
        _get_file_name_type(files_[i].second, &filename, &content_type);
        body_content_ += "\r\n--";
        body_content_ += boundary_;
        body_content_ += "\r\nContent-Disposition: form-data; name=\"";
        body_content_ += files_[i].first;
        body_content_ += "\"; filename=\"";
        body_content_ += filename;
        body_content_ += "\"\r\nContent-Type: ";
        body_content_ += content_type;
        body_content_ += "\r\n\r\n";
        body_content_ += file_content;
    }
    body_content_ += "\r\n--";
    body_content_ += boundary_;
    body_content_ += "--\r\n";
    if (!body_content_.empty())
    {
        body_content_ = body_content_.substr(2);
    }
    return body_content_;
}

private:
    void _get_file_name_type(const std::string &file_path, std::string *filename, std::string *content_type)
    {
    if (filename == NULL || content_type == NULL) return;

    size_t last_spliter = file_path.find_last_of("/\\");
    *filename = file_path.substr(last_spliter + 1);
    size_t dot_pos = filename->find_last_of(".");
    if (dot_pos == std::string::npos)
    {
        *content_type = "application/octet-stream";
        return;
    }
    std::string ext = filename->substr(dot_pos + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "jpg" || ext == "jpeg")
    {
        *content_type = "image/jpeg";
        return;
    }

    if (ext == "txt" || ext == "log")
    {
        *content_type = "text/plain";
        return;
    }
    *content_type = "application/octet-stream";
    return;
    }
    std::string boundary_;
    std::string body_content_;
    std::vector<std::pair<std::string, std::string> > params_;
    std::vector<std::pair<std::string, std::string> > files_;
    };

std::string http_wrapper(const std::string &url) {
    const char* tmp = url.c_str();
    if (strncasecmp(tmp, "http://", 7) != 0 &&
        strncasecmp(tmp, "https://", 8) != 0) 
    {
        return "http://" + url;
    }
    else 
    {
        return url;
    }
}
} // anonymous namespace

class WFHttpUtil
{
public:
	WFHttpUtil() { };
	~WFHttpUtil() { };
private:
	// static bool m_bDebug;
	static void respwrite_callback(WFHttpTask *task);

    static void fwrite_callback(WFHttpTask *task);

    static bool ErrorHandler(WFHttpTask *task);


public:
	/**
	* @brief: HTTP POST request
	* @param strUrl: the Url of the request, for example: http://api.gstore.cn
	* @param strPost: json string
	* @param filename: response write file name
	* @return: returned value
	*/
	static int Post(const std::string& strUrl, const std::string& strPost, const std::string& filename);

    /**
     * @brief: HTTP POST request
     * @param strUrl: the Url of the request, for example: http://api.gstore.cn
     * @param strPost: json string
     * @param strResponse: content returned
     * @return: returned value
     */
	static int Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse);

    /**
     * @brief: HTTP POST request
     * @param strUrl: the Url of the request, for example: http://api.gstore.cn
     * @param headers: HTTP head
     * @param timeOut: operation timeout (second), -1 means no timeout
     * @param strPost: json string
     * @param strResponse: content returned
     * @return: returned value
     */
	static int Post(const std::string& strUrl, const std::map<std::string, std::string>& headers, const int& timeOut, const std::string& strPost, std::string& strResponse);
	
    /**
	 * @brief: HTTP POST file request no bigger than 40MB
	 * @param strUrl: the Url of the request, for example: http://api.gstore.cn
	 * @param headers: HTTP head
	 * @param timeOut: operation timeout (second), -1 means no timeout
	 * @param filePath: upload file
	 * @param params: form data params
	 * @param strResponse: content returned
	 */
	static int PostFile(const std::string& strUrl, const std::map<std::string, std::string>& headers, const int& timeOut, const std::string& filePath, const std::map<std::string, std::string>& params, std::string& strResponse);
	
    /**
	* @brief: HTTP GET request
	* @param strUrl: the Url of the request, for example: http://api.gstore.cn
	* @param filename: response write file name
	* @return: returned value
	*/
	static int Get(const std::string& strUrl, const std::string& filename);
    
    /**
     * @brief: HTTP GET request
     * @param strUrl: the Url of the request, for example: http://api.gstore.cn
     * @param strResponse: content returned
     * @return: returned value
     */
	static int Get(const std::string& strUrl, std::string& strResponse);

    /**
     * @brief: HTTP GET request
     * @param strUrl: the Url of the request, for example: http://api.gstore.cn
     * @param headers: HTTP head
     * @param timeOut: operation timeout (second), -1 means no timeout
     * @param strResponse: content returned
     * @return: returned value
     */
	static int Get(const std::string& strUrl, const std::map<std::string, std::string>& headers, const int& timeOut, std::string& strResponse);
};
