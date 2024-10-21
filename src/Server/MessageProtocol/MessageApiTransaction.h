#pragma once
#include "MessageApi.h"

namespace server
{
    // begin
    struct MessageBeginRequest : public MessageRequest
    {
        std::string db_name;
        std::string isolevel;
        MessageBeginRequest(const rapidjson::Document& json_data);
    };

    struct MessageBeginResponse : public MessageResponse
    {
        std::string TID;
        void toJsonString(std::string& json_str) override;
    };

    // tquery
    struct MessageTqueryRequest : public MessageRequest
    {
        std::string db_name;
        std::string tid;
        std::string sparql;
        MessageTqueryRequest(const rapidjson::Document& json_data);
    };

    struct MessageTqueryResponse : public MessageResponse
    {
        std::string ansNum;
        std::string result;
        nlohmann::json query_json;
        void toJsonString(std::string& json_str) override;
    };

    // commit
    struct MessageCommitRequest : public MessageRequest
    {
        std::string db_name;
        std::string tid;
        MessageCommitRequest(const rapidjson::Document& json_data);
    };

    struct MessageCommitResponse : public MessageResponse
    {
        // todo
    };

    // rollback
    struct MessageRollbackRequest : public MessageRequest
    {
        std::string db_name;
        std::string tid;
        MessageRollbackRequest(const rapidjson::Document& json_data);
    };

    struct MessageRollbackResponse : public MessageResponse
    {
        //todo
    };
}