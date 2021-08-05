#pragma once
#ifndef WEB_URL_H_
#define WEB_URL_H_


#include <string>
using namespace std;
class WebUrl {
public:
	WebUrl();
	~WebUrl();
	static string CutParam(string url, string param);

};
#endif //WEB_URL_H_