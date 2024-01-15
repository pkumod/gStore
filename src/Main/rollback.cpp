#include "../Database/Database.h"
#include "../Util/Util.h"
#include "../api/http/cpp/src/GstoreConnector.h"
#include <regex>

using namespace std;
using namespace rapidjson;

const string USERNAME = "root";

int get_all_folders(string path, string folder_name, vector<string> &folders)
{
    DIR *dp = NULL;

    dp = opendir(path.c_str());
    if (!dp) {
        fprintf(stderr, "opendir: %s\n", strerror(errno));
        return -1;
    }

    struct dirent *dirp;
    while ((dirp = readdir(dp))) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
            continue;
        string folder = dirp->d_name;
        if (folder.find(folder_name.c_str()) != string::npos)
        {
            folders.push_back(folder);
        }
    }
    closedir(dp);

    return 0;
}

string undo_sparql(string line)
{
    string undo_sparql;
    if(line[0] == 'I'){
        undo_sparql = "Delete DATA{";
    }else{
        undo_sparql = "INSERT DATA{";
    }
    undo_sparql += line.substr(2, line.length());
    undo_sparql += '}';
    return undo_sparql;
}

string get_postfix(string datetime)
{
    string postfix;
    postfix = datetime.substr(2,2);
    postfix += datetime.substr(5,2);
    postfix += datetime.substr(8,2);
    postfix += datetime.substr(11,2);
    postfix += datetime.substr(14,2);
    postfix += datetime.substr(17,2);
    return postfix;
}

string gc_getUrl(string _type, string _port)
{
    string _url = "";
    _url.append("http://127.0.0.1:").append(_port);
    if (_type == "grpc")
    {
        _url.append("/grpc/api");
    }
    return _url;
}

int gc_check(GstoreConnector &gc, string _type, string _port, string &res)
{
    string strUrl = gc_getUrl(_type, _port);
    std::string strPost;
    if (_type == "grpc")
	{
		strPost = "operation=check";
	}
	else
	{
		strPost = "{\"operation\": \"check\"}";
	}
    int ret = gc.Post(strUrl, strPost, res);
    // cout << "url: " << strUrl << ", ret: " << ret << ", res: " << res << endl;
    return ret;
}

int gc_unload(GstoreConnector &gc, string _type, string _port, string _pwd, string _db_name, string &res)
{
    string strUrl = gc_getUrl(_type, _port);
    std::string strPost = "{\"operation\": \"unload\", \"db_name\": \"" + _db_name + "\", \"username\": \"" + USERNAME + "\", \"password\": \"" + _pwd + "\"}";
    int ret = gc.Post(strUrl, strPost, res);
    // cout << "url: " << strUrl << ", ret: " << ret << ", res: " << res << endl;
    return ret;
}

