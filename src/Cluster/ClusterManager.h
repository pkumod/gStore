/*
 * @Author: hexuejiang
 * @Date: 2024-9-12 14:52:50
 * @LastEditTime: 2024-09-22 14:50:20
 * @LastEditors: hexuejiang 1632802996@qq.com
 * @Description: cluster
 * @FilePath: ClusterManger.h, Cluster management interface, where all methods are defined
 * @FilePath: ClusterEntityLeader.h   -- all leader node interfaces are defined here
 * @FilePath: ClusterEntityFollower.h -- all Follower node interfaces are defined here
 * @FilePath: ClusterLog.h      -- all log info are defined here
 * @FilePath: ClusterCached.h   -- all Save log interfaces are defined here
 */
#pragma once
#include "ClusterTypedef.h"
#include "ClusterDefined.h"
#include "ClusterCached.h"
#include "ClusterEntity.h"
#include "ClusterEntityLeader.h"
#include "ClusterEntityFollower.h"
#include "ClusterLog.h"
#include "../Util/Util.h"
#include "ClusterTask.h"

typedef std::function<void(bool)> timeoutCall;

namespace cluster
{
    class ClusterManager
    {
        private:
        bool on_;
        ClusterEntityPtr role_;
        ConcurrenceQueue<ClusterEventPtr> task_queueL;

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
        bool isFollower();
        // 窘机恢复, 尝试检查所有库
        bool tryRecover(const std::vector<std::string>& dbs);

        //主从互通模块
        // 启动心跳超时检测
        void startHeartBeat(const std::string& db_name);
        // 启动更新通知, 返回应答数量
        int startNotify(std::string db_name);
        // 启动同步通知, 返回应答数量
        int startSync(std::string db_name, ClusterOperation operation, const std::string& file_name);
        // IP是否来自Leader节点
        bool fromLeader(const std::string& ip);
        // IP是否来自Follower节点
        bool fromFollower(const std::string& ip, const std::string& port);
        bool fromFollowerIp(const std::string& ip);
        // 获取从节点url列表, http://127.0.0.1:9000
        std::vector<std::string> getFollowrUrlArray();
        // 获取主节点url
        std::string getLeaderUrl();
        // 获取主节点
        ClusterNode getLearrNode();
        // 添加初始化数据库
        void addClusterDb(const std::string& db_name);
        // 获取从节点列表
        std::vector<ClusterNode> getFollowNodeL();
        // 添加任务
        bool addTask(std::string db_name, ClusterLogStatus status = ClusterLogStatus_HeartBeat, const timeoutCall& cb = nullptr, ClusterOperation operation = ClusterOperation_None, const std::string& file_name = "");
        // 启动跑任务
        void runTask();

        //日志模块
        //新增日志
        void addLog(std::string db_name, uint64 index, ClusterLogStatus status, ClusterOperation operation);
        // 更新日志状态
        void updateLogStatus(std::string db_name, uint64 index, ClusterLogStatus status);
        // 更新日志操作
        void setLogOperation(std::string db_name, uint64 index, ClusterOperation operation);
        // 更新日志操作文件
        void setLogFileName(std::string db_name, uint64 index, std::string file_name);
        // 增加响应节点数量
        void addLogReplyNum(std::string db_name, uint64 index, const std::string& ip, const std::string& port);
        // 增加同步节点数量
        void addLogSyncNum(std::string db_name, uint64 index, const std::string& ip, const std::string& port);
        // 获取通知响应数量
        uint32 getLogReplyNum(std::string db_name, uint64 index);
        // 获取同步数量
        uint32 getLogSyncNum(std::string db_name, uint64 index);
        //是否达到处理要求, 过半(k+1/2)
        bool enabelAttain(std::string db_name, uint64 index, ClusterLogStatus status);
        // 更换主节点
        void updateTerm(uint32 term);
        // 更新已完成节点索引
        void updateDbIndex(std::string db_name, uint64 index);
        // 更新需要处理的节点索引
        void updateDbNextIndex(std::string db_name, uint64 next_index);
        // 获取任期
        uint32 getTerm();
        // 获取数据库成功提交的最新日志索引
        uint64 getDbIndex(std::string db_name);
        // 获取数据库正在处理的日志索引
        uint64 getDbNextIndex(std::string db_name);
        // 获取nt文件路径
        std::string getNTFilePathByIndex(const std::string& db_name, uint64 index);
        // 获取日志状态
        ClusterLogStatus getDbLogStatus(const std::string& db_name, uint64 index);
        // 获取日志操作
        ClusterOperation getDbLogOperation(const std::string& db_name, uint64 index);
        // 获取数据库路径
        std::string getDbDirPath(std::string db_name);

