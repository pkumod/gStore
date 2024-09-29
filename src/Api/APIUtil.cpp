/*
 * @Author: wangjian
 * @Date: 2021-12-20 16:38:46
 * @LastEditTime: 2023-02-15 10:12:09
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: api util
 * @FilePath: /gstore/GRPC/APIUtil.cpp
 */
#include "APIUtil.h"



APIUtil::APIUtil()
{
    pthread_rwlock_init(&users_map_lock, NULL);
    pthread_rwlock_init(&already_build_map_lock, NULL);
    pthread_rwlock_init(&txn_m_lock, NULL);
    pthread_rwlock_init(&ips_map_lock, NULL);
    pthread_rwlock_init(&system_db_lock, NULL);
    pthread_rwlock_init(&query_log_lock, NULL);
    pthread_rwlock_init(&access_log_lock, NULL);
    pthread_rwlock_init(&transactionlog_lock, NULL);
    ipWhiteList = std::unique_ptr<IPWhiteList>(new IPWhiteList());
    ipBlackList = std::unique_ptr<IPBlackList>(new IPBlackList());
    // util.configure();
}

APIUtil::~APIUtil()
{
    SLOG_CORE("call ~APIUtil()");
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, shared_ptr<DatabaseInfo>>::iterator iter;
    for (iter = already_build.begin(); iter != already_build.end(); iter++)
    {
        string database_name = iter->first;
        //abort all transaction
        remove_txn_manager(database_name, true);
        // warning: this is going to be blocked, if the time of the system changes
        // default timeout 60 seconds
        shared_ptr<DatabaseInfo> db_info_ptr = iter->second;
        if (!trywrlock_databaseinfo(db_info_ptr, 60))
        {
            SLOG_WARN(database_name + " unable to save due to loss of lock");
            continue;
        }
        db_info_ptr->getDatabase()->save();
        unlock_databaseinfo(db_info_ptr);
    }
    if (system_database != nullptr)
    {
        pthread_rwlock_wrlock(&system_db_lock);
        system_database->save();
        system_database.reset();
        pthread_rwlock_unlock(&system_db_lock);
    }
    // remove already build database info
    already_build.clear();
    pthread_rwlock_unlock(&already_build_map_lock);

    // remove txn_manager
    pthread_rwlock_wrlock(&txn_m_lock);
    txn_managers.clear();
    pthread_rwlock_unlock(&txn_m_lock);

    // remove ips
    pthread_rwlock_wrlock(&ips_map_lock);
    ips.clear();
    pthread_rwlock_unlock(&ips_map_lock);

    // remove users
    pthread_rwlock_wrlock(&users_map_lock);
    users.clear();
    pthread_rwlock_unlock(&users_map_lock);

    pthread_rwlock_destroy(&already_build_map_lock);
    pthread_rwlock_destroy(&users_map_lock);
    pthread_rwlock_destroy(&txn_m_lock);
    pthread_rwlock_destroy(&ips_map_lock);
    pthread_rwlock_destroy(&system_db_lock);
    pthread_rwlock_destroy(&query_log_lock);
    pthread_rwlock_destroy(&access_log_lock);
    pthread_rwlock_destroy(&transactionlog_lock);

    ipWhiteList = nullptr;

    ipBlackList = nullptr;
}

int APIUtil::initialize()
{
    try
    {
        SLOG_CORE("initialization start");
        init_params();
        
        // load system db
        std::string _sys_db_path = get_Db_path() + "/system" + get_Db_suffix();
        if(!util.file_exist(Util::initfile) || !util.dir_exist(_sys_db_path))
        {
            SLOG_INFO("System has not been initialized. Now initialize it");
            if (util.dir_exist(_sys_db_path))
            {
                util.remove_path(_sys_db_path);
            }
            system_database  = make_shared<Database>(Util::system_db);
            bool _sys_build_rt = system_database->BuildEmptyDB();
            if (_sys_build_rt)
            {
                ofstream f;
                f.open(_sys_db_path + "/success.txt");
                f.close();
                f.open(Util::initfile);
                f.close();
                system_database.reset();
                Util::init_backuplog();
                string version = util.getConfigureValue("version");
                string root_pwd = util.getConfigureValue("root_password");
                string update_sparql = "INSERT DATA {\
                    <system> <built_by> <root> . \
                    <CoreVersion> <value> \"" + version + "\". \
                    <root> <has_password> \"" + root_pwd + "\" .}";
                system_database = make_shared<Database>(Util::system_db);
                system_database->load();
                update_sys_db(update_sparql);
                refresh_sys_db();
            }
            else
            {
                SLOG_INFO("System initialization failed. Please manually initialize system");
                return -1;
            }
        }
        else
        {
            system_database = make_shared<Database>(Util::system_db);
            system_database->load();
        }

        // init already_build db
        ResultSet rs;
        std::string sparql = "select ?x ?y ?z where{ ?x <database_status> \"already_built\"; <built_by> ?y; <built_time> ?z.}";
        bool query_ret = query_sys_db(sparql, rs);
        if (query_ret)
        {
            if (rs.ansNum > 0)
            {
                if (!trywrlock_already_build_map())
                {
                    return -1;
                }
                for (unsigned int i = 0; i < rs.ansNum; i++)
                {
                    string db_name = util.clear_angle_brackets(rs.answer[i][0]);
                    std::string creator = util.clear_angle_brackets(rs.answer[i][1]);
                    std::string built_time = util.replace_all(rs.answer[i][2], "\"", "");
                    shared_ptr<DatabaseInfo> temp_db = make_shared<DatabaseInfo>(db_name, creator, built_time, DatabaseStatus::AREADY_BUILT);
                    already_build.insert(pair<std::string, shared_ptr<DatabaseInfo>>(db_name, temp_db));
                }
                unlock_already_build_map();
            }
            else
            {
                SLOG_INFO("No already built database.");
            }
        }
        else
        {
            SLOG_ERROR("Load aready built database failed. Please check system.db");
            return -1;
        }
        //userinfo
        sparql = "select ?x ?y where{?x <has_password> ?y.}";
        query_ret = query_sys_db(sparql, rs);
        if (query_ret)
        {
            ResultSet _user_rs;
            pthread_rwlock_wrlock(&users_map_lock);
            for (unsigned int i = 0; i < rs.ansNum; i++)
            {
                string username = util.clear_angle_brackets(rs.answer[i][0]);
                string password = util.replace_all(rs.answer[i][1], "\"", "");
                shared_ptr<struct DBUserInfo> user = make_shared<struct DBUserInfo>(username, password);
                
                //privilege add
                string sparql2 = "select ?x ?y where{<" + username + "> ?x ?y.}";
                //string strJson2 = QuerySys(sparql2);
                query_ret = query_sys_db(sparql2, _user_rs);
                
                if (query_ret && _user_rs.ansNum > 0)
                {
                    for(unsigned j = 0; j < _user_rs.ansNum; j++)
                    {
                        std::string type = util.clear_angle_brackets(_user_rs.answer[j][0]);
                        std::string _db_name = util.clear_angle_brackets(_user_rs.answer[j][1]);
                        size_t pos1 = type.find_first_of("_");
                        size_t pos2 = type.find_last_of("_");
                        std::string _type = type.substr(pos1 + 1, pos2 - pos1 - 1);
                        SLOG_CORE("type: " + _type + " db_name: " + _db_name);
                        update_privilege(user, _type, _db_name, 1);
                    }
                }
                rapidjson::Document doc;
                rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
                rapidjson::Value user_json = user->toJSON(allocator);
                rapidjson::StringBuffer resBuffer;
                rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
                user_json.Accept(resWriter);
                SLOG_CORE(username + ":" + resBuffer.GetString());
                users.insert(pair<std::string, shared_ptr<struct DBUserInfo>>(username, user));
            }
            pthread_rwlock_unlock(&users_map_lock);
        }
        else
        {
             SLOG_ERROR("Init database users failed.");
        }
        init_transactionlog();
        // create system password file
        fstream ofp;
        system_password = util.int2string(util.getRandNum());
        std::string pid_path = PID_PATH;
        ofp.open(pid_path.c_str(), ios::out);
        ofp << getpid();
        ofp << '\n';
        ofp << system_password;
        ofp << '\n';
        ofp.flush();
        ofp.close();
        // #if defined(DEBUG)
        SLOG_CORE("initialization end");
        // #endif
        return 1;
    }
    catch (const std::exception &e)
    {
        SLOG_ERROR("initialization fail: " + string(e.what()));
        return -1;
    }
}
void APIUtil::init_params()
{
    // init params
    thread_pool_num = get_configure_value("thread_num", thread_pool_num);
    system_username = get_configure_value("system_username", system_username);
    max_database_num = get_configure_value("max_database_num", max_database_num);
    max_user_num = get_configure_value("max_user_num", max_user_num);
    max_output_size = get_configure_value("max_output_size", max_output_size);
    query_log_mode = get_configure_value("querylog_mode", query_log_mode);
    query_log_path = get_configure_value("querylog_path", query_log_path);
    access_log_mode = get_configure_value("accesslog_mode", access_log_mode);
    access_log_path = get_configure_value("accesslog_path", access_log_path);
    query_result_path = get_configure_value("queryresult_path", query_result_path);

    //load ip-list
    ipWhiteFile = get_configure_value("ip_allow_path", ipWhiteFile);
    ipBlackFile = get_configure_value("ip_deny_path", ipBlackFile);
    if (ipWhiteFile.empty()) {
        whiteList = 0;
    } else {
        whiteList = 1;
    }
    if (ipBlackFile.empty())
    {
        blackList = 0;
    } else {
        blackList = 1;
    }
    if (whiteList) {
        SLOG_INFO("IP white List enabled.");
        ipWhiteList->Load(ipWhiteFile);
    } else if (blackList) {
        SLOG_INFO("IP black list enabled.");
        ipBlackList->Load(ipBlackFile);
    }

    // init upload conf
    upload_path = get_configure_value("upload_path", upload_path);
    upload_max_body_size = get_configure_value("upload_max_body_size", upload_max_body_size);
    string configure_extensions = get_configure_value("upload_allow_extensions",  "nt|ttl|n3|rdf|txt");
    Util::split(configure_extensions, "|", upload_allow_extensions);
    string configure_compress_packages = get_configure_value("upload_allow_compress_packages",  "zip");
    Util::split(configure_compress_packages, "|", upload_allow_compress_packages);
}

