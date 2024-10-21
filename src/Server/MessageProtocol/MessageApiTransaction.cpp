#include "MessageApiTransaction.h"
#include "../ApiProvider.h"

namespace server
{
    // begin
    MessageBeginRequest::MessageBeginRequest(const rapidjson::Document& json_data)
    {
        this->db_name = db_name = jsonParam(json_data, "db_name");
        this->isolevel = jsonParam(json_data, "isolevel");
    }

    void MessageBeginResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["TID"] = this->TID;
        json_str = json.dump();
    }

    // tquery
    MessageTqueryRequest::MessageTqueryRequest(const rapidjson::Document& json_data)
    {
        this->db_name = jsonParam(json_data, "db_name");
        this->tid = jsonParam(json_data, "tid");
        this->sparql = jsonParam(json_data, "sparql");
    }

    void MessageTqueryResponse::toJsonString(std::string& json_str)
    {
        if (!result.empty())
        {
            this->query_json["result"] = this->result;
        }
        toJson(this->query_json);
        this->query_json["AnsNum"] = this->ansNum;
        json_str = this->query_json.dump();
        // SLOG_TRACE("MessageTqueryResponse:" << json_str);
    }

    // commit
    MessageCommitRequest::MessageCommitRequest(const rapidjson::Document& json_data)
    {
        this->db_name = jsonParam(json_data, "db_name");
        this->tid = jsonParam(json_data, "tid");
    }

    // rollback
    MessageRollbackRequest::MessageRollbackRequest(const rapidjson::Document& json_data)
    {
        this->db_name = jsonParam(json_data, "db_name");
        this->tid = jsonParam(json_data, "tid");
    }
}