        // nt数据存储模块
        // 普通数据更新，每次操作，单独文件进行存储
        // 事务操作, 一次完整操作, 存储一个文件
        void addCachedNtFile(const std::vector<TripleInfo>& triples, const std::string& db_name, const std::string file_name);
        // 事务操作进行追加
        void appendCachedNtData(const std::vector<TripleInfo>& triples, const std::string& db_name,  const std::string file_name);
        // 存储follower接收到的nt文件, 返回文件路径
        std::string saveFromFollowerFile(const std::pair<std::string, std::string>& file_info, const std::string& db_name);
        // 获取操作文件nt数据
        void getNtFileData(std::vector<TripleInfo>& triples, const std::string& db_name, const std::string& file_name);
        // 获取操作文件nt数据
        std::string getNtFilePath(const std::string& db_name, const std::string& file_name);
        // 微妙
        uint32 getAppendTimeout(const std::string& db_name, const std::string& file_name);
    };

    // task
    struct ClusterHeartBeatEvent : public ClusterEvent
    {
        std::string db_name_;
        cluster_operation expection_;
        ClusterEntityLeaderWeaker wer_;
        timeoutCall cb_;
        ClusterHeartBeatEvent(std::string db_name, ClusterEntityLeaderPtr per, const timeoutCall &cb, cluster_operation expection)
        {
            db_name_ = db_name;
            wer_ = per;
            cb_ = cb;
            expection_ = expection;
        }
        void runEvent()const override
        {
            ClusterEntityLeaderPtr per = wer_.lock();
            if (!per)
            {
                SLOG_TRACE("ClusterHeartBeatEvent fail, per is free");
                return;
            }
            if (expection_ == EXPECTION_COMPARE)
            {
                per->startHeardBeat(db_name_);
            }
            else if (expection_ == EXPECTION_PREPARE)
            {
                uint32 num = per->startNotify(db_name_);
                uint32 need_num = per->getNeedNum();
                if (cb_)
                {
                    SLOG_TRACE("cluster reply callback:" << num << " , need num:" << need_num);
                    if (num == 0 || num < need_num)
                        cb_(false);
                    else
                        cb_(true);
                }
            }
            else if (expection_ == EXPECTION_COMMIT)
            {
                per->startCommit(db_name_);
            }
            else if (expection_ == EXPECTION_CANCEL)
            {
                per->startCancel(db_name_);
            }
            else if (expection_ == EXPECTION_FAIL)
            {
                per->startFail(db_name_);
            }
        }
    };

    struct ClusterSyncEvent : public ClusterEvent
    {
        std::string db_name_;
        ClusterOperation operation_;
        std::string file_name_;
        ClusterEntityLeaderWeaker wer_;
        timeoutCall cb_;
        ClusterSyncEvent()
        {
            db_name_ = "";
            operation_ = ClusterOperation_None;
            file_name_ = "";
        }
        ClusterSyncEvent(std::string db_name, ClusterOperation operation, std::string file_name, ClusterEntityLeaderPtr per, const timeoutCall &cb)
        {
            db_name_ = db_name;
            operation_ = operation;
            file_name_ = file_name;
            wer_ = per;
            cb_ = cb;
        }
        void runEvent()const override
        {
            ClusterEntityLeaderPtr per = wer_.lock();
            if (!per)
            {
                SLOG_TRACE("ClusterHeartBeatEvent fail, per is free");
                return;
            }
            uint32 num = per->startSync(db_name_, operation_, file_name_);
            uint32 need_num = per->getNeedNum();
            if (cb_)
            {
                SLOG_TRACE("cluster sync callback:" << num << " , need num:" << need_num);
                if (num == 0 || num < need_num)
                    cb_(false);
                else
                    cb_(true);
            }
        }
    };
}