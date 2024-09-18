#pragma once
#include "ClusterDefined.h"
#include "ClusterEntity.h"
#include "../Api/HttpEntities.h"

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
        void postHeartBeat();
        void postNotify(std::string db_name, uint64 index);
        void postSync(std::string db_name, uint64 index, ClusterOperation operation, std::string file_name);
        void postCancel(std::string db_name, uint64 index, ClusterOperation operation, std::string file_name);
        void startHeardBeat();
        uint32 startNotify(std::string db_name, uint64 index);
        uint32 startSync(std::string db_name, uint64 index, ClusterOperation operation, const std::string& file_name);
        uint32 startCancel(std::string db_name, uint64 index, ClusterOperation operation, const std::string& file_name);
        bool tryRecover(const std::vector<std::string>& dbs);

        // virtual function in here
        public:
        void init()override;
        ClusterRoleType getCluterRoleType()const override { return cluster::ClusterRoleType_Leader; }
        std::vector<std::string> getFollowrUrlArray()const override;
        std::vector<ClusterNode> getFollowNodeL()const override;
    };
}