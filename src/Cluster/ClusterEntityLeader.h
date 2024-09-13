#pragma once
#include "ClusterDefined.h"
#include "ClusterEntity.h"

namespace cluster
{
    class ClusterEntityLeader : public cluster::ClusterEntity
    {
        std::map<std::string, ClusterNode> followNodeL_; //ip:ClusterNode
        int heartbeat_;
        int relpy_timeout_;
        int sync_timeout_;
        static uint32 headBeat_max_fail_num_;
        std::map<std::string, uint32> faileL_; //ip:失败次数，大于多少次（代表此从节点应答失败，不在发送心跳包）
        TimerProvider head_beat_timer_;
        public:
        ClusterNode FindFollower(const std::string& ip)const;
        void postHeartBeat(ClusterLogStatus type, uint32 term, uint32 index);
        void postReply(std::string db_name, ClusterLogStatus type, uint32 term, uint32 index);
        void postSync(std::string db_name, ClusterLogStatus type, uint32 term, uint32 index, std::string nt);
        void startHeardBeat();
        uint32 startNotify(std::string db_name, uint32 term, uint32 index);
        uint32 startSync(std::string db_name, uint32 term, uint32 index, const std::string& file_path);

        // virtual function in here
        public:
        void init()override;
        ClusterRoleType getCluterRoleType()const override { return cluster::ClusterRoleType_Leader; }
    };
}