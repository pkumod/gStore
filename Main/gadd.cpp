#include "../Database/Database.h"
#include "../Util/Util.h"
#include "../api/http/cpp/client.h"

using namespace std;
using namespace rapidjson;

int
main(int argc, char * argv[])
{
	//chdir(dirname(argv[0]));
//#ifdef DEBUG
	Util util;
//#endif

	cout << "argc: " << argc << "\t";
	cout << "DB_store:" << argv[1] << "\t";
	cout << "insert file:" << argv[2] << endl;

	string db_folder = string(argv[1]);
	int len = db_folder.length();

	if(db_folder.substr(len-3, 3) == ".db")
	{
		cout<<"your database can not end with .db"<<endl;
		return -1;
	}

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
	string username = "root";
	string password = "123456";
	string IP = "127.0.0.1";
	ofp.close();
	GstoreConnector gc(IP, port, username, password);
	gc.load(db_folder);
	string filename = argv[2];
    ofp.open(filename, ios::in);
    string line;
    
    int add_cnt = 0;
    int cnt = 0;

    while(getline(ofp, line))
    {
    	string newline = "INSERT DATA {";
    	string res;
        newline += line;
        newline += "}";
        res = gc.query(db_folder, "json", newline);
        Document document;
		document.Parse(res.c_str());
		cnt++;
		if(document["StatusCode"].GetInt() == 402) add_cnt++;
    }
    ofp.close();
    cout << "triple num: " << cnt << endl;
    cout << "add triple num: " << add_cnt << endl;
	return 0;
}
