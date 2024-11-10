#include "GlobalTypedef.h"

string GlobalTypedef::tmp_path = ".tmp/";
string GlobalTypedef::debug_path = ".tmp/";
string GlobalTypedef::pid_path = "./conf/topgraph.pid";
string GlobalTypedef::profile = "./conf/conf.ini";
string GlobalTypedef::initfile = "./conf/init.lock";
string GlobalTypedef::system_db = "system";
string GlobalTypedef::transaction_log_path = "./logs/transaction.json";
string GlobalTypedef::backup_log_path = "./logs/backup.json";
map<string, string> GlobalTypedef::global_config = {};

string GlobalTypedef::db_home()
{
    return global_config["db_home"];
}

string GlobalTypedef::db_suffix()
{
    return global_config["db_suffix"];
}

string GlobalTypedef::db_path(const string& db_name)
{
    return global_config["db_home"] + db_name + global_config["db_suffix"];
}

string GlobalTypedef::backup_path()
{
    return global_config["backup_path"];
}

string GlobalTypedef::upload_path()
{
    return global_config["upload_path"];
}
string GlobalTypedef::root_uname()
{
    return global_config["root_username"];
}
string GlobalTypedef::sys_uname()
{
    return global_config["system_username"];
}
int32_t GlobalTypedef::backup_interval()
{
    string backup_time = global_config["backup_time"];
    return atoi(backup_time.c_str());
}