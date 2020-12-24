/*
# Filename: client.h
# Author: suxunbin
# Last Modified: 2019-5-17 14:56
# Description: http api for C++
*/

#ifndef __HTTP_CURL_H__
#define __HTTP_CURL_H__

//REFERENCE: https://curl.haxx.se/
//libcurl is useful for developing http client, but not for server
//
//TODO: deal with cookie
//URL encode: http://www.ruanyifeng.com/blog/2010/02/url_encoding.html

#include <string>

class GstoreConnector {
  public:
  std::string serverIP;
  int serverPort;
  std::string Url;
  std::string username;
  std::string password;
  GstoreConnector(void);
  GstoreConnector(std::string _ip, int _port, std::string _user, std::string _passwd);
  ~GstoreConnector(void);

  public:
  /**
	* @brief HTTP POST请求
	* @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
	* @param strPost 输入参数,使用如下格式para1=val1?2=val2&…
	* @param strResponse 输出参数,返回的内容
	* @return 返回是否Post成功
	*/

  int Post(const std::string& strUrl, const std::string& strPost, const std::string& filename, bool SavedOnFile);

  int Post(const std::string& strUrl, const std::string& strPost, std::string& strResponse);

  /**
	* @brief HTTP GET请求
	* @param strUrl 输入参数,请求的Url地址,如:http://www.baidu.com
	* @param strResponse 输出参数,返回的内容
	* @return 返回是否Post成功
	*/

  int Get(const std::string& strUrl, const std::string& filename, bool SavedOnFile);

  int Get(const std::string& strUrl, std::string& strResponse);

  /**
	* @brief HTTPS POST请求,无证书版本
	* @param strUrl 输入参数,请求的Url地址,如:https://www.alipay.com
	* @param strPost 输入参数,使用如下格式para1=val1?2=val2&…
	* @param strResponse 输出参数,返回的内容
	* @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
	* @return 返回是否Post成功
	*/
  int Posts(const std::string& strUrl, const std::string& strPost, std::string& strResponse, const char* pCaPath = NULL);

  /**
	* @brief HTTPS GET请求,无证书版本
	* @param strUrl 输入参数,请求的Url地址,如:https://www.alipay.com
	* @param strResponse 输出参数,返回的内容
	* @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
	* @return 返回是否Post成功
	*/
  int Gets(const std::string& strUrl, std::string& strResponse, const char* pCaPath = NULL);

  public:
  std::string build(std::string db_name, std::string rdf_file_path, std::string request_type = "GET");
  std::string load(std::string db_name, std::string request_type = "GET");
  std::string unload(std::string db_name, std::string request_type = "GET");
  std::string user(std::string type, std::string username2, std::string addition, std::string request_type = "GET");
  std::string showUser(std::string request_type = "GET");
  std::string query(std::string db_name, std::string format, std::string sparql, std::string request_type = "GET");
  void fquery(std::string db_name, std::string format, std::string sparql, std::string filename, std::string request_type = "GET");
  std::string drop(std::string db_name, bool is_backup, std::string request_type = "GET");
  std::string monitor(std::string db_name, std::string request_type = "GET");
  std::string checkpoint(std::string db_name, std::string request_type = "GET");
  std::string show(std::string request_type = "GET");
  std::string getCoreVersion(std::string request_type = "GET");
  std::string getAPIVersion(std::string request_type = "GET");
  std::string exportDB(std::string db_name, std::string dir_path, std::string request_type = "GET");
  void SetDebug(bool bDebug);

  private:
  bool m_bDebug;
};

#endif
