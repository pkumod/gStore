#include "MessageApiLog.h"
#include "../ApiProvider.h"

namespace server
{
    // txn log
    MessageTxnLogRequest::MessageTxnLogRequest(const rapidjson::Document& json_data)
    {
        this->pageNo = jsonParam(json_data, "pageNo", 1);
        this->pageSize = jsonParam(json_data, "pageSize", 10);
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
    MessageQueryLogRequest::MessageQueryLogRequest(const rapidjson::Document& json_data)
    {
        this->date = jsonParam(json_data, "date");
        this->pageNo = jsonParam(json_data, "pageNo", 1);
        this->pageSize = jsonParam(json_data, "pageSize", 10);
    }

    MessageQueryLogResponse::MessageQueryLogResponse()
    {
        this->totalSize = 0;
        this->totalPage = 0;
        this->pageNo = 0;
        this->pageSize = 0;
        this->totalSize = 0;
        this->list = nlohmann::json::array();
    }

    void MessageQueryLogResponse::toJsonString(std::string& json_str)
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

    // query log date
    void MessageQueryLogDateResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["list"] = this->list;
        json_str = rJson.dump();
    }

    // access log
    MessageAccessLogRequest::MessageAccessLogRequest(const rapidjson::Document& json_data)
    {
        this->date = jsonParam(json_data, "date");
        this->pageNo = jsonParam(json_data, "pageNo", 1);
        this->pageSize = jsonParam(json_data, "pageSize", 10);
    }

    MessageAccessLogResponse::MessageAccessLogResponse()
    {
        this->totalSize = 0;
        this->totalPage = 0;
        this->pageNo = 0;
        this->pageSize = 0;
        this->totalSize = 0;
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
        rJson["totalSize"] = this->totalSize;
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

    // check operation state
    MessageCheckOperationStateRequest::MessageCheckOperationStateRequest(const rapidjson::Document& json_data)
    {
        this->opt_id = jsonParam(json_data, "opt_id");
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