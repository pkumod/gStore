/*
 * @Author: wangjian 2606583267@qq.com
 * @Date: 2022-08-18 18:31:53
 * @LastEditors: wangjian 2606583267@qq.com
 * @LastEditTime: 2022-11-30 15:40:19
 * @FilePath: /gstore/GRPC/grpc_message.cpp
 * @Description: grp message
 */
#include <sys/stat.h>
#include "workflow/HttpUtil.h"
#include "workflow/Workflow.h"
#include "workflow/WFTaskFactory.h"
#include "workflow/StringUtil.h"

#include "grpc_message.h"
#include "grpc_status_code.h"
#include "grpc_stringpiece.h"
#include "grpc_server_task.h"
#include "../Util/CompressFileUtil.h"

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


struct SaveFileContext 
{
    std::string content;
    std::string notify_msg;
};

void pread_callback(WFFileIOTask *pread_task)
{
    FileIOArgs *args = pread_task->get_args();
    long ret = pread_task->get_retval();
    auto *resp = static_cast<GRPCResp *>(pread_task->user_data);
    resp->headers["Access-Control-Allow-Origin"] = "*";
    if (pread_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
    {
        resp->Error(StatusFileReadError);
    } else
    {
        resp->append_output_body_nocopy(args->buf, ret);
    }
}

void pwrite_callback(WFFileIOTask *pwrite_task)
{
    long ret = pwrite_task->get_retval();
    GRPCServerTask *server_task = task_of(pwrite_task);
    GRPCResp *resp = server_task->get_resp();
    auto *save_context = static_cast<SaveFileContext *>(pwrite_task->user_data);
    resp->headers["Access-Control-Allow-Origin"] = "*";
    if (pwrite_task->get_state() != WFT_STATE_SUCCESS || ret < 0)
    {
        resp->Error(StatusFileWriteError);
    } 
    else
    {
        if(save_context->notify_msg.empty()) 
        {
            resp->append_output_body_nocopy("Upload file success\n", 18);
        } 
        else 
        {
            resp->headers["Content-Type"] = ContentType::to_str(APPLICATION_JSON);
            resp->append_output_body_nocopy(save_context->notify_msg.c_str(), save_context->notify_msg.size());
        }
    }
}

} // namespace grpc

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
        StringPiece body_piece(this->body());
        _req_data->form = _multi_part.parse_multipart(body_piece);
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

        StringPiece boundary_str = StringPiece::trim_pairs(boundary_piece, R"(""'')");
        _multi_part.set_boundary(boundary_str.as_string());
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
    _multi_part(std::move(_http_request._multi_part)),
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

    _headers = std::move(other._headers);
    _route_match_path = std::move(other._route_match_path);
    _route_full_path = std::move(other._route_full_path);
    _route_params = std::move(other._route_params);
    _query_params = std::move(other._query_params);
    // _cookies = std::move(other._cookies);
    _multi_part = std::move(other._multi_part);
    _parsed_uri = std::move(other._parsed_uri);

    return *this;
}

// GRPCResp
void GRPCResp::String(const std::string &str)
{
    auto *compress_data = malloc(str.size());
    size_t compress_size = 0;
    int ret = this->compress(&str, compress_data, compress_size);
    if(ret != StatusOK)   
    {
        this->append_output_body(static_cast<const void *>(str.c_str()), str.size());
    } 
    else 
    {
        this->append_output_body_nocopy(compress_data, compress_size);
    }
    this->resp_code = 0;
    this->resp_msg = "Success";
    task_of(this)->add_callback([compress_data](GRPCTask *) { free(compress_data); });
}

void GRPCResp::String(std::string &&str)
{
    auto *compress_data = malloc(str.size());
    size_t compress_size = 0;
    int ret = this->compress(&str, compress_data, compress_size);
    if(ret == StatusOK)
    {   
        this->append_output_body_nocopy(compress_data, compress_size);
    }
    else
    {
        this->append_output_body(static_cast<const void *>(str.c_str()), str.size());
    }
    // this->resp_code = 0;
    // this->resp_msg = "Success";
    task_of(this)->add_callback([compress_data](GRPCTask *) { free(compress_data); });
}

void GRPCResp::File(const std::string &path)
{
    int ret = GRPCUtil::send_file(path, 0, -1, this);
    if(ret != StatusOK)
    {
        this->Error(ret);
    }
}


