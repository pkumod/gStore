#include "APIConnector.h"


server::MessageShutdownResponse APIConnector::shutdown(const std::string& url, server::MessageShutdownRequest& request)
{
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", request.username));
	headers.insert(std::pair<std::string, std::string>("password", request.password));
	int status = WFHttpUtil::Post(url, headers, 60, "", body_str);
	return response_parser<server::MessageShutdownResponse>(status, body_str);
}

server::MessageCheckResponse APIConnector::check(const std::string& url, server::MessageCheckRequest& request)
{
	std::string param_str = request.to_params();
	std::string body_str;
	std::string strUrl = url + "?" + param_str;
	int status = WFHttpUtil::Get(strUrl, body_str);
	return response_parser<server::MessageCheckResponse>(status, body_str);
}

server::MessageResponse APIConnector::refreshConf(const std::string& url, const bool& inner, server::MessageRefreshconfRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageResponse>(status, body_str);
}

server::MessageTestConnectionResponse APIConnector::testConnection(const std::string& url, const bool& inner, server::MessageTestConnectionRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageTestConnectionResponse>(status, body_str);
}

server::MessageInitResponse APIConnector::init(const std::string& url, const bool& inner, server::MessageInitRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageInitResponse>(status, body_str);
}

server::MessageLoadResponse APIConnector::load(const std::string& url, const bool& inner, server::MessageLoadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageLoadResponse>(status, body_str);
}

server::MessageResponse APIConnector::login(const std::string& url, server::MessageLoginRequest& request)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageResponse>(status, body_str);
}

server::MessageResponse APIConnector::unload(const std::string& url, const bool& inner, server::MessageUnloadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageResponse>(status, body_str);
}

server::MessageBuildResponse APIConnector::build(const std::string& url, const bool& inner, server::MessageBuildRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageBuildResponse>(status, body_str);
}

server::MessageResponse APIConnector::drop(const std::string& url, const bool& inner, server::MessageDropRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageResponse>(status, body_str);
}

server::MessageShowResponse APIConnector::show(const std::string& url, const bool& inner, server::MessageShowRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageShowResponse>(status, body_str);
}

server::MessageMonitorResponse APIConnector::monitor(const std::string& url, const bool& inner, server::MessageMonitorRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageMonitorResponse>(status, body_str);
}

server::MessageQueryResponse APIConnector::query(const std::string& url, const bool& inner, server::MessageQueryRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageQueryResponse>(status, body_str);
}


server::MessageBatchInsertResponse APIConnector::batchInsert(const std::string& url, const bool& inner, server::MessageBatchInsertRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageBatchInsertResponse>(status, body_str);
}

server::MessageBatchRemoveResponse APIConnector::batchRemove(const std::string& url, const bool& inner, server::MessageBatchRemoveRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageBatchRemoveResponse>(status, body_str);
}



// server::MessageGetCoreVersionResponse APIConnector::getCoreVersion(const std::string& url, const bool& inner, server::MessageGetCoreVersionRequest& request)
// {
// 	std::string json_str;
// 	if (inner)
// 		request.to_inner_json(json_str);
// 	else
// 		request.to_json(json_str);
// 	std::string body_str;
// 	int status = WFHttpUtil::Post(url, json_str, body_str);
// 	return response_parser<server::MessageGetCoreVersionResponse>(status, body_str);
// }

// server::MessageIpManageResponse APIConnector::ipManage(const std::string& url, const bool& inner, server::MessageGetCoreVersionRequest& request);
// {
// 	std::string json_str;
// 	if (inner)
// 		request.to_inner_json(json_str);
// 	else
// 		request.to_json(json_str);
// 	std::string body_str;
// 	int status = WFHttpUtil::Post(url, json_str, body_str);
// 	return response_parser<server::MessageIpResponse>(status, body_str);
// }


// server::MessageStatResponse APIConnector::stat(const std::string& url, const bool& inner, server::MessageStatRequest& request)
// {
// 	std::string json_str;
// 	if (inner)
// 		request.to_inner_json(json_str);
// 	else
// 		request.to_json(json_str);
// 	std::string body_str;
// 	int status = WFHttpUtil::Post(url, json_str, body_str);
// 	return response_parser<server::MessageStatResponse>(status, body_str);
// }

server::MessageBackupResponse APIConnector::backup(const std::string& url, const bool& inner, server::MessageBackupRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageBackupResponse>(status, body_str);
}

