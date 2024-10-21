#include "MessageApiBackUpRestore.h"
#include "../ApiProvider.h"

namespace server
{
    MessageBackupRequest::MessageBackupRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->db_name     = jsonParam(json_data, "db_name");
		this->backup_path = jsonParam(json_data, "backup_path");
        this->backup_zip  = jsonBoolParam(json_data, "backup_zip", false);
        this->async = jsonBoolParam(json_data, "async", false);
        this->callback = jsonParam(json_data, "callback");
    }

    void MessageBackupResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["backupfilepath"] = this->backupfilepath;
        rJson["opt_id"] = this->opt_id;
        json_str = rJson.dump();
        // SLOG_TRACE("MessageBackupResponse:" << json_str);
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
    MessageRestoreRequest::MessageRestoreRequest(const rapidjson::Document& json_data) : MessageRequest(json_data)
    {
        this->db_name     = jsonParam(json_data, "db_name");
		this->backup_path = jsonParam(json_data, "backup_path");
        this->async = jsonBoolParam(json_data, "async", false);
        this->callback = jsonParam(json_data, "callback");
    }

    void MessageRestoreResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json rJson;
        toJson(rJson);
        rJson["opt_id"] = this->opt_id;
        json_str = rJson.dump();
    }
}