#pragma once
#include <string>
#include <cstdio>
#include "../Api/NlohmanJson.hpp"
using namespace nlohmann;
namespace httpentities {

    // void to_json(nlohmann::json& s, const ClusterRequest& t) {
    //     s = nlohmann::json{{"term", t.term},{"index", t.index},{"db_name", t.db_name}};
    // };

    // void to_json(nlohmann::json& s, const ShutdownRequest& t) {
    //     s = nlohmann::json{{"operation", t.op},{"username", t.username},{"password", t.password}};
    // };

    // void to_json(nlohmann::json& s, const TestConnectionRequest& t) {
    //     s = nlohmann::json{{"operation", t.op},{"username", t.username},{"password", t.password}};
    // };

    // void to_json(nlohmann::json& s, const LoadRequest& t) {
    //     s = nlohmann::json{{"operation", t.op},{"username", t.username},{"password", t.password},{"db_name", t.db_name},{"csr", t.csr}};
    // };

    struct BaseRequest {
        std::string op;
        std::string username;
        std::string password;
        BaseRequest(std::string op) : op(op) {}
        BaseRequest(std::string op, std::string username, std::string password) : op(op), username(username), password(password) {}
        void init(std::string username, std::string password)
        {
            this->username = username;
            this->password = password;
        }
        virtual void to_json(std::string& json_str) = 0;
        virtual void to_inner_json(std::string& json_str)
        {
            nlohmann::json json = nlohmann::json{{"operation", this->op},{"username", "root"},{"password", ""},{"inner", "true"}};
            json_str = json.dump();
        }
    };

    struct ClusterRequest {
        uint32_t term;
        uint64_t index;
        std::string db_name;
        ClusterRequest() {}

        ClusterRequest(uint32_t term) : term(term) {}
        ClusterRequest(uint32_t term, std::string db_name, uint64_t index) : term(term), db_name(db_name), index(index) {}
        void setDbName(std::string db_name)
        {
            this->db_name = db_name;
        }
        void setIndex(uint64_t index)
        {
            this->index = index;
        }
        virtual void to_json(std::string& json_str) = 0;
    };

