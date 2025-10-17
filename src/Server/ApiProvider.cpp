#include "ApiProvider.h"

namespace server
{
    bool ApiHandler::uncompress_zip(shared_ptr<APIUtil>& apiUtil, const std::string& file, std::map<std::string, unsigned long long>& uncompress_files, const std::string& uncompress_path, MessageResponse& response)
    {
        auto code = CompressUtil::FileHelper::foreachZip(file,[apiUtil](std::string filename)->bool
        {
            if( apiUtil->check_upload_allow_extensions(FileUtil::fileSuffix(filename)) == false )
                return false;
            return true;
        });
        if( code != CompressUtil::UnZipOK )
        {
            response.StatusMsg = "zip format or data is error.";
            response.StatusCode = code;
            return false;
        }
        if (!FileUtil::dirExists(uncompress_path))
            FileUtil::createDirs(uncompress_path);
        CompressUtil::UnCompressZip upfile(file, uncompress_path);
        code = upfile.unCompress();
        if (code != CompressUtil::UnZipOK)
        {
            response.StatusMsg = "uncompress failed(code="+to_string(code)+")";
            response.StatusCode = code;
            return false;
        }
        upfile.getFileList(uncompress_files);
        return true;
    }

    bool ApiHandler::remove_temp_files(std::vector<std::string>& temp_paths, const bool remove_parents_if_empty)
    {
        if (temp_paths.empty())
            return true;
        std::set<std::string> parent_paths;
        for (auto& temp_path : temp_paths)
        {
            if (FileUtil::pathExists(temp_path) && !FileUtil::isSameDir(temp_path, GlobalTypedef::upload_path()))
            {
                 FileUtil::removePath(temp_path);
            }
            if (remove_parents_if_empty)
            {
                string parent_path = FileUtil::parentPath(temp_path);
                if (FileUtil::isEmptyDir(parent_path) && !FileUtil::isSameDir(temp_path, GlobalTypedef::upload_path()))
                    parent_paths.insert(parent_path);
            }
        }
        std::vector<std::string> parent_paths_vec;
        for (auto& parent_path : parent_paths)
        {
            parent_paths_vec.push_back(parent_path);
        }
        return remove_temp_files(parent_paths_vec, remove_parents_if_empty);
    }

    void ApiHandler::load(shared_ptr<APIUtil>& apiUtil, const MessageLoadRequest& request, MessageLoadResponse& response)
    {
        try
        {
            std::string msg;
            if (apiUtil->check_param_value("db_name", request.db_name, msg) == false)
            {
                response.StatusCode = StatusParamIsIllegal;
                response.StatusMsg = msg;
                return;
            }
            shared_ptr<DatabaseInfo> db_info;
            if (!apiUtil->get_databaseinfo(request.db_name, db_info))
            {
                response.StatusCode = StatusOperationFailed;
                response.StatusMsg = "database[" + request.db_name + "] does not exist.";
                return;
            }
            if (db_info->getStatus() <= DatabaseStatus::LOADING)
            {
                response.StatusCode = StatusOperationFailed;
                response.StatusMsg = "database[" + request.db_name + "] is currently being " + db_info->getStatusDesc();
            }
            else if (db_info->isLoaded()) 
            {
                if (apiUtil->trywrlock_databaseinfo(db_info, 10))
                {
                    if (db_info->getDatabase()->csr)
                        response.csr = "1";
                    response.StatusCode = StatusOK;
                    response.StatusMsg = "The database already load yet.";
                    apiUtil->unlock_databaseinfo(db_info);
                }
                else
                {
                    response.StatusCode = StatusOperationFailed;
                    response.StatusMsg = "database[" + request.db_name + "] is currently being " + db_info->getStatusDesc();
                }
            }
            else
            {
                server::StatusCode statusCode;
                std::string statusMsg;
                if (!apiUtil->trywrlock_databaseinfo(db_info, 30))
                {
                    response.StatusCode = StatusLossOfLock;
                    response.StatusMsg = "database[" + request.db_name + "] try write lock fail";
                    SLOG_DEBUG(response.StatusMsg + " as it is currently being " + db_info->getStatusDesc());
                    return;
                }
                db_info->setStatus(DatabaseStatus::LOADING);
                // progress notification
                shared_ptr<Database> db_ptr =  db_info->getDatabase();
                SLOG_DEBUG("begin loading with csr: " << request.Csr() << " txn: " << request.txn);
                if (db_ptr && db_ptr->load(request.Csr(), request.txn))
                {
                    SLOG_DEBUG("end loading.");
                    bool schema_flag = GlobalTypedef::build_schema();
                    db_info->getDatabase()->setSchemaFlag(schema_flag);
                    db_info->setStatus(DatabaseStatus::LOADED);
                    // insert txn manager
                    apiUtil->insert_txn_manager(request.db_name, db_info);
                    if (db_info->getDatabase()->csr)
                    {
                        response.csr = "1";
                    }
                    response.StatusCode = StatusOK;
                    response.StatusMsg = "Database loaded successfully.";
                }
                else
                {
                    db_info->setStatus(DatabaseStatus::AREADY_BUILT);
                    response.StatusCode = StatusOperationFailed;
                    response.StatusMsg = "load failed.";
                }
                apiUtil->unlock_databaseinfo(db_info);
            }
        }
        catch (const std::exception &e)
        {
            response.StatusCode = StatusOperationFailed;
            response.StatusMsg = "load fail: " + string(e.what());
        }
    }

