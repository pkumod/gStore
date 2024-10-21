#pragma once
#include "MessageApi.h"

namespace server
{
    // txn log
    struct MessageTxnLogRequest : public MessageRequest
    {
        int pageNo;
        int pageSize;
        MessageTxnLogRequest(const rapidjson::Document& json_data);
    };

    struct MessageTxnLogResponse : public MessageResponse
    {
        int totalSize;
        int totalPage;
        int pageNo;
        int pageSize;
        nlohmann::json list;
        MessageTxnLogResponse();
        void toJsonString(std::string& json_str) override;
    };

    // query log
    struct MessageQueryLogRequest : public MessageRequest
    {
        std::string date;
        int pageNo;
        int pageSize;
        MessageQueryLogRequest(const rapidjson::Document& json_data);
    };

    struct MessageQueryLogResponse : public MessageResponse
    {
        int totalSize;
        int totalPage;
        int pageNo;
        int pageSize;
        nlohmann::json list;
        MessageQueryLogResponse();
        void toJsonString(std::string& json_str) override;
    };

    // query log date
    struct MessageQueryLogDateRequest : public MessageRequest
    {
    };

    struct MessageQueryLogDateResponse : public MessageResponse
    {
        std::vector<std::string> list;
        void toJsonString(std::string& json_str) override;
    };

    // access log
    struct MessageAccessLogRequest : public MessageRequest
    {
        std::string date;
        int pageNo;
        int pageSize;
        MessageAccessLogRequest(const rapidjson::Document& json_data);
    };

    struct MessageAccessLogResponse : public MessageResponse
    {
        int totalSize;
        int totalPage;
        int pageNo;
        int pageSize;
        nlohmann::json list;
        MessageAccessLogResponse();
        void toJsonString(std::string& json_str) override;
    };

    // access log date
    struct MessageAccessLogDateRequest : public MessageRequest
    {
    };

    struct MessageAccessLogDateResponse : public MessageResponse
    {
        std::vector<std::string> list;
        void toJsonString(std::string& json_str) override;
    };

    // checkOperationState
    struct MessageCheckOperationStateRequest : public MessageRequest
    {
        std::string opt_id;
        MessageCheckOperationStateRequest(const rapidjson::Document& json_data);
    };

    struct MessageCheckOperationStateResponse : public MessageResponse
    {
        int state;
        int success_num;
        int failed_num;
        std::string backupfilepath;
        void toJsonString(std::string& json_str) override;
    };
}