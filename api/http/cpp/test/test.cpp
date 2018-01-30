#include <iostream>
#include <string>
#include <cstring>
using namespace std;
string encode(string& s)
{
	string ret;
	unsigned char *ptr = (unsigned char *)s.c_str();
	ret.reserve(s.length());
	
	for(int i=0;i<s.length();++i)
	{
		if((int(ptr[i])==58) || (int(ptr[i])==47) ||(int(ptr[i])==42) || (int(ptr[i])==45) || (int(ptr[i])==46) || (int(ptr[i])==95))
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
string decode(string& s)
{
        string ret;
        char ch;
        int ii;
        for(size_t i = 0; i < s.length(); ++i)
        {
                if(int(s[i]) == 37)
                {
                        sscanf(s.substr(i+1,2).c_str(), "%x", &ii);
                        ch = static_cast<char>(ii);
                        ret += ch;
                        i = i + 2;
                }
                else if(s[i] == '+')
                {
                        ret += ' ';
                }
                else
                {
                        ret += s[i];
                }
        }
        return (ret);
}

int main()
{
	string a;
	cin >> a;
	string b=encode(a);
	cout<<b<<endl;
	string c=decode(b);
	if(a==c)
		cout<<"OK"<<endl;
	return 0;
}
