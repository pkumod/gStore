#pragma once
#include "../Api/APIUtil.h"
#include "../Cluster/ClusterManager.h"
#include "ApiDefined.h"

using namespace cluster;

namespace server
{
    class ApiHandler
    {
        private:
        public:
        ApiHandler(){};
        ~ApiHandler(){};

        static void parseRequest(const grpc::GRPCReq *request, nlohmann::json &json_data);
        static void load(shared_ptr<APIUtil>& apiUtil, const server::MessageLoadRequest& resquest, server::MessageLoadResponse& response);

        // update data
        static void batch_insert(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchInsertRequest& resquest, MessageBatchInsertResponse& response, const string& remote_ip, const std::string& _db_home, const std::string& _db_suffix);
    };

    std::string to_json_string(const rapidjson::Document& json);
    std::string jsonParam(const rapidjson::Document& json, const std::string &key, const std::string& default_val = "");
    int32_t jsonParam(const rapidjson::Document& json, const std::string &key, const int32_t &default_val);
    uint32_t jsonParam(const rapidjson::Document& json, const std::string &key, const uint32_t &default_val);
    int64_t jsonParam(const rapidjson::Document& json, const std::string &key, const int64_t &default_val);
    uint64_t jsonParam(const rapidjson::Document& json, const std::string &key, const uint64_t &default_val);
    bool jsonBoolParam(const rapidjson::Document& json, const std::string &key, const bool &default_val);
    bool hasJsonParam(const rapidjson::Document& json, const std::string &key);
}
