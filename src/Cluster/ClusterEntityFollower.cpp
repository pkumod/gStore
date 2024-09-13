#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"

namespace cluster
{
    void ClusterEntityFollower::init()
    {
        string cluster_node = Util::getConfigureValue("cluster_node");
        std::vector<std::string> node;
		Util::split(cluster_node, ":", node);
        if (node.size() < 4)
            return;
        leaderNode_.ip       = node[0];
        leaderNode_.port     = node[1];
        leaderNode_.username = node[2];
        leaderNode_.password = node[3];
    }
}