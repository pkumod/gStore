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
	static gs::MessageShutdownResponse shutdown(const std::string& url, gs::MessageShutdownRequest& request);

	static gs::MessageCheckResponse check(const std::string& url, gs::MessageCheckRequest& request);

	static gs::MessageResponse refreshConf(const std::string& url, const bool& inner, gs::MessageRefreshconfRequest& request);

	static gs::MessageTestConnectionResponse testConnection(const std::string& url, const bool& inner, gs::MessageTestConnectionRequest& request);

	static gs::MessageInitResponse init(const std::string& url, const bool& inner, gs::MessageInitRequest& request);

	static gs::MessageLoadResponse load(const std::string& url, const bool& inner, gs::MessageLoadRequest& request);

	static gs::MessageResponse login(const std::string& url, gs::MessageLoginRequest& request);

	static gs::MessageResponse unload(const std::string& url, const bool& inner, gs::MessageUnloadRequest& request);

	static gs::MessageBuildResponse build(const std::string& url, const bool& inner, gs::MessageBuildRequest& request);

	static gs::MessageResponse drop(const std::string& url, const bool& inner, gs::MessageDropRequest& request);

	static gs::MessageShowResponse show(const std::string& url, const bool& inner, gs::MessageShowRequest& request);

	static gs::MessageMonitorResponse monitor(const std::string& url, const bool& inner, gs::MessageMonitorRequest& request);

	static gs::MessageQueryResponse query(const std::string& url, const bool& inner, gs::MessageQueryRequest& request);

	static gs::MessageBatchInsertResponse batchInsert(const std::string& url, const bool& inner, gs::MessageBatchInsertRequest& request);

	static gs::MessageBatchRemoveResponse batchRemove(const std::string& url, const bool& inner, gs::MessageBatchRemoveRequest& request);

	// start
	// static gs::MessageGetCoreVersionResponse getCoreVersion(const std::string& url, const bool& inner, gs::MessageGetCoreVersionRequest& request);

	// static gs::MessageIpManageResponse ipManage(const std::string& url, const bool& inner, gs::MessageGetCoreVersionRequest& request);

	// //  static gs::MessageGetCoreVersionResponse download(const std::string& url, const bool& inner, gs::MessageGetCoreVersionRequest& request);

	// static gs::MessageStatResponse stat(const std::string& url, const bool& inner, gs::MessageStatRequest& request);

	static gs::MessageBackupResponse backup(const std::string& url, const bool& inner, gs::MessageBackupRequest& request);

	// static gs::MessageBackUpPathResponse backUpPath(const std::string& url, const bool& inner, gs::MessageBackUpPathRequest& request);

	static gs::MessageRestoreResponse restore(const std::string& url, const bool& inner, gs::MessageRestoreRequest& request);

	static gs::MessageExportResponse exportDb(const std::string& url, const bool& inner, gs::MessageExportRequest& request);

	// static gs::MessageRenameResponse rename(const std::string& url, const bool& inner, gs::MessageRenameRequest& request);

	static gs::MessageCheckOperationStateResponse checkOperationState(const std::string& url, const bool& inner, gs::MessageCheckOperationStateRequest& request);

	static gs::MessageBeginResponse begin(const std::string& url, const bool& inner, gs::MessageBeginRequest& request);

	static gs::MessageTqueryResponse tquery(const std::string& url, const bool& inner, gs::MessageTqueryRequest& request);

	static gs::MessageCommitResponse commit(const std::string& url, const bool& inner, gs::MessageCommitRequest& request);

	static gs::MessageRollbackResponse rollBack(const std::string& url, const bool& inner, gs::MessageRollbackRequest& request);
	
	static gs::MessageCheckPointResponse checkPoint(const std::string& url, const bool& inner, gs::MessageCheckPointRequest& request);

	static gs::MessageShowUserResponse showUser(const std::string& url, const bool& inner, gs::MessageShowUserRequest& request);

	static gs::MessageResponse userManage(const std::string& url, const bool& inner, gs::MessageUserManageRequest& request);

	static gs::MessageUserPrivilegeManageResponse userPrivilegeManage(const std::string& url, const bool& inner, gs::MessageUserPrivilegeManageRequest& request);

	static gs::MessageUserPasswordResponse userPassword(const std::string& url, const bool& inner, gs::MessageUserPasswordRequest& request);

	static gs::MessageFunQueryResponse funQuery(const std::string& url, const bool& inner, gs::MessageFunQueryRequest& request);

	static gs::MessageFunCudbResponse funCudb(const std::string& url, const bool& inner, gs::MessageFunCudbRequest& request);

	static gs::MessageFunReviewResponse funReview(const std::string& url, const bool& inner, gs::MessageFunReviewRequest& request);

	static gs::MessageTxnLogResponse txnLog(const std::string& url, const bool& inner, gs::MessageTxnLogRequest& request);

	static gs::MessageQueryLogDateResponse queryLogDate(const std::string& url, const bool& inner, gs::MessageQueryLogDateRequest& request);

	static gs::MessageQueryLogResponse queryLog(const std::string& url, const bool& inner, gs::MessageQueryLogRequest& request);

	static gs::MessageAccessLogDateResponse accessLogDate(const std::string& url, const bool& inner, gs::MessageAccessLogDateRequest& request);
	
	static gs::MessageAccessLogResponse accessLog(const std::string& url, const bool& inner, gs::MessageAccessLogRequest& request);
	
	static gs::MessageReasonManageResponse addReason(const std::string& url, const bool& inner, gs::MessageAddReasonRequest& request);

	static gs::MessageReasonManageResponse listReason(const std::string& url, const bool& inner, gs::MessageListReasonRequest& request);
	
	static gs::MessageReasonManageResponse cedsdReason(const std::string& url, const bool& inner, gs::MessageCedsdReasonRequest& request);

	static gs::MessageLicenseResponse importLicense(const std::string& url, const bool& inner, gs::MessageRequest& request, std::string filepath);

	static gs::MessageLicenseResponse licenseInfo(const std::string& url, const bool& inner, gs::MessageRequest& request);
	
	static gs::MessageLicenseResponse removeLicense(const std::string& url, const bool& inner, gs::MessageRequest& request);
	// static httpentities::ClusterResponse cancel(const std::string& url, httpentities::CancelRequest& request, const std::string& username, const std::string& password);
	
	static httpentities::ClusterResponse reply(const std::string& url, httpentities::ReplyRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse appendEntries(const std::string& url, httpentities::AppenEntriesRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse heartBeat(const std::string& url, httpentities::HeartBeatRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse clusterCheck(const std::string& url, httpentities::ClusterCheckRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse recoverFollower(const std::string& url, httpentities::RecoverRequest& request, const std::string& username, const std::string& password);

};
