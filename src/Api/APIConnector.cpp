#include "APIConnector.h"


gs::MessageShutdownResponse APIConnector::shutdown(const std::string& url, gs::MessageShutdownRequest& request)
{
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", request.username));
	headers.insert(std::pair<std::string, std::string>("password", request.password));
	int status = WFHttpUtil::Post(url, headers, -1, "", body_str);
	return response_parser<gs::MessageShutdownResponse>(status, body_str);
}

gs::MessageCheckResponse APIConnector::check(const std::string& url, gs::MessageCheckRequest& request)
{
	std::string param_str = request.to_params();
	std::string body_str;
	std::string strUrl = url + "?" + param_str;
	int status = WFHttpUtil::Get(strUrl, body_str);
	return response_parser<gs::MessageCheckResponse>(status, body_str);
}

gs::MessageResponse APIConnector::refreshConf(const std::string& url, const bool& inner, gs::MessageRefreshconfRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageResponse>(status, body_str);
}

gs::MessageTestConnectionResponse APIConnector::testConnection(const std::string& url, const bool& inner, gs::MessageTestConnectionRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageTestConnectionResponse>(status, body_str);
}

gs::MessageInitResponse APIConnector::init(const std::string& url, const bool& inner, gs::MessageInitRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageInitResponse>(status, body_str);
}

gs::MessageLoadResponse APIConnector::load(const std::string& url, const bool& inner, gs::MessageLoadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageLoadResponse>(status, body_str);
}

gs::MessageResponse APIConnector::login(const std::string& url, gs::MessageLoginRequest& request)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageResponse>(status, body_str);
}

gs::MessageResponse APIConnector::unload(const std::string& url, const bool& inner, gs::MessageUnloadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageResponse>(status, body_str);
}

gs::MessageBuildResponse APIConnector::build(const std::string& url, const bool& inner, gs::MessageBuildRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageBuildResponse>(status, body_str);
}

gs::MessageResponse APIConnector::drop(const std::string& url, const bool& inner, gs::MessageDropRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageResponse>(status, body_str);
}

gs::MessageShowResponse APIConnector::show(const std::string& url, const bool& inner, gs::MessageShowRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageShowResponse>(status, body_str);
}

gs::MessageMonitorResponse APIConnector::monitor(const std::string& url, const bool& inner, gs::MessageMonitorRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageMonitorResponse>(status, body_str);
}

gs::MessageQueryResponse APIConnector::query(const std::string& url, const bool& inner, gs::MessageQueryRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageQueryResponse>(status, body_str);
}


gs::MessageBatchInsertResponse APIConnector::batchInsert(const std::string& url, const bool& inner, gs::MessageBatchInsertRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageBatchInsertResponse>(status, body_str);
}

gs::MessageBatchRemoveResponse APIConnector::batchRemove(const std::string& url, const bool& inner, gs::MessageBatchRemoveRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageBatchRemoveResponse>(status, body_str);
}



// gs::MessageGetCoreVersionResponse APIConnector::getCoreVersion(const std::string& url, const bool& inner, gs::MessageGetCoreVersionRequest& request)
// {
// 	std::string json_str;
// 	if (inner)
// 		request.to_inner_json(json_str);
// 	else
// 		request.to_json(json_str);
// 	std::string body_str;
// 	int status = WFHttpUtil::Post(url, json_str, body_str);
// 	return response_parser<gs::MessageGetCoreVersionResponse>(status, body_str);
// }

// gs::MessageIpManageResponse APIConnector::ipManage(const std::string& url, const bool& inner, gs::MessageGetCoreVersionRequest& request);
// {
// 	std::string json_str;
// 	if (inner)
// 		request.to_inner_json(json_str);
// 	else
// 		request.to_json(json_str);
// 	std::string body_str;
// 	int status = WFHttpUtil::Post(url, json_str, body_str);
// 	return response_parser<gs::MessageIpResponse>(status, body_str);
// }


// gs::MessageStatResponse APIConnector::stat(const std::string& url, const bool& inner, gs::MessageStatRequest& request)
// {
// 	std::string json_str;
// 	if (inner)
// 		request.to_inner_json(json_str);
// 	else
// 		request.to_json(json_str);
// 	std::string body_str;
// 	int status = WFHttpUtil::Post(url, json_str, body_str);
// 	return response_parser<gs::MessageStatResponse>(status, body_str);
// }

gs::MessageBackupResponse APIConnector::backup(const std::string& url, const bool& inner, gs::MessageBackupRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageBackupResponse>(status, body_str);
}

// gs::MessageBackUpPathResponse APIConnector::backUpPath(const std::string& url, const bool& inner, gs::MessageBackUpPathRequest& request)
// {
// 	std::string json_str;
// 	if (inner)
// 		request.to_inner_json(json_str);
// 	else
// 		request.to_json(json_str);
// 	std::string body_str;
// 	int status = WFHttpUtil::Post(url, json_str, body_str);
// 	return response_parser<gs::MessageBackUpPathResponse>(status, body_str);
// }

gs::MessageRestoreResponse APIConnector::restore(const std::string& url, const bool& inner, gs::MessageRestoreRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageRestoreResponse>(status, body_str);
}

gs::MessageExportResponse APIConnector::exportDb(const std::string& url, const bool& inner, gs::MessageExportRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageExportResponse>(status, body_str);
}

