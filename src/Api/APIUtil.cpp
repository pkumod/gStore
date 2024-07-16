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
    pthread_rwlock_init(&databases_map_lock, NULL);
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
    // #if defined(DEBUG)
    SLOG_CODE("call APIUtil delete");
    // #endif
    pthread_rwlock_rdlock(&databases_map_lock);
    std::map<std::string, shared_ptr<Database>>::iterator iter;
    for (iter = databases.begin(); iter != databases.end(); iter++)
    {
        string database_name = iter->first;
        if (database_name == SYSTEM_DB_NAME)
            continue;
        //abort all transaction
        db_checkpoint(database_name);
        shared_ptr<Database> current_database = iter->second;
        pthread_rwlock_rdlock(&already_build_map_lock);
        std::map<std::string, shared_ptr<DatabaseInfo>>::iterator it_already_build = already_build.find(database_name);
        pthread_rwlock_unlock(&already_build_map_lock);
        // warning: this is going to be blocked, if the time of the system changes
        // default timeout 60 seconds, 60000ms
        unsigned timeout = 60*1000;
        struct timeval now;
        struct timespec str_timeout = {0};
        gettimeofday(&now, NULL);
        str_timeout.tv_sec = now.tv_sec;
        str_timeout.tv_nsec = (now.tv_usec + 1000UL*timeout)*1000UL;
        
        if (it_already_build == already_build.end() || pthread_rwlock_timedwrlock(&(it_already_build->second->db_lock), &str_timeout) != 0)
        {
            SLOG_WARN(database_name + " unable to save due to loss of lock");
            continue;
        }
        current_database->save();
        pthread_rwlock_unlock(&(it_already_build->second->db_lock));
    }
    if (databases.find(SYSTEM_DB_NAME) != databases.end())
    {
        pthread_rwlock_wrlock(&system_db_lock);
        system_database->save();
        pthread_rwlock_unlock(&system_db_lock);
    }
    databases.clear();
    pthread_rwlock_unlock(&databases_map_lock);

    // remove txn_manager
    pthread_rwlock_rdlock(&txn_m_lock);
    txn_managers.clear();
    pthread_rwlock_unlock(&txn_m_lock);

    // remove already build database info
    pthread_rwlock_rdlock(&already_build_map_lock);
    already_build.clear();
    pthread_rwlock_unlock(&already_build_map_lock);

    // remove ips
    pthread_rwlock_wrlock(&ips_map_lock);
    ips.clear();
    pthread_rwlock_unlock(&ips_map_lock);

    // remove users
    pthread_rwlock_wrlock(&users_map_lock);
    users.clear();
    pthread_rwlock_unlock(&users_map_lock);

    pthread_rwlock_destroy(&users_map_lock);
    pthread_rwlock_destroy(&databases_map_lock);
    pthread_rwlock_destroy(&already_build_map_lock);
    pthread_rwlock_destroy(&txn_m_lock);
    pthread_rwlock_destroy(&ips_map_lock);
    pthread_rwlock_destroy(&system_db_lock);
    pthread_rwlock_destroy(&query_log_lock);
    pthread_rwlock_destroy(&access_log_lock);
    pthread_rwlock_destroy(&transactionlog_lock);

    ipWhiteList = nullptr;

    ipBlackList = nullptr;

    if (Util::file_exist(system_password_path))
    {
        Util::remove_path(system_password_path);
    }
    if (Util::file_exist(system_port_path))
    {
        Util::remove_path(system_port_path);
    }
}

