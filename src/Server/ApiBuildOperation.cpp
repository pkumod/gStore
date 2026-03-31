#include "ApiProvider.h"

namespace server
{
    bool ApiHandler::build_check(shared_ptr<APIUtil>& apiUtil, const MessageBuildRequest& request, MessageBuildResponse& response, std::map<std::string, unsigned long long>& file_paths, std::vector<std::string>& temp_paths)
    {
        std::string db_name = request.db_name;
        std::string msg;
        if (!request.db_path.empty())
        {
            bool db_paths_rt = true;
            for (auto& db_path_item : request.db_path) {
                std::string local_path = GlobalTypedef::upload_path();
                // download file from remote
                if (request.remote) 
                {
                    CURLcode rt = HttpUtil::DownloadFile(db_path_item, local_path);
                    if (rt != CURLcode::CURLE_OK || FileUtil::pathExists(local_path) == false) {
                        response.StatusMsg = "Download file '" + db_path_item + "' failed.";
                        response.StatusCode = StatusOperationFailed;
                        db_paths_rt = false;
                        break;
                    }
                    temp_paths.push_back(local_path);
                }
                else
                {
                    if (FileUtil::pathExists(db_path_item) == false)
                    {
                        response.StatusMsg = "RDF file '" + db_path_item + "' not exist.";
                        response.StatusCode = StatusOperationFailed;
                        db_paths_rt = false;
                        break;
                    }
                    local_path = db_path_item;
                }
                std::string file_name = FileUtil::fileName(local_path);
                std::string file_suffix = FileUtil::fileSuffix(local_path);
                bool is_zip = apiUtil->check_upload_allow_compress_packages(file_suffix);
                // check file suffix
                if (!apiUtil->check_upload_allow_extensions(file_suffix) &&  !is_zip)
                {
                    response.StatusMsg = "The file suffix '" + file_suffix + "' is not allowed.";
                    response.StatusCode = StatusParamIsIllegal;
                    db_paths_rt = false;
                    break;
                }
                if (is_zip)
                {
                    // uncompress zip
                    size_t pos = file_name.size() - file_suffix.size() - 1;
                    std::string uncompress_path = GlobalTypedef::upload_path() + file_name.substr(0, pos) + "_" + gs::TimeUtil::now();
                    temp_paths.push_back(uncompress_path);
                    bool unzip = uncompress_zip(apiUtil, local_path, file_paths, uncompress_path, response);
                    if (!unzip)
                    {
                        db_paths_rt = false;
                        break;
                    }
                }
                else
                {
                    // add file to file_paths
                    file_paths[local_path] = FileUtil::fileSize(local_path);
                }
            }
            if (!db_paths_rt)
            {
                // delete temp files: uncompress_path and download_path
                for (auto& path : temp_paths)
                {
                    if (path != GlobalTypedef::upload_path())
                        FileUtil::removePath(path);
                }
                return false;
            }
        }
        if (apiUtil->check_param_value("db_name", db_name, msg) == false)
        {
            response.StatusMsg = msg;
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }
        //check the db_name is system
        if (db_name == GlobalTypedef::system_db)
        {
            response.StatusMsg = "The database name can not be system.";
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }
        // check if database named [db_name] is already built
        if (apiUtil->check_db_built(db_name))
        {
            response.StatusMsg = "database already built.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }
        // check databse number
        if (apiUtil->check_db_count() == false)
        {
            response.StatusMsg = "The total number of databases more than max_databse_num.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }
        return true;
    }

    void ApiHandler::build(shared_ptr<APIUtil>& apiUtil, const MessageBuildRequest& request, MessageBuildResponse& response)
    {
        try
        {
            std::map<std::string, unsigned long long> file_paths; // local file path
            std::vector<std::string> temp_paths; // download files or uncompress files
            if (!build_check(apiUtil, request, response, file_paths, temp_paths))
                return;
            std::string db_name = request.db_name;
            std::string username = request.username;
            int64_t start_time = gs::TimeUtil::timestamp();
            std::vector<std::string> nt_files;
            if (!file_paths.empty()) 
            {
                nt_files.reserve(file_paths.size());
                for (const auto& pair : file_paths)
                    nt_files.push_back(pair.first);
            }
            apiUtil->init_databaseinfo(db_name, username, gs::TimeUtil::now(NORM_DATETIME_PATTERN), DatabaseStatus::BUILDING);
            SLOG_DEBUG("Import dataset to build database...");
            SLOG_DEBUG("db_name: " + db_name + "\tRDF_data file size: " << nt_files.size());
            string result;
            shared_ptr<Database> current_database = make_shared<Database>(db_name);
            shared_ptr<DatabaseInfo> current_db_info;
            apiUtil->get_databaseinfo(db_name, current_db_info);
            if(apiUtil->trywrlock_databaseinfo(current_db_info) == false)
            {
                response.StatusMsg = "unable to build due to loss of lock.";
                response.StatusCode = StatusLossOfLock;
                apiUtil->remove_databaseinfo(db_name, result);
                return;
            }
            current_db_info->setDatabase(current_database);
            bool flag = true;
            int nt_file_num = 0;
            int success_num = 0;
            std::string db_home_path = GlobalTypedef::db_path(db_name);
            try
            {
                if (!nt_files.empty())
                {
                    flag = current_database->build(nt_files);
                    success_num = current_database->getTripleNum();
                    nt_file_num = nt_files.size();
                }
                else
                {
                    // build empty database
                    flag = current_database->BuildEmptyDB();
                }
                current_db_info->setDatabase(nullptr);
                current_database.reset();
                if (!flag) 
                {
                    result = "build failed.";
                    throw std::runtime_error(result);
                }
                apiUtil->unlock_databaseinfo(current_db_info);
            }
            catch(const std::exception& e)
            {
                apiUtil->unlock_databaseinfo(current_db_info);
                apiUtil->remove_databaseinfo(db_name, result);
                FileUtil::removePath(db_home_path);
                remove_temp_files(temp_paths);
                result = "Import RDF file to database failed:" + string(e.what());
                response.StatusMsg = result;
                response.StatusCode = StatusOperationFailed;
                current_database.reset();
                SLOG_ERROR(result);
                return;
            }
            // init databaseinfo
            shared_ptr<DatabaseInfo> db_info;
            apiUtil->get_databaseinfo(db_name, db_info);
            apiUtil->trywrlock_databaseinfo(db_info);
            db_info->setStatus(DatabaseStatus::AREADY_BUILT);
            db_info->initDatabase();
            int64_t cost_time = gs::TimeUtil::timestamp() - start_time;
            db_info->success(cost_time);
            // init user privilege
            apiUtil->init_privilege(username, db_name);
            // add backup.log
            // Util::add_backuplog(db_name);
            // build response result
            result = "Import RDF file to database done.";
            string error_log = db_home_path + "/parse_error.log";
            size_t parse_error_num = FileUtil::fileLines(error_log);
            // exclude Info line
            if (parse_error_num > 0)
                parse_error_num = parse_error_num - nt_file_num;
            if (parse_error_num > 0)
            {
                SLOG_ERROR("RDF parse error num " + to_string(parse_error_num));
                SLOG_ERROR("See log file for details " + error_log);
            }
            apiUtil->unlock_databaseinfo(db_info);
            // remove temp files
            remove_temp_files(temp_paths);
            
            // Util::add_backuplog(db_name);
            response.StatusCode = StatusOK;
            response.StatusMsg = result;
            response.failed_num = parse_error_num;
            response.successNum = success_num;
        }
        catch (const std::exception &e)
        {
            std::string msg;
            if (apiUtil->check_db_built(request.db_name))
                apiUtil->remove_databaseinfo(request.db_name, msg);
            response.StatusMsg = "Build fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
        }
    }
}