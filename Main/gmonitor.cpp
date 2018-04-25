#include <curl/curl.h>
#include <string>
#include <cstring>
#include <iostream>

using namespace std;

const string UrlEncode(const string& s)
{
        string ret;
        unsigned char *ptr = (unsigned char *)s.c_str();
        ret.reserve(s.length());

        for(int i=0;i<s.length();++i)
        {
                if((int(ptr[i])==42) || (int(ptr[i])==45) || (int(ptr[i])==46) || (int(ptr[i])==47) || (int(ptr[i])==58) ||(int(ptr[i])==95))
                        ret += ptr[i];
                else if((int(ptr[i])>=48) && (int(ptr[i])<=57))
                        ret += ptr[i];
                else if((int(ptr[i])>=65) && (int(ptr[i])<=90))
                        ret += ptr[i];
                else if((int(ptr[i])>=97) && (int(ptr[i])<=122))
                        ret += ptr[i];
                else if(int(ptr[i])==32)
                        ret += '+';
                else
                {
                        char buf[5];
                        memset(buf,0,5);
                        snprintf(buf,5,"%%%X",ptr[i]);
                        ret.append(buf);
                }
        }
        return ret;
}
size_t OnWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)
{
	string* str = dynamic_cast<string*>((string *)lpVoid);
	if( NULL == str || NULL == buffer )
		return -1;
	char* pData = (char*)buffer;
	str->append(pData, size * nmemb);
	return nmemb;
}
int
main(int argc, char * argv[])
{
	cout << "argc: " << argc << endl;
	cout << "ip: " << argv[1] << endl;
	cout << "port: " << argv[2] << endl;
	cout << "db_name: " << argv[3] << endl;

	string serverIP = string(argv[1]);
	string serverPort = string(argv[2]);
	string db_name = string(argv[3]);

	const string strUrl = "http://"+serverIP+":"+serverPort+"/?operation=monitor&db_name="+db_name;
	string res;
	res.clear();
	CURLcode ret;
	CURL* curl = curl_easy_init();
	if(NULL == curl)
		return CURLE_FAILED_INIT;
	curl_easy_setopt(curl, CURLOPT_URL, UrlEncode(strUrl).c_str());
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, NULL);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, OnWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&res);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3);
	ret = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	cout<< res <<endl;
	return 0;
}