int APIUtil::initialize(const std::string server_type, const std::string port, const std::string db_name, bool load_csr)
{
    try
    {
        // #if defined(DEBUG)
        SLOG_CODE("initialization start");
        // #endif
        default_port = get_configure_value("default_port", default_port);
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
        if (ipWhiteFile.empty())
        {
            whiteList = 0;
        }
        else
        {
            whiteList = 1;
        }
        if (ipBlackFile.empty())
        {
            blackList = 0;
        }
        else
        {
            blackList = 1;
        }
        if (whiteList) {
            // #if defined(DEBUG)
            SLOG_CODE("IP white List enabled.");
            // #endif
            ipWhiteList->Load(ipWhiteFile);
        }
        else if (blackList) {
            // #if defined(DEBUG)
            SLOG_CODE("IP black list enabled.");
            // #endif
            ipBlackList->Load(ipBlackFile);
        }

        // init upload conf
        upload_path = get_configure_value("upload_path", upload_path);
        upload_max_body_size = get_configure_value("upload_max_body_size", upload_max_body_size);
        string configure_extensions = get_configure_value("upload_allow_extensions",  "nt|ttl|n3|rdf|txt");
        Util::split(configure_extensions, "|", upload_allow_extensions);
        string configure_compress_packages = get_configure_value("upload_allow_compress_packages",  "zip");
        Util::split(configure_compress_packages, "|", upload_allow_compress_packages);
        
        // load system db
        if(!util.dir_exist(get_Db_path() + "/system" + get_Db_suffix()))
        {
            SLOG_ERROR("Can not find system" + get_Db_suffix());
            return -1;
        }
        system_database = make_shared<Database>(SYSTEM_DB_NAME);
        
        system_database->load();
        // #if defined(DEBUG)
        SLOG_CODE("add system database");
        // #endif
        APIUtil::add_database(SYSTEM_DB_NAME, system_database);

        // init already_build db
        ResultSet rs;
        FILE *output = nullptr;
        std::string sparql = "select ?x where{?x <database_status> \"already_built\".}";       
        int ret_val = system_database->query(sparql, rs, output);
        if (ret_val == -100)
        {
            ResultSet _db_rs;
            pthread_rwlock_wrlock(&already_build_map_lock);
            #if defined(DEBUG)
            rapidjson::Document doc;
            rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
            rapidjson::StringBuffer jsonBuffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> jsonWriter(jsonBuffer);
            doc.SetArray();
            #endif
            for (unsigned int i = 0; i < rs.ansNum; i++)
            {
                string db_name = util.clear_angle_brackets(rs.answer[i][0]);
                shared_ptr<DatabaseInfo> temp_db = make_shared<DatabaseInfo>(db_name);
                
                sparql = "select ?x ?y where{<" + db_name + "> <built_by> ?x. <" + db_name + "> <built_time> ?y.}";
                ret_val = system_database->query(sparql, _db_rs, output);
                if (ret_val == -100 && _db_rs.ansNum > 0)
                {
                    std::string built_by = util.clear_angle_brackets(_db_rs.answer[0][0]);
                    std::string built_time = util.replace_all(_db_rs.answer[0][1], "\"", "");
                    temp_db->setCreator(built_by);
                    temp_db->setTime(built_time);
                    #if defined(DEBUG)
                    rapidjson::Value jsonValue = temp_db->toJSON(allocator);
                    doc.PushBack(jsonValue, allocator);
                    #endif
                    already_build.insert(pair<std::string, shared_ptr<DatabaseInfo>>(db_name, temp_db));
                }
                else
                {
                    SLOG_ERROR("query dabase ["+ db_name + "] properties error: return value " + to_string(ret_val));
                }
            }
            #if defined(DEBUG)
            doc.Accept(jsonWriter);
            SLOG_CODE(jsonBuffer.GetString());
            #endif
            // insert systemdb into already_build
            // struct DatabaseInfo *system_db = new DatabaseInfo(SYSTEM_DB_NAME);
            // already_build.insert(pair<std::string, struct DatabaseInfo *>(SYSTEM_DB_NAME, system_db));
            
            pthread_rwlock_unlock(&already_build_map_lock);
        }
        else
        {
             SLOG_ERROR("init already build dabase error: return value " + to_string(ret_val));
        }
        //userinfo
        sparql = "select ?x ?y where{?x <has_password> ?y.}";
        ret_val = system_database->query(sparql, rs, output);
        if (ret_val == -100)
        {
            ResultSet _user_rs;
            pthread_rwlock_wrlock(&users_map_lock);
            #if defined(DEBUG)
            rapidjson::Document doc;
            rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
            rapidjson::StringBuffer jsonBuffer;
            rapidjson::PrettyWriter<rapidjson::StringBuffer> jsonWriter(jsonBuffer);
            doc.SetArray();
            #endif
            for (unsigned int i = 0; i < rs.ansNum; i++)
            {
                string username = util.clear_angle_brackets(rs.answer[i][0]);
                string password = util.replace_all(rs.answer[i][1], "\"", "");
                shared_ptr<struct DBUserInfo> user = make_shared<struct DBUserInfo>(username, password);
                
                //privilege add
                
                string sparql2 = "select ?x ?y where{<" + username + "> ?x ?y.}";
                //string strJson2 = QuerySys(sparql2);
                ret_val = system_database->query(sparql2, _user_rs, output);
                
                if ( ret_val == -100 )
                {
                    for(unsigned j = 0; j < _user_rs.ansNum; j++)
                    {
                        std::string type = util.clear_angle_brackets(_user_rs.answer[j][0]);
                        std::string db_name = util.clear_angle_brackets(_user_rs.answer[j][1]);
                        if(type == "has_query_priv")
                        {
                            user->query_priv.insert(db_name);
                        }
                        else if(type == "has_update_priv")
                        {
                            user->update_priv.insert(db_name);
                        }
                        else if(type == "has_load_priv")
                        {
                            user->load_priv.insert(db_name);
                        }
                        else if(type == "has_unload_priv")
                        {
                            user->unload_priv.insert(db_name);
                        }
                        else if(type == "has_restore_priv")
                        {
                            user->restore_priv.insert(db_name);
                        }else if(type == "has_backup_priv")
                        {
                            user->backup_priv.insert(db_name);
                        }else if(type == "has_export_priv")
                        {
                            user->export_priv.insert(db_name);
                        }
                    }
                }
                #if defined(DEBUG)
                rapidjson::Value jsonValue = user->toJSON(allocator);
                doc.PushBack(jsonValue, allocator);
                #endif
                users.insert(pair<std::string, shared_ptr<struct DBUserInfo>>(username, user));
            }
            #if defined(DEBUG)
            doc.Accept(jsonWriter);
            SLOG_CODE(jsonBuffer.GetString());
            #endif
            pthread_rwlock_unlock(&users_map_lock);
        }
        else
        {
             SLOG_ERROR("init database users error: return value " + to_string(ret_val));
        }
        init_transactionlog();
        // create system password file
        fstream ofp;
        system_password = util.int2string(util.getRandNum());
        system_password_path = get_Db_path() + "/" + "system" + get_Db_suffix() + "/password" + port + ".txt";
        ofp.open(system_password_path, ios::out);
        ofp << system_password;
        ofp.close();
        // create port file
        system_port_path = get_Db_path() + "/" + "system" + get_Db_suffix() + "/port.txt";
        ofp.open(system_port_path, ios::out);
        ofp << server_type;
        ofp << ":";
        ofp << port;
        ofp.close();
        // load user database
        if(!db_name.empty())
        {
            string result = check_param_value("db_name",db_name);
            if(!result.empty())
            {
                SLOG_ERROR(result);
                return -1;
            }           
            sparql = "ASK WHERE{<" + db_name + "> <database_status> \"already_built\".}";
            system_database->query(sparql, rs, output);
            if (rs.answer[0][0] == "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>")
            {
                SLOG_ERROR("Database " + db_name + " not built yet.");
			    return -1;
            }
            shared_ptr<Database> current_database = make_shared<Database>(db_name);
            bool flag = current_database->load(load_csr);
            if (!flag)
            {
                SLOG_ERROR("Failed to load the database.");
                current_database.reset();
                return -1;
            }
            insert_txn_managers(current_database, db_name);
            add_database(db_name, current_database);
        }
        // #if defined(DEBUG)
        SLOG_CODE("initialization end");
        // #endif
        return 1;
    }
    catch (const std::exception &e)
    {
        SLOG_ERROR("initialization fail: " + string(e.what()));
        return -1;
    }
}

