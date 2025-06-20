#include "MessageApiUpdate.h"
#include "../ApiProvider.h"

namespace gs
{
    // build db
    MessageBuildRequest::MessageBuildRequest(std::string db_name, std::string db_path) : MessageRequest(std::string("build"))
    {
        this->db_name = db_name;
        this->async = false;
        this->remote = false;
        if (!db_path.empty()) {
            this->db_path.push_back(db_path);
        }
    }

    MessageBuildRequest::MessageBuildRequest(std::string username, std::string password, std::string db_name, std::string db_path) : MessageRequest("build", username, password)
    {
        this->db_name = db_name;
        this->async = false;
        this->remote = false;
        if (!db_path.empty()) {
            this->db_path.push_back(db_path);
        }
    }

    MessageBuildRequest::MessageBuildRequest(const nlohmann::json& json_data) : MessageRequest(json_data)
    {
        this->db_name = JsonUtil::jsonParam(json_data, "db_name", "");
        if (json_data.contains("db_path"))
        {
            if (json_data["db_path"].is_array()) {
                JsonUtil::jsonArrayParam(json_data, "db_path", this->db_path);
            } else {
                std::string db_path = JsonUtil::jsonParam(json_data, "db_path", "");
                if (!db_path.empty()) {
                    this->db_path.push_back(db_path);
                }
            }
        }
        this->async = JsonUtil::jsonBoolParam(json_data, "async", false);
        this->remote = JsonUtil::jsonBoolParam(json_data, "remote", false);
        this->callback = JsonUtil::jsonParam(json_data, "callback");
    }

