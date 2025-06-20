#pragma once
#include "MessageApi.h"
#include "../../Cluster/ClusterDefined.h"

namespace gs
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
        MessageClusterRequest(const nlohmann::json& json_data);
        MessageClusterRequest();
    };

    // cluster reply api
    struct MessageClusterReplyRequest : public MessageClusterRequest
    {
        std::string operation;
        MessageClusterReplyRequest()=delete;
        MessageClusterReplyRequest(const nlohmann::json& json_data);
    };

    // cluster check api
    struct MessageClusterCheckRequest : public MessageClusterRequest
    {
        uint16_t result;
        MessageClusterCheckRequest()=delete;
        MessageClusterCheckRequest(const nlohmann::json& json_data);
    };

    // cluster Message Recover
    struct MessageClusterRecoverRequest : public MessageClusterRequest
    {
        std::string file_name;
        std::string file_content;
        uint64_t recoverIndex;
        cluster::ClusterUpdateType updateType;
        MessageClusterRecoverRequest()=delete;
        MessageClusterRecoverRequest(std::map<std::string, std::pair<std::string, std::string>>& form);
    };
}