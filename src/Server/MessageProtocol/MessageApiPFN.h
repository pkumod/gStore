#pragma once
#include "MessageApi.h"

namespace server
{
    // funquery
    struct MessageFunQueryRequest : public MessageRequest
    {
        nlohmann::json json_data;
        MessageFunQueryRequest(const nlohmann::json& json_data);
    };

    struct MessageFunQueryResponse : public MessageResponse
    {
        std::vector<nlohmann::json> list;
        void toJsonString(std::string& json_str) override;
    };

    // funcudb
    struct MessageFunCudbRequest : public MessageRequest
    {
        MessageFunCudbRequest(const nlohmann::json& json_data);
    };

    struct MessageFunCudbResponse : public MessageResponse
    {
    };

    // funreview
    struct MessageReviewRequest : public MessageRequest
    {
        MessageReviewRequest(const nlohmann::json& json_data);
    };

    struct MessageReviewResponse : public MessageResponse
    {
        std::string result;
        void toJsonString(std::string& json_str) override;
    };

}