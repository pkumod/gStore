#pragma once
#include "MessageApi.h"

namespace gs
{
    // begin
    struct MessageBeginRequest : public MessageRequest
    {
        std::string db_name;
        std::string isolevel;
        MessageBeginRequest(const nlohmann::json& json_data);



        // gconsole use
        MessageBeginRequest(const std::string& db_name, const std::string& isolevel) : MessageRequest(std::string("begin")), db_name(db_name), isolevel(isolevel) { }
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);

    };

    struct MessageBeginResponse : public MessageResponse
    {
        std::string TID;

        MessageBeginResponse() { }
        void toJsonString(std::string& json_str) override;
        
        // gconsole use
        MessageBeginResponse(int code, std::string msg) : MessageResponse(code, msg) { }
        MessageBeginResponse(const std::string& body);
    };

    // tquery
    struct MessageTqueryRequest : public MessageRequest
    {
        std::string db_name;
        std::string tid;
        std::string sparql;
        MessageTqueryRequest(const nlohmann::json& json_data);
        
        // gconsole use
        MessageTqueryRequest(const std::string db_name, const std::string tid, const std::string sparql) : MessageRequest(std::string("tquery")), db_name(db_name), tid(tid), sparql(sparql) { };        
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
    };

    struct MessageTqueryResponse : public MessageResponse
    {
        int ansNum;
        std::string result;
        nlohmann::json query_json;
        MessageTqueryResponse(){ansNum = 0;}
        void toJsonString(std::string& json_str) override;

        //gconsole use
        MessageTqueryResponse(int code, std::string msg) : MessageResponse(code, msg) { }
        MessageTqueryResponse(const std::string& body);
    };

    // commit
    struct MessageCommitRequest : public MessageRequest
    {
        std::string db_name;
        std::string tid;
        MessageCommitRequest(const nlohmann::json& json_data);
        
        // gconsole use
        MessageCommitRequest(const std::string db_name, const std::string tid) : MessageRequest(std::string("commit")), db_name(db_name), tid(tid) { };        
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
    };

    struct MessageCommitResponse : public MessageResponse
    {
        MessageCommitResponse(int code, std::string msg) : MessageResponse(code, msg) { }
        MessageCommitResponse(const std::string& body) : MessageResponse(body) { }
        void toJsonString(std::string& json_str) override;
        // todo
    };

    // rollback
    struct MessageRollbackRequest : public MessageRequest
    {
        std::string db_name;
        std::string tid;
        MessageRollbackRequest(const nlohmann::json& json_data);

        // gconsole use
        MessageRollbackRequest(const std::string db_name, const std::string tid) : MessageRequest(std::string("rollback")), db_name(db_name), tid(tid) { };        
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
    };

    struct MessageRollbackResponse : public MessageResponse
    {
        MessageRollbackResponse(int code, std::string msg) : MessageResponse(code, msg) { }
        MessageRollbackResponse(const std::string& body) : MessageResponse(body) { }
        void toJsonString(std::string& json_str) override;
        //todo
    };
}