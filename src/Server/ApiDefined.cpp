#include "ApiDefined.h"
#include "ApiProvider.h"

namespace server
{
    // protocol
    MessageResponse::MessageResponse()
    {
        status_code = StatusOK;
        status_msg  = "";
    }

    void MessageResponse::toJson(nlohmann::json& json)
    {
        json["StatusCode"]  = status_code;
        json["StatusMsg"]   = status_msg;
    }

    // load
    MessageLoadRequest::MessageLoadRequest(const rapidjson::Document& json_data)
    {
        this->db_name = jsonParam(json_data, "db_name");
        this->csr = jsonBoolParam(json_data, "csr", false);
    }

    MessageLoadResponse::MessageLoadResponse()
    {
        this->csr = "";
    }

    void MessageLoadResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["csr"] = this->csr;
        json_str = json.dump();
        SLOG_TRACE("MessageLoadResponse:" << json_str);
    }

    // batch insert
    MessageBatchInsertRequest::MessageBatchInsertRequest(const rapidjson::Document& json_data)
    {
        this->db_name = jsonParam(json_data, "db_name", "");
        this->file = jsonParam(json_data, "file");
        this->dir = jsonParam(json_data, "dir");
        this->async = jsonBoolParam(json_data, "async", false);
        this->callback = jsonParam(json_data, "callback");
    }

    MessageBatchInsertResponse::MessageBatchInsertResponse()
    {
        this->success_num = 0;
        this->failed_num = 0;
        this->opt_id = "";
    }

    void MessageBatchInsertResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["success_num"] = this->success_num;
        json["failed_num"] = this->failed_num;
        json["opt_id"] = this->opt_id;
        json_str = json.dump();
        SLOG_TRACE("MessageLoadResponse:" << json_str);
    }
}