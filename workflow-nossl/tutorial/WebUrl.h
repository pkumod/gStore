#pragma once
#ifndef WEB_URL_H_
#define WEB_URL_H)

#include <regex>
#include <string>
using namespace std;
class WebUrl {
public:
	WebUrl(const string& url): _url(url){}
	WebUrl(string&& url) :_url(move(url)) {}

	string Request(const string& request) const {
		smatch result;
		if (regex_search(_url.cbegin(), _url.cend(), result, regex(request + "=(.*?)&"))) {
			// 匹配具有多个参数的url

					// *? 重复任意次，但尽可能少重复  
			return result[1];

		}
		else if (regex_search(_url.cbegin(), _url.cend(), result, regex(request + "=(.*)"))) {
			// 匹配只有一个参数的url

			return result[1];

		}
		else {
			// 不含参数或制定参数不存在

			return string();

		}
	}
private:
	string _url;
};
#endif //WEB_URL_H_