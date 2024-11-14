#include "MessageApiUpdate.h"
#include "../ApiProvider.h"

namespace server
{
    // build db
    MessageBuildRequest::MessageBuildRequest(std::string db_name, std::string db_path) : MessageRequest(std::string("build"))
    {
        this->db_name = db_name;
        this->db_path = db_path;
    }

    MessageBuildRequest::MessageBuildRequest(std::string username, std::string password, std::string db_name, std::string db_path) : MessageRequest("build", username, password)
    {
        this->db_name = db_name;
        this->db_path = db_path;
    }

    MessageBuildRequest::MessageBuildRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->db_name = jsonParam(json_data, "db_name", "");
        this->db_path = jsonParam(json_data, "db_path");
        this->async = jsonBoolParam(json_data, "async", false);
        this->callback = jsonParam(json_data, "callback");
    }

    void MessageBuildRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"db_path", this->db_path}};
        json_str = json.dump();
    }

    void MessageBuildRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", ""},
            {"db_name", this->db_name},
            {"db_path", this->db_path},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageBuildResponse::MessageBuildResponse()
    {
        this->failed_num = 0;
        this->opt_id = "";
    }

    MessageBuildResponse::MessageBuildResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object() && json.contains("failed_num"))
        {
            json.at("failed_num").get_to(this->failed_num);
        }
        else
        {
            failed_num = 0;
        }
    }

    void MessageBuildResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["failed_num"] = this->failed_num;
        json["opt_id"] = this->opt_id;
        json_str = json.dump();
    }

    // drop db
    MessageDropRequest::MessageDropRequest(std::string db_name, std::string is_backup) : MessageRequest(std::string("drop"))
    {
        this->db_name = db_name;
        this->is_backup = is_backup;
    }

    MessageDropRequest::MessageDropRequest(std::string username, std::string password, std::string db_name, std::string is_backup) : MessageRequest("drop", username, password)
    {
        this->db_name = db_name;
        this->is_backup = is_backup;
    }

    MessageDropRequest::MessageDropRequest(const rapidjson::Document& json_data)
    {
        this->db_name = jsonParam(json_data, "db_name");
        this->is_backup = jsonBoolParam(json_data, "is_backup", true);
    }

    void MessageDropRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"is_backup", this->is_backup}};
        json_str = json.dump();
    }

    void MessageDropRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", ""},
            {"db_name", this->db_name},
            {"is_backup", this->is_backup},
            {"inner", "true"}};
        json_str = json.dump();
    }

    void MessageDropResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json_str = json.dump();
    }

    // query
    MessageQueryRequest::MessageQueryRequest(std::string db_name,  std::string sparql, std::string format) : MessageRequest(std::string("query"))
    {
        this->db_name = db_name;
        this->sparql = sparql;
        this->format = format;
    }

    MessageQueryRequest::MessageQueryRequest(std::string username, std::string password, std::string db_name, std::string sparql, std::string format) : MessageRequest("query", username, password)
    {
        this->db_name = db_name;
        this->sparql = sparql;
        this->format = format;
    }

    MessageQueryRequest::MessageQueryRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->db_name = jsonParam(json_data, "db_name");
        this->format = jsonParam(json_data, "format", "json");
        this->sparql = jsonParam(json_data, "sparql");
    }

    void MessageQueryRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"sparql", this->sparql},
            {"format", this->format}};
        json_str = json.dump();
    }

    void MessageQueryRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", ""},
            {"db_name", this->db_name},
            {"sparql", this->sparql},
            {"format", this->format},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageQueryResponse::MessageQueryResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("AnsNum"))
                json.at("AnsNum").get_to(this->ansNum);
            if (json.contains("OutputLimit"))
                json.at("OutputLimit").get_to(this->outputLimit);
            if (json.contains("QueryTime"))
                json.at("QueryTime").get_to(this->queryTime);
            if (json.contains("ThreadId"))
                json.at("ThreadId").get_to(this->threadId);
            if (json.contains("head"))
            {
                if (json.at("head").contains("vars"))
                    json.at("head").at("vars").get_to(this->head);
                else 
                    json.at("head").get_to(this->head);
            }
            if (json.contains("results") && !this->head.empty())
            {
                nlohmann::json results;
                if (json.at("results").contains("bindings"))
                    results = json.at("results").at("bindings");
                else
                    results = json.at("results");
                for (const auto &result : results)
                {
                    if (!result.contains(this->head[0]))
                    {
                        this->results.push_back(result);
                    }
                    else 
                    {
                        this->results.push_back({});
                        std::vector<std::string> &result_part = this->results.back();

                        for (const auto &var : this->head)
                        {
                            if (result.contains(var) && result.at(var).contains("value"))
                                result_part.push_back(result.at(var).at("value"));
                        }
                    }
                }
            }
        }
    }

    MessageQueryResponse::MessageQueryResponse()
    {
        this->ansNum = 0;
        this->outputLimit = -1;
        this->isUpdate = false;
    }

    void MessageQueryResponse::toJsonString(std::string& json_str)
    {
        if (!this->isUpdate)
        {
            toJson(this->query_json);
            this->query_json["AnsNum"] = this->ansNum;
            this->query_json["ThreadId"] = this->threadId;
            this->query_json["QueryTime"] = this->queryTime;
            this->query_json["OutputLimit"] = this->outputLimit;
            if (!this->fileName.empty())
            {
                this->query_json["FileName"] = this->fileName;
            }
            json_str = this->query_json.dump();
        }
        else
        {
            nlohmann::json json;
            toJson(json);
            json["AnsNum"] = this->ansNum;
            json["ThreadId"] = this->threadId;
            json["QueryTime"] = this->queryTime;
            json_str = json.dump();
        }
    }

    // batch insert
    MessageBatchInsertRequest::MessageBatchInsertRequest(std::string db_name, std::string file, std::string dir) : MessageRequest(std::string("batchInsert"))
    {
        this->db_name = db_name;
        this->file = file;
        this->dir = dir;
    }
    MessageBatchInsertRequest::MessageBatchInsertRequest(std::string username, std::string password,std::string db_name, std::string file, std::string dir) : MessageRequest("batchInsert", username, password)
    {
        this->db_name = db_name;
        this->file = file;
        this->dir = dir;
    }
    void MessageBatchInsertRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"file", this->file},
            {"dir", this->dir}};
        json_str = json.dump();
    }
    void MessageBatchInsertRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"file", this->file},
            {"dir", this->dir},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageBatchInsertRequest::MessageBatchInsertRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->db_name = jsonParam(json_data, "db_name", "");
        this->file = jsonParam(json_data, "file");
        this->dir = jsonParam(json_data, "dir");
        this->async = jsonBoolParam(json_data, "async", false);
        this->callback = jsonParam(json_data, "callback");
    }

    MessageBatchInsertResponse::MessageBatchInsertResponse()
    {
        this->successNum = 0;
        this->failedNum = 0;
        this->opt_id = "";
    }

    MessageBatchInsertResponse::MessageBatchInsertResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("success_num"))
                json.at("success_num").get_to(this->successNum);
            if (json.contains("failed_num"))
                json.at("failed_num").get_to(this->failedNum);
        }
    }

    void MessageBatchInsertResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["success_num"] = this->successNum;
        json["failed_num"] = this->failedNum;
        json["opt_id"] = this->opt_id;
        json_str = json.dump();
    }

    // batch remove
    MessageBatchRemoveRequest::MessageBatchRemoveRequest(std::string db_name,  std::string file) : MessageRequest(std::string("batchRemove"))
    {
        this->db_name = db_name;
        this->file = file;
    }

    MessageBatchRemoveRequest::MessageBatchRemoveRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->db_name = jsonParam(json_data, "db_name", "");
        this->file = jsonParam(json_data, "file");
        this->async = jsonBoolParam(json_data, "async", false);
        this->callback = jsonParam(json_data, "callback");
    }

    MessageBatchRemoveRequest::MessageBatchRemoveRequest(std::string username, std::string password,std::string db_name, std::string file) : MessageRequest("batchRemove", username, password) {
        this->db_name = db_name;
        this->file = file;
    }

    void MessageBatchRemoveRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"file", this->file}};
        json_str = json.dump();
    }

    void MessageBatchRemoveRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"file", this->file},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageBatchRemoveResponse::MessageBatchRemoveResponse()
    {
        this->successNum = 0;
        this->failedNum = 0;
        this->opt_id = "";
    }

    MessageBatchRemoveResponse::MessageBatchRemoveResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("success_num"))
                json.at("success_num").get_to(this->successNum);
            if (json.contains("failed_num"))
                json.at("failed_num").get_to(this->failedNum);
        }
    }

    void MessageBatchRemoveResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["success_num"] = this->successNum;
        json["failed_num"] = this->failedNum;
        json["opt_id"] = this->opt_id;
        json_str = json.dump();
    }

    // checkPoint
    MessageCheckPointRequest::MessageCheckPointRequest(const rapidjson::Document& json_data) : MessageRequest(std::string("checkpoint"))
    {
        this->db_name = jsonParam(json_data, "db_name");
    }

    void MessageCheckPointRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name}};
        json_str = json.dump();
    }
    void MessageCheckPointRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"inner", "true"}};
        json_str = json.dump();
    }
}