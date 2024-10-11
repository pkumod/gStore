#pragma once
#include <string>
#include "ServerStatusCode.h"
#include "../GRPC/grpc_server.h"
#include "../Api/NlohmanJson.hpp"
#include "ApiTypedef.h"

namespace server
{
    struct MessageRequest
    {
        std::string operation;
        std::string username;
        std::string encryption;
        std::string password;
    };

    struct MessageResponse
    {
        StatusCode status_code;
        std::string status_msg;
        MessageResponse();
        void toJson(nlohmann::json& json);
    };

    // load
    struct MessageLoadRequest : public MessageRequest
    {
        std::string db_name;
        bool csr;
        MessageLoadRequest(){}
        MessageLoadRequest(const rapidjson::Document& json_data);
    };

    struct MessageLoadResponse : public MessageResponse
    {
        std::string csr;
        MessageLoadResponse();
        void toJsonString(std::string& json_str);
    };





    // batch insert
    struct MessageBatchInsertRequest : public MessageRequest
    {
        std::string db_name;
        std::string file;
        std::string dir;
        bool async;
        std::string callback;
        MessageBatchInsertRequest(){}
        MessageBatchInsertRequest(const rapidjson::Document& json_data);
    };

    struct MessageBatchInsertResponse : public MessageResponse
    {
        int success_num;
        int failed_num;
        std::string opt_id;
        MessageBatchInsertResponse();
        void toJsonString(std::string& json_str);
    };
}