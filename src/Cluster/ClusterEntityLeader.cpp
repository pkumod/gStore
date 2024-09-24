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

        heartbeat_ = std::atoi(Util::getConfigureValue("cluster_heartbeat").c_str());
        relpy_timeout_ = std::atoi(Util::getConfigureValue("cluster_relpy_timeout").c_str())*1000;
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

    void ClusterEntityLeader::startHeardBeat(std::string db_name)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return;
        TimerProvider timer;
        head_beat_timerL_.insert(make_pair(db_name, timer));
        head_beat_timerL_[db_name].StartTimer(heartbeat_, [this, db_name]()
        {
            this->postHeartBeat(db_name);
        });
    }

    void ClusterEntityLeader::stopHeardBeatTimer(std::string db_name)
    {
        auto it = head_beat_timerL_.find(db_name);
        if (it == head_beat_timerL_.end())
            return;
        it->second.Expire();
        head_beat_timerL_.erase(db_name);
    }

    void ClusterEntityLeader::postHeartBeat(std::string db_name)
    {
        uint32 term = getTerm();
        ClusterDbPtr db = findDb(db_name);
        if (!db)
        {
            stopHeardBeatTimer(db_name);
            return;
        }
        uint64 index = getDbIndex(db_name);
        std::string expection = "compare";
        httpentities::HeartBeatRequest request(term, db_name, index, expection);
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
            auto it = faileL_.find(url);
            if (it == faileL_.end())
                continue;
            if (it->second > headBeat_max_fail_num_)
                continue;
            
            thread postHearBeat(helper, node.second);
            postHearBeat.detach();
        }
    }

    void ClusterEntityLeader::postAppendTask(std::string db_name, uint64 index, ClusterOperation operation, std::string file_name)
    {
        uint32 term = getTerm();
        std::string operation_str = to_string(operation);
        httpentities::AppenEntriesRequest request(term, db_name, index, operation_str, file_name);
        auto helper = [this, db_name, index, request](ClusterNode node)
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
            auto it = faileL_.find(url);
            if (it == faileL_.end())
                continue;
            if (it->second > headBeat_max_fail_num_)
                continue;
            
            thread postsync(helper, node.second);
            postsync.detach();
        }
    }

    bool ClusterEntityLeader::runAppendTask(std::string db_name, ClusterOperation operation, const std::string& file_name)
    {
        uint32 term = getTerm();
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return false;
        uint64 index = getDbNextIndex(db_name);
        if (index == 0)
        {
            SLOG_TRACE("start sync fail, please check term.json, index:" << index);
            return false;
        }
        std::string current_path = getDbDirPath(db_name) + file_name;
        std::string zip_path = current_path + ".zip";
        if (!CompressUtil::FileHelper::compressExportZip(current_path, zip_path, false))
        {
            SLOG_ERROR("compress fail");
            Util::remove_path(zip_path);
            return false;
        }

        std::string file_path = Util::getExactPath(zip_path.c_str());
        updateLogStatus(db_name, index, ClusterLogStatus_sync);
        postAppendTask(db_name, index, operation, file_path);
        uint64 end_time = Util::get_cur_time() + getAppendTimeout(db_name, file_name);
        return waitTimerPassNum(db_name, index, ClusterLogStatus_sync, end_time);
    }

    void ClusterEntityLeader::postTask(std::string db_name, uint64 index, std::string expection, ClusterLogStatus status)
    {
        uint32 term = getTerm();
        updateLogStatus(db_name, index, status);
        httpentities::HeartBeatRequest request(term, db_name, index, expection);
        auto helper = [this, db_name, index, request](ClusterNode node)
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
            auto it = faileL_.find(url);
            if (it == faileL_.end())
                continue;
            if (it->second > headBeat_max_fail_num_)
                continue;
            
            thread postHearBeat(helper, node.second);
            postHearBeat.detach();
        }
    }

    bool ClusterEntityLeader::waitTimerPassNum(std::string db_name, uint64 index, ClusterLogStatus status, uint64 end_time)
    {
        TimerProvider oneTimer;
        int once_run = 1000;
        uint32 pass_num = 0;
        int need_num = getNeedNum();
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
            if (status == ClusterLogStatus_pending)
            {
                oneTimer.SyncWait(once_run, [this, &pass_num, db_name, index]
                {
                    pass_num = this->getLogReplyNum(db_name, index);
                });
            }
            else if(status == ClusterLogStatus_sync)
            {
                oneTimer.SyncWait(once_run, [this, &pass_num, db_name, index]
                {
                    pass_num = this->getLogSyncNum(db_name, index);
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
        SLOG_TRACE("db name:" << db_name << " ,status:" << status << "callback pass num:" << pass_num << "  ,need num:" << need_num);
        if (pass_num != 0 && need_num != 0 && pass_num >= need_num)
        {
            return true;
        }
        return false;
    }

    bool ClusterEntityLeader::runTask(std::string db_name, ClusterLogStatus status)
    {
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return false;
        uint64 index = getDbNextIndex(db_name);
        if (index == 0)
        {
            SLOG_TRACE("start task status " << status << " fail, please check term.json, index:" << index);
            return false;
        }
        if (status == ClusterLogStatus_commit)
        {
            postTask(db_name, index, "commit", status);
        }
        else if (status == ClusterLogStatus_cancel)
        {
            postTask(db_name, index, "cancel", status);
        }
        else if (status == ClusterLogStatus_fail)
        {
            postTask(db_name, index, "fail", status);
        }
        else if (status == ClusterLogStatus_drop)
        {
            uint64 end_time = Util::get_cur_time() + relpy_timeout_;
            postTask(db_name, index, "prepare", status);
            return waitTimerPassNum(db_name, index, status, end_time);
        }
        else if (status == ClusterLogStatus_pending)
        {
            uint64 end_time = Util::get_cur_time() + relpy_timeout_;
            postTask(db_name, index, "prepare", status);
            return waitTimerPassNum(db_name, index, status, end_time);
        }
        else
        {
            SLOG_ERROR("not support task db name:" << db_name << " ,status" << status);
            return false;
        }

        return true;
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
        std::string file_path = getDbDirPath(db_name) + file_name;
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