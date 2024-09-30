#ifndef _APICONNECTOR_H
#define _APICONNECTOR_H

#include "HttpEntities.h"
#include "WorkFlowHttpUtil.hpp"

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
	static httpentities::ShutdownResponse shutdown(const std::string& url, httpentities::ShutdownRequest& request);

	static httpentities::CheckResponse check(const std::string& url, httpentities::CheckRequest& request);

	static httpentities::BaseResponse refreshConf(const std::string& url, const bool& inner, httpentities::RefreshconfRequest& request);

	static httpentities::TestConnectionResponse testConnection(const std::string& url, const bool& inner, httpentities::TestConnectionRequest& request);

	static httpentities::InitResponse init(const std::string& url, const bool& inner, httpentities::InitRequest& request);

	static httpentities::LoadResponse load(const std::string& url, const bool& inner, httpentities::LoadRequest& request);

	static httpentities::BaseResponse login(const std::string& url, httpentities::LoginRequest& request);

	static httpentities::BaseResponse unload(const std::string& url, const bool& inner, httpentities::UnloadRequest& request);

	static httpentities::BuildResponse build(const std::string& url, const bool& inner, httpentities::BuildRequest& request);

	static httpentities::BaseResponse drop(const std::string& url, const bool& inner, httpentities::DropRequest& request);

	static httpentities::ShowResponse show(const std::string& url, const bool& inner, httpentities::ShowRequest& request);

	static httpentities::MonitorResponse monitor(const std::string& url, const bool& inner, httpentities::MonitorRequest& request);

	static httpentities::QueryResponse query(const std::string& url, const bool& inner, httpentities::QueryRequest& request);

	static httpentities::BatchInsertResponse batchInsert(const std::string& url, const bool& inner, httpentities::BatchInsertRequest& request);

	static httpentities::BatchRemoveResponse batchRemove(const std::string& url, const bool& inner, httpentities::BatchRemoveRequest& request);

	static httpentities::ClusterResponse reply(const std::string& url, httpentities::ReplyRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse appendEntries(const std::string& url, httpentities::AppenEntriesRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse heartBeat(const std::string& url, httpentities::HeartBeatRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse cancel(const std::string& url, httpentities::CancelRequest& request, const std::string& username, const std::string& password);

	static httpentities::ClusterResponse clusterCheck(const std::string& url, httpentities::ClusterCheckRequest& request, const std::string& username, const std::string& password);
};


#endif