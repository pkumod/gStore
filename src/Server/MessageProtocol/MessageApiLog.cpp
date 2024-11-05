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
        if (hasJsonParam(json, "db_name"))
            this->db_name = jsonParam(json, "db_name");
        if (hasJsonParam(json, "TID"))
            this->TID = jsonParam(json, "TID");
        if (hasJsonParam(json, "user"))
            this->user = jsonParam(json, "user");
        if (hasJsonParam(json, "state"))
            this->state = jsonParam(json, "state");
        if (hasJsonParam(json, "begin_time"))
            this->begin_time = jsonParam(json, "begin_time");
        if (hasJsonParam(json, "end_time"))
            this->end_time = jsonParam(json, "end_time");
    }

    MessageTxnLogRequest::MessageTxnLogRequest(const rapidjson::Document& json_data) : MessageRequest(std::string("txnlog"))
    {
        this->pageNo = jsonParam(json_data, "pageNo", 1);
        this->pageSize = jsonParam(json_data, "pageSize", 10);
    }

    void MessageTxnLogRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"pageNo", this->pageNo},
            {"pageSize", this->pageSize}};
        json_str = json.dump();
    }

    void MessageTxnLogRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
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
        if (hasJsonParam(json, "QueryDateTime"))
            this->QueryDateTime = jsonParam(json, "QueryDateTime");
        if (hasJsonParam(json, "Sparql"))
            this->Sparql = jsonParam(json, "Sparql");
        if (hasJsonParam(json, "Format"))
            this->Format = jsonParam(json, "Format");
        if (hasJsonParam(json, "RemoteIP"))
            this->RemoteIP = jsonParam(json, "RemoteIP");
        if (hasJsonParam(json, "FileName"))
            this->FileName = jsonParam(json, "FileName");
        if (hasJsonParam(json, "QueryTime"))
            this->QueryTime = jsonParam(json, "QueryTime", 0);
        if (hasJsonParam(json, "AnsNum"))
            this->AnsNum = jsonParam(json, "AnsNum", 0);
    }

    MessageQueryLogRequest::MessageQueryLogRequest(const rapidjson::Document& json_data) : MessageRequest(std::string("querylog"))
    {
        this->date = jsonParam(json_data, "date");
        this->pageNo = jsonParam(json_data, "pageNo", 1);
        this->pageSize = jsonParam(json_data, "pageSize", 10);
    }

    void MessageQueryLogRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"pageNo", this->pageNo},
            {"pageSize", this->pageSize},
            {"date", this->date}};
        json_str = json.dump();
    }

    void MessageQueryLogRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
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
            {"username", "root"},
            {"password", "123456"}};
        json_str = json.dump();
    }

    void MessageQueryLogDateRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
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
                this->ip = jsonParam(json, "ip");
            if (json.contains("operation"))
                this->ip = jsonParam(json, "operation");
            if (json.contains("createtime"))
                this->ip = jsonParam(json, "createtime");
            if (json.contains("code"))
                this->ip = jsonParam(json, "code");
            if (json.contains("msg"))
                this->ip = jsonParam(json, "msg");
        }
    }

    MessageQueryLogDateResponse::MessageQueryLogDateResponse(const std::string& body) : MessageResponse(body) { }
    
    MessageAccessLogRequest::MessageAccessLogRequest(const rapidjson::Document& json_data) : MessageRequest(std::string("accesslog"))
    {
        this->date = jsonParam(json_data, "date");
        this->pageNo = jsonParam(json_data, "pageNo", 1);
        this->pageSize = jsonParam(json_data, "pageSize", 10);
    }

    void MessageAccessLogRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"pageNo", this->pageNo},
            {"pageSize", this->pageSize},
            {"date", this->date}};
        json_str = json.dump();
    }

    void MessageAccessLogRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
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
            {"username", "root"},
            {"password", "123456"}};
        json_str = json.dump();
    }

    void MessageAccessLogDateRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageAccessLogDateResponse::MessageAccessLogDateResponse(const std::string& body) : MessageResponse(body) { }

    // check operation state
    MessageCheckOperationStateRequest::MessageCheckOperationStateRequest(const rapidjson::Document& json_data) : MessageRequest(std::string("checkOperationState"))
    {
        this->opt_id = jsonParam(json_data, "opt_id");
    }

    void MessageCheckOperationStateRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"opt_id", this->opt_id}};
        json_str = json.dump();
    }

    void MessageCheckOperationStateRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"opt_id", this->opt_id},
            {"inner", "true"}};
        json_str = json.dump();
    }


    MessageCheckOperationStateResponse::MessageCheckOperationStateResponse(const std::string& body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("success_num"))
                this->success_num = jsonParam(json, "success_num", 0);
            if (json.contains("failed_num"))
                this->failed_num = jsonParam(json, "failed_num", 0);
            if (json.contains("state"))
                this->state = jsonParam(json, "state", 0);
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