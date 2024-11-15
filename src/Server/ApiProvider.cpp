#include "ApiProvider.h"

namespace server
{
    bool ApiHandler::stringIsTrue(std::string value)
    {
        if (value.empty())
            return false;
        else if (value == "1")
            return true;
        else if (value == "true")
            return true;
        else if (value == "bool")
            return true;
        return false;
    }

    bool ApiHandler::uncompress_zip(shared_ptr<APIUtil>& apiUtil, const std::string& file, std::vector<std::string>& nt_files, std::string& unz_dir_path, MessageResponse& response)
    {
        auto code = CompressUtil::FileHelper::foreachZip(file,[apiUtil](std::string filename)->bool
        {
            if( apiUtil->check_upload_allow_extensions(Util::fileSuffix(filename)) == false )
                return false;
            return true;
        });
        if( code != CompressUtil::UnZipOK )
        {
            response.StatusMsg = "uncompress is failed error.";
            response.StatusCode = code;
            return false;
        }
        std::string file_name = Util::fileName(file);
        size_t pos = file_name.size() - Util::fileSuffix(file).size() - 1;
        unz_dir_path = GlobalTypedef::upload_path() + file_name.substr(0, pos) + "_" + gutil::TimeUtil::now();
        Util::create_dirs(unz_dir_path);
        CompressUtil::UnCompressZip upfile(file, unz_dir_path);
        code = upfile.unCompress();
        if (code != CompressUtil::UnZipOK)
        {
            Util::remove_path(unz_dir_path);
            response.StatusMsg = "uncompress is failed error.";
            response.StatusCode = code;
            return false;
        }
        upfile.getFileList(nt_files, "");

        return true;
    }

    bool ApiHandler::uncompress_zip(shared_ptr<APIUtil>& apiUtil, const std::string& file, std::vector<std::string>& nt_files, std::string& unz_dir_path, MessageResponse& response, std::string& max_file)
    {
        auto code = CompressUtil::FileHelper::foreachZip(file,[apiUtil](std::string filename)->bool
        {
            if( apiUtil->check_upload_allow_extensions(Util::fileSuffix(filename)) == false )
                return false;
            return true;
        });
        if( code != CompressUtil::UnZipOK )
        {
            response.StatusMsg = "uncompress is failed error.";
            response.StatusCode = code;
            return false;
        }
        std::string file_name = Util::fileName(file);
        size_t pos = file_name.size() - Util::fileSuffix(file).size() - 1;
        unz_dir_path = GlobalTypedef::upload_path() + file_name.substr(0, pos) + "_" + gutil::TimeUtil::now();
        Util::create_dirs(unz_dir_path);
        CompressUtil::UnCompressZip upfile(file, unz_dir_path);
        code = upfile.unCompress();
        if (code != CompressUtil::UnZipOK)
        {
            Util::remove_path(unz_dir_path);
            response.StatusMsg = "uncompress is failed error.";
            response.StatusCode = code;
            return false;
        }
        max_file = upfile.getMaxFilePath();
        upfile.getFileList(nt_files, max_file);
        
        return true;
    }

    void ApiHandler::load(shared_ptr<APIUtil>& apiUtil, const MessageLoadRequest& resquest, MessageLoadResponse& response)
    {
        try
        {
            std::string msg;
            if (apiUtil->check_param_value("db_name", resquest.db_name, msg) == false)
            {
                response.StatusCode = StatusParamIsIllegal;
                response.StatusMsg = msg;
                return;
            }
            if (!apiUtil->check_db_built(resquest.db_name))
            {
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                response.StatusMsg = "The database [" + resquest.db_name + "] not built yet.";
                return;
            }
            shared_ptr<DatabaseInfo> current_database;
            apiUtil->get_databaseinfo(resquest.db_name, current_database);
            if (current_database->getStatus() != DatabaseStatus::LOADED)
            {
                if (!apiUtil->trywrlock_databaseinfo(current_database))
                {
                    response.StatusCode = StatusLossOfLock;
                    response.StatusMsg = "Unable to load due to loss of lock."; 
                    return;
                }
                current_database->setStatus(DatabaseStatus::LOADING);
                SLOG_DEBUG("begin loading...");
                // progress notification
                bool rt  = current_database->getDatabase()->load(resquest.Csr());
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
                    response.StatusCode = StatusOK;
                    response.StatusMsg = "Database loaded successfully.";
                    response.csr = csr_str;
                }
                else
                {
                    current_database->setStatus(DatabaseStatus::AREADY_BUILT);
                    apiUtil->unlock_databaseinfo(current_database);
                    response.StatusCode = StatusOperationFailed;
                    response.StatusMsg = "load failed: unknow error.";
                }
            }
            else
            {
                std::string csr_str = "0";
                if (current_database->getDatabase()->csr != NULL)
                {
                    csr_str = "1";
                }
                response.StatusCode = StatusOK;
                response.StatusMsg = "The database already load yet.";
                response.csr = csr_str;
            }
        }
        catch (const std::exception &e)
        {
            response.StatusCode = StatusOperationFailed;
            response.StatusMsg = "load fail: " + string(e.what());
        }
    }

    void ApiHandler::monitor(shared_ptr<APIUtil>& apiUtil, std::shared_ptr<cluster::ClusterManager>& clusterManagerPtr, const MessageMonitorRequest& resquest, MessageMonitorResponse& response)
    {
        try
        {
            std::string db_name = resquest.db_name;
            std::string disk = resquest.disk;
            std::string _db_home = GlobalTypedef::db_home();
            std::string _db_suffix = GlobalTypedef::db_suffix();
            // check the param value is legal or not.
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusOperationFailed;
                return;
            }
            if (apiUtil->check_db_built(db_name) == false)
            {
                response.StatusMsg = "the database [" + db_name + "] not built yet.";
                response.StatusCode = StatusOperationConditionsAreNotSatisfied;
                return;
            }
            shared_ptr<DatabaseInfo> database_info;
            apiUtil->get_databaseinfo(db_name, database_info);
            if (apiUtil->rdlock_databaseinfo(database_info) == false)
            {
                response.StatusMsg = "Unable to monitor due to loss of lock";
                response.StatusCode = StatusLossOfLock;
                return;
            }
            shared_ptr<Database> current_database = database_info->getDatabase();
            current_database->loadDBInfoFile();
            current_database->loadStatisticsInfoFile();
            apiUtil->unlock_databaseinfo(database_info);
            std::string creator = database_info->getCreator();
            std::string time = database_info->getTime();
            unordered_map<string, unsigned long long> umap = current_database->getStatisticsInfo();
            unsigned diskUsed = 0;
            if (disk != "0") 
            {
                string db_path = _db_home + db_name + _db_suffix;
                string real_path = Util::getExactPath(db_path.c_str());
                if (!real_path.empty()) {
                    long long unsigned count_size_byte = Util::count_dir_size(real_path.c_str());
                    // byte to MB
                    diskUsed = count_size_byte>>20;
                }
            }
            response.StatusCode = StatusOK;
            response.StatusMsg = "success";
            response.database = db_name;
            response.creator = creator;
            response.builtTime = time;
            response.tripleNum = std::to_string(current_database->getTripleNum());
            response.entityNum = current_database->getEntityNum();
            response.literalNum = current_database->getLiteralNum();
            response.subjectNum = current_database->getSubNum();
            response.predicateNum = current_database->getPreNum();
            response.connectionNum = apiUtil->get_connection_num();
            response.diskUsed = diskUsed;
            response.subjectList = umap;
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Monitor fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }
}