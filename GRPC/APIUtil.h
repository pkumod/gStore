/*
 * @Author: wangjian
 * @Date: 2021-12-20 16:35:18
 * @LastEditTime: 2022-03-10 14:38:40
 * @LastEditors: Please set LastEditors
 * @Description: grpc util
 * @FilePath: /gstore/GRPC/grpcUtil.h
 */
#pragma once
#include <stdio.h>

#include "../Database/Database.h"
#include "../Database/CSRUtil.h"
#include "../Database/Txn_manager.h"
#include "../Util/IPWhiteList.h"
#include "../Util/IPBlackList.h"

using namespace std;
using namespace rapidjson;

#define ROOT_USERNAME "root"
#define PFN_HEADER "#include <iostream>\n#include \"../../Database/CSRUtil.h\"\n\nusing namespace std;\n"


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
        doc.SetObject();
        doc.AddMember("database", rapidjson::StringRef(db_name.c_str()), doc.GetAllocator());
        doc.AddMember("creator", rapidjson::StringRef(creator.c_str()), doc.GetAllocator());
        doc.AddMember("built_time", rapidjson::StringRef(build_time.c_str()), doc.GetAllocator());
        doc.AddMember("status", rapidjson::StringRef(status.c_str()), doc.GetAllocator());
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
        password = password = _password;
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
    std::string getrestore(){
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
    std::string getbackup(){
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
    std::string getexport(){
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
    std::string toJSON()
    {
        return "{\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";
    }
};

struct IpInfo
{
private:
    int accessNum=0;
    int successNum=0;
    int errorNum=0;
    std::string ip;
public:
    IpInfo(){

    }
    IpInfo(string name)
    {
        ip=name;
        accessNum=0;
        successNum=0;
        errorNum=0;
    }
    int getAccessNum()
    {
        return accessNum;
    }
    int getSuccessNum()
    {
        return successNum;
    }
    int getErrorNum()
    {
        return errorNum;
    }
    string getName()
    {
        return ip;
    }
    void setAccessNum(int value)
    {
        accessNum=value;
    }
    void setSuccessNum(int value)
    {
        successNum=value;
    }
    void setErrorNum(int value)
    {
        errorNum=value;
    }
    void setName(string value)
    {
        ip=value;
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
public:
    DBQueryLogInfo () {}
    DBQueryLogInfo (string _queryDateTime, string _remoteIP, string _sparql, long _ansNum, string _format, string _fileName, int _statusCode, size_t _queryTime)
    {
        queryDateTime = _queryDateTime;
        remoteIP = remoteIP;
        sparql = _sparql;
        ansNum = _ansNum;
        format = _format;
        fileName = _fileName;
        statusCode = _statusCode;
        queryTime = queryTime;
    }
    DBQueryLogInfo(string json_str)
    {
        rapidjson::Document doc;
        doc.SetObject();
        if(!doc.Parse(json_str.c_str()).HasParseError())
        {
            if (doc.HasMember("QueryDateTime"))
            {
                queryDateTime = doc["QueryDateTime"].GetString();
            } 
            if (doc.HasMember("RemoteIP"))
            {
                remoteIP = doc["RemoteIP"].GetString();
            }             
            if (doc.HasMember("Sparql"))
            {
                sparql = doc["Sparql"].GetString();
            }
            if (doc.HasMember("AnsNum"))
            {
                ansNum = doc["AnsNum"].GetInt();
            }
            if (doc.HasMember("Format"))
            {
                format = doc["Format"].GetString();
            }                
            if (doc.HasMember("FileName"))
            {
                fileName = doc["FileName"].GetString();
            }               
            if (doc.HasMember("StatusCode"))
            {
                statusCode = doc["StatusCode"].GetInt();
            }               
            if (doc.HasMember("QueryTime"))
            {
                queryTime = doc["QueryTime"].GetUint64();
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

    std::string getQueryDateTime(){return queryDateTime;}
    std::string getRemoteIP() {return remoteIP;}
    std::string getSparql() {return sparql;}
    long getAnsNum() {return ansNum;}
    std::string getFormat() {return format;}
    std::string getFileName() {return fileName;}
    int getStatusCode() {return statusCode;}
    int getQueryTime() {return queryTime;}
    std::string toJSON()
    {
        rapidjson::Document doc;
        doc.SetObject();
        doc.AddMember("QueryDateTime", rapidjson::StringRef(queryDateTime.c_str()), doc.GetAllocator());
        doc.AddMember("RemoteIP", rapidjson::StringRef(remoteIP.c_str()), doc.GetAllocator());
        doc.AddMember("Sparql", rapidjson::StringRef(sparql.c_str()), doc.GetAllocator());
        doc.AddMember("AnsNum", ansNum, doc.GetAllocator());
        doc.AddMember("Format", rapidjson::StringRef(format.c_str()), doc.GetAllocator());
        doc.AddMember("FileName", rapidjson::StringRef(fileName.c_str()), doc.GetAllocator());
        doc.AddMember("StatusCode", statusCode, doc.GetAllocator());
        doc.AddMember("QueryTime", queryTime, doc.GetAllocator());
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        doc.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
    void toValue(rapidjson::Value &obj, rapidjson::Document::AllocatorType &allocator)
    {
		obj.AddMember("QueryDateTime", rapidjson::StringRef(queryDateTime.c_str()), allocator);
        obj.AddMember("RemoteIP", rapidjson::StringRef(remoteIP.c_str()), allocator);
        obj.AddMember("Sparql", rapidjson::StringRef(sparql.c_str()), allocator);
        obj.AddMember("AnsNum", ansNum, allocator);
        obj.AddMember("Format", rapidjson::StringRef(format.c_str()), allocator);
        obj.AddMember("FileName", rapidjson::StringRef(fileName.c_str()), allocator);
        obj.AddMember("StatusCode", statusCode, allocator);
        obj.AddMember("QueryTime", queryTime, allocator);
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
        cout<< " make QLinfo" <<endl;
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
            if (doc.HasMember("ip"))
                ip = doc["ip"].GetString();
            if (doc.HasMember("operation"))
                operation = doc["operation"].GetString();
            if (doc.HasMember("code"))
                code = doc["code"].GetInt();
            if (doc.HasMember("msg"))
                msg = doc["msg"].GetString();
            if (doc.HasMember("createtime"))
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
        doc.SetObject();
        doc.AddMember("ip", rapidjson::StringRef(ip.c_str()), doc.GetAllocator());
        doc.AddMember("operation", rapidjson::StringRef(operation.c_str()), doc.GetAllocator());
        doc.AddMember("code", code, doc.GetAllocator());
        doc.AddMember("msg", rapidjson::StringRef(msg.c_str()), doc.GetAllocator());
        doc.AddMember("createtime", rapidjson::StringRef(createtime.c_str()), doc.GetAllocator());
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        doc.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
    void toValue(rapidjson::Value &obj, rapidjson::Document::AllocatorType &allocator)
    {
		obj.AddMember("ip", rapidjson::StringRef(ip.c_str()), allocator);
        obj.AddMember("operation", rapidjson::StringRef(operation.c_str()), allocator);
        obj.AddMember("code", code, allocator);
        obj.AddMember("msg", rapidjson::StringRef(msg.c_str()), allocator);
        obj.AddMember("createtime", rapidjson::StringRef(createtime.c_str()), allocator);
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

struct PFNInfo
{
private:
    std::string funName;
    std::string funDesc;
    std::string funArgs;
    std::string funBody;
    std::string funSubs;
    std::string funStatus;
    std::string funReturn;
    std::string lastTime;
public:
    PFNInfo() {};
    PFNInfo(string _funName, string _funDesc, string _funArgs, string _funBody, string _funSubs,string _funStatus,string _funReturn,string _lastTime)
    {
        funName = _funName;
        funDesc = _funDesc;
        funArgs = _funArgs;
        funBody = _funBody;
        funSubs = _funSubs;
        funStatus = _funStatus;
        funReturn = _funReturn;
        lastTime = _lastTime;
    }
    PFNInfo(string json_str) 
    {
        rapidjson::Document doc;
        doc.SetObject();
        if(!doc.Parse(json_str.c_str()).HasParseError())
        {
            if (doc.HasMember("fun_name"))
                funName = doc["fun_name"].GetString();
            if (doc.HasMember("fun_desc"))
                funDesc = doc["fun_desc"].GetString();
            if (doc.HasMember("fun_args"))
                funArgs = doc["fun_args"].GetString();
            if (doc.HasMember("fun_body"))
                funBody = doc["fun_body"].GetString();
            if (doc.HasMember("fun_subs"))
                funSubs = doc["fun_subs"].GetString();
            if (doc.HasMember("fun_status"))
                funStatus = doc["fun_status"].GetString();
            if (doc.HasMember("fun_return"))
                funReturn = doc["fun_return"].GetString();
            if (doc.HasMember("last_time"))
                lastTime = doc["last_time"].GetString();
            //doc.Clear();
        }
    }
    std::string getFunName() {return funName;}
    std::string getFunDesc() {return funDesc;}
    std::string getFunArgs() {return funArgs;}
    std::string getFunBody() {return funBody;}
    std::string getFunSubs() {return funSubs;}
    std::string getFunStatus() {return funStatus;}
    std::string getFunReturn() {return funReturn;}
    std::string getLastTime() {return lastTime;}
    void setFunName(string value) {funName = value;}
    void setFunDesc(string value) {funDesc = value;}
    void setFunArgs(string value) {funArgs = value;}
    void setFunBody(string value) {funBody = value;}
    void setFunSubs(string value) {funSubs = value;}
    void setFunStatus(string value) {funStatus = value;}
    void setFunReturn(string value) {funReturn = value;}
    void setLastTime(string value) {lastTime = value;}
    string toJSON()
    {
        rapidjson::Document doc;
        doc.SetObject();
        doc.AddMember("fun_name", rapidjson::StringRef(funName.c_str()), doc.GetAllocator());
        doc.AddMember("fun_desc", rapidjson::StringRef(funDesc.c_str()), doc.GetAllocator());
        doc.AddMember("fun_args", rapidjson::StringRef(funArgs.c_str()), doc.GetAllocator());
        doc.AddMember("fun_body", rapidjson::StringRef(funBody.c_str()), doc.GetAllocator());
        doc.AddMember("fun_subs", rapidjson::StringRef(funSubs.c_str()), doc.GetAllocator());
        doc.AddMember("fun_status", rapidjson::StringRef(funStatus.c_str()), doc.GetAllocator());
        doc.AddMember("fun_return", rapidjson::StringRef(funReturn.c_str()), doc.GetAllocator());
        doc.AddMember("last_time", rapidjson::StringRef(lastTime.c_str()), doc.GetAllocator());
        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        doc.Accept(writer);
	    string json_str = strBuf.GetString();
        return json_str;
    }
    void copyFrom(struct PFNInfo &pfn_info)
    {
        funName = pfn_info.getFunName();
        funDesc = pfn_info.getFunDesc();
        funArgs = pfn_info.getFunArgs();
        funBody = pfn_info.getFunBody();
        funSubs = pfn_info.getFunSubs();
        funStatus = pfn_info.getFunStatus();
        funReturn = pfn_info.getFunReturn();
        lastTime = pfn_info.getLastTime();
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
        
    string system_path = "data/system/system.nt";
    string backup_path = "./backups";
    string DB_path = ".";
    string root_username = "root";
    unsigned int max_output_size = 10000000;
    string query_log_path = "querylog_path";
    string access_log_path = "accesslog_path";
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
    string system_password;
    int connection_num = 0;
    int blackList = 0;
    int whiteList = 0;
    string ipBlackFile = "ipDeny.config";
    string ipWhiteFile = "ipAllow.config";
    IPWhiteList* ipWhiteList;
    IPBlackList* ipBlackList;

    std::mutex query_log_lock;
    std::mutex access_log_lock;

    bool ip_check(string ip);
    bool ip_error_num_check(string ip);
    std::string fun_cppcheck(const std::string username, struct PFNInfo *fun_info);
    std::string fun_build_source_data(struct PFNInfo * fun_info, bool has_header);
    void fun_write_json_file(const std::string& username, struct PFNInfo *fun_info, std::string operation);
    void fun_parse_from_name(const std::string& username, const std::string& fun_name, struct PFNInfo *fun_info);
public:
    APIUtil();
    ~APIUtil();
    int initialize(const std::string db_name, bool load_csr);
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
    void get_already_builds(vector<struct DatabaseInfo *> &array);
    bool check_already_build(const std::string& db_name);
    bool trywrlock_database(const std::string& db_name);
    bool rdlock_database(const std::string& db_name);
    bool unlock_database(const std::string& db_name);
    std::string check_indentity(const std::string& username,const std::string& password,const std::string& encryption);
    std::string check_server_indentity(const std::string& password);
    std::string check_param_value(string paramname, string value);
    bool check_db_exist(const std::string& db_name);
    bool add_privilege(const std::string& username, const std::string& type, const std::string& db_name);
    bool del_privilege(const std::string& username, const std::string& type, const std::string& db_name);
    bool check_privilege(const std::string& username, const std::string& type, const std::string& db_name);
    bool update_sys_db(string query);
    bool refresh_sys_db();
    std::string query_sys_db(const std::string& sparql);
    bool build_db_user_privilege(std::string db_name, std::string username);
    std::string get_moniter_info(Database* database, DatabaseInfo* dbinfo);
    //todo
    txn_id_t get_txn_id(string db_name, string user);
    bool insert_txn_managers(Database* current_database, std::string database);
    bool find_txn_managers(std::string db_name);
    bool db_checkpoint(string db_name);
    string db_checkpoint_all();
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
    bool user_add(string username, string password);
    bool user_delete(string username, string password);
    bool user_pwd_alert(string username, string password);
    string get_user_info();
    int clear_user_privilege(string username);
    string check_access_ip(string ip);
    bool ip_save(string ip_type, vector<string> ipVector);
    vector<string> ip_list(string type);
    string ip_enabled_type();
    // for access log
    void get_access_log(const string &date, int &page_no, int &page_size, struct DBAccessLogs *dbAccessLogs);
    void write_access_log(string operation, string remoteIP, int statusCode, string statusMsg);
    // for query log
    void get_query_log(const string &date, int &page_no, int &page_size, struct DBQueryLogs *dbQueryLogs);
    void write_query_log(struct DBQueryLogInfo *queryLogInfo);
    // for personalized function
    void fun_query(const std::string &fun_name, const string &fun_status, const string &username, struct PFNInfos *pfn_infos);
    void fun_create(const std::string &username, struct PFNInfo *pfn_info);
    void fun_update(const std::string &username, struct PFNInfo *pfn_info);
    void fun_delete(const std::string &username, struct PFNInfo *pfn_info);
    string fun_build(const std::string &username, const std::string fun_name);
    void fun_review(const std::string &username, struct PFNInfo *pfn_info);
    // for data get
    string get_system_path();
    void set_system_path(string sys_path);
    string get_backup_path();
    void set_backup_path(string bup_path);
    string get_Db_path();
    void set_Db_path(string db_path);
    unsigned int get_max_output_size();
    string get_root_username();
};