bool APIUtil::unlock_already_build_map()
{
    int rwlock_code = pthread_rwlock_unlock(&already_build_map_lock);
    if ( rwlock_code == 0)
    {
        // #if defined(DEBUG)
        SLOG_CODE("already_build_map unlock ok");
        // #endif
        return true;
    }
    else
    {
        SLOG_ERROR("already_build_map unlock error: " + to_string(rwlock_code));
        return false;
    }
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

vector<string> APIUtil::ip_list(string type)
{
    vector<string>ip_list;
    if(type == "2")
    {
        // #if defined(DEBUG)
        SLOG_CODE("IP white List enabled.");
        // #endif
        for (std::set<std::string>::iterator it = ipWhiteList->ipList.begin(); it!=ipWhiteList->ipList.end();it++)
        {
            ip_list.push_back((*it));
        }
    }
    else
    {
        // #if defined(DEBUG)
        SLOG_CODE("IP black List enabled.");
        // #endif
        for (std::set<std::string>::iterator it = ipBlackList->ipList.begin(); it!=ipBlackList->ipList.end();it++)
        {
            ip_list.push_back((*it));
        }
    }
    return ip_list;
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
    }
    else{
        return ipBlackList->Check(ip);
    }
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

/**
 * check ip 
 * @param ip  
 * @param check_level =1 表示锁定IP  =2 表示判断IP地址错误 =3 不作判定
 * @return
*/ 
string APIUtil::check_access_ip(const string& ip, int check_level)
{
    
    string result = "";
    if(check_level < 2 && !ip_check(ip))
    {
        result = "IP Blocked!";
        return result;
    }
    if(check_level !=1 && check_level < 3 && !ip_error_num_check(ip))
    {
        result = "The ip has too many error during accessing server, the ip has been locked until the server restart!";
        return result;
    }
    return result;

}

int APIUtil::db_copy(string src_path, string dest_path)
{
    string sys_cmd;
    string log_info;
    if (util.dir_exist(src_path) == false)
    {
        // check the source path
        log_info = "Source path error, please check it again!";
        SLOG_ERROR(log_info);
        return 1;
    }
    if (util.dir_exist(dest_path) == false)
    {
        // check the destnation path
        // #if defined(DEBUG)
        log_info = "the path: " + dest_path + " is not exists, system will create it.";
        SLOG_CODE(log_info);
        // #endif
        util.create_dirs(dest_path);
    }
    sys_cmd = "cp -r " + src_path + ' ' + dest_path;
    system(sys_cmd.c_str());
    return 0; // success
}

bool APIUtil::add_database(const std::string &db_name, shared_ptr<Database> &db)
{
    int rwlock_code = pthread_rwlock_wrlock(&databases_map_lock);
    if (rwlock_code != 0) 
    {
        SLOG_ERROR("database_map write lock error: " + to_string(rwlock_code));
        return false;
    }
    // #if defined(DEBUG)
    SLOG_CODE("database_map write lock ok");
    // #endif
    databases.insert(pair<std::string, shared_ptr<Database>>(db_name, db));
    rwlock_code = pthread_rwlock_unlock(&databases_map_lock);
    if (rwlock_code == 0)
    {
        // #if defined(DEBUG)
        SLOG_CODE("database_map unlock ok");
        // #endif
        return true;
    } 
    else
    {
        SLOG_ERROR("database_map unlock error:" + to_string(rwlock_code));
        return false;
    }
}

bool APIUtil::get_databaseinfo(const std::string& db_name, shared_ptr<DatabaseInfo> &dbInfo)
{
    int rwlock_code = pthread_rwlock_rdlock(&already_build_map_lock);
    if (rwlock_code != 0) 
    {
        SLOG_ERROR("already_build_map read lock error: " + to_string(rwlock_code));
        dbInfo = NULL;
        return false;
    }
    std::map<std::string, shared_ptr<DatabaseInfo>>::iterator iter = already_build.find(db_name);
    if (iter!=already_build.end())
    {
        dbInfo = iter->second;
    }
    else
    {
        // #if defined(DEBUG)
        SLOG_ERROR("can't find [" + db_name + "] database info from already_build_map");
        // #endif
        dbInfo = NULL;
    }
    return unlock_already_build_map();
}

bool APIUtil::trywrlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo)
{
    if (dbinfo == NULL || dbinfo == nullptr)
        return false;
    int rwlock_code = pthread_rwlock_trywrlock(&(dbinfo->db_lock));
    if (rwlock_code != 0)
    {
        SLOG_ERROR("try write lock database[" + dbinfo->getName() + "] error: " + to_string(rwlock_code));
        return false;
    }
    else
    {
        // #if defined(DEBUG)
        SLOG_CODE("try write lock database[" + dbinfo->getName() + "] ok");
        // #endif
        return true;
    }
}

