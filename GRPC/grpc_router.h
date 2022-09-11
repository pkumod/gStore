#ifndef _GRPC_ROUTER_H
#define _GRPC_ROUTER_H

#include <functional>
#include "grpc_routetable.h"
#include "grpc_noncopyable.h"
#include "grpc_server_task.h"

namespace grpc
{

class GRPCServerTask;

class Router : public Noncopyable
{
public:
    void handle(const std::string &route, int compute_queue_id, const WrapHandler &handler, ReqMethod method);

    int call(ReqMethod method, const std::string &route, GRPCServerTask *server_task) const;

    void print_routes() const;   // for logging

    std::vector<std::pair<std::string, std::string>> all_routes() const;   // for test 

    struct RouteVerb
    {
        std::string route;
        mutable std::set<ReqMethod> verbs;
        bool operator()(const RouteVerb& l, const RouteVerb& r) const
        {
            return l.route > r.route;
        }
    };

    using RouteVerbIter = std::set<RouteVerb>::iterator;

    std::pair<RouteVerbIter, bool> add_route(ReqMethod method, const std::string &route);

    std::pair<RouteVerbIter, bool> add_route(const std::vector<ReqMethod> &methods, const std::string &route);
    
private:
    RouteTable _routes_map;
    std::set<RouteVerb, RouteVerb> _routes;  // for store 
};

} // namespace grpc

#endif // _GRPC_ROUTER_H
