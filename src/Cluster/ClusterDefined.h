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
        public:
        ClusterNode()
        {
            ip       = "";
            port     = "";
            username = "";
            password = "";
        }
        ClusterNode(std::string ip_, std::string port_, std::string username_, std::string password_)
        {
            ip       = ip_;
            port     = port_;
            username = username_;
            password = password_;
        }
        void setIp(std::string value){ ip = value; }
        void setPort(std::string value){ port = value; }
        void setUsername(std::string value){ username = value; }
        void setPassword(std::string value){ password = value; }
        std::string getIp()const{ return ip; }
        std::string getPort()const{ return port; }
        std::string getUsername()const{ return username; }
        std::string getPassword()const{ return password; }
        std::string getReplyUrl()const
        {
            std::string url;
            url += "http://" + ip + ":" + port + "/grpc/cluster/reply";
            return url;
        }
        std::string getHeartBeatUrl()const
        {
            std::string url;
            url += "http://" + ip + ":" + port + "/grpc/cluster/heartbeat";
            return url;
        }
        std::string getAppendEntriesUrl()const
        {
            std::string url;
            url += "http://" + ip + ":" + port + "/grpc/cluster/appendEntries";
            return url;
        }
        std::string getCancelUrl()const
        {
            std::string url;
            url += "http://" + ip + ":" + port + "/grpc/cluster/cancel";
            return url;
        }
        bool empty()const{ return ip.empty(); }
    };

    struct ClusterEvent
    {
        virtual void runEvent()const {}
    };
}