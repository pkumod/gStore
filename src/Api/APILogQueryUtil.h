#pragma once
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

using namespace std;
using namespace nlohmann;

struct DBQueryLogInfo
{
public:
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
    DBQueryLogInfo() {}
    DBQueryLogInfo(const string &_queryDateTime, const string &_remoteIP, const string &_sparql, long _ansNum, const string &_format, const string &_fileName, int _statusCode, size_t _queryTime, const string &_dbName):
    queryDateTime(_queryDateTime), remoteIP(_remoteIP), sparql(_sparql), ansNum(_ansNum), format(_format), fileName(_fileName), statusCode(_statusCode), queryTime(_queryTime), dbName(_dbName) {}
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(DBQueryLogInfo, queryDateTime, remoteIP, sparql, ansNum, format, fileName, statusCode, queryTime, dbName);

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
    void addQueryLogInfo(const string &json_str)
    {
        if(json::accept(json_str))
        {
            DBQueryLogInfo item = json::parse(json_str);
            list.push_back(item);
        }
    }
    vector<struct DBQueryLogInfo> getQueryLogInfoList()
    {
        return list;
    }
};

struct DBAccessLogInfo
{
public:
    std::string ip;
    std::string operation;
    int code;
    std::string msg;
    std::string createtime;
    std::string opt_id;
    std::string endtime;
    int state = 1;
    int num = 0;
    int fail_num = 0;
    std::string backupfilepath;
    std::string queryfilepath;
    std::string dbname;
public:
    DBAccessLogInfo() {}
    DBAccessLogInfo(const string &_ip, const string &_operation): ip(_ip), operation(_operation){}
    bool checkOperation()
    {
        if (operation == "build" || operation == "batchInsert" || operation == "batchRemove" 
            || operation == "backup" || operation == "restore" || operation == "query")
            return true;
        return false;
    }
    void toJSON(nlohmann::json& doc)
    {
        doc["ip"] = ip;
        doc["operation"] = operation;
        doc["code"] = code;
        doc["msg"] = msg;
        doc["createtime"] = createtime;
        if (checkOperation())
            doc["state"] = state;
        if (!opt_id.empty())
            doc["opt_id"] = opt_id;
        if (!endtime.empty())
            doc["endtime"] = endtime;
        if (!dbname.empty())
            doc["dbname"] = dbname;
        if (operation == "build" || operation == "batchInsert" || operation == "batchRemove")
        {
            doc["num"] = num;
            doc["fail_num"] = fail_num;
        }
        if (operation == "backup" && !backupfilepath.empty())
        {
            doc["backupfilepath"] = backupfilepath;
        }
        if (operation == "query")
        {
            doc["queryfilepath"] = queryfilepath;
        }
    }
    static bool fromJSON(const string& json_str, DBAccessLogInfo& item)
    {
        if(!json::accept(json_str))
        {
            return false;
        }
        json doc = json::parse(json_str);
        doc["ip"].get_to(item.ip);
        doc["operation"].get_to(item.operation);
        doc["code"].get_to(item.code);
        doc["msg"].get_to(item.msg);
        doc["createtime"].get_to(item.createtime);
        if (doc.contains("opt_id"))
            doc["opt_id"].get_to(item.opt_id);
        if (doc.contains("endtime"))
            doc["endtime"].get_to(item.endtime);
        if (doc.contains("dbname"))
            doc["dbname"].get_to(item.dbname);
        if (item.checkOperation())
        {
            if (doc.contains("state"))
                doc["state"].get_to(item.state);
            if (doc.contains("num"))
                doc["num"].get_to(item.num);
            if (doc.contains("fail_num"))
                doc["fail_num"].get_to(item.fail_num);
            if (doc.contains("backupfilepath"))
                doc["backupfilepath"].get_to(item.backupfilepath);
            if (doc.contains("queryfilepath"))
                doc["queryfilepath"].get_to(item.queryfilepath);
        }
        return true;
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
        struct DBAccessLogInfo item;
        if(DBAccessLogInfo::fromJSON(json_str, item))
            list.push_back(item);
    }
    vector<struct DBAccessLogInfo> getAccessLogInfoList()
    {
        return list;
    }
};

struct TransactionLogInfo
{
public:
    std::string db_name;
    std::string TID;
    std::string user;
    std::string state;
    std::string begin_time;
    std::string end_time;
public:
    TransactionLogInfo() {}
    TransactionLogInfo(string _db_name, string _TID, string _user, string _state, string _begin_time, string _end_time):
    db_name(_db_name), TID(_TID), user(_user), state(_state), begin_time(_begin_time), end_time(_end_time) {}
    void toJSON(nlohmann::json& doc)
    {
        doc["db_name"] = db_name;
        doc["TID"] = TID;
        doc["user"] = user;
        doc["state"] = state;
        doc["begin_time"] = begin_time;
        doc["end_time"] = end_time;
    }
    static bool fromJSON(const string& json_str, TransactionLogInfo& item)
    {
        if(!json::accept(json_str))
        {
            return false;
        }
        json doc = json::parse(json_str);
        if (doc.contains("db_name"))
            doc["db_name"].get_to(item.db_name);
        if (doc.contains("TID"))
            doc["TID"].get_to(item.TID);
        if (doc.contains("user"))
            doc["user"].get_to(item.user);
        if (doc.contains("state"))
            doc["state"].get_to(item.state);
        if (doc.contains("begin_time"))
            doc["begin_time"].get_to(item.begin_time);
        if (doc.contains("end_time"))
            doc["end_time"].get_to(item.end_time);
        return true;
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
        struct TransactionLogInfo item;
        if(TransactionLogInfo::fromJSON(json_str, item))
        {
            list.push_back(item);
        }
    }
    vector<struct TransactionLogInfo> getTransactionLogInfoList()
    {
        return list;
    }
};