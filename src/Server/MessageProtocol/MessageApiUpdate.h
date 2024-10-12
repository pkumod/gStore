#pragma once
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "../ServerStatusCode.h"
#include "../../Api/NlohmanJson.hpp"
#include "../ApiTypedef.h"
#include "MessageApi.h"

namespace server
{
    // build db
    struct MessageBuildRequest : public MessageRequest
    {
        std::string db_name;
        std::string db_path;
        MessageBuildRequest(std::string db_name, std::string db_path);
        MessageBuildRequest(std::string username, std::string password, std::string db_name, std::string db_path);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageBuildResponse : public MessageResponse
    {
        uint64_t failed_num;
        MessageBuildResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageBuildResponse(std::string body);
    };

    // drop db
    struct MessageDropRequest : public MessageRequest {
        std::string db_name;
        std::string is_backup;
        MessageDropRequest(std::string db_name, std::string is_backup);
        MessageDropRequest(std::string username, std::string password, std::string db_name, std::string is_backup);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    // query
    struct MessageQueryRequest : public MessageRequest
    {
        std::string db_name;
        std::string sparql;
        std::string format;
        MessageQueryRequest(std::string db_name,  std::string sparql, std::string format = "json");
        MessageQueryRequest(std::string username, std::string password, std::string db_name, std::string sparql, std::string format = "json");
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageQueryResponse : public MessageResponse
    {
        std::vector<std::string> head;
        std::vector<std::vector<std::string>> results;
        uint64_t ansNum;
        uint64_t outputLimit;
        std::string queryTime;
        std::string threadId;
        MessageQueryResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageQueryResponse(std::string body);
    };

    // batch insert
    struct MessageBatchInsertRequest : public MessageRequest
    {
        std::string db_name;
        std::string file;
        std::string dir;
        bool async;
        std::string callback;
        MessageBatchInsertRequest()=delete;
        MessageBatchInsertRequest(const rapidjson::Document& json_data);
        MessageBatchInsertRequest(std::string db_name, std::string file, std::string dir);
        MessageBatchInsertRequest(std::string username, std::string password,std::string db_name, std::string file, std::string dir);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageBatchInsertResponse : public MessageResponse
    {
        uint32_t successNum;
        uint32_t failedNum;
        std::string opt_id;
        MessageBatchInsertResponse();
        MessageBatchInsertResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageBatchInsertResponse(std::string body);
        void toJsonString(std::string& json_str);
    };

    // batch remove
    struct MessageBatchRemoveRequest : public MessageRequest
    {
        std::string db_name;
        std::string file;
        MessageBatchRemoveRequest(std::string db_name,  std::string file);
        MessageBatchRemoveRequest(std::string username, std::string password,std::string db_name, std::string file);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageBatchRemoveResponse : public MessageResponse {
        uint32_t successNum;
        uint32_t failedNum;
        MessageBatchRemoveResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageBatchRemoveResponse(std::string body);
    };
}