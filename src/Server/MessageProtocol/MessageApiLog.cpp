#include "MessageApiLog.h"
#include "../ApiProvider.h"

namespace server
{
    // txn log
    void TxnLog::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"db_name", this->db_name},
            {"TID", this->TID},
            {"user", this->user},
            {"state", this->state},
            {"begin_time", this->begin_time},
            {"end_time", this->end_time}};
        json_str = json.dump();
    }

    void TxnLog::from_json(const nlohmann::json& json)
    {
        if (json.contains("db_name"))
            json["db_name"].get_to(this->db_name);
        if (json.contains("TID"))
            json["TID"].get_to(this->TID);
        if (json.contains("user"))
            json["user"].get_to(this->user);
        if (json.contains("state"))
            json["state"].get_to(this->state);
        if (json.contains("begin_time"))
            json["begin_time"].get_to(this->begin_time);
        if (json.contains("end_time"))
            json["end_time"].get_to(this->end_time);
    }

    MessageTxnLogRequest::MessageTxnLogRequest(const nlohmann::json& json_data) : MessageRequest(std::string("txnlog"))
    {
        this->pageNo = JsonUtil::jsonParam(json_data, "pageNo", 1);
        this->pageSize = JsonUtil::jsonParam(json_data, "pageSize", 10);
    }

    void MessageTxnLogRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"pageNo", this->pageNo},
            {"pageSize", this->pageSize}};
        json_str = json.dump();
    }

    void MessageTxnLogRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"pageNo", this->pageNo},
            {"pageSize", this->pageSize},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageTxnLogResponse::MessageTxnLogResponse()
    {
        this->totalSize = 0;
        this->totalPage = 0;
        this->pageNo = 0;
        this->pageSize = 0;
        this->totalSize = 0;
        this->list = nlohmann::json::array();
    }

    MessageTxnLogResponse::MessageTxnLogResponse(const std::string& body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("totalSize"))
                json.at("totalSize").get_to(this->totalSize);
            if (json.contains("totalPage"))
                json.at("totalPage").get_to(this->totalPage);
            if (json.contains("pageNo"))
                json.at("pageNo").get_to(this->pageNo);
            if (json.contains("pageSize"))
                json.at("pageSize").get_to(this->pageSize);
            if (json.contains("list"))
                json.at("list").get_to(this->list);
        }
    }

    void MessageTxnLogResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["totalSize"] = this->totalSize;
        rJson["totalPage"] = this->totalPage;
        rJson["pageNo"] = this->pageNo;
        rJson["pageSize"] = this->pageSize;
        rJson["totalSize"] = this->totalSize;
        rJson["list"] = this->list;

        json_str = rJson.dump();
    }

    // query log
    void QueryLog::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"QueryDateTime", this->QueryDateTime},
            {"Sparql", this->Sparql},
            {"Format", this->Format},
            {"RemoteIP", this->RemoteIP},
            {"FileName", this->FileName},
            {"QueryTime", this->QueryTime},
            {"AnsNum", this->AnsNum}};
        json_str = json.dump();
    }

    void QueryLog::from_json(const nlohmann::json& json)
    {
        if (json.contains("QueryDateTime"))
            json.at("QueryDateTime").get_to(this->QueryDateTime);
        if (json.contains("Sparql"))
            json.at("Sparql").get_to(this->Sparql);
        if (json.contains("Format"))
            json.at("Format").get_to(this->Format);
        if (json.contains("RemoteIP"))
            json.at("RemoteIP").get_to(this->RemoteIP);
        if (json.contains("FileName"))
            json.at("FileName").get_to(this->FileName);
        if (json.contains("QueryTime"))
            json.at("QueryTime").get_to(this->QueryTime);
        if (json.contains("AnsNum"))
            json.at("AnsNum").get_to(this->AnsNum);
    }

    MessageQueryLogRequest::MessageQueryLogRequest(const nlohmann::json& json_data) : MessageRequest(std::string("querylog"))
    {
        this->date = JsonUtil::jsonParam(json_data, "date");
        this->pageNo = JsonUtil::jsonParam(json_data, "pageNo", 1);
        this->pageSize = JsonUtil::jsonParam(json_data, "pageSize", 10);
    }

    void MessageQueryLogRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"pageNo", this->pageNo},
            {"pageSize", this->pageSize},
            {"date", this->date}};
        json_str = json.dump();
    }

    void MessageQueryLogRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"pageNo", this->pageNo},
            {"pageSize", this->pageSize},
            {"date", this->date},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageQueryLogResponse::MessageQueryLogResponse()
    {
        this->totalSize = 0;
        this->totalPage = 0;
        this->pageNo = 0;
        this->pageSize = 0;
        this->list = nlohmann::json::array();
    }

    MessageQueryLogResponse::MessageQueryLogResponse(const std::string& body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("totalSize"))
                json.at("totalSize").get_to(this->totalSize);
            if (json.contains("totalPage"))
                json.at("totalPage").get_to(this->totalPage);
            if (json.contains("pageNo"))
                json.at("pageNo").get_to(this->pageNo);
            if (json.contains("pageSize"))
                json.at("pageSize").get_to(this->pageSize);
            if (json.contains("list"))
                json.at("list").get_to(this->list);
        }
    }

    void MessageQueryLogResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["totalSize"] = this->totalSize;
        rJson["totalPage"] = this->totalPage;
        rJson["pageNo"] = this->pageNo;
        rJson["pageSize"] = this->pageSize;
        rJson["list"] = this->list;

        json_str = rJson.dump();
    }

    void MessageQueryLogDateRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password}};
        json_str = json.dump();
    }

    void MessageQueryLogDateRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"inner", "true"}};
        json_str = json.dump();
    }

    // query log date
    void MessageQueryLogDateResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["list"] = this->list;
        json_str = rJson.dump();
    }

    // access log
    void AccessLog::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"ip", this->ip},
            {"operation", this->operation},
            {"createtime", this->createtime},
            {"code", this->code},
            {"msg", this->msg}};
        json_str = json.dump();
    }

    void AccessLog::from_json(const nlohmann::json& json)
    {
        if (json.is_object())
        {
            if (json.contains("ip"))
                json.at("ip").get_to(this->ip);
            if (json.contains("operation"))
                json.at("operation").get_to(this->operation);
            if (json.contains("createtime"))
                json.at("createtime").get_to(this->createtime);
            if (json.contains("code"))
                json.at("code").get_to(this->code);
            if (json.contains("msg"))
                json.at("msg").get_to(this->msg);
        }
    }

    MessageQueryLogDateResponse::MessageQueryLogDateResponse(const std::string& body) : MessageResponse(body) { }
    
    MessageAccessLogRequest::MessageAccessLogRequest(const nlohmann::json& json_data) : MessageRequest(std::string("accesslog"))
    {
        this->date = JsonUtil::jsonParam(json_data, "date");
        this->pageNo = JsonUtil::jsonParam(json_data, "pageNo", 1);
        this->pageSize = JsonUtil::jsonParam(json_data, "pageSize", 10);
    }

    void MessageAccessLogRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"pageNo", this->pageNo},
            {"pageSize", this->pageSize},
            {"date", this->date}};
        json_str = json.dump();
    }

    void MessageAccessLogRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"pageNo", this->pageNo},
            {"pageSize", this->pageSize},
            {"date", this->date},
            {"inner", "true"}};
        json_str = json.dump();
    }
    MessageAccessLogResponse::MessageAccessLogResponse()
    {
        this->totalSize = 0;
        this->totalPage = 0;
        this->pageNo = 0;
        this->pageSize = 0;
        this->list = nlohmann::json::array();
    }
    
    
    void MessageAccessLogResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["totalSize"] = this->totalSize;
        rJson["totalPage"] = this->totalPage;
        rJson["pageNo"] = this->pageNo;
        rJson["pageSize"] = this->pageSize;
        rJson["list"] = this->list;

        json_str = rJson.dump();
    }

    // access log date
    void MessageAccessLogDateResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["list"] = this->list;
        json_str = rJson.dump();
    }
    
    MessageAccessLogResponse::MessageAccessLogResponse(const std::string& body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("totalSize"))
                json.at("totalSize").get_to(this->totalSize);
            if (json.contains("totalPage"))
                json.at("totalPage").get_to(this->totalPage);
            if (json.contains("pageNo"))
                json.at("pageNo").get_to(this->pageNo);
            if (json.contains("pageSize"))
                json.at("pageSize").get_to(this->pageSize);
            if (json.contains("list"))
                json.at("list").get_to(this->list);
        }
    }

    void MessageAccessLogDateRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password}};
        json_str = json.dump();
    }

    void MessageAccessLogDateRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageAccessLogDateResponse::MessageAccessLogDateResponse(const std::string& body) : MessageResponse(body) { }

    // check operation state
    MessageCheckOperationStateRequest::MessageCheckOperationStateRequest(const nlohmann::json& json_data) : MessageRequest(std::string("checkOperationState"))
    {
        if(json_data.contains("operation"))
            json_data.at("operation").get_to(this->op);
    }

    void MessageCheckOperationStateRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"opt_id", this->opt_id}};
        json_str = json.dump();
    }

    void MessageCheckOperationStateRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"opt_id", this->opt_id},
            {"inner", "true"}};
        json_str = json.dump();
    }


    MessageCheckOperationStateResponse::MessageCheckOperationStateResponse(const std::string& body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("success_num"))
                json["success_num"].get_to(this->success_num);
            if (json.contains("failed_num"))
                json["failed_num"].get_to(this->failed_num);
            if (json.contains("state"))
                json["state"].get_to(this->state);
        }
    }

    void MessageCheckOperationStateResponse::toJsonString(std::string& json_str)
    {
    nlohmann::json rJson;
    toJson(rJson);
    rJson["success_num"] = this->success_num;
    rJson["failed_num"] = this->failed_num;
    rJson["state"] = this->state;
    if (this->backupfilepath.empty())
    {
        rJson["backupfilepath"] = this->backupfilepath;
    }

    json_str = rJson.dump();
    }
}