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
        void postHeartBeat(ClusterLogStatus type, uint32 term, uint32 index);
        void postSync(ClusterLogStatus type, uint32 term, uint32 index, std::string nt);
        void getNotifyNum()const;

        // virtual function in here
        public:
        void init()override;
        ClusterRoleType getCluterRoleType()const override { return cluster::ClusterRoleType_Leader; }
        // 开启心跳检测, 永久性定时器
        void startHeardBeat()override;
        // 开启通知应答，一次性定时器，可复用
        uint32 startNotify(uint32 term, uint32 index)override;
        uint32 startSyncNum(uint32 term, uint32 index, const std::string& nt)override;
    };
}