void APIUtil::refresh_conf()
{
    // reload config file;
    util.configure();
    // init params
    init_params();
}

bool APIUtil::trywrlock_already_build_map()
{
    int lock_code = pthread_rwlock_wrlock(&already_build_map_lock);
    if ( lock_code == 0)
    {
        SLOG_CORE("get already_build_map wrlock ok");
        return true;
    }
    else
    {
        SLOG_ERROR("get already_build_map wrlock error: " + to_string(lock_code));
        return false;
    }
}

bool APIUtil::unlock_already_build_map()
{
    int rwlock_code = pthread_rwlock_unlock(&already_build_map_lock);
    if ( rwlock_code == 0)
    {
        SLOG_CORE("already_build_map unlock ok");
        return true;
    }
    else
    {
        SLOG_ERROR("already_build_map unlock error: " + to_string(rwlock_code));
        return false;
    }
}

bool APIUtil::init_databaseinfo(const std::string& db_name, const std::string creator, const std::string build_time, const DatabaseStatus status)
{
    if (trywrlock_already_build_map())
    {
        string current_time = build_time; 
        if (current_time.empty())
        {
            current_time = util.get_date_time();
        }
        shared_ptr<DatabaseInfo> temp_db = make_shared<DatabaseInfo>(db_name, creator, build_time, status);
        string update = "INSERT DATA {\
            <" + db_name + "> <database_status> \"already_built\". \
            <" + db_name + "> <built_by> <" + creator + "> . \
            <" + db_name + "> <built_time> \"" + build_time + "\". \
        }";
        bool update_result = update_sys_db(update);
        if (update_result) {
            refresh_sys_db();
            already_build.insert(pair<std::string, shared_ptr<DatabaseInfo>>(db_name, temp_db));
            SLOG_CORE("add database into already build map done.");
            unlock_already_build_map();
            return true;
        }
        else
        {
            unlock_already_build_map();
            return false;
        }
    }
    else
    {
        return false;
    }
}

bool APIUtil::update_database_status(const std::string& db_name, const DatabaseStatus status)
{
    shared_ptr<DatabaseInfo> db_info;
    get_databaseinfo(db_name, db_info);
    if (db_info == nullptr)
    {
        return false;
    }
    if (trywrlock_already_build_map())
    {
        db_info->setStatus(status);
        unlock_already_build_map();
        return true;
    }
    else
    {
        return false;
    }
}

bool APIUtil::remove_databaseinfo(const std::string& db_name, std::string msg)
{
    shared_ptr<DatabaseInfo> db_info;
    if (get_databaseinfo(db_name, db_info) == false) {
        msg = "can't find [" + db_name + "] database info from already builts list";
        return false;
    }
    if (trywrlock_databaseinfo(db_info, 600) == false) {
        msg = "Unable to drop due to loss of lock";
        return false;
    } 
    // remove databse info from system.db
    std::vector<std::string> privileges = {"query", "update", "load", "unload", "restore", "backup", "export"};
    bool update_result = true;
    for (auto& iter : privileges) {
        string update = "DELETE WHERE {<"+ db_name + "> <has_"+ iter + "_priv> ?o.}";
        update_result = update_result && update_sys_db(update);
    }
    if (!update_result) {
        unlock_databaseinfo(db_info);
        msg = "Remove db info from system failed.";
        return false;
    }
    // system checkpoint
    refresh_sys_db();
    // clear all privileges 
    for (auto& iter : users) {
        update_privilege(iter.second, "all", db_name, -1);
    }
    unlock_databaseinfo(db_info);
    // remove from already build map
    if (trywrlock_already_build_map()) {
        already_build.erase(db_name);
        unlock_already_build_map();
    } else {
        SLOG_WARN("Remove db info from already build list failed.");
    }
    // delete backup log
    util.delete_backuplog(db_name);
    return true;
}

bool APIUtil::backup_databaseinfo(const std::string& db_name, const bool& compress, std::string& backup_path, std::string& msg)
{
    shared_ptr<DatabaseInfo> db_info;
    get_databaseinfo(db_name, db_info);
    if (db_info == nullptr) {
        msg = "can't find [" + db_name + "] database info from already builts list";
        return false;
    }
    if( trywrlock_databaseinfo(db_info) ==  false) {
        msg = "Unable to drop due to loss of lock";
        return false;
    }
    // Delete the oldest backup file
    std::string db_name_suffix = db_name + get_Db_suffix();
    vector<std::string> backup_files;
    Util::dir_files(backup_path, db_name_suffix, backup_files);
    int16_t max_backups = get_configure_value("max_backups", 3);
    int16_t cur_backups = backup_files.size();
    if (cur_backups > max_backups)
    {
        // sort asc
        std::sort(backup_files.begin(), backup_files.end(), [](const std::string& a, const std::string& b) {
            return a < b;
        });
        for (auto file_name : backup_files) {
            if (cur_backups <= max_backups) {
                break;
            }
            std::string remove_file_path = backup_path + "/" + file_name;
            Util::remove_path(remove_file_path);
            cur_backups--;
        }
    }
    // begin backup
    bool backup_rt = db_info->getDatabase()->backup(backup_path);
    unlock_databaseinfo(db_info);
    if (backup_rt && compress) {
        string timestamp = Util::get_timestamp();
        std::string zip_file_path = backup_path + "_" + timestamp + ".zip";
        CompressUtil::CompressZip compress_util;
        backup_rt = compress_util.compressDirExportZip(backup_path, zip_file_path);
        if (backup_rt) {
            Util::remove_path(backup_path);
        }
        backup_path = zip_file_path;
    }
    return backup_rt;
}

bool APIUtil::restore_databaseinfo(const std::string& username, const std::string& db_name, std::string& backup_path, std::string& msg)
{
    shared_ptr<DatabaseInfo> db_info;
    get_databaseinfo(db_name, db_info);
    if (db_info != nullptr) {
        if (db_info->getStatus() != DatabaseStatus::AREADY_BUILT) {
            msg = "Database alreay load, need unload it first.";
            return false;
        }
    } else {
        // restore for null
        std::string built_time = Util::get_backup_time(backup_path, db_name);
        db_info = std::make_shared<DatabaseInfo>(db_name, username, built_time, DatabaseStatus::BUILDING);
    }
    if (trywrlock_databaseinfo(db_info) ==  false) {
        msg = "Unable to restore due to loss of lock";
        return false;
    }
    if (!Util::file_exist(backup_path) && !Util::dir_exist(backup_path)) {
        msg = "backup path is not exist";
        return false;
    }
    std::string db_suffix = get_Db_suffix();
    std::string db_home_path = get_Db_path() + db_name + db_suffix;
    bool restore_bool = false;
    // mv db_home to db_home.bak
    if (Util::dir_exist(db_home_path)) {
        mv_or_cp(db_home_path, db_home_path + ".bak", true);
    }
    // is zip file
    if (Util::is_file(backup_path)) {
        // unzip
        std::string unzip_path = Util::get_parent_path(backup_path);
        CompressUtil::UnCompressZip unzip(backup_path, unzip_path);
        if (unzip.unCompress() != CompressUtil::UnZipOK) {
            msg = "backup compress fail";
            return false;
        }
        // mv unzip file to db_home
        Util::string_suffix(unzip_path, '/');
        restore_bool = mv_or_cp(unzip_path + "*", db_home_path, true);
    } else {
        // cp backup path to db_home
        Util::string_suffix(backup_path, '/');
        restore_bool = mv_or_cp(backup_path + "*", db_home_path, false);
    }
    if (restore_bool) {
        if (db_info->getStatus() == DatabaseStatus::BUILDING) {
            db_info->setStatus(DatabaseStatus::AREADY_BUILT);
            init_privilege(username, db_name);
            Util::add_backuplog(db_name);
        }
        // remove old db_home
        Util::remove_path(db_home_path + ".bak");
    } else {
        msg = "restore fail";
        if (Util::dir_exist(db_home_path + ".bak")) {
            mv_or_cp(db_home_path + ".bak", db_home_path, true);
        }
    }
    return restore_bool;
}

