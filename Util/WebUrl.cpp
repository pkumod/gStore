#include "WebUrl.h"

using namespace std;
WebUrl::Request(const string& request) const {
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