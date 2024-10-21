#pragma once
#include "MessageApi.h"

namespace server
{
    // show user list
    struct MessageResonManageRequest : public MessageRequest
    {
    };

    struct MessageReasonManageResponse : public MessageResponse
    {
        std::string type;
        nlohmann::json list;
        int num;
        std::string insert_sparql;
        std::string delete_sparql;
        std::string check_sparql;
        nlohmann::json ruleinfo;
        std::string checkMsg;
        MessageReasonManageResponse(){num = 0;}
        void toJsonString(std::string& json_str);
    };

}