bool APIUtil::rename_databaseinfo(const std::string& db_name, const std::string& new_db_name, std::string& msg)
{
    if (check_db_built(db_name) == false)
    {
        msg =  "Database not built yet.";
        return false;
    }
    if (check_db_loaded(db_name))
    {
        msg = "Database is loaded, need unload first.";
        return false;
    }
    // check new_name available
    if (check_db_built(new_db_name))
    {
        msg = "Database name " + new_db_name + " already exists.";
        return false;
    }
    std::string db_home = get_Db_path();
    std::string db_suffix = get_Db_suffix();
    std::string db_new_path = db_home + new_db_name + db_suffix;
    // check new_db_path
    if (Util::dir_exist(db_new_path))
    {
        msg =  "Database path " + db_new_path + " already exists.";
        return false;
    }
    shared_ptr<DatabaseInfo> db_info;
    get_databaseinfo(db_name, db_info);
    if (trywrlock_databaseinfo(db_info) == false)
    {
        msg = "Unable to rename due to loss of lock.";
        return false;
    }
    string db_path = db_home + db_name + db_suffix;
    if (mv_or_cp(db_path, db_new_path, true))
    {
        // add new db info
        init_databaseinfo(new_db_name, db_info->getCreator(), db_info->getTime(), DatabaseStatus::AREADY_BUILT);
        // copy privilege
        copy_privilege(db_name, new_db_name);
        // add backup log
        Util::add_backuplog(new_db_name);

        // unlock old db
        unlock_databaseinfo(db_info);
        // remove old db info
        std::string remove_msg;
        if(remove_databaseinfo(db_name, remove_msg) == false);
        {
            SLOG_WARN("remove old db failed: " + remove_msg);
        }
        return true;
    }
    else
    {
        msg = "Rename fail.";
        return false;
    }
}

bool APIUtil::get_databaseinfo(const std::string& db_name, shared_ptr<DatabaseInfo> &dbInfo)
{
    bool result = false;
    int rwlock_code = pthread_rwlock_rdlock(&already_build_map_lock);
    if (rwlock_code != 0) 
    {
        SLOG_ERROR("already_build_map read lock error: " + to_string(rwlock_code));
        dbInfo = nullptr;
        return false;
    }
    std::map<std::string, shared_ptr<DatabaseInfo>>::iterator iter = already_build.find(db_name);
    if (iter != already_build.end())
    {
        dbInfo = iter->second;
        result = true;
    }
    else
    {
        SLOG_ERROR("can't find [" + db_name + "] database info from already_build_map");
        dbInfo = nullptr;
    }
    unlock_already_build_map();
    return result;
}

void APIUtil::get_databaseinfos(const std::string& username, vector<shared_ptr<DatabaseInfo>> &array)
{
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, shared_ptr<DatabaseInfo>>::iterator iter;
    for (iter = already_build.begin(); iter != already_build.end(); iter++) {
        std::string db_name = iter->first;
        shared_ptr<DatabaseInfo> db_info = iter->second;
        if (username != ROOT_USERNAME && check_privilege(username, "query", db_name) == false) {
            continue;
        }
        array.push_back(db_info);
    }
    pthread_rwlock_unlock(&already_build_map_lock);
}

bool APIUtil::erase_databaseinfo(const std::string& username)
{
    if (trywrlock_already_build_map()) {
        already_build.erase(username);
        unlock_already_build_map();
        return true;
    } else {
        return false;
    }
}

bool APIUtil::wrlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo)
{
    bool result = false;
    if (dbinfo == NULL || dbinfo == nullptr)
        return result;
    if (pthread_rwlock_wrlock(&(dbinfo->db_lock)) == 0)
    {
        SLOG_CORE("gets databaseinfo[" + dbinfo->getName() + "] write lock ok.");
        result = true;
    }
    else
    {
        SLOG_CORE("gets databaseinfo[" + dbinfo->getName() + "] write lock fail.");
    }
    return result;
}

bool APIUtil::trywrlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo)
{
    return trywrlock_databaseinfo(dbinfo, 30);
}

bool APIUtil::trywrlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo, const time_t& timeout_s)
{
    bool result = false;
    if (dbinfo == NULL || dbinfo == nullptr)
        return result;
    if (pthread_rwlock_trywrlock(&(dbinfo->db_lock)) == 0)
    {
        SLOG_CORE("try gets databaseinfo[" + dbinfo->getName() + "] write lock ok.");
        result = true;
    }
    else
    {
        struct timeval now;
        struct timespec str_timeout = {0};
        gettimeofday(&now, NULL);
        str_timeout.tv_sec = now.tv_sec + timeout_s;
        str_timeout.tv_nsec = now.tv_usec * 1000;
        if (pthread_rwlock_timedwrlock(&(dbinfo->db_lock), &str_timeout) == 0)
        {
            SLOG_CORE("gets databaseinfo[" + dbinfo->getName() + "] write lock ok.");
            result = true;
        }
        else
        {
            SLOG_CORE("gets databaseinfo[" + dbinfo->getName() + "] write lock timeout.");
        }
    }
    return result;
}

bool APIUtil::rdlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo)
{
    int rwlock_code = pthread_rwlock_rdlock(&(dbinfo->db_lock));
    if (rwlock_code == 0)
    {
        // #if defined(DEBUG)
        SLOG_CORE("read lock database[" + dbinfo->getName() + "] ok");
        // #endif
        return true;
    }
    else
    {
        SLOG_ERROR("read lock database[" + dbinfo->getName() + "] error: " + to_string(rwlock_code));
        return false;
    }
}

bool APIUtil::unlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo)
{
    
    if (dbinfo == nullptr || dbinfo == NULL)
    {
        SLOG_ERROR("database info ptr is null");
        return false;
    }
    int rwlock_code = pthread_rwlock_unlock(&(dbinfo->db_lock));
    if (rwlock_code == 0)
    {
        SLOG_CORE("database [" + dbinfo->getName() + "] unlock ok");
        return true;
    }
    else
    {
        SLOG_ERROR("database [" + dbinfo->getName() + "] unlock error: " + to_string(rwlock_code));
        return false;
    }
}

bool APIUtil::get_txn_manager(const std::string& db_name, shared_ptr<Txn_manager> &txn_manager)
{
    pthread_rwlock_rdlock(&txn_m_lock);
    bool rt = false;
    auto it = txn_managers.find(db_name);
	if (it != txn_managers.end()){
	    txn_manager = it->second;
        rt = true;
	} else {
        txn_manager = nullptr;
    }
	pthread_rwlock_unlock(&txn_m_lock);
    return rt;
}

bool APIUtil::insert_txn_manager(const std::string& db_name, shared_ptr<DatabaseInfo> &dbinfo)
{
    if(pthread_rwlock_trywrlock(&txn_m_lock) != 0)
    {
        struct timeval now;
        struct timespec str_timeout = {0};
        gettimeofday(&now, NULL);
        str_timeout.tv_sec = now.tv_sec + 60;
        str_timeout.tv_nsec = now.tv_usec * 1000;
        if (pthread_rwlock_timedwrlock(&txn_m_lock, &str_timeout) == 0)
        {
            SLOG_ERROR("add txn manager of " + db_name + " error: get wrlock timeout");
            return false;
        }
    }
    shared_ptr<Txn_manager> txn_m = make_shared<Txn_manager>(dbinfo->getDatabase().get(), db_name);
    txn_managers.insert(pair<string, shared_ptr<Txn_manager>>(db_name, txn_m));
    SLOG_CORE("add txn manager for " + db_name + " ok");
    pthread_rwlock_unlock(&txn_m_lock);
    return true;
}

bool APIUtil::remove_txn_manager(const std::string& db_name, bool checkpoint)
{
    if(pthread_rwlock_trywrlock(&txn_m_lock) != 0)
    {
        struct timeval now;
        struct timespec str_timeout = {0};
        gettimeofday(&now, NULL);
        str_timeout.tv_sec = now.tv_sec + 60;
        str_timeout.tv_nsec = now.tv_usec * 1000;
        if (pthread_rwlock_timedwrlock(&txn_m_lock, &str_timeout) == 0)
        {
            SLOG_ERROR("remove txn manager of " + db_name + " error: get wrlock timeout");
            return false;
        }
    }
    auto it = txn_managers.find(db_name);
	if (it == txn_managers.end())
	{
        SLOG_ERROR("can not get " + db_name + " txn manager.");
		pthread_rwlock_unlock(&txn_m_lock);
		return false;
	}
    if (checkpoint)
    {
        auto txn_m = it->second;
	    txn_m->abort_all_running();
	    txn_m->Checkpoint();
        SLOG_CORE("checkpoint " + db_name + " ok.");
    }
	txn_managers.erase(db_name);
    pthread_rwlock_unlock(&txn_m_lock);
    return true;
}

bool APIUtil::get_txn_begin_time(shared_ptr<Txn_manager>& txn_m, txn_id_t& tid, std::string& begin_time)
{
    begin_time = to_string(txn_m->Get_Transaction(tid)->GetStartTime());
    return true;
}

bool APIUtil::begin_process(const std::string& db_name, int level , std::string username, txn_id_t& tid)
{
    shared_ptr<Txn_manager> txn_m;
    get_txn_manager(db_name, txn_m);
    if (txn_m == nullptr) 
    {
        return false;
    }
	tid = txn_m->Begin(static_cast<IsolationLevelType>(level));
	string begin_time = to_string(txn_m->Get_Transaction(tid)->GetStartTime());
	string time_tid = begin_time + "_" + to_string(tid);
	add_transactionlog(db_name, username, time_tid, begin_time, "RUNNING", "INF");
    if (tid == INVALID_ID) {
		return false;
	} else {
        return true;
    }
}

