#pragma once
#include <string>
#include <vector>
#include <sys/file.h>
#include "nlohmann/json.hpp"
using namespace std;
using namespace nlohmann;

enum DatabaseStatus 
{
    RESTORING = 0,
    BUILDING = 1,
    LOADING = 2,
    UNLOADING = 3,
    BACKUPING = 4,
    INSERTING = 5,
    AREADY_BUILT = 6,
    LOADED = 7
};

enum DatabaseLock
{
    R = 0,
    W = 1
};

const std::map<DatabaseStatus, std::string> DatabseStatusMap
{
    {RESTORING,   "restoring"},
    {BUILDING,     "building"},
    {LOADING,      "loading"},
    {UNLOADING,    "unloading"},
    {BACKUPING,    "backuping"},
    {INSERTING,    "inserting"},
    {AREADY_BUILT, "already_built"},
    {LOADED,       "loaded"}
};

struct DatabaseInfo
{
private:
    std::string db_path;
    std::string db_name;    //! the name of database
    std::string creator;    //! the creator of database
    std::string build_time; //! the built time of database;
    DatabaseStatus status;
    bool loaded;            //! whether the database is loaded
    shared_ptr<Database> db_ptr;

public:
    pthread_rwlock_t db_lock;
    std::atomic<int> lock_count;

    DatabaseInfo()
    {
        db_ptr = nullptr;
        lock_count = 0;
        loaded = false;
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
        loaded = false;
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
        if (_status == DatabaseStatus::LOADED)
            loaded = true;
    }
    bool isLoaded()
    {
        return loaded;
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
    std::string getStatusDesc()
    {
        std::string statusStr;
        switch (status)
        {
        case RESTORING:
            statusStr = "restored";
            break;
        case BUILDING:
            statusStr = "built";
            break;
        case LOADING:
            statusStr = "loaded";
            break;
        case UNLOADING:
        case AREADY_BUILT:
            statusStr = "unloaded";
            break;
        case BACKUPING:
            statusStr = "backed up";
            break;
        case INSERTING:
            statusStr = "inserted";
            break;
        default:
            statusStr = "unknown";
            break;
        }
        return statusStr;
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
        status = DatabaseStatus::UNLOADING;
        if (db_ptr != nullptr) {
            db_ptr.reset();
            db_ptr = make_shared<Database>(db_name);
        }
        db_ptr->setLoadTxnFlag(false);
        status = DatabaseStatus::AREADY_BUILT;
        loaded = false;
        return true;
    }
    nlohmann::json toJSON()
    {
        int lockCount = lock_count.load();
        nlohmann::json json = {
            {"database", db_name},
            {"creator", creator},
            {"built_time", build_time},
            {"lockCount", lockCount},
            {"status", getStatusStr()},
            {"costTime", getCostTime()}
        };
        if (db_ptr) 
        {
            if (db_ptr->csr)
                json["csr"] = true;
            else
                json["csr"] = false;
            json["txn"] = db_ptr->isLoadTxn();
        }
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