/*=============================================================================
# Filename: shutdown.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2018-12-25 15:15
# Description: used to stop the ghttp server
=============================================================================*/

#include "../Api/HttpUtil.h"
#include "../Util/Util.h"

using namespace std;

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

int gc_check(string _type, string _port, string &res)
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
    int ret = HttpUtil::Post(strUrl, strPost, res);
	// cout << "url: " << strUrl << ", ret: " << ret << ", res: " << res << endl;
    return ret;
}

int main(int argc, char *argv[])
{
	Util util;
	string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
	if (argc == 2)
	{
		string command = argv[1];
		if (command == "-h" || command == "--help")
		{
			cout << endl;
			cout << "Shutdown the http server" << endl;
			cout << endl;
			cout << "Usage:\tbin/shutdown" << endl;
			cout << endl;
			cout << "Options:" << endl;
			cout << "\t-h,--help\t\tDisplay this message." << endl;
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
		string system_port_path = _db_home + "/system" + _db_suffix + "/port.txt";
		if (Util::file_exist(system_port_path) == false)
		{
			cout << "http server is not running!" << endl;
			return 0;
		}

		string port;
		string type;
		string system_password;

		string type_port;
		fstream ofp;
		ofp.open(system_port_path, ios::in);
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
				string res = "";
				gc_check(type, port, res);
				rapidjson::Document document;
				document.SetObject();
				document.Parse(res.c_str());
				if (document.HasParseError()) {
					string cmd = _db_home + "/system" +_db_suffix + "/password*.txt";
					Util::remove_path(cmd);
					Util::remove_path(system_port_path);
					cout << "http server is not running!" << endl;
					return 0;
				}
			}
			else
			{
				cout << "http server port is invalid: " << type_port << endl;
				return 0;
			}
		}
		else if (Util::is_number(type_port))
		{
			port = type_port;
			string res = "";
			gc_check("ghttp", port, res);
			rapidjson::Document document;
			document.SetObject();
			document.Parse(res.c_str());
			// ghttp server is running
			if(!document.HasParseError() && document.HasMember("StatusCode") && document["StatusCode"].GetInt() == 0)
			{
				type = "ghttp";
			}
			else
			{
				res = "";
				gc_check("grpc", port, res);
				document.Parse(res.c_str());
				// grpc server is running
				if(!document.HasParseError() && document.HasMember("StatusCode") && document["StatusCode"].GetInt() == 0)
				{
					type = "grpc";
				} 
				else
				{
					string cmd = _db_home + "/system" +_db_suffix + "/password*.txt";
					Util::remove_path(cmd);
					Util::remove_path(system_port_path);
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
		string system_password_path = _db_home + "/system" +_db_suffix + "/password" + port + ".txt";
		if (util.file_exist(system_password_path) == false)
		{
			cout << "password file missed, shutdown fail!" << endl;
			return 0;
		}
		ofp.open(system_password_path, ios::in);
		ofp >> system_password;
		ofp.close();
		
		string system_user = util.getConfigureValue("system_username");
		string res;
		string postdata;
		string strUrl = gc_getUrl(type, port);
		strUrl.append("/shutdown");
		if (type == "grpc")
		{
			postdata = "username=" + system_user + "&password=" + system_password;
		} 
		else
		{
			postdata = "{\"username\":\"" + system_user + "\",\"password\":\"" + system_password + "\"}";
		}
		HttpUtil::Post(strUrl, postdata, res);
		rapidjson::Document jsonRes;
		jsonRes.Parse(res.c_str());
		if (!jsonRes.HasParseError() && jsonRes.HasMember("StatusCode") && jsonRes["StatusCode"].GetInt() == 0)
		{
			cout << "the Server is stopped successfully!" << endl;
		}
		else
		{
			cout << "the server stop fail." << endl;
		}
		return 0;
	}
}
