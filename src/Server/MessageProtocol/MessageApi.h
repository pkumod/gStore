#pragma once
#include <string>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "../ServerStatusCode.h"
#include "../../Api/NlohmanJson.hpp"
#include "../ApiTypedef.h"

namespace server
{
    struct MessageRequest
    {
        std::string op;
        std::string username;
        std::string password;
        MessageRequest(){}
        MessageRequest(std::string op) : op(op) {}
        MessageRequest(std::string op, std::string username, std::string password) : op(op), username(username), password(password) {}
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
        void toJson(nlohmann::json& json);
    };

    // shutdown
    struct MessageShutdownRequest : public MessageRequest
    {
        MessageShutdownRequest() : MessageRequest("shutdown") {}
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
        MessageTestConnectionRequest() : MessageRequest("testConnect") {}
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
        MessageCheckRequest() : MessageRequest("check") {}
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
        MessageLoadRequest()=delete;
        MessageLoadRequest(const rapidjson::Document& json_data);
        MessageLoadRequest(std::string db_name, std::string csr);
        MessageLoadRequest(std::string username, std::string password, std::string db_name, std::string csr);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
        bool Csr()const;
    };

    struct MessageLoadResponse : public MessageResponse
    {
        std::string csr;
        MessageLoadResponse();
        void toJsonString(std::string& json_str);
        MessageLoadResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageLoadResponse(std::string body) : MessageResponse(body) {}
    };

    // login
    struct MessageLoginRequest : public MessageRequest
    {
        MessageLoginRequest() : MessageRequest("login") {}
        MessageLoginRequest(std::string username, std::string password) : MessageRequest("login",username,password) {}
        void to_json(std::string& json_str) override;
    };

    // refresh configure
    struct MessageRefreshconfRequest : public MessageRequest
    {
        MessageRefreshconfRequest() : MessageRequest("refreshconf") {}
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
        MessageShowRequest() : MessageRequest("show") {};
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
        MessageUnloadRequest(std::string db_name);
        MessageUnloadRequest(std::string username, std::string password, std::string db_name);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    // mpnitor
    struct MessageMonitorRequest : public MessageRequest
    {
        std::string db_name;
        MessageMonitorRequest(std::string db_name);
        MessageMonitorRequest(std::string username, std::string password, std::string db_name);
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
        MessageMonitorResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageMonitorResponse(std::string body);
    };
}