#pragma once
#include <string>
#include "ClusterTypedef.h"
#include "../Util/Util.h"

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
        ClusterLogStatus_fail      = -1,
        ClusterLogStatus_None      = 0,
        ClusterLogStatus_HeartBeat = 1,
        ClusterLogStatus_pending   = 2,
        ClusterLogStatus_handling  = 3,
        ClusterLogStatus_sync      = 4,
        ClusterLogStatus_commit    = 5,
        ClusterLogStatus_cancel    = 6,
        ClusterLogStatus_build     = 8,
        ClusterLogStatus_drop      = 9,
        ClusterLogStatus_recover   = 10,
    };

    enum ClusterTranctionType
    {
        ClusterTranctionType_Insert = 1,
        ClusterTranctionType_Delete = 2,
    };

    enum cluster_operation
    {
        CLUSTER_OPERATION_TYPE_NONE = 0,
        LEADER_HEARTBEAT            = 1,
        LEADER_APPEND               = 2,
        FOLLOWER_REPLY              = 3,
        FOLLOWER_CHECK              = 4,
        EXPECTION_COMPARE           = 5,
        EXPECTION_PREPARE           = 6,
        EXPECTION_COMMIT            = 7,
        EXPECTION_CANCEL            = 8,
        EXPECTION_FAIL              = 9,
        EXPECTION_BUILD             = 10,
        EXPECTION_DROP              = 11,
        EXPECTION_RECOVER           = 12,
        CLUSTER_OPERATION_TYPE_UNDEFINE = 13,
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
        void setBaseUrl(){ base_url = "http://" + ip + ":" + port; }
        std::string getIp()const{ return ip; }
        std::string getPort()const{ return port; }
        std::string getUsername()const{ return username; }
        std::string getPassword()const{ return password; }
        std::string getBaseUrl()const{ return base_url; }
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
}