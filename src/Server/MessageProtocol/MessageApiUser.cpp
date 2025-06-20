#include "MessageApiUser.h"
#include "../ApiProvider.h"

namespace gs
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
            {"username", this->username},
            {"password", this->password},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageShowUserResponse::MessageShowUserResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object() && json.contains("ResponseBody"))
        {
            nlohmann::json array = json.at("ResponseBody");
            for (const auto &info : array)
            {
                MessageShowUserResponseBody m;
                if (info.contains("username"))
                    info.at("username").get_to(m.username);
                if (info.contains("password"))
                    info.at("password").get_to(m.password);
                if (info.contains("query_privilege"))
                    info.at("query_privilege").get_to(m.query_privilege);
                if (info.contains("update_privilege"))
                    info.at("update_privilege").get_to(m.update_privilege);
                if (info.contains("load_privilege"))
                    info.at("load_privilege").get_to(m.load_privilege);
                if (info.contains("unload_privilege"))
                    info.at("unload_privilege").get_to(m.unload_privilege);
                if (info.contains("backup_privilege"))
                    info.at("backup_privilege").get_to(m.backup_privilege);
                if (info.contains("restore_privilege"))
                    info.at("restore_privilege").get_to(m.restore_privilege);
                if (info.contains("export_privilege"))    
                    info.at("export_privilege").get_to(m.export_privilege);
                this->ResponseBody.push_back(m);
            }
        }
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
    MessageUserManageRequest::MessageUserManageRequest(const nlohmann::json& json_data) : MessageRequest(json_data)
    {
        this->op_username = JsonUtil::jsonParam(json_data, "op_username");
        this->op_password = JsonUtil::jsonParam(json_data, "op_password");
        this->type = JsonUtil::jsonParam(json_data, "type");
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
            {"username", this->username},
            {"password", this->password},
            {"op_username", this->op_username},
            {"op_password", this->op_password},
            {"type", this->type},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageUserManageRequest::MessageUserManageRequest(int type, std::string username, std::string password) : MessageRequest(std::string("usermanage"))
    {
        this->type = std::to_string(type);
        this->op_username = username;
        this->op_password = password;
    }

    void MessageUserManageResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json_str = json.dump();
    }
    
    MessageUserPrivilegeManageRequest::MessageUserPrivilegeManageRequest(int type, std::string username, std::string privileges, std::string db_name) : MessageRequest(std::string("userprivilegemanage"))
    {
        this->type = std::to_string(type);
        this->op_username = username;
        this->privileges = privileges;
        this->db_name = db_name;
    }

    // user privilege manger
    MessageUserPrivilegeManageRequest::MessageUserPrivilegeManageRequest(const nlohmann::json& json_data) : MessageRequest(json_data)
    {
        this->type = JsonUtil::jsonParam(json_data, "type");
        this->op_username = JsonUtil::jsonParam(json_data, "op_username");
        this->db_name = JsonUtil::jsonParam(json_data, "db_name");
        this->privileges = JsonUtil::jsonParam(json_data, "privileges");
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
            {"username", this->username},
            {"password", this->password},
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

    MessageUserPasswordRequest::MessageUserPasswordRequest(std::string username, std::string password, std::string op_password) : MessageRequest(std::string("userpassword"))
    {
        this->username = username;
        this->password = password;
        this->op_password = op_password;
    }
    // user password
    MessageUserPasswordRequest::MessageUserPasswordRequest(const nlohmann::json& json_data) : MessageRequest(json_data)
    {
        this->username = JsonUtil::jsonParam(json_data, "username");
        this->password = JsonUtil::jsonParam(json_data, "password");
        this->op_password = JsonUtil::jsonParam(json_data, "op_password");
    }

    void MessageUserPasswordRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["username"] = this->username;
        json["password"] = this->password;
        json["op_password"] = this->op_password;
        json_str = json.dump();
    }

    void MessageUserPasswordRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"op_password", this->op_password}};
        json_str = json.dump();
    }
}