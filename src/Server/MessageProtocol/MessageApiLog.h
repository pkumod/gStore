#pragma once
#include "MessageApi.h"

namespace server
{

    struct TxnLog {
        std::string db_name;
        std::string TID;
        std::string user;
        std::string state;
        std::string begin_time;
        std::string end_time;
        void to_json(std::string& json_str);
        void from_json(const nlohmann::json& json);
    };

    // txn log
    struct MessageTxnLogRequest : public MessageRequest
    {
        int pageNo;
        int pageSize;
        MessageTxnLogRequest(const nlohmann::json& json_data);
        
        // gconsole use
        MessageTxnLogRequest(int pageNo, int pageSize) : MessageRequest(std::string("txnlog")), pageNo(pageNo), pageSize(pageSize) { }

        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
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

        // gconsole use
        MessageTxnLogResponse(const std::string& body);
        MessageTxnLogResponse(int code, std::string msg) : MessageResponse(code, msg) { }
    };

    // query log

    struct QueryLog {
        std::string QueryDateTime;
        std::string Sparql;
        std::string Format;
        std::string RemoteIP;
        std::string FileName;
        int QueryTime;
        int AnsNum;
        void to_json(std::string& json_str);
        void from_json(const nlohmann::json& json);
    };

    struct MessageQueryLogRequest : public MessageRequest
    {
        std::string date;
        int pageNo;
        int pageSize;
        std::string db_name;
        MessageQueryLogRequest(const nlohmann::json& json_data);

        // gconsole use;
        MessageQueryLogRequest(const std::string& date, int pageNo, int pageSize) : MessageRequest(std::string("querylog")), date(date), pageNo(pageNo), pageSize(pageSize) { }
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
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

        // gconsole use
        MessageQueryLogResponse(const std::string& body);
        MessageQueryLogResponse(int code, std::string msg) : MessageResponse(code, msg) { }
    };

    // query log date
    struct MessageQueryLogDateRequest : public MessageRequest
    {
        // gconsole use
        MessageQueryLogDateRequest() : MessageRequest(std::string("querylogdate")) { }
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
    };

    struct MessageQueryLogDateResponse : public MessageResponse
    {
        
        std::vector<std::string> list;
        void toJsonString(std::string& json_str) override;
    
        MessageQueryLogDateResponse() { }
        // gconsole use
        MessageQueryLogDateResponse(const std::string& body);
        MessageQueryLogDateResponse(int code, std::string msg) : MessageResponse(code, msg) { }
    };

    // access log

    struct AccessLog {
        std::string ip;
        std::string operation;
        std::string createtime;
        std::string code;
        std::string msg;
        void to_json(std::string& json_str);
        void from_json(const nlohmann::json& json);
    };

    struct MessageAccessLogRequest : public MessageRequest
    {
        std::string date;
        int pageNo;
        int pageSize;
        std::string db_name;
        std::string db_operation;
        MessageAccessLogRequest(const std::string date, int pageNo, int pageSize) : MessageRequest(std::string("accesslog")), date(date), pageNo(pageNo), pageSize(pageSize) { }
        MessageAccessLogRequest(const nlohmann::json& json_data);
        // gconsole use
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
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

        // gconsole use
        MessageAccessLogResponse(const std::string& body);
        MessageAccessLogResponse(int code, std::string msg) : MessageResponse(code, msg) { }
    };

    // access log date
    struct MessageAccessLogDateRequest : public MessageRequest
    {

        MessageAccessLogDateRequest() : MessageRequest(std::string("accesslogdate")) { }
        // gconsole use
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
        
    };

    struct MessageAccessLogDateResponse : public MessageResponse
    {
        std::vector<std::string> list;
        void toJsonString(std::string& json_str) override;
        MessageAccessLogDateResponse() { }
        // gconsole use
        MessageAccessLogDateResponse(const std::string& body);
        MessageAccessLogDateResponse(int code, std::string msg) : MessageResponse(code, msg) { }
    };

    // checkOperationState
    struct MessageCheckOperationStateRequest : public MessageRequest
    {
        std::string opt_id;
        MessageCheckOperationStateRequest(const nlohmann::json& json_data);
        // gconsole use
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
    };

    struct MessageCheckOperationStateResponse : public MessageResponse
    {
        std::string operation;
        int state;
        unsigned long long success_num;
        int failed_num;
        std::string filepath;
        void toJsonString(std::string& json_str) override;

        MessageCheckOperationStateResponse() { }
        // gconsole use
        MessageCheckOperationStateResponse(const std::string& body);
        MessageCheckOperationStateResponse(int code, std::string msg) : MessageResponse(code, msg) { }
    };
}