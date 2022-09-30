/*
 * @Author: wangjian
 * @Date: 2021-12-20 16:35:18
 * @LastEditTime: 2022-09-30 10:05:54
 * @LastEditors: wangjian 2606583267@qq.com
 * @Description: api util
 * @FilePath: /gstore/GRPC/grpcUtil.h
 */
#pragma once
#include <stdio.h>

#include "../Database/Database.h"
#include "../Database/Txn_manager.h"
#include "../Util/Util.h"
#include "../Util/IPWhiteList.h"
#include "../Util/IPBlackList.h"

using namespace std;
using namespace rapidjson;

#define ROOT_USERNAME "root"
#define SYSTEM_DB_NAME "system"
#define PFN_HEADER "#include \"../../Query/PathQueryHandler.h\"\n\nusing namespace std;\n\n"
#define TRANSACTION_LOG_PATH "./logs/transaction.json"
#define TRANSACTION_LOG_TEMP_PATH "./logs/transaction_temp.json"

struct DatabaseInfo
{
private:
    std::string db_name;    //! the name of database
    std::string creator;    //! the creator of database
    std::string build_time; //! the built time of database;
    std::string status;

public:
    pthread_rwlock_t db_lock;

    DatabaseInfo()
    {
        pthread_rwlock_init(&db_lock, NULL);
    }
    DatabaseInfo(string _name, string _creator, string _time)
    {
        db_name = _name;
        creator = _creator;
        build_time = _time;
        pthread_rwlock_init(&db_lock, NULL);
    }
    DatabaseInfo(string _name)
    {
        db_name = _name;
        pthread_rwlock_init(&db_lock, NULL);
    }
    ~DatabaseInfo()
    {
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
    std::string getStatus()
    {
        return status;
    }
    void setStatus(string _status)
    {
        status = _status;
    }
    std::string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("database", rapidjson::Value().SetString(db_name.c_str(), allocator).Move(), allocator);
        doc.AddMember("creator", rapidjson::Value().SetString(creator.c_str(), allocator).Move(), allocator);
        doc.AddMember("built_time", rapidjson::Value().SetString(build_time.c_str(), allocator).Move(), allocator);
        doc.AddMember("status", rapidjson::Value().SetString(status.c_str(), allocator).Move(), allocator);
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        doc.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
};

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
        /// <summary>
    /// @brief get the database list which the user can query/
    /// </summary>
    /// <returns></returns>
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
    std::string toJSON() {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("username", rapidjson::Value().SetString(this->username.c_str(),allocator).Move(), allocator);
        doc.AddMember("password", rapidjson::Value().SetString(this->password.c_str(),allocator).Move(), allocator);
        doc.AddMember("query_privilege", rapidjson::Value().SetString(this->getQuery().c_str(),allocator).Move(), allocator);
        doc.AddMember("update_privilege", rapidjson::Value().SetString(this->getUpdate().c_str(),allocator).Move(), allocator);
        doc.AddMember("load_privilege", rapidjson::Value().SetString(this->getLoad().c_str(),allocator).Move(), allocator);
        doc.AddMember("unload_privilege", rapidjson::Value().SetString(this->getUnload().c_str(),allocator).Move(), allocator);
        doc.AddMember("backup_privilege", rapidjson::Value().SetString(this->getBackup().c_str(),allocator).Move(), allocator);
        doc.AddMember("restore_privilege", rapidjson::Value().SetString(this->getRestore().c_str(),allocator).Move(), allocator);
        doc.AddMember("export_privilege", rapidjson::Value().SetString(this->getExport().c_str(),allocator).Move(), allocator);
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        doc.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
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

struct DBQueryLogInfo
{
private:
    std::string queryDateTime;
    std::string remoteIP;
    std::string sparql;
    long ansNum;
    std::string format;
    std::string fileName;
    int statusCode;
    size_t queryTime;
    std::string dbName;
public:
    DBQueryLogInfo () {}
    DBQueryLogInfo (string _queryDateTime, string _remoteIP, string _sparql, long _ansNum, string _format, string _fileName, int _statusCode, size_t _queryTime, string _dbName)
    {
        queryDateTime = _queryDateTime;
        remoteIP = _remoteIP;
        sparql = _sparql;
        ansNum = _ansNum;
        format = _format;
        fileName = _fileName;
        statusCode = _statusCode;
        queryTime = _queryTime;
        dbName = _dbName;
    }
    DBQueryLogInfo(string json_str)
    {
        rapidjson::Document doc;
        doc.SetObject();
        if(!doc.Parse(json_str.c_str()).HasParseError())
        {
            if (doc.HasMember("QueryDateTime") && doc["QueryDateTime"].IsString())
            {
                queryDateTime = doc["QueryDateTime"].GetString();
            } 
            if (doc.HasMember("RemoteIP") && doc["RemoteIP"].IsString())
            {
                remoteIP = doc["RemoteIP"].GetString();
            }             
            if (doc.HasMember("Sparql") && doc["Sparql"].IsString())
            {
                sparql = doc["Sparql"].GetString();
            }
            if (doc.HasMember("AnsNum") && doc["AnsNum"].IsInt())
            {
                ansNum = doc["AnsNum"].GetInt();
            }
            if (doc.HasMember("Format") && doc["Format"].IsString())
            {
                format = doc["Format"].GetString();
            }                
            if (doc.HasMember("FileName") && doc["FileName"].IsString())
            {
                fileName = doc["FileName"].GetString();
            }               
            if (doc.HasMember("StatusCode") && doc["StatusCode"].IsInt())
            {
                statusCode = doc["StatusCode"].GetInt();
            }               
            if (doc.HasMember("QueryTime") && doc["QueryTime"].IsUint64())
            {
                queryTime = doc["QueryTime"].GetUint64();
            }               
            if (doc.HasMember("DbName") && doc["DbName"].IsString())
            {
                dbName = doc["DbName"].GetString();
            }
        }
    }
    void setQueryDateTime(string _queryDateTime){queryDateTime = _queryDateTime;}
    void setRemoteIP(string _remoteIP) {remoteIP = remoteIP;}
    void setSparql(string _sparql) {sparql = _sparql;}
    void setAnsNum(long _ansNum) {ansNum = _ansNum;}
    void setFormat(string _format) {format = _format;}
    void setFileName(string _fileName) {fileName = _fileName;}
    void setStatusCode(int _statusCode) {statusCode = _statusCode;}
    void setQueryTime(int _queryTime) {queryTime = _queryTime;}
    void setDbName(string _dbName) {dbName = _dbName;}

    std::string getQueryDateTime(){return queryDateTime;}
    std::string getRemoteIP() {return remoteIP;}
    std::string getSparql() {return sparql;}
    long getAnsNum() {return ansNum;}
    std::string getFormat() {return format;}
    std::string getFileName() {return fileName;}
    int getStatusCode() {return statusCode;}
    int getQueryTime() {return queryTime;}
    std::string getDbName() {return dbName;}
    std::string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("QueryDateTime", rapidjson::Value().SetString(queryDateTime.c_str(), allocator).Move(), allocator);
        doc.AddMember("RemoteIP", rapidjson::Value().SetString(remoteIP.c_str(), allocator).Move(), allocator);
        doc.AddMember("Sparql", rapidjson::Value().SetString(sparql.c_str(), allocator).Move(), allocator);
        doc.AddMember("AnsNum", ansNum, allocator);
        doc.AddMember("Format", rapidjson::Value().SetString(format.c_str(), allocator).Move(), allocator);
        doc.AddMember("FileName", rapidjson::Value().SetString(fileName.c_str(), allocator).Move(), allocator);
        doc.AddMember("StatusCode", statusCode, allocator);
        doc.AddMember("QueryTime", queryTime, allocator);
        doc.AddMember("DbName", rapidjson::Value().SetString(dbName.c_str(), allocator).Move(), allocator);
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        doc.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
};

struct DBQueryLogs
{
private:
    int totalSize;
    int totalPage;
    vector<struct DBQueryLogInfo> list;
public:
    DBQueryLogs() 
    {
        totalSize = 0;
        totalPage = 0;
    }
    DBQueryLogs(int _totalSize, int _totalPage)
    {
        totalSize = _totalSize;
        totalPage = _totalPage;
    }
    void setTotalSize(int _totalSize)
    {
        totalSize = _totalSize;
    }
    void setTotalPage(int _totalPage)
    {
        totalPage = _totalPage;
    }
    int getTotalSize() 
    {
        return totalSize;
    }
    int getTotalPage()
    {
        return totalPage;
    }
    void addQueryLogInfo(const string & json_str)
    {
        DBQueryLogInfo item(json_str);
        list.push_back(item);
    }
    vector<struct DBQueryLogInfo> getQueryLogInfoList()
    {
        return list;
    }
};

struct DBAccessLogInfo
{
private:
    std::string ip;
    std::string operation;
    int code;
    std::string msg;
    std::string createtime;
public:
    DBAccessLogInfo() {}
    DBAccessLogInfo(string _ip, string _operation, int _code, string _msg, string _createtime) 
    {
        ip = _ip;
        operation = _operation;
        code = _code;
        msg = _msg;
        createtime = _createtime;
    }
    DBAccessLogInfo(string json_str)
    {
        rapidjson::Document doc;
        doc.SetObject();
        if(!doc.Parse(json_str.c_str()).HasParseError())
        {
            if (doc.HasMember("ip") && doc["ip"].IsString())
                ip = doc["ip"].GetString();
            if (doc.HasMember("operation") && doc["operation"].IsString())
                operation = doc["operation"].GetString();
            if (doc.HasMember("code") && doc["code"].IsInt())
                code = doc["code"].GetInt();
            if (doc.HasMember("msg") && doc["msg"].IsString())
                msg = doc["msg"].GetString();
            if (doc.HasMember("createtime") && doc["createtime"].IsString())
                createtime = doc["createtime"].GetString();
        }
    }
    std::string getIP() {return ip;}
    std::string getOperation() {return operation;}
    int getCode() {return code;}
    std::string getMsg() {return msg;}
    std::string getCreateTime() {return createtime;}
    std::string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("ip", rapidjson::Value().SetString(ip.c_str(), allocator).Move(), allocator);
        doc.AddMember("operation", rapidjson::Value().SetString(operation.c_str(), allocator).Move(), allocator);
        doc.AddMember("code", code, allocator);
        doc.AddMember("msg", rapidjson::Value().SetString(msg.c_str(), allocator).Move(), allocator);
        doc.AddMember("createtime", rapidjson::Value().SetString(createtime.c_str(), allocator).Move(), allocator);
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        doc.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
};

struct DBAccessLogs
{
private:
    int totalSize;
    int totalPage;
    vector<struct DBAccessLogInfo> list;
public:
    DBAccessLogs() 
    {
        totalSize = 0;
        totalPage = 0;
    }
    DBAccessLogs(int _totalSize, int _totalPage)
    {
        totalSize = _totalSize;
        totalPage = _totalPage;
    }
    void setTotalSize(int _totalSize)
    {
        totalSize = _totalSize;
    }
    void setTotalPage(int _totalPage)
    {
        totalPage = _totalPage;
    }
    int getTotalSize() 
    {
        return totalSize;
    }
    int getTotalPage()
    {
        return totalPage;
    }
    void addAccessLogInfo(const string & json_str)
    {
        DBAccessLogInfo item(json_str);
        list.push_back(item);
    }
    vector<struct DBAccessLogInfo> getAccessLogInfoList()
    {
        return list;
    }
};

struct TransactionLogInfo
{
private:
    std::string db_name;
    std::string TID;
    std::string user;
    std::string state;
    std::string begin_time;
    std::string end_time;
public:
    TransactionLogInfo() {}
    TransactionLogInfo(string _db_name, string _TID, string _user, string _state, string _begin_time, string _end_time) 
    {
        db_name = _db_name;
        TID = _TID;
        user = _user;
        state = _state;
        begin_time = _begin_time;
        end_time = _end_time;

    }
    TransactionLogInfo(string json_str)
    {
        rapidjson::Document doc;
        doc.SetObject();
        if(!doc.Parse(json_str.c_str()).HasParseError())
        {
            if (doc.HasMember("db_name") && doc["db_name"].IsString())
                db_name = doc["db_name"].GetString();
            if (doc.HasMember("TID") && doc["TID"].IsString())
                TID = doc["TID"].GetString();
            if (doc.HasMember("user") && doc["user"].IsString())
                user = doc["user"].GetString();
            if (doc.HasMember("state") && doc["state"].IsString())
                state = doc["state"].GetString();
            if (doc.HasMember("begin_time") && doc["begin_time"].IsString())
                begin_time = doc["begin_time"].GetString();
            if (doc.HasMember("end_time") && doc["end_time"].IsString())
                end_time = doc["end_time"].GetString();
        }
    }
    std::string getDbName() {return db_name;}
    std::string getTID() {return TID;}
    std::string getUser() {return user;}
    std::string getState() {return state;}
    std::string getBeginTime() {return begin_time;}
    std::string getEndTime() {return end_time;}
    void setState(string value) {state = value;}
    void setEndTime(string value) {end_time = value;}
    std::string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("db_name", rapidjson::Value().SetString(db_name.c_str(), allocator).Move(), allocator);
        doc.AddMember("TID", rapidjson::Value().SetString(TID.c_str(), allocator).Move(), allocator);
        doc.AddMember("user", rapidjson::Value().SetString(user.c_str(), allocator).Move(), allocator);
        doc.AddMember("state", rapidjson::Value().SetString(state.c_str(), allocator).Move(), allocator);
        doc.AddMember("begin_time", rapidjson::Value().SetString(begin_time.c_str(), allocator).Move(), allocator);
        doc.AddMember("end_time", rapidjson::Value().SetString(end_time.c_str(), allocator).Move(), allocator);
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        doc.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
};

struct TransactionLogs
{
private:
    int totalSize;
    int totalPage;
    vector<struct TransactionLogInfo> list;
public:
    TransactionLogs() 
    {
        totalSize = 0;
        totalPage = 0;
    }
    TransactionLogs(int _totalSize, int _totalPage)
    {
        totalSize = _totalSize;
        totalPage = _totalPage;
    }
    void setTotalSize(int _totalSize)
    {
        totalSize = _totalSize;
    }
    void setTotalPage(int _totalPage)
    {
        totalPage = _totalPage;
    }
    int getTotalSize() 
    {
        return totalSize;
    }
    int getTotalPage()
    {
        return totalPage;
    }
    void addTransactionLogInfo(const string & json_str)
    {
        TransactionLogInfo item(json_str);
        list.push_back(item);
    }
    vector<struct TransactionLogInfo> getTransactionLogInfoList()
    {
        return list;
    }
};

struct PFNInfo
{
private:
    std::string fun_name;
    std::string fun_desc;
    std::string fun_args;
    std::string fun_body;
    std::string fun_subs;
    std::string fun_status;
    std::string fun_return;
    std::string last_time;
public:
    PFNInfo() 
    {
        fun_name = "";
        fun_desc = "";
        fun_args = "";
        fun_body = "";
        fun_subs = "";
        fun_status = "";
        fun_return = "";
        last_time = "";
    };
    PFNInfo(string _fun_name, string _fun_desc, string _fun_args, string _fun_body, string _fun_subs,string _fun_status,string _fun_return,string _last_time)
    {
        fun_name = _fun_name;
        fun_desc = _fun_desc;
        fun_args = _fun_args;
        fun_body = _fun_body;
        fun_subs = _fun_subs;
        fun_status = _fun_status;
        fun_return = _fun_return;
        last_time = _last_time;
    }
    PFNInfo(string json_str) 
    {
        rapidjson::Document doc;
        doc.SetObject();
        if(!doc.Parse(json_str.c_str()).HasParseError())
        {
            if (doc.HasMember("funName") && doc["funName"].IsString())
                fun_name = doc["funName"].GetString();
            if (doc.HasMember("funDesc") && doc["funDesc"].IsString())
                fun_desc = doc["funDesc"].GetString();
            if (doc.HasMember("funArgs") && doc["funArgs"].IsString())
                fun_args = doc["funArgs"].GetString();
            if (doc.HasMember("funBody") && doc["funBody"].IsString())
                fun_body = doc["funBody"].GetString();
            if (doc.HasMember("funSubs") && doc["funSubs"].IsString())
                fun_subs = doc["funSubs"].GetString();
            if (doc.HasMember("funStatus") && doc["funStatus"].IsString())
                fun_status = doc["funStatus"].GetString();
            if (doc.HasMember("funReturn") && doc["funReturn"].IsString())
                fun_return = doc["funReturn"].GetString();
            if (doc.HasMember("lastTime") && doc["lastTime"].IsString())
                last_time = doc["lastTime"].GetString();
            //doc.Clear();
        }
    }
    std::string getFunName() {return fun_name;}
    std::string getFunDesc() {return fun_desc;}
    std::string getFunArgs() {return fun_args;}
    std::string getFunBody() {return fun_body;}
    std::string getFunSubs() {return fun_subs;}
    std::string getFunStatus() {return fun_status;}
    std::string getFunReturn() {return fun_return;}
    std::string getLastTime() {return last_time;}
    void setFunName(string value) {fun_name = value;}
    void setFunDesc(string value) {fun_desc = value;}
    void setFunArgs(string value) {fun_args = value;}
    void setFunBody(string value) {fun_body = value;}
    void setFunSubs(string value) {fun_subs = value;}
    void setFunStatus(string value) {fun_status = value;}
    void setFunReturn(string value) {fun_return = value;}
    void setLastTime(string value) {last_time = value;}
    string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        doc.SetObject();
        doc.AddMember("funName", rapidjson::Value().SetString(fun_name.c_str(), allocator).Move(), allocator);
        doc.AddMember("funDesc", rapidjson::Value().SetString(fun_desc.c_str(), allocator).Move(), allocator);
        doc.AddMember("funArgs", rapidjson::Value().SetString(fun_args.c_str(), allocator).Move(), allocator);
        doc.AddMember("funBody", rapidjson::Value().SetString(fun_body.c_str(), allocator).Move(), allocator);
        doc.AddMember("funSubs", rapidjson::Value().SetString(fun_subs.c_str(), allocator).Move(), allocator);
        doc.AddMember("funStatus", rapidjson::Value().SetString(fun_status.c_str(), allocator).Move(), allocator);
        doc.AddMember("funReturn", rapidjson::Value().SetString(fun_return.c_str(), allocator).Move(), allocator);
        doc.AddMember("lastTime", rapidjson::Value().SetString(last_time.c_str(), allocator).Move(), allocator);
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        doc.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
    void copyFrom(struct PFNInfo &pfn_info)
    {
        fun_name = pfn_info.getFunName();
        fun_desc = pfn_info.getFunDesc();
        fun_args = pfn_info.getFunArgs();
        fun_body = pfn_info.getFunBody();
        fun_subs = pfn_info.getFunSubs();
        fun_status = pfn_info.getFunStatus();
        fun_return = pfn_info.getFunReturn();
        last_time = pfn_info.getLastTime();
    }
};

struct PFNInfos
{
private:
    int totalSize;
    int totalPage;
    vector<struct PFNInfo> list;
public:
    PFNInfos() 
    {
        totalSize = 0;
        totalPage = 0;
    }
    PFNInfos(int _totalSize, int _totalPage)
    {
        totalSize = _totalSize;
        totalPage = _totalPage;
    }
    void setTotalSize(int _totalSize)
    {
        totalSize = _totalSize;
    }
    void setTotalPage(int _totalPage)
    {
        totalPage = _totalPage;
    }
    int getTotalSize() 
    {
        return totalSize;
    }
    int getTotalPage()
    {
        return totalPage;
    }
    void addPFNInfo(const string & json_str)
    {
        PFNInfo item(json_str);
        list.push_back(item);
    }
    vector<struct PFNInfo> getPFNInfoList()
    {
        return list;
    }
};

class APIUtil
{
private:
    Util util;
    string default_port = "9000";
    int thread_pool_num = 30;
    
    unsigned int max_output_size = 10000000;
    int max_database_num = 100;
    int max_user_num = 1000;
    string system_path = "data/system/system.nt";
    string DB_path = ".";
    string backup_path = "./backups";
    string query_log_mode = "0";
    string query_log_path = "logs/endpoint/";
    string access_log_path = "logs/ipaccess/";
    string query_result_path = "logs/query_result/";
    std::string pfn_file_path = "fun/";
    std::string pfn_lib_path = "lib/";
    std::string pfn_include_header = "";

    std::map<std::string, Database *> databases;
    std::map<std::string, struct DBUserInfo *> users;
    std::map<std::string, struct DatabaseInfo *> already_build;
    std::map<std::string, struct IpInfo *> ips;
    std::map<std::string, shared_ptr<Txn_manager>> txn_managers;
    std::map<std::string, txn_id_t> running_txn;

    Database *system_database;
    pthread_rwlock_t users_map_lock;
    pthread_rwlock_t databases_map_lock;
    pthread_rwlock_t already_build_map_lock;
    pthread_rwlock_t txn_m_lock;
    pthread_rwlock_t ips_map_lock;
    string system_username = "system";
    string system_password;
    string system_password_path;
    unsigned int connection_num = 0;
    int blackList = 0;
    int whiteList = 0;
    string ipBlackFile = "ipDeny.config";
    string ipWhiteFile = "";
    IPWhiteList* ipWhiteList;
    IPBlackList* ipBlackList;

    pthread_rwlock_t query_log_lock;
    pthread_rwlock_t access_log_lock;
    pthread_rwlock_t transactionlog_lock;
    pthread_rwlock_t fun_data_lock;

    bool ip_check(const string& ip);
    bool ip_error_num_check(const string& ip);
    std::string fun_cppcheck(const std::string username, struct PFNInfo *fun_info);
    std::string fun_build_source_data(struct PFNInfo * fun_info, bool has_header);
    void fun_write_json_file(const std::string& username, struct PFNInfo *fun_info, std::string operation);
    void fun_parse_from_name(const std::string& username, const std::string& fun_name, struct PFNInfo *fun_info);
public:
    APIUtil();
    ~APIUtil();
    int initialize(const std::string server_type, const std::string port, const std::string db_name, bool load_csr);
    bool trywrlock_database_map();
    bool unlock_database_map();
    bool trywrlock_already_build_map();
    bool unlock_already_build_map();
    bool rw_wrlock_build_map();
    bool rw_wrlock_database_map();
    bool add_database(const std::string& db_name, Database *& db);
    Database* get_database(const std::string& db_name);
    DatabaseInfo* get_databaseinfo(const std::string& db_name);
    bool trywrlock_databaseinfo(DatabaseInfo* dbinfo);
    bool tryrdlock_databaseinfo(DatabaseInfo* dbinfo);
    bool unlock_databaseinfo(DatabaseInfo* dbinfo);
    bool check_already_load(const std::string& db_name);
    shared_ptr<Txn_manager> get_Txn_ptr(string db_name);
    bool add_already_build(const std::string& db_name, const std::string& creator, const std::string& build_time, bool try_lock);
    std::string get_already_build(const std::string& db_name);
    void get_already_builds(const std::string& username, vector<struct DatabaseInfo *> &array);
    bool check_already_build(const std::string& db_name);
    bool trywrlock_database(const std::string& db_name);
    bool rdlock_database(const std::string& db_name);
    bool unlock_database(const std::string& db_name);
    std::string check_indentity(const std::string& username,const std::string& password,const std::string& encryption);
    std::string check_server_indentity(const std::string& password);
    std::string check_param_value(const string& paramname, const string& value);
    bool check_user_exist(const std::string& username);
    bool check_user_count();
    bool check_db_exist(const std::string& db_name);
    bool check_db_count();
    bool add_privilege(const std::string& username, const std::string& type, const std::string& db_name);
    bool del_privilege(const std::string& username, const std::string& type, const std::string& db_name);
    bool check_privilege(const std::string& username, const std::string& type, const std::string& db_name);
    bool update_sys_db(string query);
    bool refresh_sys_db();
    std::string query_sys_db(const std::string& sparql);
    bool build_db_user_privilege(std::string db_name, std::string username);
    txn_id_t get_txn_id(string db_name, string user);
    bool insert_txn_managers(Database* current_database, std::string database);
    bool find_txn_managers(std::string db_name);
    bool db_checkpoint(string db_name);
    bool db_checkpoint_all();
    bool delete_from_databases(string db_name);
    bool delete_from_already_build(string db_name);
    //used by drop
    int db_copy(string src_path, string dest_path);
    txn_id_t get_txn_id(shared_ptr<Txn_manager> ptr, int level);
    txn_id_t check_txn_id(string TID);
    string get_txn_begin_time(shared_ptr<Txn_manager> ptr, txn_id_t tid);
    string begin_process(string db_name, int level , string username);
    bool commit_process(shared_ptr<Txn_manager> txn_m, txn_id_t TID);
    bool rollback_process(shared_ptr<Txn_manager> txn_m, txn_id_t TID);
    bool user_add(const string& username, const string& password);
    bool user_delete(const string& username);
    bool user_pwd_alert(const string& username, const string& password);
    void get_user_info(vector<struct DBUserInfo *> *_users);
    int clear_user_privilege(string username);
    string check_access_ip(const string& ip);
    void update_access_ip_error_num(const string& ip);
    bool ip_save(string ip_type, vector<string> ipVector);
    vector<string> ip_list(string type);
    string ip_enabled_type();
    // for access log
    void get_access_log_files(std::vector<std::string> &file_list);
    void get_access_log(const string &date, int &page_no, int &page_size, struct DBAccessLogs *dbAccessLogs);
    void write_access_log(string operation, string remoteIP, int statusCode, string statusMsg);
    // for query log
    void get_query_log_files(std::vector<std::string> &file_list);
    void get_query_log(const string &date, int &page_no, int &page_size, struct DBQueryLogs *dbQueryLogs);
    void write_query_log(struct DBQueryLogInfo *queryLogInfo);
    // for transaction log
    void init_transactionlog();
	int add_transactionlog(std::string db_name, std::string user, std::string TID,  std::string begin_time, std::string status = "RUNNING",  std::string end_time = "INF");
	int delete_transactionlog(std::string db_name, std::string TID);
	int update_transactionlog(std::string db_name, std::string status, std::string end_time);
	void get_transactionlog(int &page_no, int &page_size, struct TransactionLogs *dbQueryLogs);
	void abort_transactionlog(long end_time);
    // for personalized function
    void fun_query(const std::string &fun_name, const string &fun_status, const string &username, struct PFNInfos *pfn_infos);
    void fun_create(const std::string &username, struct PFNInfo *pfn_info);
    void fun_update(const std::string &username, struct PFNInfo *pfn_info);
    void fun_delete(const std::string &username, struct PFNInfo *pfn_info);
    string fun_build(const std::string &username, const std::string fun_name);
    void fun_review(const std::string &username, struct PFNInfo *pfn_info);
    // for data get
    string get_system_path();
    string get_backup_path();
    string get_Db_path();
    string get_query_result_path();
    string get_default_port();
    int get_thread_pool_num();
    unsigned int get_max_output_size();
    string get_root_username();
    string get_system_username();
    unsigned int get_connection_num();
    void increase_connection_num();
    void string_suffix(string& str, const char suffix);
    string get_configure_value(const string& key, string default_value);
    unsigned int get_configure_value(const string& key, unsigned int default_value);
};