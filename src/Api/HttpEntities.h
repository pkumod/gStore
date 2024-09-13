#pragma once
#include <string>
#include "../Api/NlohmanJson.hpp"

namespace httpentities {

    void to_json(nlohmann::json& s, const ReplyRequest& t)
    {
        s["operation"] = t.op;
        s["username"] = t.username;
        s["password"] = t.password;
        s["term"]  = t.term;
        s["index"] = t.index;
        s["db_name"] = t.db_name;
    };

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
             json_str = "{\"operation\":\"" + op + "\",\"inner\":\"true\",\"username\":\"root\",\"password\":\"\"}";
        }
    };

    struct ClusterRequest {
        std::string op;
        std::string username;
        std::string password;
        int32_t term;
        int64_t index;
        std::string db_name;
        ClusterRequest(std::string op) : op(op) {}

        ClusterRequest(std::string op, std::string username, std::string password, int32_t term) : op(op), username(username), password(password), term(term) {}
        void init(std::string username, std::string password)
        {
            this->username = username;
            this->password = password;
        }
        void setDbName(std::string db_name)
        {
            this->db_name = db_name;
        }
        void setIndex(int64_t index)
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
            if (json.accept(this->body))
            {
                json.parse(this->body);
                json.at("StatusCode").get_to(this->StatusCode);
                json.at("StatusMsg").get_to(this->StatusMsg);
            }
            else
            {
                json = R"({})"_json;
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
            json_str = "{\"operation\":\"" + op + "\",\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";
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
            json_str = "{\"operation\":\"" + op + "\",\"username\":\"" + username + "\",\"password\":\"" + password + "\"}";
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
        void to_json (std::string& json_str) override
        {
            json_str = "{\"operation\":\"" + op + "\"}";
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
        void to_json (std::string& json_str) override
        {
            json_str = "{\"operation\":\"" + op + "\",\"username\":\"" + username + "\",\"password\":\"" + password + "\",\"db_name\":\"" + db_name + "\",\"csr\":\"" + csr + "\"}";
        }
        void to_inner_json(std::string& json_str) override
        {
            json_str = "{\"operation\":\"" + op + "\",\"inner\":\"true\",\"username\":\"root\",\"password\":\"\",\"db_name\":\"" + db_name + "\",\"csr\":\"" + csr + "\"}";
        }
    };

    struct LoadResponse : public BaseResponse {
        LoadResponse(int code, std::string msg) : BaseResponse(code, msg) {}
        LoadResponse(std::string body) : BaseResponse(body) {}
    };


    struct ReplyRequest: public ClusterRequest {
        ReplyRequest(std::string username, std::string password, int32_t term, std::string db_name, int64_t index): ClusterRequest("reply", username, password, term) {
            this->index = index;
            this->db_name = db_name;
        }
        void to_json(std::string& json_str) override
        {
            nlohmann::json json = this;
            json_str = json.dump();
        }
    };
}