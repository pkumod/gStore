#pragma once
#include <string>
#include "ClusterTypedef.h"
#include "../Util/Util.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <map>
#include <vector>

namespace cluster
{
    enum ClusterRoleType
    {
        ClusterRoleType_Leader = 1,
        ClusterRoleType_Follow = 2,
    };

    enum ClusterOperation
    {
        ClusterOperation_None   = 0,
        ClusterOperation_Insert = 1,
        ClusterOperation_Delete = 2,
    };

    enum ClusterLogStatus
    {
        ClusterLogStatus_HeartBeat = -2,
        ClusterLogStatus_Fail      = -1,
        ClusterLogStatus_pending   = 0,
        ClusterLogStatus_handling  = 1,
        ClusterLogStatus_sync      = 2,
        ClusterLogStatus_commit    = 3,
        ClusterLogStatus_cancel    = 4,
    };

    enum ClusterTranctionType
    {
        ClusterTranctionType_Insert = 1,
        ClusterTranctionType_Delete = 2,
    };

    struct ClusterNode
    {
        private:
        std::string ip;
        std::string port;
        std::string username;
        std::string password;
        std::string base_url;
        public:
        ClusterNode()
        {
            ip       = "";
            port     = "";
            username = "";
            password = "";
            base_url = "";
        }
        ClusterNode(std::string ip_, std::string port_, std::string username_, std::string password_)
        {
            ip       = ip_;
            port     = port_;
            username = username_;
            password = password_;
            base_url = "http://" + ip_ + ":" + port_;
        }
        void setIp(std::string value){ ip = value; }
        void setPort(std::string value){ port = value; }
        void setUsername(std::string value){ username = value; }
        void setPassword(std::string value){ password = value; }
        std::string getIp()const{ return ip; }
        std::string getPort()const{ return port; }
        std::string getUsername()const{ return username; }
        std::string getPassword()const{ return password; }
        std::string getCheckUrl()const
        {
            return base_url + "/cluster/check";
        }
        std::string getReplyUrl()const
        {
            return base_url + "/cluster/reply";
        }
        std::string getHeartBeatUrl()const
        {
            return base_url + "/cluster/heartbeat";
        }
        std::string getAppendEntriesUrl()const
        {
            return base_url + "/cluster/appendEntries";
        }
        std::string getCancelUrl()const
        {
            return base_url + "/cluster/cancel";
        }
        bool empty()const{ return ip.empty(); }
    };

    struct ClusterEvent
    {
        virtual void runEvent()const {}
    };

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
            CLUSTER_OPERATION_TYPES_MAP(CLUSTEROP)
    #undef CLUSTEROP
            default:
                return "";
        }
        return std::string();
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