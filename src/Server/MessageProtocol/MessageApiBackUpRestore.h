#pragma once
#include "MessageApi.h"

namespace server
{
    // backup
    struct MessageBackupRequest : public MessageRequest
    {
        std::string db_name;
        std::string backup_path;
        bool async;
        std::string callback;
        bool backup_zip;
        MessageBackupRequest()=delete;
        MessageBackupRequest(const rapidjson::Document& json_data);
    };

    struct MessageBackupResponse : public MessageResponse
    {
        std::string backupfilepath;
        std::string opt_id;
        void toJsonString(std::string& json_str) override;
    };

    //backup path
    struct MessageBackupPathRequest : public MessageRequest
    {
        std::string db_name;
        MessageBackupPathRequest()=delete;
        MessageBackupPathRequest(const rapidjson::Document& json_data);
    };

    struct MessageBackupPathResponse : public MessageResponse
    {
        std::vector<std::string> paths;
        void toJsonString(std::string& json_str) override;
    };

    // restore
    struct MessageRestoreRequest : public MessageRequest
    {
        std::string db_name;
        std::string backup_path;
        bool async;
        std::string callback;
        bool backup_zip;
        MessageRestoreRequest()=delete;
        MessageRestoreRequest(const rapidjson::Document& json_data);
    };

    struct MessageRestoreResponse : public MessageResponse
    {
        std::string opt_id;
        void toJsonString(std::string& json_str) override;
    };
}