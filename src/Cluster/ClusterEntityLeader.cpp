#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"

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
            followNodeL_[node[0]] = follow;
            faileL_[node[0]] = 0;
        }

        heartbeat_ = std::atoi(Util::getConfigureValue("cluster_heartbeat").c_str());
        relpy_timeout_ = std::atoi(Util::getConfigureValue("cluster_relpy_timeout").c_str())*1000;
        sync_timeout_ = std::atoi(Util::getConfigureValue("cluster_sync_timeout").c_str())*1000;
    }

    ClusterNode ClusterEntityLeader::FindFollower(const std::string& ip)const
    {
        auto it = followNodeL_.find(ip);
        if (it == followNodeL_.end())
            return ClusterNode();
        return it->second;
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
                faileL_[node.getIp()] += 1;
                return;
            }
            faileL_[node.getIp()] = 0;
        };

        for (const auto& node : followNodeL_)
        {
            std::string ip = node.second.getIp();
            auto it = faileL_.find(ip);
            if (it == faileL_.end())
                continue;
            if (it->second > headBeat_max_fail_num_)
                continue;
            
            thread postHearBeat(helper, node.second);
            postHearBeat.detach();
        }
    }

    void ClusterEntityLeader::postNotify(std::string db_name, uint64 index)
    {
        uint32 term = getTerm();
        std::string expection = "prepare";
        httpentities::HeartBeatRequest request(term, db_name, index, expection);
        auto helper = [this, db_name, index, request](ClusterNode node)
        {
            httpentities::HeartBeatRequest request_ = request;
		    httpentities::ClusterResponse responce = HttpUtil::heartBeat(node.getHeartBeatUrl(), request_, node.getUsername(), node.getPassword());
            std::lock_guard<std::mutex> lock(fail_ip_mutex_);
            if (responce.getStatusCode() != CURLE_OK)
            {
                faileL_[node.getIp()] += 1;
                return;
            }
            faileL_[node.getIp()] = 0;
        };

        for (const auto& node : followNodeL_)
        {
            std::string ip = node.second.getIp();
            auto it = faileL_.find(ip);
            if (it == faileL_.end())
                continue;
            if (it->second > headBeat_max_fail_num_)
                continue;
            
            thread postHearBeat(helper, node.second);
            postHearBeat.detach();
        }
    }

    uint32 ClusterEntityLeader::startNotify(std::string db_name)
    {
        uint32 term = getTerm();
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return 0;
        uint64 index = getDbNextIndex(db_name);
        updateLogStatus(db_name, index, ClusterLogStatus_pending);
        postNotify(db_name, index);
        uint64 end_time = Util::get_cur_time() + relpy_timeout_;
        int once_run = 1000;
        uint32 pass_num = 0;
        TimerProvider oneTimer;
        int need_num = getNeedNum();
        while (1)
        {
            if (once_run > (end_time - Util::get_cur_time()))
                once_run = end_time - Util::get_cur_time();
            oneTimer.SyncWait(once_run, [this, &pass_num, db_name, index]
            {
                pass_num = this->getLogReplyNum(db_name, index);
            });

            if (pass_num >= need_num)
                break;
            if (Util::get_cur_time() >= end_time)
                break;
        }
        oneTimer.Expire();
        return pass_num;
    }

    void ClusterEntityLeader::postSync(std::string db_name, uint64 index, ClusterOperation operation, std::string file_name)
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
                faileL_[node.getIp()] += 1;
                return;
            }
            faileL_[node.getIp()] = 0;
        };

        for (const auto& node : followNodeL_)
        {
            std::string ip = node.second.getIp();
            auto it = faileL_.find(ip);
            if (it == faileL_.end())
                continue;
            if (it->second > headBeat_max_fail_num_)
                continue;
            
            thread postsync(helper, node.second);
            postsync.detach();
        }
    }

    uint32 ClusterEntityLeader::startSync(std::string db_name, ClusterOperation operation, const std::string& file_name)
    {
        uint32 term = getTerm();
        ClusterDbPtr db = findDb(db_name);
        if (!db)
            return 0;
        uint64 index = getDbNextIndex(db_name);
        updateLogStatus(db_name, index, ClusterLogStatus_sync);
        postSync(db_name, index, operation, file_name);
        uint32 end_time = Util::get_cur_time() + sync_timeout_;
        TimerProvider oneTimer;
        int once_run = 1000;
        uint32 pass_num = 0;
        int need_num = getNeedNum();
        while (1)
        {
            if (once_run > (end_time - Util::get_cur_time()))
                once_run = end_time - Util::get_cur_time();
            oneTimer.SyncWait(once_run, [this, &pass_num, db_name, index]
            {
                pass_num = this->getLogSyncNum(db_name, index);
            });

            if (pass_num >= need_num)
                break;
            if (Util::get_cur_time() >= end_time)
                break;
        }
        oneTimer.Expire();
        return pass_num;
    }

    void ClusterEntityLeader::startCommit(std::string db_name)
    {
        uint32 term = getTerm();
        std::string expection = "commit";
        uint64 index = getDbNextIndex(db_name);
        updateLogStatus(db_name, index, ClusterLogStatus_commit);
        httpentities::HeartBeatRequest request(term, db_name, index, expection);
        auto helper = [this, db_name, index, request](ClusterNode node)
        {
            httpentities::HeartBeatRequest request_ = request;
		    httpentities::ClusterResponse responce = HttpUtil::heartBeat(node.getHeartBeatUrl(), request_, node.getUsername(), node.getPassword());
            std::lock_guard<std::mutex> lock(fail_ip_mutex_);
            if (responce.getStatusCode() != CURLE_OK)
            {
                faileL_[node.getIp()] += 1;
                return;
            }
            faileL_[node.getIp()] = 0;
        };

        for (const auto& node : followNodeL_)
        {
            std::string ip = node.second.getIp();
            auto it = faileL_.find(ip);
            if (it == faileL_.end())
                continue;
            if (it->second > headBeat_max_fail_num_)
                continue;
            
            thread postHearBeat(helper, node.second);
            postHearBeat.detach();
        }
    }

    void ClusterEntityLeader::startCancel(std::string db_name)
    {
        uint32 term = getTerm();
        std::string expection = "cancel";
        uint64 index = getDbNextIndex(db_name);
        updateLogStatus(db_name, index, ClusterLogStatus_cancel);
        httpentities::HeartBeatRequest request(term, db_name, index, expection);
        auto helper = [this, db_name, index, request](ClusterNode node)
        {
            httpentities::HeartBeatRequest request_ = request;
		    httpentities::ClusterResponse responce = HttpUtil::heartBeat(node.getHeartBeatUrl(), request_, node.getUsername(), node.getPassword());
            std::lock_guard<std::mutex> lock(fail_ip_mutex_);
            if (responce.getStatusCode() != CURLE_OK)
            {
                faileL_[node.getIp()] += 1;
                return;
            }
            faileL_[node.getIp()] = 0;
        };

        for (const auto& node : followNodeL_)
        {
            std::string ip = node.second.getIp();
            auto it = faileL_.find(ip);
            if (it == faileL_.end())
                continue;
            if (it->second > headBeat_max_fail_num_)
                continue;
            
            thread postHearBeat(helper, node.second);
            postHearBeat.detach();
        }
    }

    std::vector<std::string> ClusterEntityLeader::getFollowrUrlArray()const
    {
        std::vector<std::string> urlL;
        std::string url;
        for (const auto&m : followNodeL_)
        {
            url = "http://" + m.second.getIp() + ":" + m.second.getPort();
            urlL.push_back(url);
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