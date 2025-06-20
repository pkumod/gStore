#include "MessageCluster.h"
#include "../ApiProvider.h"

namespace gs
{
    MessageClusterRequest::MessageClusterRequest()
    {
        this->term = 0;
        this->index = 0;
        this->nextIndex = 0;
        this->uid = 0;
    }

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

    MessageClusterRecoverRequest::MessageClusterRecoverRequest(std::map<std::string, std::pair<std::string, std::string>>& form)
    {
        // 文件转移所有权, fileinfo.second将失去意义, 不能再使用
        std::pair<std::string, std::string>& fileinfo = form.at("file");
        file_name = fileinfo.first;
        file_content = std::move(fileinfo.second);

        term = std::stol(form.at("term").second);
        index = std::stoul(form.at("index").second);
        nextIndex = std::stoul(form.at("nextIndex").second);
        uid = std::stoul(form.at("uid").second);
        db_name = form.at("db_name").second;
        recoverIndex = std::stoul(form.at("recoverIndex").second);
        updateType = cluster::ClusterUpdateTypeHandle::to_enum(form.at("updateType").second);
    }
}