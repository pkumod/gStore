#include "MessageApiPFN.h"
#include "../ApiProvider.h"

namespace server
{
    void MessageFunQueryRequest::to_json(std::string& json_str)
    {
        nlohmann::json json_data = nlohmann::json::object();
        nlohmann::json funInfo = nlohmann::json::object();
        toJson(json_data);
        this->funInfo.toJSON(funInfo);
        json_data["funInfo"] = funInfo;
        json_str = json_data.dump();
    }

    void MessageFunQueryRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json_data = nlohmann::json::object();
        nlohmann::json funInfo = nlohmann::json::object();
        toJson(json_data);
        this->funInfo.toJSON(funInfo);
        json_data["inner"] = "true";
        json_data["funInfo"] = funInfo;
        json_str = json_data.dump();  
    }

    void MessageFunQueryResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json::object();
        toJson(json);
        json["list"] = nlohmann::json::array();
        for (PFNInfo& m : list)
        {
            nlohmann::json item;
            if (m.toJSON(item))
                json["list"].push_back(std::move(item));
        }
        json_str = json.dump();
    }

    void MessageFunCudbRequest::to_json(std::string& json_str)
    {
        nlohmann::json json_data = nlohmann::json::object();
        nlohmann::json funInfo = nlohmann::json::object();
        toJson(json_data);
        this->funInfo.toJSON(funInfo);
        json_data["type"] = this->type;
        json_data["funInfo"] = funInfo;
        json_str = json_data.dump();
    }

    void MessageFunCudbRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json_data = nlohmann::json::object();
        nlohmann::json funInfo = nlohmann::json::object();
        toJson(json_data);
        this->funInfo.toJSON(funInfo);
        json_data["inner"] = "true";
        json_data["type"] = this->type;
        json_data["funInfo"] = funInfo;
        json_str = json_data.dump();
    }

    void MessageFunCudbResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json_data = nlohmann::json::object();
        toJson(json_data);
        json_str = json_data.dump();
    }

    void MessageFunReviewRequest::to_json(std::string& json_str)
    {
        nlohmann::json json_data = nlohmann::json::object();
        nlohmann::json funInfo = nlohmann::json::object();
        toJson(json_data);
        this->funInfo.toJSON(funInfo);
        json_data["funInfo"] = funInfo;
        json_str = json_data.dump();
    }

    void MessageFunReviewRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json_data = nlohmann::json::object();
        nlohmann::json funInfo = nlohmann::json::object();
        toJson(json_data);
        this->funInfo.toJSON(funInfo);
        json_data["inner"] = "true";
        json_data["funInfo"] = funInfo;
        json_str = json_data.dump();
    }

    void MessageFunReviewResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json_data = nlohmann::json::object();
        nlohmann::json funInfo = nlohmann::json::object();
        toJson(json_data);
        json_data["result"] = this->body;
        json_str = json_data.dump();
    }
}