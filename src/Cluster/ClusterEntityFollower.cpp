#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"

namespace cluster
{
    void ClusterEntityFollower::init()
    {
        getTerm();
        string cluster_node = Util::getConfigureValue("cluster_node");
        std::vector<std::string> node;
		Util::split(cluster_node, ":", node);
        if (node.size() < 4)
            return;
        leaderNode_.setIp(node[0]);
        leaderNode_.setPort(node[1]);
        leaderNode_.setUsername(node[2]);
        leaderNode_.setPassword(node[3]);
    }

    std::string ClusterEntityFollower::getLeaderUrl()const
    {
        std::string url = "http://" + leaderNode_.getIp() + ":" + leaderNode_.getPort();
        return url;
    }

    ClusterNode ClusterEntityFollower::getLearrNode()const
    {
        return leaderNode_;
    }
}