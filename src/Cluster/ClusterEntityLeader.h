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
        uint64 relpy_timeout_;
        static uint32 headBeat_max_fail_num_;
        std::map<std::string, uint32> faileL_; //url:失败次数，大于多少次（代表此从节点应答失败，不在发送心跳包）
        std::map<std::string, TimerProvider> head_beat_timerL_; // db_name
        public:
        ClusterNode FindFollower(const std::string& ip, const std::string& port)const;
        bool IsFollowerIp(const std::string& ip)const;
        void postHeartBeat(std::string db_name);
        void postNotify(std::string db_name, uint64 index);
        void postSync(std::string db_name, uint64 index, ClusterOperation operation, std::string file_name);
        void startHeardBeat(std::string db_name);
        void stopHeardBeatTimer(std::string db_name);
        uint32 startNotify(std::string db_name);
        uint32 startSync(std::string db_name, ClusterOperation operation, const std::string& file_name);
        void startCommit(std::string db_name);
        void startCancel(std::string db_name);
        void startFail(std::string db_name);
        bool tryRecover(const std::vector<std::string>& dbs);
        uint32 getNeedNum(){ return (followNodeL_.size() + 1) / 2; }
        uint32 getAppendTimeout(const std::string& db_name, const std::string& file_name);

        // virtual function in here
        public:
        void init()override;
        ClusterRoleType getCluterRoleType()const override { return cluster::ClusterRoleType_Leader; }
        std::vector<std::string> getFollowrUrlArray()const override;
        std::vector<ClusterNode> getFollowNodeL()const override;
    };
}