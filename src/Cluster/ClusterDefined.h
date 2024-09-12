#pragma once
#include <string>
#include "ClusterTypedef.h"

namespace cluster
{
    enum ClusterRoleType
    {
        ClusterRoleType_Leader = 1,
        ClusterRoleType_Follow = 2,
    };

    enum ClusterOperition
    {
        ClusterRoleType_Insert = 1,
        ClusterRoleType_Delete = 2,
    };

    enum ClusterLogStatus
    {
        ClusterLogStatus_HeartBeat = -2,
        ClusterLogStatus_Fail      = -1,
        ClusterLogStatus_pending   = 0,
        ClusterLogStatus_handling  = 1,
        ClusterLogStatus_sync      = 2,
        ClusterLogStatus_commit    = 3,
    };

    enum ClusterTranctionType
    {
        ClusterTranctionType_Insert = 1,
        ClusterTranctionType_Delete = 2,
    };

    struct ClusterNode
    {
        std::string ip;
        std::string port;
        std::string username;
        std::string password;
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
        std::string getUrlString()const
        {
            std::string url;
            url += "http://" + ip + ":" + port + "/grpc/api";
            return url;
        }
    };

    struct ClusterHeartBeat
    {
        ClusterLogStatus type;
        uint32 term;
        uint32 index;

        ClusterHeartBeat()
        {
            type  = ClusterLogStatus_HeartBeat;
            term  = 0;
            index = 0;
        }

        ClusterHeartBeat(ClusterLogStatus type_, uint32 term_, uint32 index_)
        {
            type  = type_;
            term  = term_;
            index = index_;
        }

        void setType(ClusterLogStatus value){ type = value; }
        void setTerm(uint32 value){ term = value; }
        void setIndex(uint32 value){ index = value; }
        std::string toPostString(std::string username, std::string password)const
        {
            std::string res;
            res += "{\"operation\":\"ClusterHeartBeat\",";
            res += "\"username\":\"" + username + "\",";
            res += "\"password\":\"" + password + "\",";
            res += "\"type\":\"" + std::to_string(type) + "\",";
            res += "\"term\":\"" + std::to_string(term) + "\",";
            res += "\"index\":\"" + std::to_string(index) + "\"}";

            return res;
        }
    };

    struct ClusterSync
    {
        ClusterOperition opersion_nt;
        ClusterLogStatus type;
        int term;
        int index;
        std::string sync_nt; //数据流数据
    };
}