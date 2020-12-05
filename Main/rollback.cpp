#include "../Database/Database.h"
#include "../Util/Util.h"
#include "../api/http/cpp/client.h"

using namespace std;
using namespace rapidjson;

const string BACKUP_PATH = "./backups";
int get_all_folders(string path, vector<string> &folders)
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
        folders.push_back(folder);
    }
    closedir(dp);

    return 0;
}

long int get_timestamp(string& line)
{
    int n = line.length()-2;
    long int timestamp = 0;
    int i = 0;
    while(isdigit(line[n]))
    {
        timestamp = pow(10, i) * (line[n] - '0') + timestamp;
        i++;
        n--;
    }
    line = line.substr(0, line.length() - i - 2);
    return timestamp;
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

string stamp2time(int timestamp)
{
    time_t tick = (time_t)timestamp;
    struct tm tm;
    char s[100];
    tm = *localtime(&tick);
    strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);

    return s;
}

int
main(int argc, char * argv[])
{
//#ifdef DEBUG
	Util util;
//#endif= 
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

    if(argc < 3){
        cout << "usage: rollback db_name Restore_point(day)" << endl;
        cout << "Restore_time example 2019-10-25(day) 12:00:00(time)" << endl;
        return 0;
    }

    cout << "argc: " << argc << endl;
    cout << "DB_store: " << argv[1] << endl;
    cout << "Restore Point(day): " << argv[2] << endl;
    cout << "Restore Point(time): " << argv[3] << endl;

    string db_name = argv[1];
    string restore_time = argv[2];
    restore_time += ' ';
    restore_time += argv[3];

    vector<string> folders;
    get_all_folders(BACKUP_PATH, folders);
    if(folders.size() == 0){
        cout << "Backups Folder Empty,Please check ./backups" << endl;
        return 0;
    }
    cout << restore_time << endl;
    int timestamp = Util::time_to_stamp(restore_time);
    cout << timestamp << endl;
    if(timestamp >  Util::get_cur_time() / 1000){
        cout << "Restore Time Error, Rollback Failed." << endl;
        return 0;
    }
    string backup_name = db_name + ".db" + "_" + get_postfix(restore_time);
    cout << backup_name << endl;
    sort(folders.begin(), folders.end());
    int inx = lower_bound(folders.begin(), folders.end(), backup_name) - folders.begin();
    if(inx >= folders.size()){
        cout << "No Backups for Database " + db_name << "!" << endl;
        return 0;
    }
    
    //unload
    string res = gc.unload(db_name);
    cout << res << endl;
    Document document;
    document.Parse(res.c_str());
    if(document["StatusCode"].GetInt() != 0 && document["StatusCode"].GetInt() != 304){\
        cout << "Rollback Failed: ";
        cout << document["StatusMsg"].GetString() << endl;
        return 0;
    }
    
    //cover the old
    string cmd;
    cmd = "rm -rf " + db_name + ".db";
    system(cmd.c_str());
    cmd = "cp -r " + BACKUP_PATH + '/' + folders[inx] + " ./";
    system(cmd.c_str());
    cmd = "mv " + folders[inx] + ' ' + db_name + ".db";
    system(cmd.c_str());
    //load

    gc.load(db_name);
    string log_path = db_name + ".db/" + "update_since_backup.log";
    cout << log_path << endl;
    ofp.open(log_path, ios::in);
    int flag = 0; int undo_point;
    string rec;
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
        cout << rec << endl;
        ResultSet rs;
        gc.query(db_name, "json", rec);
    }
    cout << flag << endl;
    //undo updates according to log
    if(flag == 1)
    cout << "Database " << db_name << " has restored to time: " 
    << stamp2time(undo_point) << endl;
    else
    cout << "Database " << db_name << " has restored to time: " 
    << folders[inx] << endl;
    gc.unload(db_name);
    //gc.load(db_name);
    return 0;
}
