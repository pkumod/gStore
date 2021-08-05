#include "WebUrl.h"

using namespace std;
string WebUrl::Request(string& url,string& request) {
	smatch result;
	if (regex_search(url.cbegin(), url.cend(), result, regex(request + "=(.*?)&"))) {
		// 匹配具有多个参数的url

				// *? 重复任意次，但尽可能少重复  
		return result[1];

	}
	else if (regex_search(url.cbegin(), url.cend(), result, regex(request + "=(.*)"))) {
		// 匹配只有一个参数的url

		return result[1];

	}
	else {
		// 不含参数或制定参数不存在

		return string();

	}
}

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