    void ApiHandler::loadCSR(shared_ptr<APIUtil>& apiUtil, const server::MessageLoadCSRRequest& request, server::MessageLoadCSRResponse& response)
    {
        try
        {
            std::string msg;
            if (apiUtil->check_param_value("db_name", request.db_name, msg) == false)
            {
                response.StatusCode = StatusParamIsIllegal;
                response.StatusMsg = msg;
                return;
            }
            shared_ptr<DatabaseInfo> db_info;
            if (!apiUtil->get_databaseinfo(request.db_name, db_info))
            {
                response.StatusCode = StatusOperationFailed;
                response.StatusMsg = "database[" + request.db_name + "] does not exist.";
                return;
            }
            if (db_info->getStatus() != DatabaseStatus::LOADED)
            {
                response.StatusCode = StatusOperationFailed;
                response.StatusMsg = "database[" + request.db_name + "] is currently being " + db_info->getStatusDesc();
            }
            else if (db_info->getDatabase()->csr) 
            {
                response.StatusCode = StatusOK;
                response.StatusMsg = "CSR already loaded.";
            }
            else
            {
                db_info->getDatabase()->loadCSR();
                response.StatusCode = StatusOK;
                response.StatusMsg = "CSR loaded successfully.";
            }
        }
        catch (const std::exception &e)
        {
            response.StatusCode = StatusOperationFailed;
            response.StatusMsg = "load csr fail: " + string(e.what());
        }
    }

    void ApiHandler::monitor(shared_ptr<APIUtil>& apiUtil, const MessageMonitorRequest& request, MessageMonitorResponse& response)
    {
        try
        {
            std::string db_name = request.db_name;
            std::string disk = request.disk;
            string db_path = GlobalTypedef::db_path(db_name);
            // check the param value is legal or not.
            std::string msg;
            if (apiUtil->check_param_value("db_name", db_name, msg) == false)
            {
                response.StatusMsg = msg;
                response.StatusCode = StatusOperationFailed;
                return;
            }
            shared_ptr<DatabaseInfo> database_info;
            server::StatusCode statusCode;
            std::string statusMsg;
            if (!apiUtil->validate_databaseinfo(request.db_name, database_info, statusCode, statusMsg, false))
            {
                response.StatusCode = statusCode;
                response.StatusMsg = statusMsg;
                return;
            }

            shared_ptr<Database> current_database = database_info->getDatabase();
            response.StatusCode = StatusOK;
            response.StatusMsg = "success";
            response.database = db_name;
            response.creator = database_info->getCreator();
            response.builtTime = database_info->getTime();
            response.connectionNum = apiUtil->get_connection_num();
            response.subjectList = current_database->getStatisticsInfo();
            current_database->getSchemaInfo(response.schema, true);

            unsigned long long triple_num = 0;
            unsigned int entityNum = 0;
            unsigned int subjectNum = 0;
            int predicateNum = 0;  
            unsigned int literalNum = 0;
            current_database->getDBMonitorInfo(triple_num, entityNum, subjectNum, predicateNum, literalNum);
            response.tripleNum = std::to_string(triple_num);
            response.entityNum = entityNum;
            response.subjectNum = subjectNum;
            response.predicateNum = predicateNum;
            response.literalNum = literalNum;
            response.costTime = database_info->getCostTime();
            response.status = database_info->getStatusStr();
            
            apiUtil->unlock_databaseinfo(database_info);
            response.lockNum = database_info->lock_count.load();
            unsigned diskUsed = 0;
            if (disk != "0") 
            {
                string real_path = Util::getExactPath(db_path.c_str());
                if (!real_path.empty()) {
                    uint64_t count_size_byte = FileUtil::dirSize(real_path.c_str());
                    // byte to MB
                    diskUsed = count_size_byte>>20;
                }
            }
            response.diskUsed = diskUsed;
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Monitor fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }
}