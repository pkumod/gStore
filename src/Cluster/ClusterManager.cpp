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

    void ClusterManager::setFollowIp(const std::string& ip)
    {
        if (!isEnable() || !role_ || ip.empty())
            return;
        ClusterEntityFollowerPtr follower = std::dynamic_pointer_cast<ClusterEntityFollower>(role_);
        if (!follower)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return;
        }

        follower->setIp(ip);
    }

    std::string ClusterManager::getFollowIp()
    {
        if (!isEnable() || !role_)
            return "";
        ClusterEntityFollowerPtr follower = std::dynamic_pointer_cast<ClusterEntityFollower>(role_);
        if (!follower)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return "";
        }

        return follower->getIp();
    }

    void ClusterManager::addRestoreDb(const std::string& db_name)
    {
        if (!isEnable() || !role_)
            return;
        ClusterEntityFollowerPtr follower = std::dynamic_pointer_cast<ClusterEntityFollower>(role_);
        if (!follower)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return;
        }

        follower->addRestoreDb(db_name);
    }

    void ClusterManager::removeRestoreDb(const std::string& db_name)
    {
        if (!isEnable() || !role_)
            return;
        ClusterEntityFollowerPtr follower = std::dynamic_pointer_cast<ClusterEntityFollower>(role_);
        if (!follower)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return;
        }

        follower->removeRestoreDb(db_name);
    }

    bool ClusterManager::isFollowerRestoring(const std::string& db_name)
    {
        if (!isEnable() || !role_)
            return false;
        ClusterEntityFollowerPtr follower = std::dynamic_pointer_cast<ClusterEntityFollower>(role_);
        if (!follower)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return false;
        }

        return follower->isFollowerRestoring(db_name);
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
        leader->startCompare();
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
        ClusterTaskInfo info(db_name, ClusterOperation_Prepare);
        TermDbLog db_log = role_->getTermInfoDbLog(db_name);
        info.setIndex(db_log.getIndex());
        info.setNextIndex(db_log.getNextIndex());
        info.setUid(db_log.getUid());
        return leader->runTask(info);
    }

    void ClusterManager::addClusterDb(const std::string& db_name)
    {
        if (!isEnable() || !role_)
            return;
        role_->addClusterDb(db_name);
    }

    bool ClusterManager::startSync(std::string db_name, ClusterUpdateType update_type, const std::string& file_name)
    {
        if (!isEnable() || !role_)
            return false;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return false;
        }

        ClusterTaskInfo info(db_name, ClusterOperation_Append, update_type, file_name);
        TermDbLog db_log = role_->getTermInfoDbLog(db_name);
        info.setIndex(db_log.getIndex());
        info.setNextIndex(db_log.getNextIndex());
        info.setUid(db_log.getUid());

        return leader->runAppendTask(info);
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

    void ClusterManager::addLog(std::string db_name, uint64 index, ClusterOperation operation, ClusterUpdateType update_type)
    {
        if (!isEnable() || !role_)
            return;

        role_->addLog(db_name, index, operation, update_type);
    }

    void ClusterManager::addCommitLog(std::string db_name, uint64 index, ClusterUpdateType update_type, const std::string& file_name)
    {
        if (!isEnable() || !role_)
            return;

        role_->addCommitLog(db_name, index, update_type, file_name);
    }

    void ClusterManager::buildDb(std::string db_name, uint64 uid)
    {
        if (!isEnable() || !role_)
            return;

        role_->buildDb(db_name, uid);
    }

    void ClusterManager::updateLogOperation(std::string db_name, uint64 index, ClusterOperation operation)
    {
        if (!isEnable() || !role_)
            return;
        role_->updateLogOperation(db_name, index, operation);
    }

    void ClusterManager::updateLogInfo(std::string db_name, uint64 index, ClusterOperation operation, ClusterUpdateType update_type, std::string file_name)
    {
        if (!isEnable() || !role_)
            return;
        role_->updateLogOperation(db_name, index, operation, update_type, file_name);
    }

    void ClusterManager::setLogUpdateType(std::string db_name, uint64 index, ClusterUpdateType operation)
    {
        if (!isEnable() || !role_)
            return;
        role_->setLogUpdateType(db_name, index, operation);
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

    bool ClusterManager::enabelAttain(std::string db_name, uint64 index, ClusterOperation status)
    {
        if (!isEnable() || !role_)
            return false;

        uint32 num = 0;
        if (status == ClusterOperation_Prepare)
        {
            num = role_->getLogReplyNum(db_name, index);
        }
        else if (status == ClusterOperation_Append)
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

    ClusterOperation ClusterManager::getDbLogOperation(const std::string& db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return ClusterOperation_None;
        return role_->getDbLogOperation(db_name, index);
    }

    ClusterUpdateType ClusterManager::getDbLogUpdateType(const std::string& db_name, uint64 index)
    {
        if (!isEnable() || !role_)
            return ClusterUpdateType_None;
        return role_->getDbLogUpdateType(db_name, index);
    }

    std::string ClusterManager::getDbDirPath(std::string db_name)
    {
        return ClusterDb::getDbDirPath(db_name);
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

    uint64 ClusterManager::getDbNextIndexByIndex(const std::string& db_name, uint64 follower_index)
    {
        if (!isEnable() || !role_)
            return 0;
        if (db_name.empty())
            return 0;
        return role_->getDbNextIndexByindex(db_name, follower_index);
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

    bool ClusterManager::IsSupportTask(ClusterOperation status)
    {
        if (status == ClusterOperation_HeartBeat
         || status == ClusterOperation_Commit
         || status == ClusterOperation_Cancel
         || status == ClusterOperation_Fail
         || status == ClusterOperation_Drop
         || status == ClusterOperation_Append
         || status == ClusterOperation_Prepare
         || status == ClusterOperation_Build
         || status == ClusterOperation_Compare)
        {
            return true;
        }
        return false;
    }

    bool ClusterManager::IsSupportSync(ClusterOperation status)
    {
        if (status == ClusterOperation_Append
         || status == ClusterOperation_Prepare
         || status == ClusterOperation_Drop
         || status == ClusterOperation_Build)
        {
            return true;
        }
        return false;
    }

    void ClusterManager::dropDb(const std::string& db_name)
    {
        if (!isEnable() || !role_)
            return;
        role_->dropDb(db_name);
    }

    TermDbLog ClusterManager::getTermInfoDbLog(const std::string& db_name)
    {
        if (!isEnable() || !role_)
            return TermDbLog();
        return role_->getTermInfoDbLog(db_name);
    }

    void ClusterManager::stopHeart()
    {
        if (!isEnable() || !role_)
            return;
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (leader)
            leader->stopCompareTimer();
    }

    bool ClusterManager::addTask(ClusterTaskInfo info, bool sync)
    {
        if (!isEnable() || !role_)
            return false;
        if (info.db_name.empty())
        {
            SLOG_TRACE("please task db name is empty");
            return false;
        }
        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return false;
        }
        TermDbLog db_log = getTermInfoDbLog(info.db_name);
        if (db_log.empty())
        {
            SLOG_TRACE("db log is empty db name:" << info.db_name);
            return false;
        }
        // index = 0, empty db
        if (info.operation != ClusterOperation_Drop && db_log.getNextIndex() == 0)
        {
            SLOG_TRACE("start task opeartion " << info.operation << " fail, please check term.json, nextindex:" << info.index);
            return false;
        }

        info.setIndex(db_log.getIndex());
        info.setNextIndex(db_log.getNextIndex());
        info.setUid(db_log.getUid());

        if (info.operation == ClusterOperation_Append)
        {
            if (info.update_type == ClusterUpdateType_None || info.file_name.empty())
            {
                SLOG_TRACE("please sync task parm operation:" << info.operation << " ,file name:" << info.file_name);
                return false;
            }
        }

        if (!sync && IsSupportTask(info.operation))
        {
            ClusterEventPtr task = std::make_shared<ClusterTaskEvent>(info, leader);
            task_queueL.push(task);
        }
        else if (sync && IsSupportSync(info.operation))
        {
            ClusterEventPtr task = std::make_shared<ClusterTaskEvent>(info, leader);
            task_queueL.push(task);
            uint64 end_time = leader->getTimeOutEndTime(info.db_name, info.file_name);
            end_time = leader->getTimeOutEndTime();
            return leader->waitTimerPassNum(info.db_name, info.nextIndex, info.operation, end_time);
        }
        else
        {
            SLOG_TRACE("not support task status " << info.operation << " ,db name:" << info.db_name << " fail, please check update.json, index:" << info.nextIndex);
        }
        
        return false;
    }

    bool ClusterManager::addTask(ClusterRecoverInfo info)
    {
        if (!isEnable() || !role_)
            return false;
        if (info.db_name.empty())
        {
            SLOG_TRACE("please task db name is empty");
            return false;
        }
        if (info.index == 0)
        {
            SLOG_TRACE("please task index is 0");
            return false;
        }

        ClusterEntityLeaderPtr leader = std::dynamic_pointer_cast<ClusterEntityLeader>(role_);
        if (!leader)
        {
            SLOG_TRACE("please check conf.ini, not set leader");
            return false;
        }
        
        ClusterEventPtr task = std::make_shared<ClusterRecoverTaskEvent>(info, leader);
        task_queueL.push(task);
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