// server::MessageBackUpPathResponse APIConnector::backUpPath(const std::string& url, const bool& inner, server::MessageBackUpPathRequest& request)
// {
// 	std::string json_str;
// 	if (inner)
// 		request.to_inner_json(json_str);
// 	else
// 		request.to_json(json_str);
// 	std::string body_str;
// 	int status = WFHttpUtil::Post(url, json_str, body_str);
// 	return response_parser<server::MessageBackUpPathResponse>(status, body_str);
// }

server::MessageRestoreResponse APIConnector::restore(const std::string& url, const bool& inner, server::MessageRestoreRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageRestoreResponse>(status, body_str);
}

server::MessageExportResponse APIConnector::exportDb(const std::string& url, const bool& inner, server::MessageExportRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageExportResponse>(status, body_str);
}

// server::MessageRenameResponse rename(const std::string& url, const bool& inner, server::MessageRenameRequest& request)
// {
// 	std::string json_str;
// 	if (inner)
// 		request.to_inner_json(json_str);
// 	else
// 		request.to_json(json_str);
// 	std::string body_str;
// 	int status = WFHttpUtil::Post(url, json_str, body_str);
// 	return response_parser<server::MessageRenameResponse>(status, body_str);
// }

server::MessageCheckOperationStateResponse APIConnector::checkOperationState(const std::string& url, const bool& inner, server::MessageCheckOperationStateRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageCheckOperationStateResponse>(status, body_str);
}


server::MessageBeginResponse APIConnector::begin(const std::string& url, const bool& inner, server::MessageBeginRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageBeginResponse>(status, body_str);
}

server::MessageTqueryResponse APIConnector::tquery(const std::string& url, const bool& inner, server::MessageTqueryRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageTqueryResponse>(status, body_str);
}

server::MessageCommitResponse APIConnector::commit(const std::string& url, const bool& inner, server::MessageCommitRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageCommitResponse>(status, body_str);
}

server::MessageRollbackResponse APIConnector::rollBack(const std::string& url, const bool& inner, server::MessageRollbackRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageRollbackResponse>(status, body_str);
}

server::MessageCheckPointResponse APIConnector::checkPoint(const std::string& url, const bool& inner, server::MessageCheckPointRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageCheckPointResponse>(status, body_str);
}

server::MessageShowUserResponse APIConnector::showUser(const std::string& url, const bool& inner, server::MessageShowUserRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageShowUserResponse>(status, body_str);
}

server::MessageResponse APIConnector::userManage(const std::string& url, const bool& inner, server::MessageUserManageRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageResponse>(status, body_str);
}

server::MessageUserPrivilegeManageResponse APIConnector::userPrivilegeManage(const std::string& url, const bool& inner, server::MessageUserPrivilegeManageRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageUserPrivilegeManageResponse>(status, body_str);
}

server::MessageUserPasswordResponse APIConnector::userPassword(const std::string& url, const bool& inner, server::MessageUserPasswordRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageUserPasswordResponse>(status, body_str);
}

server::MessageFunQueryResponse APIConnector::funQuery(const std::string& url, const bool& inner, server::MessageFunQueryRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageFunQueryResponse>(status, body_str);
}

server::MessageFunCudbResponse APIConnector::funCudb(const std::string& url, const bool& inner, server::MessageFunCudbRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageFunCudbResponse>(status, body_str);
}

server::MessageReviewResponse APIConnector::funReview(const std::string& url, const bool& inner, server::MessageReviewRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageReviewResponse>(status, body_str);
}

server::MessageTxnLogResponse APIConnector::txnLog(const std::string& url, const bool& inner, server::MessageTxnLogRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageTxnLogResponse>(status, body_str);
}

server::MessageQueryLogDateResponse APIConnector::queryLogDate(const std::string& url, const bool& inner, server::MessageQueryLogDateRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageQueryLogDateResponse>(status, body_str);
}

server::MessageQueryLogResponse APIConnector::queryLog(const std::string& url, const bool& inner, server::MessageQueryLogRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageQueryLogResponse>(status, body_str);
}

server::MessageAccessLogDateResponse APIConnector::accessLogDate(const std::string& url, const bool& inner, server::MessageAccessLogDateRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageAccessLogDateResponse>(status, body_str);
}

server::MessageAccessLogResponse APIConnector::accessLog(const std::string& url, const bool& inner, server::MessageAccessLogRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageAccessLogResponse>(status, body_str);
}

server::MessageReasonManageResponse APIConnector::addReason(const std::string& url, const bool& inner, server::MessageAddReasonRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageReasonManageResponse>(status, body_str);
}


