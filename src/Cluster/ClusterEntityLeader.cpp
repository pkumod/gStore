#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollow.h"

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

    void ClusterEntityLeader::startHeardBeat()
    {
        head_beat_timer_.StartTimer(heartbeat_, [this]()
        {
            this->postHeartBeat(ClusterLogStatus_HeartBeat, 0, 0);
        });
    }

    void ClusterEntityLeader::postHeartBeat(ClusterLogStatus type, uint32 term, uint32 index)
    {
        ClusterHeartBeat postdata;
        postdata.setType(type);
        postdata.setTerm(term);
        postdata.setIndex(index);

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

    uint32 ClusterEntityLeader::startNotify(uint32 term, uint32 index)
    {
        postHeartBeat(ClusterLogStatus_HeartBeat, term, index);
        uint32 end_time = Util::get_cur_time() + relpy_timeout_;
        TimerProvider oneTimer;
        int once_run = 1000;
        uint32 pass_num = 0;
        int need_num = (followNodeL_.size() + 1) / 2;
        while (1)
        {
            if (once_run > (end_time - Util::get_cur_time()))
                once_run = end_time - Util::get_cur_time();
            oneTimer.AsyncWait(once_run, [this, &pass_num]
            {
                pass_num = ClusterLog::getNodeNum();
            });

            if (pass_num >= need_num)
                break;
            if (Util::get_cur_time() >= end_time)
                break;
        }

        return pass_num;
    }

    void ClusterEntityLeader::postSync(ClusterLogStatus type, uint32 term, uint32 index, std::string nt)
    {

    }

    uint32 ClusterEntityLeader::startSyncNum(uint32 term, uint32 index, const std::string& nt)
    {
        return 0;
    }
}