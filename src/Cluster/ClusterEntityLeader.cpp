#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"
#include "../Util/CompressFileUtil.h"

namespace cluster
{
    uint32 ClusterEntityLeader::headBeat_max_fail_num_ = 5;
    void ClusterEntityLeader::init()
    {
        getTerm();
        string cluster_node = Util::getConfigureValue("cluster_node");
        std::vector<std::string> nodes;
		Util::split(cluster_node, ",", nodes);
        for (const auto m : nodes)
        {
            std::vector<std::string> node;
            Util::split(m, ":", node);
            if (node.size() < 4)
                continue;
            ClusterNode follow(node[0], node[1], node[2], node[3]);
            std::string url = follow.getBaseUrl();
            followNodeL_[url] = follow;
            faileL_[url] = 0;
        }

        heartbeat_ = std::atoi(Util::getConfigureValue("cluster_heartbeat").c_str())*1000;
        startCompare();
    }

    ClusterNode ClusterEntityLeader::FindFollower(const std::string& ip, const std::string& port)const
    {
        ClusterNode node(ip, port, "", "");
        auto it = followNodeL_.find(node.getBaseUrl());
        if (it == followNodeL_.end())
            return ClusterNode();
        return it->second;
    }

    bool ClusterEntityLeader::IsFollowerIp(const std::string& ip)const
    {
        for (const auto& m : followNodeL_)
        {
            if (m.second.getIp() == ip)
                return true;
        }
        return false;
    }

    void ClusterEntityLeader::startCompare()
    {
        head_beat_timer_.StartTimer(heartbeat_, [this]
        {
            ClusterTermInfo info;
            if (!getTermInfoDbLogs(info))
            {
                SLOG_TRACE("heart beat fail compare");
                return;
            }
            setInlineTerm(info.getTerm());
            std::string expection = "compare";
            for (const auto& m : info.Logs())
            {
                std::string db_name = m.first;
                uint64 index = m.second.getIndex();
                uint64 nextIndex = m.second.getNextIndex();
                uint64 uid = m.second.getUid();
                for (const auto& node : followNodeL_)
                {
                    httpentities::HeartBeatRequest request(info.getTerm(), db_name, index, nextIndex, uid, expection);
                    request.setFollowIp(node.second.getIp());
                    HttpUtil::heartBeat(node.second.getHeartBeatUrl(), request, node.second.getUsername(), node.second.getPassword());
                }
            }
        });
    }

    void ClusterEntityLeader::stopCompareTimer()
    {
        head_beat_timer_.Expire();
    }

    void ClusterEntityLeader::postAppendTask(const ClusterTaskInfo& info, const std::string& file_path)
    {
        uint32 term = getTerm();
        std::string update_type = to_string(info.update_type);
        httpentities::AppenEntriesRequest request(term, info.db_name, info.index, info.nextIndex, info.uid, update_type, file_path);
        auto helper = [this, request](ClusterNode node)
        {
            httpentities::AppenEntriesRequest request_ = request;
		    httpentities::ClusterResponse responce = HttpUtil::appendEntries(node.getAppendEntriesUrl(), request_, node.getUsername(), node.getPassword());
            std::lock_guard<std::mutex> lock(fail_ip_mutex_);
            if (responce.getStatusCode() != CURLE_OK)
            {
                faileL_[node.getBaseUrl()] += 1;
                return;
            }
            faileL_[node.getBaseUrl()] = 0;
        };

        for (const auto& node : followNodeL_)
        {
            std::string url = node.second.getBaseUrl();
            thread postsync(helper, node.second);
            postsync.detach();
        }
    }

    bool ClusterEntityLeader::runAppendTask(const ClusterTaskInfo& info)
    {
        uint32 term = getTerm();
        ClusterDbPtr db = findDb(info.db_name);
        if (!db)
            return false;
        if (info.nextIndex == 0 || info.file_name.empty())
        {
            SLOG_TRACE("start sync fail, please check term.json, index:" << info.index << " ,file name:" << info.file_name);
            return false;
        }
        std::string current_path = ClusterDb::getDbDirPath(info.db_name) + info.file_name;
        std::string zip_path = current_path + ".zip";
        if (!CompressUtil::FileHelper::compressExportZip(current_path, zip_path, false))
        {
            SLOG_ERROR("compress fail");
            Util::remove_path(zip_path);
            return false;
        }

        std::string file_path = Util::getExactPath(zip_path.c_str());
        updateLogOperation(info.db_name, info.nextIndex, ClusterOperation_Append);
        setLogUpdateType(info.db_name, info.nextIndex, info.update_type);
        setLogFileName(info.db_name, info.nextIndex, info.file_name);
        postAppendTask(info, file_path);
        return true;
    }