bool APIUtil::commit_process(shared_ptr<Txn_manager>& txn_m, txn_id_t& tid, std::string& msg)
{
    int code = txn_m->Commit(tid);
    if (code == -1)
    {
		msg = "Wrong transaction id.";
		return false;
    }
    else if (code == 1)
    {
        msg = "Transaction not in running state!";
        return false;
    }
    else
    {
        string begin_time = to_string(txn_m->Get_Transaction(tid)->GetStartTime());
        string time_tid = begin_time + "_" + to_string(tid);
        update_transactionlog(time_tid, "COMMITED", to_string(txn_m->Get_Transaction(tid)->GetEndTime()));
        return true;
    }
}

bool APIUtil::rollback_process(shared_ptr<Txn_manager>& txn_m, txn_id_t& tid, std::string& msg)
{
    int code = txn_m->Rollback(tid);
    if (code == -1)
    {
		msg = "Wrong transaction id.";
		return false;
    }
    else if (code == 1)
    {
        msg = "Transaction not in running state!";
        return false;
    }
    else 
    {
        string begin_time = to_string(txn_m->Get_Transaction(tid)->GetStartTime());
        string time_tid = begin_time + "_" + to_string(tid);
        update_transactionlog(time_tid, "ROLLBACK", to_string(txn_m->Get_Transaction(tid)->GetEndTime()));
        return true;
    }
}

bool APIUtil::aborted_process(shared_ptr<Txn_manager>& txn_m, txn_id_t& tid, std::string& msg)
{
    string begin_time = to_string(txn_m->Get_Transaction(tid)->GetStartTime());
    string time_tid = begin_time + "_" + to_string(tid);
    int ret = update_transactionlog(time_tid, "ABORTED", to_string(txn_m->Get_Transaction(tid)->GetEndTime()));
    return ret == 0;
}

bool APIUtil::check_txn_id(const string& tid_s, txn_id_t& tid)
{
    if(util.is_number(tid_s)) {
		tid = strtoull(tid_s.c_str(), NULL, 0);
	} else if (tid_s.find("_") != string::npos) {
        // case for workbench call commit and rollback: "beginTime_tid"
        int pos = tid_s.find("_") + 1;
        string tid_s_new = tid_s.substr(pos, tid_s.size()-pos);
        if (util.is_number(tid_s_new)) {
            tid = strtoull(tid_s_new.c_str(), NULL, 0);
        } else {
            tid = INVALID_ID;
        }
    } else {
        tid = INVALID_ID;
    }
    return tid == INVALID_ID || tid == 0;
}

bool APIUtil::check_db_loaded(const std::string &db_name)
{
    shared_ptr<DatabaseInfo> dbinfo;
    bool rt = get_databaseinfo(db_name, dbinfo);
    if (rt && dbinfo != nullptr) {
        return dbinfo->getStatus() == DatabaseStatus::LOADED;
    } else {
        return false;
    }
}

