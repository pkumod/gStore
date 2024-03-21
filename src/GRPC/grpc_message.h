#ifndef _GRPC_MESSAGE_H
#define _GRPC_MESSAGE_H

#include "workflow/HttpMessage.h"
#include "workflow/WFTaskFactory.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "grpc_noncopyable.h"
#include "grpc_content.h"

namespace grpc
{

using Json = rapidjson::Document;

struct GRPCReqData;

struct SaveFileContext;

void pread_callback(WFFileIOTask *pread_task);

void pwrite_callback(WFFileIOTask *pwrite_task);

extern const std::string string_not_found;

class GRPCReq : public protocol::HttpRequest, public Noncopyable
{
public:
    std::string &body() const;

    std::map<std::string, std::string> &formData() const;

    Form &form() const;

    Json &json() const;

    content_type contentType() const
    {
        return _content_type;
    }

    const std::string &header(const std::string &key) const;

    bool hasHeader(const std::string &key) const;

    const std::string &routeParam(const std::string &key) const;

    bool hasRouteParam(const std::string &key) const;

    const std::string &queryParam(const std::string &key) const;

    const std::string &queryParam(const std::string &key, const std::string &default_val) const;

    const std::map<std::string, std::string> &queryList() const
    { 
        return _query_params; 
    }

    bool hasQueryParam(const std::string &key) const;

    std::string current_path() const
    { 
        return _parsed_uri.path;
    }

public:
    void fill_content_type();
    
    void fill_header_map();

    void set_full_path(const std::string &route_full_path)
    { 
        _route_full_path = route_full_path; 
    }

     // /{name}/{id} params in route
    void set_route_params(std::map<std::string, std::string> &&params)
    { 
        _route_params = std::move(params); 
    }

    // /match*  
    // /match123 -> match123
    void set_route_match_path(const std::string &match_path)
    { 
        _route_match_path = match_path; 
    }

    void set_full_path(std::string &&route_full_path)
    { 
        _route_full_path = std::move(route_full_path); 
    }

    void set_query_params(std::map<std::string, std::string> &&query_params)
    { 
        _query_params = std::move(query_params);
    }

    void set_parsed_uri(ParsedURI &&parsed_uri)
    { 
        _parsed_uri = std::move(parsed_uri); 
    }

public:
    GRPCReq();

    GRPCReq(protocol::HttpRequest &&_http_request) 
        : protocol::HttpRequest::HttpRequest(std::move(_http_request))
    {}

    ~GRPCReq();

    GRPCReq(GRPCReq&& other);

    GRPCReq &operator=(GRPCReq&& other);
private:
    using HeaderMap = std::map<std::string, std::vector<std::string>, MapStringCaseLess>;

    content_type _content_type;

    GRPCReqData *_req_data;
    
    HeaderMap _headers;

    std::string _route_match_path;
    std::string _route_full_path;

    std::map<std::string, std::string> _route_params;
    std::map<std::string, std::string> _query_params;

    MultiPartForm _multi_part;

    ParsedURI _parsed_uri;
};

class GRPCResp : public protocol::HttpResponse, public Noncopyable
{
public:
    // send string
    void String(const std::string &str);

    void String(std::string &&str);

    // send file 
    void File(const std::string &path);

    // save file
    void Save(const std::string &file_dst, const std::string &content, const std::string &notify_msg);

    // send json string
    void Json(const Json &json);

    void Json(const std::string &str);

    // TODO compress with gzip
    // void set_compress(const Compress &compress);

    void set_status(int status_code);

    int get_state() const; 

    int get_error() const;

    template<class FUNC, class... ARGS>
    void Compute(int compute_queue_id, FUNC&& func, ARGS&&... args)
    {
        WFGoTask *go_task = WFTaskFactory::create_go_task(
                "grpc" + std::to_string(compute_queue_id),
                std::forward<FUNC>(func),
                std::forward<ARGS>(args)...);
        this->add_task(go_task);
    }

    void Success(const std::string &msg);

    void Error(int error_code);

    void Error(int error_code, const std::string &errmsg);

    void add_task(SubTask *task);

private:
    int compress(const std::string * const data, void *compress_data, size_t &compress_size);

public:
    GRPCResp() = default;

    GRPCResp(protocol::HttpResponse && _http_resp) 
        : protocol::HttpResponse::HttpResponse(std::move(_http_resp))
    {}

    ~GRPCResp() = default;

    GRPCResp(GRPCResp&& _grpc_resp);

    GRPCResp &operator=(GRPCResp&& _grpc_resp);
    
public:
    std::map<std::string, std::string, MapStringCaseLess> headers;
    void *user_data;
    int resp_code;
    std::string resp_msg;
};

class GRPCUtil
{
public:
    static int fileSize(const std::string &path, size_t *size);
    
    // path = /usr/local/upload/test.nt
    // suffix = nt
    static std::string fileSuffix(const std::string &path);

    // path = /usr/local/upload/test.nt
    // name = test.nt
    static std::string fileName(const std::string &path);

    static bool isFile(const std::string &path);

    static bool isDir(const std::string &path);
public:
    static int send_file(const std::string &path, size_t start, size_t end, GRPCResp *resp);

    static void saveFile(const std::string &dst_path, const std::string &content,  GRPCResp *resp, const std::string &notify_msg);
};
using GRPCTask = WFNetworkTask<GRPCReq, GRPCResp>;

} // namespace grpc

#endif // _GRPC_MESSAGE_H