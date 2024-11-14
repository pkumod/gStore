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

    void MessageBeginRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"username", this->username},
            {"password", this->password},
            {"operation", this->op},
            {"db_name", this->db_name},
            {"isolevel", this->isolevel}};
        json_str = json.dump();
    }

    void MessageBeginRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"username", this->username},
            {"password", this->password},
            {"operation", this->op},
            {"db_name", this->db_name},
            {"isolevel", this->isolevel},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageBeginResponse::MessageBeginResponse(const std::string& body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("TID"))
                json.at("TID").get_to(TID);
        }
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

    void MessageTqueryRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"username", this->username},
            {"password", this->password},
            {"operation", this->op},
            {"db_name", this->db_name},
            {"tid", this->tid},
            {"sparql", this->sparql}};
        json_str = json.dump();
    }

    void MessageTqueryRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"username", this->username},
            {"password", this->password},
            {"operation", this->op},
            {"db_name", this->db_name},
            {"tid", this->tid},
            {"sparql", this->sparql},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageTqueryResponse::MessageTqueryResponse(const std::string& body) : MessageResponse(body)
    {
        if (json.is_object())
        {

        }
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

    void MessageCommitRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"username", this->username},
            {"password", this->password},
            {"operation", this->op},
            {"db_name", this->db_name},
            {"tid", this->tid}};
        json_str = json.dump();
    }

    void MessageCommitRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"username", this->username},
            {"password", this->password},
            {"operation", this->op},
            {"db_name", this->db_name},
            {"tid", this->tid},
            {"inner", "true"}};
        json_str = json.dump();
    }

    void MessageCommitResponse::toJsonString(std::string& json_str)
    {

    }

    // rollback
    MessageRollbackRequest::MessageRollbackRequest(const rapidjson::Document& json_data)
    {
        this->db_name = jsonParam(json_data, "db_name");
        this->tid = jsonParam(json_data, "tid");
    }

    void MessageRollbackRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"username", this->username},
            {"password", this->password},
            {"operation", this->op},
            {"db_name", this->db_name},
            {"tid", this->tid}};
        json_str = json.dump();
    }

    void MessageRollbackRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"username", this->username},
            {"password", this->password},
            {"operation", this->op},
            {"db_name", this->db_name},
            {"tid", this->tid},
            {"inner", "true"}};
        json_str = json.dump();
    }

    void MessageRollbackResponse::toJsonString(std::string& json_str)
    {

    }
}