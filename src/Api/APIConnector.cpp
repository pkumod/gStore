#include "APIConnector.h"


httpentities::ShutdownResponse APIConnector::shutdown(const std::string& url, httpentities::ShutdownRequest& request)
{
	std::string body_str;
	std::map<std::string, std::string> headers;
	headers.insert(std::pair<std::string, std::string>("username", request.username));
	headers.insert(std::pair<std::string, std::string>("password", request.password));
	int status = WFHttpUtil::Post(url, headers, 60, "", body_str);
	return response_parser<httpentities::ShutdownResponse>(status, body_str);
}

httpentities::CheckResponse APIConnector::check(const std::string& url, httpentities::CheckRequest& request)
{
	std::string param_str = request.to_params();
	std::string body_str;
	std::string strUrl = url + "?" + param_str;
	int status = WFHttpUtil::Get(strUrl, body_str);
	return response_parser<httpentities::CheckResponse>(status, body_str);
}

httpentities::BaseResponse APIConnector::refreshConf(const std::string& url, const bool& inner, httpentities::RefreshconfRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::TestConnectionResponse APIConnector::testConnection(const std::string& url, const bool& inner, httpentities::TestConnectionRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::TestConnectionResponse>(status, body_str);
}

httpentities::InitResponse APIConnector::init(const std::string& url, const bool& inner, httpentities::InitRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::InitResponse>(status, body_str);
}

httpentities::LoadResponse APIConnector::load(const std::string& url, const bool& inner, httpentities::LoadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::LoadResponse>(status, body_str);
}

httpentities::BaseResponse APIConnector::login(const std::string& url, httpentities::LoginRequest& request)
{
	std::string json_str;
	request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::BaseResponse APIConnector::unload(const std::string& url, const bool& inner, httpentities::UnloadRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::BuildResponse APIConnector::build(const std::string& url, const bool& inner, httpentities::BuildRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::BuildResponse>(status, body_str);
}

httpentities::BaseResponse APIConnector::drop(const std::string& url, const bool& inner, httpentities::DropRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::BaseResponse>(status, body_str);
}

httpentities::ShowResponse APIConnector::show(const std::string& url, const bool& inner, httpentities::ShowRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::ShowResponse>(status, body_str);
}

httpentities::MonitorResponse APIConnector::monitor(const std::string& url, const bool& inner, httpentities::MonitorRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::MonitorResponse>(status, body_str);
}

httpentities::QueryResponse APIConnector::query(const std::string& url, const bool& inner, httpentities::QueryRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::QueryResponse>(status, body_str);
}


httpentities::BatchInsertResponse APIConnector::batchInsert(const std::string& url, const bool& inner, httpentities::BatchInsertRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	fprintf(stderr, "xx%s", body_str.c_str());
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::BatchInsertResponse>(status, body_str);
}

httpentities::BatchRemoveResponse APIConnector::batchRemove(const std::string& url, const bool& inner, httpentities::BatchRemoveRequest& request)
{
	std::string json_str;
	if (inner)
		request.to_inner_json(json_str);
	else
		request.to_json(json_str);
	std::string body_str;
	fprintf(stderr, "xx%s", body_str.c_str());
	int status = WFHttpUtil::Post(url, json_str, body_str);
	return response_parser<httpentities::BatchRemoveResponse>(status, body_str);
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
