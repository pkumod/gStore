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
        std::string local_port; // local server port
        MessageClusterRequest()=delete;
        MessageClusterRequest(const rapidjson::Document& json_data, std::string local_port);
    };

    // cluster reply api
    struct MessageClusterReplyRequest : public MessageClusterRequest
    {
        std::string port;
        std::string operation;
        MessageClusterReplyRequest()=delete;
        MessageClusterReplyRequest(const rapidjson::Document& json_data, std::string local_port);
    };

    // cluster check api
    struct MessageClusterCheckRequest : public MessageClusterRequest
    {
        uint16_t result;
        std::string port;
        MessageClusterCheckRequest()=delete;
        MessageClusterCheckRequest(const rapidjson::Document& json_data, std::string local_port);
    };
}