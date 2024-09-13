#pragma once
#include "ClusterDefined.h"
#include "ClusterTypedef.h"
#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"
#include "ClusterLog.h"
#include "../Util/Util.h"

namespace cluster
{
    class ClusterManager
    {
        private:
        bool on_;
        ClusterEntityPtr role_;

        public:
        ClusterManager();
        ~ClusterManager();

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
        void startHeartBeat();
        // 启动更新通知, 返回应答数量
        int startNotify(std::string db_name, uint32 term, uint32 index);
        // 启动同步通知, 返回应答数量
        int startSync(std::string db_name, uint32 term, uint32 index, const std::string& file_path);

        //日志模块
        //新增日志
        void addLog(std::string db_name, uint64 index, int status);
        // 更新日志状态
        void updateLogStatus(std::string db_name, uint64 index, int status);
        // 增加响应节点数量
        void addLogReplyNum(std::string db_name, uint64 index);
        // 增加同步节点数量
        void addLogSyncNum(std::string db_name, uint64 index);
        // 获取通知响应数量
        uint32 getLogReplyNum(std::string db_name, uint64 index);
        // 获取同步数量
        uint32 getLogSyncNum(std::string db_name, uint64 index);
        // 更换主节点
        void updateTerm(std::string db_name, uint32 term);
        // 更换主节点索引
        void updateTermIndex(std::string db_name, uint32 term, uint64 index);
    };
}