bool APIUtil::rdlock_databaseinfo(shared_ptr<DatabaseInfo> &dbinfo)
{
    int rwlock_code = pthread_rwlock_rdlock(&(dbinfo->db_lock));
    if (rwlock_code == 0)
    {
        // #if defined(DEBUG)
        SLOG_CODE("read lock database[" + dbinfo->getName() + "] ok");
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
    
    if (dbinfo == NULL)
    {
        // #if defined(DEBUG)
        SLOG_ERROR("db_info is null");
        // #endif
        return false;
    }
    int rwlock_code = pthread_rwlock_unlock(&(dbinfo->db_lock));
    if (rwlock_code == 0)
    {
        // #if defined(DEBUG)
        SLOG_CODE("database [" + dbinfo->getName() + "] unlock ok");
        // #endif
        return true;
    }
    else
    {
        SLOG_ERROR("database [" + dbinfo->getName() + "] unlock error: " + to_string(rwlock_code));
        return false;
    }
}

bool APIUtil::insert_txn_managers(shared_ptr<Database> &current_database, std::string database)
{
    shared_ptr<Txn_manager> txn_m = make_shared<Txn_manager>(current_database.get(), database);
    if(pthread_rwlock_trywrlock(&txn_m_lock) ==0)
    {
        txn_managers.insert(pair<string, shared_ptr<Txn_manager>>(database, txn_m));
        if (pthread_rwlock_unlock(&txn_m_lock) == 0)
        {
            // #if defined(DEBUG)
            SLOG_CODE("add txn manager for " + database + " ok");
            // #endif
            return true;
        }
    }
    SLOG_ERROR("add txn manager for " + database + " error!");
    return false;
}

bool APIUtil::remove_txn_managers(std::string db_name)
{
    pthread_rwlock_wrlock(&txn_m_lock);
	if (txn_managers.find(db_name) == txn_managers.end())
	{
		string error = "get " + db_name + " txn error!";
        SLOG_ERROR(error);
		pthread_rwlock_unlock(&txn_m_lock);
		return false;
	}
	txn_managers.erase(db_name);
    pthread_rwlock_unlock(&txn_m_lock);
    return true;
}

bool APIUtil::db_checkpoint(string db_name)
{
    int rwlock_code = pthread_rwlock_wrlock(&txn_m_lock);
    if (rwlock_code != 0) 
    {
        SLOG_ERROR("txn_m write lock error: " + to_string(rwlock_code));
        return false;
    }
    // #if defined(DEBUG)
    SLOG_CODE("txn_m write lock ok");
    // #endif
	if (txn_managers.find(db_name) == txn_managers.end())
	{
        SLOG_WARN(db_name + " checkpoint error: can't find txn manager!");
		pthread_rwlock_unlock(&txn_m_lock);
		return false;
	}
	shared_ptr<Txn_manager> txn_m = txn_managers[db_name];
	txn_m->abort_all_running();
	txn_m->Checkpoint();
	txn_managers.erase(db_name);
	rwlock_code = pthread_rwlock_unlock(&txn_m_lock);
    if (rwlock_code == 0)
    {
        // #if defined(DEBUG)
        SLOG_CODE("txn_m unlock ok");
        SLOG_CODE(db_name + " checkpoint success!");
        // #endif
        return true;
    }
    else
    {
        SLOG_ERROR("txn_m unlock error:" + to_string(rwlock_code));
        return false;
    }
}

// bool APIUtil::db_checkpoint_all()
// {
//     pthread_rwlock_rdlock(&databases_map_lock);
//     std::map<std::string, shared_ptr<Database>>::iterator iter;
// 	string return_msg = "";
// 	abort_transactionlog(util.get_cur_time());
//     for(iter=databases.begin(); iter != databases.end(); iter++)
// 	{
// 		string database_name = iter->first;
// 		if (database_name == SYSTEM_DB_NAME)
// 			continue;
// 		//abort all transaction
// 		db_checkpoint(database_name);
// 		shared_ptr<Database> current_database = iter->second;
// 		pthread_rwlock_rdlock(&already_build_map_lock);
// 		std::map<std::string, struct DatabaseInfo *>::iterator it_already_build = already_build.find(database_name);
// 		pthread_rwlock_unlock(&already_build_map_lock);
// 		if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0)
// 		{
// 			pthread_rwlock_unlock(&databases_map_lock);
// 			SLOG_ERROR(database_name + " unable to checkpoint due to loss of lock");
//             break;
// 		}
// 		current_database->save();
// 		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
// 	}
//     if (return_msg.empty())
//     {
//         system_database->save();
//         return true;
//     }
//     else
//     {
//         return false;
//     }
// }

bool APIUtil::delete_from_databases(string db_name)
{
    int rwlock_code = pthread_rwlock_wrlock(&databases_map_lock);
    if (rwlock_code != 0) 
    {
        SLOG_ERROR("database_map write lock error: " + to_string(rwlock_code));
        return false;
    }
    // #if defined(DEBUG)
    SLOG_CODE("database_map write lock ok");
    // #endif
    databases.erase(db_name);
    rwlock_code = pthread_rwlock_unlock(&databases_map_lock);
    if (rwlock_code == 0)
    {
        // #if defined(DEBUG)
        SLOG_CODE("database_map unlock ok");
        // #endif
        return true;
    } 
    else
    {
        SLOG_ERROR("database_map unlock error:" + to_string(rwlock_code));
        return false;
    }
}

bool APIUtil::delete_from_already_build(string db_name)
{
    int rwlock_code = pthread_rwlock_wrlock(&already_build_map_lock);
    if(rwlock_code == 0)
    {
        // remove databse info from system.db
        string update = "DELETE WHERE {<" 
            + db_name + "> <database_status> ?y1. <" 
            + db_name + "> <built_by> ?y2. <" 
            + db_name + "> <built_time> ?y3. }";
		bool update_result = update_sys_db(update);
        // remove all privileges of db_name
        update = "DELETE WHERE {?s <has_query_priv> <" + db_name + ">. }";
        update_result = update_sys_db(update) || update_result;
        update = "DELETE WHERE {?s <has_load_priv> <" + db_name + ">. }";
        update_result = update_sys_db(update) || update_result;
        update = "DELETE WHERE {?s <has_unload_priv> <" + db_name + ">. }";
        update_result = update_sys_db(update) || update_result;
        update = "DELETE WHERE {?s <has_update_priv> <" + db_name + ">. }";
        update_result = update_sys_db(update) || update_result;
        update = "DELETE WHERE {?s <has_backup_priv> <" + db_name + ">. }";
        update_result = update_sys_db(update) || update_result;
        update = "DELETE WHERE {?s <has_restore_priv> <" + db_name + ">. }";
        update_result = update_sys_db(update) || update_result;
        update = "DELETE WHERE {?s <has_export_priv> <" + db_name + ">. }";
        update_result = update_sys_db(update) || update_result;
        if (update_result)
        {
            refresh_sys_db();

            // remove from already build map
            already_build.erase(db_name);

            // clear all privileges 
            std::map<std::string,  shared_ptr<struct DBUserInfo>>::iterator iter;
            for (iter = users.begin(); iter != users.end(); iter++)
            {
                pthread_rwlock_wrlock(&(iter->second->query_priv_set_lock));
                iter->second->query_priv.erase(db_name);
                pthread_rwlock_unlock(&(iter->second->query_priv_set_lock));

                pthread_rwlock_wrlock(&(iter->second->update_priv_set_lock));
                iter->second->update_priv.erase(db_name);
                pthread_rwlock_unlock(&(iter->second->update_priv_set_lock));

                pthread_rwlock_wrlock(&(iter->second->load_priv_set_lock));
                iter->second->load_priv.erase(db_name);
                pthread_rwlock_unlock(&(iter->second->load_priv_set_lock));

                pthread_rwlock_wrlock(&(iter->second->unload_priv_set_lock));
                iter->second->unload_priv.erase(db_name);
                pthread_rwlock_unlock(&(iter->second->unload_priv_set_lock));

                pthread_rwlock_wrlock(&(iter->second->backup_priv_set_lock));
                iter->second->backup_priv.erase(db_name);
                pthread_rwlock_unlock(&(iter->second->backup_priv_set_lock));

                pthread_rwlock_wrlock(&(iter->second->restore_priv_set_lock));
                iter->second->restore_priv.erase(db_name);
                pthread_rwlock_unlock(&(iter->second->restore_priv_set_lock));

                pthread_rwlock_wrlock(&(iter->second->export_priv_set_lock));
                iter->second->export_priv.erase(db_name);
                pthread_rwlock_unlock(&(iter->second->export_priv_set_lock));
            }
        }
        return unlock_already_build_map();
    }
    else
    {
        SLOG_ERROR("already_build_map write lock error:" + to_string(rwlock_code));
        return false;
    }
}

bool APIUtil::get_Txn_ptr(const std::string& db_name, shared_ptr<Txn_manager> &txn_manager)
{
    pthread_rwlock_rdlock(&txn_m_lock);
    bool rt = false;
	if (txn_managers.find(db_name) != txn_managers.end())
	{
        rt = true;
	    txn_manager = txn_managers[db_name];
	}
    else
    {
        txn_manager = NULL;
    }
	pthread_rwlock_unlock(&txn_m_lock);
    return rt;
}

string APIUtil::get_txn_begin_time(shared_ptr<Txn_manager> ptr, txn_id_t tid)
{
    string begin_time = to_string(ptr->Get_Transaction(tid)->GetStartTime());
    return begin_time;
}

string APIUtil::begin_process(string db_name, int level , string username)
{
    string result = "";
    shared_ptr<Txn_manager> txn_m;
    get_Txn_ptr(db_name, txn_m);
    if (txn_m == NULL) 
    {
        return result;
    }
	txn_id_t TID = txn_m->Begin(static_cast<IsolationLevelType>(level));
	// SLOG_CODE("Transcation Id:"<< to_string(TID));
	// SLOG_CODE(to_string(txn_m->Get_Transaction(TID)->GetStartTime()));
	string begin_time = to_string(txn_m->Get_Transaction(TID)->GetStartTime());
	string Time_TID = begin_time + "_" + to_string(TID);
	add_transactionlog(db_name, username, Time_TID, begin_time, "RUNNING", "INF");
    if (TID == INVALID_ID)
	{
		return result;
	}
    result = to_string(TID);
    return result;
}

bool APIUtil::commit_process(shared_ptr<Txn_manager> txn_m, txn_id_t TID)
{
    string begin_time = to_string(txn_m->Get_Transaction(TID)->GetStartTime());
    string Time_TID = begin_time + "_" + to_string(TID);
    int ret = update_transactionlog(Time_TID, "COMMITED", to_string(txn_m->Get_Transaction(TID)->GetEndTime()));
    return ret == 0;
}

bool APIUtil::rollback_process(shared_ptr<Txn_manager> txn_m, txn_id_t TID)
{
    string begin_time = to_string(txn_m->Get_Transaction(TID)->GetStartTime());
    string Time_TID = begin_time + "_" + to_string(TID);
    int ret = update_transactionlog(Time_TID, "ROLLBACK", to_string(txn_m->Get_Transaction(TID)->GetEndTime()));
    return ret == 0;
}

bool APIUtil::aborted_process(shared_ptr<Txn_manager> txn_m, txn_id_t TID)
{
    string begin_time = to_string(txn_m->Get_Transaction(TID)->GetStartTime());
    string Time_TID = begin_time + "_" + to_string(TID);
    int ret = update_transactionlog(Time_TID, "ABORTED", to_string(txn_m->Get_Transaction(TID)->GetEndTime()));
    return ret == 0;
}

txn_id_t APIUtil::check_txn_id(string TID_s)
{
    txn_id_t TID = (unsigned long long)0;
    if(util.is_number(TID_s))
	{
		TID = strtoull(TID_s.c_str(), NULL, 0);
	} 
    else if (TID_s.find("_") != string::npos)
    {   // case for workbench call commit and rollback: "beginTime_tid"
        int pos = TID_s.find("_") + 1;
        string TID_s_new = TID_s.substr(pos, TID_s.size()-pos);
        if (util.is_number(TID_s_new))
        {
            TID = strtoull(TID_s_new.c_str(), NULL, 0);
        }
    }
    return TID;
}

bool APIUtil::get_database(const std::string &db_name, shared_ptr<Database> &db)
{
    bool rwlock_code = pthread_rwlock_rdlock(&databases_map_lock);
    if (rwlock_code != 0) 
    {
        // #if defined(DEBUG)
        SLOG_CODE("database_map read lock error: " + to_string(rwlock_code));
        // #endif
        return false;
    }
    // #if defined(DEBUG)
    SLOG_CODE("database_map read lock ok");
    // #endif
    std::map<std::string, shared_ptr<Database>>::iterator iter = databases.find(db_name);
    if (iter != databases.end())
    {
        db = iter->second;
    }
    else
    {
        db = NULL;
    }
    rwlock_code = pthread_rwlock_unlock(&databases_map_lock);
    if (rwlock_code == 0)
    {
        // #if defined(DEBUG)
        SLOG_CODE("database_map unlock ok");
        // #endif
        return true;
    } 
    else
    {
        // #if defined(DEBUG)
        SLOG_CODE("database_map unlock error:" + to_string(rwlock_code));
        // #endif
        return false;
    }
}

bool APIUtil::check_already_load(const std::string &db_name)
{
    shared_ptr<Database> db;
    bool rt = APIUtil::get_database(db_name, db);
    if (rt && db != NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool APIUtil::add_already_build(const std::string &db_name, const std::string &creator, const std::string &build_time)
{
    int rwlock_code = pthread_rwlock_wrlock(&already_build_map_lock);
    if (rwlock_code != 0)
    {
        SLOG_ERROR("already_build_map write lock error: " + to_string(rwlock_code));
        return false;
    }
    // #if defined(DEBUG)
	SLOG_CODE("already_build_map write lock ok.");
    // #endif
    shared_ptr<DatabaseInfo> temp_db = make_shared<DatabaseInfo>(db_name, creator, build_time);
    already_build.insert(pair<std::string, shared_ptr<DatabaseInfo>>(db_name, temp_db));
    unlock_already_build_map();
    string update = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." +
		"<" + db_name + "> <built_by> <" + creator + "> ." + "<" + db_name + "> <built_time> \"" + build_time + "\".}";
    bool update_result = update_sys_db(update);
    if (update_result)
        refresh_sys_db();
    // #if defined(DEBUG)
    SLOG_CODE("database add done.");
    // #endif
    return update_result;
}

void APIUtil::get_already_builds(const std::string& username, vector<shared_ptr<DatabaseInfo>> &array)
{
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, shared_ptr<DatabaseInfo>>::iterator iter;
    for (iter = already_build.begin(); iter != already_build.end(); iter++)
    {
        shared_ptr<DatabaseInfo> db_info = iter->second;
        if (db_info->getName() == SYSTEM_DB_NAME)
        {
            continue;
        }
        if (username != ROOT_USERNAME && check_privilege(username, "query", db_info->getName()) == false)
        {
            continue;
        }
        
        if (APIUtil::check_already_load(db_info->getName()))
        {
            db_info->setStatus("loaded");
        }
        else
        {
            db_info->setStatus("unloaded");
        }
        array.push_back(db_info);
    }
    pthread_rwlock_unlock(&already_build_map_lock);
}

bool APIUtil::check_already_build(const std::string &db_name)
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

bool APIUtil::trywrlock_database(const std::string &db_name)
{
    bool result = false;
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, shared_ptr<DatabaseInfo>>::iterator iter = already_build.find(db_name);
    pthread_rwlock_unlock(&already_build_map_lock);
    if (pthread_rwlock_trywrlock(&(iter->second->db_lock)) == 0)
    {
        result = true;
    }
    return result;
}

bool APIUtil::rdlock_database(const std::string &db_name)
{
    bool result = false;
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, shared_ptr<DatabaseInfo>>::iterator iter = already_build.find(db_name);
    pthread_rwlock_unlock(&already_build_map_lock);
    if (pthread_rwlock_rdlock(&(iter->second->db_lock)) == 0)
    {
        result = true;
    }
    return result;
}

bool APIUtil::unlock_database(const std::string &db_name)
{
    bool result = false;
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, shared_ptr<DatabaseInfo>>::iterator iter = already_build.find(db_name);
    pthread_rwlock_unlock(&already_build_map_lock);
    if (pthread_rwlock_unlock(&(iter->second->db_lock)) == 0)
    {
        result = true;
    }
    return result;
}

std::string APIUtil::check_indentity(const std::string &username, const std::string &password, const std::string &encryption)
{
    pthread_rwlock_rdlock(&users_map_lock);
    std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
    string error = "";
    if (it == users.end())
    {
        error = "Username or password is wrong.";
    }
    else if (encryption == "1")
    {
        if (util.md5(it->second->getPassword()) != password)
        {
            error = "Username or password is wrong.";
        }
    }
    else if (it->second->getPassword() != password)
    {
        error = "Username or password is wrong.";
    }
    pthread_rwlock_unlock(&users_map_lock);
    return error;
}

std::string APIUtil::check_server_indentity(const std::string& password)
{
    string error = "";
	if(password != system_password)
	{
		error = "the password is wrong. please check the " + system_password_path;
	}
    return error;
}

std::string APIUtil::check_param_value(const string& paramname, const string& value)
{
    string result = "";
	if (value.empty()) 
	{
		result = "the value of " + paramname + " can not be empty!";
		return result;
	}
	if (paramname == "db_name")
	{
		string database = value;
		if (database == SYSTEM_DB_NAME)
		{
			result = "you can not operate the system database";
			return result;
		}
		string db_suffix = get_Db_suffix();
        size_t len_suffix = db_suffix.length();
		if (database.length() > len_suffix && database.substr(database.length() - len_suffix, len_suffix) == db_suffix)
		{
			result = "Your db name to be built should not end with \"" + db_suffix + "\".";
			return result;
		}
		
	}
	return "";
}

bool APIUtil::check_user_exist(const std::string& username)
{
    pthread_rwlock_rdlock(&users_map_lock);
    std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
    pthread_rwlock_unlock(&users_map_lock);
	if(it != users.end())
        return true;
    else
        return false;
}

bool APIUtil::check_user_count()
{
    return users.size() < max_user_num;
}

bool APIUtil::check_db_exist(const std::string& db_name)
{
    bool result = true;
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, shared_ptr<DatabaseInfo>>::iterator iter = already_build.find(db_name);
    if (iter == already_build.end())
	{
		result=false;
	}
    pthread_rwlock_unlock(&already_build_map_lock);
	return result;
}

bool APIUtil::check_db_count()
{
    return already_build.size() < max_database_num;
}

bool APIUtil::add_privilege(const std::string& username, const vector<string>& types, const std::string& db_name)
{
    if(username == ROOT_USERNAME)
	{
		return 1;
	}
    pthread_rwlock_rdlock(&users_map_lock);
    std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
	if(it != users.end() && db_name != SYSTEM_DB_NAME)
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
        bool add_result = APIUtil::update_sys_db(update);
        if (add_result)
        {
            refresh_sys_db();
            for (unsigned i = 0; i < types.size(); i++)
            {
                string type = types[i];
                if(type == "query")
                {
                    pthread_rwlock_wrlock(&(it->second->query_priv_set_lock));
                    it->second->query_priv.insert(db_name);
                    pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
                }
                else if(type == "update")
                {
                    pthread_rwlock_wrlock(&(it->second->update_priv_set_lock));
                    it->second->update_priv.insert(db_name);
                    pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
                }
                else if(type == "load")
                {
                    pthread_rwlock_wrlock(&(it->second->load_priv_set_lock));
                    it->second->load_priv.insert(db_name);
                    pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
                }
                else if(type == "unload")
                {
                    pthread_rwlock_wrlock(&(it->second->unload_priv_set_lock));
                    it->second->unload_priv.insert(db_name);
                    pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
                }
                else if(type == "restore")
                {
                    pthread_rwlock_wrlock(&(it->second->restore_priv_set_lock));
                    it->second->restore_priv.insert(db_name);
                    pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
                }
                else if(type == "backup")
                {
                    pthread_rwlock_wrlock(&(it->second->backup_priv_set_lock));
                    it->second->backup_priv.insert(db_name);
                    pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
                }
                else if(type == "export")
                {
                    pthread_rwlock_wrlock(&(it->second->export_priv_set_lock));
                    it->second->export_priv.insert(db_name);
                    pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
                }
            }
        }
        pthread_rwlock_unlock(&users_map_lock);
		return add_result;
	}
	else
	{
		pthread_rwlock_unlock(&users_map_lock);
		return 0;
	}
}

bool APIUtil::update_sys_db(string query)
{
    if (query.empty())
    {
        return 0;
    }
    // #if defined(DEBUG)
    SLOG_CODE("update sparql: " + query);
    // #endif
    pthread_rwlock_wrlock(&system_db_lock);
    ResultSet _rs;
    FILE* ofp = stdout;
    string msg;
	int ret = system_database->query(query, _rs, ofp);
    if (ret <= -100)  //select query
    {
        if(ret == -100)
        {
            msg = _rs.to_str();
        }
        else //query error
        {
            msg = "query failed.";
        }
        // #if defined(DEBUG)
        SLOG_CODE(msg);
        // #endif
        pthread_rwlock_unlock(&system_db_lock);
        return false;
    }
    else //update query
    {
        if(ret >= 0)
        {
            // #if defined(DEBUG)
            msg = "update num: " + util.int2string(ret);
            SLOG_CODE(msg);
            // #endif
            pthread_rwlock_unlock(&system_db_lock);
            return true;
        }
        else //update error
        {
            msg = "update failed.";
            SLOG_ERROR(msg);
            pthread_rwlock_unlock(&system_db_lock);
            return false;
        }
    }
}

bool APIUtil::refresh_sys_db()
{
    pthread_rwlock_wrlock(&system_db_lock);
	system_database->save();
    APIUtil::delete_from_databases(SYSTEM_DB_NAME);
	system_database = make_shared<Database>(SYSTEM_DB_NAME);
	bool flag = system_database->load();
    // #if defined(DEBUG)
	SLOG_CODE("system database refresh");
    // #endif
    if (flag) 
    {
        APIUtil::add_database(SYSTEM_DB_NAME, system_database);
    }
    pthread_rwlock_unlock(&system_db_lock);
	return flag;
}

std::string APIUtil::query_sys_db(const std::string& sparql)
{
	pthread_rwlock_rdlock(&system_db_lock);
    ResultSet rs;
	FILE* output = NULL;

	int ret_val = system_database->query(sparql, rs, output);
	bool ret = false, update = false;
	if(ret_val < -1)   //non-update query
	{
		ret = (ret_val == -100);
	}
	else  //update query, -1 for error, non-negative for num of triples updated
	{
		update = true;
	}

	if(ret)
	{
        // #if defined(DEBUG)
		SLOG_CODE("search system db returned successfully.");
        // #endif
		string success = rs.to_JSON();
		pthread_rwlock_unlock(&system_db_lock);
		return success;
	}
	else
	{
		string error = "";
		// todo: return this error code
		// int error_code;
		if(!update)
		{
			SLOG_ERROR("search system db returned error.");
			error = "search query returns false.";
			// error_code = 403;
		}
		
		pthread_rwlock_unlock(&system_db_lock);

		return error;
	}
	
}

bool APIUtil::build_db_user_privilege(std::string db_name, std::string username)
{
	string time = util.get_date_time();
    return add_already_build(db_name, username, time);
}

bool APIUtil::user_add(const string& username, const string& password)
{
    pthread_rwlock_wrlock(&users_map_lock);
    bool result = false;
    if(users.find(username) == users.end())
    {
        // #if defined(DEBUG)
        SLOG_CODE("user ready to add.");
        // #endif
        shared_ptr<struct DBUserInfo> temp_user = make_shared<DBUserInfo>(username, password);
        users.insert(pair<std::string, shared_ptr<struct DBUserInfo>>(username, temp_user));
        string update = "INSERT DATA {<" + username + "> <has_password> \"" + password + "\".}";
        result = update_sys_db(update);
        if (result)
            refresh_sys_db();
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
        users.erase(username);
        string update = "DELETE WHERE {<" + username + "> <has_password> ?o.}" ;
        result = update_sys_db(update);
        // clear privileges
        update = "DELETE WHERE {<" + username + "> <has_query_priv> ?o.}";
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_load_priv> ?o.}";
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_unload_priv> ?o.}"; 
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_update_priv> ?o.}";
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_backup_priv> ?o.}";
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_restore_priv> ?o.}"; 
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_export_priv> ?o.}";
		result = update_sys_db(update) || result;
        if (result)
            refresh_sys_db();
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
        iter->second->setPassword(password);
        string update = "DELETE WHERE {<" + username + "> <has_password> ?o.}";
        result = update_sys_db(update);
        if (result)
        {
            update = "INSERT DATA {<" + username + "> <has_password>  \"" + password + "\".}";
            result = update_sys_db(update) || result;
            refresh_sys_db();
        }
    }
    pthread_rwlock_unlock(&users_map_lock);
    return result;
}

