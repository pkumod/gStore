#include "../api/http/cpp/client.h"
#include "../Util/Util.h"

using namespace std;

bool isNum(char *str)
{
	for(int i = 0; i < strlen(str); i++)
	{
		int tmp = (int)(str[i]);
		if(tmp < 48 || tmp > 57)
			return false;
	}
	return true;
}

string int2string(int n)
{
	string s;
	stringstream ss;
	ss<<n;
	ss>>s;
	return s;
}

int main(int argc, char *argv[])
{
	string port;
	if(argc == 1)
	{
		cout << "You need to input the server port that you want to stop." << endl;
		return 0;
	}
	else if(argc == 2)
	{
		if(isNum(argv[1]))
			port = argv[1];
		else
		{
			cout << "wrong format of server port" <<endl;
			return 0;
		}
	}
	else
	{
		cout << "The number of parameters is not correct." << endl;
		return 0;
	}
	CHttpClient hc;
	string res;
	int ret;
	ret = hc.Get("http://127.0.0.1:"+port+"/?operation=stop", res);
	return 0;
}
