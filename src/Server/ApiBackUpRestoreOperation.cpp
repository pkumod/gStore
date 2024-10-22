#include "ApiProvider.h"

namespace server
{
    void ApiHandler::backup(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupRequest& resquest, server::MessageBackupResponse& response, const backup_call& async_cb)
    {
        try
        {
            std::string db_name = resquest.db_name;
            std::string backup_path = resquest.backup_path;
            bool compress = resquest.backup_zip;
            std::string msg;
             std::string _db_home = Util::getConfigureValue("db_home");
            std::string _db_suffix = Util::getConfigureValue("db_suffix");
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            if (apiUtil->check_db_built(db_name) == false)
            {
                msg = "the database [" + db_name + "] not built yet.";
                response.Error(StatusOperationConditionsAreNotSatisfied, msg);
                return;
            }
            // check backup path
            if (backup_path.empty())
            {
                backup_path = Util::backup_path;
                SLOG_DEBUG("backup_path is empty, set to default path: " + backup_path);
            }
            if (backup_path == "." || backup_path == "./" || Util::getExactPath(backup_path.c_str()) == Util::getExactPath(_db_home.c_str()))
            {
                msg = "Backup path can not be root or \"" + _db_home + "\" .";
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            string remote_ip = resquest.remote_ip;
            bool async = resquest.async;
            std::string callback = resquest.callback;
            std::string operation = "backup";
            std::string opt_id = apiUtil->generateUid();
            response.opt_id = opt_id;
            if (async)
            {
                async_cb(opt_id);
                msg = "Operation success";
                apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
                response.StatusCode = StatusOK;
                response.StatusMsg = msg;
            }
            else
            {
                bool backup_rt = apiUtil->backup_databaseinfo(db_name, compress, backup_path, msg);
                if (backup_rt)
                {
                    msg = "Database backup successfully.";
                    apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
                    response.StatusCode = StatusOK;
                    response.StatusMsg = msg;
                    response.backupfilepath = backup_path;
                }
                else
                {
                    apiUtil->write_access_log(operation, remote_ip, StatusOperationFailed, msg, opt_id);
                    response.Error(StatusOperationFailed, msg);
                }
            }
        }
        catch (const std::exception &e)
        {
            std::string error = "Backup fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::backup(shared_ptr<APIUtil>& apiUtil, const std::string& opt_id, const std::string& db_name, std::string& backup_path, bool compress, const std::string& callback)
    {
        std::string msg;
        bool backup_rt = apiUtil->backup_databaseinfo(db_name, compress, backup_path, msg);
        nlohmann::json j = {
            {"opt_id", opt_id}
        };
        if (backup_rt)
        {
            j["StatusCode"] = 0;
            j["StatusMsg"] = "Backup success";
            j["backupfilepath"] = backup_path;
            apiUtil->update_access_log(StatusOK, msg, opt_id, 0, 0, 0, backup_path);
        }
        else
        {
            j["StatusCode"] = StatusOperationFailed;
            j["StatusMsg"] = msg;
            apiUtil->update_access_log(StatusOperationFailed, msg, opt_id, -1, 0, 0);
        }
        if (!callback.empty())
        {
            HttpUtil::Post(callback, j.dump(), msg);
        }
    }

    void ApiHandler::backup_path(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupPathRequest& resquest, server::MessageBackupPathResponse& response)
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
            std::vector<std::string> file_list;
            string backup_path = Util::backup_path;
            Util::dir_files(backup_path, db_name, file_list);
            Document resp_data;
            Document pathsDoc;
            resp_data.SetObject();
            pathsDoc.SetArray();
            Document::AllocatorType &allocator = resp_data.GetAllocator();

            for (size_t i = 0; i < file_list.size(); i++)
            {
                response.paths.push_back(backup_path + file_list[i]);
            }
            response.StatusCode = StatusOK;
            response.StatusMsg = "success";
        }
        catch (const std::exception &e)
        {
            std::string error = "Query backup path fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::restore(shared_ptr<APIUtil>& apiUtil, const server::MessageRestoreRequest& resquest, server::MessageRestoreResponse& response, const restore_call& cb)
    {
        try
        {
            std::string db_name = resquest.db_name;
            std::string backup_path = resquest.backup_path;
            std::string username = resquest.username;
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            if (apiUtil->check_param_value("backup_path", backup_path, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            if (Util::is_file(backup_path)) {
                if (Util::fileSuffix(backup_path) != "zip") {
                    response.Error(StatusParamIsIllegal, "Backup file is not zip file.");
                    return;
                } else if (Util::file_exist(backup_path) == false) {
                    response.Error(StatusParamIsIllegal, "Backup file not exist.");
                    return;
                }
            } else if (Util::is_dir(backup_path)) {
                if (Util::dir_exist(backup_path) == false) {
                    response.Error(StatusParamIsIllegal, "Backup path not exist.");
                    return;
                }
            } else {
                response.Error(StatusParamIsIllegal, "Backup path not exist.");
                return;
            }
            
            std::string opt_id = apiUtil->generateUid();
            string remote_ip = resquest.remote_ip;
            string operation = "restore";
            bool async = resquest.async;
            std::string callback = resquest.callback;
            
            response.opt_id = opt_id;
            if (async)
            {
                cb(opt_id);
                msg = "Operation success";
                apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
                response.StatusCode = 0;
                response.StatusMsg = msg;
            }
            else
            {
                bool backup_rt = apiUtil->restore_databaseinfo(username, db_name, backup_path, msg);
                if (backup_rt)
                {
                    msg = "Database restore successfully.";
                    apiUtil->write_access_log(operation, remote_ip, 0, msg, opt_id);
                    response.StatusCode = 0;
                    response.StatusMsg = msg;
                }
                else
                {
                    apiUtil->write_access_log(operation, remote_ip, StatusOperationFailed, msg, opt_id);
                    response.Error(StatusOperationFailed, msg);
                }
            }
        }
        catch (const std::exception &e)
        {
            std::string error = "Restore fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::restore(shared_ptr<APIUtil>& apiUtil, const std::string& opt_id, const std::string& db_name, const std::string& username, std::string& backup_path, const std::string& callback)
    {
        std::string msg;
        bool restore_rt = apiUtil->restore_databaseinfo(username, db_name, backup_path, msg);
        nlohmann::json j = {
            {"opt_id", opt_id}
        };
        if (restore_rt)
        {
            msg = "Restore success";
            j["StatusCode"] = 0;
            j["StatusMsg"] = msg;
            j["backupfilepath"] = backup_path;
            apiUtil->update_access_log(StatusOK, msg, opt_id, 0, 0, 0, backup_path);
        }
        else
        {
            j["StatusCode"] = StatusOperationFailed;
            j["StatusMsg"] = msg;
            apiUtil->update_access_log(StatusOperationFailed, msg, opt_id, -1, 0, 0);
        }
        if (!callback.empty())
        {
            HttpUtil::Post(callback, j.dump(), msg);
        }
    }

    void ApiHandler::export_db(shared_ptr<APIUtil>& apiUtil, const server::MessageExportRequest& resquest, server::MessageExportResponse& response)
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
            std::string db_path = resquest.db_path;
            if (apiUtil->check_param_value("db_path", db_path, msg) == false)
            {
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            if (apiUtil->check_db_built(db_name) == false)
            {
                msg = "Database not build yet.";
                response.Error(StatusOperationConditionsAreNotSatisfied, msg);
                return;
            }
            // check if database named [db_name] is already load
            if (apiUtil->check_db_loaded(db_name) == false)
            {
                msg = "Database not load yet.";
                response.Error(StatusOperationConditionsAreNotSatisfied, msg);
                return;
            }
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            if (apiUtil->rdlock_databaseinfo(db_info) == false)
            {
                msg = "get current database read lock fail.";
                response.Error(StatusLossOfLock, msg);
                return;
            }
            Util::string_suffix(db_path, '/');
            if (Util::dir_exist(db_path) == false)
            {
                Util::create_dirs(db_path);
            }
            std::string export_path = db_path + db_name + "_" + Util::get_timestamp() + ".nt";
            bool compress = resquest.compress;
            SLOG_DEBUG("export_path: " << export_path << " ,compress:" << compress);
            FILE *ofp = fopen(export_path.c_str(), "w");
            db_info->getDatabase()->export_db(ofp);
            fflush(ofp);
            fclose(ofp);
            ofp = NULL;
            // unlock
            apiUtil->unlock_databaseinfo(db_info);
            if (compress)
            {
                std::string zip_path = db_path + db_name + "_" + Util::get_timestamp() + ".zip";
                if (!CompressUtil::FileHelper::compressExportZip(export_path, zip_path))
                {
                    Util::remove_path(export_path);
                    Util::remove_path(zip_path);
                    msg = "export compress fail.";
                    response.Error(StatusCompressError, msg);
                    return;
                }
                Util::remove_path(export_path);
                export_path = zip_path;
            }
            msg = "Export the database successfully.";
            response.StatusCode = 0;
            response.StatusMsg = msg;
            response.filepath = export_path;
        }
        catch (const std::exception &e)
        {
            std::string error = "Export fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}