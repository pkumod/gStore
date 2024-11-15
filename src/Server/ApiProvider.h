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

namespace server
{
    typedef std::function<void(struct DBQueryLogInfo*)> query_call;
    class ApiHandler
    {
        private:
        static bool uncompress_zip(shared_ptr<APIUtil>& apiUtil, const std::string& file, std::vector<std::string>& nt_files, std::string& unz_dir_path, MessageResponse& response);
        static bool uncompress_zip(shared_ptr<APIUtil>& apiUtil, const std::string& file, std::vector<std::string>& nt_files, std::string& unz_dir_path, MessageResponse& response, std::string& max_file);
        public:
        ApiHandler(){};
        ~ApiHandler(){};
        

        static bool stringIsTrue(std::string value);

        static void load(shared_ptr<APIUtil>& apiUtil, const server::MessageLoadRequest& request, server::MessageLoadResponse& response);
        static void monitor(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageMonitorRequest& request, MessageMonitorResponse& response);
        
        // update api
        static void drop(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageDropRequest& request, MessageDropResponse& response);
        static void checkpoint(shared_ptr<APIUtil>& apiUtil, const server::MessageCheckPointRequest& request, server::MessageResponse& response);

        // query
        static bool query_check(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& resquest, MessageQueryResponse& response);
        static void query_result_notify(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response);
        static void query_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageQueryRequest& request, MessageQueryResponse& response, bool &is_update, const query_call& cb);
        static bool query_async(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response, const std::string& opt_id);
        static bool query(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response, const query_call& cb);

        // user manger api
        static void show_users(shared_ptr<APIUtil>& apiUtil, server::MessageShowUserResponse& response);
        static void user_manage(shared_ptr<APIUtil>& apiUtil, server::MessageUserManageRequest& request, server::MessageUserManageResponse& response);
        static void user_privilege_manage(shared_ptr<APIUtil>& apiUtil, server::MessageUserPrivilegeManageRequest& request, server::MessageUserPrivilegeManageResponse& response);
        static void user_passworrd(shared_ptr<APIUtil>& apiUtil, server::MessageUserPasswordRequest& request, server::MessageUserPasswordResponse& response);

        // transaction
        static void begin(shared_ptr<APIUtil>& apiUtil, const server::MessageBeginRequest& request, server::MessageBeginResponse& response);
        static void tquery(shared_ptr<APIUtil>& apiUtil, const server::MessageTqueryRequest& request, server::MessageTqueryResponse& response);
        static void commit(shared_ptr<APIUtil>& apiUtil, const server::MessageCommitRequest& request, server::MessageResponse& response);
        static void rollback(shared_ptr<APIUtil>& apiUtil, const server::MessageCommitRequest& request, server::MessageResponse& response);

        //PFN personalize function
        static void funquery(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageFunQueryRequest& request, server::MessageFunQueryResponse& response);
        static void funcudb(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageFunCudbRequest& request, server::MessageFunCudbResponse& response);
        static void funreview(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageFunReviewRequest& request, server::MessageFunReviewResponse& response);

        // reason
        static void reason_manage(shared_ptr<APIUtil>& apiUtil, server::MessageReasonManageResponse& response, nlohmann::json& json);

        // log
        static void txn_log(shared_ptr<APIUtil>& apiUtil, server::MessageTxnLogRequest& request, server::MessageTxnLogResponse& response);
        static void query_log(shared_ptr<APIUtil>& apiUtil, server::MessageQueryLogRequest& request, server::MessageQueryLogResponse& response);
        static void query_log_date(shared_ptr<APIUtil>& apiUtil, server::MessageQueryLogDateRequest& request, server::MessageQueryLogDateResponse& response);
        static void access_log(shared_ptr<APIUtil>& apiUtil, server::MessageAccessLogRequest& request, server::MessageAccessLogResponse& response);
        static void access_log_date(shared_ptr<APIUtil>& apiUtil, server::MessageAccessLogDateRequest& request, server::MessageAccessLogDateResponse& response);
        static void checkOperationState(shared_ptr<APIUtil>& apiUtil, server::MessageCheckOperationStateRequest& request, server::MessageCheckOperationStateResponse& response);

        // export
        static void export_db(shared_ptr<APIUtil>& apiUtil, const server::MessageExportRequest& request, server::MessageExportResponse& response);

        // backup
        static bool backup_check(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupRequest& request, server::MessageBackupResponse& response, std::string& backup_path);
        static void backup(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupRequest& request, server::MessageBackupResponse& response);
        static void backup_async(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupRequest& request, server::MessageBackupResponse& response);
        static void backup_path(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupPathRequest& request, server::MessageBackupPathResponse& response);

        // restore
        static bool restore_check(shared_ptr<APIUtil>& apiUtil, const server::MessageRestoreRequest& request, server::MessageRestoreResponse& response);
        static void restore(shared_ptr<APIUtil>& apiUtil, const server::MessageRestoreRequest& request, server::MessageRestoreResponse& response);
        static void restore_async(shared_ptr<APIUtil>& apiUtil, const server::MessageRestoreRequest& request, server::MessageRestoreResponse& response);

        // build
        static bool build_check(shared_ptr<APIUtil>& apiUtil, const MessageBuildRequest& request, MessageBuildResponse& response);
        static void build(shared_ptr<APIUtil>& apiUtil, const MessageBuildRequest& request, MessageBuildResponse& response);
        static void build_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBuildRequest& request, MessageBuildResponse& response);
        // static void build(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBuildRequest& request, MessageBuildResponse& response, const string& remote_ip);

        // batchInsert
        static bool batch_insert_check(shared_ptr<APIUtil>& apiUtil, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response, bool& is_file);
        static void batch_insert(shared_ptr<APIUtil>& apiUtil, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response);
        static void batch_insert_cluster(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageBatchInsertRequest& request, MessageBatchInsertResponse& response);

        // batchRemove
        static bool batch_remove_check(shared_ptr<APIUtil>& apiUtil, const MessageBatchRemoveRequest& request, MessageBatchRemoveResponse& response);
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
    };
}
