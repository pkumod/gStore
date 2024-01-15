#include "WebUrl.h"

using namespace std;

WebUrl::WebUrl() {}

WebUrl::~WebUrl() {}
string WebUrl::CutParam(string url, string param)
{
	int index = url.find("?" + param + "=");
	if (index < 0)
	{
		index = url.find("&" + param + "=");
	}
	if (index < 0)
		return string();
	int index2 = index + 1 + (param + "=").length();
	string substr = url.substr(index2);
	int endindex = substr.find("&");
	if (endindex > 0)
	{
		string substr2 = substr.substr(0, endindex);
		return substr2;
	}
	else
		return substr;

}