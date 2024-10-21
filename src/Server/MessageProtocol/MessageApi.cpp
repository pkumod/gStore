#include "MessageApi.h"
#include "../ApiProvider.h"

namespace server
{
    // base request
    void MessageRequest::init(std::string username, std::string password)
    {
        this->username = username;
        this->password = password;
    }

    MessageRequest::MessageRequest(const rapidjson::Document& json_data)
    {
        this->username = jsonParam(json_data, "username");
        this->password = jsonParam(json_data, "password");
        this->op = jsonParam(json_data, "operation");
        this->remote_ip = jsonParam(json_data, "remote_ip");
    }

    MessageRequest::MessageRequest(const nlohmann::json& json_data)
    {
        this->username = jsonParam(json_data, "username");
        this->password = jsonParam(json_data, "password");
        this->op = jsonParam(json_data, "operation");
        this->remote_ip = jsonParam(json_data, "remote_ip");
    }

    void MessageRequest::toJson(nlohmann::json& json)
    {
        json["operation"] = this->op;
        json["username"]  = this->username;
        json["password"] = this->password;
    }

    void MessageRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{{"operation", this->op},{"username", "root"},{"password", ""},{"inner", "true"}};
        json_str = json.dump();
    }

    // base response
    MessageResponse::MessageResponse()
    {
        StatusCode = 0;
        StatusMsg  = "";
    }

    void MessageResponse::toJson(nlohmann::json& json)
    {
        json["StatusCode"]  = StatusCode;
        json["StatusMsg"]   = StatusMsg;
    }

    void MessageResponse::toJsonString(std::string& json_str)
    {
        this->json["StatusCode"]  = StatusCode;
        this->json["StatusMsg"]   = StatusMsg;
        json_str = this->json.dump();
    }

    MessageResponse::MessageResponse(const std::string& body) : body(body)
    {
        if (nlohmann::json::accept(body))
        {
            try
            {
                json = nlohmann::json::parse(body.c_str());
                json.at("StatusCode").get_to(this->StatusCode);
                json.at("StatusMsg").get_to(this->StatusMsg);
            }
            catch(const nlohmann::json::exception& e)
            {
                SLOG_ERROR(e.what());
            }
        }
    }

    // shutdown
    void MessageShutdownRequest::to_json (std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json_str = json.dump();
    }

    // test connect
    void MessageTestConnectionRequest::to_json (std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json_str = json.dump();
    }

    // check server
    void MessageCheckRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{{"operation", this->op}};
        json_str = json.dump();
    }

    // load
    MessageLoadRequest::MessageLoadRequest(const rapidjson::Document& json_data)
    {
        this->db_name = jsonParam(json_data, "db_name");
        this->csr = jsonParam(json_data, "csr");
    }

    MessageLoadRequest::MessageLoadRequest(std::string db_name, std::string csr) : MessageRequest(std::string("load"))
    {
        this->db_name = db_name;
        this->csr = csr;
    }

    MessageLoadRequest::MessageLoadRequest(std::string username, std::string password, std::string db_name, std::string csr) : MessageRequest("load", username, password)
    {
        this->db_name = db_name;
        this->csr = csr;
    }

    void MessageLoadRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["db_name"] = this->db_name,
        json["csr"] = this->csr;
        json_str = json.dump();
    }

    void MessageLoadRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", ""},
            {"db_name", this->db_name},
            {"inner", "true"}};
        json_str = json.dump();
    }

    bool MessageLoadRequest::Csr()const
    {
        if (this->csr.empty())
            return false;
        else if (this->csr == "1")
            return true;
        else if (this->csr == "true")
            return true;
        else if (this->csr == "bool")
            return true;
        return false;
    }

    MessageLoadResponse::MessageLoadResponse()
    {
        this->csr = "";
    }

    void MessageLoadResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["csr"] = this->csr;
        json_str = json.dump();
        SLOG_TRACE("MessageLoadResponse:" << json_str);
    }

    // login
    void MessageLoginRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json_str = json.dump();
    }

    // refresh configure
    void MessageRefreshconfRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json_str = json.dump();
    }

    void MessageRefreshconfRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", ""},
            {"inner", "true"}};
        json_str = json.dump();
    }

    // init db
    MessageInitRequest::MessageInitRequest(std::string db_names) : MessageRequest(std::string("init"))
    {
        this->db_names = db_names;
    }

    MessageInitRequest::MessageInitRequest(std::string username, std::string password, std::string db_names) : MessageRequest("init", username, password)
    {
        this->db_names = db_names;
    }

    void MessageInitRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["db_names"] = this->db_names;
        json_str = json.dump();
    }
    
    void MessageInitRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", ""},
            {"db_names", this->db_names},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageInitData::MessageInitData(std::string _db_name, std::string _status, std::string _msg)
    {
        this->db_name = _db_name;
        this->status = _status;
        this->msg = _msg;
    }

    MessageInitResponse::MessageInitResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object() && json.contains("data"))
        {
            for (auto& j0 : json["data"])
            {
                std::string db_name = j0["db_name"];
                std::string status = j0["status"];
                std::string msg = j0["msg"];
                data.push_back(MessageInitData(db_name, status, msg));
            }
        }
    }

    // show dbs
    void MessageShowRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json_str = json.dump();
    }

    MessageShowResponseBody::MessageShowResponseBody(std::string _database, std::string _creator, std::string _builtTime, std::string _status)
    {
        this->database = _database;
        this->creator = _creator;
        this->builtTime = _builtTime;
        this->status = _status;
    }

    MessageShowResponse::MessageShowResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object() && json.contains("ResponseBody"))
        {
            for (auto& j0 : json["ResponseBody"])
            {
                std::string database = j0["database"];
                std::string creator = j0["creator"];
                std::string built_time = j0["built_time"];
                std::string status = j0["status"];
                responseBody.push_back(MessageShowResponseBody(database, creator, built_time, status));
            }
        }
    }

    // unload db
    MessageUnloadRequest::MessageUnloadRequest(std::string db_name) : MessageRequest(std::string("unload"))
    {
        this->db_name = db_name;
    }

    MessageUnloadRequest::MessageUnloadRequest(std::string username, std::string password, std::string db_name) : MessageRequest("unload", username, password)
    {
        this->db_name = db_name;
    }

    void MessageUnloadRequest::to_json(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["db_name"] = this->db_name;
        json_str = json.dump();
    }

    void MessageUnloadRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", ""},
            {"db_name", this->db_name},
            {"inner", "true"}};
        json_str = json.dump();
    }

    // monitor
    MessageMonitorRequest::MessageMonitorRequest(std::string db_name) : MessageRequest(std::string("monitor"))
    {
        this->db_name = db_name;
    }

    MessageMonitorRequest::MessageMonitorRequest(std::string username, std::string password, std::string db_name) : MessageRequest("monitor", username, password)
    {
        this->db_name = db_name;
    }

    MessageMonitorRequest::MessageMonitorRequest(const rapidjson::Document& json_data)
    {
        this->db_name = jsonParam(json_data, "db_name");
        this->disk = jsonParam(json_data, "disk");
    }

    void MessageMonitorRequest::to_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", this->username},
            {"password", this->password},
            {"db_name", this->db_name}};
        json_str = json.dump();
    }

    void MessageMonitorRequest::to_inner_json(std::string& json_str)
    {
        nlohmann::json json = nlohmann::json{
            {"operation", this->op},
            {"username", "root"},
            {"password", ""},
            {"db_name", this->db_name},
            {"inner", "true"}};
        json_str = json.dump();
    }

    MessageMonitorResponse::MessageMonitorResponse(std::string body) : MessageResponse(body)
    {
        if (json.is_object())
        {
            if (json.contains("database"))
                json.at("database").get_to(this->database);
            if (json.contains("creator"))
                json.at("creator").get_to(this->creator);
            if (json.contains("builtTime"))
                json.at("builtTime").get_to(this->builtTime);
            if (json.contains("tripleNum"))
                json.at("tripleNum").get_to(this->tripleNum);
            if (json.contains("entityNum"))
                json.at("entityNum").get_to(this->entityNum);
            if (json.contains("literalNum"))
                json.at("literalNum").get_to(this->literalNum);
            if (json.contains("subjectNum"))
                json.at("subjectNum").get_to(this->subjectNum);
            if (json.contains("predicateNum"))
                json.at("predicateNum").get_to(this->predicateNum);
            if (json.contains("connectionNum"))
                json.at("connectionNum").get_to(this->connectionNum);
            if (json.contains("diskUsed"))
                json.at("diskUsed").get_to(this->diskUsed);
        }
    }

    MessageMonitorResponse::MessageMonitorResponse()
    {
        this->database = "";
        this->creator = "";
        this->database = "";
        this->tripleNum = "0";
        this->entityNum = 0;
        this->literalNum = 0;
        this->subjectNum = 0;
        this->predicateNum = 0;
        this->connectionNum = 0;
        this->diskUsed = 0;
    }

    void MessageMonitorResponse::toJsonString(std::string& json_str)
    {
        nlohmann::json json;
        toJson(json);
        json["database"] = this->database;
        json["creator"] = this->creator;
        json["builtTime"] = this->database;
        json["tripleNum"] = this->tripleNum;
        json["entityNum"] = this->entityNum;
        json["literalNum"] = this->literalNum;
        json["subjectNum"] = this->subjectNum;
        json["predicateNum"] = this->predicateNum;
        json["connectionNum"] = this->connectionNum;
        json["diskUsed"] = this->diskUsed;
        json["subjectList"] = nlohmann::json::array();
        if (!this->subjectList.empty())
        {
            nlohmann::json temp;
            for (auto &m : this->subjectList)
            {
                temp["name"] = m.first;
                temp["value"] = m.second;
                json["subjectList"].push_back(temp);
            }
        }
        json_str = json.dump();
    }
}