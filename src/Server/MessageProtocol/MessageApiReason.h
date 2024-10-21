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
        std::vector<nlohmann::json> list;
        void toJsonString(std::string& json_str);
    };
}