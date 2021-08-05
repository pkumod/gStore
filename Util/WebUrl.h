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

	static string Request(string& url,string& request) const;
private:
	string _url;
};
#endif //WEB_URL_H_