void GRPCResp::Save(const std::string &file_dst, const std::string &content, const std::string &notify_msg)
{
    GRPCUtil::saveFile(file_dst, content, this, notify_msg);
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

// void GRPCResp::Gzip(const ::Json &json)
// {
//     this->headers["Content-Type"] = ContentType::to_str(APPLICATION_JSON);
//     this->headers["Content-Encoding"] = "gzip";
//     rapidjson::StringBuffer resBuffer;
//     rapidjson::Writer<rapidjson::StringBuffer> resWriter(resBuffer);
//     json.Accept(resWriter);
//     if (json.HasMember("StatusCode") && json["StatusCode"].IsInt())
//     {
//         this->resp_code = json["StatusCode"].GetInt();
//     }
//     if (json.HasMember("StatusMsg") && json["StatusMsg"].IsString())
//     {
//         this->resp_msg = json["StatusMsg"].GetString();
//     }
//     std::string data = resBuffer.GetString();
//     void* compress_ = malloc(data.size());
//     if (compress_ == nullptr)
//     {
//         std::cout<<"malloc failed Cache Not Enough:"<<std::endl;
//         return;
//     }
//     int compress_size = 0;
//     int status = CompressUtil::GzipHelper::compress(&data, compress_, compress_size);
//     if (status != 0)
//     {
//         std::cout<<"Gzip StatusMsg Error Code:"<<status<<std::endl;
//         free(compress_);
//         return;
//     }
//     this->append_output_body_nocopy(compress_, compress_size);
//     task_of(this)->add_callback([compress_](GRPCTask *) { free(compress_); });
// }

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

int GRPCResp::compress(const std::string * const data, void *compress_data, size_t &compress_size)
{
    if (headers.find("Content-Encoding") != headers.end())
    {
        if (headers["Content-Encoding"].find("gzip") != std::string::npos)
        {
            if (compress_data != nullptr)
            {
                int rt = CompressUtil::GzipHelper::compress(data, compress_data, compress_size);
                if (rt == 0)
                {
                    return StatusOK;
                }
                else
                {
                    return StatusCompressError;
                }
            }
            else
            {
                return -1;
            }
        }
        else
        {
            return StatusCompressNotSupport;
        }
    }
    return StatusNoComrpess;
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

// GRPCUtil
int GRPCUtil::fileSize(const std::string &filepath, size_t *size)
{
    // https://linux.die.net/man/2/stat
    struct stat st;
    memset(&st, 0, sizeof st);
    int ret = stat(filepath.c_str(), &st);
    int status = StatusOK;
    if (ret == -1)
    {
        *size = 0;
        status = StatusNotFound;
    }
    else
    {
        *size = st.st_size;
    }
    return status;
}

/**
 * get file suffix
 * path = /usr/local/upload/test.nt
 * suffix = nt
 * 
 * @param filepath 
 * @return std::string 
 */
std::string GRPCUtil::fileSuffix(const std::string &filepath)
{
    std::string::size_type pos1 = filepath.find_last_of("/");
    if (pos1 == std::string::npos)
    {
        pos1 = 0;
    }
    else
    {
        pos1++;
    }
    std::string file = filepath.substr(pos1, -1);

    std::string::size_type pos2 = file.find_last_of(".");
    if (pos2 == std::string::npos)
    {
        return "";
    }
    return file.substr(pos2 + 1, -1);
}

/**
 * get file name include suffix
 * path = /usr/local/upload/test.nt
 * name = test.nt
 * 
 * @param filepath 
 * @return std::string 
 */
std::string GRPCUtil::fileName(const std::string &filepath)
{
    std::string::size_type pos1 = filepath.find_last_not_of("/");
    if (pos1 == std::string::npos)
    {
        return "/";
    }
    std::string::size_type pos2 = filepath.find_last_of("/", pos1);
    if (pos2 == std::string::npos)
    {
        pos2 = 0;
    } else
    {
        pos2++;
    }

    return filepath.substr(pos2, pos1 - pos2 + 1);
}

bool GRPCUtil::isFile(const std::string &path)
{
    struct stat st;
    return stat(path.c_str(), &st) >= 0 && S_ISREG(st.st_mode);
}

bool GRPCUtil::isDir(const std::string &path)
{
    struct stat st;
    return stat(path.c_str(), &st) >= 0 && S_ISDIR(st.st_mode);
}

int GRPCUtil::send_file(const std::string &path, size_t file_start, size_t file_end, GRPCResp *resp)
{
    if(!GRPCUtil::isFile(path))
    {
        return StatusNotFound;
    }
    int start = file_start;
    int end = file_end;
    if (end == -1 || start < 0)
    {
        size_t file_size;
        int ret = GRPCUtil::fileSize(path, &file_size);

        if (ret != StatusOK)
        {
            return ret;
        }
        if (end == -1) end = file_size;
        if (start < 0) start = file_size + start;
    }

    if (end <= start)
    {
        return StatusFileRangeInvalid;
    }

    content_type content_type = CONTENT_TYPE_NONE;
    std::string suffix = GRPCUtil::fileSuffix(path);
    if(!suffix.empty())
    {
        content_type = ContentType::to_enum_by_suffix(suffix);
    }
    if (content_type == CONTENT_TYPE_NONE || content_type == CONTENT_TYPE_UNDEFINED) {
        content_type = APPLICATION_OCTET_STREAM;
    }
    resp->headers["Content-Type"] = ContentType::to_str(content_type);

    size_t size = end - start;
    void *buf = malloc(size);

    GRPCServerTask *server_task = task_of(resp);
    server_task->add_callback([buf](GRPCTask *server_task)
                              {
                                  free(buf);
                              });
    // https://datatracker.ietf.org/doc/html/rfc7233#section-4.2
    // Content-Range: bytes 42-1233/1234
    resp->headers["Content-Range"] = "bytes " + std::to_string(start)
                                            + "-" + std::to_string(end)
                                            + "/" + std::to_string(size);

    WFFileIOTask *pread_task = WFTaskFactory::create_pread_task(path,
                                                                buf,
                                                                size,
                                                                static_cast<off_t>(start),
                                                                pread_callback);
    pread_task->user_data = resp;  
    **server_task << pread_task;
    return StatusOK;
}

void GRPCUtil::saveFile(const std::string &dst_path, const std::string &content, GRPCResp *resp, const std::string &notify_msg) 
{
    GRPCServerTask *server_task = task_of(resp);

    auto *save_context = new SaveFileContext; 
    save_context->content = content;    // copy
    save_context->notify_msg = notify_msg;  // copy

    WFFileIOTask *pwrite_task = WFTaskFactory::create_pwrite_task(dst_path,
                                                                  static_cast<const void *>(save_context->content.c_str()),
                                                                  save_context->content.size(),
                                                                  0,
                                                                  pwrite_callback);
    **server_task << pwrite_task;
    server_task->add_callback([save_context](GRPCTask *) {
        delete save_context;
    });
    pwrite_task->user_data = save_context;
}