int
main(int argc, char * argv[])
{
//#ifdef DEBUG
	Util util;
//#endif= 
    fstream ofp;
    // ofp.open("./system.db/port.txt", ios::in);
    // int ch = ofp.get();
    // if(ofp.eof()){
    //     cout << "ghttp is not running!" << endl;
    //     return 0;
    // }
    // ofp.close();
    string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
    string _default_backup_path = util.backup_path;
	size_t _len_suffix = _db_suffix.length();
    string db_name, backup_date, backup_time, restore_time;
    if (argc < 2 || (2 < argc && argc < 7))
	{
		cout << "Invalid arguments! Input \"bin/rollback -h\" for help." << endl;
		return 0;
	}
    else if (argc == 2)
    {
        string command = argv[1];
		if (command == "-h" || command == "--help")
        {
            cout << endl;
            cout << "gStore Rollback Tools(rollback)" << endl;
            cout << endl;
            cout << "Usage:\tbin/rollback -db [dbname] -d [date] -t [time]" << endl;
            cout << endl;
            cout << "Options:" << endl;
            cout << "\t-h,--help\t\tDisplay this message." << endl;
            cout << "\t-db,--database,\t\tthe database name." << endl;
            cout << "\t-d,--date,\t\tthe database restore date, format is yyyy-mm-dd." << endl;
            cout << "\t-t,--time,\t\tthe database restore time, format is hh:mm:ss." << endl;
            cout << endl;
            return 0;
        }
		else
		{
			cout<<"Invalid arguments! Input \"bin/rollback -h\" for help."<<endl;
			return 0;
		}
    }
    else
	{
		db_name = Util::getArgValue(argc, argv, "db", "database");
		backup_date = Util::getArgValue(argc, argv, "d", "date");
		backup_time = Util::getArgValue(argc, argv, "t", "time");
		size_t len = db_name.length();
		if (len > _len_suffix && db_name.substr(len - _len_suffix, _len_suffix) == _db_suffix)
		{
			cout<<"your database name can not end with " + _db_suffix + "! Input \"bin/rollback -h\" for help."<<endl;
			return -1;
		}
		if (db_name == "system")
		{
			cout << "Your database's name can not be system." << endl;
			return -1;
		}

    }

    cout << "argc: " << argc << endl;
    cout << "DB_store: " << db_name << endl;
    cout << "Restore Point(date): " << backup_date << endl;
    cout << "Restore Point(time): " << backup_time << endl;
    
    // check date format
    std::regex datePattern("(\\d{4})-(0\\d{1}|1[0-2])-(0\\d{1}|[12]\\d{1}|3[01])");
    bool match_flag = std::regex_match(backup_date, datePattern);
    if (match_flag == false)
    {
        cout << "the date value is invalid, fomat is yyyy-MM-dd." << endl;
        return 0;
    }
    // check time format
    std::regex timePattern("(0\\d{1}|1\\d{1}|2[0-3]):[0-5]\\d{1}:([0-5]\\d{1})");
    match_flag = std::regex_match(backup_time, timePattern);
    if (match_flag == false)
    {
        cout << "the time value is invalid, fomat is hh:mm:ss." << endl;
        return 0;
    }
    
    restore_time = backup_date + ' ' + backup_time;

    vector<string> folders;
    string folder_name = db_name +_db_suffix + "_";
    get_all_folders(_default_backup_path, folder_name, folders);
    if(folders.size() == 0){
        cout << "Backups Folder Empty, Please check " + _default_backup_path << endl;
        return 0;
    }
    cout << restore_time << endl;
    int timestamp = Util::time_to_stamp(restore_time);
    cout << timestamp << endl;
    if(timestamp >  Util::get_cur_time() / 1000){
        cout << "Restore Time Error, Rollback Failed." << endl;
        return 0;
    }
    string backup_name = db_name + _db_suffix + "_" + get_postfix(restore_time);
    cout << backup_name << endl;
    sort(folders.begin(), folders.end());
    size_t inx = lower_bound(folders.begin(), folders.end(), backup_name) - folders.begin();
    cout << "match folder is: " << folders[inx] << endl;
    if(inx >= folders.size() || folders[inx].find(db_name + _db_suffix) == string::npos){
        cout << "No Backups for Database " + db_name << "!" << endl;
        return 0;
    }

    // check http server status
    string system_port_path = _db_home + "/system" + _db_suffix + "/port.txt";
    if (Util::file_exist(system_port_path))
    {
        cout << "http server is running!" << endl;
        string port;
        string type;
        string type_port;
        GstoreConnector gc;
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
            } 
        }
        else if (Util::is_number(type_port))
        {
            port = type_port;
            string res;
            gc_check(gc, "ghttp", port, res);
            Document document;
            document.SetObject();
            document.Parse(res.c_str());
            // ghttp server is running
            if(document.HasMember("StatusCode") && document["StatusCode"].GetInt() == 0)
            {
                type = "ghttp";
            }
            else
            {
                type = "grpc";
            }
        }
        else
        {
            cout << "http server port is invalid: " << type_port << endl;
            return 0;
        }
        string res = "";
        gc_check(gc, type, port, res);
        Document document;
        document.SetObject();
        document.Parse(res.c_str());
        if(document.HasMember("StatusCode") && document["StatusCode"].GetInt() == 0)
        {
            Database system_db("system");
            system_db.load();
            string root_pwd = "";
            string query_sparql = "select ?x where{<"+USERNAME+"> <has_password> ?x.}";
            ResultSet query_rs;
            FILE* query_ofp = stdout;
            system_db.query(query_sparql, query_rs, query_ofp);
            root_pwd = query_rs.answer[0][0];
            root_pwd = Util::replace_all(root_pwd, "\"", "");
            // cout << "root_pwd: " << root_pwd << endl;
            system_db.unload();
            //unload
            res = "";
            gc_unload(gc, type, port, root_pwd, db_name, res);
            cout << "Unload result: " << res << endl;
            document.Parse(res.c_str());
            if(document.HasMember("StatusCode") && document["StatusCode"].GetInt() != 0 && document["StatusCode"].GetInt() != 304)
            {
                res = document["StatusMsg"].GetString();
                if (res != "the database not load yet.")
                {   
                    cout << "Rollback Failed: please unload the database from "<< type << endl;
                    return 0;
                }
            }
        }
    }
        
    //cover the old
    string cmd;
    string db_path = _db_home + "/" + db_name + _db_suffix;
    cmd = "cp -r " + _default_backup_path + "/" + folders[inx] + " " + _db_home;
    system(cmd.c_str());
    cout << cmd << endl;
    Util::remove_path(db_path);
    cout << cmd << endl;
    cmd = "mv " + _db_home + "/" + folders[inx] + " " + db_path;
    system(cmd.c_str());
    cout << cmd << endl;

    //load
    // gc.load(db_name);
    Database* current_database = new Database(db_name);
    current_database->load();

    string log_path = db_path + "/update_since_backup.log";
    ofp.open(log_path, ios::in);
    int flag = 0; int undo_point;
    string rec;
    int ret_val = 0;
    while(getline(ofp, rec))
    {
        //TODO: remove divide 1000
        int _timestamp = Util::get_timestamp(rec)/1000;
        if(_timestamp < timestamp) continue;
        if(!flag){
            undo_point = _timestamp;
            flag = 1;
        }
        if (rec[0] != 'I' && rec[0] != 'R') continue;
        rec = undo_sparql(rec);
        cout << "rollback sparql: "<< rec << endl;
        ResultSet rs;
        // gc.query(db_name, "json", rec, "POST");
        ret_val = current_database->query(rec, rs, nullptr, true, false, nullptr);
        cout << "rollback result: "<< ret_val << endl;
    }
    // save databse
    current_database->save();
    delete current_database;
    //undo updates according to log
    if(flag == 1)
        cout << "Database " << db_name << " has restored to time: "
             << Util::stamp2time(undo_point) << endl;
    else
        cout << "Database " << db_name << " has restored to time: " 
             << folders[inx] << endl;
    //gc.unload(db_name);
    //gc.load(db_name);
    return 0;
}