// gs::MessageRenameResponse rename(const std::string& url, const bool& inner, gs::MessageRenameRequest& request)
// {
// 	std::string json_str;
// 	if (inner)
// 		request.to_inner_json(json_str);
// 	else
// 		request.to_json(json_str);
// 	std::string body_str;
// 	int status = WFHttpUtil::Post(url, json_str, body_str);
// 	return response_parser<gs::MessageRenameResponse>(status, body_str);
// }

gs::MessageCheckOperationStateResponse APIConnector::checkOperationState(const std::string& url, const bool& inner, gs::MessageCheckOperationStateRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageCheckOperationStateResponse>(status, body_str);
}


gs::MessageBeginResponse APIConnector::begin(const std::string& url, const bool& inner, gs::MessageBeginRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageBeginResponse>(status, body_str);
}

gs::MessageTqueryResponse APIConnector::tquery(const std::string& url, const bool& inner, gs::MessageTqueryRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageTqueryResponse>(status, body_str);
}

gs::MessageCommitResponse APIConnector::commit(const std::string& url, const bool& inner, gs::MessageCommitRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageCommitResponse>(status, body_str);
}

gs::MessageRollbackResponse APIConnector::rollBack(const std::string& url, const bool& inner, gs::MessageRollbackRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageRollbackResponse>(status, body_str);
}

gs::MessageCheckPointResponse APIConnector::checkPoint(const std::string& url, const bool& inner, gs::MessageCheckPointRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageCheckPointResponse>(status, body_str);
}

gs::MessageShowUserResponse APIConnector::showUser(const std::string& url, const bool& inner, gs::MessageShowUserRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageShowUserResponse>(status, body_str);
}

gs::MessageResponse APIConnector::userManage(const std::string& url, const bool& inner, gs::MessageUserManageRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageResponse>(status, body_str);
}

gs::MessageUserPrivilegeManageResponse APIConnector::userPrivilegeManage(const std::string& url, const bool& inner, gs::MessageUserPrivilegeManageRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageUserPrivilegeManageResponse>(status, body_str);
}

gs::MessageUserPasswordResponse APIConnector::userPassword(const std::string& url, const bool& inner, gs::MessageUserPasswordRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageUserPasswordResponse>(status, body_str);
}

gs::MessageFunQueryResponse APIConnector::funQuery(const std::string& url, const bool& inner, gs::MessageFunQueryRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageFunQueryResponse>(status, body_str);
}

gs::MessageFunCudbResponse APIConnector::funCudb(const std::string& url, const bool& inner, gs::MessageFunCudbRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageFunCudbResponse>(status, body_str);
}

gs::MessageFunReviewResponse APIConnector::funReview(const std::string& url, const bool& inner, gs::MessageFunReviewRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageFunReviewResponse>(status, body_str);
}

gs::MessageTxnLogResponse APIConnector::txnLog(const std::string& url, const bool& inner, gs::MessageTxnLogRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageTxnLogResponse>(status, body_str);
}

gs::MessageQueryLogDateResponse APIConnector::queryLogDate(const std::string& url, const bool& inner, gs::MessageQueryLogDateRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageQueryLogDateResponse>(status, body_str);
}

gs::MessageQueryLogResponse APIConnector::queryLog(const std::string& url, const bool& inner, gs::MessageQueryLogRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageQueryLogResponse>(status, body_str);
}

gs::MessageAccessLogDateResponse APIConnector::accessLogDate(const std::string& url, const bool& inner, gs::MessageAccessLogDateRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageAccessLogDateResponse>(status, body_str);
}

gs::MessageAccessLogResponse APIConnector::accessLog(const std::string& url, const bool& inner, gs::MessageAccessLogRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageAccessLogResponse>(status, body_str);
}

gs::MessageReasonManageResponse APIConnector::addReason(const std::string& url, const bool& inner, gs::MessageAddReasonRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageReasonManageResponse>(status, body_str);
}


gs::MessageReasonManageResponse APIConnector::listReason(const std::string& url, const bool& inner, gs::MessageListReasonRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageReasonManageResponse>(status, body_str);
}


gs::MessageReasonManageResponse APIConnector::cedsdReason(const std::string& url, const bool& inner, gs::MessageCedsdReasonRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<gs::MessageReasonManageResponse>(status, body_str);
}


gs::MessageLicenseResponse APIConnector::importLicense(const std::string& url, const bool& inner, gs::MessageRequest& request, std::string filepath)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	std::map<std::string, std::string> params;
	params["operation"] = request.op;
	params["username"] = GlobalTypedef::root_uname();
	params["remote_ip"] = request.remote_ip;
	params["password"] = "";
	params["inner"] = inner ? "true" : "false";
	int status = WFHttpUtil::PostFile(url + "/lic/import", {}, -1, filepath, params, body_str);
	std::cout << body_str << endl;
	return response_parser<gs::MessageLicenseResponse>(status, body_str);
}

gs::MessageLicenseResponse APIConnector::licenseInfo(const std::string& url, const bool& inner, gs::MessageRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url + "/lic/info", json_str, body_str);
	return response_parser<gs::MessageLicenseResponse>(status, body_str);
}

gs::MessageLicenseResponse APIConnector::removeLicense(const std::string& url, const bool& inner, gs::MessageRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url + "/lic/remove", json_str, body_str);
	return response_parser<gs::MessageLicenseResponse>(status, body_str);
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