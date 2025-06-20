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
#include "../Pfn/PFNUtil.h"
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

namespace gs
{
    typedef std::function<void(std::shared_ptr<DBQueryLogInfo>)> query_call;
    class ApiHandler
    {
        private:
        static bool uncompress_zip(shared_ptr<APIUtil>& apiUtil, const std::string& file, std::map<std::string, unsigned long long>& uncompress_files, const std::string& uncompress_path, MessageResponse& response);
        static bool remove_temp_files(std::vector<std::string>& temp_paths, const bool remove_parents_if_empty = true);
        public:
        ApiHandler(){};
        ~ApiHandler(){};

        static void load(shared_ptr<APIUtil>& apiUtil, const gs::MessageLoadRequest& request, gs::MessageLoadResponse& response);
        static void monitor(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageMonitorRequest& request, MessageMonitorResponse& response);
        
        // update api
        static void drop(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageDropRequest& request, MessageDropResponse& response);
        static void checkpoint(shared_ptr<APIUtil>& apiUtil, const gs::MessageCheckPointRequest& request, gs::MessageResponse& response);

        // query
        private:
        static std::string repalce_pfn_query(const std::string& sparql, MessageQueryResponse& response);
        static bool query_check(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response);
        static void query_format_response_data(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response, ResultSet& rs);
        public:
        static void query_result_notify(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response);
        static void query_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageQueryRequest& request, MessageQueryResponse& response, bool &is_update, const query_call& cb);
        static void query(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response, const query_call& cb, bool format_check = false);

        // user manger api
        static void show_users(shared_ptr<APIUtil>& apiUtil, gs::MessageShowUserResponse& response);
        static void user_manage(shared_ptr<APIUtil>& apiUtil, gs::MessageUserManageRequest& request, gs::MessageUserManageResponse& response);
        static void user_privilege_manage(shared_ptr<APIUtil>& apiUtil, gs::MessageUserPrivilegeManageRequest& request, gs::MessageUserPrivilegeManageResponse& response);
        static void user_passworrd(shared_ptr<APIUtil>& apiUtil, gs::MessageUserPasswordRequest& request, gs::MessageUserPasswordResponse& response);

        // transaction
        static void begin(shared_ptr<APIUtil>& apiUtil, const gs::MessageBeginRequest& request, gs::MessageBeginResponse& response);
        static void tquery(shared_ptr<APIUtil>& apiUtil, const gs::MessageTqueryRequest& request, gs::MessageTqueryResponse& response);
        static void commit(shared_ptr<APIUtil>& apiUtil, const gs::MessageCommitRequest& request, gs::MessageResponse& response);
        static void rollback(shared_ptr<APIUtil>& apiUtil, const gs::MessageCommitRequest& request, gs::MessageResponse& response);

        //PFN personalize function
        static void funquery(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, gs::MessageFunQueryRequest& request, gs::MessageFunQueryResponse& response);
        static void funcudb(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, gs::MessageFunCudbRequest& request, gs::MessageFunCudbResponse& response);
        static void funreview(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, gs::MessageFunReviewRequest& request, gs::MessageFunReviewResponse& response);

        // reason
        static void reason_manage(shared_ptr<APIUtil>& apiUtil, gs::MessageReasonManageResponse& response, nlohmann::json& json);

        // log
        static void txn_log(shared_ptr<APIUtil>& apiUtil, gs::MessageTxnLogRequest& request, gs::MessageTxnLogResponse& response);
        static void query_log(shared_ptr<APIUtil>& apiUtil, gs::MessageQueryLogRequest& request, gs::MessageQueryLogResponse& response);
        static void query_log_date(shared_ptr<APIUtil>& apiUtil, gs::MessageQueryLogDateRequest& request, gs::MessageQueryLogDateResponse& response);
        static void access_log(shared_ptr<APIUtil>& apiUtil, gs::MessageAccessLogRequest& request, gs::MessageAccessLogResponse& response);
        static void access_log_date(shared_ptr<APIUtil>& apiUtil, gs::MessageAccessLogDateRequest& request, gs::MessageAccessLogDateResponse& response);
        static void checkOperationState(shared_ptr<APIUtil>& apiUtil, gs::MessageCheckOperationStateRequest& request, gs::MessageCheckOperationStateResponse& response);

