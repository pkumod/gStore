/*
 * @Author: wangjian 2606583267@qq.com
 * @Date: 2022-08-18 18:31:53
 * @LastEditors: wangjian 2606583267@qq.com
 * @LastEditTime: 2022-09-10 01:54:48
 * @FilePath: /gstore/GRPC/grpc_message.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
#include "workflow/HttpUtil.h"
#include "workflow/Workflow.h"
#include "workflow/WFTaskFactory.h"
#include "workflow/StringUtil.h"

#include "grpc_message.h"
#include "grpc_status_code.h"
#include "grpc_stringpiece.h"
#include "grpc_server_task.h"

using namespace grpc;
using namespace protocol;

const std::string grpc::string_not_found = "";

namespace grpc
{
struct GRPCReqData
{
    std::string body;
    std::map<std::string, std::string> form_kv;
    Form form;
    Json json = NULL;
};

GRPCReq::GRPCReq() : _req_data(new GRPCReqData)
{
    
}

GRPCReq::~GRPCReq()
{
    delete _req_data;
}

std::string &GRPCReq::body() const
{
    if (_req_data->body.empty())
    {
        std::string content = protocol::HttpUtil::decode_chunked_body(this);

        const std::string &header = this->header("Content-Encoding");
        int status = StatusOK;
        if (header.find("gzip") != std::string::npos)
        {
            // status = Compressor::ungzip(&content, &_req_data->body);
            status = StatusUncompressNotSupport;
        }
        else
        {
            status = StatusNoUncomrpess;
        }
        if(status != StatusOK)
        {
            _req_data->body = std::move(content);
        }
    }
    // std::cout << "body_content = " << _req_data->body << std:: endl;
    return _req_data->body;
}

std::map<std::string, std::string> &GRPCReq::formData() const
{
    if (_content_type == APPLICATION_URLENCODED && _req_data->form_kv.empty())
    {
        _req_data->form_kv = UrlEncode::parse_post_body(this->body());
    }
    return _req_data->form_kv;
}

Form &GRPCReq::form() const
{
    if (_content_type == MULTIPART_FORM_DATA && _req_data->form.empty())
    {
        // TODO
        // _req_data->form = multi_part_.parse_multipart(this->body());
    }
    return _req_data->form;
}

Json &GRPCReq::json() const
{
    if (_content_type == APPLICATION_JSON && _req_data->json.IsNull())
    {
        const std::string &body_content = this->body();
        _req_data->json.Parse(body_content.c_str());
        if (_req_data->json.HasParseError())
        {
            fprintf(stderr, "[GRPC] %s:%s\n", "Parse json data error: ", body_content.c_str());
            _req_data->json.Clear();
        }
        return _req_data->json;
    }
    return _req_data->json;
}

const std::string &GRPCReq::header(const std::string &key) const
{
    const auto it = _headers.find(key);

    if (it == _headers.end() || it->second.empty())
        return string_not_found;

    return it->second[0];
}

bool GRPCReq::hasHeader(const std::string &key) const
{
    return _headers.count(key) > 0;
}

const std::string &GRPCReq::routeParam(const std::string &key) const
{
    if (_route_params.count(key))
        return _route_params.at(key);
    else
        return string_not_found;
}

bool GRPCReq::hasRouteParam(const std::string &key) const
{
    return _route_params.count(key) > 0;
}

const std::string &GRPCReq::queryParam(const std::string &key) const
{
    if (_query_params.count(key))
        return _query_params.at(key);
    else
        return string_not_found;
}

const std::string &GRPCReq::queryParam(const std::string &key, const std::string &default_val) const
{
    if (_query_params.count(key))
        return _query_params.at(key);
    else
        return default_val;
}

bool GRPCReq::hasQueryParam(const std::string &key) const
{
    if (_query_params.find(key) != _query_params.end())
    {
        return true;
    } 
    else
    {
        return false;
    }
}

void GRPCReq::fill_content_type()
{
    const std::string &content_type_str = header("Content-Type");
    _content_type = ContentType::to_enum(content_type_str);

    if (_content_type == MULTIPART_FORM_DATA)
    {
        // if type is multipart form, we reserve the boudary first
        const char *boundary = strstr(content_type_str.c_str(), "boundary=");
        if (boundary == nullptr)
        {
            return;
        }
        boundary += strlen("boundary=");
        StringPiece boundary_piece(boundary);

        // StringPiece boundary_str = StringPiece::trim_pairs(boundary_piece, R"(""'')");
        
        // TODO
        // multi_part_.set_boundary(boundary_str.as_string());
    }
}

void GRPCReq::fill_header_map()
{
    // std::cout << "fill_header_map begin" << std::endl;
    http_header_cursor_t cursor;
    struct protocol::HttpMessageHeader header;

    http_header_cursor_init(&cursor, this->get_parser());
    while (http_header_cursor_next(&header.name, &header.name_len,
                                   &header.value, &header.value_len,
                                   &cursor) == 0)
    {
        std::string key(static_cast<const char *>(header.name), header.name_len);
        std::string value(static_cast<const char *>(header.value), header.value_len);
        // std::cout << "headers[" << key << "]=" << value << std::endl;
        _headers[key].emplace_back(value);
    }

    http_header_cursor_deinit(&cursor);
    // std::cout << "fill_header_map end" << std::endl;
}

GRPCReq::GRPCReq(GRPCReq&& _http_request)
    : protocol::HttpRequest::HttpRequest(std::move(_http_request)),
    _content_type(_http_request._content_type),
    _headers(std::move(_http_request._headers)),
    _route_match_path(std::move(_http_request._route_match_path)),
    _route_full_path(std::move(_http_request._route_full_path)),
    _route_params(std::move(_http_request._route_params)),
    _query_params(std::move(_http_request._query_params)),
    // _cookies(std::move(_http_request._cookies)),
    // _multi_part(std::move(_http_request._multi_part)),
    _parsed_uri(std::move(_http_request._parsed_uri))
{
    _req_data = _http_request._req_data;
    _http_request._req_data = nullptr;
}

GRPCReq &GRPCReq::operator=(GRPCReq&& other)
{
    HttpRequest::operator=(std::move(other));
    _content_type = other._content_type;

    _req_data = other._req_data;
    other._req_data = nullptr;

    _route_match_path = std::move(other._route_match_path);
    _route_full_path = std::move(other._route_full_path);
    _route_params = std::move(other._route_params);
    _query_params = std::move(other._query_params);
    // _cookies = std::move(other._cookies);
    // _multi_part = std::move(other._multi_part);
    _headers = std::move(other._headers);
    _parsed_uri = std::move(other._parsed_uri);

    return *this;
}

// GRPCResp
void GRPCResp::String(const std::string &str)
{
    auto *compress_data = new std::string;
    int ret = this->compress(&str, compress_data);
    if(ret != StatusOK)   
    {
        this->append_output_body(static_cast<const void *>(str.c_str()), str.size());
    } 
    else 
    {
        this->append_output_body_nocopy(compress_data->c_str(), compress_data->size());
    }
    this->resp_code = 0;
    this->resp_msg = "Success";
    task_of(this)->add_callback([compress_data](GRPCTask *) { delete compress_data; });
}

void GRPCResp::String(std::string &&str)
{
    auto *data = new std::string;
    int ret = this->compress(&str, data);
    if(ret != StatusOK)
    {   
        *data = std::move(str);
    } 
    this->append_output_body_nocopy(data->c_str(), data->size());
    task_of(this)->add_callback([data](GRPCTask *) { delete data; });
}

void GRPCResp::File(const std::string &path)
{
    this->File(path, 0, -1);
}

void GRPCResp::File(const std::string &path, size_t start)
{
    this->File(path, start, -1);
}

void GRPCResp::File(const std::string &path, size_t start, size_t end)
{
    // TODO
    // int ret = HttpFile::send_file(path, start, end, this);
    int ret = StatusCode::StatusFileWriteError;
    if(ret != StatusOK)
    {
        this->Error(ret);
    }
}

void GRPCResp::Json(const ::Json &json)
{
    this->headers["Content-Type"] = ContentType::to_str(APPLICATION_JSON);
    rapidjson::StringBuffer resBuffer;
    rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
    json.Accept(resWriter);
    if (json.HasMember("StatusCode") && json["StatusCode"].IsInt())
    {
        this->resp_code = json["StatusCode"].GetInt();
    }
    if (json.HasMember("StatusMsg") && json["StatusMsg"].IsString())
    {
        this->resp_msg = json["StatusMsg"].GetString();
    }
    this->String(resBuffer.GetString());
}

void GRPCResp::Json(const std::string &str)
{
    ::Json json_doc;
    json_doc.Parse(str.c_str());
    if (json_doc.HasParseError())
    {
        this->Error(StatusJsonInvalid);
        return;
    }
    this->headers["Content-Type"] = ContentType::to_str(APPLICATION_JSON);
    this->String(str);
}

void GRPCResp::set_status(int status_code)
{
    protocol::HttpUtil::set_response_status(this, status_code);
}

int GRPCResp::get_state() const
{
    GRPCServerTask *server_task = task_of(this);
    return server_task->get_state();   
}

int GRPCResp::get_error() const
{
    GRPCServerTask *server_task = task_of(this);
    return server_task->get_error();   
}

void GRPCResp::Success(const std::string &msg)
{
    this->headers["Content-Type"] = ContentType::to_str(APPLICATION_JSON);
    ::Json js;
    js.SetObject();
    ::Json::AllocatorType &allocator = js.GetAllocator();
    std::string code_msg;
    if(!msg.empty())
    {
        code_msg = msg;
    } 
    else
    {
        code_msg = error_code_to_str(StatusOK);
    }
    js.AddMember("StatusCode", StatusOK, allocator);
    js.AddMember("StatusMsg", rapidjson::StringRef(code_msg.c_str()), allocator);
    
    this->Json(js);
}

void GRPCResp::Error(int error_code)
{
    this->Error(error_code, "");
}

void GRPCResp::Error(int error_code, const std::string &errmsg)
{
    this->headers["Content-Type"] = ContentType::to_str(APPLICATION_JSON);
    ::Json js;
    ::Json::AllocatorType &allocator = js.GetAllocator();
    js.SetObject();
    std::string code_msg;
    if (errmsg.empty())
    {
       code_msg = error_code_to_str(error_code);
    }
    else
    {
        code_msg = errmsg;
        if (UrlEncode::is_url_encode(code_msg))
        {
            StringUtil::url_decode(code_msg);
        }
    }
    js.AddMember("StatusCode", error_code, allocator);
    js.AddMember("StatusMsg", rapidjson::StringRef(code_msg.c_str()), allocator);   
    this->Json(js);
}

void GRPCResp::add_task(SubTask *task)
{
    GRPCServerTask *server_task = task_of(this);
    **server_task << task;
}

int GRPCResp::compress(const std::string * const data, std::string *compress_data)
{
    int status = StatusOK;
    if (headers.find("Content-Encoding") != headers.end())
    {
        if (headers["Content-Encoding"].find("gzip") != std::string::npos)
        {
            // TODO
            // status = Compressor::gzip(data, compress_data);
        }
    } else 
    {
        status = StatusNoComrpess;
    }
    return status;
}

GRPCResp::GRPCResp(GRPCResp&& other)
    : protocol::HttpResponse::HttpResponse(std::move(other)),
    headers(std::move(other.headers))
{
    user_data = other.user_data;
    resp_code = other.resp_code;
    resp_msg = other.resp_msg;
    other.user_data = nullptr;
}

GRPCResp &GRPCResp::operator=(GRPCResp&& other)
{
    GRPCResp::operator=(std::move(other));
    headers = std::move(other.headers);
    user_data = other.user_data;
    resp_code = other.resp_code;
    resp_msg = other.resp_msg;
    other.user_data = nullptr;
    return *this;
}

} // namespace grpc