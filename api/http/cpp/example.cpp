//NOTICE: you need to use libcurl-devel for C++ to use HTTP client, please read client.cpp and client.h seriously


#include <stdio.h> 
#include <iostream>
#include <string> 
#include "client.h"

using namespace std;

int main()
{
	CHttpClient hc;
	string res;
	int ret;
	//NOTICE: here no need to add admin.html
	ret = hc.Get("127.0.0.1:8080/build/lubm/data/LUBM_10.n3", res); 
	cout<<res<<endl;
	ret = hc.Get("127.0.0.1:8080/load/lubm", res); 
	cout<<res<<endl;
	ret = hc.Get("127.0.0.1:8080/query/data/ex0.sql", res); 
	cout<<res<<endl;
	ret = hc.Get("127.0.0.1:8080/monitor", res); 
	cout<<res<<endl;
	ret = hc.Get("127.0.0.1:8080/unload", res); 
	cout<<res<<endl;

	return 0;
}

