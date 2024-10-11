#include "ApiProvider.h"

namespace server
{
    void ApiHandler::parseRequest(const grpc::GRPCReq *request, nlohmann::json &json_data)
    {
    }

    void ApiHandler::load(shared_ptr<APIUtil>& apiUtil, const MessageLoadRequest& resquest, MessageLoadResponse& response)
    {
        try
        {
            std::string msg;
            if (apiUtil->check_param_value("db_name", resquest.db_name, msg) == false)
            {
                response.status_code = StatusParamIsIllegal;
                response.status_msg = msg;
                return;
            }
            if (!apiUtil->check_db_built(resquest.db_name))
            {
                response.status_code = StatusOperationConditionsAreNotSatisfied;
                response.status_msg = "The database [" + resquest.db_name + "] not built yet.";
                return;
            }
            shared_ptr<DatabaseInfo> current_database;
            apiUtil->get_databaseinfo(resquest.db_name, current_database);
            if (current_database->getStatus() != DatabaseStatus::LOADED)
            {
                if (!apiUtil->trywrlock_databaseinfo(current_database))
                {
                    response.status_code = StatusLossOfLock;
                    response.status_msg = "Unable to load due to loss of lock."; 
                    return;
                }
                current_database->setStatus(DatabaseStatus::LOADING);
                SLOG_DEBUG("begin loading...");
                // progress notification
                bool rt  = current_database->getDatabase()->load(resquest.csr);
                SLOG_DEBUG("end loading.");
                if (rt)
                {
                    current_database->setStatus(DatabaseStatus::LOADED);
                    // insert txn manager
                    apiUtil->insert_txn_manager(resquest.db_name, current_database);
                    std::string csr_str = "0";
                    if (current_database->getDatabase()->csr != NULL)
                    {
                        csr_str = "1";
                    }
                    apiUtil->unlock_databaseinfo(current_database);
                    response.status_code = StatusOK;
                    response.status_msg = "Database loaded successfully.";
                    response.csr = csr_str;
                }
                else
                {
                    current_database->setStatus(DatabaseStatus::AREADY_BUILT);
                    apiUtil->unlock_databaseinfo(current_database);
                    response.status_code = StatusOperationFailed;
                    response.status_msg = "load failed: unknow error.";
                }
            }
            else
            {
                std::string csr_str = "0";
                if (current_database->getDatabase()->csr != NULL)
                {
                    csr_str = "1";
                }
                response.status_code = StatusOK;
                response.status_msg = "The database already load yet.";
                response.csr = csr_str;
            }
        }
        catch (const std::exception &e)
        {
            response.status_code = StatusOperationFailed;
            response.status_msg = "load fail: " + string(e.what());
        }
    }
}