bool APIUtil::check_db_built(const std::string &db_name)
{
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, shared_ptr<DatabaseInfo>>::iterator iter = already_build.find(db_name);
    pthread_rwlock_unlock(&already_build_map_lock);
    if (iter == already_build.end())
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool APIUtil::check_db_count()
{
    return already_build.size() < max_database_num;
}

bool APIUtil::check_indentity(const std::string &username, const std::string &password, const std::string &encryption, std::string& msg)
{
    pthread_rwlock_rdlock(&users_map_lock);
    auto it = users.find(username);
    if (it == users.end())
    {
        SLOG_CORE("username not exist.");
        msg = "Username or password is wrong, username:" +username + ", password:" + password;
        return false;
    }
    else if (encryption == "1")
    {
        if (util.md5(it->second->getPassword()) != password)
        {
            SLOG_CORE("encryption password wrong.");
            msg = "Username or password is wrong.username:" + username + ", password:" + password;
            return false;
        }
    }
    else if (it->second->getPassword() != password)
    {
        SLOG_CORE("password wrong.");
        msg = "Username or password is wrong.username:" + username + ", password:" + password;
        return false;
    }
    pthread_rwlock_unlock(&users_map_lock);
    msg = "";
    return true;
}

bool APIUtil::check_server_indentity(const std::string& password, std::string& msg)
{
	if(password != system_password)
	{
		msg = "the password is wrong. please check the " + system_password_path;
        return false;
	}
    msg = "";
    return true;
}

bool APIUtil::check_param_value(const string& paramname, const string& value, std::string& msg)
{
	if (value.empty()) 
	{
		msg = "the value of " + paramname + " can not be empty!";
		return false;
	}
	if (paramname == "db_name")
	{
		if (value == Util::system_db)
		{
			msg = "you can not operate the system database";
			return false;
		}
		string db_suffix = get_Db_suffix();
        size_t len_suffix = db_suffix.length();
        string _tmp = value.substr(value.length() - len_suffix, len_suffix);
		if (value.length() > len_suffix && _tmp == db_suffix)
		{
			msg = "Your db name to be built should not end with \"" + db_suffix + "\".";
			return false;
		}
	}
    msg = "";
	return true;
}

bool APIUtil::check_user_exist(const std::string& username)
{
    pthread_rwlock_rdlock(&users_map_lock);
    auto it = users.find(username);
    pthread_rwlock_unlock(&users_map_lock);
	if(it != users.end()) {
        return true;
    } else {
        return false;
    }
}

bool APIUtil::check_user_count()
{
    return users.size() < max_user_num;
}

bool APIUtil::add_privilege(const std::string& username, const vector<string>& types, const std::string& db_name)
{
    if(username == ROOT_USERNAME) {
		return true;
	}
    pthread_rwlock_rdlock(&users_map_lock);
    std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
	if(it != users.end() && db_name != Util::system_db)
	{
        string update = "INSERT DATA { ";
        for (unsigned i = 0; i < types.size(); i++)
        {
            string type = types[i];
            if(type == "query")
            {
                update = update + "<" + username + "> <has_query_priv> <" + db_name + ">. ";
            }
            else if(type == "update")
            {
                update = update + "<" + username + "> <has_update_priv> <" + db_name + ">. ";
            }
            else if(type == "load")
            {
                update = update + "<" + username + "> <has_load_priv> <" + db_name + ">. ";
            }
            else if(type == "unload")
            {
                update = update + "<" + username + "> <has_unload_priv> <" + db_name + ">. ";
            }
            else if(type == "restore")
            {
                update = update + "<" + username + "> <has_restore_priv> <" + db_name + ">. ";
            }
            else if(type == "backup")
            {
                update = update + "<" + username + "> <has_backup_priv> <" + db_name + ">. ";
            }
            else if(type == "export")
            {
                update = update + "<" + username + "> <has_export_priv> <" + db_name + ">. ";
            }
        }
        update = update + "}";
        bool add_result = update_sys_db(update);
        if (add_result)
        {
            refresh_sys_db();
            for (unsigned i = 0; i < types.size(); i++)
            {
                // add privilege
                update_privilege(it->second, types[i], db_name, 1);
            }
        }
        pthread_rwlock_unlock(&users_map_lock);
		return add_result;
	}
	else
	{
		pthread_rwlock_unlock(&users_map_lock);
		return false;
	}
}

bool APIUtil::del_privilege(const std::string& username, const vector<string>& types, const std::string& db_name)
{
    if (username == ROOT_USERNAME) {
		return false;
	}
    pthread_rwlock_rdlock(&users_map_lock);
	std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
	if(it != users.end() && db_name != Util::system_db)
	{
        string update = "";
        bool del_result = false;
        bool refresh_flag = false;
        for (unsigned i = 0; i < types.size(); i++)
        {
            string type = types[i];
            if(type == "query" && it->second->query_priv.find(db_name) != it->second->query_priv.end())
            {
                update = "DELETE DATA { <" + username + "> <has_query_priv> <" + db_name + ">. }";
            }
            else if(type == "update" && it->second->update_priv.find(db_name) != it->second->update_priv.end())
            {
                update = "DELETE DATA { <" + username + "> <has_update_priv> <" + db_name + ">. }";
            }
            else if(type == "load" && it->second->load_priv.find(db_name) != it->second->load_priv.end())
            {
                update = "DELETE DATA { <" + username + "> <has_load_priv> <" + db_name + ">. }";
            }
            else if(type == "unload" && it->second->unload_priv.find(db_name) != it->second->unload_priv.end())
            {
                update = "DELETE DATA { <" + username + "> <has_unload_priv> <" + db_name + ">. }";
            }
            else if(type == "backup" && it->second->backup_priv.find(db_name) != it->second->backup_priv.end())
            {
                update = "DELETE DATA { <" + username + "> <has_backup_priv> <" + db_name + ">. }";
            }
            else if(type == "restore" && it->second->restore_priv.find(db_name) != it->second->restore_priv.end())
            {
                update = "DELETE DATA { <" + username + "> <has_restore_priv> <" + db_name + ">. }";
            }
            else if(type == "export" && it->second->export_priv.find(db_name) != it->second->export_priv.end())
            {
                update = "DELETE DATA { <" + username + "> <has_export_priv> <" + db_name + ">. }";
            } 
            else 
            {
                continue;
            }
            // delete privilege
            del_result = update_sys_db(update);
            refresh_flag = refresh_flag || del_result;
            // remove from privilege set
            if (del_result) {
                update_privilege(it->second, type, db_name, 0);
            }
        }
        if (refresh_flag)
            refresh_sys_db();
        pthread_rwlock_unlock(&users_map_lock);
        return del_result;
	} else {
        pthread_rwlock_unlock(&users_map_lock);
		return false;
    }
}

bool APIUtil::check_privilege(const std::string& username, const std::string& type, const std::string& db_name)
{
	if (db_name == Util::system_db) {
		return false;
    }

	if (username == ROOT_USERNAME) {
		return true;
    }

    if (type == "login" || type == "testConnect" || type == "getCoreVersion" 
        || type == "funquery" || type == "funcudb" || type == "funreview"
        || type == "check" || type == "show" || type == "userpassword" || type == "upload" || type == "download") {
        return true;
    }
    
	pthread_rwlock_rdlock(&users_map_lock);
	std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
	int check_result = 0;
	if(type == "query" || type == "monitor")
	{
		pthread_rwlock_rdlock(&(it->second->query_priv_set_lock));
		if(it->second->query_priv.find(db_name) != it->second->query_priv.end())
		{
			check_result = 1;
		}
		pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
	}
	else if(type == "update" || type == "batchInsert" || type == "batchRemove" 
        || type == "begin" || type == "tquery" || type == "commit" || type == "rollback" || type == "checkpoint")
	{
		pthread_rwlock_rdlock(&(it->second->update_priv_set_lock));
		if(it->second->update_priv.find(db_name) != it->second->update_priv.end())
		{
			check_result = 1;
		}
		pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
	}
	else if(type == "load")
	{
		pthread_rwlock_rdlock(&(it->second->load_priv_set_lock));
		if(it->second->load_priv.find(db_name) != it->second->load_priv.end())
		{
			check_result = 1;
		}
		pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
	}
	else if(type == "unload")
	{
		pthread_rwlock_rdlock(&(it->second->unload_priv_set_lock));
		if(it->second->unload_priv.find(db_name) != it->second->unload_priv.end())
		{
			check_result = 1;
		}
		pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
	}
	else if(type == "restore")
	{
		pthread_rwlock_rdlock(&(it->second->restore_priv_set_lock));
		if(it->second->restore_priv.find(db_name) != it->second->restore_priv.end())
		{
			check_result = 1;
		}
		pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
	}
	else if(type == "backup")
	{
		pthread_rwlock_rdlock(&(it->second->backup_priv_set_lock));
		if(it->second->backup_priv.find(db_name) != it->second->backup_priv.end())
		{
			check_result = 1;
		}
		pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
	}
	else if(type == "export")
	{
		pthread_rwlock_rdlock(&(it->second->export_priv_set_lock));
		if(it->second->export_priv.find(db_name) != it->second->export_priv.end())
		{
			check_result = 1;
		}
		pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
	}
	SLOG_CORE("check ["+ username + "] [" + db_name + "] [" + type + "] privilege: " + to_string(check_result));
	pthread_rwlock_unlock(&users_map_lock);
	return check_result;
}

bool APIUtil::init_privilege(const std::string& username, const std::string& db_name)
{
    if(username == ROOT_USERNAME) {
		return true;
	}
    pthread_rwlock_rdlock(&users_map_lock);
    auto it = users.find(username);
	if(it != users.end() && db_name != Util::system_db)
	{
        string update = "INSERT DATA { \
            <" + username + "> <has_query_priv> <" + db_name + ">. \
            <" + username + "> <has_update_priv> <" + db_name + ">. \
            <" + username + "> <has_load_priv> <" + db_name + ">. \
            <" + username + "> <has_unload_priv> <" + db_name + ">. \
            <" + username + "> <has_restore_priv> <" + db_name + ">. \
            <" + username + "> <has_backup_priv> <" + db_name + ">. \
            <" + username + "> <has_export_priv> <" + db_name + ">. \
        }";
        bool rt = update_sys_db(update);
		if(rt)
		{
            refresh_sys_db();
            update_privilege(it->second, "all", db_name, 1);
		}
        pthread_rwlock_unlock(&users_map_lock);
		return true;
	}
	else
	{
		pthread_rwlock_unlock(&users_map_lock);
		return false;
	}
}

bool APIUtil::copy_privilege(const std::string& src_db_name, const std::string& dst_db_name)
 {
    std::vector<std::string> privileges = {"query", "update", "load", "unload", "restore", "backup", "export"};
    ResultSet rs;
    FILE *output = nullptr;
    std::string sparql;
    stringstream ss;
    ss << "INSERT DATA {";
    unsigned int total_privilegs = 0;
    bool ret_bool = false;
    std::map<std::string, std::vector<std::string>> priv_user_map;
    for (std::string type : privileges)
    {
        sparql = "select ?x where {?x <has_" + type + "_priv> <" + src_db_name + ">.}";
        ret_bool = query_sys_db(sparql, rs);
        if (ret_bool && rs.ansNum > 0) //copy privilege to dst_db_name
        {
            total_privilegs += rs.ansNum;
            auto it = priv_user_map.find(type);
            if (it == priv_user_map.end()) {
                priv_user_map.insert(std::pair<std::string, std::vector<std::string>>(type, {}));
                it = priv_user_map.find(type);
            }
            for (unsigned int i = 0; i < rs.ansNum; i++) {
                ss << rs.answer[i][0] + " <has_" + type + "_priv> <" + dst_db_name + ">.";
                it->second.push_back(util.clear_angle_brackets(rs.answer[i][0]));
            }
        }
        rs.release();
    }
    ss << "}";
    if (total_privilegs > 0)
    {
        std::string insert_sparql = ss.str();
        bool rt = update_sys_db(insert_sparql);
        if (rt)
		{
            refresh_sys_db();
            pthread_rwlock_rdlock(&users_map_lock);
            std::map<std::string, std::vector<std::string>>::iterator iter_priv;
            for (iter_priv = priv_user_map.begin(); iter_priv != priv_user_map.end(); iter_priv++)
            {
                std::string type = iter_priv->first;
                for (std::string username: iter_priv->second)
                {
                    std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
                    if(it != users.end()) {
                        update_privilege(it->second, type, dst_db_name, 1);
                    }
                }
            }
            pthread_rwlock_unlock(&users_map_lock);
            return true;
        } else {
            SLOG_WARN("excuse sparql return error: " + insert_sparql);
            return false;
        }
    }
	SLOG_CORE("no privileges to copy");
    return true;
 }

bool APIUtil::clear_privilege(const string& username)
{
    if (username == ROOT_USERNAME)
	{
		return false;
	}
    pthread_rwlock_rdlock(&users_map_lock);
	std::map<std::string,  shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
	if(it != users.end())
	{
        std::vector<std::string> privileges = {"query", "update", "load", "unload", "restore", "backup", "export"};
        bool result = true;
		for (std::string type : privileges)
		{
            std::string update = "DELETE WHERE{ <" + username + "> <has_" + type + "_priv> ?o.}";
            result = result && update_sys_db(update);
		}
        if (result)
        {
            refresh_sys_db();
            update_privilege(it->second, "all", "", -1);
        }
		pthread_rwlock_unlock(&users_map_lock);
		return result;
	}
	else
	{
		pthread_rwlock_unlock(&users_map_lock);
        return false;
	}
}

bool APIUtil::update_privilege(std::shared_ptr<DBUserInfo>& userinfo, const string& type, const string& db_name, int16_t op)
{
    if ((type == "query" || type == "all") && userinfo->query_priv.find(db_name) != userinfo->query_priv.end())
    {
        pthread_rwlock_wrlock(&(userinfo->query_priv_set_lock));
        if (op == 1)
            userinfo->query_priv.insert(db_name);
        else if (op == 0)
            userinfo->query_priv.erase(db_name);
        else if (op == -1) 
            userinfo->query_priv.clear();
        pthread_rwlock_unlock(&(userinfo->query_priv_set_lock));
    }
    else if((type == "update" || type == "all") && userinfo->update_priv.find(db_name) != userinfo->update_priv.end())
    {
        pthread_rwlock_wrlock(&(userinfo->update_priv_set_lock));
        if (op == 1)
            userinfo->update_priv.insert(db_name);
        else if (op == 0)
            userinfo->update_priv.erase(db_name);
        else if (op == -1) 
            userinfo->update_priv.clear();
        pthread_rwlock_unlock(&(userinfo->update_priv_set_lock));
    }
    else if((type == "load" || type == "all") && userinfo->load_priv.find(db_name) != userinfo->load_priv.end())
    {
        pthread_rwlock_wrlock(&(userinfo->load_priv_set_lock));
        if (op == 1)
            userinfo->load_priv.insert(db_name);
        else if (op == 0)
            userinfo->load_priv.erase(db_name);
        else if (op == -1) 
            userinfo->load_priv.clear();
        pthread_rwlock_unlock(&(userinfo->load_priv_set_lock));
    }
    else if((type == "unload" || type == "all") && userinfo->unload_priv.find(db_name) != userinfo->unload_priv.end())
    {
        pthread_rwlock_wrlock(&(userinfo->unload_priv_set_lock));
        if (op == 1)
            userinfo->unload_priv.insert(db_name);
        else if (op == 0)
            userinfo->unload_priv.erase(db_name);
        else if (op == -1) 
            userinfo->unload_priv.clear();
        pthread_rwlock_unlock(&(userinfo->unload_priv_set_lock));
    }
    else if((type == "restore" || type == "all") && userinfo->restore_priv.find(db_name) != userinfo->restore_priv.end())
    {
        pthread_rwlock_wrlock(&(userinfo->restore_priv_set_lock));
        if (op == 1)
            userinfo->restore_priv.insert(db_name);
        else if (op == 0) 
            userinfo->restore_priv.erase(db_name);
        else if (op == -1) 
            userinfo->restore_priv.clear();
        pthread_rwlock_unlock(&(userinfo->restore_priv_set_lock));
    }
    else if((type == "backup" || type == "all") && userinfo->backup_priv.find(db_name) != userinfo->backup_priv.end())
    {
        pthread_rwlock_wrlock(&(userinfo->backup_priv_set_lock));
        if (op == 1)
            userinfo->backup_priv.insert(db_name);
        else if (op == 0) 
            userinfo->backup_priv.erase(db_name);
        else if (op == -1) 
            userinfo->backup_priv.clear();
        pthread_rwlock_unlock(&(userinfo->backup_priv_set_lock));
    }
    else if((type == "export" || type == "all") && userinfo->export_priv.find(db_name) != userinfo->export_priv.end())
    {
        pthread_rwlock_wrlock(&(userinfo->export_priv_set_lock));
        if (op == 1)
            userinfo->export_priv.insert(db_name);
        else if (op == 0)  
            userinfo->export_priv.erase(db_name);
        else if (op == -1) 
            userinfo->export_priv.clear();
        pthread_rwlock_unlock(&(userinfo->export_priv_set_lock));
    }
    return true;
}

bool APIUtil::query_sys_db(const std::string& sparql, ResultSet& _rs)
{
	pthread_rwlock_rdlock(&system_db_lock);
	FILE* output = NULL;
    QueryTree::UpdateType update_type;
    system_database->isUpdate(sparql, update_type);
    if (update_type == QueryTree::Not_Update)
    {
    	int ret_val = system_database->query(sparql, _rs, output);
        SLOG_CORE("select sparql: " + sparql + ", ansNum: " + to_string(_rs.ansNum) + ", ret_val: " + to_string(ret_val));
        pthread_rwlock_unlock(&system_db_lock);
        return (ret_val == -100);
    }
    else
    {
        SLOG_CORE("not a select query: " << sparql);
        pthread_rwlock_unlock(&system_db_lock);
        return false;
    }
}

bool APIUtil::update_sys_db(const string& sparql)
{
    if (sparql.empty())
    {
        return 0;
    }
    QueryTree::UpdateType update_type;
    if (system_database->isUpdate(sparql, update_type))
    {
        ResultSet _rs;
        pthread_rwlock_wrlock(&system_db_lock);
	    int ret = system_database->query(sparql, _rs, nullptr);
        pthread_rwlock_unlock(&system_db_lock);
        SLOG_CORE("update sparql: " + sparql + ", update num: " + to_string(ret));
        return (ret >= 0);
    }
    else
    {
        SLOG_CORE("not a update query: " + sparql);
        return false;
    }
}

bool APIUtil::refresh_sys_db()
{
    pthread_rwlock_wrlock(&system_db_lock);
	system_database->save();
    system_database.reset();
    system_database = make_shared<Database>(Util::system_db);
	bool flag = system_database->load();
	SLOG_CORE("system database refresh ok.");
    pthread_rwlock_unlock(&system_db_lock);
	return flag;
}

bool APIUtil::mv_or_cp(const string& src, const string& dsc, bool is_mv)
{
    string sys_cmd;
    string log_info;
    if (util.dir_exist(src) == false) {
        // check the source path
        SLOG_ERROR("Source path not exist!");
        return false;
    }
    // check the destnation path
    if (util.dir_exist(dsc) == false) {
        SLOG_CORE("create desc path: " + dsc);
        util.create_dirs(dsc);
    }
    if (is_mv) {
        sys_cmd = "mv " + src + ' ' + dsc;
    } else {
        sys_cmd = "cp -r " + src + ' ' + dsc;
    }
    int code = system(sys_cmd.c_str());
    return code == 0;
}

bool APIUtil::user_add(const string& username, const string& password)
{
    pthread_rwlock_wrlock(&users_map_lock);
    bool result = false;
    if(users.find(username) == users.end())
    {
        string update = "INSERT DATA {<" + username + "> <has_password> \"" + password + "\".}";
        result = update_sys_db(update);
        if (result)
        {
            refresh_sys_db();
            SLOG_CORE("user["+username+"] add success.");
            shared_ptr<struct DBUserInfo> temp_user = make_shared<DBUserInfo>(username, password);
            users.insert(pair<std::string, shared_ptr<struct DBUserInfo>>(username, temp_user));
        }
        else
        {
            SLOG_ERROR("user["+username+"] add failed.");
        }
    }
    pthread_rwlock_unlock(&users_map_lock);
    return result;
}

bool APIUtil::user_delete(const string& username)
{
    pthread_rwlock_wrlock(&users_map_lock);
    bool result = false;
    if(users.find(username) != users.end())
    {
        // clear privileges
        string update = "DELETE WHERE { \
            <" + username + "> <has_password> ?o1. \
            <" + username + "> <has_query_priv> ?o2. \
            <" + username + "> <has_load_priv> ?o3. \
            <" + username + "> <has_unload_priv> ?o4. \
            <" + username + "> <has_update_priv> ?o5. \
            <" + username + "> <has_backup_priv> ?o6. \
            <" + username + "> <has_restore_priv> ?o7. \
            <" + username + "> <has_export_priv> ?o8. \
        }";
        result = update_sys_db(update);
        if (result) 
        {
            users.erase(username);
            refresh_sys_db();
        }
    }
    pthread_rwlock_unlock(&users_map_lock);
    return result;
}

bool APIUtil::user_pwd_alert(const string& username, const string& password)
{
    pthread_rwlock_wrlock(&users_map_lock);
    bool result = false;
    std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator iter;
    iter = users.find(username);
    if(iter != users.end())
    {
        // remove old password
        string update = "DELETE WHERE {<" + username + "> <has_password> ?o.}";
        result = update_sys_db(update);
        if (result)
        {
            // insert new password
            update = "INSERT DATA {<" + username + "> <has_password>  \"" + password + "\".}";
            result = update_sys_db(update);
            if (result)
            {
                iter->second->setPassword(password);
                refresh_sys_db();
            }
            else
            {
                // rollback
                update = "INSERT DATA {<" + username + "> <has_password>  \"" + iter->second->getPassword() + "\".}";
                update_sys_db(update);
                refresh_sys_db();
            }
        }
    }
    pthread_rwlock_unlock(&users_map_lock);
    return result;
}

void APIUtil::get_user_info(vector<shared_ptr<struct DBUserInfo>> &_users)
{
    pthread_rwlock_rdlock(&users_map_lock);
    if(!users.empty())
    {
        std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator iter;
        for (iter = users.begin(); iter != users.end(); iter++)
        {
            _users.push_back(iter->second);
        }
    }
    pthread_rwlock_unlock(&users_map_lock);
}

string APIUtil::ip_enabled_type()
{
    if(whiteList){
        return "2";
    }
    else if (blackList){
        return "1";
    }
    else{
        return "3";
    }
}

void APIUtil::ip_list(const string& type, std::vector<std::string>& ip_list)
{
    if( type == "2") {
        SLOG_CORE("IP white List enabled.");
        for (std::set<std::string>::iterator it = ipWhiteList->ipList.begin(); it!=ipWhiteList->ipList.end();it++)
        {
            ip_list.push_back((*it));
        }
    } else {
        SLOG_CORE("IP black List enabled.");
        for (std::set<std::string>::iterator it = ipBlackList->ipList.begin(); it!=ipBlackList->ipList.end();it++)
        {
            ip_list.push_back((*it));
        }
    }
}

bool APIUtil::ip_save(string ip_type, vector<string> ipVector)
{
    if(ip_type == "1" )
    {
        if(blackList)
        {
            ipBlackList->UpdateIPToFile(ipBlackFile, ipVector, "update by wrokbanch");
            // realod ip list
            ipBlackList->Load(ipBlackFile);
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if(whiteList)
        {
            ipWhiteList->UpdateIPToFile(ipWhiteFile, ipVector, "update by wrokbanch");
            // realod ip list
            ipWhiteList->Load(ipWhiteFile);
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool APIUtil::ip_check(const string& ip)
{
    if(whiteList == 1){
        return ipWhiteList->Check(ip);
    } else if(blackList == 1) {
        return ipBlackList->Check(ip);
    }
    return false;
}

/**
 * check ip 
 * @param ip  
 * @param check_level =1 表示锁定IP  =2 表示判断IP地址错误 =3 不作判定
 * @param msg
 * @return 
*/ 
bool APIUtil::check_access_ip(const string& ip, int check_level, std::string& msg)
{
    if(check_level < 2 && !ip_check(ip))
    {
        msg = "IP Blocked!";
        return false;
    }
    if(check_level !=1 && check_level < 3 && !ip_error_num_check(ip))
    {
        msg = "The ip has too many error during accessing server, the ip has been locked until the server restart!";
        return false;
    }
    msg = "";
    return true;

}

bool APIUtil::ip_error_num_check(const string& ip)
{
    pthread_rwlock_rdlock(&ips_map_lock);
    std::map<std::string, shared_ptr<IpInfo>>::iterator it = ips.find(ip);
    if (it != ips.end())
    {
        pthread_rwlock_unlock(&ips_map_lock);
        unsigned int errornum = it->second->getErrorNum();
        if (errornum > 4)
        {
            return false;
        }
    }
    else
    {
        shared_ptr<IpInfo> ipinfo = make_shared<IpInfo>(ip);
        ips.insert(pair<std::string, shared_ptr<IpInfo>>(ip, ipinfo));
        pthread_rwlock_unlock(&ips_map_lock);
    }
    return true;
}

void APIUtil::update_access_ip_error_num(const string& ip)
{
    pthread_rwlock_rdlock(&ips_map_lock);
    std::map<std::string, shared_ptr<IpInfo>>::iterator it = ips.find(ip);
    if (it != ips.end())
    {
        it->second->addErrorNum();
        pthread_rwlock_unlock(&ips_map_lock);
    }
    else
    {
        shared_ptr<IpInfo> ipinfo = make_shared<IpInfo>(ip);
        ipinfo->addErrorNum();
        ips.insert(pair<std::string, shared_ptr<IpInfo>>(ip, ipinfo));
        pthread_rwlock_unlock(&ips_map_lock);
    }
}

void APIUtil::reset_access_ip_error_num(const string& ip)
{
    pthread_rwlock_rdlock(&ips_map_lock);
    std::map<std::string, shared_ptr<IpInfo>>::iterator it = ips.find(ip);
    if (it != ips.end())
    {
        it->second->setErrorNum(0);
    }
    pthread_rwlock_unlock(&ips_map_lock);
}

void APIUtil::get_access_log_files(std::vector<std::string> &file_list)
{
    DIR *dirp = opendir(APIUtil::access_log_path.c_str());
    if (dirp == NULL)
    {
        SLOG_WARN("access log dir is not exist.");
        return;
    }
    struct dirent *dir_entry = NULL;
    string file_name;
    while ((dir_entry = readdir(dirp)) != NULL)
    {
        file_name = dir_entry->d_name;
        if (file_name.find(".log") != string::npos)
        {
            file_list.push_back(dir_entry->d_name);
        }
        
    }
    closedir(dirp);
}

void APIUtil::get_access_log(const string &date, int &page_no, int &page_size, shared_ptr<struct DBAccessLogs> logPtr)
{
    string accessLog = APIUtil::access_log_path + date + ".log";
    vector<std::string> lines;
    int total_size = 0;
    int total_page = 0;
    if (get_file_lines(lines, accessLog, page_no, page_size, total_size, total_page, &access_log_lock)) 
    {   
        size_t count = lines.size();			
        string line;
        for (size_t i = 0; i < count; i++)
        {
            line = lines[count - i - 1];
            if (line[line.length() - 1] == ',')
            {
                line = line.substr(0, (line.length() - 1));
            }
            logPtr->addAccessLogInfo(line);
        }
    }
    logPtr->setTotalSize(total_size);
    logPtr->setTotalPage(total_page);
}

void APIUtil::write_access_log(string operation, string remoteIP, int statusCode, string statusMsg, string opt_id)
{
    if (access_log_mode == "0")
    {
        return;
    }
    string iplog_name = util.get_date_day();
    string iplogfile = access_log_path + iplog_name + ".log";
    if (util.file_exist(iplogfile) == false)
    {
        SLOG_CORE("ip access log file is not exist, now create it.");
        util.create_file(iplogfile);
    }
    // SLOG_CORE("accesslog: " + iplogfile);
    FILE *ip_logfp = fopen(iplogfile.c_str(), "a");
    if (ip_logfp == NULL)
    {
        SLOG_ERROR("open ip log error.");
        return;
    }
    // Another way to locka many: lock(lk1, lk2...)
    pthread_rwlock_wrlock(&access_log_lock);
    // build json
    string createTime = util.get_date_time();
    string status_msg = string(statusMsg.c_str());
    status_msg = util.string_replace(status_msg, "\r\n", "");
	status_msg = util.string_replace(status_msg, "\n", "");
    status_msg = util.string_replace(status_msg, "    ", "");
    struct DBAccessLogInfo dbAccessLogInfo(remoteIP, operation, statusCode, status_msg, createTime);
    if (!opt_id.empty())
    {
        dbAccessLogInfo.setOptId(opt_id);
    }
    
    string _info = dbAccessLogInfo.toJSON();
    // _info.push_back(',');
    _info.push_back('\n');
    fprintf(ip_logfp, "%s", _info.c_str());

    util.Csync(ip_logfp);
    // long logSize = ftell(ip_logfp);
    fclose(ip_logfp);
    // SLOG_CORE("logSize:" + to_string(logSize);
    pthread_rwlock_unlock(&access_log_lock);
}

void APIUtil::update_access_log(int statusCode, string statusMsg, string opt_id, int state, int num, int failnum, string backupfilepath)
{
    if (opt_id.empty())
        return;
    pthread_rwlock_wrlock(&access_log_lock);
    string iplog_name = getConvertTimeById(opt_id);
    string filename = access_log_path + iplog_name + ".log";
    string file_temp_name = access_log_path + iplog_name + "temp.log";
    if (util.file_exist(filename) == false)
    {
        SLOG_CORE("error ip access log file is not exist");
        return;
    }
    FILE* file = fopen(filename.c_str(), "r");
    FILE* temp_file = fopen(file_temp_name.c_str(), "w");
    char readBuffer[0xffff];
    struct DBAccessLogInfo *logInfo = nullptr;
    while (fgets(readBuffer, 1024, file))
    {
        string rec = readBuffer;
        logInfo = new DBAccessLogInfo(rec);
        if (logInfo->getOptId() != opt_id)
        {
            fputs(readBuffer, temp_file);
            delete logInfo;
            logInfo = NULL;
            continue;
        }
        if (logInfo->checkOperation())
        {
            logInfo->setCode(statusCode);
            logInfo->setMsg(statusMsg);
            string endtime = util.get_date_time();
            logInfo->setEndTime(endtime);
            logInfo->setState(state);
            logInfo->setNum(num);
            logInfo->setFailNum(failnum);
            logInfo->setBackupfilepath(backupfilepath);
            string line = logInfo->toJSON();
            line.push_back('\n');
            fputs(line.c_str(), temp_file);
        }
        else
        {
            fputs(readBuffer, temp_file);
            SLOG_ERROR("access log corrupted, this operation not it!");
        }
        delete logInfo;
        logInfo = NULL;
    }
    fclose(file);
    fclose(temp_file);
    Util::remove_path(filename);
    string cmd = "mv " + file_temp_name + ' ' + filename;
    system(cmd.c_str());
    pthread_rwlock_unlock(&access_log_lock);
}

bool APIUtil::getAccessLogByOptId(string opt_id, struct DBAccessLogInfo& log)
{
    pthread_rwlock_wrlock(&access_log_lock);
    string iplog_name = getConvertTimeById(opt_id);
    string filename = access_log_path + iplog_name + ".log";
    if (util.file_exist(filename) == false)
    {
        SLOG_CORE("error ip access log file is not exist");
        return false;
    }
    FILE* file = fopen(filename.c_str(), "r");
    char readBuffer[0xffff];
    struct DBAccessLogInfo logInfo;
    bool find = false;
    while (fgets(readBuffer, 1024, file))
    {
        string rec = readBuffer;
        logInfo = DBAccessLogInfo(rec);
        if (logInfo.checkOperation() && logInfo.getOptId() == opt_id)
        {
            log = logInfo;
            find = true;
            break;
        }
    }
    fclose(file);
    pthread_rwlock_unlock(&access_log_lock);
    return find;
}

void APIUtil::get_query_log_files(std::vector<std::string> &file_list)
{
    DIR *dirp = opendir(APIUtil::query_log_path.c_str());
    if (dirp == NULL)
    {
        SLOG_WARN("query log dir is not exist.");
        return;
    }
    struct dirent *dir_entry = NULL;
    string file_name;
    while ((dir_entry = readdir(dirp)) != NULL)
    {
        file_name = dir_entry->d_name;
        if (file_name.find(".log") != string::npos)
        {
            file_list.push_back(dir_entry->d_name);
        }
        
    }
    closedir(dirp);
}

void APIUtil::get_query_log(const string &date, int &page_no, int &page_size, shared_ptr<struct DBQueryLogs> logPtr)
{
    string queryLog = APIUtil::query_log_path + date + ".log";
    vector<std::string> lines;
    int total_size = 0;
    int total_page = 0;
    if (get_file_lines(lines, queryLog, page_no, page_size, total_size, total_page, &query_log_lock)) 
    {
        size_t count = lines.size();
        string line;
        for (size_t i = 0; i < count; i++)
        {
            line = lines[count - i - 1];
            if (line[line.length() - 1] == ',')
            {
                line = line.substr(0, (line.length() - 1));
            }
            logPtr->addQueryLogInfo(line);
        }
    }
    logPtr->setTotalSize(total_size);
    logPtr->setTotalPage(total_page);
}

void APIUtil::write_query_log(DBQueryLogInfo* log)
{
    if (query_log_mode == "0")
    {
        return;
    }
    std::string queyrlog_name = util.get_date_day();
    std::string querylog_file = query_log_path + queyrlog_name + ".log";
    if (util.file_exist(querylog_file) == false)
    {
        SLOG_CORE("query log file is not exist, now create it.");
        util.create_file(querylog_file);
    }
    // SLOG_CORE("querylog: " + to_string(querylog_file);
    FILE *querylog_fp = fopen(querylog_file.c_str(), "a");
    if (querylog_fp == NULL)
    {
        SLOG_ERROR("open query log error.");
        return;
    }

    // Another way to locka many: lock(lk1, lk2...)
    pthread_rwlock_wrlock(&query_log_lock);
    std::string _info = log->toJSON();
    _info.push_back(',');
    _info.push_back('\n');
    std::fprintf(querylog_fp, "%s", _info.c_str());

    util.Csync(querylog_fp);
    // long logSize = ftell(querylog_fp);
    std::fclose(querylog_fp);
    // SLOG_CORE("logSize: " + to_string(logSize));
    pthread_rwlock_unlock(&query_log_lock);
}

void APIUtil::init_transactionlog()
{
    pthread_rwlock_wrlock(&transactionlog_lock);
    if (util.file_exist(TRANSACTION_LOG_PATH)) {
        SLOG_CORE("transaction log has been created.");
        pthread_rwlock_unlock(&transactionlog_lock);
        return;
    }
    FILE* fp = fopen(TRANSACTION_LOG_PATH, "w");
    fclose(fp);
    pthread_rwlock_unlock(&transactionlog_lock);
}

int APIUtil::add_transactionlog(std::string db_name, std::string user, std::string TID, std::string begin_time, std::string state , std::string end_time)
{
    pthread_rwlock_wrlock(&transactionlog_lock);
    FILE* fp = fopen(TRANSACTION_LOG_PATH, "a");
    struct TransactionLogInfo logInfo(db_name, TID, user, state, begin_time, end_time);
    string rec = logInfo.toJSON();
    rec.push_back('\n');
    fputs(rec.c_str(), fp);
    fclose(fp);
    pthread_rwlock_unlock(&transactionlog_lock);
    return 0;
}

int APIUtil::update_transactionlog(std::string TID, std::string state, std::string end_time)
{
    pthread_rwlock_wrlock(&transactionlog_lock);
    FILE* fp = fopen(TRANSACTION_LOG_PATH, "r");
    FILE* fp1 = fopen(TRANSACTION_LOG_TEMP_PATH, "w");
    char readBuffer[0xffff];
    int ret = 0;
    struct TransactionLogInfo *logInfo = nullptr;
    while (fgets(readBuffer, 1024, fp)) {
        string rec = readBuffer;
        logInfo = new TransactionLogInfo(rec);
        if (logInfo->getTID() != TID) {
            fputs(readBuffer, fp1);
            delete logInfo;
            logInfo = NULL;
            continue;
        }
        if (!logInfo->getState().empty() && !logInfo->getEndTime().empty()) {
            // COMMITED is final state, dosen't be change
            if (logInfo->getState() != "COMMITED" && logInfo->getState() != "ABORTED" && logInfo->getState() != "ROLLBACK")
            {
                logInfo->setState(state);
                logInfo->setEndTime(end_time);
                string line = logInfo->toJSON();
                line.push_back('\n');
                fputs(line.c_str(), fp1);
            }
            else
            {
                fputs(readBuffer, fp1);
            }
        }
        else 
        {
            fputs(readBuffer, fp1);
            SLOG_ERROR("Transaction log corrupted, please initilize it!");
            ret = 1;
        }
        delete logInfo;
        logInfo = NULL;
    }
    fclose(fp);
    fclose(fp1);
    Util::remove_path(TRANSACTION_LOG_PATH);
    string cmd = "mv ";
    cmd += TRANSACTION_LOG_TEMP_PATH;
    cmd += ' ';
    cmd += TRANSACTION_LOG_PATH;
    system(cmd.c_str());
    pthread_rwlock_unlock(&transactionlog_lock);
    return ret;
}

void APIUtil::get_transactionlog(int &page_no, int &page_size, shared_ptr<struct TransactionLogs> logPtr)
{
    string transactionLog = TRANSACTION_LOG_PATH;
    vector<std::string> lines;
    int total_size = 0;
    int total_page = 0;
    if (get_file_lines(lines, transactionLog, page_no, page_size, total_size, total_page, &transactionlog_lock)) 
    {
        size_t count = lines.size();
        string line;
        for (size_t i = 0; i < count; i++)
        {
            line = lines[count - i - 1];
            logPtr->addTransactionLogInfo(line);
        }
    }
    logPtr->setTotalSize(total_size);
    logPtr->setTotalPage(total_page);
}

void APIUtil::abort_transactionlog(long end_time)
{
    pthread_rwlock_wrlock(&transactionlog_lock);
    FILE* fp = fopen(TRANSACTION_LOG_PATH, "r");
    FILE* fp1 = fopen(TRANSACTION_LOG_TEMP_PATH, "w");
    char readBuffer[0xffff];
    struct TransactionLogInfo *logInfo = nullptr;
    while (fgets(readBuffer, 1024, fp)) {
        string rec = readBuffer;
        logInfo = new TransactionLogInfo(rec);
        if (logInfo->getState() == "RUNNING") 
        {
            logInfo->setState("ROLLBACK");
            logInfo->setEndTime(to_string(end_time));
            string line = logInfo->toJSON();
            line.push_back('\n');
            fputs(line.c_str(), fp1);
        }
        else 
        {
            fputs(readBuffer, fp1);
        }
        delete logInfo;
        logInfo = NULL;
    }
    fclose(fp);
    fclose(fp1);
    Util::remove_path(TRANSACTION_LOG_PATH);
    string cmd = "mv ";
    cmd += TRANSACTION_LOG_TEMP_PATH;
    cmd += ' ';
    cmd += TRANSACTION_LOG_PATH;
    system(cmd.c_str());
    pthread_rwlock_unlock(&transactionlog_lock);
}

string APIUtil::get_Db_path()
{
    return util.getConfigureValue("db_home");
}

string APIUtil::get_Db_suffix()
{
    return util.getConfigureValue("db_suffix");
}

string APIUtil::get_query_result_path()
{
    return query_result_path;
}

int APIUtil::get_thread_pool_num() 
{
    return thread_pool_num;
}

int APIUtil::get_max_output_size()
{
    return max_output_size;
}

string APIUtil::get_root_username()
{
    return ROOT_USERNAME;
}

string APIUtil::get_system_username()
{
    return system_username;
}
int APIUtil::get_connection_num()
{
    return connection_num;
}

void APIUtil::increase_connection_num()
{
    if (connection_num < INT32_MAX)
    {
        connection_num += 1;
    }
}

string APIUtil::get_configure_value(const string& key, string default_value)
{
    string value = util.getConfigureValue(key);
    if (value.empty())
    {
        value = default_value;
    }
    return value;
    
}

int APIUtil::get_configure_value(const string& key, int default_value)
{
    string value = util.getConfigureValue(key);
    if (value.empty())
    {
       return default_value;
    } 
    else if (util.is_number(value))
    {
        return util.string2int(value);
    }
    else
    {
        return default_value;
    }
}

size_t APIUtil::get_configure_value(const string& key, size_t default_value)
{
    string value = util.getConfigureValue(key);
    if (value.empty())
    {
       return default_value;
    } 
    else if (util.is_number(value))
    {
        return stoul(value, nullptr, 0);
    }
    else
    {
        return default_value;
    }
}

std::string 
APIUtil::get_upload_path()
{
    return upload_path;
}

size_t
APIUtil::get_upload_max_body_size()
{
    return upload_max_body_size;
}

bool
APIUtil::check_upload_allow_extensions(const string& suffix)
{
    for (std::string item : upload_allow_extensions)
    {
        if (item == suffix)
        {
            return true;
        }
    }
    return false;
}

bool
APIUtil::check_upload_allow_compress_packages(const string& suffix)
{
    for (std::string item : upload_allow_compress_packages)
    {
        if (item == suffix)
        {
            return true;
        }
    }
    return false;
}

bool 
APIUtil::get_file_lines(vector<string> &lines, string &log_file, int &page_no, int &page_size, int &total_size, int &total_page, pthread_rwlock_t *rw_lock) {
    total_size = 0;
    total_page = 0;
    if(util.file_exist(log_file))
    {
        pthread_rwlock_rdlock(rw_lock);
        ifstream in;
        string line;
        in.open(log_file.c_str(), ios::in);
        int startLine;
        int endLine;
        if(page_no < 1)
        {
            page_no = 1;
        }
        if(page_size < 1)
        {
            page_size = 10;
        }
        startLine = (page_no - 1)*page_size + 1;
        endLine = page_no*page_size + 1;
        //count total
        while (getline(in, line, '\n'))
        {
            total_size++;
        }
        in.close();
        if (total_size == 0)
        {
            pthread_rwlock_unlock(rw_lock);
            return false;
        }
        total_page = (total_size/page_size) + (total_size%page_size == 0 ? 0 : 1);
        if (page_no > total_page)
        {
            pthread_rwlock_unlock(rw_lock);
            throw std::invalid_argument("more then max page number " + to_string(total_page));
        }
        startLine = total_size - page_size*page_no + 1;
        endLine = total_size - page_size*(page_no - 1) + 1;
        if (startLine < 1)
        {
            startLine = 1;
        }
        // seek to start line;
        in.open(log_file.c_str(), ios::in);
        int i_temp;
        char buf_temp[1024];
        in.seekg(0, ios::beg);
        for (i_temp = 1; i_temp < startLine; i_temp++)
        {
            in.getline(buf_temp, sizeof(buf_temp));
        }
        while (startLine < endLine && getline(in, line, '\n')) {
            lines.push_back(line);
            startLine++;
        }
        in.close();
        pthread_rwlock_unlock(rw_lock);
        return true;
    }
    else 
    {
        return false;
    }
}