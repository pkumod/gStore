#include "MessageApiBackUpRestore.h"
#include "../ApiProvider.h"

namespace server
{
    MessageBackupRequest::MessageBackupRequest(std::string db_name, std::string backup_path, bool async, std::string callback, bool backup_zip) : MessageRequest(std::string("backup"))
    {
        this->db_name = db_name;
        this->backup_path = backup_path;
        this->async = async;
        this->callback = callback;
        this->backup_zip = backup_zip;
    }

    MessageBackupRequest::MessageBackupRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->db_name     = jsonParam(json_data, "db_name");
		this->backup_path = jsonParam(json_data, "backup_path");
        this->backup_zip  = jsonBoolParam(json_data, "backup_zip", false);
        this->async = jsonBoolParam(json_data, "async", false);
        this->callback = jsonParam(json_data, "callback");
    }

    void MessageBackupRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"backup_path", this->backup_path},
            {"backup_zip", "false"},
            {"async", false},
            {"callback", this->callback}};
        json_str = json.dump();
    }

    void MessageBackupRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", ""},
            {"db_name", this->db_name},
            {"backup_path", this->backup_path},
            {"backup_zip", "false"},
            {"async", false},
            {"callback", this->callback},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageBackupResponse::MessageBackupResponse()
    {
        this->backupfilepath = "";
        this-> opt_id = "";
    }

    MessageBackupResponse::MessageBackupResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("backupfilepath"))
                json.at("backupfilepath").get_to(this->backupfilepath);
            if (json.contains("opt_id"))
                json.at("opt_id").get_to(this->opt_id);
        }
    }

    void MessageBackupResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["backupfilepath"] = this->backupfilepath;
        rJson["opt_id"] = this->opt_id;
        json_str = rJson.dump();
    }

    // backup path
    MessageBackupPathRequest::MessageBackupPathRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->db_name     = jsonParam(json_data, "db_name");
    }

    void MessageBackupPathResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["paths"] = nlohmann::json::array();
        if (!this->paths.empty())
        {
            rJson["paths"] = this->paths;
        }
        json_str = rJson.dump();
    }

    // restore
    MessageRestoreRequest::MessageRestoreRequest(std::string db_name, std::string backup_path, bool async, std::string callback, bool backup_zip) : MessageRequest(std::string("restore"))
    {
        this->db_name = db_name;
        this->backup_path = backup_path;
        this->async = async;
        this->callback = callback;
        this->backup_zip = backup_zip; 
    }

    MessageRestoreRequest::MessageRestoreRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->db_name     = jsonParam(json_data, "db_name");
		this->backup_path = jsonParam(json_data, "backup_path");
        this->async = jsonBoolParam(json_data, "async", false);
        this->callback = jsonParam(json_data, "callback");
    }

    void MessageRestoreRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"backup_path", this->backup_path},
            {"backup_zip", "false"},
            {"async", false},
            {"callback", this->callback}};
        json_str = json.dump();
    }

    void MessageRestoreRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", ""},
            {"db_name", this->db_name},
            {"backup_path", this->backup_path},
            {"backup_zip", "false"},
            {"async", false},
            {"callback", this->callback},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageRestoreResponse::MessageRestoreResponse()
    {
        this-> opt_id = "";
    }

    MessageRestoreResponse::MessageRestoreResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("opt_id"))
                json.at("opt_id").get_to(this->opt_id);
        }
    }

    void MessageRestoreResponse::MessageRestoreResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["opt_id"] = this->opt_id;
        json_str = rJson.dump();
    }


    MessageExportRequest::MessageExportRequest(std::string db_name, std::string db_path, bool compress) : MessageRequest(std::string("export"))
    {
        this->db_name = db_name;
        this->db_path = db_path;
        this->compress = compress;
    }

    MessageExportRequest::MessageExportRequest(const rapidjson::Document& json_data)
    {
        this->db_name     = jsonParam(json_data, "db_name");
		this->db_path = jsonParam(json_data, "db_path");
        this->compress  = jsonBoolParam(json_data, "compress", false);
    }

    void MessageExportRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"db_path", this->db_path},
            {"compress", "false"}};
        json_str = json.dump();
    }

    void MessageExportRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name},
            {"db_path", this->db_path},
            {"compress", "false"},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageExportResponse::MessageExportResponse()
    {
        this->filepath = "";
    }

    MessageExportResponse::MessageExportResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("filepath"))
                json.at("filepath").get_to(this->filepath);
        }
    }

    void MessageExportResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["filepath"] = this->filepath;
        json_str = rJson.dump();   
    }
}