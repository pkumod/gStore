#include "MessageApiReason.h"
#include "../ApiProvider.h"

namespace server
{
    MessageReasonManageRequest::MessageReasonManageRequest(std::string db_name, std::string type) : MessageRequest(std::string("reasonManage"))
    {
        this->db_name = db_name;
        this->type = type;
    }
    MessageReasonManageRequest::MessageReasonManageRequest(const rapidjson::Document& json_data)
    {
        this->db_name = jsonParam(json_data, "db_name");
        this->type = jsonParam(json_data, "type");
    }
    void MessageReasonManageRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"db_name", this->db_name},
            {"type", this->type}};
        json_str = json.dump();
    }
    void MessageReasonManageRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"db_name", this->db_name},
            {"type", this->type},
            {"inner", "true"}};
        json_str = json.dump();
    }

    void Pattern::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"subject", this->subject},
            {"predicate", this->predicate},
            {"object", this->object}};
        json_str = json.dump();
    }

    void Pattern::from_json(const nlohmann::json &json)
    {
        if (hasJsonParam(json, "subject"))
            this->subject = jsonParam(json, "subject");
        if (hasJsonParam(json, "predicate"))
            this->predicate = jsonParam(json, "predicate");
        if (hasJsonParam(json, "object"))
            this->object = jsonParam(json, "object");
    }

    void Condition::to_json(std::string& json_str)
    {

        nlohmann::json json;
        json["patterns"] = nlohmann::json::array();
        for (auto &p : this->patterns)
        {
            std::string json_str;
            p.to_json(json_str);
            json["patterns"].push_back(std::move(json_str));
        }

        json["filters"] = nlohmann::json::array();
        for (auto f : this->filters)
        {
            json["filters"].push_back(f);
        }

        std::string countInfo;
        this->countInfo.to_json(countInfo);
        json["countInfo"] = countInfo;

        json_str = json.dump();
    }

    void Condition::from_json(const nlohmann::json &json)
    {
        if (hasJsonParam(json, "patterns"))
        {
            for (auto pattern_json : json["patterns"])
            {
                Pattern p;
                p.from_json(pattern_json);
                this->patterns.push_back(std::move(p));
            }
        }

        if (hasJsonParam(json, "filters"))
        {
            for (auto filter_json : json["filters"])
            {
                this->filters.push_back(filter_json);
            }
        }

        if (hasJsonParam(json, "countInfo"))
            this->countInfo.from_json(json["countInfo"]);
    }

    void Return::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"source", this->source},
            {"target", this->target},
            {"label", this->label},
            {"value", this->value}};
        json_str = json.dump();
    }

    void Return::from_json(const nlohmann::json &json)
    {
        if (hasJsonParam(json, "source"))
            this->source = jsonParam(json, "source");
        if (hasJsonParam(json, "target"))    
            this->target = jsonParam(json, "target");
        if (hasJsonParam(json, "label"))
            this->label = jsonParam(json, "label");
        if (hasJsonParam(json, "value"))
            this->value = jsonParam(json, "value");
    }

    void RuleInfo::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"rulename", this->rulename},
            {"description", this->description},
            {"isenable", this->isenable},
            {"type", this->type},
            {"logic", logic},
            {"status", this->status},
            {"insert_sparql", this->insert_sparql},
            {"delete_sparql", this->delete_sparql},
            {"createtime", this->createtime}};
        
        json["condition"] = nlohmann::json::array();
        for (auto &c : this->conditions) {
            std::string condition_str;
            c.to_json(condition_str);
            json["condition"].push_back(std::move(condition_str));
        }

        std::string return_str;
        this->returnInfo.to_json(return_str);
        json["return"] = std::move(return_str);

        json_str = json.dump();
    }

    void RuleInfo::from_json(const nlohmann::json &json)
    {
        if (hasJsonParam(json, "rulename"))
            this->rulename = jsonParam(json, "rulename");
        if (hasJsonParam(json, "description"))
            this->description = jsonParam(json, "description");
        if (hasJsonParam(json, "isenable"))
            this->isenable = jsonParam(json, "isenable", 0);
        if (hasJsonParam(json, "type"))
            this->type = jsonParam(json, "type", 0);
        if (hasJsonParam(json, "logic"))
            this->logic = jsonParam(json, "logic", 0);
        if (hasJsonParam(json, "status"))
            this->status = jsonParam(json, "status");
        if (hasJsonParam(json, "insert_sparql"))
            this->insert_sparql = jsonParam(json, "insert_sparql");
        if (hasJsonParam(json, "delete_sparql"))
            this->delete_sparql = jsonParam(json, "delete_sparql");
        if (hasJsonParam(json, "createtime"))
            this->createtime = jsonParam(json, "createtime");

        if (hasJsonParam(json, "conditions"))
        {
            for (auto &condition_json : json.at("conditions"))
            {
                Condition condition;
                condition.from_json(condition_json);
                conditions.push_back(std::move(condition));
            }
        }

        if (hasJsonParam(json, "return"))
        {
            Return ret;
            ret.from_json(jsonParam(json, "return"));
            this->returnInfo = std::move(ret);
        }
    }

    MessageAddReasonRequest::MessageAddReasonRequest(std::string db_name, nlohmann::json ruleinfo) : MessageReasonManageRequest(db_name, "1")
    {
        this->ruleinfo = ruleinfo;
    }
    MessageAddReasonRequest::MessageAddReasonRequest(const rapidjson::Document& json_data) : MessageReasonManageRequest(json_data)
    {
        std::string ruleinfo;
        ruleinfo = jsonParam(json_data, "ruleinfo");
        this->ruleinfo = nlohmann::json::parse(ruleinfo);
    }
    void MessageAddReasonRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"db_name", this->db_name},
            {"type", this->type},
        };        
        json["ruleinfo"] = this->ruleinfo;

        json_str = json.dump();
    }
    void MessageAddReasonRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"db_name", this->db_name},
            {"type", this->type},
            {"inner", "true"}};
        json["ruleinfo"] = this->ruleinfo;
        
        json_str = json.dump();
    }

    MessageListReasonRequest::MessageListReasonRequest(std::string db_name) : MessageReasonManageRequest(db_name, "2") { };
    MessageListReasonRequest::MessageListReasonRequest(const rapidjson::Document& json_data) : MessageReasonManageRequest(json_data) { };
    void MessageListReasonRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"db_name", this->db_name},
            {"type", this->type}};
        json_str = json.dump();
    }
    void MessageListReasonRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"db_name", this->db_name},
            {"type", this->type},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageCedsdReasonRequest::MessageCedsdReasonRequest(std::string db_name, std::string type, std::string rulename) : MessageReasonManageRequest(db_name, type)
    {
        this->rulename = rulename;
    }
    MessageCedsdReasonRequest::MessageCedsdReasonRequest(const rapidjson::Document& json_data) : MessageReasonManageRequest(json_data)
    {
        this->rulename = jsonParam(json_data, "rulename");
    }
    void MessageCedsdReasonRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"db_name", this->db_name},
            {"type", this->type},
            {"rulename", this->rulename}};
        json_str = json.dump();
    }
    void MessageCedsdReasonRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"db_name", this->db_name},
            {"type", this->type},
            {"rulename", this->rulename},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageReasonManageResponse::MessageReasonManageResponse(std::string body) : MessageResponse(body) {
        if (json.is_object())
        {
            if (json.contains("type")) 
                json.at("type").get_to(this->type);
            if (json.contains("list"))
                json.at("list").get_to(list);
            if (json.contains("AnsNum"))
                json.at("AnsNum").get_to(this->num);
            if (json.contains("insert_sparql"))
                json.at("insert_sparql").get_to(this->insert_sparql);
            if (json.contains("delete_sparql"))
                json.at("delete_sparql").get_to(this->delete_sparql);
            if (json.contains("check_sparql"))
                json.at("check_sparql").get_to(this->check_sparql);
            if (json.contains("checkMsg"))
                json.at("checkMsg").get_to(this->checkMsg);
            if (json.contains("ruleinfo"))
                this->ruleinfo = json.at("ruleinfo");
        }
    }
    void MessageReasonManageResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        if (type == "1")
        {
        }
        else if (type == "2")
        {
            json["list"] = nlohmann::json::array();
            if (list.is_array())
            {
                json["list"] = this->list;
            }
            json["num"] = this->num;
        }
        else if (type == "3")
        {
            json["insert_sparql"] = this->insert_sparql;
            json["delete_sparql"] = this->delete_sparql;
            json["delete_sparql"] = this->check_sparql;
        }
        else if (type == "4")
        {
            json["insert_sparql"] = this->insert_sparql;
            json["AnsNum"] = this->num;
        }
        else if (type == "5")
        {
            json["delete_sparql"] = this->delete_sparql;
            json["AnsNum"] = this->num;
        }
        else if (type == "6")
        {
            json["ruleinfo"] = this->ruleinfo;
        }
        else if (type == "7")
        {
        }
        else if (type == "8")
        {
            json["check_sparql"] = this->check_sparql;
            json["effectNum"] = this->num;
        }

        json_str = json.dump();
    }
}