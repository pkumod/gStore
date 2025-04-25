#include "ApiProvider.h"

namespace server
{
    bool ApiHandler::backup_check(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupRequest& request, server::MessageBackupResponse& response, std::string& backup_path)
    {
        std::string _db_home = GlobalTypedef::db_home();
        std::string db_name = request.db_name;
        std::string msg;
        if (apiUtil->check_param_value("db_name", db_name, msg) == false)
        {
            response.Error(StatusParamIsIllegal, msg);
            return false;
        }
        if (apiUtil->check_db_built(db_name) == false)
        {
            msg = "the database [" + db_name + "] not built yet.";
            response.Error(StatusOperationConditionsAreNotSatisfied, msg);
            return false;
        }
        // check backup path
        if (backup_path.empty())
        {
            backup_path = GlobalTypedef::backup_path();
            SLOG_DEBUG("backup_path is empty, set to default path: " + backup_path);
        }
        if (backup_path == "." || backup_path == "./" || Util::getExactPath(backup_path.c_str()) == Util::getExactPath(_db_home.c_str()))
        {
            msg = "Backup path can not be root or \"" + _db_home + "\" .";
            response.Error(StatusParamIsIllegal, msg);
            return false;
        }

        return true;
    }

    void ApiHandler::backup(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupRequest& request, server::MessageBackupResponse& response)
    {
        try
        {
            std::string backup_path = request.backup_path;
            if (!backup_check(apiUtil, request, response, backup_path))
                return;

            std::string msg;
            bool backup_rt = apiUtil->backup_databaseinfo(request.db_name, request.backup_zip, backup_path, msg);
            if (backup_rt)
            {
                msg = "Database backup successfully.";
                response.StatusCode = StatusOK;
                response.StatusMsg = msg;
                response.backupfilepath = backup_path;
            }
            else
            {
                response.Error(StatusOperationFailed, msg);
            }
        }
        catch (const std::exception &e)
        {
            std::string error = "Backup fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::backup_async(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupRequest& request, server::MessageBackupResponse& response)
    {
        try
        {
            std::string opt_id = response.opt_id;
            std::string backup_path = request.backup_path;
            if (!backup_check(apiUtil, request, response, backup_path))
            {
                apiUtil->update_access_log(StatusOK, response.StatusMsg, opt_id, -1, 0, 0, backup_path);
                return;
            }

            std::string msg;
            bool backup_rt = apiUtil->backup_databaseinfo(request.db_name, request.backup_zip, backup_path, msg);
            nlohmann::json j = {
                {"opt_id", opt_id}
            };
            if (backup_rt)
            {
                msg = "Backup success";
                j["StatusCode"] = 0;
                j["StatusMsg"] = msg;
                j["backupfilepath"] = backup_path;
                apiUtil->update_access_log(StatusOK, msg, opt_id, 1, 0, 0, backup_path);
            }
            else
            {
                j["StatusCode"] = StatusOperationFailed;
                j["StatusMsg"] = msg;
                apiUtil->update_access_log(StatusOperationFailed, msg, opt_id, -1, 0, 0);
            }
            std::string callback = request.callback;
            if (!callback.empty())
            {
                HttpUtil::Post(callback, j.dump(), msg);
            }
        }
        catch (const std::exception &e)
        {
            std::string error = "Backup fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::backup_path(shared_ptr<APIUtil>& apiUtil, const server::MessageBackupPathRequest& request, server::MessageBackupPathResponse& response)
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
            std::vector<std::string> file_list;
            string backup_path = GlobalTypedef::backup_path();
            FileUtil::dir_filenames(backup_path, file_list, db_name + GlobalTypedef::db_suffix());
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

    bool ApiHandler::restore_check(shared_ptr<APIUtil>& apiUtil, const server::MessageRestoreRequest& request, server::MessageRestoreResponse& response)
    {
        std::string msg;
        std::string backup_path = request.backup_path;
        if (apiUtil->check_param_value("db_name", request.db_name, msg) == false)
        {
            response.Error(StatusParamIsIllegal, msg);
            return false;
        }
        if (apiUtil->check_param_value("backup_path", backup_path, msg) == false)
        {
            response.Error(StatusParamIsIllegal, msg);
            return false;
        }
        if (FileUtil::is_file(backup_path))
        {
            if (FileUtil::fileSuffix(backup_path) != "zip")
            {
                response.Error(StatusParamIsIllegal, "Backup file is not zip file.");
                return false;
            }
        }
        else if (!FileUtil::dirExists(backup_path))
        {
            response.Error(StatusParamIsIllegal, "Backup path not exist.");
            return false;
        }
        return true;
    }

    void ApiHandler::restore(shared_ptr<APIUtil>& apiUtil, const server::MessageRestoreRequest& request, server::MessageRestoreResponse& response)
    {
        try
        {
            if (!restore_check(apiUtil, request, response))
                return;
            
            std::string backup_path = request.backup_path;
            std::string msg;
            bool backup_rt = apiUtil->restore_databaseinfo(request.username, request.db_name, backup_path, msg);
            if (backup_rt)
            {
                response.StatusCode = 0;
                response.StatusMsg = "Database restore successfully.";
            }
            else
            {
                response.Error(StatusOperationFailed, msg);
            }
        }
        catch (const std::exception &e)
        {
            std::string error = "Restore fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
    void ApiHandler::restore_async(shared_ptr<APIUtil>& apiUtil, const server::MessageRestoreRequest& request, server::MessageRestoreResponse& response)
    {
        std::string opt_id = response.opt_id;
        if (!restore_check(apiUtil, request, response))
        {
            apiUtil->update_access_log(response.StatusCode, response.StatusMsg, opt_id, -1, 0, 0);
            return;
        }

        std::string backup_path = request.backup_path;
        std::string msg;
        bool restore_rt = apiUtil->restore_databaseinfo(request.username, request.db_name, backup_path, msg);
        nlohmann::json j = {
            {"opt_id", opt_id}
        };
        if (restore_rt)
        {
            msg = "Restore success";
            j["StatusCode"] = 0;
            j["StatusMsg"] = msg;
            j["backupfilepath"] = backup_path;
            apiUtil->update_access_log(StatusOK, msg, opt_id, 1, 0, 0, backup_path);
        }
        else
        {
            j["StatusCode"] = StatusOperationFailed;
            j["StatusMsg"] = msg;
            apiUtil->update_access_log(StatusOperationFailed, msg, opt_id, -1, 0, 0);
        }
        std::string callback = request.callback;
        if (!callback.empty())
        {
            HttpUtil::Post(callback, j.dump(), msg);
        }
    }

    void ApiHandler::export_db(shared_ptr<APIUtil>& apiUtil, const server::MessageExportRequest& request, server::MessageExportResponse& response)
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
            std::string db_path = request.db_path;
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
            gutil::StringUtil::append(db_path, '/');
            if (FileUtil::dirExists(db_path) == false)
            {
                FileUtil::createDirs(db_path);
            }
            std::string export_path = db_path + db_name + "_" + gutil::TimeUtil::now() + ".nt";
            bool compress = request.compress;
            SLOG_DEBUG("export_path:" << export_path << ", compress:" << compress);
            FILE *ofp = fopen(export_path.c_str(), "w");
            db_info->getDatabase()->export_db(ofp);
            fflush(ofp);
            fclose(ofp);
            ofp = NULL;
            // unlock
            apiUtil->unlock_databaseinfo(db_info);
            if (compress)
            {
                std::string zip_path = db_path + db_name + "_" + gutil::TimeUtil::now() + ".zip";
                if (!CompressUtil::FileHelper::compressExportZip(export_path, zip_path, false))
                {
                    FileUtil::removePath(export_path);
                    FileUtil::removePath(zip_path);
                    msg = "export compress fail.";
                    response.Error(StatusCompressError, msg);
                    return;
                }
                FileUtil::removePath(export_path);
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