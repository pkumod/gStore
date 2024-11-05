#include "MessageApiPFN.h"
#include "../ApiProvider.h"

namespace server
{

    void FunInfo::to_json(std::string &json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"funName", this->funName},
            {"funStatus", this->funStatus},
            {"funDesc", this->funDesc},
            {"funArgs", this->funArgs},
            {"funBody", this->funBody},
            {"funSubs", this->funSubs},
            {"funReturn", this->funReturn},
            {"lastTime", this->lastTime}};
        json_str = json.dump();
    }

    void FunInfo::from_json(const nlohmann::json &json)
    {
        if (hasJsonParam(json, "funName"))
            this->funName = jsonParam(json, "funName");
        if (hasJsonParam(json, "funName"))
            this->funStatus = jsonParam(json, "funStatus");
        if (hasJsonParam(json, "funDesc"))
            this->funDesc = jsonParam(json, "funDesc");
        if (hasJsonParam(json, "funArgs"))
            this->funArgs = jsonParam(json, "funArgs");
        if (hasJsonParam(json, "funBody"))
            this->funBody = jsonParam(json, "funBody");
        if (hasJsonParam(json, "funSubs"))
            this->funSubs = jsonParam(json, "funSubs");
        if (hasJsonParam(json, "funReturn"))
            this->funReturn = jsonParam(json, "funReturn");
        if (hasJsonParam(json, "lastTime"))
            this->lastTime = jsonParam(json, "lastTime");
    }

    // MessageFunQueryRequest::MessageFunQueryRequest(const nlohmann::json& json_data) : MessageRequest(json_data) { }
    
    //gconsole use
    MessageFunQueryRequest::MessageFunQueryRequest(const nlohmann::json &funInfo) : MessageRequest(std::string("funquery"))
    {
        this->funInfo = funInfo;
    }
    MessageFunQueryRequest::MessageFunQueryRequest(const rapidjson::Document& json_data)
    {
        std::string funInfo;
        funInfo = jsonParam(json_data, "funInfo");
        this->funInfo = nlohmann::json::parse(funInfo);
    }
    void MessageFunQueryRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"}};

        json["funInfo"] = this->funInfo;

        json_str = json.dump();
    }
    void MessageFunQueryRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"inner", "true"}};

        json["funInfo"] = this->funInfo;

        json_str = json.dump();     
    }

    MessageFunQueryResponse::MessageFunQueryResponse(const std::string& body) : MessageResponse(body)
    {
        if (json.is_object() && json.contains("list"))
        {
            for (auto fun_json : json["list"])
            {
                this->list.push_back(fun_json);
            }
        }        
    }

    void MessageFunQueryResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["list"] = nlohmann::json::array();
        for (const auto& m : list)
        {
            json["list"].push_back(m);
        }
        json_str = json.dump();
    }

    MessageFunCudbRequest::MessageFunCudbRequest(const nlohmann::json& json_data) {
        this->type = jsonParam(json_data, "type");
        this->funInfo = jsonParam(json_data, "funInfo");
    }
    
    MessageFunCudbRequest::MessageFunCudbRequest(const std::string type, const nlohmann::json& funInfo) : MessageRequest(std::string("funcudb"))
    {
        this->type = type;
        this->funInfo = funInfo;
    }

    void MessageFunCudbRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"type", this->type}};
        
        json["funInfo"] = this->funInfo;

        json_str = json.dump();
    }

    void MessageFunCudbRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"type", this->type},
            {"inner", "true"}};
        
        json["funInfo"] = this->funInfo;

        json_str = json.dump();
    }

    MessageReviewRequest::MessageReviewRequest(const nlohmann::json& funInfo) : MessageRequest(std::string("funreview"))
    {
        this->funInfo = funInfo;
    }

    // MessageReviewRequest::MessageReviewRequest(const nlohmann::json& json_data)
    // {
    //     if (hasJsonParam(json_data, "funInfo"))
    //         this->funInfo = jsonParam(json_data, "funInfo");
    // }

    void MessageReviewRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"funInfo", this->funInfo}};
        
        json_str = json.dump();
    }

    void MessageReviewRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", "123456"},
            {"funInfo", this->funInfo},
            {"inner", "true"}};
        
        json_str = json.dump();
    }



    MessageReviewResponse::MessageReviewResponse(const std::string &body) : MessageResponse(body)
    {
       if (json.is_object())
        {
            if (json.contains("Result")) 
                json.at("Result").get_to(this->result);
        }
    }

    void MessageReviewResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["Result"] = this->result;
        json_str = json.dump();
    }
}