int APIUtil::clear_user_privilege(string username)
{
    if (username == ROOT_USERNAME)
	{
		return 0;
	}
    pthread_rwlock_rdlock(&users_map_lock);
	std::map<std::string,  shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
	if(it != users.end())
	{
        string update = "DELETE WHERE {<" + username + "> <has_query_priv> ?o.}";
        bool result = update_sys_db(update);
        update = "DELETE WHERE {<" + username + "> <has_load_priv> ?o.}";
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_unload_priv> ?o.}"; 
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_update_priv> ?o.}";
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_backup_priv> ?o.}";
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_restore_priv> ?o.}"; 
        result = update_sys_db(update) || result;
        update = "DELETE WHERE {<" + username + "> <has_export_priv> ?o.}";
		result = update_sys_db(update) || result;
        if (result) 
        {
            refresh_sys_db();
            pthread_rwlock_wrlock(&(it->second->query_priv_set_lock));
            it->second->query_priv.clear();
            pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
                    
            pthread_rwlock_wrlock(&(it->second->load_priv_set_lock));
            it->second->load_priv.clear();
            pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
            
            pthread_rwlock_wrlock(&(it->second->unload_priv_set_lock));
            it->second->unload_priv.clear();
            pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
            
            pthread_rwlock_wrlock(&(it->second->update_priv_set_lock));
            it->second->update_priv.clear();
            pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
            
            pthread_rwlock_wrlock(&(it->second->backup_priv_set_lock));
            it->second->backup_priv.clear();
            pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
            
            pthread_rwlock_wrlock(&(it->second->restore_priv_set_lock));
            it->second->restore_priv.clear();
            pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
            
            pthread_rwlock_wrlock(&(it->second->export_priv_set_lock));
            it->second->export_priv.clear();
            pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
        }
		pthread_rwlock_unlock(&users_map_lock);
		return result;
	}
	else
	{
		pthread_rwlock_unlock(&users_map_lock);
        return 0;
	}
}

