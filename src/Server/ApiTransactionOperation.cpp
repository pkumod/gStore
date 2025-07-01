#include "ApiProvider.h"

namespace server
{
    bool  ApiHandler::transaction_check(shared_ptr<APIUtil>& apiUtil, const std::string& db_name, const std::string& tid_s, txn_id_t& tid, server::MessageResponse& response)
    {
        std::string msg;
        if (apiUtil->check_param_value("db_name", db_name, msg) == false)
        {
            response.Error(StatusParamIsIllegal, msg);
            return false;
        }
        if (apiUtil->check_param_value("tid", tid_s, msg) == false)
        {
            response.Error(StatusParamIsIllegal, msg);
            return false;
        }
        if (apiUtil->check_txn_id(tid_s, tid))
        {
            msg = "TID " + tid_s + " is not a pure number.";
            response.Error(StatusParamIsIllegal, msg);
            return false;
        }
        return true;
    }

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
        shared_ptr<DatabaseInfo> db_info = nullptr;
        try
        {
            std::string db_name = request.db_name;
            txn_id_t tid;
            if (transaction_check(apiUtil, db_name, request.tid, tid, response) == false)
            {
                return;
            }
            std::string msg;
            std::string sparql = request.sparql;
            if (apiUtil->check_param_value("sparql", sparql, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            StatusCode statusCode;
            if(!apiUtil->validate_databaseinfo(db_name, db_info, statusCode, msg, true, DatabaseLock::W))
            {
                response.Error(statusCode, msg);
                return;
            }
            shared_ptr<Txn_manager> txn_m;
            if (apiUtil->get_txn_manager(db_name, txn_m) == false)
            {
                msg = "Get database transaction manager error.";
                throw new std::runtime_error(msg);
            }
            SLOG_DEBUG("tquery sparql: " + sparql);
            std::string res;
            int ret = txn_m->Query(tid, sparql, res);
            apiUtil->unlock_databaseinfo(db_info);
            db_info.reset();
            if (ret >= 0)
            {
                response.ansNum = ret;
                response.StatusMsg = StatusOK;
                response.StatusMsg = "Transaction query success";
            }
            else if (ret == -100)
            {
                try
                {
                    response.query_json = nlohmann::json::parse(res);
                }
                catch (nlohmann::json::exception& e)
                {
                    SLOG_WARN("tquery result parse error.\n" + res);
                    response.result = res;
                }
                response.StatusMsg = "success";
            }
            else 
            {
                if (ret == -20)
                {
                    apiUtil->aborted_process(txn_m, tid);
                }
                response.Error(StatusOperationFailed, res);
            }
        }
        catch (const std::exception &e)
        {
            apiUtil->unlock_databaseinfo(db_info);
            string error = "Transaction query fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::commit(shared_ptr<APIUtil>& apiUtil, const server::MessageCommitRequest& request, server::MessageResponse& response)
    {
        shared_ptr<DatabaseInfo> db_info = nullptr;
        try
        {
            std::string db_name = request.db_name;
            txn_id_t tid;
            if (transaction_check(apiUtil, db_name, request.tid, tid, response) == false)
            {
                return;
            }
            server::StatusCode statusCode;
            std::string msg;
            if (!apiUtil->validate_databaseinfo(request.db_name, db_info, statusCode, msg, true, DatabaseLock::W))
            {
                response.StatusCode = statusCode;
                response.StatusMsg = msg;
                return;
            }
            shared_ptr<Txn_manager> txn_m;
            if (apiUtil->get_txn_manager(db_name, txn_m) == false)
            {
                msg = "Get database transaction manager error.";
                throw new std::runtime_error(msg);
            }
            if (apiUtil->commit_process(txn_m, tid, msg) == false)
            {
                throw new std::runtime_error(msg);
            }
            apiUtil->unlock_databaseinfo(db_info);
            db_info.reset();
            response.StatusMsg = "Transaction commit success.";
        }
        catch (const std::exception &e)
        {
            apiUtil->unlock_databaseinfo(db_info);
            string msg = "Transaction commit fail: " + string(e.what());
            response.Error(StatusOperationFailed, msg);
        }
    }

    void ApiHandler::rollback(shared_ptr<APIUtil>& apiUtil, const server::MessageCommitRequest& request, server::MessageResponse& response)
    {
        shared_ptr<DatabaseInfo> db_info = nullptr;
        try
        {
            std::string db_name = request.db_name;
            txn_id_t tid;
            if (transaction_check(apiUtil, db_name, request.tid, tid, response) == false)
            {
                return;
            }
            server::StatusCode statusCode;
            std::string msg;
            if (!apiUtil->validate_databaseinfo(request.db_name, db_info, statusCode, msg, true, DatabaseLock::W))
            {
                response.StatusCode = statusCode;
                response.StatusMsg = msg;
                return;
            }
            shared_ptr<Txn_manager> txn_m;
            if (apiUtil->get_txn_manager(db_name, txn_m) == false)
            {
                msg = "Get database transaction manager error.";
                throw new std::runtime_error(msg);
            }
            if (apiUtil->rollback_process(txn_m, tid, msg) == false)
            {
                throw new std::runtime_error(msg);
            }
            apiUtil->unlock_databaseinfo(db_info);
            response.StatusMsg = "Transaction rollback success.";
        }
        catch (const std::exception &e)
        {
            apiUtil->unlock_databaseinfo(db_info);
            string error = "Transaction rollback fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}