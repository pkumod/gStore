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
            else if (cluster_role == "follower")
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
            std::thread run_task = std::thread(&ClusterManager::runTask, this);
            run_task.detach();

            SLOG_CORE("cluster success on");
        }
    }

    void ClusterManager::refresh()
    {
        bool latst_on = on_;
        if (on_ && role_)
        {
            return;
        }
        
        if (Util::getConfigureValue("cluster_on") == "on")
            on_ = true;
        else
            on_ = false;

        if (latst_on && !on_ && role_)
        {
            role_.reset();
            role_ = nullptr;
            return;
        }

        if (on_ && !role_)
        {
            string cluster_role = Util::getConfigureValue("cluster_role");
            if (cluster_role == "leader")
            {
                role_ = std::make_shared<ClusterEntityLeader>();
            }
            else if (cluster_role == "follower")
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
            std::thread run_task = std::thread(&ClusterManager::runTask, this);
            run_task.detach();

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

    bool ClusterManager::isFollower()
    {
        if (!isEnable() || !role_)
            return false;
        return getCluterRole() == cluster::ClusterRoleType_Follow;
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

        return leader->tryRecover(dbs);
    }

    void ClusterManager::startHeartBeat(const std::string& db_name)
    {
        if (!isEnable() || !role_)
            return;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return;
        }
        leader->startHeardBeat(db_name);
    }

    bool ClusterManager::startNotify(std::string db_name)
    {
        if (!isEnable() || !role_)
            return false;
        role_->addClusterDb(db_name);
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return false;
        }
        return leader->runTask(db_name, ClusterLogStatus_pending);
    }

    void ClusterManager::addClusterDb(const std::string& db_name)
    {
        if (!isEnable() || !role_)
            return;
        role_->addClusterDb(db_name);
    }

    bool ClusterManager::startSync(std::string db_name, ClusterOperation operation, const std::string& file_name)
    {
        if (!isEnable() || !role_)
            return false;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return false;
        }
        return leader->runAppendTask(db_name, operation, file_name);
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

    bool ClusterManager::fromFollower(const std::string& ip, const std::string& port)
    {
        if (!isEnable() || !role_ || ip.empty())
            return false;
        
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
            return false;
        ClusterNode node = leader->FindFollower(ip, port);
        if (!node.empty() && node.getIp() == ip)
            return true;
        return false;
    }

    bool ClusterManager::fromFollowerIp(const std::string& ip)
    {
        if (!isEnable() || !role_ || ip.empty())
            return false;
        
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
            return false;
        return leader->IsFollowerIp(ip);
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

    void ClusterManager::addLog(std::string db_name, uint64 index, ClusterLogStatus status, ClusterOperation operation)
    {
        if (!isEnable() || !role_)
            return;

        role_->addLog(db_name, index, status, operation);
    }

    void ClusterManager::updateLogStatus(std::string db_name, uint64 index, ClusterLogStatus status)
    {
        if (!isEnable() || !role_)
            return;
        role_->updateLogStatus(db_name, index, status);
    }

    void ClusterManager::setLogOperation(std::string db_name, uint64 index, ClusterOperation operation)
    {
        if (!isEnable() || !role_)
            return;
        role_->setLogOperation(db_name, index, operation);
    }

    void ClusterManager::setLogFileName(std::string db_name, uint64 index, std::string file_name)
    {
        if (!isEnable() || !role_)
            return;
        role_->setLogFileName(db_name, index, file_name);
    }

    void ClusterManager::addLogReplyNum(std::string db_name, uint64 index, const std::string& ip, const std::string& port)
    {
        if (!isEnable() || !role_)
            return;
        role_->addLogReplyNum(db_name, index, ip, port);
    }

    void ClusterManager::addLogSyncNum(std::string db_name, uint64 index, const std::string& ip, const std::string& port)
    {
        if (!isEnable() || !role_)
            return;
        role_->addLogSyncNum(db_name, index, ip, port);
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

    bool ClusterManager::enabelAttain(std::string db_name, uint64 index, ClusterLogStatus status)
    {
        if (!isEnable() || !role_)
            return false;

        uint32 num = 0;
        if (status == ClusterLogStatus_pending)
        {
            num = role_->getLogReplyNum(db_name, index);
        }
        else if (status == ClusterLogStatus_sync)
        {
            num = role_->getLogSyncNum(db_name, index);
        }
        uint32 total = getFollowNodeL().size();
        if (total == 0)
        {
            SLOG_TRACE("follow node is 0");
            return false;
        }
        return num >= ((total + 1)/2) ? true : false;
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

    void ClusterManager::updateDbNextIndex(std::string db_name, uint64 next_index)
    {
        if (!isEnable() || !role_)
            return;
        role_->updateDbNextIndex(db_name, next_index);
    }

    uint32 ClusterManager::getTerm()
    {
        if (!isEnable() || !role_)
            return INVALID;
        return role_->getTerm();
    }

    uint64 ClusterManager::getDbIndex(std::string db_name)
    {
        if (!isEnable() || !role_)
            return 0;
        return role_->getDbIndex(db_name);
    }

    uint64 ClusterManager::getDbNextIndex(std::string db_name)
    {
        if (!isEnable() || !role_)
            return 0;
        return role_->getDbNextIndex(db_name);
    }

    std::string ClusterManager::getNTFilePathByIndex(const std::string& db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return "";
        return role_->getNTFilePathByIndex(db_name, index);
    }

    ClusterLogStatus ClusterManager::getDbLogStatus(const std::string& db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return ClusterLogStatus_None;
        return role_->getDbLogStatus(db_name, index);
    }

    ClusterOperation ClusterManager::getDbLogOperation(const std::string& db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return ClusterOperation_None;
        return role_->getDbLogOperation(db_name, index);
    }

    std::string ClusterManager::getDbDirPath(std::string db_name)
    {
        if (!isEnable() || !role_)
            return "";
        return role_->getDbDirPath(db_name);
    }

    void ClusterManager::getDbNextIndexL(const std::string& db_name, uint64 index, std::vector<uint64StringPair>& indexl)
    {
        if (!isEnable() || !role_)
            return;
        if (db_name.empty() || index == 0)
            return;
        return role_->getDbNextIndexL(db_name, index, indexl);
    }

    uint64 ClusterManager::getDbFirstIndex(const std::string& db_name)
    {
        if (!isEnable() || !role_)
            return 0;
        if (db_name.empty())
            return 0;
        return role_->getFirstIndex(db_name);
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

    std::string ClusterManager::getNtFilePath(const std::string& db_name, const std::string& file_name)
    {
        if (!isEnable() || !role_)
            return std::string();
        if (file_name.empty())
            return std::string();
        
        return role_->getNtFilePath(db_name, file_name);
    }

    uint32 ClusterManager::getAppendTimeout(const std::string& db_name, const std::string& file_name)
    {
        if (!isEnable() || !role_)
            return 60000000;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return 60000000;
        }
        return leader->getAppendTimeout(db_name, file_name)*1000;
    }

    bool ClusterManager::IsHeartBeatTask(ClusterLogStatus status)
    {
        if (status == ClusterLogStatus_HeartBeat
         || status == ClusterLogStatus_pending
         || status == ClusterLogStatus_commit
         || status == ClusterLogStatus_cancel
         || status == ClusterLogStatus_fail
         || status == ClusterLogStatus_drop)
        {
            return true;
        }
        return false;
    }

    bool ClusterManager::IsUpdateTask(ClusterLogStatus status)
    {
        if (status == ClusterLogStatus_sync
         || status == ClusterLogStatus_build)
        {
            return true;
        }
        return false;
    }

    bool ClusterManager::addTask(std::string db_name, ClusterLogStatus status, const timeoutCall& cb, ClusterOperation operation, const std::string& file_name)
    {
        if (!isEnable() || !role_)
            return false;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return false;
        }
        if (IsHeartBeatTask(status))
        {
            ClusterEventPtr task = std::make_shared<ClusterHeartBeatEvent>(db_name, leader, cb, status);
            task_queueL.push(task);
        }
        else if (IsUpdateTask(status))
        {
            if (cb == nullptr)
            {
                SLOG_TRACE("Please sure cluster cluster update is nullptr");
            }
            ClusterEventPtr task = std::make_shared<ClusterAppendEvent>(db_name, operation, file_name, leader, cb);
            task_queueL.push(task);
        }
        else
        {
            SLOG_ERROR("not support task status:" << status);
            return false;
        }
        return true;
    }

    void ClusterManager::runTask()
    {
        if (!isEnable() || !role_)
            return;
        SLOG_TRACE("cluster task is run");
        while(1)
        {
            task_queueL.pop()->runEvent();
        }
    }
}