#ifndef _GRPC_ROUTETABLE_H
#define _GRPC_ROUTETABLE_H

#include <vector>
#include <memory>
#include <cassert>
#include <unordered_map>

#include "grpc_noncopyable.h"
#include "grpc_request_handler.h"
#include "grpc_stringpiece.h"

namespace grpc
{

class RouteTableNode : public Noncopyable
{
public:
    ~RouteTableNode();

    struct iterator
    {
        const RouteTableNode *ptr;
        StringPiece first;
        RequestHandler second;

        iterator *operator->()
        { return this; }

        bool operator==(const iterator &other) const
        { return this->ptr == other.ptr; }

        bool operator!=(const iterator &other) const
        { return this->ptr != other.ptr; }
    };

    RequestHandler &find_or_create(const StringPiece &route, size_t cursor);

    iterator end() const
    { return iterator{nullptr, StringPiece(), RequestHandler()}; }

    iterator find(const StringPiece &route,
                  size_t cursor,
                  std::map<std::string, std::string> &route_params,
                  std::string &route_match_path) const;

    template<typename Func>
    void all_routes(const Func &func, std::string prefix) const;
    
private:
    RequestHandler request_handler_;
    std::map<StringPiece, RouteTableNode *> children_;
};

template<typename Func>
void RouteTableNode::all_routes(const Func &func, std::string prefix) const
{
    if (children_.empty())
    {
        func(prefix, request_handler_);
    } else
    {
        if (!prefix.empty() && prefix.back() != '/')
            prefix += '/';
        for (auto &pair: children_)
        {
            pair.second->all_routes(func, prefix + pair.first.as_string());
        }
    }
}

class RouteTable : public Noncopyable
{ 
public:
    // Find a route and return reference to the procedure.
    RequestHandler &find_or_create(const char *route);

    RouteTableNode::iterator find(const StringPiece &route, 
                                std::map<std::string, std::string> &route_params,
                                std::string &route_match_path) const
    { return _root.find(route, 0, route_params, route_match_path); }

    template<typename Func>
    void all_routes(const Func &func) const
    { _root.all_routes(func, ""); }

    RouteTableNode::iterator end() const
    { return _root.end(); }

    ~RouteTable() = default;
    
private:
    RouteTableNode _root;
    std::set<StringPiece> _string_pieces;
};

} // namespace grpc

#endif // _GRPC_ROUTETABLE_H