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
        
        void toJsonString(std::string& json_str);
    };
}