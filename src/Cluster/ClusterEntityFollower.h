#pragma once
#include "ClusterDefined.h"
#include "ClusterEntity.h"

namespace cluster
{
    class ClusterEntityFollower : public cluster::ClusterEntity
    {
        ClusterNode leaderNode_;
        public:
        std::string getLeaderIp()const{ return leaderNode_.ip; }
        std::string getLeaderUrl();

        // virtual function in here
        public:
        void init()override;
        ClusterRoleType getCluterRoleType()const override { return cluster::ClusterRoleType_Follow; }
    };
}