#include "MessageCluster.h"
#include "../ApiProvider.h"

namespace server
{
    MessageClusterRequest::MessageClusterRequest(const rapidjson::Document& json_data, std::string local_port)
    {
        this->term = jsonParam(json_data, "uid", 0ul);
        this->index = jsonParam(json_data, "index", 0ul);
        this->nextIndex = jsonParam(json_data, "nextIndex", 0ul);
        this->db_name = jsonParam(json_data, "db_name");
        this->uid = jsonParam(json_data, "uid", 0ul);
        this->local_port = local_port;
        this->follow_ip = jsonParam(json_data, "follow_ip", "");
    }

    MessageClusterReplyRequest::MessageClusterReplyRequest(const rapidjson::Document& json_data, std::string local_port) : MessageClusterRequest(json_data, local_port)
    {
        this->operation = jsonParam(json_data, "operation");
        this->port = jsonParam(json_data, "port");
    }

    MessageClusterCheckRequest::MessageClusterCheckRequest(const rapidjson::Document& json_data, std::string local_port) : MessageClusterRequest(json_data, local_port)
    {
        this->result = jsonParam(json_data, "result", 0);
        this->port = jsonParam(json_data, "port");
        this->follow_ip = jsonParam(json_data, "follow_ip", "");
    }
}