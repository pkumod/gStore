#pragma once
#include <string>
#include <vector>
#include <sys/file.h>
#include "nlohmann/json.hpp"
using namespace std;
using namespace nlohmann;

enum DatabaseStatus 
{
    RESTOREING = 0,
    BUILDING = 1,
    AREADY_BUILT = 2,
    LOADING = 3,
    LOADED = 4,
    UNLOADING = 5,
    BACKUPING = 6
};

enum DatabaseLock
{
    R = 0,
    W = 1
};

const std::map<DatabaseStatus, std::string> DatabseStatusMap
{
    {RESTOREING,   "restoreing"},
    {BUILDING,     "building"},
    {AREADY_BUILT, "already_built"},
    {LOADING,      "loading"},
    {LOADED,       "loaded"},
    {UNLOADING,    "unloading"},
    {BACKUPING,    "backuping"}
};

struct DatabaseInfo
{
private:
    std::string db_path;
    std::string db_name;    //! the name of database
    std::string creator;    //! the creator of database
    std::string build_time; //! the built time of database;
    DatabaseStatus status;
    shared_ptr<Database> db_ptr;

public:
    pthread_rwlock_t db_lock;
    std::atomic<int> lock_count;

    DatabaseInfo()
    {
        db_ptr = nullptr;
        lock_count = 0;
        pthread_rwlock_init(&db_lock, NULL);
    }
    DatabaseInfo(string _path, string _name, string _creator, string _time, DatabaseStatus _status)
    {
        db_path = _path;
        db_name = _name;
        creator = _creator;
        build_time = _time;
        status = _status;
        if (_status > DatabaseStatus::BUILDING)
            db_ptr = make_shared<Database>(db_name);
        else
            db_ptr = nullptr;
        lock_count = 0;
        pthread_rwlock_init(&db_lock, NULL);
    }
    ~DatabaseInfo()
    {
        db_ptr.reset();
        lock_count = 0;
        pthread_rwlock_destroy(&db_lock);
    }
    std::string getPath()
    {
        return db_path;
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
    void setDatabase(shared_ptr<Database> db){ db_ptr = db; }
    void initDatabase(bool schema_flag)
    {
        if (db_ptr == nullptr) {
            db_ptr = make_shared<Database>(db_name, schema_flag);
        }
    }
    
    void success(int64_t cost_time=0L) 
    {
        std::string file = db_path + "/success";
        FILE *fp = fopen(file.c_str(), "wb");
        fwrite(&cost_time, sizeof(int64_t), 1, fp);
        fclose(fp);
    }

    int64_t getCostTime()
    {
        std::string file = db_path + "/success";
        FILE *fp = fopen(file.c_str(), "rb");
        int64_t cost_time = 0;
        if (fp != nullptr) {
            fseek(fp, 0, SEEK_SET);
            fread(&cost_time, sizeof(int64_t), 1, fp);
            fclose(fp);
        }
        return cost_time;
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
    nlohmann::json toJSON()
    {
        nlohmann::json json = {
            {"database", db_name},
            {"creator", creator},
            {"built_time", build_time},
            {"status", getStatusStr()},
            {"cost_time", getCostTime()}
        };
        return json;
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