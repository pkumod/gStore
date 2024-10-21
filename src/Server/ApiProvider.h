/*
 * @Author: hexuejiang
 * @Date: 2024-9-9 14:52:50
 * @LastEditTime: 2024-10-18 14:50:20
 * @LastEditors: hexuejiang 1632802996@qq.com
 * @Description: api manage
 * @DirPath: MessageProtocol, provide api message
 */
#pragma once
#include "workflow/WFFacilities.h"
#include "../Api/APIUtil.h"
#include "../Api/PFNUtil.h"
#include "../Cluster/ClusterManager.h"
#include "MessageProtocol/MessageApi.h"
#include "MessageProtocol/MessageCluster.h"
#include "MessageProtocol/MessageApiUpdate.h"
#include "MessageProtocol/MessageApiUser.h"
#include "MessageProtocol/MessageApiTransaction.h"
#include "MessageProtocol/MessageApiPFN.h"
#include "MessageProtocol/MessageApiReason.h"
#include "MessageProtocol/MessageApiLog.h"
#include "MessageProtocol/MessageApiBackUpRestore.h"
#include "../Reason/Reason.h"
#include <unordered_map>

using namespace cluster;

namespace server
{
    typedef std::function<void(struct DBQueryLogInfo*)> DbQueryLogCall;
    typedef std::function<void(std::string)> backup_call;
    typedef std::function<void(std::string)> restore_call;
    class ApiHandler
    {
        private:
        public:
        ApiHandler(){};
        ~ApiHandler(){};

        static bool stringIsTrue(std::string value);

        static void load(shared_ptr<APIUtil>& apiUtil, const server::MessageLoadRequest& resquest, server::MessageLoadResponse& response);
        static void monitor(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageMonitorRequest& resquest, MessageMonitorResponse& response);
        
        // update api
        static void build(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBuildRequest& resquest, MessageBuildResponse& response, const string& remote_ip);
        static void batch_insert(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchInsertRequest& resquest, MessageBatchInsertResponse& response, const string& remote_ip);
        static void batch_remove(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchRemoveRequest& resquest, MessageBatchRemoveResponse& response, const string& remote_ip);
        static void drop(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageDropRequest& resquest, MessageDropResponse& response);
        static void query(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageQueryRequest& resquest, MessageQueryResponse& response, bool &redirect, bool &is_query, const DbQueryLogCall& db_queryLog_cb);
        static void checkpoint(shared_ptr<APIUtil>& apiUtil, const server::MessageCheckPointRequest& resquest, server::MessageResponse& response);

        // user manger api
        static void show_users(shared_ptr<APIUtil>& apiUtil, server::MessageShowUserResponse& response);
        static void user_manage(shared_ptr<APIUtil>& apiUtil, server::MessageUserManageRequest& resquest, server::MessageUserManageResponse& response);
        static void user_privilege_manage(shared_ptr<APIUtil>& apiUtil, server::MessageUserPrivilegeManageRequest& resquest, server::MessageUserPrivilegeManageResponse& response);
        static void user_passworrd(shared_ptr<APIUtil>& apiUtil, server::MessageUserPasswordRequest& resquest, server::MessageUserPasswordResponse& response);

        // transaction
        static void begin(shared_ptr<APIUtil>& apiUtil, const server::MessageBeginRequest& resquest, server::MessageBeginResponse& response);
        static void tquery(shared_ptr<APIUtil>& apiUtil, const server::MessageTqueryRequest& resquest, server::MessageTqueryResponse& response);
        static void commit(shared_ptr<APIUtil>& apiUtil, const server::MessageCommitRequest& resquest, server::MessageResponse& response);
        static void rollback(shared_ptr<APIUtil>& apiUtil, const server::MessageCommitRequest& resquest, server::MessageResponse& response);

        //PFN personalize function
        static void funquery(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageFunQueryResponse& response, rapidjson::Document& json);
        static void funcudb(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageFunCudbResponse& response, rapidjson::Document& json);
        static void funreview(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageReviewResponse& response, rapidjson::Document& json);

