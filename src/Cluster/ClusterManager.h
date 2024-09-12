#pragma once
#include "ClusterDefined.h"
#include "ClusterTypedef.h"
#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollow.h"
#include "ClusterLog.h"
#include "../Util/Util.h"

namespace cluster
{
    class CluterManager
    {
        private:
        bool on_;
        ClusterLogPtr log_;
        ClusterEntityPtr role_;

        public:
        CluterManager();
        ~CluterManager();

        // 从配置表读取数据, 初始化主从节点
        void init();

        public:
        // 是否启用集群
        bool isEnable(){ return on_; }
        // 集群身份
        ClusterRoleType getCluterRole(){ return role_->getCluterRoleType(); }
        // 是否是主节点
        bool isLeader();
        // 窘机恢复
        void tryRecover();

        //主从互通模块
        // 启动心跳超时检测
        void startHeartBeatTimeOut() { role_->startHeardBeat(); }
        // 启动更新通知, 返回应答数量
        uint32 startNotify(uint32 term, uint32 index);
        // 启动同步通知, 返回应答数量
        uint32 startSync(uint32 term, uint32 index, const std::string& file_path);

        //日志模块
        //新增日志
        static void addLog(uint32 term, uint32 index, uint32 status);
        // 更新日志状态
        static void updateLogStatus(uint32 term, uint32 index, uint32 status);
        // 增加响应节点数量
        static void addLogReplyNum(uint32 index, uint32 replyNum);
        // 增加同步节点数量
        static void addLogSyncNum(uint32 index, uint32 replyNum);
        // 获取通知响应数量
        static uint32 getLogReplyNum(uint32 index);
        // 获取同步数量
        static uint32 getLogSyncNum(uint32 index);
    };
}