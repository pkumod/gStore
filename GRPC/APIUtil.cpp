/*
 * @Author: wangjian
 * @Date: 2021-12-20 16:38:46
 * @LastEditTime: 2022-04-01 09:43:02
 * @LastEditors: Please set LastEditors
 * @Description: grpc util
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
    pthread_rwlock_init(&query_log_lock, NULL);
    pthread_rwlock_init(&access_log_lock, NULL);
    pthread_rwlock_init(&transactionlog_lock, NULL);
    ipWhiteList = new IPWhiteList();
    ipBlackList = new IPBlackList();
}

APIUtil::~APIUtil()
{
    cout << "call APIUtil delete" << endl;
    pthread_rwlock_rdlock(&databases_map_lock);
    std::map<std::string, Database *>::iterator iter;
    for (iter = databases.begin(); iter != databases.end(); iter++)
    {
        string database_name = iter->first;
        if (database_name == SYSTEM_DB_NAME)
            continue;
        Database *current_database = iter->second;
        pthread_rwlock_rdlock(&already_build_map_lock);
        std::map<std::string, struct DatabaseInfo *>::iterator it_already_build = already_build.find(database_name);
        pthread_rwlock_unlock(&already_build_map_lock);
        if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0)
        {
            continue;
        }
        current_database->save();
        delete current_database;
        current_database = NULL;
        pthread_rwlock_unlock(&(it_already_build->second->db_lock));
    }
    system_database->save();
    delete system_database;
    system_database = NULL;
    pthread_rwlock_unlock(&databases_map_lock);

    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, DatabaseInfo *>::iterator it_already_build;
    for (it_already_build = already_build.begin(); it_already_build != already_build.end(); it_already_build++)
    {
        DatabaseInfo *temp_db = it_already_build->second;
        delete temp_db;
        temp_db = NULL;
    }
    pthread_rwlock_unlock(&already_build_map_lock);

    pthread_rwlock_destroy(&users_map_lock);
    pthread_rwlock_destroy(&databases_map_lock);
    pthread_rwlock_destroy(&already_build_map_lock);
    pthread_rwlock_destroy(&query_log_lock);
    pthread_rwlock_destroy(&access_log_lock);
    pthread_rwlock_destroy(&transactionlog_lock);
    delete ipWhiteList;
    delete ipBlackList;

    string cmd = "rm " + system_password_path;
    system(cmd.c_str());
    cmd = "rm system.db/port.txt";
    system(cmd.c_str());
}

int APIUtil::initialize(const std::string port, const std::string db_name, bool load_csr)
{
    try
    {
        std::cout << "--------APIUtil initialization start--------" << std::endl;
        Util util;
        util.configure_new();
        system_path = Util::getConfigureValue("system_path");
        system_path = Util::replace_all(system_path, "\"", "");
        std::cout << "system_path:\t\t" << system_path << endl;
        backup_path = Util::getConfigureValue("backup_path");
        backup_path = Util::replace_all(backup_path, "\"", "");
        std::cout << "backup_path:\t\t" << backup_path << endl;
        DB_path = Util::getConfigureValue("db_path");
        std::cout << "DB_path:\t\t" << DB_path << endl;
        root_username = Util::getConfigureValue("root_username");
        root_username = Util::replace_all(root_username, "\"", "");
        std::cout << "root_username:\t\t" << root_username << endl;
        system_username = Util::getConfigureValue("system_username");
        system_username = Util::replace_all(system_username, "\"", "");
        std::cout << "system_username:\t\t" << system_username << endl;
        max_output_size = Util::string2int(Util::getConfigureValue("max_output_size"));
        std::cout << "max_output_size:\t" << max_output_size << endl;
        query_log_path = Util::getConfigureValue("querylog_path");
        query_log_path = Util::replace_all(query_log_path, "\"", "");
        std::cout << "query_log_path:\t\t" << query_log_path << endl;
        if (Util::dir_exist(query_log_path) == false)
        {
            Util::create_dir(query_log_path);
        }
        access_log_path = Util::getConfigureValue("accesslog_path");
        access_log_path = Util::replace_all(access_log_path, "\"", "");
        std::cout << "access_log_path:\t" << access_log_path << endl;
        if (Util::dir_exist(access_log_path) == false)
        {
            Util::create_dir(access_log_path);
        }
        pfn_file_path = Util::getConfigureValue("pfn_file_path");
        pfn_file_path = Util::replace_all(pfn_file_path, "\"", "");
        std::cout << "pfn_file_path:\t\t" << pfn_file_path << endl;
        pfn_lib_path = Util::getConfigureValue("pfn_lib_path");
        pfn_lib_path = Util::replace_all(pfn_lib_path, "\"", "");
        std::cout << "pfn_lib_path:\t\t" << pfn_lib_path << endl;

        pfn_include_header = PFN_HEADER;

        //load ip-list
        ipWhiteFile = Util::getConfigureValue("ip_allow_path");
        ipWhiteFile = Util::replace_all(ipWhiteFile, "\"", "");
        std::cout << "ipWhiteFile:\t\t" << ipWhiteFile << endl;
        ipBlackFile = Util::getConfigureValue("ip_deny_path");
        ipBlackFile = Util::replace_all(ipBlackFile, "\"", "");
        std::cout << "ipBlackFile:\t\t" << ipBlackFile << endl;
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
            cout << "IP white List enabled." << endl;
            ipWhiteList = new IPWhiteList();
            ipWhiteList->Load(ipWhiteFile);
        }
        else if (blackList) {
            cout << "IP black list enabled." << endl;
            ipBlackList = new IPBlackList();
            ipBlackList->Load(ipBlackFile);
        }
        
        // load system db
        if(!Util::dir_exist("system.db"))
        {
            Util::formatPrint("Can not find system.db.","ERROR");
            return -1;
        }
        system_database = new Database(SYSTEM_DB_NAME);
        
        system_database->load();
        cout << "add system database" << endl;
        APIUtil::add_database(SYSTEM_DB_NAME, system_database);

        // init already_build db
        ResultSet rs;
        rapidjson::Document doc;
        FILE *output = NULL;
        std::string sparql = "select ?x where{?x <database_status> \"already_built\".}";       
        int ret_val = system_database->query(sparql, rs, output);
        if (ret_val == -100)
        {
            std::string json_str = rs.to_JSON();
            cout << json_str << endl;
            doc.Parse(json_str.c_str());
            if (!doc.HasParseError())
            {
                rapidjson::Value &p1 = doc["results"];
                rapidjson::Value &p2 = p1["bindings"];
                pthread_rwlock_wrlock(&already_build_map_lock);
                for (int i = 0; i < p2.Size(); i++)
                {
                    rapidjson::Value &p21 = p2[i];
                    rapidjson::Value &p22 = p21["x"];
                    string db_name = p22["value"].GetString();
                    struct DatabaseInfo *temp_db = new DatabaseInfo(db_name);

                    sparql = "select ?x ?y where{<" + db_name + "> ?x ?y.}";
                    ret_val = system_database->query(sparql, rs, output);
                    if (ret_val == -100)
                    {
                        json_str = rs.to_JSON();
                        doc.Parse(json_str.c_str());
                        if (!doc.HasParseError())
                        {
                            p21 = doc["results"];
                            p22 = p21["bindings"];
                            for (int j = 0; j < p22.Size(); j++)
                            {
                                rapidjson::Value &p31 = p22[j];
                                rapidjson::Value &p32 = p31["x"];
                                rapidjson::Value &p33 = p31["y"];
                                std::string type = p32["value"].GetString();
                                std::string info = p33["value"].GetString();
                                if (type == "built_by")
                                    temp_db->setCreator(info);
                                else if (type == "built_time")
                                    temp_db->setTime(info);
                            }
                        }
                        else
                        {
                            std::cout << "parse dabase " << db_name << "properties error: " << doc.GetParseError() << std::endl;
                        }
                    }
                    else
                    {
                        std::cout << "query dabase " << db_name << "properties error: " << doc.GetParseError() << std::endl;
                    }
                    already_build.insert(pair<std::string, struct DatabaseInfo *>(db_name, temp_db));
                    std::cout << db_name << " already build at " << temp_db->getTime() << endl;
                }
                //insert systemdb into already_build
                // struct DatabaseInfo *system_db = new DatabaseInfo(SYSTEM_DB_NAME);
                // already_build.insert(pair<std::string, struct DatabaseInfo *>(SYSTEM_DB_NAME, system_db));
                
                pthread_rwlock_unlock(&already_build_map_lock);
            }
            else
            {
                std::cout << "init already build database error:" << doc.GetParseError() << std::endl;
            }
        }
        //userinfo
        sparql = "select ?x ?y where{?x <has_password> ?y.}";
        ret_val = system_database->query(sparql, rs, output);
        if (ret_val == -100)
        {
            std::string json_str = rs.to_JSON();
            doc.Parse(json_str.c_str());
            if (!doc.HasParseError())
            {
                rapidjson::Value &p1 = doc["results"];
                rapidjson::Value &p2 = p1["bindings"];
                pthread_rwlock_wrlock(&users_map_lock);
                for (int i = 0; i < p2.Size(); i++)
                {
                    rapidjson::Value &pp = p2[i];
                    rapidjson::Value &pp1 = pp["x"];
                    rapidjson::Value &pp2 = pp["y"];
                    string username = pp1["value"].GetString();
                    string password = pp2["value"].GetString();
                    struct DBUserInfo *user = new DBUserInfo(username, password);
                    
                    //privilege add
                    
                    string sparql2 = "select ?x ?y where{<" + username + "> ?x ?y.}";
                    //string strJson2 = QuerySys(sparql2);
                    ret_val = system_database->query(sparql2, rs, output);
                    
                    if ( ret_val == -100 ){
                        std::string json_str2 = rs.to_JSON();
                        doc.Parse(json_str2.c_str());
                        rapidjson::Value &p12 = doc["results"];
                        rapidjson::Value &p22 = p12["bindings"];
                        for(int j = 0; j < p22.Size(); j++)
                        {
                            rapidjson::Value &ppj = p22[j];
                            rapidjson::Value &pp12 = ppj["x"];
                            rapidjson::Value &pp22 = ppj["y"];
                            std::string type = pp12["value"].GetString();
                            std::string db_name = pp22["value"].GetString();
                            //cout << "DDDDDDDDDDDDDDDDDB2Map: type: " + type << " db_name: " << db_name << endl;
                        
                            if(type == "has_query_priv")
                            {
                                //cout << username << type << db_name << endl;
                                user->query_priv.insert(db_name);
                            }
                            else if(type == "has_update_priv")
                            {
                                //cout << username << type << db_name << endl;
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
                    users.insert(pair<std::string, struct DBUserInfo *>(username, user));
                }
                
                pthread_rwlock_unlock(&users_map_lock);
            }
        }
        init_transactionlog();
        // create system password file
        fstream ofp;
        system_password = Util::int2string(Util::getRandNum());
        system_password_path = "system.db/password" + port + ".txt";
        ofp.open(system_password_path, ios::out);
        ofp << system_password;
        ofp.close();
        // create port file
        ofp.open("system.db/port.txt", ios::out);
        ofp << port;
        ofp.close();
        // load user database
        if(!db_name.empty())
        {
            string result = check_param_value("db_name",db_name);
            if(!result.empty())
            {
                cout<<result<<endl;
                return -1;
            }           
            if(Util::dir_exist(db_name + ".db") == false)
            {
                cout << "Database " << db_name << ".db has not been built." << endl;
			    return -1;
            }
            Database* current_database = new Database(db_name);
            bool flag = current_database->load(load_csr);
            if (!flag)
            {
                Util::formatPrint("Failed to load the database.","ERROR");
                delete current_database;
                current_database = NULL;
                return -1;
            }
            insert_txn_managers(current_database,db_name);
            add_database(db_name,current_database);
        }
        std::cout << "--------APIUtil initialization end--------" << std::endl;
        return 1;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }
}

bool APIUtil::trywrlock_database_map()
{
    if (pthread_rwlock_trywrlock(&databases_map_lock) == 0)
    {
        Util::formatPrint("trywrlock_database_map success");
        return true;
    }
    else {
        Util::formatPrint("trywrlock_database_map unsuccess");
        return false;
    }
}

bool APIUtil::unlock_database_map()
{
    if (pthread_rwlock_unlock(&databases_map_lock) == 0)
    {
        Util::formatPrint("unlock database_map success");
        return true;
    }
    else
    {
        Util::formatPrint("unlock database_map unsuccess");
        return false;
    }
        
}

bool APIUtil::trywrlock_already_build_map()
{
    if (pthread_rwlock_trywrlock(&already_build_map_lock) == 0)
    {
        Util::formatPrint("trywrlock_already_build_map success");
        return true;
    }
        
    else
    {
        Util::formatPrint("trywrlock_already_build_map unsuccess");
        return false;
    }
        
}

bool APIUtil::unlock_already_build_map()
{
    if (pthread_rwlock_unlock(&already_build_map_lock) == 0)
    {
        Util::formatPrint("unlock_already_build_map success");
        return true;
    }
        
    else
    {
        Util::formatPrint("unlock_already_build_map unsuccess");
        return false;
    }
        
}

bool APIUtil::rw_wrlock_build_map()
{
    if(pthread_rwlock_wrlock(&already_build_map_lock) == 0)
    {
        Util::formatPrint("lock already_build_map success");
        return true;
    }
    else
    {
        Util::formatPrint("lock already_build_map fail", "ERROR");
        return false;
    }
}

bool APIUtil::rw_wrlock_database_map()
{
    if(pthread_rwlock_wrlock(&databases_map_lock) == 0)
    {
        Util::formatPrint("lock database_map success");
        return true;
    }
    else
    {
        Util::formatPrint("lock database_map fail", "ERROR");
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
        Util::formatPrint("IP white List enabled.");
        for (std::set<std::string>::iterator it = ipWhiteList->ipList.begin(); it!=ipWhiteList->ipList.end();it++)
        {
            ip_list.push_back((*it));
        }
    }
    else{
        Util::formatPrint("IP black List enabled.");
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

bool APIUtil::ip_check(string ip)
{
    if(whiteList == 1){
        return ipWhiteList->Check(ip);
    }
    else{
        return ipBlackList->Check(ip);
    }
    return true;
}

bool APIUtil::ip_error_num_check(string ip)
{
    pthread_rwlock_rdlock(&ips_map_lock);
	std::map<std::string, struct IpInfo *>::iterator it = ips.find(ip);
	if(it != ips.end())
	{
		pthread_rwlock_unlock(&ips_map_lock);
		int errornum=it->second->getErrorNum();
		if(errornum>5)
		{
			return false;
		}
	}
	else{
		struct IpInfo *ipinfo=new IpInfo(ip);
		ips.insert(pair<std::string, struct IpInfo *>(ip, ipinfo));
		pthread_rwlock_unlock(&ips_map_lock);
	}
	return true;
}

string APIUtil::check_access_ip(string ip)
{
    string result = "";
    if(!ip_check(ip))
    {
        result = "IP Blocked!";
        return result;
    }
    if(!ip_error_num_check(ip))
    {
        result = "The ip havs too many error during accessing ghttp, the ip has been locked until the ghttp restart!";
        return result;
    }
    return result;

}

int APIUtil::db_copy(string src_path, string dest_path)
{
    string sys_cmd;
    string log_info;
    if (Util::dir_exist(src_path) == false)
    {
        // check the source path
        log_info = "Source path error, please check it again!";
        Util::formatPrint(log_info, "ERROR");
        return 1;
    }
    if (Util::dir_exist(dest_path) == false)
    {
        // check the destnation path
        log_info = "the path: " + dest_path + " is not exist ,system will create it.";
        Util::formatPrint(log_info, "INFO");
        sys_cmd = "mkdir -p ./" + dest_path;
        system(sys_cmd.c_str());
    }
    sys_cmd = "cp -r " + src_path + ' ' + dest_path;
    system(sys_cmd.c_str());
    return 0; // success
}

bool APIUtil::add_database(const std::string &db_name, Database *&db)
{
    Util::formatPrint("try lock database_map");
    if (!APIUtil::rw_wrlock_database_map())
    {
        Util::formatPrint("database_map lock false", "ERROR");
        return false;
    }

    Util::formatPrint("database_map lock true");
    databases.insert(pair<std::string, Database *>(db_name, db));
    if (APIUtil::unlock_database_map())
    {
        Util::formatPrint("database_map unlock true");
    } 
    else
    {
        Util::formatPrint("database_map unlock false", "ERROR");
    }
    return true;
}

DatabaseInfo* APIUtil::get_databaseinfo(const std::string& db_name)
{
    pthread_rwlock_rdlock(&already_build_map_lock);

    DatabaseInfo* dbinfo = NULL;
    std::map<std::string, struct DatabaseInfo *>::iterator iter = already_build.find(db_name);
    if (iter!=already_build.end())
    {
        dbinfo = iter->second;
    }

    pthread_rwlock_unlock(&already_build_map_lock);
 
    return dbinfo;
}

bool APIUtil::trywrlock_databaseinfo(DatabaseInfo *dbinfo)
{
    if (pthread_rwlock_trywrlock(&(dbinfo->db_lock)) == 0)
        return true;
    else
        return false;
}

bool APIUtil::tryrdlock_databaseinfo(DatabaseInfo* dbinfo)
{
    if (pthread_rwlock_tryrdlock(&(dbinfo->db_lock)) == 0)
    {
        Util::formatPrint("tryrdlock_databaseinfo success");
        return true;
    }
    else
    {
        Util::formatPrint("tryrdlock_databaseinfo unsuccess","ERROR");
        return false;
    }
}

bool APIUtil::unlock_databaseinfo(DatabaseInfo* dbinfo)
{
    
    if(dbinfo == NULL) Util::formatPrint("db_info is null","ERROR");
    if (pthread_rwlock_unlock(&(dbinfo->db_lock)) == 0){
        Util::formatPrint("unlock_databaseinfo success");
        return true;
    }
    else
    {
        Util::formatPrint("unlock_databaseinfo unsuccess","ERROR");
        return false;
    }
}

bool APIUtil::insert_txn_managers(Database* current_database, std::string database)
{
    shared_ptr<Txn_manager> txn_m = make_shared<Txn_manager>(current_database, database);
    if(pthread_rwlock_trywrlock(&txn_m_lock) ==0)
    {
        cout<<"begin txn manager..."<<endl;
        txn_managers.insert(pair<string, shared_ptr<Txn_manager>>(database, txn_m));
        if (pthread_rwlock_unlock(&txn_m_lock) == 0)
        {
            return true;
        }
    }
    return false;
    //cout<<"begin txn manager..."<<endl;
    
}

bool APIUtil::find_txn_managers(std::string db_name)
{
    cout<<"unload txn_manager:"<<txn_managers.size()<<endl;
    pthread_rwlock_rdlock(&txn_m_lock);
    if (txn_managers.find(db_name) == txn_managers.end())
    {
        pthread_rwlock_unlock(&txn_m_lock);
        return false;
    }
    pthread_rwlock_unlock(&txn_m_lock);
    return true;
}

bool APIUtil::db_checkpoint(string db_name)
{
    pthread_rwlock_wrlock(&txn_m_lock);
	if (txn_managers.find(db_name) == txn_managers.end())
	{
		string error = "checkpoint error: " + db_name;
        Util::formatPrint(error, "ERROR");
		pthread_rwlock_unlock(&txn_m_lock);
		return false;
	}
	shared_ptr<Txn_manager> txn_m = txn_managers[db_name];
	txn_managers.erase(db_name);
	pthread_rwlock_unlock(&txn_m_lock);
	txn_m->abort_all_running();
	txn_m->Checkpoint();
    Util::formatPrint("txn_m checkpoint success!");
	return true;
}

// string APIUtil::db_checkpoint_all()
// {
//     pthread_rwlock_rdlock(&databases_map_lock);
//     std::map<std::string, Database *>::iterator iter;
// 	string return_msg = "";
// 	abort_transactionlog(Util::get_cur_time());
//     for(iter=databases.begin(); iter != databases.end(); iter++)
// 	{
// 		string database_name = iter->first;
// 		if (database_name == SYSTEM_DB_NAME)
// 			continue;
// 		//abort all transaction
// 		db_checkpoint(database_name);
// 		Database *current_database = iter->second;
// 		pthread_rwlock_rdlock(&already_build_map_lock);
// 		std::map<std::string, struct DatabaseInfo *>::iterator it_already_build = already_build.find(database_name);
// 		pthread_rwlock_unlock(&already_build_map_lock);
// 		if (pthread_rwlock_trywrlock(&(it_already_build->second->db_lock)) != 0)
// 		{
// 			pthread_rwlock_unlock(&databases_map_lock);
// 			return_msg = "Unable to checkpoint due to loss of lock";
//             break;
// 		}
// 		current_database->save();
// 		delete current_database;
// 		current_database = NULL;
// 		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
// 	}
//     if (return_msg.empty())
//     {
//         system_database->save();
//         delete system_database;
//         system_database = NULL;
//         pthread_rwlock_unlock(&databases_map_lock);

//         pthread_rwlock_rdlock(&already_build_map_lock);
//         std::map<std::string, struct DatabaseInfo *>::iterator it_already_build;
//         for (it_already_build = already_build.begin(); it_already_build != already_build.end(); it_already_build++)
//         {
//             struct DatabaseInfo *temp_db = it_already_build->second;
//             delete temp_db;
//             temp_db = NULL;
//         }
//         pthread_rwlock_unlock(&already_build_map_lock);
//     }
//     return return_msg;
// }

bool APIUtil::delete_from_databases(string db_name)
{
    pthread_rwlock_wrlock(&databases_map_lock);
    Database *db = NULL;
    std::map<std::string, Database *>::iterator iter = databases.find(db_name);
    if (iter != databases.end())
    {
        db = iter->second;
        delete db;
        db = NULL;
    }
    databases.erase(db_name);
    pthread_rwlock_unlock(&databases_map_lock);
    return true;
}

bool APIUtil::delete_from_already_build(string db_name)
{
    if(APIUtil::trywrlock_already_build_map()){
        already_build.erase(db_name);
        APIUtil::unlock_already_build_map();
        return true;
    }
    else
    {
        return false;
    }
}

shared_ptr<Txn_manager> APIUtil::get_Txn_ptr(string db_name)
{
    pthread_rwlock_rdlock(&txn_m_lock);
	if (txn_managers.find(db_name) == txn_managers.end())
	{
		pthread_rwlock_unlock(&txn_m_lock);
		return NULL;
	}

	auto txn_m = txn_managers[db_name];
	pthread_rwlock_unlock(&txn_m_lock);
    return txn_m;
}

txn_id_t APIUtil::get_txn_id(shared_ptr<Txn_manager> ptr, int level)
{
    txn_id_t TID = ptr->Begin(static_cast<IsolationLevelType>(level));
	// cout <<"Transcation Id:"<< to_string(TID) << endl;
    return TID;
}

string APIUtil::get_txn_begin_time(shared_ptr<Txn_manager> ptr, txn_id_t tid)
{
    string begin_time = to_string(ptr->Get_Transaction(tid)->GetStartTime());
    return begin_time;
}

string APIUtil::begin_process(string db_name, int level , string username)
{
    string result = "";
    shared_ptr<Txn_manager> txn_m = APIUtil::get_Txn_ptr(db_name);
    cerr << "Isolation Level Type:" << level << endl;
	txn_id_t TID = txn_m->Begin(static_cast<IsolationLevelType>(level));
	// cout <<"Transcation Id:"<< to_string(TID) << endl;
	// cout << to_string(txn_m->Get_Transaction(TID)->GetStartTime()) << endl;
	string begin_time = to_string(txn_m->Get_Transaction(TID)->GetStartTime());
	string Time_TID = begin_time + "_" + to_string(TID);
	add_transactionlog(db_name, username, Time_TID, begin_time, "RUNNING", "INF");
    if (TID == INVALID_ID)
	{
		return result;
	}
    string TID_s = to_string(TID);
    return TID_s;
}

bool APIUtil::commit_process(shared_ptr<Txn_manager> txn_m, txn_id_t TID)
{
    string begin_time = to_string(txn_m->Get_Transaction(TID)->GetStartTime());
    string Time_TID = begin_time + "_" + to_string(TID);
    update_transactionlog(Time_TID, "COMMITED", to_string(txn_m->Get_Transaction(TID)->GetEndTime()));
}

bool APIUtil::rollback_process(shared_ptr<Txn_manager> txn_m, txn_id_t TID)
{
    string begin_time = to_string(txn_m->Get_Transaction(TID)->GetStartTime());
    string Time_TID = begin_time + "_" + to_string(TID);
    update_transactionlog(Time_TID, "ROLLBACK", to_string(txn_m->Get_Transaction(TID)->GetEndTime()));
}

txn_id_t APIUtil::check_txn_id(string TID_s)
{
    txn_id_t TID = NULL;
    if(Util::is_number(TID_s))
	{
		TID = strtoull(TID_s.c_str(), NULL, 0);
	} 
    else if (TID_s.find("_") != string::npos)
    {   // case for workbench call commit and rollback: "begin_time tid"
        int pos = TID_s.find("_") + 1;
        string TID_s_new = TID_s.substr(pos, TID_s.size()-pos);
        if (Util::is_number(TID_s_new))
        {
            TID = strtoull(TID_s_new.c_str(), NULL, 0);
        }
    }
    return TID;
}

Database *APIUtil::get_database(const std::string &db_name)
{
    pthread_rwlock_rdlock(&databases_map_lock);
    Database *db = NULL;
    std::map<std::string, Database *>::iterator iter = databases.find(db_name);
    if (iter != databases.end())
    {
        db = iter->second;
    }
    pthread_rwlock_unlock(&databases_map_lock);
    return db;
}

bool APIUtil::check_already_load(const std::string &db_name)
{
    Database *rt = APIUtil::get_database(db_name);
    if (rt == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool APIUtil::add_already_build(const std::string &db_name, const std::string &creator, const std::string &build_time, bool try_lock)
{
    if (try_lock && !APIUtil::trywrlock_already_build_map())
    {
        return false;
    }
    struct DatabaseInfo *temp_db = new DatabaseInfo(db_name, creator, build_time);
    already_build.insert(pair<std::string, struct DatabaseInfo *>(db_name, temp_db));
    if (try_lock)
    {
        APIUtil::unlock_already_build_map();
    }
    return true;
}

std::string APIUtil::get_already_build(const std::string &db_name)
{
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, struct DatabaseInfo *>::iterator iter = already_build.find(db_name);
    pthread_rwlock_unlock(&already_build_map_lock);
    if (iter == already_build.end())
    {
        return "";
    }
    else
    {
        return iter->second->toJSON();
    }
}

void APIUtil::get_already_builds(vector<struct DatabaseInfo *> &array)
{
    pthread_rwlock_rdlock(&already_build_map_lock);
    // rapidjson::StringBuffer strBuf;
    // rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
    // writer.StartArray();
    std::map<std::string, struct DatabaseInfo *>::iterator iter;
    for (iter = already_build.begin(); iter != already_build.end(); iter++)
    {
        DatabaseInfo *db_info = iter->second;
        if (db_info->getName() == SYSTEM_DB_NAME)
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
        // writer.String(db_info->toJSON().c_str());
    }
    // writer.EndArray();
    pthread_rwlock_unlock(&already_build_map_lock);
    // return strBuf.GetString();
}

bool APIUtil::check_already_build(const std::string &db_name)
{
    string rt = APIUtil::get_already_build(db_name);
    if (rt.empty())
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
    std::map<std::string, struct DatabaseInfo *>::iterator iter = already_build.find(db_name);
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
    std::map<std::string, struct DatabaseInfo *>::iterator iter = already_build.find(db_name);
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
    std::map<std::string, struct DatabaseInfo *>::iterator iter = already_build.find(db_name);
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
    std::map<std::string, struct DBUserInfo *>::iterator it = users.find(username);
    string error = "";
    if (it == users.end())
    {
        error = "username not find.";
    }
    else if (encryption == "1")
    {
        if (Util::md5(it->second->getPassword()) != password)
        {
            error = "wrong password.";
        }
    }
    else if (it->second->getPassword() != password)
    {
        error = "wrong password.";
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

std::string APIUtil::check_param_value(string paramname, string value)
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
		if (database.length() > 3 && database.substr(database.length() - 3, 3) == ".db")
		{
			//cout << "Your db name to be built should not end with \".db\"." << endl;
			result = "Your db name to be built should not end with \".db\".";
			return result;
		}
		
	}
	return "";
}

bool APIUtil::check_db_exist(const std::string& db_name)
{
    bool result = true;
    pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, struct DatabaseInfo *>::iterator iter = already_build.find(db_name);
    if (iter == already_build.end())
	{
		result=false;
	}
    pthread_rwlock_unlock(&already_build_map_lock);
	return result;
}

bool APIUtil::add_privilege(const std::string& username, const std::string& type, const std::string& db_name)
{
    if(username == ROOT_USERNAME)
	{
		return 1;
	}
    pthread_rwlock_rdlock(&users_map_lock);
    std::map<std::string, struct DBUserInfo *>::iterator it = users.find(username);
	if(it != users.end() && db_name != SYSTEM_DB_NAME)
	{
		pthread_rwlock_unlock(&users_map_lock);
		if(type == "query")
		{
			pthread_rwlock_wrlock(&(it->second->query_priv_set_lock));
			it->second->query_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_query_priv> <" + db_name + ">.}";
			update_sys_db(update);
				
			pthread_rwlock_unlock(&(it->second->query_priv_set_lock));

		}
		else if(type == "update")
		{
			pthread_rwlock_wrlock(&(it->second->update_priv_set_lock));
			it->second->update_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_update_priv> <" + db_name + ">.}";
			update_sys_db(update);
				
			pthread_rwlock_unlock(&(it->second->update_priv_set_lock));

		}
		else if(type == "load")
		{
			pthread_rwlock_wrlock(&(it->second->load_priv_set_lock));
			it->second->load_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_load_priv> <" + db_name + ">.}";
			update_sys_db(update);
			
			pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
		}
		else if(type == "unload")
		{
			pthread_rwlock_wrlock(&(it->second->unload_priv_set_lock));
			it->second->unload_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_unload_priv> <" + db_name + ">.}";
			update_sys_db(update);
			
			pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
		}
		else if(type == "restore")
		{
			pthread_rwlock_wrlock(&(it->second->restore_priv_set_lock));
			it->second->restore_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_restore_priv> <" + db_name + ">.}";
			update_sys_db(update);

			pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
		}
		else if(type == "backup")
		{
			pthread_rwlock_wrlock(&(it->second->backup_priv_set_lock));
			it->second->backup_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_backup_priv> <" + db_name + ">.}";
			update_sys_db(update);

			pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
		}
		else if(type == "export")
		{
			pthread_rwlock_wrlock(&(it->second->export_priv_set_lock));
			it->second->export_priv.insert(db_name);
			string update = "INSERT DATA {<" + username + "> <has_export_priv> <" + db_name + ">.}";
			update_sys_db(update);

			pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
		}
		return 1;
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
    printf("query is:\n%s\n\n", query.c_str());
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

			return false;
		}
		else //update query
		{
			if(ret >= 0)
			{
				msg = "update num: " + Util::int2string(ret);
				cout << msg << endl;
				refresh_sys_db();
				//system_database->save();
				//delete system_database;
				//system_database=NULL;
				return true;
			}
			else //update error
			{
				msg = "update failed.";
				cout << msg << endl;
				return false;
			}
		}

}

bool APIUtil::refresh_sys_db()
{
    pthread_rwlock_rdlock(&databases_map_lock);
	system_database->save();
	delete system_database;
	system_database = NULL;
	system_database = new Database(SYSTEM_DB_NAME);
	bool flag = system_database->load();
	cout << "system database refresh" << endl;
	pthread_rwlock_unlock(&databases_map_lock);
	return flag;
}

std::string APIUtil::query_sys_db(const std::string& sparql)
{
    string db_name = SYSTEM_DB_NAME;
	pthread_rwlock_rdlock(&already_build_map_lock);
    std::map<std::string, struct DatabaseInfo *>::iterator it_already_build = already_build.find(db_name);
    pthread_rwlock_unlock(&already_build_map_lock);
	
	pthread_rwlock_rdlock(&(it_already_build->second->db_lock));
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
		cout << "search query returned successfully." << endl;
		
		string success = rs.to_JSON();
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));
		return success;
	}
	else
	{
		string error = "";
		int error_code;
		if(!update)
		{
			cout << "search query returned error." << endl;
			error = "search query returns false.";
			error_code = 403;
		}
		
		pthread_rwlock_unlock(&(it_already_build->second->db_lock));

		return error;
	}
	
}

bool APIUtil::build_db_user_privilege(std::string db_name, std::string username)
{
    pthread_rwlock_wrlock(&already_build_map_lock);
	cout << "already_build_map_lock acquired." << endl;
	string time = Util::get_date_time();
    struct DatabaseInfo* temp_db = new DatabaseInfo(db_name, username, time);
    already_build.insert(pair<std::string, struct DatabaseInfo*>(db_name, temp_db));
    pthread_rwlock_unlock(&already_build_map_lock);
    string update = "INSERT DATA {<" + db_name + "> <database_status> \"already_built\"." +
		"<" + db_name + "> <built_by> <" + username + "> ." + "<" + db_name + "> <built_time> \"" + time + "\".}";
    update_sys_db(update);
    cout << "database add done." << endl;
    return true;
}

bool APIUtil::user_add(string username, string password)
{
    pthread_rwlock_trywrlock(&users_map_lock);
    bool result = false;
    if(users.find(username) == users.end())
    {
        cout << "user ready to add." << endl;				
        struct DBUserInfo *temp_user = new DBUserInfo(username, password);
        users.insert(pair<std::string, struct DBUserInfo *>(username, temp_user));
        string update = "INSERT DATA {<" + username + "> <has_password> \"" + password + "\".}";
        update_sys_db(update);	
        result = true;
    }
    pthread_rwlock_unlock(&users_map_lock);
    return result;
}

bool APIUtil::user_delete(string username, string password)
{
    pthread_rwlock_trywrlock(&users_map_lock);
    bool result = false;
    if(users.find(username) != users.end())
    {
        users.erase(username);
        string update = "DELETE where {<" + username + "> ?p ?o.}";
		update_sys_db(update);
        result = true;
    }
    pthread_rwlock_unlock(&users_map_lock);
    return result;
}

bool APIUtil::user_pwd_alert(string username, string password)
{
    pthread_rwlock_trywrlock(&users_map_lock);
    bool result = false;
    std::map<std::string, struct DBUserInfo *>::iterator iter;
			iter = users.find(username);
    if(iter != users.end())
    {
        iter->second->setPassword(password);
        string update = "DELETE WHERE {<" + username + "> <has_password> ?o.}";
        update_sys_db(update);
        string update2 = "INSERT DATA {<" + username + "> <has_password>  \"" + password + "\".}";
        update_sys_db(update2);
        result = true;
    }
    pthread_rwlock_unlock(&users_map_lock);
    return result;
}

int APIUtil::clear_user_privilege(string username)
{
    pthread_rwlock_tryrdlock(&users_map_lock);
	std::map<std::string, struct DBUserInfo *>::iterator it = users.find(username);
	string update="";
	if(it != users.end() && username != ROOT_USERNAME)
	{
		pthread_rwlock_unlock(&users_map_lock);
		update = "DELETE where {<" + username + "> <has_query_priv> ?x.}";
		pthread_rwlock_trywrlock(&(it->second->query_priv_set_lock));
		it->second->query_priv.clear();
		pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
				
		
		update_sys_db(update);
		update = "DELETE where {<" + username + "> <has_load_priv> ?x.}";
		update_sys_db(update);
		pthread_rwlock_trywrlock(&(it->second->load_priv_set_lock));
		it->second->load_priv.clear();
		pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
		update = "DELETE where {<" + username + "> <has_unload_priv> ?x.}";
		update_sys_db(update);
		pthread_rwlock_trywrlock(&(it->second->unload_priv_set_lock));
		it->second->unload_priv.clear();
		pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
		update = "DELETE where {<" + username + "> <has_update_priv> ?x.}";
		update_sys_db(update);	
		pthread_rwlock_trywrlock(&(it->second->update_priv_set_lock));
		it->second->update_priv.clear();
		pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
		update = "DELETE where {<" + username + "> <has_backup_priv> ?x.}";
		update_sys_db(update);
		pthread_rwlock_trywrlock(&(it->second->backup_priv_set_lock));
		it->second->backup_priv.clear();
		pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
		update = "DELETE where {<" + username + "> <has_restore_priv> ?x.}";
		update_sys_db(update);
		pthread_rwlock_trywrlock(&(it->second->restore_priv_set_lock));
		it->second->restore_priv.clear();
		pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
		update = "DELETE where {<" + username + "> <has_export_priv> ?x.}";
		update_sys_db(update);
		pthread_rwlock_trywrlock(&(it->second->export_priv_set_lock));
		it->second->export_priv.clear();
		pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
		refresh_sys_db();
	
		pthread_rwlock_unlock(&users_map_lock);
		return 1;

	    	
	}
	else
	{
		pthread_rwlock_unlock(&users_map_lock);
        return -1;
	}
}

// ifstream APIUtil::Seek_to_line(ifstream & in, int line)
// {
//     int i;
// 	char buf[1024];
// 	in.seekg(0, ios::beg);
// 	for(i=1; i< line; i++)
// 	{
// 		in.getline(buf, sizeof(buf));
// 	}
// 	return in;
// }

bool APIUtil::del_privilege(const std::string& username, const std::string& type, const std::string& db_name)
{
    pthread_rwlock_tryrdlock(&users_map_lock);
	std::map<std::string, struct DBUserInfo *>::iterator it = users.find(username);
	if(it != users.end() && username != ROOT_USERNAME)
	{
		pthread_rwlock_unlock(&users_map_lock);
		if(type == "query" && it->second->query_priv.find(db_name) != it->second->query_priv.end())
		{
			pthread_rwlock_trywrlock(&(it->second->query_priv_set_lock));
			it->second->query_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_query_priv> <" + db_name + ">.}";
			update_sys_db(update);
			
			pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
			return 1;
		}
		else if(type == "update" && it->second->update_priv.find(db_name) != it->second->update_priv.end())
		{
			pthread_rwlock_trywrlock(&(it->second->update_priv_set_lock));
			it->second->update_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_update_priv> <" + db_name + ">.}";
			update_sys_db(update);
			
			pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
			return 1;
		}

		else if(type == "load" && it->second->load_priv.find(db_name) != it->second->load_priv.end())
		{
			pthread_rwlock_trywrlock(&(it->second->load_priv_set_lock));
			it->second->load_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_load_priv> <" + db_name + ">.}";
			update_sys_db(update);
			
			pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
			return 1;
		}
		else if(type == "unload" && it->second->unload_priv.find(db_name) != it->second->unload_priv.end())
		{
			pthread_rwlock_trywrlock(&(it->second->unload_priv_set_lock));
			it->second->unload_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_load_priv> <" + db_name + ">.}";
			update_sys_db(update);
			
			pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
			return 1;
		}
		else if(type == "backup" && it->second->backup_priv.find(db_name) != it->second->backup_priv.end())
		{
			pthread_rwlock_trywrlock(&(it->second->backup_priv_set_lock));
			it->second->backup_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_backup_priv> <" + db_name + ">.}";
			update_sys_db(update);
			
			pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
			return 1;
		}
		else if(type == "restore" && it->second->restore_priv.find(db_name) != it->second->restore_priv.end())
		{
			pthread_rwlock_trywrlock(&(it->second->restore_priv_set_lock));
			it->second->restore_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_restore_priv> <" + db_name + ">.}";
			update_sys_db(update);
			
			pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
			return 1;
		}
		else if(type == "export" && it->second->export_priv.find(db_name) != it->second->export_priv.end())
		{
			pthread_rwlock_trywrlock(&(it->second->export_priv_set_lock));
			it->second->export_priv.erase(db_name);
			string update = "DELETE DATA {<" + username + "> <has_export_priv> <" + db_name + ">.}";
			update_sys_db(update);
			
			pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
			return 1;
		}
	}
	pthread_rwlock_unlock(&users_map_lock);
	return 0;
}

bool APIUtil::check_privilege(const std::string& username, const std::string& type, const std::string& db_name)
{
	if(db_name == SYSTEM_DB_NAME)
		return 0;

	if(username == ROOT_USERNAME)
		return 1;
    if (type == "login" || type == "testConnect" || type == "getCoreVersion" 
        || type == "funquery" || type == "funcudb" || type == "funreview"
        || type == "check")
    {
        return 1;
    }
    
	pthread_rwlock_rdlock(&users_map_lock);
	std::map<std::string, struct DBUserInfo *>::iterator it = users.find(username);
	//pthread_rwlock_unlock(&users_map_lock);
	if(type == "query" || type == "show" || type == "monitor")
	{
		pthread_rwlock_rdlock(&(it->second->query_priv_set_lock));
		if(it->second->query_priv.find(db_name) != it->second->query_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);	
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->query_priv_set_lock));
	}
	else if(type == "update" || type == "batchInsert" || type == "batchRemove" 
        || type == "begin" || type == "tquery" || type == "commit" || type == "rollback")
	{
		pthread_rwlock_rdlock(&(it->second->update_priv_set_lock));
		if(it->second->update_priv.find(db_name) != it->second->update_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);	
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->update_priv_set_lock));
	}
	else if(type == "load")
	{
		pthread_rwlock_rdlock(&(it->second->load_priv_set_lock));
		if(it->second->load_priv.find(db_name) != it->second->load_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->load_priv_set_lock));
	}
	else if(type == "unload")
	{
		pthread_rwlock_rdlock(&(it->second->unload_priv_set_lock));
		if(it->second->unload_priv.find(db_name) != it->second->unload_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->unload_priv_set_lock));
	}
	else if(type == "restore")
	{
		pthread_rwlock_rdlock(&(it->second->restore_priv_set_lock));
		if(it->second->restore_priv.find(db_name) != it->second->restore_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->restore_priv_set_lock));
	}
	else if(type == "backup")
	{
		pthread_rwlock_rdlock(&(it->second->backup_priv_set_lock));
		if(it->second->backup_priv.find(db_name) != it->second->backup_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->backup_priv_set_lock));
	}
	else if(type == "export")
	{
		pthread_rwlock_rdlock(&(it->second->export_priv_set_lock));
		if(it->second->export_priv.find(db_name) != it->second->export_priv.end())
		{
			pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
			pthread_rwlock_unlock(&users_map_lock);
			return 1;
		}
		pthread_rwlock_unlock(&(it->second->export_priv_set_lock));
	}
	pthread_rwlock_unlock(&users_map_lock);
	return 0;
}

void APIUtil::get_user_info(vector<struct DBUserInfo *> *_users)
{
    pthread_rwlock_tryrdlock(&users_map_lock);
    if(!users.empty())
    {
        std::map<std::string, struct DBUserInfo *>::iterator iter;
        for (iter = users.begin(); iter != users.end(); iter++)
        {
            _users->push_back(iter->second);
        }
    }
    pthread_rwlock_unlock(&users_map_lock);
}

void APIUtil::get_access_log(const string &date, int &page_no, int &page_size, struct DBAccessLogs *dbAccessLogs)
{
    pthread_rwlock_rdlock(&access_log_lock);
    int totalSize = 0;
    int totalPage = 0;
    string accessLog = APIUtil::access_log_path + date + ".log";
    if(Util::file_exist(accessLog))
    {
        ifstream in;
        string line;
        in.open(accessLog.c_str(), ios::in);
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
            totalSize++;
        }
        in.close();
        totalPage = (totalSize/page_size) + (totalSize%page_size == 0 ? 0 : 1);
        if (page_no > totalPage)
        {
            pthread_rwlock_unlock(&access_log_lock);
            throw runtime_error("page_no more then max_page_no " + to_string(totalPage));
        }
        startLine = totalSize - page_size*page_no + 1;
        endLine = totalSize - page_size*(page_no - 1) + 1;
        if (startLine < 1)
        {
            startLine = 1;
        }
        // seek to start line;
        in.open(accessLog.c_str(), ios::in);
        int i_temp;
        char buf_temp[1024];
        in.seekg(0,ios::beg);
        for (i_temp = 1;i_temp<startLine;i_temp++)
        {
            in.getline(buf_temp, sizeof(buf_temp));
        }
        //APIUtil::Seek_to_line(in, startLine);
        vector<std::string> lines;
        while (startLine < endLine && getline(in, line, '\n')) {
            lines.push_back(line);
            startLine++;
        }
        in.close();
        pthread_rwlock_unlock(&access_log_lock);
        size_t count;
        count =  lines.size();			
        // cout<<"access log count : "<<count<<endl;
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
    dbAccessLogs->setTotalSize(totalSize);
    dbAccessLogs->setTotalPage(totalPage);
}

void APIUtil::write_access_log(string operation, string remoteIP, int statusCode, string statusMsg)
{
    string iplog_name = Util::get_date_day();
    string iplogfile = access_log_path + iplog_name + ".log";
    if (Util::file_exist(iplogfile) == false)
    {
        Util::formatPrint("ip access log file is not exist, now create it.");
        Util::create_file(iplogfile);
    }
    // std::cout << "accesslog: " << iplogfile << endl;
    FILE *ip_logfp = fopen(iplogfile.c_str(), "a");
    if (ip_logfp == NULL)
    {
        Util::formatPrint("open ip log error.","ERROR");
        return;
    }
    // Another way to locka many: lock(lk1, lk2...)
    pthread_rwlock_wrlock(&access_log_lock);
    // build json
    string createTime = Util::get_date_time();
    struct DBAccessLogInfo *dbAccessLogInfo = new DBAccessLogInfo(remoteIP, operation, statusCode, statusMsg, createTime);
    
    string _info = dbAccessLogInfo->toJSON();
    _info.push_back(',');
    _info.push_back('\n');
    fprintf(ip_logfp, "%s", _info.c_str());

    Util::Csync(ip_logfp);
    // long logSize = ftell(ip_logfp);
    fclose(ip_logfp);
    // std::cout << "logSize:" << logSize << endl;
    delete dbAccessLogInfo;
    pthread_rwlock_unlock(&access_log_lock);
}

void APIUtil::get_query_log(const string &date, int &page_no, int &page_size, struct DBQueryLogs *dbQueryLogs)
{
    int rt = pthread_rwlock_rdlock(&query_log_lock);
    Util::formatPrint("query_log_lock lock:" + to_string(rt));
    int totalSize = 0;
    int totalPage = 0;
    string queryLog = APIUtil::query_log_path + date + ".log";
    if(Util::file_exist(queryLog))
    {
        ifstream in;
        string line;
        in.open(queryLog.c_str(), ios::in);
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
            totalSize++;
        }
        in.close();
        totalPage = (totalSize/page_size) + (totalSize%page_size == 0 ? 0 : 1);
        if (page_no > totalPage)
        {
            rt = pthread_rwlock_unlock(&query_log_lock);
            Util::formatPrint("query_log_lock unlock1:" + to_string(rt));
            throw runtime_error("page_no more then max_page_no " + to_string(totalPage));
        }
        startLine = totalSize - page_size*page_no + 1;
        endLine = totalSize - page_size*(page_no - 1) + 1;
        if (startLine < 1)
        {
            startLine = 1;
        }
        // seek to start line;
        in.open(queryLog.c_str(), ios::in);
        int i_temp;
        char buf_temp[1024];
        in.seekg(0, ios::beg);
        for (i_temp = 1; i_temp < startLine; i_temp++)
        {
            in.getline(buf_temp, sizeof(buf_temp));
        }
        //APIUtil::Seek_to_line(in, startLine);
        vector<std::string> lines;
        while (startLine < endLine && getline(in, line, '\n')) {
            lines.push_back(line);
            startLine++;
        }
        in.close();
        rt = pthread_rwlock_unlock(&query_log_lock);
        Util::formatPrint("query_log_lock unlock2:" + to_string(rt));
        size_t count;
        count =  lines.size();
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
    dbQueryLogs->setTotalSize(totalSize);
    dbQueryLogs->setTotalPage(totalPage);
}

void APIUtil::write_query_log(struct DBQueryLogInfo *queryLogInfo)
{
    std::string queyrlog_name = Util::get_date_day();
    std::string querylog_file = query_log_path + queyrlog_name + ".log";
    if (Util::file_exist(querylog_file) == false)
    {
        Util::formatPrint("qeury log file is not exist, now create it.");
        Util::create_file(querylog_file);
    }
    // std::cout << "querylog: " << querylog_file << endl;
    FILE *querylog_fp = fopen(querylog_file.c_str(), "a");
    if (querylog_fp == NULL)
    {
        Util::formatPrint("open query log error.","ERROR");
        return;
    }
    std::string _info = queryLogInfo->toJSON();

    // Another way to locka many: lock(lk1, lk2...)
    pthread_rwlock_wrlock(&query_log_lock);
    _info.push_back(',');
    _info.push_back('\n');
    std::fprintf(querylog_fp, "%s", _info.c_str());

    Util::Csync(querylog_fp);
    // long logSize = ftell(querylog_fp);
    std::fclose(querylog_fp);
    // std::cout << "logSize: " << logSize << std::endl;
    pthread_rwlock_unlock(&query_log_lock);
}

void APIUtil::init_transactionlog()
{
    pthread_rwlock_wrlock(&transactionlog_lock);
    if (boost::filesystem::exists(TRANSACTION_LOG_PATH)) {
        Util::formatPrint("transaction log has been created.");
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
    struct TransactionLogInfo *logInfo = new TransactionLogInfo(db_name, TID, user, state, begin_time, end_time);
    string rec = logInfo->toJSON();
    rec.push_back('\n');
    fputs(rec.c_str(), fp);
    fclose(fp);
    delete logInfo;
    pthread_rwlock_unlock(&transactionlog_lock);
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
            continue;
        }
        if (!logInfo->getState().empty() && !logInfo->getEndTime().empty()) {
            logInfo->setState(state);
            logInfo->setEndTime(end_time);
            string line = logInfo->toJSON();
            line.push_back('\n');
            fputs(line.c_str(), fp1);
        }
        else 
        {
            fputs(readBuffer, fp1);
            Util::formatPrint("Transaction log corrupted, please initilize it!", "ERROR");
            ret = 1;
        }
        delete logInfo;
    }
    fclose(fp);
    fclose(fp1);
    string cmd = "rm ";
    cmd += TRANSACTION_LOG_PATH;
    system(cmd.c_str());
    cmd = "mv ";
    cmd += TRANSACTION_LOG_TEMP_PATH;
    cmd += ' ';
    cmd += TRANSACTION_LOG_PATH;
    system(cmd.c_str());
    pthread_rwlock_unlock(&transactionlog_lock);
    return ret;
}

void APIUtil::get_transactionlog(int &page_no, int &page_size, struct TransactionLogs *dbQueryLogs)
{
    int totalSize = 0;
    int totalPage = 0;
    if (Util::file_exist(TRANSACTION_LOG_PATH))
    {
        pthread_rwlock_rdlock(&transactionlog_lock);
        ifstream in;
        in.open(TRANSACTION_LOG_PATH, ios::in);
        string line;
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
            totalSize++;
        }
        in.close();
        if (totalSize > 0)
        {
            totalPage = (totalSize/page_size) + (totalSize%page_size == 0 ? 0 : 1);
            if (page_no > totalPage)
            {
                pthread_rwlock_unlock(&transactionlog_lock);
                throw runtime_error("page_no more then max_page_no " + to_string(totalPage));
            }
            startLine = totalSize - page_size*page_no + 1;
            endLine = totalSize - page_size*(page_no - 1) + 1;
            if (startLine < 1)
            {
                startLine = 1;
            }
            // seek to start line;
            in.open(TRANSACTION_LOG_PATH, ios::in);
            int i_temp;
            char buf_temp[1024];
            in.seekg(0, ios::beg);
            for (i_temp = 1; i_temp < startLine; i_temp++)
            {
                in.getline(buf_temp, sizeof(buf_temp));
            }
            vector<std::string> lines;
            while (startLine < endLine && getline(in, line, '\n')) {
                lines.push_back(line);
                startLine++;
            }
            in.close();
            pthread_rwlock_unlock(&transactionlog_lock);
            size_t count;
            count =  lines.size();
            for (size_t i = 0; i < count; i++)
            {
                line = lines[count - i - 1];
                dbQueryLogs->addTransactionLogInfo(line);
            }
        }
    }
    dbQueryLogs->setTotalPage(totalPage);
    dbQueryLogs->setTotalSize(totalSize);
}

void APIUtil::abort_transactionlog(long end_time)
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
    }
    fclose(fp);
    fclose(fp1);
    string cmd = "rm ";
    cmd += TRANSACTION_LOG_PATH;
    system(cmd.c_str());
    cmd = "mv ";
    cmd += TRANSACTION_LOG_TEMP_PATH;
    cmd += ' ';
    cmd += TRANSACTION_LOG_PATH;
    system(cmd.c_str());
    pthread_rwlock_unlock(&transactionlog_lock);
}

std::string APIUtil::fun_cppcheck(std::string username, struct PFNInfo *fun_info)
{
    string report_detail = "";
    string file_name = fun_info->getFunName();
    string report_path = APIUtil::pfn_file_path + username + "/report.txt";
    string check_file_path = APIUtil::pfn_file_path + username + "/" + file_name + "_temp.cpp";
    string cppcheck = "cppcheck -j 10 --force suppress=missingIncludeSystem --template=\"[line:{line}]:({severity}) {message}\" --output-file="+report_path+" "+check_file_path;     
    string report_delete = "rm -f " + report_path;
    string cppcheckFile_delete = "rm -f " + check_file_path;
    string lookAtfile = "cat " + check_file_path;
    ofstream fout(check_file_path.c_str());
    if (fout.is_open())
    {
        string content;
        content = APIUtil::fun_build_source_data(fun_info, false);
        fout << content;
        fout.close();
    } else {
        throw new runtime_error("cannot write to file " + check_file_path);
    }
    system(lookAtfile.c_str());
    system(cppcheck.c_str());
    ifstream cppcheck_fin(report_path.c_str());
    if(cppcheck_fin.is_open()){
        string data = "";
        while(getline(cppcheck_fin,data)){
            report_detail += data;
            report_detail +='\r\n';
        }
    }
    cppcheck_fin.close();
    system(report_delete.c_str());
    system(cppcheckFile_delete.c_str());
    return report_detail;
}

void APIUtil::fun_query(const string &fun_name, const string &fun_status, const string &username, struct PFNInfos *pfn_infos)
{
    string json_file_path = APIUtil::pfn_file_path + username + "/data.json";
    if (Util::file_exist(json_file_path) == false)
    {
        return;
    }
    ifstream in;
    string line;
    string temp_str;
    in.open(json_file_path.c_str(), ios::in);
    PFNInfo *temp_ptr;
    while (getline(in, line))
    {
        temp_ptr = new PFNInfo(line);
        if (!fun_name.empty() && temp_ptr->getFunName().find(fun_name) == string::npos)
        {
            continue;
        }
        if (!fun_status.empty() && temp_ptr->getFunStatus() != fun_status)
        {
            continue;
        }
        pfn_infos->addPFNInfo(line);
        delete temp_ptr;
    }
    in.close();
}

void APIUtil::fun_create(const string &username, struct PFNInfo *pfn_info)
{
    string content;
    string file_name;
    file_name = pfn_info->getFunName();
    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    string file_dir = APIUtil::pfn_file_path + username;
    if (!Util::dir_exist(file_dir))
    {
        //Util::create_dir(file_dir);
        string mkdir_cmd = "mkdir "+ APIUtil::pfn_file_path;
        system(mkdir_cmd.c_str());
        mkdir_cmd = "mkdir " + file_dir;
        system(mkdir_cmd.c_str());
    }
    string file_path = file_dir + "/" + file_name + ".cpp";
    std::cout << "file_path: " << file_path << endl;
    if (Util::file_exist(file_path))
    {
        throw runtime_error("function name " + pfn_info->getFunName() + " already exists");
    }
    // cppcheck start
    string report_detail = "";
    // report_detail = fun_cppcheck(username, pfn_infos);
    // cppcheck end
    
    // save fun in file
    if(report_detail.size() == 0){
        ofstream fout(file_path.c_str());
        cout << "fout ok" <<endl;
        if (fout)
        {
            cout << "begin get source data " <<endl;
            content = APIUtil::fun_build_source_data(pfn_info, true);
            fout << content;
            fout.close();
        } else {
            cout << "open file error "<<endl;
            throw new runtime_error("cannot write to file " + file_path);
        }
        // save method info to json file
        pfn_info->setFunStatus("1");
        APIUtil::fun_write_json_file(username, pfn_info, "1");
    }
    else
    {
        throw runtime_error(report_detail);    
    }
}

void APIUtil::fun_update(const std::string &username, struct PFNInfo *pfn_infos)
{
    string content;
    string file_name;
    file_name = pfn_infos->getFunName();

    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    string file_path = APIUtil::pfn_file_path + username + "/" + file_name + ".cpp";
    if (Util::file_exist(file_path) == false)
    {
        throw runtime_error("function name " + pfn_infos->getFunName() + " not exists");
    }
    
    // cpp check start
    string report_detail = "";
    // report_detail = fun_cppcheck(username, pfn_infos);
    // cpp check end
    
    // save fun in file
    if(report_detail.size() == 0){
        content = APIUtil::fun_build_source_data(pfn_infos, true);
        cout << "fun_build_source_data success "<<endl;
        ofstream fout(file_path.c_str());
        if (fout.is_open())
        {
            fout << content;
            fout.close();
        }
        // save method info to json file
        pfn_infos->setFunStatus("1");
        APIUtil::fun_write_json_file(username, pfn_infos, "2");
        cout << "APIUtil::fun_write_json_file success "<<endl;
    }
    else
    {
        throw runtime_error(report_detail);
    }
}

void APIUtil::fun_delete(const std::string &username, struct PFNInfo *pfn_infos)
{
     APIUtil::fun_write_json_file(username, pfn_infos, "3");
}

string APIUtil::fun_build(const std::string &username, const std::string fun_name)
{
    string file_name = fun_name;
    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    string sourceFile = APIUtil::pfn_file_path + username + "/" + file_name + ".cpp";
    if (!Util::file_exist(sourceFile))
    {
        throw new runtime_error("function file is not exist");
    }
    // get function info from json file
    PFNInfo *fun_info = new PFNInfo();
    string json_str;
    APIUtil::fun_parse_from_name(username, fun_name, fun_info);

    //create a temp file
    string last_time = Util::get_timestamp();
    string md5str = Util::md5(last_time);
    string targetDir = APIUtil::pfn_lib_path + username;
    if (!Util::dir_exist(targetDir))
    {
        Util::create_dir(targetDir);
    }
    targetDir = APIUtil::pfn_lib_path + username + "/.temp";
    if (!Util::dir_exist(targetDir))
    {
        Util::create_dir(targetDir);
    }
    string targetFile = targetDir + "/lib" + file_name + md5str + ".so";
    string logFile = APIUtil::pfn_file_path + username + "/error.out";
    string cmd = "rm -f " + targetFile;
    system(cmd.c_str());
    cmd = "g++ -std=c++11 -fPIC -shared -I./Database -lgcsr -lgcsrutil -Llib -o" + targetFile + " " + sourceFile + " 2>" + logFile;
    int status;
    status = system(cmd.c_str());
    string error_msg = "";
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
        // update function status to 2
        fun_info->setFunStatus("2");
        //delete old so
        string usingPath = APIUtil::pfn_lib_path + username ;
        string rmOldSo = "rm -f "+ usingPath +"/lib" + file_name + "*.so";
        system(rmOldSo.c_str());
        //mv the new into using Path
        string mvCmd = "mv " +  targetFile + " " + usingPath +"/";
        system(mvCmd.c_str());
    }
    else if (Util::file_exist(logFile))
    {
        // update function status to 3
        fun_info->setFunStatus("3");
        ifstream errorFile(logFile.c_str());
        string line;
        stringstream strStream;
        while (getline(errorFile, line, '\n'))
        {
            strStream << line;
        }
        error_msg = strStream.str();
    }
    else
    {
        // update function status to 3
        fun_info->setFunStatus("3");
        error_msg = "unknown error";
    }

    // update function status and last_time
    fun_info->setLastTime(last_time);
    APIUtil::fun_write_json_file(username, fun_info, "2");
    // delete
    delete fun_info;
    cmd = "rm -f " + logFile;
    system(cmd.c_str());
    // has error_msg
    if (error_msg.size() > 0)
    {
        return error_msg;
    }
    return "";
}

void APIUtil::fun_review(const std::string &username, struct PFNInfo *pfn_info)
{
    string content = APIUtil::fun_build_source_data(pfn_info, true);
    pfn_info->setFunBody(content);
}

std::string APIUtil::fun_build_source_data(struct PFNInfo * fun_info, bool has_header)
{
    const string fun_name = fun_info->getFunName();
    const string fun_args = fun_info->getFunArgs();
    const string fun_body = Util::urlDecode(fun_info->getFunBody());
    const string fun_subs = Util::urlDecode(fun_info->getFunSubs());
    stringstream _buf;
    if (has_header)
    {
        cout << APIUtil::pfn_include_header << endl;
        _buf << APIUtil::pfn_include_header;
    }
    
    if (fun_subs.length() > 0)
    {
        _buf << fun_subs << '\n';
    }

    _buf << "extern \"C\" string " + fun_name;
    cout << "fun_args "<<fun_args <<endl;
    if (fun_args == "1") // int uid, int vid, bool directed, vector<int> pred_set
    {
        _buf << "(std::vector<int> iri_set, bool directed, std::vector<int> pred_set, CSRUtil* csrUtil)\n";
    }
    else if (fun_args == "2") // int uid, int vid, bool directed, int k, vector<int> pred_set
    {
        _buf << "(std::vector<int> iri_set, bool directed, int k, std::vector<int> pred_set, CSRUtil* csrUtil)\n";
    }
    else
    {
        throw std::runtime_error("the fun_args " + fun_args + " not match: {\"1\", \"2\"}");
    }
    _buf << fun_body;
    return _buf.str();
}

void APIUtil::fun_write_json_file(const std::string& username, struct PFNInfo *fun_info, std::string operation)
{
    string json_file_path = APIUtil::pfn_file_path + username + "/data.json";
    std::string fun_name = fun_info->getFunName(); 
    std::string json_str = fun_info->toJSON();
    if (operation == "1") // create
    {

        if (!Util::file_exist(json_file_path))
        {
            string json_file_dir = APIUtil::pfn_file_path + username;
            if (!Util::dir_exist(json_file_dir))
            {
                Util::create_dir(json_file_dir);
            }
            Util::create_file(json_file_path);
        }
        FILE *fp = fopen(json_file_path.c_str(), "a");
        if (fp == NULL)
        {
            throw runtime_error("open function json file error.");
        }
        json_str.push_back('\n');
        fprintf(fp, "%s", json_str.c_str());
        fclose(fp);
    }
    else if (operation == "2" || operation == "3") // update or remove
    {
        ifstream in;
        stringstream _buf;
        string line;
        string cmd;
        string temp_name;
        in.open(json_file_path.c_str(), ios::in);
        if (!in.is_open())
        {
            throw runtime_error("open function json file error.");
        }
        PFNInfo *fun_info_tmp;
        while (getline(in, line))
        {
            fun_info_tmp = new PFNInfo(line);
            temp_name = fun_info_tmp->getFunName();
            if (temp_name == fun_name)
            {
                if (operation == "2") // update
                {
                    _buf << json_str << '\n';
                }
                else // remove
                {
                    // rename cpp file and remove so file
                    string file_name = fun_name;
                    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
                    string sourcePath = APIUtil::pfn_file_path + username + "/" + file_name + ".cpp";
                    // string backPath = sourcePath + "." + Util::getTimeString2();
                    string libPath = APIUtil::pfn_lib_path + username + "/lib" + file_name + "*.so";
                    cmd = "rm -f " + sourcePath;
                    system(cmd.c_str());
                    cmd = "rm -f " + libPath;
                    system(cmd.c_str());
                }
            }
            else
            {
                _buf << line << '\n';
            }
            delete fun_info_tmp;
        }
        in.close();
        line = _buf.str();
        string temp_path = APIUtil::pfn_file_path + username + "/temp.json";
        string back_path = APIUtil::pfn_file_path + username + "/back.json";
        ofstream out(temp_path.c_str());
        if (!out.is_open())
        {
            throw runtime_error("open function json temp file error.");
        }
        out << line;
        out.close();
        // mv fun/username/data.json fun/username/back.json
        cmd = "mv -f " + json_file_path + " " + back_path;
        int status;
        status = system(cmd.c_str());
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        {
            // mv fun/username/temp.json fun/username/data.json
            cmd = "mv -f " + temp_path + " " + json_file_path;
            status = system(cmd.c_str());
            cout << cmd <<endl;
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            {
                // remove old json file
                cmd = "rm -f " + back_path;
                system(cmd.c_str());
                cout << cmd <<endl;
            }
            else // recover back.json to data.json
            {
                cmd = "mv -f " + back_path + " " + json_file_path;
                system(cmd.c_str());
                throw runtime_error("save function info to json file error, status code:" + status);
            }
        }
        else
        {
            throw runtime_error("save function info to json file error, status code:" + status);
        }
    }
    else
    {
        throw runtime_error("save function info to json file error, no match operation:" + operation);
    }
}

void APIUtil::fun_parse_from_name(const std::string& username, const std::string& fun_name, struct PFNInfo *fun_info)
{
    string json_file_path = APIUtil::pfn_file_path + username + "/data.json";
    ifstream in;
    in.open(json_file_path.c_str(), ios::in);
    if (!in.is_open())
    {
        throw runtime_error("open function json file error.");
    }
    string line;
    bool isMatch;
    string temp_name;
    PFNInfo* temp_ptr;
    isMatch = false;
    while (getline(in, line))
    {
        temp_ptr = new PFNInfo(line);
        temp_name = temp_ptr->getFunName();
        if (temp_name == fun_name)
        {
            isMatch = true;
            break;
        }
        delete temp_ptr;
    }
    in.close();
    if (isMatch)
    {
        fun_info->copyFrom(*temp_ptr);
        delete temp_ptr;
    }
    else
    {
        delete temp_ptr;
        throw runtime_error("function " + fun_name + " not exists");
    }
}

string APIUtil::get_system_path()
{
    return system_path;
}

string APIUtil::get_backup_path()
{
    return backup_path;
}

string APIUtil::get_Db_path()
{
    return DB_path;
}

unsigned int APIUtil::get_max_output_size()
{
    return max_output_size;
}

string APIUtil::get_root_username()
{
    return root_username;
}

string APIUtil::get_system_username()
{
    return system_username;
}

unsigned int APIUtil::get_connection_num()
{
    return connection_num;
}