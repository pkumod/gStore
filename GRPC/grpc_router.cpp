#include "workflow/HttpUtil.h"
#include "workflow/StringUtil.h"

#include "grpc_router.h"
#include "grpc_message.h"
#include "grpc_status_code.h"

using namespace grpc;

void Router::handle(const std::string &route, int compute_queue_id, const WrapHandler &handler, ReqMethod method)
{
    std::pair<RouteVerbIter, bool> rv_pair = add_route(method, route);
    RequestHandler &vh = _routes_map.find_or_create(rv_pair.first->route.c_str());
    if(vh.request_handler_map.find(method) != vh.request_handler_map.end()) 
    {
        fprintf(stderr, "Duplicate Method\n");
        return;
    }

    vh.request_handler_map.insert({method, handler});
    vh.path = rv_pair.first->route;
    vh.compute_queue_id = compute_queue_id;
}

int Router::call(ReqMethod method, const std::string &route, GRPCServerTask *server_task) const
{
    GRPCReq *req = server_task->get_req();
    GRPCResp *resp = server_task->get_resp();

    // skip the last / of the url. Except for /
    // /hello ==  /hello/
    // / not change
    StringPiece route2(route);
    if (route2.size() > 1 and route2[static_cast<int>(route2.size()) - 1] == '/')
        route2.remove_suffix(1);
        
    std::map<std::string, std::string> route_params;
    std::string route_match_path;
    auto it = _routes_map.find(route2, route_params, route_match_path);

    int error_code = StatusOK;
    if (it != _routes_map.end())   // has route
    {
        // match verb
        // it == <StringPiece : path, WrapHandler>
        std::map<ReqMethod, WrapHandler> &method_handler_map = it->second.request_handler_map;
        bool has_method = method_handler_map.find(method) != method_handler_map.end() ? true : false;
        if(method_handler_map.find(ReqMethod::ANY) != method_handler_map.end() or has_method)
        {
            req->set_full_path(it->second.path.as_string());
            req->set_route_params(std::move(route_params));
            req->set_route_match_path(std::move(route_match_path));
            WFGoTask * go_task;
            if(has_method) 
            {
                go_task = it->second.request_handler_map[method](req, resp, series_of(server_task));
            } else 
            {
                go_task = it->second.request_handler_map[ReqMethod::ANY](req, resp, series_of(server_task));
            }
            if(go_task)
                **server_task << go_task;
        } else
        {
            error_code = StatusRouteVerbNotImplment;
        }
    } 
    else
    {
        error_code = StatusRouteNotFound;
    }
    return error_code;
}

void Router::print_routes() const
{
    for(auto &rv : _routes) 
    {   
        for(auto verb : rv.verbs)
        {
            if (UrlEncode::is_url_encode(rv.route)) 
            {
                std::string route = std::move(rv.route);
                StringUtil::url_decode(route);
                fprintf(stderr, "[GRPC] %s\t%s\n", method_to_str(verb), route.c_str());
            } 
            else 
            {
                fprintf(stderr, "[GRPC] %s\t%s\n", method_to_str(verb), rv.route.c_str());
            }
        }
    }
}

std::vector<std::pair<std::string, std::string> > Router::all_routes() const
{
    std::vector<std::pair<std::string, std::string> > res;
    _routes_map.all_routes([&res](const std::string &prefix, const RequestHandler &request_handler)
                        {
                            for(auto& vh : request_handler.request_handler_map)
                            {
                                res.emplace_back(method_to_str(vh.first), prefix.c_str());
                            }
                        });
    return res;
}

std::pair<Router::RouteVerbIter, bool> Router::add_route(ReqMethod method, const std::string &route)
{
    RouteVerb rv;
    rv.route = route;
    auto it = _routes.find(rv);
    if(it != _routes.end()) 
    {
        it->verbs.insert(method);
    } else 
    {
        rv.verbs.insert(method);
    }
    return _routes.emplace(std::move(rv));
}

std::pair<Router::RouteVerbIter, bool> Router::add_route(const std::vector<ReqMethod> &methods, const std::string &route) 
{
    RouteVerb rv;
    rv.route = route;
    auto it = _routes.find(rv);
    if(it != _routes.end()) 
    {
        for(auto method : methods) 
            it->verbs.insert(method);
    } else 
    {
        for(auto method : methods) 
            rv.verbs.insert(method);
    }
    return _routes.emplace(std::move(rv));
}