    bool ClusterEntityLeader::runRestoreTask(const ClusterRecoverInfo& info)
    {
        uint32 term = getTerm();
        ClusterDbPtr db = findDb(info.db_name);
        if (!db)
            return false;
        if (isSendFollowerRestoring(info.ip, info.db_name))
        {
            SLOG_ERROR("follower data is sending, please waiting" << info.db_name);
            return false;
        }
        TermDbLog db_info = getTermInfoDbLog(info.db_name);
        if (db_info.empty() || db_info.getFirstIndex() == 0)
        {
            SLOG_ERROR("recover file is empty:" << info.db_name);
            return false;
        }
        std::string file_name = db->getFileName(info.index);
        if (file_name.empty())
        {
            SLOG_TRACE("file is not exist index:" << info.index << " ,file name:" << info.index << ".log" << " ,db name:" << info.db_name);
            return false;
        }
        addRestoreDb(info.ip, info.db_name);
        std::string current_path = ClusterDb::getDbDirPath(info.db_name) + file_name;
        std::string zip_path = current_path + ".zip";
        if (!Util::file_exist(zip_path))
        {
            if (!CompressUtil::FileHelper::compressExportZip(current_path, zip_path, false))
            {
                SLOG_ERROR("compress fail");
                Util::remove_path(zip_path);
                removeRestoreDb(info.ip, info.db_name);
                return false;
            }
        }
        std::string update_type = std::to_string(db->getUpdateType(info.index));
        std::string file_path = Util::getExactPath(zip_path.c_str());
        httpentities::RecoverRequest request(term, db_info.dbName, db_info.index, db_info.nextIndex, db_info.uid, update_type, file_path, info.index);
        ClusterNode node = FindFollower(info.ip, info.port);
        HttpUtil::recoverFollower(node.getRecoverlUrl(), request, node.getUsername(), node.getPassword());
        removeRestoreDb(info.ip, info.db_name);
        return true;
    }

    void ClusterEntityLeader::postTask(const ClusterTaskInfo& info)
    {
        uint32 term = getTerm();
        if (info.operation != ClusterOperation_Drop)
        {
            updateLogOperation(info.db_name, info.nextIndex, info.operation);
        }

        std::string expection = ClusterOperationHandle::to_str(info.operation);
        httpentities::HeartBeatRequest request(term, info.db_name, info.index, info.nextIndex, info.uid, expection);
        auto helper = [this, request](ClusterNode node)
        {
            httpentities::HeartBeatRequest request_ = request;
		    httpentities::ClusterResponse responce = HttpUtil::heartBeat(node.getHeartBeatUrl(), request_, node.getUsername(), node.getPassword());
            std::lock_guard<std::mutex> lock(fail_ip_mutex_);
            if (responce.getStatusCode() != CURLE_OK)
            {
                faileL_[node.getBaseUrl()] += 1;
                return;
            }
            faileL_[node.getBaseUrl()] = 0;
        };

        for (const auto& node : followNodeL_)
        {
            std::string url = node.second.getBaseUrl();
            thread postHearBeat(helper, node.second);
            postHearBeat.detach();
        }
    }

    bool ClusterEntityLeader::waitTimerPassNum(std::string db_name, uint64 index, ClusterOperation operation, uint64 end_time)
    {
        TimerProvider oneTimer;
        int once_run = 200;
        uint32 pass_num = 0;
        int need_num = getNeedNum();
        std::string waiting = ".";
        SLOG_DEBUG("waiting query task callback need num:" << need_num);
        while (1)
        {
            uint64 current_time = Util::get_cur_time();
            if (end_time > current_time )
            {
                if (once_run > (end_time - current_time))
                    once_run = end_time - current_time;
            }
            else
            {
                break;
            }
            if (operation == ClusterOperation_Prepare)
            {
                oneTimer.SyncWait(once_run, [this, &pass_num, db_name, index, &waiting]
                {
                    pass_num = this->getLogReplyNum(db_name, index);
                    SLOG_DEBUG("waiting prepare task callback pass num:" << pass_num << waiting);
			        waiting.append(".");
                });
            }
            else if(operation == ClusterOperation_Append)
            {
                oneTimer.SyncWait(once_run, [this, &pass_num, db_name, index, &waiting]
                {
                    pass_num = this->getLogSyncNum(db_name, index);
                    SLOG_DEBUG("waiting query append task callback pass num:" << pass_num << waiting);
                    waiting.append(".");
                });
            }
            else
            {
                break;
            }

            if (pass_num >= need_num)
                break;
            if (Util::get_cur_time() >= end_time)
                break;
        }
        SLOG_TRACE("db name:" << db_name << " ,operation:" << operation << "callback pass num:" << pass_num << "  ,need num:" << need_num);
        if (pass_num != 0 && need_num != 0 && pass_num >= need_num)
        {
            return true;
        }
        return false;
    }

