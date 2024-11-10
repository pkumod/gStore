#include "MessageCluster.h"
#include "../ApiProvider.h"

namespace server
{
    MessageClusterRequest::MessageClusterRequest(const nlohmann::json& json_data)
    {
        this->term = JsonUtil::jsonParam(json_data, "uid", 0ul);
        this->index = JsonUtil::jsonParam(json_data, "index", 0ul);
        this->nextIndex = JsonUtil::jsonParam(json_data, "nextIndex", 0ul);
        this->db_name = JsonUtil::jsonParam(json_data, "db_name");
        this->uid = JsonUtil::jsonParam(json_data, "uid", 0ul);
        this->follow_port = JsonUtil::jsonParam(json_data, "follow_port", "");
        this->follow_ip = JsonUtil::jsonParam(json_data, "follow_ip", "");
    }

    MessageClusterReplyRequest::MessageClusterReplyRequest(const nlohmann::json& json_data) : MessageClusterRequest(json_data)
    {
        this->operation = JsonUtil::jsonParam(json_data, "operation");
    }

    MessageClusterCheckRequest::MessageClusterCheckRequest(const nlohmann::json& json_data) : MessageClusterRequest(json_data)
    {
        this->result = JsonUtil::jsonParam(json_data, "result", 0);
    }
}