        // reason
        static void reason_manage(shared_ptr<APIUtil>& apiUtil, server::MessageReasonManageResponse& response, rapidjson::Document& json);

        // log
        static void txn_log(shared_ptr<APIUtil>& apiUtil, server::MessageTxnLogRequest& resquest, server::MessageTxnLogResponse& response);
        static void query_log(shared_ptr<APIUtil>& apiUtil, server::MessageQueryLogRequest& resquest, server::MessageQueryLogResponse& response);
        static void query_log_date(shared_ptr<APIUtil>& apiUtil, server::MessageQueryLogDateRequest& resquest, server::MessageQueryLogDateResponse& response);
        static void access_log(shared_ptr<APIUtil>& apiUtil, server::MessageAccessLogRequest& resquest, server::MessageAccessLogResponse& response);
        static void access_log_date(shared_ptr<APIUtil>& apiUtil, server::MessageAccessLogDateRequest& resquest, server::MessageAccessLogDateResponse& response);
        static void checkOperationState(shared_ptr<APIUtil>& apiUtil, server::MessageCheckOperationStateRequest& resquest, server::MessageCheckOperationStateResponse& response);

        // backup restore
        static void backup(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupRequest& resquest, server::MessageBackupResponse& response, const backup_call& cb);
        static void backup(shared_ptr<APIUtil>& apiUtil, const std::string& opt_id, const std::string& db_name, std::string& backup_path, bool compress, const std::string& callback);
        static void backup_path(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupPathRequest& resquest, server::MessageBackupPathResponse& response);
        static void restore(shared_ptr<APIUtil>& apiUtil, const server::MessageRestoreRequest& resquest, server::MessageRestoreResponse& response, const restore_call& cb);
        static void restore(shared_ptr<APIUtil>& apiUtil, const std::string& opt_id, const std::string& db_name, const std::string& username, std::string& backup_path, const std::string& callback);

        // cluster api
        static void cluster_heartbeat_compare(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest);
        static void cluster_heartbeat_prepare(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest);
        static void cluster_heartbeat_commit(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest);
        static void cluster_heartbeat_cancel(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest);
        static void cluster_heartbeat_fail(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest);
        static void cluster_heartbeat_drop(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& resquest);
        static void cluster_append(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::map<std::string, std::pair<std::string, std::string>>& form, MessageResponse& response, const string& local_port);
        static void cluster_reply(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterReplyRequest& resquest, const string& remote_ip);
        static void cluster_check(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterCheckRequest& resquest, const string& remote_ip);
        static void cluster_recover(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::map<std::string, std::pair<std::string, std::string>>& form, MessageResponse& response, const string& local_port);
    };

    // rapidjson
    std::string to_json_string(const rapidjson::Document& json);
    std::string jsonParam(const rapidjson::Document& json, const std::string &key, const std::string& default_val = "");
    int32_t jsonParam(const rapidjson::Document& json, const std::string &key, const int32_t &default_val);
    uint32_t jsonParam(const rapidjson::Document& json, const std::string &key, const uint32_t &default_val);
    int64_t jsonParam(const rapidjson::Document& json, const std::string &key, const int64_t &default_val);
    uint64_t jsonParam(const rapidjson::Document& json, const std::string &key, const uint64_t &default_val);
    bool jsonBoolParam(const rapidjson::Document& json, const std::string &key, const bool &default_val);
    bool hasJsonParam(const rapidjson::Document& json, const std::string &key);
    
    // nlohmann
    std::string to_json_string(const nlohmann::json& json);
    std::string jsonParam(const nlohmann::json& json, const std::string &key, const std::string& default_val = "");
    int32_t jsonParam(const nlohmann::json& json, const std::string &key, const int32_t &default_val);
    uint32_t jsonParam(const nlohmann::json& json, const std::string &key, const uint32_t &default_val);
    int64_t jsonParam(const nlohmann::json& json, const std::string &key, const int64_t &default_val);
    uint64_t jsonParam(const nlohmann::json& json, const std::string &key, const uint64_t &default_val);
    bool jsonBoolParam(const nlohmann::json& json, const std::string &key, const bool &default_val);
    bool hasJsonParam(const nlohmann::json& json, const std::string &key);
}
