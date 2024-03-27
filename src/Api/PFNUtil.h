#pragma once

#include "../Util/Util.h"

using namespace std;
using namespace rapidjson;

#define PFN_HEADER "#include \"../../../src/Query/PathQueryHandler.h\"\n\nusing namespace std;\n\n"

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
    rapidjson::Value toJSON(rapidjson::Document::AllocatorType& allocator)
    {
        rapidjson::Value doc(kObjectType);
        doc.AddMember("funName", rapidjson::Value().SetString(fun_name.c_str(), allocator).Move(), allocator);
        doc.AddMember("funDesc", rapidjson::Value().SetString(fun_desc.c_str(), allocator).Move(), allocator);
        doc.AddMember("funArgs", rapidjson::Value().SetString(fun_args.c_str(), allocator).Move(), allocator);
        doc.AddMember("funBody", rapidjson::Value().SetString(fun_body.c_str(), allocator).Move(), allocator);
        doc.AddMember("funSubs", rapidjson::Value().SetString(fun_subs.c_str(), allocator).Move(), allocator);
        doc.AddMember("funStatus", rapidjson::Value().SetString(fun_status.c_str(), allocator).Move(), allocator);
        doc.AddMember("funReturn", rapidjson::Value().SetString(fun_return.c_str(), allocator).Move(), allocator);
        doc.AddMember("lastTime", rapidjson::Value().SetString(last_time.c_str(), allocator).Move(), allocator);
        return doc;
    }
    string toJSON()
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

class PFNUtil
{
private:
    pthread_rwlock_t pfn_data_lock;
    std::string pfn_base_path = "./pfn/";
    std::string fun_cppcheck(const std::string username, struct PFNInfo *fun_info);
    std::string fun_build_source_data(struct PFNInfo * fun_info, bool has_header);
    void fun_write_json_file(const std::string& username, struct PFNInfo *fun_info, std::string operation);
    void fun_parse_from_name(const std::string& username, const std::string& fun_name, struct PFNInfo *fun_info);
public:
    PFNUtil();
    ~PFNUtil();
    void fun_query(const std::string &fun_name, const string &fun_status, const string &username, struct PFNInfos *pfn_infos);
    void fun_create(const std::string &username, struct PFNInfo *pfn_info);
    void fun_update(const std::string &username, struct PFNInfo *pfn_info);
    void fun_delete(const std::string &username, struct PFNInfo *pfn_info);
    string fun_build(const std::string &username, const std::string fun_name);
    void fun_review(const std::string &username, struct PFNInfo *pfn_info);
    void build_PFNInfo(rapidjson::Value &fun_info, struct PFNInfo *pfn_info);
};
