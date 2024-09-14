#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"

namespace cluster
{
    uint32 ClusterEntityLeader::headBeat_max_fail_num_ = 5;
    void ClusterEntityLeader::init()
    {
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
        relpy_timeout_ = std::atoi(Util::getConfigureValue("relpy_timeout_").c_str());
        sync_timeout_ = std::atoi(Util::getConfigureValue("sync_timeout_").c_str());
    }

    ClusterNode ClusterEntityLeader::FindFollower(const std::string& ip)const
    {
        auto it = followNodeL_.find(ip);
        if (it == followNodeL_.end())
            return ClusterNode();
        return it->second;
    }

    void ClusterEntityLeader::startHeardBeat()
    {
        head_beat_timer_.StartTimer(heartbeat_, [this]()
        {
            this->postHeartBeat();
        });
    }

    void ClusterEntityLeader::postHeartBeat()
    {
        ClusterHeartBeat postdata;
        postdata.setStatus(ClusterLogStatus_HeartBeat);

        auto helper = [this, postdata](ClusterNode node)
        {
            string res;
            std::string remote = node.ip + node.port;
            int error = HttpUtil::Post(node.getUrlString(), postdata.toPostString(node.username, node.password), res);
            if (error != CURLE_OK)
            {
                faileL_[node.ip] += 1;
                return;
            }
            faileL_[node.ip] = 0;
        };

        for (const auto& node : followNodeL_)
        {
            std::string ip = node.second.ip;
            auto it = faileL_.find(ip);
            if (it == faileL_.end())
                continue;
            if (it->second > headBeat_max_fail_num_)
                continue;
            
            thread postHearBeat(helper, node.second);
            postHearBeat.detach();
        }
    }

    void ClusterEntityLeader::postNotify(std::string db_name, uint32 term, uint64 index)
    {
        ClusterHeartBeat postdata;
        postdata.setStatus(ClusterLogStatus_pending);
        postdata.setTerm(term);
        postdata.setIndex(index);
        postdata.setDbName(db_name);

        auto helper = [this, db_name, index, postdata](ClusterNode node)
        {
            string res;
            std::string remote = node.ip + node.port;
            int error = HttpUtil::Post(node.getUrlString(), postdata.toPostString(node.username, node.password), res);
            if (error != CURLE_OK)
            {
                faileL_[node.ip] += 1;
                return;
            }
            faileL_[node.ip] = 0;
        };

        for (const auto& node : followNodeL_)
        {
            std::string ip = node.second.ip;
            auto it = faileL_.find(ip);
            if (it == faileL_.end())
                continue;
            if (it->second > headBeat_max_fail_num_)
                continue;
            
            thread postHearBeat(helper, node.second);
            postHearBeat.detach();
        }
    }

    uint32 ClusterEntityLeader::startNotify(std::string db_name, uint32 term, uint64 index)
    {
        postNotify(db_name, term, index);
        uint64 end_time = Util::get_cur_time() + relpy_timeout_;
        TimerProvider oneTimer;
        int once_run = 1000;
        uint32 pass_num = 0;
        int need_num = (followNodeL_.size() + 1) / 2;
        while (1)
        {
            if (once_run > (end_time - Util::get_cur_time()))
                once_run = end_time - Util::get_cur_time();
            oneTimer.AsyncWait(once_run, [this, &pass_num, db_name, index]
            {
                pass_num = this->getLogReplyNum(db_name, index);
            });

            if (pass_num >= need_num)
                break;
            if (Util::get_cur_time() >= end_time)
                break;
        }

        return pass_num;
    }

    void ClusterEntityLeader::postSync(std::string db_name, uint32 term, uint64 index, ClusterOperation operation, std::string file_path)
    {
        // file_path以二进制打开文件读取数据
        ClusterSync postdata;
        postdata.setOperation(operation);
        postdata.setStatus(ClusterLogStatus_sync);
        postdata.setTerm(term);
        postdata.setIndex(index);
        postdata.setDbName(db_name);
        postdata.setFilePath(file_path);

    }

    uint32 ClusterEntityLeader::startSync(std::string db_name, uint32 term, uint64 index, ClusterOperation operation, const std::string& file_path)
    {
        postSync(db_name, term, index, operation, file_path);
        uint32 end_time = Util::get_cur_time() + sync_timeout_;
        TimerProvider oneTimer;
        int once_run = 1000;
        uint32 pass_num = 0;
        int need_num = (followNodeL_.size() + 1) / 2;
        while (1)
        {
            if (once_run > (end_time - Util::get_cur_time()))
                once_run = end_time - Util::get_cur_time();
            oneTimer.AsyncWait(once_run, [this, &pass_num, db_name, index]
            {
                pass_num = this->getLogSyncNum(db_name, index);
            });

            if (pass_num >= need_num)
                break;
            if (Util::get_cur_time() >= end_time)
                break;
        }

        return pass_num;
    }

    std::vector<std::string> ClusterEntityLeader::getFollowrUrlArray()
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
}