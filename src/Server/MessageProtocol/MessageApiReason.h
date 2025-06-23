#pragma once
#include "MessageApi.h"

namespace server
{
    // TODO
    struct CountInfo {
        void to_json(std::string& json_str) {};
        void from_json(const nlohmann::json &json) {};
    };


    struct Pattern {
        std::string subject;
        std::string predicate;
        std::string object;
        void to_json(std::string& json_str);
        void from_json(const nlohmann::json &json);
    };

    struct Condition {
        std::vector<Pattern> patterns;
        std::vector<std::string> filters;
        CountInfo countInfo;
        void to_json(std::string& json_str);
        void from_json(const nlohmann::json &json);
    };

    struct Return {
        std::string source;
        std::string target;
        std::string label;
        std::string value;
        void to_json(std::string& json_str);
        void from_json(const nlohmann::json &json);
    };

    struct RuleInfo {
        std::string rulename;
        std::string description;
        int isenable;
        int type;
        int logic;
        std::vector<Condition> conditions;
        Return returnInfo;
        std::string status;
        std::string insert_sparql;
        std::string delete_sparql;
        std::string createtime;
        void to_json(std::string& json_str);
        void from_json(const nlohmann::json &json);
    };


    // show user list
    struct MessageReasonManageRequest : public MessageRequest
    {
        std::string db_name;
        std::string type;
        MessageReasonManageRequest(std::string db_name, std::string type);
        MessageReasonManageRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };
    
    // gconsole use
    struct MessageAddReasonRequest : MessageReasonManageRequest
    {
        nlohmann::json ruleinfo;
        MessageAddReasonRequest(std::string db_name, nlohmann::json ruleinfo);
        MessageAddReasonRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };


    struct MessageListReasonRequest : MessageReasonManageRequest
    {
        MessageListReasonRequest(std::string db_name);
        MessageListReasonRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageCedsdReasonRequest : MessageReasonManageRequest
    {
        std::string rulename;
        MessageCedsdReasonRequest(std::string db_name, std::string type, std::string rulename);
        MessageCedsdReasonRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageReasonManageResponse : public MessageResponse
    {
        std::string type;
        nlohmann::json list;
        int num;
        std::string insert_sparql;
        std::string delete_sparql;
        std::string check_sparql;
        std::string select_sparql;
        nlohmann::json ruleinfo;
        std::string checkMsg;
        MessageReasonManageResponse(){num = 0;}
        MessageReasonManageResponse(int code, std::string msg) : MessageResponse(code, msg) { }
        MessageReasonManageResponse(std::string body); // gconsole use
        void toJsonString(std::string& json_str);
    };

}