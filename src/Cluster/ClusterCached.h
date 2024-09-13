#pragma once
#include "ClusterDefined.h"
#include "../Util/Util.h"
#include "../Api/NlohmanJson.hpp"

namespace cluster
{
    struct TripleInfo
    {
        std::string subject;
        std::string predicate;
        std::string object;
        std::string batch_index;
        ClusterOperation operation;
        uint64 index;
        public:
        TripleInfo()
        {
            subject = "";
            predicate = "";
            object = "";
            batch_index = "";
            operation = ClusterOperation_None;
        }
    };
}