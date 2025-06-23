#pragma once
#include "MessageApi.h"

namespace server
{
    // show user list
    struct MessageShowUserRequest : public MessageRequest
    {
        MessageShowUserRequest() : MessageRequest(std::string("showuser")) {};
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageShowUserResponseBody
    {
        std::string username;
        std::string password;
        std::string query_privilege;
        std::string update_privilege;
        std::string load_privilege;
        std::string unload_privilege;
        std::string backup_privilege;
        std::string restore_privilege;
        std::string export_privilege;
    };

    struct MessageShowUserResponse : public MessageResponse
    {
        std::vector<MessageShowUserResponseBody> ResponseBody;
        MessageShowUserResponse(){};
        MessageShowUserResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageShowUserResponse(std::string body);
        void toJsonString(std::string& json_str);
    };


    // user manger
    struct MessageUserManageRequest : public MessageRequest
    {
        std::string type;
        std::string op_username;
        std::string op_password;
        MessageUserManageRequest(int type, std::string username, std::string password);
        MessageUserManageRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageUserManageResponse : public MessageResponse
    {
        MessageUserManageResponse(){};
        MessageUserManageResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageUserManageResponse(std::string body) : MessageResponse(body) {};
        void toJsonString(std::string& json_str);
    };

    // user privilege manger
    struct MessageUserPrivilegeManageRequest : public MessageRequest
    {
        std::string type;
        std::string op_username;
        std::string privileges;
        std::string db_name;
        MessageUserPrivilegeManageRequest(int type, std::string username, std::string privileges, std::string db_name);
        MessageUserPrivilegeManageRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageUserPrivilegeManageResponse : public MessageResponse
    {
        MessageUserPrivilegeManageResponse(){};
        MessageUserPrivilegeManageResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageUserPrivilegeManageResponse(std::string body) : MessageResponse(body) {};
        void toJsonString(std::string& json_str);
    };

    // user password
    struct MessageUserPasswordRequest : public MessageRequest
    {
        std::string username;
        std::string password;
        std::string op_password;
        MessageUserPasswordRequest(std::string username, std::string password, std::string op_password);
        MessageUserPasswordRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageUserPasswordResponse : public MessageResponse
    {
        MessageUserPasswordResponse(){}
        MessageUserPasswordResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageUserPasswordResponse(std::string body) : MessageResponse(body) {};
    };
}