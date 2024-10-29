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
        leaderNode_.setBaseUrl();
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

    void ClusterEntityFollower::addRestoreDb(const std::string& db_name)
    {
        if (restoreDbL_.find(db_name) != restoreDbL_.end())
            return;
        restoreDbL_.insert(db_name);
    }

    void ClusterEntityFollower::removeRestoreDb(const std::string& db_name)
    {
        restoreDbL_.erase(db_name);
    }

    bool ClusterEntityFollower::isFollowerRestoring(const std::string& db_name)
    {
        if (restoreDbL_.find(db_name) != restoreDbL_.end())
            return true;
        return false;
    }
}