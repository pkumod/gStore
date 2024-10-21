#include "MessageApiUser.h"
#include "../ApiProvider.h"

namespace server
{
    // show user
    void MessageShowUserRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json_str = json.dump();
    }
    
    void MessageShowUserRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", ""},
            {"inner", "true"}};
        json_str = json.dump();
    }

    void MessageShowUserResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["ResponseBody"] = nlohmann::json::array();
        if (!this->ResponseBody.empty())
        {
            nlohmann::json temp;
            for (auto &m : this->ResponseBody)
            {
                temp["username"] = m.username;
                temp["password"] = m.password;
                temp["query_privilege"] = m.query_privilege;
                temp["update_privilege"] = m.update_privilege;
                temp["load_privilege"] = m.load_privilege;
                temp["unload_privilege"] = m.unload_privilege;
                temp["backup_privilege"] = m.backup_privilege;
                temp["restore_privilege"] = m.restore_privilege;
                temp["export_privilege"] = m.export_privilege;
                json["ResponseBody"].push_back(temp);
            }
        }
        json_str = json.dump();
    }

    // user manger
    MessageUserManageRequest::MessageUserManageRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->op_username = jsonParam(json_data, "op_username");
        this->op_password = jsonParam(json_data, "op_password");
        this->type = jsonParam(json_data, "type");
    }

    void MessageUserManageRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["op_username"] = this->op_username;
        json["op_password"] = this->op_password;
        json["type"] = this->type;
        json_str = json.dump();
    }

    void MessageUserManageRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", ""},
            {"op_username", this->op_username},
            {"op_password", this->op_password},
            {"type", this->type},
            {"inner", "true"}};
        json_str = json.dump();
    }

    void MessageUserManageResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json_str = json.dump();
    }

    // user privilege manger
    MessageUserPrivilegeManageRequest::MessageUserPrivilegeManageRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->type = jsonParam(json_data, "type");
        this->op_username = jsonParam(json_data, "op_username");
        this->db_name = jsonParam(json_data, "db_name");
        this->privileges = jsonParam(json_data, "privileges");
    }

    void MessageUserPrivilegeManageRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["op_username"] = this->op_username;
        json["db_name"] = this->db_name;
        json["privileges"] = this->privileges;
        json["type"] = this->type;
        json_str = json.dump();
    }

    void MessageUserPrivilegeManageRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", ""},
            {"op_username", this->op_username},
            {"db_name", this->db_name},
            {"privileges", this->privileges},
            {"type", this->type},
            {"inner", "true"}};
        json_str = json.dump();
    }

    void MessageUserPrivilegeManageResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json_str = json.dump();
    }

    // user password
    MessageUserPasswordRequest::MessageUserPasswordRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->op_password = jsonParam(json_data, "op_password");
    }

    void MessageUserPasswordRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["op_password"] = this->op_password;
        json_str = json.dump();
    }

    void MessageUserPasswordRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", ""},
            {"op_password", this->op_password}};
        json_str = json.dump();
    }
}