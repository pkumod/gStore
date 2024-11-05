#pragma once
#include "MessageApi.h"

namespace server
{
    // gconsole use
    struct FunInfo {
        std::string funName;
        std::string funStatus;
        std::string funDesc;
        std::string funArgs;
        std::string funBody;
        std::string funSubs;
        std::string funReturn;
        std::string lastTime;
        FunInfo() {}
        FunInfo(std::string funName, std::string funStatus) : funName(funName), funStatus(funStatus) {}
        void to_json(std::string& json_str);
        void from_json(const nlohmann::json &json);
    };

    // funquery
    struct MessageFunQueryRequest : public MessageRequest
    {
        // nlohmann::json json_data;
        MessageFunQueryRequest() { }

        //gconsole use
        nlohmann::json funInfo;
        MessageFunQueryRequest(const nlohmann::json &funInfo);
        MessageFunQueryRequest(const rapidjson::Document& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;

        // MessageFunQueryRequest(const nlohmann::json& json_data);


    };

    struct MessageFunQueryResponse : public MessageResponse
    {
        std::vector<nlohmann::json> list;
        MessageFunQueryResponse() { }
        // gconsole use
        MessageFunQueryResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageFunQueryResponse(const std::string &body);
        void toJsonString(std::string& json_str) override;
    };

    // funcudb
    struct MessageFunCudbRequest : public MessageRequest
    {
        // gconsole use
        std::string type;
        nlohmann::json funInfo;
        
        MessageFunCudbRequest(const nlohmann::json& json_data);
        MessageFunCudbRequest(const std::string type, const nlohmann::json& funInfo);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageFunCudbResponse : public MessageResponse
    {

        MessageFunCudbResponse() { }
        // gconsole use
        MessageFunCudbResponse(int code, std::string msg) : MessageResponse(code, msg) { }
        MessageFunCudbResponse(const std::string &body) : MessageResponse(body) { }

    };

    // funreview
    struct MessageReviewRequest : public MessageRequest
    {
        nlohmann::json funInfo;
        MessageReviewRequest(const nlohmann::json& funInfo);
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
        // MessageReviewRequest(const nlohmann::json& json_data);
    };

    struct MessageReviewResponse : public MessageResponse
    {
        std::string result;
        MessageReviewResponse() { }
        MessageReviewResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageReviewResponse(const std::string &body);
        void toJsonString(std::string& json_str) override;
    };

}