server::MessageReasonManageResponse APIConnector::listReason(const std::string& url, const bool& inner, server::MessageListReasonRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageReasonManageResponse>(status, body_str);
}


server::MessageReasonManageResponse APIConnector::cedsdReason(const std::string& url, const bool& inner, server::MessageCedsdReasonRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<server::MessageReasonManageResponse>(status, body_str);
}


server::MessageLicenseResponse APIConnector::importLicense(const std::string& url, const bool& inner, server::MessageRequest& request, std::string filepath)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> params;
	params["operation"] = request.op;
	params["username"] = "root";
	params["remote_ip"] = request.remote_ip;
	params["password"] = "123456";
	params["inner"] = inner ? "true" : "false";
	int status = WFHttpUtil::PostFile(url + "/lic/import", {}, -1, filepath, params, body_str);
	return response_parser<server::MessageLicenseResponse>(status, body_str);
}

server::MessageLicenseResponse APIConnector::licenseInfo(const std::string& url, const bool& inner, server::MessageRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url + "/lic/info", json_str, body_str);
	return response_parser<server::MessageLicenseResponse>(status, body_str);
}

server::MessageLicenseResponse APIConnector::removeLicense(const std::string& url, const bool& inner, server::MessageRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url + "/lic/remove", json_str, body_str);
	return response_parser<server::MessageLicenseResponse>(status, body_str);
}

httpentities::ClusterResponse APIConnector::reply(const std::string& url, httpentities::ReplyRequest& request, const std::string& username, const std::string& password)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	int status = WFHttpUtil::Post(url, headers, 60, json_str, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

httpentities::ClusterResponse APIConnector::appendEntries(const std::string& url, httpentities::AppenEntriesRequest& request, const std::string& username, const std::string& password)
{
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	std::map<std::string, std::string> params;
	params.insert(std::pair<std::string, std::string>("db_name", request.db_name));
	params.insert(std::pair<std::string, std::string>("term", std::to_string(request.term)));
	params.insert(std::pair<std::string, std::string>("index", std::to_string(request.index)));
	params.insert(std::pair<std::string, std::string>("nextIndex", std::to_string(request.nextIndex)));
	params.insert(std::pair<std::string, std::string>("uid", std::to_string(request.uid)));
	params.insert(std::pair<std::string, std::string>("updateType", request.updateType));
	std::string body_str;
	int status = WFHttpUtil::PostFile(url, headers, 3600, request.file_path, params, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

httpentities::ClusterResponse APIConnector::heartBeat(const std::string& url, httpentities::HeartBeatRequest& request, const std::string& username, const std::string& password)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	int status = WFHttpUtil::Post(url, headers, 60, json_str, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}

// httpentities::ClusterResponse APIConnector::cancel(const std::string& url, httpentities::CancelRequest& request, const std::string& username, const std::string& password)
// {
// 	std::string json_str;
// 	request.to_json(json_str);
// 	std::string body_str;
// 	std::map<std::string, std::string> headers;
// 	headers.insert(std::pair<std::string, std::string>("username", username));
// 	headers.insert(std::pair<std::string, std::string>("password", password));
// 	int status = WFHttpUtil::Post(url, headers, 60, json_str, body_str);
// 	return response_parser<httpentities::ClusterResponse>(status, body_str);
// }

httpentities::ClusterResponse APIConnector::clusterCheck(const std::string& url, httpentities::ClusterCheckRequest& request, const std::string& username, const std::string& password)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	int status = WFHttpUtil::Post(url, headers, 60, json_str, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}


httpentities::ClusterResponse APIConnector::recoverFollower(const std::string& url, httpentities::RecoverRequest& request, const std::string& username, const std::string& password)
{
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", username));
	headers.insert(std::pair<std::string, std::string>("password", password));
	std::map<std::string, std::string> params;
	params.insert(std::pair<std::string, std::string>("db_name", request.db_name));
	params.insert(std::pair<std::string, std::string>("term", std::to_string(request.term)));
	params.insert(std::pair<std::string, std::string>("index", std::to_string(request.index)));
	params.insert(std::pair<std::string, std::string>("nextIndex", std::to_string(request.nextIndex)));
	params.insert(std::pair<std::string, std::string>("uid", std::to_string(request.uid)));
	params.insert(std::pair<std::string, std::string>("updateType", request.updateType));
	params.insert(std::pair<std::string, std::string>("recoverIndex", std::to_string(request.recoverIndex)));
	std::string body_str;
	int status = WFHttpUtil::PostFile(url, headers, 3600, request.file_path, params, body_str);
	return response_parser<httpentities::ClusterResponse>(status, body_str);
}