    void MessageBuildRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"remote", this->remote},
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
            {"remote", this->remote},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageBuildResponse::MessageBuildResponse()
    {
        this->successNum = 0;
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
        if (!this->opt_id.empty())
            json["opt_id"] = this->opt_id;
        else
        {
            json["failed_num"] = this->failed_num;
            json["success_num"] = this->successNum;
        }
        json_str = json.dump();
    }

    // drop db
    MessageDropRequest::MessageDropRequest(std::string db_name, bool is_backup):MessageRequest(std::string("drop"))
    {
        this->db_name = db_name;
        this->is_backup = is_backup;
    }

    MessageDropRequest::MessageDropRequest(std::string username, std::string password, std::string db_name, bool is_backup) : MessageRequest("drop", username, password)
    {
        this->db_name = db_name;
        this->is_backup = is_backup;
    }

    MessageDropRequest::MessageDropRequest(const nlohmann::json& json_data):MessageRequest(json_data)
    {
        this->db_name = JsonUtil::jsonParam(json_data, "db_name");
        this->is_backup = JsonUtil::jsonBoolParam(json_data, "is_backup", true);
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
    MessageQueryRequest::MessageQueryRequest(std::string db_name,  std::string sparql, std::string format, bool async) : MessageRequest(std::string("query"))
    {
        this->db_name = db_name;
        this->sparql = sparql;
        this->format = format;
        this->async = async;
    }

    MessageQueryRequest::MessageQueryRequest(std::string username, std::string password, std::string db_name, std::string sparql, std::string format, bool async) : MessageRequest("query", username, password)
    {
        this->db_name = db_name;
        this->sparql = sparql;
        this->format = format;
        this->async = async;
    }

    MessageQueryRequest::MessageQueryRequest(const nlohmann::json& json_data) : MessageRequest(json_data)
    {
        this->db_name = JsonUtil::jsonParam(json_data, "db_name");
        this->format = JsonUtil::jsonParam(json_data, "format", "json");
        this->sparql = JsonUtil::jsonParam(json_data, "sparql");
        this->callback = JsonUtil::jsonParam(json_data, "callback");
        this->async = JsonUtil::jsonBoolParam(json_data, "async", false);
    }

    MessageQueryRequest::MessageQueryRequest(const MessageQueryRequest& other) : MessageRequest(other)
    {
        this->db_name = other.db_name;
        this->format = other.format;
        this->sparql = other.sparql;
        this->callback = other.callback;
        this->async = other.async;
    }

    MessageQueryRequest &MessageQueryRequest::operator=(const MessageQueryRequest& other) 
    {
        if (this != &other)
        {
            MessageRequest::operator=(other);
            this->db_name = other.db_name;
            this->format = other.format;
            this->sparql = other.sparql;
            this->callback = other.callback;
            this->async = other.async;
        }
        return *this;
    }

    void MessageQueryRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"sparql", this->sparql},
            {"callback", this->callback},
            {"async", this->async},
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
            {"async", "false"},
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
                nlohmann::json head = json.at("head");
                if (head.is_array())
                    head.get_to(this->head);
                else if (head.is_object() && head.contains("vars"))
                    head.at("vars").get_to(this->head);
            }
            if (json.contains("results") && !this->head.empty())
            {
                nlohmann::json results = json.at("results");
                if (results.is_object())
                {   
                    nlohmann::json bindings = results.at("bindings");
                    for (const auto &result : bindings)
                    {
                        this->results.push_back({});
                        std::vector<std::string> &result_part = this->results.back();
                        for (const auto &var : this->head)
                        {
                            result_part.push_back(result.at(var).at("value"));
                        }
                    }
                }
                else if (results.is_array())
                {
                    results.get_to(this->results);
                }
            }
        }
    }

    MessageQueryResponse::MessageQueryResponse()
    {
        this->ansNum = 0;
        this->outputLimit = -1;
        this->isUpdate = false;
        this->is_pfn = false;
    }

    void MessageQueryResponse::toJson(nlohmann::json& json)
    {
        if (!this->isUpdate)
        {
            json = this->query_json;
            if (!this->is_pfn)
                json["OutputLimit"] = this->outputLimit;
            if (!this->fileName.empty())
            {
                json["FileName"] = this->fileName;
            }
        }
        json["StatusCode"] = this->StatusCode;
        json["StatusMsg"] = this->StatusMsg;
        json["QueryTime"] = this->queryTime;
        if (!this->is_pfn)
        {
            json["AnsNum"] = this->ansNum;
            json["ThreadId"] = this->threadId;
        }
    }

    void MessageQueryResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json_data;
        this->toJson(json_data);
        json_str = json_data.dump();
    }

    void MessageQueryResponse::toAsyncJsonString(std::string& json_str)
    {
        nlohmann::json j;
        j["opt_id"] = this->opt_id;
        j["StatusCode"] = this->StatusCode;
        j["StatusMsg"] = this->StatusMsg;
        json_str = j.dump();
    }

    // batch insert
    MessageBatchInsertRequest::MessageBatchInsertRequest(std::string db_name, std::string file) : MessageRequest(std::string("batchInsert"))
    {
        this->db_name = db_name;
        this->async = false;
        this->remote = false;
        if (!file.empty())
            this->file.push_back(file);
    }
    MessageBatchInsertRequest::MessageBatchInsertRequest(std::string username, std::string password,std::string db_name, std::string file) : MessageRequest("batchInsert", username, password)
    {
        this->db_name = db_name;
        this->async = false;
        this->remote = false;
        if (!file.empty())
            this->file.push_back(file);
    }
    void MessageBatchInsertRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"file", this->file}};
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
            {"remote", this->remote},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageBatchInsertRequest::MessageBatchInsertRequest(const nlohmann::json& json_data) : MessageRequest(json_data)
    {
        this->db_name = JsonUtil::jsonParam(json_data, "db_name", "");
        if (json_data.contains("file"))
        {
            if (json_data["file"].is_array())
            {
                JsonUtil::jsonArrayParam(json_data, "file", this->file);
            }
            else
            {
                std::string value = JsonUtil::jsonParam(json_data, "file", "");
                if (!value.empty())
                    this->file.push_back(value);
            }
        }
        this->async = JsonUtil::jsonBoolParam(json_data, "async", false);
        this->remote = JsonUtil::jsonBoolParam(json_data, "remote", false);
        this->callback = JsonUtil::jsonParam(json_data, "callback");
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
        if (!this->opt_id.empty())
            json["opt_id"] = this->opt_id;
        else
        {
            json["success_num"] = this->successNum;
            json["failed_num"] = this->failedNum;
        }
        json_str = json.dump();
    }

    // batch remove
    MessageBatchRemoveRequest::MessageBatchRemoveRequest(std::string db_name,  std::string file) : MessageRequest(std::string("batchRemove"))
    {
        this->db_name = db_name;
        this->file = file;
        this->remote = false;
        this->async = false;
    }

    MessageBatchRemoveRequest::MessageBatchRemoveRequest(const nlohmann::json& json_data) : MessageRequest(json_data)
    {
        this->db_name = JsonUtil::jsonParam(json_data, "db_name", "");
        this->file = JsonUtil::jsonParam(json_data, "file", "");
        this->remote = JsonUtil::jsonBoolParam(json_data, "remote", false);
        this->async = JsonUtil::jsonBoolParam(json_data, "async", false);
        this->callback = JsonUtil::jsonParam(json_data, "callback");
    }

    MessageBatchRemoveRequest::MessageBatchRemoveRequest(std::string username, std::string password,std::string db_name, std::string file) : MessageRequest("batchRemove", username, password) {
        this->db_name = db_name;
        this->file = file;
        this->remote = false;
        this->async = false;
    }

    void MessageBatchRemoveRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"remote", this->remote},
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
            {"remote", this->remote},
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
        if (!this->opt_id.empty())
            json["opt_id"] = this->opt_id;
        else
        {
            json["success_num"] = this->successNum;
            json["failed_num"] = this->failedNum;
        }
        json_str = json.dump();
    }

    // checkPoint
    MessageCheckPointRequest::MessageCheckPointRequest(const nlohmann::json& json_data) : MessageRequest(std::string("checkpoint"))
    {
        this->db_name = JsonUtil::jsonParam(json_data, "db_name");
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