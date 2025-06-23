#include "TaskManager.h"
#include "../Api/TimerProvider.h"

namespace Task
{
    std::map<uint64_t, std::shared_ptr<OperationTask>> TaskManager::taskL_ = {};
    std::shared_mutex TaskManager::task_mtx;
    // query task
    void OperationTask::finishTask(const std::string& resultFile)
    {
        endTime_ = gs::TimeUtil::timestamp();
        if (status_ == 0)
            status_ = 1;
        resultFile_ = resultFile;
    }

    void OperationTask::cancelTask()
    {
        cb_ = [this]()
        {
        	SLOG_TRACE("query task opt_id:" << opt_id_);
            endTime_ = gs::TimeUtil::timestamp();
            status_ = -1;
            cb_ = nullptr;
        };
        TimerProvider once;
        int once_run = 250;
        while (1)
        {
            once.SyncWait(once_run, [this]{});
            if (status_ == 1 || status_ == -1)
                return;
        }
    }

    // task event
    void OperationTaskEvent::checkOpCancel()
    {
        if (!task_)
            return;
        if (task_->isOpCancel())
        {
            SLOG_TRACE("call back cancel:" << task_->opt_id_);
            task_->cb_();
            std::string msg = "operation cancel opt_id:" + std::to_string(task_->opt_id_);
            throw std::runtime_error(msg);
        }
    }

    // task manager
    OperationTaskEvent TaskManager::addQueryTask(uint64_t opt_id, const std::string& operation, const std::string& database, const std::string& sparql, bool async)
    {
        if (operation != "query")
            return OperationTaskEvent();

        std::shared_ptr<OperationTask> task = std::make_shared<OperationTask>();
        task->opt_id_ = opt_id;
        task->database_ = database;
        task->startTime_ = gs::TimeUtil::timestamp();
        task->sparql_ = sparql;
        task->operation_ = operation;
        task->async_ = async;
        auto task_insert = [task]()
        {
            std::unique_lock<std::shared_mutex> lock(TaskManager::task_mtx);
            TaskManager::taskL_.insert(std::make_pair(task->opt_id_, task));
        };
        std::thread t(task_insert);
        t.detach();
        OperationTaskEvent task_event(task);
        return task_event;
    }

    std::shared_ptr<OperationTask> TaskManager::findTask(uint64_t opt_id)
    {
        std::shared_lock<std::shared_mutex> lock(task_mtx);
        auto it = taskL_.find(opt_id);
        if (it == taskL_.end())
            return nullptr;
        return it->second;
    }

    void TaskManager::finishTask(uint64_t opt_id, const std::string& resultFile)
    {
        std::shared_lock<std::shared_mutex> lock(task_mtx);
        std::shared_ptr<OperationTask> task = findTask(opt_id);
        if (!task)
            return;
        task->finishTask(resultFile);
    }

    void TaskManager::getTaskList(std::map<uint64_t, std::shared_ptr<OperationTask>>& taskL)
    {
        std::shared_lock<std::shared_mutex> lock(task_mtx);
        taskL = taskL_;
    }

    void TaskManager::getTaskList(int status, int& page_no, int& page_size, int& total_page, int& tatal_size, std::vector<std::shared_ptr<OperationTask>>& taskL)
    {
        std::map<uint64_t, std::shared_ptr<OperationTask>> tempL;
        getTaskList(tempL);

        if (page_no <= 0)
            page_no = 1;
        if (page_size <= 0)
            page_size = 10;
        int cur_pos = 0;
        bool isall = false;
        if (status != 0 && status != 1 && status != -1)
            isall = true;
        int startPos = (page_no-1)*page_size + 1;
        int endPos = page_no*page_size;
        for (const auto& m : tempL)
        {
            if (!isall && status != m.second->status_)
                continue;
            cur_pos++;
            if (startPos <= cur_pos && cur_pos <= endPos)
                taskL.push_back(m.second);
        }
        total_page = cur_pos/page_size + (cur_pos%page_size==0 ? 0:1);
        tatal_size = cur_pos;
    }
}