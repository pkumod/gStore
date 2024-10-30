#pragma once
#include "APIUtilDefined.h"

using namespace std;
using namespace rapidjson;

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
    void setRemoteIP(string _remoteIP) {remoteIP = _remoteIP;}
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
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value doc(kObjectType);
        doc.AddMember("QueryDateTime", rapidjson::Value().SetString(queryDateTime.c_str(), allocator).Move(), allocator);
        doc.AddMember("RemoteIP", rapidjson::Value().SetString(remoteIP.c_str(), allocator).Move(), allocator);
        doc.AddMember("Sparql", rapidjson::Value().SetString(sparql.c_str(), allocator).Move(), allocator);
        doc.AddMember("AnsNum", ansNum, allocator);
        doc.AddMember("Format", rapidjson::Value().SetString(format.c_str(), allocator).Move(), allocator);
        doc.AddMember("FileName", rapidjson::Value().SetString(fileName.c_str(), allocator).Move(), allocator);
        doc.AddMember("StatusCode", statusCode, allocator);
        doc.AddMember("QueryTime", queryTime, allocator);
        doc.AddMember("DbName", rapidjson::Value().SetString(dbName.c_str(), allocator).Move(), allocator);
        return doc;
    }

    void toJSON(nlohmann::json& doc)
    {
        doc["QueryDateTime"] = queryDateTime;
        doc["RemoteIP"] = remoteIP;
        doc["Sparql"] = sparql;
        doc["AnsNum"] = ansNum;
        doc["Format"] = format;
        doc["FileName"] = fileName;
        doc["StatusCode"] = statusCode;
        doc["QueryTime"] = queryTime;
        doc["DbName"] = dbName;
    }

    std::string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        rapidjson::Value jsonValue = toJSON(allocator);

        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        jsonValue.Accept(writer);
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
    std::string opt_id;
    std::string endtime;
    int state;
    int num;
    int fail_num;
    std::string backupfilepath;
public:
    DBAccessLogInfo() {}
    DBAccessLogInfo(string _ip, string _operation, int _code, string _msg, string _createtime) 
    {
        ip = _ip;
        operation = _operation;
        code = _code;
        msg = _msg;
        createtime = _createtime;
        opt_id = "";
        endtime = "";
        state = 0;
        num = 0;
        fail_num = 0;
        backupfilepath = "";
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
            if (checkOperation())
            {
                if (doc.HasMember("opt_id") && doc["opt_id"].IsString())
                    opt_id = doc["opt_id"].GetString();
                if (doc.HasMember("endtime") && doc["endtime"].IsString())
                    endtime = doc["endtime"].GetString();
                if (doc.HasMember("state") && doc["state"].IsInt())
                    state = doc["state"].GetInt();
                if (doc.HasMember("num") && doc["num"].IsInt())
                    num = doc["num"].GetInt();
                if (doc.HasMember("fail_num") && doc["fail_num"].IsInt())
                    fail_num = doc["fail_num"].GetInt();
                if (doc.HasMember("backupfilepath") && doc["backupfilepath"].IsString())
                    backupfilepath = doc["backupfilepath"].GetString();
            }
        }
    }
    std::string getIP() {return ip;}
    std::string getOperation() {return operation;}
    int getCode() {return code;}
    std::string getMsg() {return msg;}
    std::string getCreateTime() {return createtime;}
    std::string getOptId() {return opt_id;}
    int getState() {return state;}
    int getNum() {return num;}
    int getFailNum() {return fail_num;}
    std::string getBackupfilepath() {return backupfilepath;}
    void setOptId(const std::string& value) {opt_id = value;}
    void setCode(int value) {code = value;}
    void setMsg(const std::string& value) {msg = value;}
    void setEndTime(const std::string& value) {endtime = value;}
    void setState(int value) {state = value;}
    void setNum(int value) {num = value;}
    void setFailNum(int value) {fail_num = value;}
    void setBackupfilepath(const std::string& value) {backupfilepath = value;}
    bool checkOperation()
    {
        if (operation == "build" || operation == "batchInsert" || operation == "batchRemove" || operation == "backup" || operation == "restore")
            return true;
        return false;
    }
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value doc(kObjectType);
        doc.AddMember("ip", rapidjson::Value().SetString(ip.c_str(), allocator).Move(), allocator);
        doc.AddMember("operation", rapidjson::Value().SetString(operation.c_str(), allocator).Move(), allocator);
        doc.AddMember("code", code, allocator);
        doc.AddMember("msg", rapidjson::Value().SetString(msg.c_str(), allocator).Move(), allocator);
        doc.AddMember("createtime", rapidjson::Value().SetString(createtime.c_str(), allocator).Move(), allocator);
        if (checkOperation() && !opt_id.empty())
        {
            doc.AddMember("opt_id", rapidjson::Value().SetString(opt_id.c_str(), allocator).Move(), allocator);
            doc.AddMember("endtime", rapidjson::Value().SetString(endtime.c_str(), allocator).Move(), allocator);
            doc.AddMember("state", state, allocator);
            if (operation == "build" || operation == "batchInsert" || operation == "batchRemove")
            {
                doc.AddMember("num", num, allocator);
                doc.AddMember("fail_num", fail_num, allocator);
            }
            else if (operation == "backup")
                doc.AddMember("backupfilepath", rapidjson::Value().SetString(backupfilepath.c_str(), allocator).Move(), allocator);
        }
        return doc;
    }

    void toJSON(nlohmann::json& doc)
    {
        doc["ip"] = ip;
        doc["operation"] = operation;
        doc["code"] = code;
        doc["msg"] = msg;
        doc["createtime"] = createtime;
        if (checkOperation() && !opt_id.empty())
        {
            doc["opt_id"] = opt_id;
            doc["endtime"] = endtime;
            doc["state"] = state;
            if (operation == "build" || operation == "batchInsert" || operation == "batchRemove")
            {
                doc["num"] = num;
                doc["fail_num"] = fail_num;
            }
            else if (operation == "backup")
            {
                doc["backupfilepath"] = backupfilepath;
            }
        }
    }

    std::string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        rapidjson::Value jsonValue = toJSON(allocator);

        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        jsonValue.Accept(writer);
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
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value doc(rapidjson::kObjectType);
        doc.AddMember("db_name", rapidjson::Value().SetString(db_name.c_str(), allocator).Move(), allocator);
        doc.AddMember("TID", rapidjson::Value().SetString(TID.c_str(), allocator).Move(), allocator);
        doc.AddMember("user", rapidjson::Value().SetString(user.c_str(), allocator).Move(), allocator);
        doc.AddMember("state", rapidjson::Value().SetString(state.c_str(), allocator).Move(), allocator);
        doc.AddMember("begin_time", rapidjson::Value().SetString(begin_time.c_str(), allocator).Move(), allocator);
        doc.AddMember("end_time", rapidjson::Value().SetString(end_time.c_str(), allocator).Move(), allocator);
        return doc;
    }
    void toJSON(nlohmann::json& doc)
    {
        doc["db_name"] = db_name;
        doc["TID"] = TID;
        doc["user"] = user;
        doc["state"] = state;
        doc["begin_time"] = begin_time;
        doc["end_time"] = end_time;
    }
    std::string toJSON()
    {
        rapidjson::Document doc;
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
        rapidjson::Value jsonValue = toJSON(allocator);

        rapidjson::StringBuffer strBuf;
        rapidjson::Writer<rapidjson::StringBuffer> writer(strBuf);
        jsonValue.Accept(writer);
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