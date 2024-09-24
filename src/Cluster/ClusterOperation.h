
#pragma once
#include <string>
#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include "ClusterDefined.h"

namespace cluster
{
    const std::map<cluster_operation, std::string> cluster_operation_str =
    {
        {CLUSTER_OPERATION_TYPE_NONE,     "NONE"},
        {LEADER_HEARTBEAT,                "heartbeat"},
        {LEADER_APPEND,                   "appendEntries"},
        {FOLLOWER_REPLY,                  "reply"},
        {FOLLOWER_CHECK,                  "check"},
        {EXPECTION_COMPARE,               "compare"},
        {EXPECTION_PREPARE,               "prepare"},
        {EXPECTION_COMMIT,                "commit"},
        {EXPECTION_CANCEL,                "cancel"},
        {EXPECTION_FAIL,                  "fail"},
        {EXPECTION_BUILD,                 "build"},
        {EXPECTION_DROP,                  "drop"},
        {EXPECTION_RECOVER,               "recover"},
        {CLUSTER_OPERATION_TYPE_UNDEFINE, "UNDEFINE"}
    };

    class ClusterOperationHandle
    {
    public:
        static std::string to_str(const cluster_operation &type);
        static cluster_operation to_enum(const std::string &type_str);
    };

    std::string ClusterOperationHandle::to_str(const cluster_operation &type)
    {
        auto it = cluster_operation_str.find(type);
        if (it == cluster_operation_str.end())
            return std::string();
        return it->second;
    }

    cluster_operation ClusterOperationHandle::to_enum(const std::string &type_str)
    {
        if (type_str.empty())
        {
            return CLUSTER_OPERATION_TYPE_NONE;
        }

        for (const auto& m : cluster_operation_str)
        {
            if (m.second == type_str)
            {
                return m.first;
            }
        }
        return CLUSTER_OPERATION_TYPE_UNDEFINE;
    }
}