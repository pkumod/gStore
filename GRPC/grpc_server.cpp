#include "workflow/HttpMessage.h"

#include <utility>
#include "grpc_server.h"
#include "grpc_server_task.h"
#include "grpc_request_handler.h"
#include "grpc_status_code.h"

using namespace grpc;

void GRPCServer::ROUTE(const std::string &route, const DefaultHandler &handler, ReqMethod method)
{
    WrapHandler wrap_handler = 
            [handler](const GRPCReq *req, GRPCResp *resp, SeriesWork *) -> WFGoTask *
            {
                handler(req, resp);
                return nullptr;
            };

    router_.handle(route, -1, wrap_handler, method);
}

void GRPCServer::ROUTE(const std::string &route, const DefaultHandler &handler, int compute_queue_id, ReqMethod method)
{
    WrapHandler wrap_handler =
            [handler, compute_queue_id](const GRPCReq *req, GRPCResp *resp, SeriesWork *) -> WFGoTask *
            {
                WFGoTask *go_task = WFTaskFactory::create_go_task(
                        "grpc" + std::to_string(compute_queue_id),
                        handler,
                        req,
                        resp);
                return go_task;
            };

    router_.handle(route, compute_queue_id, wrap_handler, method);    
}

void GRPCServer::ROUTE(const std::string &route, const DefaultHandler &handler, const std::vector<std::string> &methods)
{
    for(const auto &method : methods)
    {
        this->ROUTE(route, handler, str_to_method(method));
    }
}

void GRPCServer::ROUTE(const std::string &route, const DefaultHandler &handler, int compute_queue_id, 
                const std::vector<std::string> &methods)
{
    for(const auto &method : methods)
    {
        this->ROUTE(route, handler, compute_queue_id, str_to_method(method));
    }
}

void GRPCServer::GET(const std::string &route, const DefaultHandler &handler)
{
    this->ROUTE(route, handler, ReqMethod::GET);
}

void GRPCServer::GET(const std::string &route, const DefaultHandler &handler, int compute_queue_id)
{
    this->ROUTE(route, handler, compute_queue_id, ReqMethod::GET);
}

void GRPCServer::POST(const std::string &route, const DefaultHandler &handler)
{
    this->ROUTE(route, handler, ReqMethod::POST);
}

void GRPCServer::POST(const std::string &route, const DefaultHandler &handler, int compute_queue_id)
{
    this->ROUTE(route, handler, compute_queue_id, ReqMethod::POST);
}

void GRPCServer::ROUTE(const std::string &route, const SeriesHandler &handler, ReqMethod verb)
{
    WrapHandler wrap_handler =
            [handler, this](const GRPCReq *req, GRPCResp *resp, SeriesWork *series) -> WFGoTask *
            {
                handler(req, resp, series);
                return nullptr;
            };

    router_.handle(route, -1, wrap_handler, verb);
}

void GRPCServer::ROUTE(const std::string &route, const SeriesHandler &handler, int compute_queue_id, ReqMethod verb)
{
    WrapHandler wrap_handler =
            [handler, compute_queue_id, this](GRPCReq *req, GRPCResp *resp, SeriesWork *series) -> WFGoTask *
            {
                WFGoTask *go_task = WFTaskFactory::create_go_task(
                        "grpc" + std::to_string(compute_queue_id),
                        handler,
                        req,
                        resp,
                        series);
                return go_task;
            };

    router_.handle(route, compute_queue_id, wrap_handler, verb);
}

void GRPCServer::ROUTE(const std::string &route, const SeriesHandler &handler, const std::vector<std::string> &methods)
{
    for(const auto &method : methods)
    {
        this->ROUTE(route, handler, str_to_method(method));
    }
}

void GRPCServer::ROUTE(const std::string &route, const SeriesHandler &handler, int compute_queue_id, 
            const std::vector<std::string> &methods)
{
    for(const auto &method : methods)
    {
        this->ROUTE(route, handler, compute_queue_id, str_to_method(method));
    }
}

void GRPCServer::GET(const std::string &route, const SeriesHandler &handler)
{
    this->ROUTE(route, handler, ReqMethod::GET);
}

void GRPCServer::GET(const std::string &route, const SeriesHandler &handler, int compute_queue_id)
{
     this->ROUTE(route, handler, compute_queue_id, ReqMethod::GET);
}

void GRPCServer::POST(const std::string &route, const SeriesHandler &handler)
{
    this->ROUTE(route, handler, ReqMethod::POST);
}

void GRPCServer::POST(const std::string &route, const SeriesHandler &handler, int compute_queue_id)
{
    this->ROUTE(route, handler, compute_queue_id, ReqMethod::POST);
}

void GRPCServer::process(GRPCTask *task)
{
    auto server_task = static_cast<GRPCServerTask *>(task);

    auto *req = server_task->get_req();
    auto *resp = server_task->get_resp();

    req->fill_header_map();
    req->fill_content_type();

    const std::string &host = req->header("Host");

    if (host.empty())
    {
        resp->set_status(HttpStatusBadRequest);
        return;
    }

    std::string request_uri = "http://" + host + req->get_request_uri();
    ParsedURI uri;
    if (URIParser::parse(request_uri, uri) < 0)
    {
        resp->set_status(HttpStatusBadRequest);
        return;
    }

    std::string route;

    if (uri.path && uri.path[0])
        route = uri.path;
    else
        route = "/";

    if (uri.query)
    {
        char* query(uri.query);
        req->set_query_params(URIParser::split_query(query));
    }

    req->set_parsed_uri(std::move(uri));
    std::string method = req->get_method();
    route = StringUtil::url_encode(route);
    int ret = router().call(str_to_method(method), route, server_task);
    if (ret != StatusOK)
    {
        resp->Error(ret, method + " " + route);
    }
    if (track_func_)
    {
        server_task->add_callback(track_func_);
    }
}

CommSession *GRPCServer::new_session(long long seq, CommConnection *conn)
{
    GRPCTask *task = new GRPCServerTask(this, this->WFServer<GRPCReq, GRPCResp>::process);
    task->set_keep_alive(this->params.keep_alive_timeout);
    task->set_receive_timeout(this->params.receive_timeout);
    task->get_req()->set_size_limit(this->params.request_size_limit);

    return task;
}

GRPCServer &GRPCServer::track()
{
    track_func_ = [](GRPCTask *server_task)
    {
        GRPCResp *resp = server_task->get_resp();
        GRPCReq *req = server_task->get_req();
        GRPCServerTask *task = static_cast<GRPCServerTask *>(server_task);
        Timestamp current_time = Timestamp::now();
        std::string fmt_time = current_time.to_format_str();
        // time | http status code | peer ip address | verb | route path
        fprintf(stderr, "[GRPC] %s | %s | %s : %d | %s | \"%s\" | -- \n",
                fmt_time.c_str(),
                resp->get_status_code(),
                task->peer_addr().c_str(),
                task->peer_port(),
                req->get_method(),
                req->current_path().c_str());
    };
    return *this;
}

GRPCServer &GRPCServer::track(const TrackFunc &track_func)
{
    track_func_ = track_func;
    return *this;
}

GRPCServer &GRPCServer::track(TrackFunc &&track_func)
{
    track_func_ = std::move(track_func);
    return *this;
}