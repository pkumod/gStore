#include "MessageCluster.h"
#include "../ApiProvider.h"

namespace server
{
    MessageClusterRequest::MessageClusterRequest(const rapidjson::Document& json_data)
    {
        this->term = jsonParam(json_data, "uid", 0ul);
        this->index = jsonParam(json_data, "index", 0ul);
        this->nextIndex = jsonParam(json_data, "nextIndex", 0ul);
        this->db_name = jsonParam(json_data, "db_name");
        this->uid = jsonParam(json_data, "uid", 0ul);
        this->follow_port = jsonParam(json_data, "follow_port", "");
        this->follow_ip = jsonParam(json_data, "follow_ip", "");
    }

    MessageClusterReplyRequest::MessageClusterReplyRequest(const rapidjson::Document& json_data) : MessageClusterRequest(json_data)
    {
        this->operation = jsonParam(json_data, "operation");
    }

    MessageClusterCheckRequest::MessageClusterCheckRequest(const rapidjson::Document& json_data) : MessageClusterRequest(json_data)
    {
        this->result = jsonParam(json_data, "result", 0);
    }
}