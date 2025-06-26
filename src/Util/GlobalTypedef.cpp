#include "GlobalTypedef.h"
log4cplus::Logger GlobalTypedef::_logger = log4cplus::Logger::getRoot();
std::string GlobalTypedef::product_name = "gStore";
std::string GlobalTypedef::product_version = "1.4";
std::string GlobalTypedef::product_website = "https://www.gstore.cn";
std::string GlobalTypedef::tmp_path = ".tmp/";
std::string GlobalTypedef::debug_path = ".tmp/";
std::string GlobalTypedef::pid_path = "conf/gstore.pid";
std::string GlobalTypedef::profile = "conf/conf.ini";
std::string GlobalTypedef::initfile = "conf/init.lock";
std::string GlobalTypedef::system_db = "system";
std::string GlobalTypedef::transaction_log_path = "logs/transaction.json";
std::string GlobalTypedef::backup_log_path = "logs/backup.json";
std::string GlobalTypedef::export_path = "export/";
std::map<std::string, std::string> GlobalTypedef::global_config = {};

std::string GlobalTypedef::db_home()
{
    return global_config["db_home"];
}

std::string GlobalTypedef::db_suffix()
{
    return global_config["db_suffix"];
}

std::string GlobalTypedef::db_path(const std::string& db_name)
{
    return global_config["db_home"] + db_name + global_config["db_suffix"];
}

std::string GlobalTypedef::backup_path()
{
    return global_config["backup_path"];
}

std::string GlobalTypedef::upload_path()
{
    return global_config["upload_path"];
}

std::string GlobalTypedef::root_uname()
{
    return global_config["root_username"];
}

std::string GlobalTypedef::sys_uname()
{
    return global_config["system_username"];
}

int32_t GlobalTypedef::backup_max()
{
    std::string max_backups = global_config["max_backups"];
    return atoi(max_backups.c_str());
}

int32_t GlobalTypedef::thread_num()
{
    std::string thread_num = global_config["thread_num"];
    return atoi(thread_num.c_str());
}

int32_t GlobalTypedef::querylog_days()
{
    std::string log_num = global_config["querylog_days"];
    return atoi(log_num.c_str());
}

int32_t GlobalTypedef::accesslog_days()
{
    std::string log_num = global_config["accesslog_days"];
    return atoi(log_num.c_str());
}

bool GlobalTypedef::build_schema()
{
    std::string build_schema = global_config["build_schema"];
    return build_schema == "on";
}

void GlobalTypedef::upload_allow_extensions(std::set<std::string>& allow_extensions)
{
    std::string allow_ext = global_config["upload_allow_extensions"];
    std::stringstream ss(allow_ext);
    std::string item;
    while (std::getline(ss, item, '|')) {
        allow_extensions.insert(item);
    }
}

void GlobalTypedef::upload_allow_compress_packages(std::set<std::string>& allow_compress_packages)
{
    std::string allow_ext = global_config["upload_allow_compress_packages"];
    std::stringstream ss(allow_ext);
    std::string item;
    while (std::getline(ss, item, '|')) {
        allow_compress_packages.insert(item);
    }
}

bool GlobalTypedef::isEnabledFor(log4cplus::LogLevel ll)
{
    return _logger.isEnabledFor(ll);
}