bool APIUtil::del_privilege(const std::string& username, const vector<string>& types, const std::string& db_name)
{
    if (username == ROOT_USERNAME)
	{
		return 0;
	}
    pthread_rwlock_rdlock(&users_map_lock);
	std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
	if(it != users.end() && db_name != SYSTEM_DB_NAME)
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
            } else 
            {
                continue;
            }
            // delete privilege
            del_result = update_sys_db(update);
            refresh_flag = refresh_flag || del_result;
            // remove from privilege set
            if (del_result)
            {
                if(type == "query" && it->second->query_priv.find(db_name) != it->second->query_priv.end())
                {
                    pthread_rwlock_wrlock(&(it->second->query_priv_set_lock));
                    it->second->query_priv.erase(db_name);
                    pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
                }
                else if(type == "update" && it->second->update_priv.find(db_name) != it->second->update_priv.end())
                {
                    pthread_rwlock_wrlock(&(it->second->update_priv_set_lock));
                    it->second->update_priv.erase(db_name);
                    pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
                }
                else if(type == "load" && it->second->load_priv.find(db_name) != it->second->load_priv.end())
                {
                    pthread_rwlock_wrlock(&(it->second->load_priv_set_lock));
                    it->second->load_priv.erase(db_name);
                    pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
                }
                else if(type == "unload" && it->second->unload_priv.find(db_name) != it->second->unload_priv.end())
                {
                    pthread_rwlock_wrlock(&(it->second->unload_priv_set_lock));
                    it->second->unload_priv.erase(db_name);
                    pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
                }
                else if(type == "backup" && it->second->backup_priv.find(db_name) != it->second->backup_priv.end())
                {
                    pthread_rwlock_wrlock(&(it->second->backup_priv_set_lock));
                    it->second->backup_priv.erase(db_name);
                    pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
                }
                else if(type == "restore" && it->second->restore_priv.find(db_name) != it->second->restore_priv.end())
                {
                    pthread_rwlock_wrlock(&(it->second->restore_priv_set_lock));
                    it->second->restore_priv.erase(db_name);
                    pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
                }
                else if(type == "export" && it->second->export_priv.find(db_name) != it->second->export_priv.end())
                {
                    pthread_rwlock_wrlock(&(it->second->export_priv_set_lock));
                    it->second->export_priv.erase(db_name);
                    pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
                }
            }
        }
        if (refresh_flag)
            refresh_sys_db();
        pthread_rwlock_unlock(&users_map_lock);
        return del_result;
	}
    else
    {
        pthread_rwlock_unlock(&users_map_lock);
		return 0;
    }
}

