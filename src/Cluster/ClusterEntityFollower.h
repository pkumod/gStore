#pragma once
#include "ClusterDefined.h"
#include "ClusterEntity.h"

namespace cluster
{
    class ClusterEntityFollower : public cluster::ClusterEntity
    {
        ClusterNode leaderNode_;
        public:

        // virtual function in here
        public:
        void init()override;
        ClusterRoleType getCluterRoleType()const override { return cluster::ClusterRoleType_Follow; }
        std::string getLeaderIp()const override{ return leaderNode_.getIp(); }
        std::string getLeaderUrl()const override;
        ClusterNode getLearrNode()const override;
    };
}