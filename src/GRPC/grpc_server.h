#ifndef _GRPC_SERVER_H
#define _GRPC_SERVER_H

#include "workflow/WFHttpServer.h"
#include "workflow/HttpUtil.h"
#include "workflow/StringUtil.h"

#include <unordered_map>
#include <string>

#include "grpc_message.h"
#include "grpc_router.h"

namespace grpc
{
using DefaultHandler = std::function<void(const GRPCReq *, GRPCResp *)>;

using SeriesHandler = std::function<void(const GRPCReq *, GRPCResp *, SeriesWork *)>;

class GRPCServer : public WFServer<GRPCReq, GRPCResp>, public Noncopyable
{
public:

    void ROUTE(const std::string &route, const DefaultHandler &handler, ReqMethod method);

    void ROUTE(const std::string &route, const DefaultHandler &handler, int compute_queue_id, ReqMethod method);

    void ROUTE(const std::string &route, const DefaultHandler &handler, const std::vector<std::string> &methods);

    void ROUTE(const std::string &route, const DefaultHandler &handler, int compute_queue_id, 
                const std::vector<std::string> &methods);

    void GET(const std::string &route, const DefaultHandler &handler);

    void GET(const std::string &route, const DefaultHandler &handler, int compute_queue_id);

    void POST(const std::string &route, const DefaultHandler &handler);

    void POST(const std::string &route, const DefaultHandler &handler, int compute_queue_id);

// public:
//     template<typename... AP>
//     void ROUTE(const std::string &route, const DefaultHandler &handler, 
//                 const std::vector<std::string> &methods, const AP &... ap);

//     template<typename... AP>
//     void ROUTE(const std::string &route, const DefaultHandler &handler, 
//                 int compute_queue_id, 
//                 const std::vector<std::string> &methods, const AP &... ap);

//     template<typename... AP>
//     void GET(const std::string &route, const DefaultHandler &handler, const AP &... ap);

//     template<typename... AP>
//     void GET(const std::string &route, const DefaultHandler &handler, 
//              int compute_queue_id, const AP &... ap);

//     template<typename... AP>
//     void POST(const std::string &route, const DefaultHandler &handler, const AP &... ap);

//     template<typename... AP>
//     void POST(const std::string &route, const DefaultHandler &handler, 
//               int compute_queue_id, const AP &... ap);

public:

    void ROUTE(const std::string &route, const SeriesHandler &handler, ReqMethod verb);

    void ROUTE(const std::string &route, const SeriesHandler &handler, int compute_queue_id, ReqMethod verb);

    void ROUTE(const std::string &route, const SeriesHandler &handler, const std::vector<std::string> &methods);

    void ROUTE(const std::string &route, const SeriesHandler &handler, int compute_queue_id, 
                const std::vector<std::string> &methods);
    
    void GET(const std::string &route, const SeriesHandler &handler);

    void GET(const std::string &route, const SeriesHandler &handler, int compute_queue_id);

    void POST(const std::string &route, const SeriesHandler &handler);

    void POST(const std::string &route, const SeriesHandler &handler, int compute_queue_id); 

// public:
//     template<typename... AP>
//     void ROUTE(const std::string &route, const SeriesHandler &handler, 
//                 const std::vector<std::string> &methods, const AP &... ap);

//     template<typename... AP>
//     void ROUTE(const std::string &route, int compute_queue_id, 
//                 const SeriesHandler &handler, 
//                 const std::vector<std::string> &methods, const AP &... ap);
                
//     template<typename... AP>
//     void GET(const std::string &route, const SeriesHandler &handler, const AP &... ap);

//     template<typename... AP>
//     void GET(const std::string &route, int compute_queue_id,
//              const SeriesHandler &handler, const AP &... ap);

//     template<typename... AP>
//     void POST(const std::string &route, const SeriesHandler &handler, const AP &... ap);

//     template<typename... AP>
//     void POST(const std::string &route, int compute_queue_id,
//               const SeriesHandler &handler, const AP &... ap);

public:
    GRPCServer() :
            WFServer(std::bind(&GRPCServer::process, this, std::placeholders::_1))
    {}

    GRPCServer &max_connections(size_t max_connections)
    {
        this->params.max_connections = max_connections;
        return *this;
    }

    GRPCServer &peer_response_timeout(int peer_response_timeout)
    {
        this->params.peer_response_timeout = peer_response_timeout;
        return *this;
    }

    GRPCServer &receive_timeout(int receive_timeout)
    {
        this->params.receive_timeout = receive_timeout;
        return *this;
    }

    GRPCServer &keep_alive_timeout(int keep_alive_timeout)
    {
        this->params.keep_alive_timeout = keep_alive_timeout;
        return *this;
    }

    GRPCServer &request_size_limit(size_t request_size_limit)
    {
        this->params.request_size_limit = request_size_limit;
        return *this;
    }

    GRPCServer &ssl_accept_timeout(int ssl_accept_timeout)
    {
        this->params.ssl_accept_timeout = ssl_accept_timeout;
        return *this;
    }

    using TrackFunc = std::function<void(GRPCTask *server_task)>;
    
    GRPCServer &track();

    GRPCServer &track(const TrackFunc &track_func);

    GRPCServer &track(TrackFunc &&track_func);

public:
    const Router& router() const
    {
        return router_;
    }

protected:
    CommSession *new_session(long long seq, CommConnection *conn) override;

private:
    void process(GRPCTask *task);

private:
    Router router_;
    TrackFunc track_func_;
};
} // namespace grpc

#endif // _GRPC_SERVER_H