/*
 * @Author: wangjian 2606583267@qq.com
 * @Date: 2024-09-13 12:00:00
 * @LastEditors: wangjian 2606583267@qq.com
 * @LastEditTime: 2024-09-13 12:00:00
 * @FilePath: /gstore/src/Cluster/ClusterOperation.h
 * @Description: 
 */
#pragma once
#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace cluster
{
    // CLUSTEROP(name,string)
    #define CLUSTER_OPERATION_TYPES_MAP(CLUSTEROP)                       \
        CLUSTEROP(LEADER_HEARTBEAT,                   heartbeat) \
        CLUSTEROP(LEADER_APPEND,                  appendEntries) \
        CLUSTEROP(FOLLOWER_REPLY,                         reply) \
        CLUSTEROP(FOLLOWER_CHECK,                         check) \
        CLUSTEROP(EXPECTION_COMPARE,                    compare) \
        CLUSTEROP(EXPECTION_PREPARE,                    prepare) \
        CLUSTEROP(EXPECTION_COMMIT,                      commit) 

    enum cluster_operation
    {
    #define CLUSTEROP(name, string) name,
        CLUSTER_OPERATION_TYPE_NONE,
        CLUSTER_OPERATION_TYPES_MAP(CLUSTEROP)
        CLUSTER_OPERATION_TYPE_UNDEFINE
    #undef CLUSTEROP
    };

    class ClusterOperationHandle
    {
    public:
        static std::string to_str(enum cluster_operation type);
        static enum cluster_operation to_enum(const std::string &type_str);
    };

    std::string ClusterOperationHandle::to_str(enum cluster_operation type)
    {
        switch (type)
        {
    #define CLUSTEROP(name, string) case name: return #string;
            OPERATION_TYPES_MAP(CLUSTEROP)
    #undef CLUSTEROP
            default:
                return "";
        }
    }

    enum cluster_operation ClusterOperationHandle::to_enum(const std::string &type_str)
    {
        if (type_str.empty())
        {
            return CLUSTER_OPERATION_TYPE_NONE;
        }
    #define CLUSTEROP(name, string) \
        if (type_str == #string) { \
            return name; \
        }
        CLUSTER_OPERATION_TYPES_MAP(CLUSTEROP)
    #undef CLUSTEROP
        return CLUSTER_OPERATION_TYPE_UNDEFINE;
    }
}
