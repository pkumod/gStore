#pragma once
#include <string>
#include "../Api/NlohmanJson.hpp"

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
        virtual void to_json_str(std::string& json_str) = 0;
        virtual void to_inner_json(std::string& json_str)
        {
            nlohmann::json json = nlohmann::json{{"operation", this->op},{"username", this->username},{"password", ""},{"inner", "true"}};
            json_str = json.dump();
        }
    };

    struct ClusterRequest {
        int term;
        long long index;
        std::string db_name;
        ClusterRequest() {}

        ClusterRequest(int32_t term) : term(term) {}
        ClusterRequest(int32_t term, std::string db_name, int64_t index) : term(term), db_name(db_name), index(index) {}
        void setDbName(std::string db_name)
        {
            this->db_name = db_name;
        }
        void setIndex(int64_t index)
        {
            this->index = index;
        }
        virtual void to_json_str(std::string& json_str) = 0;
    };

    struct BaseResponse {
        int StatusCode;
        std::string StatusMsg;
        std::string body;
        nlohmann::json json;
        BaseResponse(int StatusCode, std::string StatusMsg) : StatusCode(StatusCode), StatusMsg(StatusMsg) {}
        BaseResponse(const std::string& body) : body(body) {
            if (json.accept(this->body))
            {
                json.parse(this->body);
                json.at("StatusCode").get_to(this->StatusCode);
                json.at("StatusMsg").get_to(this->StatusMsg);
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
        void to_json_str (std::string& json_str) override
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
        void to_json_str (std::string& json_str) override
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
        void to_json_str(std::string& json_str) override
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
        void to_json_str(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{
                {"operation", this->op},
                {"username", this->username},
                {"password", this->password},
                {"db_name", this->db_name},
                {"csr", this->csr}};
            json_str = json.dump();
        }
    };

    struct LoadResponse : public BaseResponse {
        LoadResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        LoadResponse(std::string body) : BaseResponse(body) {}
    };


    struct ReplyRequest: public ClusterRequest {
        ReplyRequest(int32_t term, std::string db_name, int64_t index): ClusterRequest(term, db_name, index) {}
        void to_json_str(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{{"term", this->term},{"index", this->index},{"db_name", this->db_name}};
            json_str = json.dump();
        }
    };

    struct AppenEntriesRequest: public ClusterRequest {
        std::string filepath;
        AppenEntriesRequest(int32_t term, std::string db_name, int64_t index, std::string filepath): ClusterRequest(term, db_name, index) {
            this->filepath = filepath;
        }
        void to_json_str(std::string& json_str) override
        {
            nlohmann::json json = nlohmann::json{{"term", this->term},{"index", this->index},{"db_name", this->db_name}};
            json_str = json.dump();
        }
        std::string getFilePath() {return this->filepath;}
    };
}