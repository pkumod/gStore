#include "ApiProvider.h"

namespace server
{
    void ApiHandler::drop(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageDropRequest& resquest, MessageDropResponse& response)
    {
        try
        {
            std::string db_name = resquest.db_name;
            bool is_backup = resquest.is_backup;
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (apiUtil->check_db_built(db_name) == false)
            {
                response.StatusMsg = "the database [" + db_name + "] not built yet.";
                response.StatusCode = StatusParamIsIllegal;
                return;
            }
            if (apiUtil->check_db_loaded(db_name))
            {
                apiUtil->remove_txn_manager(db_name, false);
                SLOG_DEBUG("remove " + db_name + " from the txn managers.");
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
            clusterManagerPtr->addTask(ClusterTaskInfo(db_name, ClusterOperation_Drop));
            clusterManagerPtr->dropDb(db_name);
            response.StatusMsg = success;
            response.StatusCode = StatusOK;
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Drop fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::checkpoint(shared_ptr<APIUtil>& apiUtil, const server::MessageCheckPointRequest& resquest, server::MessageResponse& response)
    {
        try
        {
            std::string db_name = resquest.db_name;
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
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
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            if (apiUtil->trywrlock_databaseinfo(db_info) == false)
            {
                msg = "Unable to checkpoint due to loss of lock.";
                response.Error(StatusLossOfLock, msg);
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
            txn_m->Checkpoint();
            bool is_save = db_info->getDatabase()->save();
            if (!is_save)
            {
                apiUtil->unlock_databaseinfo(db_info);
                response.Error(StatusOperationFailed, "disk or memory not enough.");
                return;
            }
            apiUtil->unlock_databaseinfo(db_info);
            response.StatusMsg = "Database saved successfully.";
        }
        catch (const std::exception &e)
        {
            string error = "Checkpoint fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}