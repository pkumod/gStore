#include "ApiProvider.h"

namespace server
{
    void ApiHandler::drop(shared_ptr<APIUtil>& apiUtil, const MessageDropRequest& request, MessageDropResponse& response)
    {
        try
        {
            std::string db_name = request.db_name;
            bool is_backup = request.is_backup;
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (apiUtil->remove_databaseinfo(db_name, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusOperationFailed;
                return;
            }
            SLOG_DEBUG("remove " + db_name + " from the already build database list success.");
            string db_path = GlobalTypedef::db_path(db_name);
            if (is_backup)
            {
                FileUtil::movePath(db_path, db_path+".bak");
                SLOG_DEBUG("bak_path: " + db_path + ".bak");
            }
            else
            {
                FileUtil::removePath(db_path);
                SLOG_DEBUG("remove_path: " + db_path);
            }
            string success = "Database " + db_name + " dropped.";
            response.StatusMsg = success;
            response.StatusCode = StatusOK;
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Drop fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::checkpoint(shared_ptr<APIUtil>& apiUtil, const server::MessageCheckPointRequest& request, server::MessageResponse& response)
    {
        std::string db_name = request.db_name;
        std::string msg;
        if (apiUtil->check_param_value("db_name", db_name, msg) == false)
        {
            response.Error(StatusParamIsIllegal, msg);
            return;
        }
        shared_ptr<DatabaseInfo> db_info;
        server::StatusCode statusCode;
        if (!apiUtil->validate_databaseinfo(request.db_name, db_info, statusCode, msg, true, DatabaseLock::W))
        {
            response.StatusCode = statusCode;
            response.StatusMsg = msg;
            return;
        }
        shared_ptr<Txn_manager> txn_m;
        if(apiUtil->get_txn_manager(db_name, txn_m) == false)
        {
            msg = "Get database transaction manager error.";
            apiUtil->unlock_databaseinfo(db_info);
            response.Error(StatusTranscationManageFailed, msg);
            return;
        }
        try
        {
            txn_m->Checkpoint();
            bool is_save = db_info->getDatabase()->save();
            if (!is_save)
            {
                msg = "disk or memory not enough.";
                throw std::runtime_error(msg);
            }
            apiUtil->unlock_databaseinfo(db_info);
            response.StatusMsg = "Database saved successfully.";
        }
        catch (const std::exception &e)
        {
            apiUtil->unlock_databaseinfo(db_info);
            string error = "Checkpoint fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}