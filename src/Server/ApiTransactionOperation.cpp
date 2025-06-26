#include "ApiProvider.h"

namespace server
{
    void ApiHandler::begin(shared_ptr<APIUtil>& apiUtil, const server::MessageBeginRequest& request, server::MessageBeginResponse& response)
    {
        try
        {
            std::string db_name = request.db_name;
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            std::string isolevel = request.isolevel;
            if (apiUtil->check_param_value("isolevel", isolevel, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            int level = stoi(isolevel);
            if (level <= 0 || level > 3)
            {
                msg = "The isolation level's value only can been 1/2/3";
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            if (apiUtil->check_db_built(db_name) == false)
            {
                msg = "Database not built yet.";
                response.Error(StatusOperationConditionsAreNotSatisfied, msg);
                return;
            }
            if (apiUtil->check_db_loaded(db_name) == false)
            {
                msg = "Database not load yet.";
                response.Error(StatusOperationConditionsAreNotSatisfied, msg);
                return;
            }
            std::string username = request.username;
            txn_id_t tid;
            if (apiUtil->begin_process(db_name, level, username, tid) == false)
            {
                msg = "Transaction begin failed.";
                response.Error(StatusTranscationManageFailed, msg);
                return;
            }
            response.StatusMsg = "Transaction begin success";
            response.TID = std::to_string(tid);
        }
        catch (const std::exception &e)
        {
            std::string error = "Transaction begin fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::tquery(shared_ptr<APIUtil>& apiUtil, const server::MessageTqueryRequest& request, server::MessageTqueryResponse& response)
    {
        try
        {
            std::string db_name = request.db_name;
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            std::string tid_s = request.tid;
            if (apiUtil->check_param_value("tid", tid_s, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            txn_id_t tid;
            if (apiUtil->check_txn_id(tid_s, tid))
            {
                msg = "TID " + tid_s + " is not a pure number.";
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            std::string sparql = request.sparql;
            if (apiUtil->check_param_value("sparql", sparql, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            shared_ptr<Txn_manager> txn_m;
            if (apiUtil->get_txn_manager(db_name, txn_m) == false)
            {
                msg = "Get database transaction manager error.";
                response.Error(StatusTranscationManageFailed, msg);
                return;
            }
            shared_ptr<DatabaseInfo> db_info;
            StatusCode statusCode;
            if(!apiUtil->validate_databaseinfo(db_name, db_info, statusCode, msg, true, DatabaseLock::W))
            {
                response.Error(statusCode, msg);
                return;
            }
            SLOG_DEBUG("tquery sparql: " + sparql);
            std::string res;
            int ret = txn_m->Query(tid, sparql, res);
            apiUtil->unlock_databaseinfo(db_info);
            if (ret == -1)
            {
                msg = "Transaction query failed due to wrong TID";
                response.Error(StatusOperationFailed, msg);
            }
            else if (ret == -10)
            {
                msg = "Database has been flushed or removed";
                response.Error(StatusOperationFailed, msg);
            }
            else if (ret == -99)
            {
                msg = "Transaction is not in running status!";
                response.Error(StatusOperationFailed, msg);
            }
            else if (ret == -20)
            {
                apiUtil->aborted_process(txn_m, tid, msg);
                msg = "Transaction Abort due to Query failed!";
                response.Error(StatusOperationFailed, msg);
            }
            else if (ret == -101)
            {
                msg = "Transaction query failed. Unknown query error";
                response.Error(StatusOperationFailed, msg);
            }
            if (ret == -100)
            {
                try
                {
                    response.query_json = nlohmann::json::parse(res);
                }
                catch (nlohmann::json::exception& e)
                {
                    SLOG_ERROR("tquery result parse error.\n" + res);
                    response.result = res;
                }
                response.StatusMsg = "success";
            }
            else
            {
                response.ansNum = ret;
                response.StatusMsg = StatusOK;
                response.StatusMsg = "Transaction query success";
            }
        }
        catch (const std::exception &e)
        {
            string error = "Transaction query fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::commit(shared_ptr<APIUtil>& apiUtil, const server::MessageCommitRequest& request, server::MessageResponse& response)
    {
        try
        {
            std::string db_name = request.db_name;
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            std::string tid_s = request.tid;
            if (apiUtil->check_param_value("TID", tid_s, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            txn_id_t tid;
            if (apiUtil->check_txn_id(tid_s, tid))
            {
                msg = "TID " + tid_s + " is not a pure number.";
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            shared_ptr<DatabaseInfo> db_info;
            server::StatusCode statusCode;
            std::string statusMsg;
            if (!apiUtil->validate_databaseinfo(request.db_name, db_info, statusCode, statusMsg, true, DatabaseLock::W))
            {
                response.StatusCode = statusCode;
                response.StatusMsg = statusMsg;
                return;
            }
            shared_ptr<Txn_manager> txn_m;
            if (apiUtil->get_txn_manager(db_name, txn_m) == false)
            {
                apiUtil->unlock_databaseinfo(db_info);
                msg = "Get database transaction manager error.";
                response.Error(StatusTranscationManageFailed, msg);
                return;
            }
            if (apiUtil->commit_process(txn_m, tid, msg) ==  false)
            {
                response.Error(StatusOperationFailed, msg);
            }
            else
            {
                response.StatusMsg = "Transaction commit success.";
            }
            apiUtil->unlock_databaseinfo(db_info);
        }
        catch (const std::exception &e)
        {
            string msg = "Transaction commit fail: " + string(e.what());
            response.Error(StatusOperationFailed, msg);
        }
    }

    void ApiHandler::rollback(shared_ptr<APIUtil>& apiUtil, const server::MessageCommitRequest& request, server::MessageResponse& response)
    {
        try
        {
            std::string db_name = request.db_name;
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            std::string tid_s = request.tid;
            if (apiUtil->check_param_value("TID", tid_s, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            txn_id_t tid;
            if (apiUtil->check_txn_id(tid_s, tid))
            {
                msg = "TID " + tid_s + " is not a pure number.";
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            shared_ptr<DatabaseInfo> db_info;
            server::StatusCode statusCode;
            std::string statusMsg;
            if (!apiUtil->validate_databaseinfo(request.db_name, db_info, statusCode, statusMsg, true, DatabaseLock::W))
            {
                response.StatusCode = statusCode;
                response.StatusMsg = statusMsg;
                return;
            }
            shared_ptr<Txn_manager> txn_m;
            if (apiUtil->get_txn_manager(db_name, txn_m) == false)
            {
                apiUtil->unlock_databaseinfo(db_info);
                msg = "Get database transaction manager error.";
                response.Error(StatusTranscationManageFailed, msg);
                return;
            }
            if (apiUtil->rollback_process(txn_m, tid, msg) == false)
            {
                response.Error(StatusOperationFailed, msg);
            }
            else
            {
                response.StatusMsg = "Transaction rollback success.";
            }
            apiUtil->unlock_databaseinfo(db_info);
        }
        catch (const std::exception &e)
        {
            string error = "Transaction rollback fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}