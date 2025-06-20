#pragma once
#include "MessageApi.h"

namespace gs
{
    // backup
    struct MessageBackupRequest : public MessageRequest
    {
        std::string db_name;
        std::string backup_path;
        bool async;
        std::string callback;
        bool backup_zip;
        MessageBackupRequest(std::string db_name, std::string backup_path, bool async, std::string callback, bool backup_zip);
        MessageBackupRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageBackupResponse : public MessageResponse
    {
        std::string backupfilepath;
        std::string opt_id;
        MessageBackupResponse();
        MessageBackupResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageBackupResponse(std::string body);
        void toJsonString(std::string& json_str) override;
    };

    //backup path
    struct MessageBackupPathRequest : public MessageRequest
    {
        std::string db_name;
        MessageBackupPathRequest()=delete;
        MessageBackupPathRequest(const nlohmann::json& json_data);
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
        MessageRestoreRequest(std::string db_name, std::string backup_path, bool async, std::string callback, bool backup_zip);
        MessageRestoreRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageRestoreResponse : public MessageResponse
    {
        std::string opt_id;
        MessageRestoreResponse();
        MessageRestoreResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageRestoreResponse(std::string body);
        void toJsonString(std::string& json_str) override;
    };

    struct MessageExportRequest : public MessageRequest
    {
        std::string db_name;
        std::string db_path;
        bool compress;
        MessageExportRequest(std::string db_name, std::string db_path, bool compress);
        MessageExportRequest(const nlohmann::json& json_data);
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageExportResponse : public MessageResponse
    {
        std::string filepath;
        MessageExportResponse();
        MessageExportResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageExportResponse(std::string body);
        void toJsonString(std::string& json_str) override;
    };

}