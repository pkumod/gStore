/*=============================================================================
# Filename: shutdown.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2018-12-25 15:15
# Description: used to stop the ghttp server
=============================================================================*/

#include "../api/http/cpp/src/GstoreConnector.h"
#include "../Util/Util.h"

using namespace std;

#define SYSTEM_USERNAME "system"
string system_password;

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

    if (argc < 2)
	{
		/*cout << "please input the complete command:\t" << endl;
		cout << "\t bin/gadd -h" << endl;*/
		//Log.Error("Invalid arguments! Input \"bin/gbuild -h\" for help.");
		cout<<"Invalid arguments! Input \"bin/shutdown -h\" for help."<<endl;
		return 0;
	}
	else if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "Shutdown the ghttp server" << endl;
			cout << endl;
			cout << "Usage:\tbin/shutdown -p [port] " << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			cout << "\t-p,--port,\t\t the ghttp  server listen port . " << endl;
	
			cout << endl;
			return 0;
		}
		else
		{
			//cout << "the command is not complete." << endl;
			cout<<"Invalid arguments! Input \"bin/shutdown -h\" for help."<<endl;
			return 0;
		}
	}
	else
	{
		port = Util::getArgValue(argc, argv, "p", "port");
	
	fstream ofp;
	ofp.open("system.db/password" + port + ".txt", ios::in);
	ofp >> system_password;
	ofp.close();
	GstoreConnector gc;
	string res;
	int ret;
	string postdata="{\"username\":\"system\",\"password\":\""+system_password+"\"}";
	//ret = gc.Get("http://127.0.0.1:" + port + "/shutdown/?username=" + SYSTEM_USERNAME + "&password=" + system_password, res);
	ret=gc.Post("http://127.0.0.1:" + port + "/shutdown",postdata,res);
	if(res=="")
	{
		res="the Server is stopped successfully!";
	}
	cout<<"response:"<<res<<endl;
	return 0;
	}

	// if(argc == 1)
	// {
	// 	cout << "You need to input the server port that you want to stop." << endl;
	// 	return 0;
	// }
	// else if(argc == 2)
	// {
	// 	if(isNum(argv[1]))
	// 		port = argv[1];
	// 	else
	// 	{
	// 		cout << "wrong format of server port" <<endl;
	// 		return 0;
	// 	}
	// }
	// else
	// {
	// 	cout << "The number of parameters is not correct." << endl;
	// 	return 0;
	// }
	// fstream ofp;
	// ofp.open("system.db/password" + port + ".txt", ios::in);
	// ofp >> system_password;
	// ofp.close();
	// GstoreConnector gc;
	// string res;
	// int ret;
	// ret = gc.Get("http://127.0.0.1:" + port + "/?operation=shutdown&username=" + SYSTEM_USERNAME + "&password=" + system_password, res);
	// return 0;
}
