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

bool is_number(string s)
{
    if (s.empty())
    {
        return false;
    }
    string::size_type pos = 0;
    for(; pos < s.size(); pos++){
        if(!isdigit(s[pos])) return false;
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

string gc_getUrl(string _type, string _port)
{
    string _url = "";
    _url.append("http://127.0.0.1:").append(_port);
    if (_type == "grpc")
    {
        _url.append("/grpc");
    }
    return _url;
}

int gc_check(GstoreConnector &gc, string _type, string _port, string &res)
{
    std::string strUrl = gc_getUrl(_type, _port).append("/api");
	std::string strPost;
	if (_type == "grpc")
    {
        strUrl.append("/grpc");
		strPost = "operation=check";
    }
	else
	{
		strUrl.append("/");
		strPost = "{\"operation\": \"check\"}";
	}
    int ret = gc.Post(strUrl, strPost, res);
	// cout << "url: " << strUrl << ", ret: " << ret << ", res: " << res << endl;
    return ret;
}

int main(int argc, char *argv[])
{
	Util util;
	if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "Shutdown the http server" << endl;
			cout << endl;
			cout << "Usage:\tbin/shutdown -t [type]" << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
			// cout << "\t-p,--port,\t\t the ghttp  server listen port. " << endl;
			// cout << "\t-t,--type,\t\tthe server type. grpc or ghttp, Default value is grpc." << endl;
			cout << endl;
			return 0;
		}
		else
		{
			// cout << "the command is not complete." << endl;
			cout << "Invalid arguments! Input \"bin/shutdown -h\" for help." << endl;
			return 0;
		}
	}
	else
	{
		// port = Util::getArgValue(argc, argv, "p", "port");
		if (Util::file_exist("system.db/port.txt") == false)
		{
			cout << "http server is not running!" << endl;
			return 0;
		}

		GstoreConnector gc;
		string port;
		string type;
		string system_password;

		string type_port;
		fstream ofp;
		ofp.open("system.db/port.txt", ios::in);
		ofp >> type_port;
		ofp.close();
		if (type_port.find(":") != string::npos)
		{
			std::vector<std::string> res;
			Util::split(type_port, ":", res);
			if (res.size() == 2)
			{
				type = res[0];
				port = res[1];
			} 
		} 
		else if (is_number(type_port))
		{
			port = type_port;
			string res = "";
			gc_check(gc, "ghttp", port, res);
			rapidjson::Document document;
			document.SetObject();
			document.Parse(res.c_str());
			// ghttp server is running
			if(document.HasMember("StatusCode") && document["StatusCode"].GetInt() == 0)
			{
				type = "ghttp";
			}
			else
			{
				res = "";
				gc_check(gc, "grpc", port, res);
				document.Parse(res.c_str());
				// grpc server is running
				if(document.HasMember("StatusCode") && document["StatusCode"].GetInt() == 0)
				{
					type = "grpc";
				} 
				else
				{
					string cmd = "rm system.db/password*.txt";;
					system(cmd.c_str());
					cmd = "rm system.db/port.txt";
					system(cmd.c_str());
					cout << "http server is not running!" << endl;
					return 0;
				}
			}
		}
		else
		{
			cout << "http server port is invalid: " << type_port << endl;
			return 0;
		}
		
		cout << "server type: " << type << ", port: " << port << endl;
		ofp.open("system.db/password" + port + ".txt", ios::in);
		ofp >> system_password;
		ofp.close();
		
		util.configure_new();
		string system_user = util.getConfigureValue("system_username");
		string res;
		string postdata;
		string strUrl = gc_getUrl(type, port);
		strUrl.append("/shutdown");
		if (type == "grpc")
		{
			postdata = "username=" + system_user + "&password=" + system_password;
			gc.Post(strUrl, postdata, res);
			cout << "Result: " << res << endl;
			rapidjson::Document jsonRes;
			jsonRes.Parse(res.c_str());
			if (jsonRes.HasParseError())
			{
				cout << "the server stop fail.";
			}
			if (jsonRes.HasMember("StatusCode") && jsonRes["StatusCode"].GetInt() == 0)
			{
				if (Util::file_exist("system.db/port.txt"))
				{
					string cmd = "rm system.db/password*.txt";;
					system(cmd.c_str());
					cmd = "rm system.db/port.txt";
					system(cmd.c_str());
				}
				cout << "the Server is stopped successfully!";
			}
		} 
		else
		{
			postdata = "{\"username\":\"" + system_user + "\",\"password\":\"" + system_password + "\"}";
			gc.Post(strUrl, postdata, res);
			if (Util::file_exist("system.db/port.txt"))
			{
				string cmd = "rm system.db/password*.txt";
				system(cmd.c_str());
				cmd = "rm system.db/port.txt";
				system(cmd.c_str());
			}
			cout << "the Server is stopped successfully!";
		}
		return 0;
	}
}