        // export
        static void export_db(shared_ptr<APIUtil>& apiUtil, const gs::MessageExportRequest& request, gs::MessageExportResponse& response);

        // backup
        static bool backup_check(shared_ptr<APIUtil>& apiUtil, const gs::MessageBackupRequest& request, gs::MessageBackupResponse& response, std::string& backup_path);
        static void backup(shared_ptr<APIUtil>& apiUtil, const gs::MessageBackupRequest& request, gs::MessageBackupResponse& response);
        static void backup_async(shared_ptr<APIUtil>& apiUtil, const gs::MessageBackupRequest& request, gs::MessageBackupResponse& response);
        static void backup_path(shared_ptr<APIUtil>& apiUtil, const gs::MessageBackupPathRequest& request, gs::MessageBackupPathResponse& response);

        // restore
        static bool restore_check(shared_ptr<APIUtil>& apiUtil, const gs::MessageRestoreRequest& request, gs::MessageRestoreResponse& response);
        static void restore(shared_ptr<APIUtil>& apiUtil, const gs::MessageRestoreRequest& request, gs::MessageRestoreResponse& response);
        static void restore_async(shared_ptr<APIUtil>& apiUtil, const gs::MessageRestoreRequest& request, gs::MessageRestoreResponse& response);

        // build
        private:
        static bool build_check(shared_ptr<APIUtil>& apiUtil, const MessageBuildRequest& request, MessageBuildResponse& response, std::map<std::string, unsigned long long>& file_paths, std::vector<std::string>& temp_paths);
        public:
        static void build(shared_ptr<APIUtil>& apiUtil, const MessageBuildRequest& request, MessageBuildResponse& response);
        static void build_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBuildRequest& request, MessageBuildResponse& response);

        // batchInsert
        private:
        static bool batch_insert_check(shared_ptr<APIUtil>& apiUtil, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response, std::vector<std::string>& file_paths, std::vector<std::string>& temp_paths);
        public:
        static void batch_insert(shared_ptr<APIUtil>& apiUtil, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response);
        static void batch_insert_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response);

        // batchRemove
        private:
        static bool batch_remove_check(shared_ptr<APIUtil>& apiUtil, const MessageBatchRemoveRequest& request, MessageBatchRemoveResponse& response, std::vector<std::string>& file_paths, std::vector<std::string>& temp_paths);
        public:
        static void batch_remove(shared_ptr<APIUtil>& apiUtil, const MessageBatchRemoveRequest& request, MessageBatchRemoveResponse& response);
        static void batch_remove_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchRemoveRequest& request, MessageBatchRemoveResponse& response);


        // cluster api
        static void cluster_heartbeat_compare(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& request);
        static void cluster_heartbeat_prepare(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& request);
        static void cluster_heartbeat_commit(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& request);
        static void cluster_heartbeat_cancel(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& request);
        static void cluster_heartbeat_fail(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& request);
        static void cluster_heartbeat_drop(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterRequest& request);
        static void cluster_append(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::map<std::string, std::pair<std::string, std::string>>& form, MessageResponse& response, const string& local_port);
        static void cluster_reply(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterReplyRequest& request, const string& remote_ip);
        static void cluster_check(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageClusterCheckRequest& request, const string& remote_ip);
        static void cluster_recover(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::map<std::string, std::pair<std::string, std::string>>& form, MessageResponse& response, const string& local_port);
        static void cluster_recover_by_index(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::shared_ptr<MessageClusterRecoverRequest> request);
        static void cluster_recover_by_init(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, std::shared_ptr<MessageClusterRecoverRequest> request);
    };
}
