#pragma once
#include "MessageApi.h"
#include "../../Pfn/PFNEntity.h"

namespace gs
{

    // funquery
    struct MessageFunQueryRequest : public MessageRequest
    {
        PFNInfo funInfo;
        MessageFunQueryRequest(): MessageRequest(string("funquery")) {};
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageFunQueryResponse : public MessageResponse
    {
        std::vector<PFNInfo> list;
        MessageFunQueryResponse() {}
        MessageFunQueryResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageFunQueryResponse(const std::string & body);
        void toJsonString(std::string& json_str) override;
    };

    // funcudb
    struct MessageFunCudbRequest : public MessageRequest
    {
        std::string type;
        PFNInfo funInfo;
        MessageFunCudbRequest() : MessageRequest(string("funcudb")) {};
        MessageFunCudbRequest(const std::string& type) : MessageRequest(string("funcudb")), type(type) {};
        void to_json(std::string& json_str) override;
        void to_inner_json(std::string& json_str) override;
    };

    struct MessageFunCudbResponse : public MessageResponse
    {
        MessageFunCudbResponse() {}
        MessageFunCudbResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageFunCudbResponse(const std::string & body): MessageResponse(body){}
        void toJsonString(std::string& json_str) override;
    };

    // funreview
    struct MessageFunReviewRequest : public MessageRequest
    {
        PFNInfo funInfo;
        MessageFunReviewRequest() : MessageRequest(string("funreview")) {};
        void to_json(std::string& json_str);
        void to_inner_json(std::string& json_str);
    };

    struct MessageFunReviewResponse : public MessageResponse
    {
        PFNInfo funInfo;
        MessageFunReviewResponse() { }
        MessageFunReviewResponse(int code, std::string msg) : MessageResponse(code, msg) {}
        MessageFunReviewResponse(const std::string& body): MessageResponse(body){}
        void toJsonString(std::string& json_str) override;
    };

}