bool APIUtil::check_privilege(const std::string& username, const std::string& type, const std::string& db_name)
{
	if(db_name == SYSTEM_DB_NAME)
		return 0;

	if(username == ROOT_USERNAME)
		return 1;

    if (type == "login" || type == "testConnect" || type == "getCoreVersion" 
        || type == "funquery" || type == "funcudb" || type == "funreview"
        || type == "check" || type == "show" || type == "userpassword" || type == "upload" || type == "download")
    {
        return 1;
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
    // #if defined(DEBUG)
	SLOG_CODE("check ["+ username + "] [" + db_name + "] [" + type + "] privilege: " + to_string(check_result));
    // #endif
	pthread_rwlock_unlock(&users_map_lock);
	return check_result;
}

bool APIUtil::init_privilege(const std::string& username, const std::string& db_name)
{
    if(username == ROOT_USERNAME)
	{
		return 1;
	}
    pthread_rwlock_rdlock(&users_map_lock);
    std::map<std::string, shared_ptr<struct DBUserInfo>>::iterator it = users.find(username);
	if(it != users.end() && db_name != SYSTEM_DB_NAME)
	{
        string update = "INSERT DATA {<" + username + "> <has_query_priv> <" + db_name 
            + ">.<" + username + "> <has_update_priv> <" + db_name 
            + ">.<" + username + "> <has_load_priv> <" + db_name 
            + ">.<" + username + "> <has_unload_priv> <" + db_name 
            + ">.<" + username + "> <has_restore_priv> <" + db_name 
            + ">.<" + username + "> <has_backup_priv> <" + db_name 
            + ">.<" + username + "> <has_export_priv> <" + db_name + ">.}";
        bool rt = update_sys_db(update);
		if(rt)
		{
            refresh_sys_db();
			pthread_rwlock_wrlock(&(it->second->query_priv_set_lock));
			it->second->query_priv.insert(db_name);
			pthread_rwlock_unlock(&(it->second->query_priv_set_lock));

			pthread_rwlock_wrlock(&(it->second->update_priv_set_lock));
			it->second->update_priv.insert(db_name);
			pthread_rwlock_unlock(&(it->second->update_priv_set_lock));

			pthread_rwlock_wrlock(&(it->second->load_priv_set_lock));
			it->second->load_priv.insert(db_name);
			pthread_rwlock_unlock(&(it->second->load_priv_set_lock));

			pthread_rwlock_wrlock(&(it->second->unload_priv_set_lock));
			it->second->unload_priv.insert(db_name);
			pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));

			pthread_rwlock_wrlock(&(it->second->restore_priv_set_lock));
			it->second->restore_priv.insert(db_name);
			pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));

			pthread_rwlock_wrlock(&(it->second->backup_priv_set_lock));
			it->second->backup_priv.insert(db_name);
			pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));

			pthread_rwlock_wrlock(&(it->second->export_priv_set_lock));
			it->second->export_priv.insert(db_name);
			pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
		}
        pthread_rwlock_unlock(&users_map_lock);
		return 1;
	}
	else
	{
		pthread_rwlock_unlock(&users_map_lock);
		return 0;
	}
}

 bool APIUtil::copy_privilege(const std::string& src_db_name, const std::string& dst_db_name)
 {
    std::vector<std::string> privileges = {"query", "update", "load", "unload", "restore", "backup","export"};
    ResultSet rs;
    FILE *output = nullptr;
    std::string sparql;
    stringstream ss;
    ss << "INSERT DATA {";
    unsigned int total_privilegs = 0;
    std::map<std::string, std::vector<std::string>> priv_user_map;
    pthread_rwlock_rdlock(&system_db_lock);
    for (std::string type:privileges)
    {
        sparql = "select ?x where {?x <has_" + type + "_priv> <" + src_db_name + ">.}";
        int ret_val = system_database->query(sparql, rs, output);
        total_privilegs += rs.ansNum;
        if (ret_val == -100 && rs.ansNum > 0) //copy privilege to dst_db_name
        {
            std::map<std::string, std::vector<std::string>>::iterator it = priv_user_map.find(type);
            if (it == priv_user_map.end())
            {
                priv_user_map.insert(std::pair<std::string, std::vector<std::string>>(type, {}));
                it = priv_user_map.find(type);
            }
            
            for (unsigned int i = 0; i < rs.ansNum; i++)
            {
                ss << rs.answer[i][0] + " <has_" + type + "_priv> <" + dst_db_name + ">.";
                it->second.push_back(util.clear_angle_brackets(rs.answer[i][0]));
            }
        }
    }
    pthread_rwlock_unlock(&system_db_lock);
    ss << "}";
    if (total_privilegs > 0)
    {
        std::string insert_sparql = ss.str();
        bool rt = update_sys_db(insert_sparql);
        if(rt)
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
                    if(it != users.end()) 
                    {
                        if (type == "query")
                        {
                            pthread_rwlock_wrlock(&(it->second->query_priv_set_lock));
                            it->second->query_priv.insert(dst_db_name);
                            pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
                        }
                        else if(type == "update")
                        {
                            pthread_rwlock_wrlock(&(it->second->update_priv_set_lock));
                            it->second->update_priv.insert(dst_db_name);
                            pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
                        }
                        else if(type == "load")
                        {
                            pthread_rwlock_wrlock(&(it->second->load_priv_set_lock));
                            it->second->load_priv.insert(dst_db_name);
                            pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
                        }
                        else if(type == "unload")
                        {
                            pthread_rwlock_wrlock(&(it->second->unload_priv_set_lock));
                            it->second->unload_priv.insert(dst_db_name);
                            pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
                        }
                        else if(type == "restore")
                        {
                            pthread_rwlock_wrlock(&(it->second->restore_priv_set_lock));
                            it->second->restore_priv.insert(dst_db_name);
                            pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
                        }
                        else if(type == "backup")
                        {
                            pthread_rwlock_wrlock(&(it->second->backup_priv_set_lock));
                            it->second->backup_priv.insert(dst_db_name);
                            pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
                        }
                        else if(type == "export")
                        {
                            pthread_rwlock_wrlock(&(it->second->export_priv_set_lock));
                            it->second->export_priv.insert(dst_db_name);
                            pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
                        }
                    }
                }
            }
            pthread_rwlock_unlock(&users_map_lock);
            return 1;
        }
        else
        {
            SLOG_WARN("excuse sparql return error: " + insert_sparql);
            return 0;
        }
    }
    // #if defined(DEBUG)
	SLOG_CODE("no privileges to copy");
    // #endif
    return 1;
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

