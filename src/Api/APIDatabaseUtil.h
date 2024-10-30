/*
 * @Author: wangjian
 * @Date: 2021-12-20 16:35:18
 * @LastEditTime: 2023-02-09 13:38:24
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: api util
 * @FilePath: /gstore/GRPC/grpcUtil.h
 */
#pragma once
#include "APIUtilDefined.h"

using namespace std;
using namespace rapidjson;

enum DatabaseStatus 
{
    NORMAL = 0,
    BUILDING = 1,
    AREADY_BUILT = 2,
    LOADING = 3,
    LOADED = 4,
    UNLOADED = 5
};

const std::map<DatabaseStatus, std::string> DatabseStatusMap
{
    {NORMAL,       "unknown"},
    {BUILDING,     "building"},
    {AREADY_BUILT, "already_built"},
    {LOADING,      "loading"},
    {LOADED,       "loaded"},
    {UNLOADED,     "unloaded"}
};

struct DatabaseInfo
{
private:
    std::string db_name;    //! the name of database
    std::string creator;    //! the creator of database
    std::string build_time; //! the built time of database;
    DatabaseStatus status;
    shared_ptr<Database> db_ptr;

public:
    pthread_rwlock_t db_lock;

    DatabaseInfo()
    {
        db_ptr = nullptr;
        pthread_rwlock_init(&db_lock, NULL);
    }
    DatabaseInfo(string _name, string _creator, string _time, DatabaseStatus _status)
    {
        db_name = _name;
        creator = _creator;
        build_time = _time;
        status = _status;
        if (_status > DatabaseStatus::BUILDING)
            db_ptr = make_shared<Database>(db_name);
        else
            db_ptr = nullptr;
        pthread_rwlock_init(&db_lock, NULL);
    }
    ~DatabaseInfo()
    {
        db_ptr.reset();
        pthread_rwlock_destroy(&db_lock);
    }
    std::string getName()
    {
        return db_name;
    }
    std::string getCreator()
    {
        return creator;
    }
    void setCreator(string _creator)
    {
        creator = _creator;
    }
    std::string getTime()
    {
        return build_time;
    }
    void setTime(string _time)
    {
        build_time = _time;
    }
    DatabaseStatus getStatus()
    {
        return status;
    }
    void setStatus(DatabaseStatus _status)
    {
        status = _status;
    }
    std::string getStatusStr()
    {
        auto it = DatabseStatusMap.find(status);
        if (it != DatabseStatusMap.end())
        {
            return it->second;
        }
        else
        {
            return "";
        }
    }
    shared_ptr<Database>& getDatabase()
    {
        return db_ptr;
    }
    void initDatabase()
    {
        if (db_ptr == nullptr) {
            db_ptr = make_shared<Database>(db_name);
        }
    }
    bool unloadDatabase()
    {
        if (db_ptr != nullptr) {
            db_ptr.reset();
            status = DatabaseStatus::AREADY_BUILT;
            db_ptr = make_shared<Database>(db_name);
            return true;
        }
        return false;
    }
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value doc(rapidjson::kObjectType);
        // doc.SetObject();
        doc.AddMember("database", rapidjson::Value().SetString(db_name.c_str(), allocator).Move(), allocator);
        doc.AddMember("creator", rapidjson::Value().SetString(creator.c_str(), allocator).Move(), allocator);
        doc.AddMember("built_time", rapidjson::Value().SetString(build_time.c_str(), allocator).Move(), allocator);
        doc.AddMember("status", rapidjson::Value().SetString(getStatusStr().c_str(), allocator).Move(), allocator);
        return doc;
    }
};

struct IpInfo
{
private:
    unsigned int accessNum = 0;
    unsigned int successNum = 0;
    unsigned int errorNum = 0;
    std::string ip;

public:
    IpInfo()
    {
    }
    IpInfo(string name)
    {
        ip = name;
        accessNum = 0;
        successNum = 0;
        errorNum = 0;
    }
    unsigned int getAccessNum()
    {
        return accessNum;
    }
    unsigned int getSuccessNum()
    {
        return successNum;
    }
    unsigned int getErrorNum()
    {
        return errorNum;
    }
    string getName()
    {
        return ip;
    }
    void setAccessNum(unsigned int value)
    {
        accessNum = value;
    }
    void addAccessNum()
    {
        accessNum += 1;
    }
    void setSuccessNum(unsigned int value)
    {
        successNum = value;
    }
    void addSuccessNum() 
    {
        successNum += 1;
    }
    void setErrorNum(unsigned int value)
    {
        errorNum = value;
    }
    void addErrorNum() {
        errorNum += 1;
    }
    void setName(string value)
    {
        ip = value;
    }
};