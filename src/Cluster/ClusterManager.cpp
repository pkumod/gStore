#include "ClusterManager.h"

namespace cluster
{
    ClusterManager::ClusterManager()
    {
        role_ = nullptr;
        if (Util::getConfigureValue("cluster_on") == "on")
            on_ = true;
        else
            on_ = false;
    }

    ClusterManager::~ClusterManager()
    {
    }

    void ClusterManager::init()
    {
        if (isEnable())
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
            if (!Util::dir_exist(ClusterDb::getClusterDir()))
            {
                Util::create_dir(ClusterDb::getClusterDir());
            }
            role_->init();

            SLOG_CORE("cluster success on");
        }
    }

    bool ClusterManager::isLeader()
    {
        if (!isEnable() || !role_)
            return false;
        if (getCluterRole() == cluster::ClusterRoleType_Leader)
            return true;
        return false;
    }

    bool ClusterManager::tryRecover(const std::vector<std::string>& dbs)
    {
        if (!isEnable() || !role_)
            return false;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("not is leader");
            return false;
        }

        leader->tryRecover(dbs);
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

    int ClusterManager::startNotify(std::string db_name, uint32 index)
    {
        if (!isEnable() || !role_)
            return -1;
        role_->addClusterDb(db_name);
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return -1;
        }
        return leader->startNotify(db_name, index);
    }

    int ClusterManager::startSync(std::string db_name, uint32 index, ClusterOperation operation, const std::string& file_path)
    {
        if (!isEnable() || !role_)
            return -1;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return -1;
        }
        return leader->startSync(db_name, index, operation, file_path);
    }

    bool ClusterManager::fromLeader(const std::string& ip)
    {
        if (!isEnable() || !role_ || ip.empty())
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
        if (!isEnable() || !role_ || ip.empty())
            return false;
        
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
            return false;
        ClusterNode node = leader->FindFollower(ip);
        if (!node.empty() && node.getIp() == ip)
            return true;
        return false;
    }

    std::vector<std::string> ClusterManager::getFollowrUrlArray()
    {
        if (!isEnable() || !role_)
            return std::vector<std::string>();
        return role_->getFollowrUrlArray();
    }

    std::string ClusterManager::getLeaderUrl()
    {
        if (!isEnable() || !role_)
            return std::string();
        return role_->getLeaderUrl();
    }

    ClusterNode ClusterManager::getLearrNode()
    {
        if (!isEnable() || !role_)
            return ClusterNode();
        return role_->getLearrNode();
    }

    std::vector<ClusterNode> ClusterManager::getFollowNodeL()
    {
        if (!isEnable() || !role_)
            return std::vector<ClusterNode>();
        return role_->getFollowNodeL();
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

    void ClusterManager::updateTerm(uint32 term)
    {
        if (!isEnable() || !role_)
            return;
        
        role_->updateTerm(term);
    }

    void ClusterManager::updateDbIndex(std::string db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return;
        role_->updateDbIndex(db_name, index);
    }

    uint32 ClusterManager::getTerm()
    {
        if (!isEnable() || !role_)
            return INVALID;
        return role_->getTerm();
    }

    void ClusterManager::getDbIndex(std::string db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return;
        role_->getDbIndex(db_name);
    }

    void ClusterManager::addCachedNtFile(const std::vector<TripleInfo>& triples, const std::string& db_name, const std::string file_name)
    {
        if (!isEnable() || !role_)
            return;
        role_->addCachedNtFile(triples, db_name, file_name);
    }

    void ClusterManager::appendCachedNtData(const std::vector<TripleInfo>& triples, const std::string& db_name, const std::string file_name)
    {
        if (!isEnable() || !role_)
            return;
        role_->appendCachedNtData(triples, db_name, file_name);
    }

    std::string ClusterManager::saveFromFollowerFile(const std::pair<std::string, std::string>& file_info, const std::string& db_name)
    {
        if (!isEnable() || !role_)
            return std::string();
        std::string file_path = ClusterDb::getClusterDir() + db_name + file_info.first;
        ofstream fout(file_path.c_str());
        if (!fout)
        {
            SLOG_ERROR("file open fail");
            return std::string();
        }
        fout << file_info.second;
        fout.close();
        return file_path;
    }

    void ClusterManager::getNtFileData(std::vector<TripleInfo>& triples, const std::string& db_name, const std::string& file_name)
    {
        if (!isEnable() || !role_)
            return;
        if (file_name.empty())
            return;
        
        role_->getNtFileData(triples, db_name, file_name);
    }
}