void APIUtil::get_access_log(const string &date, int &page_no, int &page_size, struct DBAccessLogs *dbAccessLogs)
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
            dbAccessLogs->addAccessLogInfo(line);
        }
    }
    dbAccessLogs->setTotalSize(total_size);
    dbAccessLogs->setTotalPage(total_page);
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
        SLOG_CODE("ip access log file is not exist, now create it.");
        util.create_file(iplogfile);
    }
    // SLOG_CODE("accesslog: " + iplogfile);
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
    // SLOG_CODE("logSize:" + to_string(logSize);
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
        SLOG_CODE("error ip access log file is not exist");
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
        SLOG_CODE("error ip access log file is not exist");
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

void APIUtil::get_query_log(const string &date, int &page_no, int &page_size, struct DBQueryLogs *dbQueryLogs)
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
            dbQueryLogs->addQueryLogInfo(line);
        }
    }
    dbQueryLogs->setTotalSize(total_size);
    dbQueryLogs->setTotalPage(total_page);
}

void APIUtil::write_query_log(struct DBQueryLogInfo *queryLogInfo)
{
    if (query_log_mode == "0")
    {
        return;
    }
    std::string queyrlog_name = util.get_date_day();
    std::string querylog_file = query_log_path + queyrlog_name + ".log";
    if (util.file_exist(querylog_file) == false)
    {
        SLOG_CODE("query log file is not exist, now create it.");
        util.create_file(querylog_file);
    }
    // SLOG_CODE("querylog: " + to_string(querylog_file);
    FILE *querylog_fp = fopen(querylog_file.c_str(), "a");
    if (querylog_fp == NULL)
    {
        SLOG_ERROR("open query log error.");
        return;
    }
    std::string _info = queryLogInfo->toJSON();

    // Another way to locka many: lock(lk1, lk2...)
    pthread_rwlock_wrlock(&query_log_lock);
    _info.push_back(',');
    _info.push_back('\n');
    std::fprintf(querylog_fp, "%s", _info.c_str());

    util.Csync(querylog_fp);
    // long logSize = ftell(querylog_fp);
    std::fclose(querylog_fp);
    // SLOG_CODE("logSize: " + to_string(logSize));
    pthread_rwlock_unlock(&query_log_lock);
}

void APIUtil::init_transactionlog()
{
    pthread_rwlock_wrlock(&transactionlog_lock);
    if (util.file_exist(TRANSACTION_LOG_PATH)) {
        SLOG_CODE("transaction log has been created.");
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

void APIUtil::get_transactionlog(int &page_no, int &page_size, struct TransactionLogs *dbQueryLogs)
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
            dbQueryLogs->addTransactionLogInfo(line);
        }
    }
    dbQueryLogs->setTotalSize(total_size);
    dbQueryLogs->setTotalPage(total_page);
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

string APIUtil::get_default_port()
{
    return default_port;
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