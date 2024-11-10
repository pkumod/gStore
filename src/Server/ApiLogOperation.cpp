#include "ApiProvider.h"

namespace server
{
    void ApiHandler::txn_log(shared_ptr<APIUtil>& apiUtil, server::MessageTxnLogRequest& resquest, server::MessageTxnLogResponse& response)
    {
        try
        {
            // std::string username = jsonParam(json_data, "username");
            // if (username != apiUtil->get_root_username())
            // {
            // 	std::string msg =  "Root User Only!";
            // 	response->Error(StatusOperationConditionsAreNotSatisfied, error);
            // 	return;
            // }
            int page_no = resquest.pageNo;
            int page_size = resquest.pageSize;
            shared_ptr<struct TransactionLogs> transactionLogsPtr = make_shared<struct TransactionLogs>();
            apiUtil->get_transactionlog(page_no, page_size, transactionLogsPtr);
            vector<struct TransactionLogInfo> logList = transactionLogsPtr->getTransactionLogInfoList();
            size_t count = logList.size();
            nlohmann::json info;
            for (size_t i = 0; i < count; i++)
            {
                TransactionLogInfo log_info = logList[i];
                log_info.toJSON(info);
                response.list.push_back(info);
            }
            response.StatusCode = StatusOK;
            response.StatusMsg = "Get transaction log success.";
            response.totalSize = transactionLogsPtr->getTotalSize();
            response.totalPage = transactionLogsPtr->getTotalPage();
            response.pageNo = page_no;
            response.pageSize = page_size;
        }
        catch (const std::exception &e)
        {
            string error = "Get transaction log fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::query_log(shared_ptr<APIUtil>& apiUtil, server::MessageQueryLogRequest& resquest, server::MessageQueryLogResponse& response)
    {
        try
        {
            std::string date = resquest.date;
            std::string msg;
            if (apiUtil->check_param_value("date", date, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            int page_no = resquest.pageNo;
            int page_size = resquest.pageSize;
            shared_ptr<struct DBQueryLogs> dbQueryLogsPtr = make_shared<struct DBQueryLogs>();
            apiUtil->get_query_log(date, page_no, page_size, dbQueryLogsPtr);
            vector<struct DBQueryLogInfo> logList = dbQueryLogsPtr->getQueryLogInfoList();
            size_t count = logList.size();
            
            for (size_t i = 0; i < count; i++)
            {
                nlohmann::json info = logList[i];
                response.list.push_back(info);
            }

            response.StatusCode = StatusOK;
            response.StatusMsg = "Get query log success.";
            response.totalSize = dbQueryLogsPtr->getTotalSize();
            response.totalPage = dbQueryLogsPtr->getTotalPage();
            response.pageNo = page_no;
            response.pageSize = page_size;
        }
        catch (const std::exception &e)
        {
            string error = "Get query log fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::query_log_date(shared_ptr<APIUtil>& apiUtil, server::MessageQueryLogDateRequest& resquest, server::MessageQueryLogDateResponse& response)
    {
        try
        {
            vector<string> logfiles;
            apiUtil->get_query_log_files(logfiles);
            sort(logfiles.begin(), logfiles.end(), [](const string& a, const string& b) {
                return a > b;
            });
            size_t count = logfiles.size();
            std::string item;
            for (size_t i = 0; i < count; i++)
            {
                item = logfiles[i];
                item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
                response.list.push_back(item);
            }
            response.StatusCode = StatusOK;
            response.StatusMsg = "Get query log date success";
        }
        catch(const std::exception& e)
        {
            string error = "Get query log date fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::access_log(shared_ptr<APIUtil>& apiUtil, server::MessageAccessLogRequest& resquest, server::MessageAccessLogResponse& response)
    {
        try
        {
            std::string date = resquest.date;
            std::string msg;
            if (apiUtil->check_param_value("date", date, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            int page_no = resquest.pageNo;
            int page_size = resquest.pageSize;
            shared_ptr<struct DBAccessLogs> dbAccessLogsPtr = make_shared<struct DBAccessLogs>();
            apiUtil->get_access_log(date, page_no, page_size, dbAccessLogsPtr);
            vector<struct DBAccessLogInfo> logList = dbAccessLogsPtr->getAccessLogInfoList();
            size_t count = logList.size();
            nlohmann::json info;
            for (size_t i = 0; i < count; i++)
            {
                DBAccessLogInfo log_info = logList[i];
                log_info.toJSON(info);
                response.list.push_back(info);
            }

            response.StatusCode = StatusOK;
            response.StatusMsg = "Get access log success.";
            response.totalSize = dbAccessLogsPtr->getTotalSize();
            response.totalPage = dbAccessLogsPtr->getTotalPage();
            response.pageNo = page_no;
            response.pageSize = page_size;
        }
        catch (const std::exception &e)
        {
            string error = "Get access log fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::access_log_date(shared_ptr<APIUtil>& apiUtil, server::MessageAccessLogDateRequest& resquest, server::MessageAccessLogDateResponse& response)
    {
        try
        {
            vector<string> logfiles;
            apiUtil->get_access_log_files(logfiles);
            sort(logfiles.begin(), logfiles.end(), [](const string& a, const string& b) {
                return a > b;
            });
            size_t count = logfiles.size();
            std::string item;
            for (size_t i = 0; i < count; i++)
            {
                item = logfiles[i];
                item = item.substr(0, item.length()-4); // file_name: yyyyMMdd.log
                response.list.push_back(item);
            }
            response.StatusCode = StatusOK;
            response.StatusMsg = "Get access log date success";
        }
        catch(const std::exception& e)
        {
            string error = "Get access log date fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::checkOperationState(shared_ptr<APIUtil>& apiUtil, server::MessageCheckOperationStateRequest& resquest, server::MessageCheckOperationStateResponse& response)
    {
        string msg;
        string operation = "checkOperationState";
        try
        {
            std::string opt_id = resquest.opt_id;
            if (apiUtil->check_param_value("opt_id", opt_id, msg))
            {
                response.Error(StatusOperationFailed, msg);
                return;
            }
            struct DBAccessLogInfo log;
            if (!apiUtil->getAccessLogByOptId(opt_id, log))
            {
                msg =  "opt_id not found.";
                response.Error(StatusOperationFailed, msg);
                return;
            }
            std::string backupfilepath = log.backupfilepath;
            response.StatusCode = StatusOK;
            response.StatusMsg = log.msg;
            response.state = log.state;
            std::string log_operation = log.operation;
            if (log_operation == "backup")
            {
                response.backupfilepath = backupfilepath;
            }
            else if(log_operation != "restore")
            {
                response.success_num = log.num;
                response.failed_num = log.fail_num;
            }
        }
        catch (const std::exception &e)
        {
            string error = "checkbatchInsertUid fail:" + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}