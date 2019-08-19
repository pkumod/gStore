/*=============================================================================
# Filename: gshow.cpp
# Author: suxunbin
# Mail: suxunbin@pku.edu.cn
# Last Modified: 2019-07-25 17:00
# Description: used to show all the databases that have already been built
=============================================================================*/

#include "../Util/Util.h"
#include "../Database/Database.h"
#include "../api/http/cpp/client.h"

using namespace std;
using namespace rapidjson;

struct DBInfo{
        public:
                string db_name;
                string creator;
                string built_time;
                string is_backup;
                string backup_interval;
                string backup_timer;
                string last_backup_time;
                DBInfo(){
                }
                DBInfo(string _db_name){
                        db_name = _db_name;
                }
                ~DBInfo(){
                }
};

string Getstr(string s)
{
        int len = s.length();
        return s.substr(1,len-2);
}

int main(int argc, char * argv[])
{
        fstream ofp;
        ofp.open("./system.db/port.txt", ios::in);
        int ch = ofp.get();
        if(ofp.eof()){
                cout << "ghttp is not running!" << endl;
                return 0;
        }
        ofp.close();
        ofp.open("./system.db/port.txt", ios::in);
        int port;
        ofp >> port;
        ofp.close();
        string username = "root";
        string password = "123456";
        string IP = "127.0.0.1";
        GstoreConnector gc(IP, port, username, password);
        string res = gc.show();
        Document document;
        document.Parse(res.c_str());
        cout << "StatusCode: " << document["StatusCode"].GetInt() << endl;
        cout << "StatusMsg: " << document["StatusMsg"].GetString() << endl;
        Value &dbs = document["ResponseBody"];

        for (size_t i = 0; i < dbs.Size(); i++)
        {
           Value &v = dbs[i];
           cout << "==================================================================" << endl;
           cout << "database: " << v["database"].GetString() << endl;
           cout << "creator: " << v["creator"].GetString() << endl;
           cout << "built_time: " << v["built_time"].GetString() << endl;
           cout << "status: " << v["status"].GetString() << endl;
        }
        cout << "==================================================================" << endl;
        return 0;
}

