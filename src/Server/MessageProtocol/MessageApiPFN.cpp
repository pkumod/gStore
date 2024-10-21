#include "MessageApiPFN.h"
#include "../ApiProvider.h"

namespace server
{
    MessageFunQueryRequest::MessageFunQueryRequest(const nlohmann::json& json_data) : MessageRequest(json_data)
    {
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

    void MessageReviewResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["Result"] = this->result;
        json_str = json.dump();
    }
}