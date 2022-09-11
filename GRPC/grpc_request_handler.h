#ifndef _GRPC_REQUESTHANDLER_H
#define _GRPC_REQUESTHANDLER_H

#include <functional>
#include <set>
#include "grpc_message.h"
#include "grpc_stringpiece.h"

namespace grpc
{

#define ReqMethodAny		"ANY"
#define ReqMethodGet		"GET"
#define ReqMethodPost		"POST"
#define ReqMethodUnknown    "[UNKNOWN]"

enum class ReqMethod
{
    ANY, GET, POST,
};

inline ReqMethod str_to_method(const std::string &method)
{
    if (strcasecmp(method.c_str(), ReqMethodGet) == 0)
        return ReqMethod::GET;
    if (strcasecmp(method.c_str(), ReqMethodPost) == 0)
        return ReqMethod::POST;
    return ReqMethod::ANY;
}

inline const char *method_to_str(const ReqMethod &method)
{
    switch (method)
    {
        case ReqMethod::ANY:
            return ReqMethodAny;
        case ReqMethod::GET:
            return ReqMethodGet;
        case ReqMethod::POST:
            return ReqMethodPost;
        default:
            return ReqMethodUnknown;
    }
}

using WrapHandler = std::function<WFGoTask *(GRPCReq * , GRPCResp *, SeriesWork *)>;

struct RequestHandler
{
    std::map<ReqMethod, WrapHandler> request_handler_map;
    StringPiece path;
    int compute_queue_id;
};

} // namespace grpc

#endif // _GRPC_REQUESTHANDLER_H