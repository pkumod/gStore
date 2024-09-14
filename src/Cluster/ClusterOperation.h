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
    // OP(name,string)
    #define CLUSTER_OPERATION_TYPES_MAP(OP)                       \
        OP(LEADER_HEARTBEAT,                   heartbeat) \
        OP(LEADER_APPEND,                  appendEntries) \
        OP(FOLLOWER_REPLY,                         reply) \
        OP(EXPECTION_CHECK,                        check) \
        OP(EXPECTION_PREPARE,                    prepare) \
        OP(EXPECTION_COMMIT,                      commit) 

    enum cluster_operation
    {
    #define OP(name, string) name,
        CLUSTER_OPERATION_TYPE_NONE,
        CLUSTER_OPERATION_TYPES_MAP(OP)
        CLUSTER_OPERATION_TYPE_UNDEFINE
    #undef OP
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
    #define OP(name, string) case name: return #string;
            OPERATION_TYPES_MAP(OP)
    #undef OP
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
    #define OP(name, string) \
        if (type_str == #string) { \
            return name; \
        }
        CLUSTER_OPERATION_TYPES_MAP(OP)
    #undef OP
        return CLUSTER_OPERATION_TYPE_UNDEFINE;
    }
}
