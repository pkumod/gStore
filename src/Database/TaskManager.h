#pragma once
#include <vector>
#include <map>
#include "../Util/GlobalTypedef.h"
#include "../Util/TimeUtil.h"
#include <memory>
#include <functional>
#include <shared_mutex>

namespace Task
{
    typedef std::function<void()> task_call;
    struct OperationTask
    {
        uint64_t opt_id_;
        std::string operation_;
        int async_;
        std::string database_;
        int status_; //0:handing, 1:finish, -1:cancel
        uint64_t startTime_;
        uint64_t endTime_;
        std::string sparql_;
        std::string resultFile_;
        // callback cancel
        task_call cb_;
        OperationTask()
        {
            opt_id_ = 0;
            async_ = 0;
            startTime_ = 0;
            endTime_ = 0;
            status_ = 0;
        }
        ~OperationTask(){}
        void finishTask(const std::string& resultFile);
        void cancelTask();
        bool isOpCancel()
        {
            if (cb_)
                return true;
            return false;
        }
    };

    struct OperationTaskEvent
    {
        std::shared_ptr<OperationTask> task_;
        OperationTaskEvent(){}
        OperationTaskEvent(std::shared_ptr<OperationTask> task){task_ = task;}
        void checkOpCancel();
    };
    
    class TaskManager
    {
        static std::shared_mutex task_mtx;;
        public:
        static std::map<uint64_t, std::shared_ptr<OperationTask>> taskL_;
        static std::shared_ptr<OperationTask> findTask(uint64_t opt_id);
        static OperationTaskEvent addQueryTask(uint64_t opt_id, const std::string& operation, const std::string& database, const std::string& sparql, bool async);
        static void finishTask(uint64_t opt_id, const std::string& resultFile = "");
        static void getTaskList(std::map<uint64_t, std::shared_ptr<OperationTask>>& taskL);
        static void getTaskList(int status, int& page_no, int& page_size, int& total_page, int& tatal_size, std::vector<std::shared_ptr<OperationTask>>& taskL);
    };
}