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
        uint32 relpy_timeout_;
        static uint32 headBeat_max_fail_num_;
        std::map<std::string, uint32> faileL_; //url:失败次数，大于多少次（代表此从节点应答失败，不在发送心跳包）
        // std::map<std::string, TimerProvider> head_beat_timerL_; // db_name
        TimerProvider head_beat_timer_;
        public:
        ClusterNode FindFollower(const std::string& ip, const std::string& port)const;
        bool IsFollowerIp(const std::string& ip)const;
        void postAppendTask(const ClusterTaskInfo& info, const std::string& file_path);
        void startCompare();
        void stopCompareTimer();
        void postTask(const ClusterTaskInfo& info);
        bool runTask(const ClusterTaskInfo& info);
        bool runAppendTask(const ClusterTaskInfo& info);
        bool runRestoreTask(const ClusterRecoverInfo& info);
        bool waitTimerPassNum(std::string db_name, uint64 index, ClusterOperation operation, uint64 end_time);
        bool tryRecover(const std::vector<std::string>& dbs);
        uint32 getNeedNum(){ return (followNodeL_.size()/2)+1; }
        uint32 getAppendTimeout(const std::string& db_name, const std::string& file_name);
        uint64 getTimeOutEndTime(const std::string& db_name = "", const std::string& file_name = "");

        // virtual function in here
        public:
        void init()override;
        ClusterRoleType getCluterRoleType()const override { return cluster::ClusterRoleType_Leader; }
        std::vector<std::string> getFollowrUrlArray()const override;
        std::vector<ClusterNode> getFollowNodeL()const override;
    };
}