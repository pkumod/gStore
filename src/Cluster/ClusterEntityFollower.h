#pragma once
#include "ClusterDefined.h"
#include "ClusterEntity.h"

namespace cluster
{
    class ClusterEntityFollower : public cluster::ClusterEntity
    {
        ClusterNode leaderNode_;
        std::string ip_;
        std::string port_;
        ClusterNode own_;
        std::set<std::string> restoreDbL_;
        public:

        // virtual function in here
        public:
        void init()override;
        ClusterRoleType getCluterRoleType()const override { return cluster::ClusterRoleType_Follow; }
        std::string getLeaderIp()const override{ return leaderNode_.getIp(); }
        std::string getLeaderUrl()const override;
        ClusterNode getLearrNode()const override;
        void setIpPort(const std::string& ip, const std::string& port);
        std::string getIp()const{ return own_.getIp(); }
        std::string getPort()const{ return own_.getPort(); }
        void addRestoreDb(const std::string& db_name);
        void removeRestoreDb(const std::string& db_name);
        bool isFollowerRestoring(const std::string& db_name);
    };
}