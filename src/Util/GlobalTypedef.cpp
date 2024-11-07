#include "GlobalTypedef.h"

string GlobalTypedef::tmp_path = ".tmp/";
string GlobalTypedef::debug_path = ".tmp/";
string GlobalTypedef::profile = "./conf/conf.ini";
string GlobalTypedef::initfile = "./conf/init.lock";
string GlobalTypedef::system_db = "system";
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