#pragma once
#include "MessageApi.h"

namespace server
{
    // build db
    struct MessageBuildRequest : public MessageRequest
    {
        std::string db_name;
        std::vector<std::string> db_path;
        bool remote; 
        bool async;
        std::string callback;
        MessageBuildRequest(std::string db_name, std::string db_path);
        MessageBuildRequest(std::string username, std::string password, std::string db_name, std::string db_path);
        MessageBuildRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageBuildResponse : public MessageResponse
    {
        uint64_t failed_num;
        uint32_t successNum;
        std::string opt_id;
        MessageBuildResponse();
        MessageBuildResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageBuildResponse(std::string body);
        void toJsonString(std::string& json_str);
    };

    // drop db
    struct MessageDropRequest : public MessageRequest {
        std::string db_name;
        bool is_backup;
        MessageDropRequest(std::string db_name, bool is_backup);
        MessageDropRequest(std::string username, std::string password, std::string db_name, bool is_backup);
        MessageDropRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageDropResponse : public MessageResponse
    {
        MessageDropResponse() : MessageResponse(){}
        MessageDropResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        void toJsonString(std::string& json_str);
    };

    // query
    struct MessageQueryRequest : public MessageRequest
    {
        std::string db_name;
        std::string sparql;
        std::string format;
        std::string callback;
        bool async;
        MessageQueryRequest(const MessageQueryRequest& other);
        MessageQueryRequest& operator=(const MessageQueryRequest& other);
        MessageQueryRequest(const nlohmann::json& json_data);
        MessageQueryRequest(std::string db_name,  std::string sparql, std::string format = "json", bool async = false);
        MessageQueryRequest(std::string username, std::string password, std::string db_name, std::string sparql, std::string format = "json", bool async = false);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageQueryResponse : public MessageResponse
    {
        std::vector<std::string> head;
        std::vector<std::vector<std::string>> results;
        uint64_t ansNum = 0;
        int outputLimit = -1;
        std::string queryTime;
        std::string threadId;
        bool isUpdate;
        std::string fileName;
        std::string opt_id;
        nlohmann::json query_json;
        bool is_pfn;
        MessageQueryResponse();
        MessageQueryResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageQueryResponse(std::string body);
        void toJson(nlohmann::json& json);
        void toJsonString(std::string& json_str);
        void toAsyncJsonString(std::string& json_str);
    };

    // batch insert
    struct MessageBatchInsertRequest : public MessageRequest
    {
        std::string db_name;
        std::vector<std::string> file;
        bool remote;
        bool async;
        std::string callback;
        MessageBatchInsertRequest()=delete;
        MessageBatchInsertRequest(const nlohmann::json& json_data);
        MessageBatchInsertRequest(std::string db_name, std::string file);
        MessageBatchInsertRequest(std::string username, std::string password,std::string db_name, std::string file);
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
        bool remote;
        bool async;
        std::string callback;
        MessageBatchRemoveRequest(const nlohmann::json& json_data);
        MessageBatchRemoveRequest(std::string db_name, std::string file);
        MessageBatchRemoveRequest(std::string username, std::string password,std::string db_name, std::string file);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageBatchRemoveResponse : public MessageResponse {
        uint32_t successNum;
        uint32_t failedNum;
        std::string opt_id;
        MessageBatchRemoveResponse();
        MessageBatchRemoveResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageBatchRemoveResponse(std::string body);
        void toJsonString(std::string& json_str);
    };

    // check point
    struct MessageCheckPointRequest : public MessageRequest
    {
        std::string db_name;
        MessageCheckPointRequest(const nlohmann::json& json_data);
        MessageCheckPointRequest(const std::string& db_name) : MessageRequest(std::string("checkpoint")), db_name(db_name) { }
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
    };

    struct MessageCheckPointResponse : public MessageResponse
    {
        MessageCheckPointResponse(int code, std::string msg) : MessageResponse(code, msg) { };
        MessageCheckPointResponse(const std::string& body) : MessageResponse(body) { }
        //todo
    };
}