    struct BaseResponse {
        int StatusCode;
        std::string StatusMsg;
        std::string body;
        nlohmann::json json;
        BaseResponse(int StatusCode, std::string StatusMsg) : StatusCode(StatusCode), StatusMsg(StatusMsg) {}
        BaseResponse(const std::string& body) : body(body) {
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
                    std::cerr << e.what() << std::endl;
                }
            }
        }
        int getStatusCode() { return StatusCode; }
        std::string getStatusMsg() { return StatusMsg; }
        bool success() { return StatusCode == 0; }
    };

    struct ClusterResponse : public BaseResponse {
        ClusterResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        ClusterResponse(std::string body) : BaseResponse(body) {}
    };

    struct ShutdownRequest : public BaseRequest {
        ShutdownRequest() : BaseRequest("shutdown") {}
        ShutdownRequest(std::string username, std::string password) : BaseRequest("shutdown", username, password) {}
        void to_json (std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{{"operation", this->op},{"username", this->username},{"password", this->password}};
            json_str = json.dump();
        }
    };

    struct ShutdownResponse : public BaseResponse {
        ShutdownResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        ShutdownResponse(std::string body) : BaseResponse(body) {}
    };

    struct TestConnectionRequest : public BaseRequest {
        TestConnectionRequest() : BaseRequest("testConnect") {}
        TestConnectionRequest(std::string username, std::string password) : BaseRequest("testConnect", username, password) {}
        void to_json (std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{{"operation", this->op},{"username", this->username},{"password", this->password}};
            json_str = json.dump();
        }
    };

    struct TestConnectionResponse : public BaseResponse {
        TestConnectionResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        TestConnectionResponse(std::string body) : BaseResponse(body) {
            // parse other fields
        }
    };

    struct CheckRequest : public BaseRequest {
        CheckRequest() : BaseRequest("check") {}
        std::string to_params ()
        {
            return "operation="+op;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{{"operation", this->op}};
            json_str = json.dump();
        }
    };

    struct CheckResponse : public BaseResponse {
        CheckResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        CheckResponse(std::string body) : BaseResponse(body) {}
    };

    struct LoadRequest : public BaseRequest {
        std::string db_name;
        std::string csr;
        LoadRequest(std::string db_name, std::string csr) : BaseRequest("load") {
            this->db_name = db_name;
            this->csr = csr;
        }
        LoadRequest(std::string username, std::string password, std::string db_name, std::string csr) : BaseRequest("load", username, password) {
            this->db_name = db_name;
            this->csr = csr;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", this->username},
                {"password", this->password},
                {"db_name", this->db_name},
                {"csr", this->csr}};
            json_str = json.dump();
        }
        void to_inner_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", "root"},
                {"password", ""},
                {"db_name", this->db_name},
                {"inner", "true"}};
            json_str = json.dump();
        }
    };

    struct LoginRequest : public BaseRequest {
        LoginRequest() : BaseRequest("login") {}
        LoginRequest(std::string username, std::string password) : BaseRequest("login",username,password) {}
        void to_json(std::string& json_str) override
        {
             nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", this->username},
                {"password", this->password}};
            json_str = json.dump();
        }
    };

    struct ShowRequest : public BaseRequest {
        ShowRequest() : BaseRequest("show") {};
        ShowRequest(std::string username, std::string password) : BaseRequest("show",username,password) {}
        void to_json(std::string& json_str) override
        {
             nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", this->username},
                {"password", this->password}};
            json_str = json.dump();
        }
    };

    struct UnloadRequest : public BaseRequest {
        std::string db_name;
        UnloadRequest(std::string db_name) : BaseRequest("unload") {
            this->db_name = db_name;
        }
        UnloadRequest(std::string username, std::string password, std::string db_name) : BaseRequest("unload", username, password) {
            this->db_name = db_name;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", this->username},
                {"password", this->password},
                {"db_name", this->db_name}};
            json_str = json.dump();
        }
        void to_inner_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", "root"},
                {"password", ""},
                {"db_name", this->db_name},
                {"inner", "true"}};
            json_str = json.dump();
        }
    };

    struct BuildRequest : public BaseRequest {
        std::string db_name;
        std::string db_path;
        BuildRequest(std::string db_name, std::string db_path) : BaseRequest("build") {
            this->db_name = db_name;
            this->db_path = db_path;
        }
        BuildRequest(std::string username, std::string password, std::string db_name, std::string db_path) : BaseRequest("build", username, password) {
            this->db_name = db_name;
            this->db_path = db_path;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", this->username},
                {"password", this->password},
                {"db_name", this->db_name},
                {"db_path", this->db_path}};
            json_str = json.dump();
        }
        void to_inner_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", "root"},
                {"password", ""},
                {"db_name", this->db_name},
                {"db_path", this->db_path},
                {"inner", "true"}};
            json_str = json.dump();
        }
    };

    struct DropRequest : public BaseRequest {
        std::string db_name;
        std::string is_backup;
        DropRequest(std::string db_name, std::string is_backup) : BaseRequest("drop") {
            this->db_name = db_name;
            this->is_backup = is_backup;
        }
        DropRequest(std::string username, std::string password, std::string db_name, std::string is_backup) : BaseRequest("drop", username, password) {
            this->db_name = db_name;
            this->is_backup = is_backup;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", this->username},
                {"password", this->password},
                {"db_name", this->db_name},
                {"is_backup", this->is_backup}};
            json_str = json.dump();
        }
        void to_inner_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", "root"},
                {"password", ""},
                {"db_name", this->db_name},
                {"is_backup", this->is_backup},
                {"inner", "true"}};
            json_str = json.dump();
        }
    };

    struct MonitorRequest : public BaseRequest {
        std::string db_name;
        MonitorRequest(std::string db_name) : BaseRequest("monitor") {
            this->db_name = db_name;
        }
        MonitorRequest(std::string username, std::string password, std::string db_name) : BaseRequest("monitor", username, password) {
            this->db_name = db_name;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", this->username},
                {"password", this->password},
                {"db_name", this->db_name}};
            json_str = json.dump();
        }
        void to_inner_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", "root"},
                {"password", ""},
                {"db_name", this->db_name},
                {"inner", "true"}};
            json_str = json.dump();
        }
    };

    struct QueryRequest : public BaseRequest {
        std::string db_name;
        std::string sparql;
        std::string format;
        QueryRequest(std::string db_name,  std::string sparql, std::string format = "json") : BaseRequest("query") {
            this->db_name = db_name;
            this->sparql = sparql;
            this->format = format;
        }
        QueryRequest(std::string username, std::string password, std::string db_name, std::string sparql, std::string format = "json") : BaseRequest("query", username, password) {
            this->db_name = db_name;
            this->sparql = sparql;
            this->format = format;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", this->username},
                {"password", this->password},
                {"db_name", this->db_name},
                {"sparql", this->sparql},
                {"format", this->format}};
            json_str = json.dump();
        }
        void to_inner_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", "root"},
                {"password", ""},
                {"db_name", this->db_name},
                {"sparql", this->sparql},
                {"format", this->format},
                {"inner", "true"}};
            json_str = json.dump();
        }
    };


    struct LoadResponse : public BaseResponse {
        LoadResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        LoadResponse(std::string body) : BaseResponse(body) {}
    };

    struct BuildResponse : public BaseResponse {
        uint64_t failed_num;
        BuildResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        BuildResponse(std::string body) : BaseResponse(body) {
            if (json.is_object())
            {
                json.at("failed_num").get_to(this->failed_num);
            }
            else
            {
                failed_num = 0;
            }
        }
    };
    
    struct ShowResponseBody {
        std::string database;
        std::string creator;
        std::string builtTime;
        std::string status;
        ShowResponseBody(std::string _database, std::string _creator, std::string _builtTime, std::string _status) {
            this->database = _database;
            this->creator = _creator;
            this->builtTime = _builtTime;
            this->status = _status;
        }
    };

    struct ShowResponse : public BaseResponse {
        std::vector<struct ShowResponseBody> responseBody;
        ShowResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        ShowResponse(std::string body) : BaseResponse(body) {
            if (json.is_object() && json.contains("ResponseBody"))
            {
                for (auto& j0 : json["ResponseBody"])
                {
                    std::string database = j0["database"];
                    std::string creator = j0["creator"];
                    std::string built_time = j0["built_time"];
                    std::string status = j0["status"];
                    responseBody.push_back(ShowResponseBody(database, creator, built_time, status));
                }
            }
        }
    };

    struct MonitorResponse : public BaseResponse {
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
        MonitorResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        MonitorResponse(std::string body) : BaseResponse(body) {
            if (json.is_object())
            {
                json.at("database").get_to(this->database);
                json.at("creator").get_to(this->creator);
                json.at("builtTime").get_to(this->builtTime);
                json.at("tripleNum").get_to(this->tripleNum);
                json.at("entityNum").get_to(this->entityNum);
                json.at("literalNum").get_to(this->literalNum);
                json.at("subjectNum").get_to(this->subjectNum);
                json.at("predicateNum").get_to(this->predicateNum);
                json.at("connectionNum").get_to(this->connectionNum);
                json.at("diskUsed").get_to(this->diskUsed);
            }
        }
    };

    struct QueryResponse : public BaseResponse {
        std::vector<std::string> head;
        std::vector<std::vector<std::string>> results;
        uint64_t ansNum;
        uint64_t outputLimit;
        std::string queryTime;
        std::string threadId;
        QueryResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        QueryResponse(std::string body) : BaseResponse(body) {
            if (json.is_object())
            {
                if (json.contains("AnsNum"))
                    json.at("AnsNum").get_to(this->ansNum);
                if (json.contains("OutputLimit"))
                    json.at("OutputLimit").get_to(this->outputLimit);
                if (json.contains("QueryTime"))
                    json.at("QueryTime").get_to(this->queryTime);
                if (json.contains("ThreadId"))
                    json.at("ThreadId").get_to(this->threadId);
                if (json.contains("head"))
                    json.at("head").get_to(this->head);
                if (json.contains("results"))
                    json.at("results").get_to(this->results);
            }
        }
    };

    struct ReplyRequest: public ClusterRequest {
        std::string expection;
        ReplyRequest(uint32_t term, std::string db_name, uint64_t index, std::string expection): ClusterRequest(term, db_name, index) {
            this->expection = expection;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{{"term", this->term},{"index", this->index},{"db_name", this->db_name},{"expection", this->expection}};
            json_str = json.dump();
        }
    };

    struct AppenEntriesRequest: public ClusterRequest {
        std::string file_path;
        std::string operation;
        AppenEntriesRequest(uint32_t term, std::string db_name, uint64_t index, std::string operation, std::string file_path): ClusterRequest(term, db_name, index) {
            this->file_path = file_path;
            this->operation = operation;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{{"term", this->term},{"index", this->index},{"db_name", this->db_name}, {"operaton", this->operation}};
            json_str = json.dump();
        }
        std::string getFilePath() {return this->file_path;}
    };

    struct HeartBeatRequest: public ClusterRequest {
        std::string expection;
        HeartBeatRequest(uint32_t term, std::string db_name, uint64_t index, std::string expection): ClusterRequest(term, db_name, index) {
            this->expection = expection;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{{"term", this->term},{"index", this->index},{"db_name", this->db_name},{"expection", this->expection}};
            json_str = json.dump();
        }
    };

    struct ClusterCheckRequest: public ClusterRequest
    {
        uint16_t result;
        ClusterCheckRequest(uint32_t term, std::string db_name, uint64_t index, uint16_t result):  ClusterRequest(term, db_name, index) {
            this->result = result;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{{"term", this->term},{"index", this->index},{"db_name", this->db_name}, {"result", this->result}};
            json_str = json.dump();
        }
    };
    

    struct CancelRequest: public ClusterRequest {
        std::string filename;
        std::string operation;
        CancelRequest(uint32_t term, std::string db_name, uint64_t index, std::string operation, std::string filename): ClusterRequest(term, db_name, index) {
            this->filename = filename;
            this->operation = operation;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{{"term", this->term},{"index", this->index},{"db_name", this->db_name}, {"operaton", this->operation}, {"filename", this->filename}};
            json_str = json.dump();
        }
    };
}