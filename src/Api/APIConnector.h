#pragma once

#include "../Server/MessageProtocol/MessageApi.h"
#include "../Server/MessageProtocol/MessageApiUpdate.h"
#include "../Server/MessageProtocol/MessageApiUser.h"
#include "../Server/MessageProtocol/MessageApiBackUpRestore.h"
#include "../Server/MessageProtocol/MessageApiPFN.h"
#include "../Server/MessageProtocol/MessageApiLog.h"
#include "../Server/MessageProtocol/MessageApiReason.h"
#include "../Server/MessageProtocol/MessageCluster.h"
#include "../Server/MessageProtocol/MessageApiTransaction.h"
#include "WFHttpUtil.h"
#include "HttpEntities.h"

class APIConnector {
	template<typename TResponse>
	static TResponse response_parser(int& code, const std::string& body) 
    {
        if (code == WFT_STATE_SUCCESS)
            return TResponse(body);
        else
            return TResponse(code, strerror(code));
    }
private:

public:
	static server::MessageShutdownResponse shutdown(const std::string& url, server::MessageShutdownRequest& request);

	static server::MessageCheckResponse check(const std::string& url, server::MessageCheckRequest& request);

	static server::MessageResponse refreshConf(const std::string& url, const bool& inner, server::MessageRefreshconfRequest& request);

	static server::MessageTestConnectionResponse testConnection(const std::string& url, const bool& inner, server::MessageTestConnectionRequest& request);

	static server::MessageInitResponse init(const std::string& url, const bool& inner, server::MessageInitRequest& request);

	static server::MessageLoadResponse load(const std::string& url, const bool& inner, server::MessageLoadRequest& request);

	static server::MessageResponse login(const std::string& url, server::MessageLoginRequest& request);

	static server::MessageResponse unload(const std::string& url, const bool& inner, server::MessageUnloadRequest& request);

	static server::MessageBuildResponse build(const std::string& url, const bool& inner, server::MessageBuildRequest& request);

	static server::MessageResponse drop(const std::string& url, const bool& inner, server::MessageDropRequest& request);

	static server::MessageShowResponse show(const std::string& url, const bool& inner, server::MessageShowRequest& request);

	static server::MessageMonitorResponse monitor(const std::string& url, const bool& inner, server::MessageMonitorRequest& request);

	static server::MessageQueryResponse query(const std::string& url, const bool& inner, server::MessageQueryRequest& request);

	static server::MessageBatchInsertResponse batchInsert(const std::string& url, const bool& inner, server::MessageBatchInsertRequest& request);

	static server::MessageBatchRemoveResponse batchRemove(const std::string& url, const bool& inner, server::MessageBatchRemoveRequest& request);

	// start
	// static server::MessageGetCoreVersionResponse getCoreVersion(const std::string& url, const bool& inner, server::MessageGetCoreVersionRequest& request);

	// static server::MessageIpManageResponse ipManage(const std::string& url, const bool& inner, server::MessageGetCoreVersionRequest& request);

	// //  static server::MessageGetCoreVersionResponse download(const std::string& url, const bool& inner, server::MessageGetCoreVersionRequest& request);

	// static server::MessageStatResponse stat(const std::string& url, const bool& inner, server::MessageStatRequest& request);

	static server::MessageBackupResponse backup(const std::string& url, const bool& inner, server::MessageBackupRequest& request);

	// static server::MessageBackUpPathResponse backUpPath(const std::string& url, const bool& inner, server::MessageBackUpPathRequest& request);

	static server::MessageRestoreResponse restore(const std::string& url, const bool& inner, server::MessageRestoreRequest& request);

	static server::MessageExportResponse exportDb(const std::string& url, const bool& inner, server::MessageExportRequest& request);

	// static server::MessageRenameResponse rename(const std::string& url, const bool& inner, server::MessageRenameRequest& request);

	static server::MessageCheckOperationStateResponse checkOperationState(const std::string& url, const bool& inner, server::MessageCheckOperationStateRequest& request);

	static server::MessageBeginResponse begin(const std::string& url, const bool& inner, server::MessageBeginRequest& request);

	static server::MessageTqueryResponse tquery(const std::string& url, const bool& inner, server::MessageTqueryRequest& request);

	static server::MessageCommitResponse commit(const std::string& url, const bool& inner, server::MessageCommitRequest& request);

	static server::MessageRollbackResponse rollBack(const std::string& url, const bool& inner, server::MessageRollbackRequest& request);
	
	static server::MessageCheckPointResponse checkPoint(const std::string& url, const bool& inner, server::MessageCheckPointRequest& request);

	static server::MessageShowUserResponse showUser(const std::string& url, const bool& inner, server::MessageShowUserRequest& request);

	static server::MessageResponse userManage(const std::string& url, const bool& inner, server::MessageUserManageRequest& request);

	static server::MessageUserPrivilegeManageResponse userPrivilegeManage(const std::string& url, const bool& inner, server::MessageUserPrivilegeManageRequest& request);

	static server::MessageUserPasswordResponse userPassword(const std::string& url, const bool& inner, server::MessageUserPasswordRequest& request);

	static server::MessageFunQueryResponse funQuery(const std::string& url, const bool& inner, server::MessageFunQueryRequest& request);

	static server::MessageFunCudbResponse funCudb(const std::string& url, const bool& inner, server::MessageFunCudbRequest& request);

	static server::MessageReviewResponse funReview(const std::string& url, const bool& inner, server::MessageReviewRequest& request);

	static server::MessageTxnLogResponse txnLog(const std::string& url, const bool& inner, server::MessageTxnLogRequest& request);

	static server::MessageQueryLogDateResponse queryLogDate(const std::string& url, const bool& inner, server::MessageQueryLogDateRequest& request);

	static server::MessageQueryLogResponse queryLog(const std::string& url, const bool& inner, server::MessageQueryLogRequest& request);

	static server::MessageAccessLogDateResponse accessLogDate(const std::string& url, const bool& inner, server::MessageAccessLogDateRequest& request);
	
	static server::MessageAccessLogResponse accessLog(const std::string& url, const bool& inner, server::MessageAccessLogRequest& request);
	
	static server::MessageReasonManageResponse addReason(const std::string& url, const bool& inner, server::MessageAddReasonRequest& request);

	static server::MessageReasonManageResponse listReason(const std::string& url, const bool& inner, server::MessageListReasonRequest& request);
	
	static server::MessageReasonManageResponse cedsdReason(const std::string& url, const bool& inner, server::MessageCedsdReasonRequest& request);

	// static httpentities::ClusterResponse cancel(const std::string& url, httpentities::CancelRequest& request, const std::string& username, const std::string& password);
	
	static httpentities::ClusterResponse reply(const std::string& url, httpentities::ReplyRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse appendEntries(const std::string& url, httpentities::AppenEntriesRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse heartBeat(const std::string& url, httpentities::HeartBeatRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse clusterCheck(const std::string& url, httpentities::ClusterCheckRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse recoverFollower(const std::string& url, httpentities::RecoverRequest& request, const std::string& username, const std::string& password);

};
