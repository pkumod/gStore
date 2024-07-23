#include "../Database/Database.h"
#include "../Util/Util.h"
#include "../Api/HttpUtil.h"
#include <regex>

using namespace std;
using namespace rapidjson;

const string USERNAME = "root";

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

int gc_check(HttpUtil &gc, string _type, string _port, string &res)
{
    string strUrl = gc_getUrl(_type, _port);
    std::string strPost;
	strPost = "{\"operation\": \"check\"}";
    int ret = gc.Post(strUrl, strPost, res);
    // cout << "url: " << strUrl << ", ret: " << ret << ", res: " << res << endl;
    return ret;
}

int
main(int argc, char * argv[])
{
	Util util;
    fstream ofp;
    string _db_home = util.getConfigureValue("db_home");
	string _db_suffix = util.getConfigureValue("db_suffix");
    string _default_backup_path = util.getConfigureValue("backup_path");
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

    SLOG_INFO("Database Name: " + db_name);
    SLOG_INFO("Restore Point(date): " + backup_date);
    SLOG_INFO("Restore Point(time): " + backup_time);
    
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
    util.dir_files(_default_backup_path, folder_name, folders);
    if(folders.size() == 0){
        cout << "Backups Folder Empty, Please check " + _default_backup_path << endl;
        return 0;
    }
    time_t timestamp = Util::time_to_stamp(restore_time);
    time_t cur_time = Util::get_cur_time() / 1000l;
    if(timestamp >  cur_time){
        cout << "Restore Time Error, Rollback Failed." << endl;
        return 0;
    }
    string backup_name = db_name + _db_suffix + "_" + get_postfix(restore_time);
    sort(folders.begin(), folders.end());
    size_t inx = lower_bound(folders.begin(), folders.end(), backup_name) - folders.begin();
    if(inx >= folders.size() || folders[inx].find(db_name + _db_suffix) == string::npos){
        cout << "No Backups for Database " + db_name << "!" << endl;
        return 0;
    }
    SLOG_INFO("Match folder is: " + folders[inx]);

    // check http server status
    string system_port_path = _db_home + "/system" + _db_suffix + "/port.txt";
    if (Util::file_exist(system_port_path))
    {
        string port;
        string type;
        string type_port;
        HttpUtil gc;
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
        string res = "";
        if (gc_check(gc, type, port, res) == 0)
        {
            cout << "http server is running!, please shutdown this server to avoid data loss." << endl;
            return 0;
        }
    }
        
    //cover the old
    string cmd;
    string db_path = _db_home + "/" + db_name + _db_suffix;
    cmd = "cp -r " + _default_backup_path + "/" + folders[inx] + " " + _db_home;
    system(cmd.c_str());
    SLOG_DEBUG(cmd);
    Util::remove_path(db_path);
    cmd = "mv " + _db_home + "/" + folders[inx] + " " + db_path;
    system(cmd.c_str());
    SLOG_DEBUG(cmd);

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
        SLOG_CORE("rollback sparql: " << rec);
        ResultSet rs;
        // gc.query(db_name, "json", rec, "POST");
        ret_val = current_database->query(rec, rs, nullptr, true, false, nullptr);
        SLOG_CORE("rollback result: " << ret_val);
    }
    // save databse
    current_database->save();
    delete current_database;
    //undo updates according to log
    if(flag == 1)
        SLOG_INFO("Database " + db_name + " has restored to time: " << Util::stamp2time(undo_point));
    else
        SLOG_INFO("Database " + db_name + " has restored to time: " << folders[inx]);
    return 0;
}