    bool ClusterEntityLeader::runTask(const ClusterTaskInfo& info)
    {
        ClusterOperation operation = info.operation;
        if (operation != ClusterOperation_Drop)
        {
            ClusterDbPtr db = findDb(info.db_name);
            if (!db)
                return false;
            if (info.nextIndex == 0)
            {
                SLOG_TRACE("start task status " << operation << " fail, please check term.json, index:" << info.index);
                return false;
            }    
        }

        if (operation == ClusterOperation_Commit
         || operation == ClusterOperation_Cancel
         || operation == ClusterOperation_Fail
         || operation == ClusterOperation_Drop
         || operation == ClusterOperation_Prepare)
        {
            postTask(info);
        }
        else if (operation == ClusterOperation_Append)
        {
            runAppendTask(info);
        }
        else
        {
            SLOG_ERROR("not support task db name:" << info.db_name << " ,operation" << operation);
            return false;
        }

        return true;
    }

    uint64 ClusterEntityLeader::getTimeOutEndTime(const std::string& db_name, const std::string& file_name)
    {
        if (!db_name.empty() && !file_name.empty())
        {
            return Util::get_cur_time() + getAppendTimeout(db_name, file_name);
        }
        
        return Util::get_cur_time() + std::atoi(Util::getConfigureValue("cluster_relpy_timeout").c_str())*1000;
    }

    std::vector<std::string> ClusterEntityLeader::getFollowrUrlArray()const
    {
        std::vector<std::string> urlL;
        std::string url;
        for (const auto&m : followNodeL_)
        {
            urlL.push_back(m.second.getBaseUrl());
            url.clear();
        }
        return urlL;
    }

    std::vector<ClusterNode> ClusterEntityLeader::getFollowNodeL()const
    {
        std::vector<ClusterNode> nodeL;
        std::string url;
        for (const auto&m : followNodeL_)
        {
            nodeL.push_back(m.second);
        }
        return nodeL;
    }

    uint32 ClusterEntityLeader::getAppendTimeout(const std::string& db_name, const std::string& file_name)
    {
        // min time is 1 minute
        // triple_num 1000000 is 1 second
        // file size 100m is 1 second
        std::string file_path = ClusterDb::getDbDirPath(db_name) + file_name;
        if (!Util::file_exist(file_path))
        {
            SLOG_ERROR("file not exits:" << db_name << " ,file name:" << file_name);
            return 60000;
        }
        size_t triple_num = Util::count_lines(file_path);
        long long unsigned size_byte = Util::getFileSize(file_path);
        uint32 tripe_time = triple_num/1000000;
        uint32 disk_m = size_byte>>20;
        uint32 time_out = (triple_num/1000000 + disk_m/100) * 1000 * 1.5;

        SLOG_TRACE("file num size:" << triple_num << "  ,size:" << size_byte << "  ,time_out:" << time_out);

        if (time_out < 60000)
            return 60000;
        return time_out*1.5;
    }

    void ClusterEntityLeader::addRestoreDb(const std::string& ip, const std::string& db_name)
    {
        auto it = restoreDbL_.find(ip);
        if (it == restoreDbL_.end())
        {
            std::set<std::string> dbL;
            dbL.insert(db_name);
            restoreDbL_.insert(std::make_pair(ip, dbL));
        }
        else
        {
            it->second.insert(db_name);
        }
    }

    void ClusterEntityLeader::removeRestoreDb(const std::string& ip, const std::string& db_name)
    {
        auto it = restoreDbL_.find(ip);
        if (it == restoreDbL_.end())
        {
            return;
        }
        else
        {
            it->second.erase(db_name);
        }
    }

    bool ClusterEntityLeader::isSendFollowerRestoring(const std::string& ip, const std::string& db_name)
    {
        auto it = restoreDbL_.find(ip);
        if (it != restoreDbL_.end())
        {
            if (it->second.find(db_name) != it->second.end())
                return false;
        }
        return false;
    }

    bool ClusterEntityLeader::tryRecover(const std::vector<std::string>& dbs)
    {
        auto helper = [this]()
        {
            // 处理恢复逻辑
        };
        std::map<std::string, uint32> fail_ip_L;
        for (const auto& m : faileL_)
        {
            if (m.second > headBeat_max_fail_num_)
                continue;
            fail_ip_L.insert(std::make_pair(m.first, m.second));
        }
        for (const auto& m : dbs)
        {
            if (findDb(m))
                continue;
            ClusterDbPtr db = addClusterDb(m);
            if (!db)
                continue;

            // 处理恢复逻辑
            // thread postTryRecover(helper);
            // postTryRecover.detach();
        }

          return true;   
    }
}