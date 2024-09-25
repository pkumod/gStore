#include "ClusterDefined.h"

namespace cluster
{
    std::string ClusterOperationHandle::to_str(const ClusterOperation &type)
    {
        auto it = ClusterOperation_str.find(type);
        if (it == ClusterOperation_str.end())
            return std::string();
        return it->second;
    }

    ClusterOperation ClusterOperationHandle::to_enum(const std::string &type_str)
    {
        if (type_str.empty())
        {
            return ClusterOperation_None;
        }

        for (const auto& m : ClusterOperation_str)
        {
            if (m.second == type_str)
            {
                return m.first;
            }
        }
        return ClusterOperation_Undefine;
    }
}