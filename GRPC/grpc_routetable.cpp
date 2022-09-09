#include <queue>
#include "grpc_routetable.h"

using namespace grpc;

RouteTableNode::~RouteTableNode()
{
    for (auto &child: children_)
    {
        delete child.second;
    }
}

RequestHandler &RouteTableNode::find_or_create(const StringPiece &route, size_t cursor)
{
    if (cursor == route.size())
        return request_handler_;

    // store GET("/", ...)
    if(cursor == 0 && route.as_string() == "/")
    {
        auto *new_node = new RouteTableNode();
        children_.insert({route, new_node});
        return new_node->find_or_create(route, ++cursor);
    }
    // ignore the last '/'
    if (cursor == route.size() - 1 && route[cursor] == '/')
    {
        return request_handler_;
    }

    if (route[cursor] == '/')
        cursor++; // skip the /
    int anchor = cursor;
    while (cursor < route.size() and route[cursor] != '/')
        cursor++;
    // get the '/ {mid} /' part
    StringPiece mid(route.begin() + anchor, cursor - anchor);
    auto it = children_.find(mid);
    if (it != children_.end())
    {
        return it->second->find_or_create(route, cursor);
    } else
    {
        auto *new_node = new RouteTableNode();
        children_.insert({mid, new_node});
        return new_node->find_or_create(route, cursor);
    }
}

RouteTableNode::iterator RouteTableNode::find(const StringPiece &route,
                                        size_t cursor,
                                        std::map<std::string, std::string> &route_params,
                                        std::string &route_match_path) const
{
    assert(cursor >= 0);
    // We found the route
    if ((cursor == route.size() and !request_handler_.request_handler_map.empty()) or (children_.empty()))
        return iterator{this, route, request_handler_};
    // /*
    if(cursor == route.size() and !children_.empty())
    {
        auto it = children_.find(StringPiece("*"));
        if(it != children_.end())
        {
            if(it->second->request_handler_.request_handler_map.empty())
                fprintf(stderr, "handler nullptr");
            return iterator{it->second, route, it->second->request_handler_};
        }
    }

    // route does not match any.
    if (cursor == route.size() and request_handler_.request_handler_map.empty())
        return iterator{nullptr, route, request_handler_};

    // find GET("/", ...)
    if(cursor == 0 && route == "/")
    {
        // look for "/" in the children.
        auto it = children_.find(route);
        // it == <StringPiece: path level part, RouteTableNode* >
        if (it != children_.end())
        {
            // it2 == RouteTableNode::iterator
            // search in the corresponding child.
            auto it2 = it->second->find(route, ++cursor, route_params, route_match_path); 
            if (it2 != it->second->end())
                return it2;
        }
    }   

    if (route[cursor] == '/')
        cursor++; // skip the first /
    // Find the next /.
    // mark an anchor here
    int anchor = cursor;
    while (cursor < route.size() and route[cursor] != '/')
        cursor++;

    // mid is the string between the 2 /.
    // / {mid} /
    StringPiece mid(route.begin() + anchor, cursor - anchor);

    // look for mid in the children.
    auto it = children_.find(mid);
    // it == <StringPiece: path level part, RouteTableNode* >
    if (it != children_.end())
    {
        // it2 == RouteTableNode::iterator
        auto it2 = it->second->find(route, cursor, route_params, route_match_path); // search in the corresponding child.
        if (it2 != it->second->end())
            return it2;
    }

    // if one child is an url param {name}, choose it
    for (auto &kv: children_)
    {
        StringPiece param(kv.first);
        if (!param.empty() && param[param.size() - 1] == '*')
        {
            StringPiece match(param);
            match.remove_suffix(1);
            if (mid.starts_with(match))
            {
                StringPiece match_path(route.data() + cursor);
                route_match_path = mid.as_string() + match_path.as_string();
                return iterator{kv.second, route, kv.second->request_handler_};
            } 
        }

        if (param.size() > 2 and param[0] == '{' and
            param[param.size() - 1] == '}')
        {
            int i = 1;
            int j = param.size() - 2;
            while (param[i] == ' ') i++;
            while (param[j] == ' ') j--;

            param.shrink(i, param.size() - 1 - j);
            route_params[param.as_string()] = mid.as_string();
            return kv.second->find(route, cursor, route_params, route_match_path);
        }
    }
    return end();
}

RequestHandler &RouteTable::find_or_create(const char *route)
{
    // Use pointer to prevent iterator invalidation
    // StringPiece is only a watcher, so we should store the string.
    StringPiece route_piece(route);
    auto it = _string_pieces.find(route_piece);
    if(it != _string_pieces.end())
    {
        return _root.find_or_create(*it, 0);
    }
    _string_pieces.insert(route_piece);
    return _root.find_or_create(route_piece, 0);
}