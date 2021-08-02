#pragma once
# #ifndef WEB_URL_H_

#endif // !WEB_URL_H_

#include <string>
using namespace std;
class WebUrl {
public:
	WebUrl(const string& url): _url(url){}
	WebUrl(string&& url) :_url(move(url)) {}

	string Request(const string& request) const;
private:
	string _url;
};