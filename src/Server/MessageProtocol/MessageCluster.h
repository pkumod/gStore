#pragma once
#include "MessageApi.h"

namespace server
{
    // cluster api
    struct MessageClusterRequest
    {
        uint32_t term;
        uint64_t index;
        uint64_t nextIndex;
        std::string db_name;
        uint64_t uid;
        std::string follow_port; // local server port
        std::string follow_ip;
        MessageClusterRequest()=delete;
        MessageClusterRequest(const rapidjson::Document& json_data);
    };

    // cluster reply api
    struct MessageClusterReplyRequest : public MessageClusterRequest
    {
        std::string operation;
        MessageClusterReplyRequest()=delete;
        MessageClusterReplyRequest(const rapidjson::Document& json_data);
    };

    // cluster check api
    struct MessageClusterCheckRequest : public MessageClusterRequest
    {
        uint16_t result;
        MessageClusterCheckRequest()=delete;
        MessageClusterCheckRequest(const rapidjson::Document& json_data);
    };
}