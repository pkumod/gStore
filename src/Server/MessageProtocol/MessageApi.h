#pragma once
#include <string>
#include "../ServerStatusCode.h"
#include "../ApiTypedef.h"
#include "../../Util/JsonUtil.h"

namespace server
{
    struct MessageRequest
    {
        std::string op;
        std::string username;
        std::string password;
        std::string remote_ip;
        MessageRequest() {}
        MessageRequest(const MessageRequest &other);
        MessageRequest &operator=(const MessageRequest &other);
        MessageRequest(std::string op) : op(op) {}
        MessageRequest(std::string op, std::string username, std::string password) : op(op), username(username), password(password) {}
        MessageRequest(const nlohmann::json& json_data);
        void init(std::string username, std::string password);
        void toJson(nlohmann::json& json);
        virtual void to_json(std::string& json_str){};
        virtual void to_inner_json(std::string& json_str);
    };

    struct MessageResponse
    {
        int StatusCode;
        std::string StatusMsg;
        std::string body;
        nlohmann::json json;
        MessageResponse();
        MessageResponse(int StatusCode, std::string StatusMsg) : StatusCode(StatusCode), StatusMsg(StatusMsg) {}
        MessageResponse(const std::string& body);
        int getStatusCode() { return StatusCode; }
        std::string getStatusMsg() { return StatusMsg; }
        bool success() { return StatusCode == 0; }
        virtual void toJson(nlohmann::json& json)
        {
            json["StatusCode"]  = StatusCode;
            json["StatusMsg"]   = StatusMsg;
        };
        virtual void toJsonString(std::string& json_str);
        void Error(int code, const std::string& msg){StatusCode = code; StatusMsg = msg;};
    };

    // shutdown
    struct MessageShutdownRequest : public MessageRequest
    {
        MessageShutdownRequest() : MessageRequest(std::string("shutdown")) {}
        MessageShutdownRequest(std::string username, std::string password) : MessageRequest("shutdown", username, password) {}
        void to_json (std::string& json_str) override;
    };

    struct MessageShutdownResponse : public MessageResponse
    {
        MessageShutdownResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageShutdownResponse(std::string body) : MessageResponse(body) {}
    };

    // test connect
    struct MessageTestConnectionRequest : public MessageRequest
    {
        MessageTestConnectionRequest() : MessageRequest(std::string("testConnect")) {}
        MessageTestConnectionRequest(std::string username, std::string password) : MessageRequest("testConnect", username, password) {}
        void to_json (std::string& json_str) override;
    };

    struct MessageTestConnectionResponse : public MessageResponse
    {
        MessageTestConnectionResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageTestConnectionResponse(std::string body) : MessageResponse(body) {
            // parse other fields
        }
    };

    // check server
    struct MessageCheckRequest : public MessageRequest
    {
        MessageCheckRequest() : MessageRequest(std::string("check")) {}
        std::string to_params(){return "operation="+op;}
        void to_json(std::string& json_str) override;
    };

    struct MessageCheckResponse : public MessageResponse
    {
        MessageCheckResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageCheckResponse(std::string body) : MessageResponse(body) {}
    };

    // load db
    struct MessageLoadRequest : public MessageRequest
    {
        std::string db_name;
        std::string csr;
        bool async;
        bool txn;
        MessageLoadRequest()=delete;
        MessageLoadRequest(const nlohmann::json& json_data);
        MessageLoadRequest(std::string db_name, std::string csr = "0", bool async = false, bool txn = true);
        MessageLoadRequest(std::string username, std::string password, std::string db_name, std::string csr = "0", bool async = false, bool txn = true);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
        bool Csr()const;
    };

    struct MessageLoadResponse : public MessageResponse
    {
        std::string csr;
        std::string opt_id;
        MessageLoadResponse();
        void toJsonString(std::string& json_str);
        MessageLoadResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageLoadResponse(std::string body) : MessageResponse(body) {}
    };

    // login
    struct MessageLoginRequest : public MessageRequest
    {
        MessageLoginRequest() : MessageRequest(std::string("login")) {}
        MessageLoginRequest(std::string username, std::string password) : MessageRequest("login",username,password) {}
        void to_json(std::string& json_str) override;
    };

    // refresh configure
    struct MessageRefreshconfRequest : public MessageRequest
    {
        MessageRefreshconfRequest() : MessageRequest(std::string("refreshconf")) {}
        MessageRefreshconfRequest(std::string username, std::string password) : MessageRequest("refreshconf",username,password) {}
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    // init data
    struct MessageInitRequest : public MessageRequest
    {
        std::string db_names;
        MessageInitRequest(std::string db_names);
        MessageInitRequest(std::string username, std::string password, std::string db_names);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageInitData
    {
        std::string db_name;
        std::string status;
        std::string msg;
        MessageInitData(std::string _db_name, std::string _status, std::string _msg);
    };

    struct MessageInitResponse : public MessageResponse {
        std::vector<struct MessageInitData> data;
        MessageInitResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageInitResponse(std::string body);
    };

    // show dbs
    struct MessageShowRequest : public MessageRequest {
        MessageShowRequest() : MessageRequest(std::string("show")) {};
        MessageShowRequest(std::string username, std::string password) : MessageRequest("show",username,password) {}
        void to_json(std::string& json_str) override;
    };

    struct MessageShowResponseBody
    {
        std::string database;
        std::string creator;
        std::string builtTime;
        std::string status;
        MessageShowResponseBody(std::string _database, std::string _creator, std::string _builtTime, std::string _status);
    };

    struct MessageShowResponse : public MessageResponse
    {
        std::vector<struct MessageShowResponseBody> responseBody;
        MessageShowResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageShowResponse(std::string body);
    };

    // unload db
    struct MessageUnloadRequest : public MessageRequest
    {
        std::string db_name;
        bool async;
        MessageUnloadRequest(const nlohmann::json& json_data);
        MessageUnloadRequest(std::string db_name, bool async = false);
        MessageUnloadRequest(std::string username, std::string password, std::string db_name, bool async = false);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageUnloadResponse : public MessageResponse
    {
        std::string opt_id;
        MessageUnloadResponse(): opt_id("") {};
        void toJsonString(std::string& json_str);
        MessageUnloadResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageUnloadResponse(std::string body) : MessageResponse(body) {}
    };

    // monitor
    struct MessageMonitorRequest : public MessageRequest
    {
        std::string db_name;
        std::string disk;
        MessageMonitorRequest(std::string db_name);
        MessageMonitorRequest(std::string username, std::string password, std::string db_name);
        MessageMonitorRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageMonitorResponse : public MessageResponse
    {
        std::string database;
        std::string creator;
        std::string builtTime;
        std::string tripleNum;
        uint64_t entityNum;
        uint64_t literalNum;
        uint64_t subjectNum;
        uint64_t predicateNum;
        uint32_t connectionNum;
        uint64_t diskUsed;
        int64_t costTime;
        uint32_t lockNum;
        std::string status;
        std::unordered_map<std::string, unsigned long long> subjectList;
        nlohmann::json schema;
        MessageMonitorResponse();
        MessageMonitorResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageMonitorResponse(std::string body);
        void toJsonString(std::string& json_str);
    };

    struct MessageLicenseResponse: public MessageResponse
    {
        bool isvalid;
        std::string product;
        std::string version;
        std::string cpu;
        std::string mac;
        std::string startdate;
        std::string enddate;
        std::string company;
        std::string type;
        std::string desc;
        MessageLicenseResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageLicenseResponse(std::string body);
        void toJson(nlohmann::json& json_data);
        void toJsonString(std::string& json_str);
    };
}