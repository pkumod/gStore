#include "ClusterManager.h"

namespace cluster
{
    ClusterManager::ClusterManager()
    {
        on_   = false;
        role_ = nullptr;
    }

    ClusterManager::~ClusterManager()
    {
    }

    void ClusterManager::init()
    {
        string cluster_on = Util::getConfigureValue("cluster_on");
        if (cluster_on == "yes")
        {
            string cluster_role = Util::getConfigureValue("cluster_role");
            if (cluster_role == "leader")
            {
                role_ = std::make_shared<ClusterEntityLeader>();
            }
            else if (cluster_role == "follow")
            {
                role_ = std::make_shared<ClusterEntityFollower>();
            }
            else
            {
                SLOG_ERROR("cluster_role config is error");
                return;
            }
            on_ = true;
            role_->init();
        }
    }

    void ClusterManager::initClusterDir(const std::vector<std::string>& dbList)
    {
        for (const auto m: dbList)
        {
            std::string db_path = Util::getConfigureValue("db_home");
        }
    }

    bool ClusterManager::isLeader()
    {
        if (getCluterRole() == cluster::ClusterRoleType_Leader)
            return true;
        return false;
    }

    void ClusterManager::startHeartBeat()
    {
        if (!isEnable() || !role_)
            return;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return;
        }
        leader->startHeardBeat();
    }

    int ClusterManager::startNotify(std::string db_name, uint32 term, uint32 index)
    {
        if (!isEnable() || !role_)
            return -1;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return -1;
        }
        return leader->startNotify(db_name, term, index);
    }

    int ClusterManager::startSync(std::string db_name, uint32 term, uint32 index, ClusterOperation operation, const std::string& file_path)
    {
        if (!isEnable() || !role_)
            return -1;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return -1;
        }
        return leader->startSync(db_name, term, index, operation, file_path);
    }

    bool ClusterManager::fromLeader(const std::string& ip)
    {
        if (ip.empty())
            return false;
        
        ClusterEntityFollowerPtr follower = std::dynamic_pointer_cast<ClusterEntityFollower>(role_);
        if (!follower)
            return false;
        if (follower->getLeaderIp() == ip)
            return true;
        return false;
    }

    bool ClusterManager::fromFollower(const std::string& ip)
    {
        if (ip.empty())
            return false;
        
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
            return false;
        ClusterNode node = leader->FindFollower(ip);
        if (!node.empty() && node.getIp() == ip)
            return true;
        return false;
    }

    void ClusterManager::addLog(std::string db_name, uint64 index, int status, ClusterOperation operation)
    {
        if (!isEnable() || !role_)
            return;

        role_->addLog(db_name, index, status, operation);
    }

    void ClusterManager::updateLogStatus(std::string db_name, uint64 index, int status)
    {
        if (!isEnable() || !role_)
            return;
        role_->updateLogStatus(db_name, index, status);
    }

    void ClusterManager::addLogReplyNum(std::string db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return;
        role_->addLogReplyNum(db_name, index);
    }

    void ClusterManager::addLogSyncNum(std::string db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return;
        role_->addLogSyncNum(db_name, index);
    }

    uint32 ClusterManager::getLogReplyNum(std::string db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return 0;
        return role_->getLogReplyNum(db_name, index);
    }

    uint32 ClusterManager::getLogSyncNum(std::string db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return 0;
        return role_->getLogSyncNum(db_name, index);
    }

    void ClusterManager::updateTerm(std::string db_name, uint32 term)
    {
        if (!isEnable() || !role_)
            return;
        
        role_->updateTerm(db_name, term);
    }

    void ClusterManager::updateTermIndex(std::string db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return;
        role_->updateTermIndex(db_name, index);
    }

    void ClusterManager::addCachedData(const std::vector<TripleInfo>& triple, ClusterOperation operation, const std::string file_path)
    {

    }

    void ClusterManager::appendCachedData(const std::vector<TripleInfo>& triple, ClusterOperation operation, const std::string file_path)
    {
        
    }
}