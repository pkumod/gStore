#pragma once
#include "APIUtilDefined.h"

using namespace std;
using namespace rapidjson;

struct DBUserInfo
{
private:
    std::string username;
    std::string password;

public:
    std::set<std::string> query_priv;//! the query privilege
    std::set<std::string> update_priv;//! the update privilege
    std::set<std::string> load_priv;//! the load privilege
    std::set<std::string> unload_priv;//! the unload privilege
    std::set<std::string> backup_priv;//! the backup privilege
    std::set<std::string> restore_priv;//! the restore privilege
    std::set<std::string> export_priv;//! the export privilege

    pthread_rwlock_t query_priv_set_lock;
    pthread_rwlock_t update_priv_set_lock;
    pthread_rwlock_t load_priv_set_lock;
    pthread_rwlock_t unload_priv_set_lock;
    pthread_rwlock_t backup_priv_set_lock;
    pthread_rwlock_t restore_priv_set_lock;
    pthread_rwlock_t export_priv_set_lock;
    DBUserInfo() {}
    DBUserInfo(std::string _username, std::string _password)
    {
        username = _username;
        password = _password;
        pthread_rwlock_init(&query_priv_set_lock, NULL);
        pthread_rwlock_init(&update_priv_set_lock, NULL);
        pthread_rwlock_init(&load_priv_set_lock, NULL);
        pthread_rwlock_init(&unload_priv_set_lock, NULL);
        pthread_rwlock_init(&backup_priv_set_lock, NULL);
        pthread_rwlock_init(&restore_priv_set_lock, NULL);
        pthread_rwlock_init(&export_priv_set_lock, NULL);
    }
    ~DBUserInfo() {
        pthread_rwlock_destroy(&query_priv_set_lock);
        pthread_rwlock_destroy(&update_priv_set_lock);
        pthread_rwlock_destroy(&load_priv_set_lock);
        pthread_rwlock_destroy(&unload_priv_set_lock);
        pthread_rwlock_destroy(&backup_priv_set_lock);
        pthread_rwlock_destroy(&restore_priv_set_lock);
        pthread_rwlock_destroy(&export_priv_set_lock);
    }
    std::string getUsernname()
    {
        return username;
    }
    std::string getPassword()
    {
        return password;
    }
    void setPassword(std::string _password)
    {
        password = _password;
    }
    std::string getQuery(){
        std::string query_db;
        if(username == ROOT_USERNAME)
        {
            query_db = "all";
            return query_db;
        }
        std::set<std::string>::iterator it = query_priv.begin();
        while(it != query_priv.end())
        {
            query_db = query_db + *it + ",";
            ++it;
        }
        return query_db;
    }
    std::string getUpdate(){
        std::string update_db;
        if(username == ROOT_USERNAME)
        {
            update_db = "all";
            return update_db;
        }
        std::set<std::string>::iterator it = update_priv.begin();
        while(it != update_priv.end())
        {
            update_db = update_db + *it + ",";
            ++it;
        }
        return update_db;
    }
    std::string getLoad(){
        std::string load_db;
        if(username == ROOT_USERNAME)
        {
            load_db = "all";
            return load_db;
        }

        std::set<std::string>::iterator it = load_priv.begin();
        while(it != load_priv.end())
        {
            load_db = load_db + *it + ",";
            ++it;
        }
        return load_db;
    }
    std::string getUnload(){
        std::string unload_db;
        if(username == ROOT_USERNAME)
        {
            unload_db = "all";
            return unload_db;
        }

        std::set<std::string>::iterator it = unload_priv.begin();
        while(it != unload_priv.end())
        {
            unload_db = unload_db + *it + ",";
            ++it;
        }
        return unload_db;
    }
    std::string getRestore(){
        std::string restore_db;
        if(username == ROOT_USERNAME)
        {
            restore_db = "all";
            return restore_db;
        }
        std::set<std::string>::iterator it = restore_priv.begin();
        while(it != restore_priv.end())
        {
            restore_db = restore_db + *it + ",";
            ++it;
        }
        return restore_db;
    }
    std::string getBackup(){
        std::string backup_db;
        if(username == ROOT_USERNAME)
        {
            backup_db = "all";
            return backup_db;
        }
        std::set<std::string>::iterator it = backup_priv.begin();
        while(it != backup_priv.end())
        {
            backup_db = backup_db + *it + ",";
            ++it;
        }
        return backup_db;
    }
    std::string getExport(){
        std::string export_db;
        if(username == ROOT_USERNAME)
        {
            export_db = "all";
            return export_db;
        }
        std::set<std::string>::iterator it = export_priv.begin();
        while(it != export_priv.end())
        {
            export_db = export_db + *it + ",";
            ++it;
        }
        return export_db;
    }
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator) {
        rapidjson::Value doc(kObjectType);
        doc.AddMember("username", rapidjson::Value().SetString(this->username.c_str(),allocator).Move(), allocator);
        doc.AddMember("password", rapidjson::Value().SetString(this->password.c_str(),allocator).Move(), allocator);
        doc.AddMember("query_privilege", rapidjson::Value().SetString(this->getQuery().c_str(),allocator).Move(), allocator);
        doc.AddMember("update_privilege", rapidjson::Value().SetString(this->getUpdate().c_str(),allocator).Move(), allocator);
        doc.AddMember("load_privilege", rapidjson::Value().SetString(this->getLoad().c_str(),allocator).Move(), allocator);
        doc.AddMember("unload_privilege", rapidjson::Value().SetString(this->getUnload().c_str(),allocator).Move(), allocator);
        doc.AddMember("backup_privilege", rapidjson::Value().SetString(this->getBackup().c_str(),allocator).Move(), allocator);
        doc.AddMember("restore_privilege", rapidjson::Value().SetString(this->getRestore().c_str(),allocator).Move(), allocator);
        doc.AddMember("export_privilege", rapidjson::Value().SetString(this->getExport().c_str(),allocator).Move(), allocator);
        return doc;
    }
};