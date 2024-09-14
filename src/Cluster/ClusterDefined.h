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
        std::string getUrlString()const
        {
            std::string url;
            url += "http://" + ip + ":" + port + "/grpc/cluster";
            return url;
        }
        bool empty()const{ return ip.empty(); }
    };

    struct ClusterHeartBeat
    {
        ClusterLogStatus status;
        uint32 term;
        uint32 index;
        std::string db_name;

        ClusterHeartBeat()
        {
            status  = ClusterLogStatus_HeartBeat;
            term  = 0;
            index = 0;
            db_name = "";
        }

        ClusterHeartBeat(ClusterLogStatus status_, uint32 term_, uint32 index_)
        {
            status  = status_;
            term  = term_;
            index = index_;
            db_name = "";
        }

        void setStatus(ClusterLogStatus value){ status = value; }
        void setTerm(uint32 value){ term = value; }
        void setIndex(uint32 value){ index = value; }
        void setDbName(const std::string& value){ db_name = value; }
        std::string toPostString(std::string username, std::string password)const
        {
            std::string res;
            res += "{\"operation\":\"ClusterHeartBeat\",";
            res += "\"username\":\"" + username + "\",";
            res += "\"password\":\"" + password + "\",";
            res += "\"type\":\"" + std::to_string(status) + "\",";
            res += "\"term\":\"" + std::to_string(term) + "\",";
            res += "\"index\":\"" + std::to_string(index) + "\",";
            res += "\"db_name\":\"" + db_name + "\"}";

            return res;
        }
    };

    struct ClusterSync
    {
        ClusterOperation operation;
        ClusterLogStatus status;
        uint32 term;
        uint64 index;
        std::string db_name;
        std::string file_path;

        ClusterSync()
        {
            operation = ClusterOperation_None;
            status    = ClusterLogStatus_HeartBeat;
            term      = 0;
            index     = 0;
            db_name   = "";
            file_path = "";
        }
        void setOperation(ClusterOperation value){ operation = value; }
        void setStatus(ClusterLogStatus value){ status = value; }
        void setTerm(uint32 value){ term = value; }
        void setIndex(uint64 value){ index = value; }
        void setDbName(const std::string& value){ db_name = value; }
        void setFilePath(const std::string& value){ file_path = value; }
    };
}