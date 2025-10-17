#include "ApiProvider.h"
#include "../Api/WFHttpUtil.h"

namespace server
{
    std::string ApiHandler::repalce_pfn_query(const std::string& sparql, MessageQueryResponse& response)
    {
        if (sparql.size() < 13)
            return sparql;
        std::string sparql_prefix = sparql.substr(0, 12);
        std::transform(sparql_prefix.begin(), sparql_prefix.end(), sparql_prefix.begin(), [](unsigned char c){return std::tolower(c);});
        if (sparql_prefix != "select (pfn.")
            return sparql;
        std::string pfn_sparql = "SELECT (PFN " + sparql.substr(12, sparql.size()-12);
        response.is_pfn = true;
        return pfn_sparql;
    }

    bool ApiHandler::query_check(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response)
    {
        int32_t min_memory = Util::getConfigureIntValue("min_memory", 512); // MB
        int32_t memoryLeft = gs::ResourceUtil::memoryLeft();
        if (memoryLeft < (min_memory >> 10))
        {
            response.StatusMsg = "memory not enough, available:" + std::to_string(memoryLeft) + "MB, need minimum:" + std::to_string(min_memory) + "MB";
            response.StatusCode = StatusOperationFailed;
            return false;
        }

        // check db_name paramter
        std::string msg;
        if (apiUtil->check_param_value("db_name", request.db_name, msg) == false)
        {
            response.StatusMsg = msg;
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }

        // check sparql paramter
        if (apiUtil->check_param_value("sparql", request.sparql, msg) == false)
        {
            response.StatusMsg = msg;
            response.StatusCode = StatusParamIsIllegal;
            return false;
        }

        // check database exist
        if (apiUtil->check_db_built(request.db_name) == false)
        {
            response.StatusMsg = "Database does not exist.";
            response.StatusCode = StatusOperationConditionsAreNotSatisfied;
            return false;
        }

        return true;
    }

    void ApiHandler::query_format_response_data(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response, ResultSet& rs)
    {
        response.fileName = "";
        if (request.format == "json")
        {
            rs.to_JSON(response.query_json);
        }
        else if (request.format == "file")
        {
            // file format does not limit the number
            rs.output_limit = -1;
            string file_name = request.db_name + "_" + response.threadId + "_" + gs::TimeUtil::now() + ".json";
            string file_path = GlobalTypedef::export_path + file_name;
            nlohmann::json json_data;
            rs.to_JSON(json_data);
            ofstream outfile;
            outfile.open(file_path);
            outfile << json_data.dump();
            outfile.close();
            response.fileName = file_name;
        }
        else if (request.format == "n-triple-file")
        {
            string file_name = request.db_name + "_" + response.threadId + "_" + gs::TimeUtil::now() + ".txt";
            string file_path = GlobalTypedef::export_path + file_name;
            FILE* fptr = fopen(file_path.c_str(), "w");
            rs.output_limit = -1;
            rs.output(fptr);
            fflush(fptr);
            fclose(fptr);
            response.fileName = file_name;
        }
        else if (request.format == "n-triple")
        {
            rs.to_NT_TRIPLE_JSON(response.query_json);
        }
        else
        {
            response.StatusMsg = "Unknown result format.";
            response.StatusCode = StatusOperationFailed;
        }
    }

    void ApiHandler::query_result_notify(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response)
    {
        int state = response.StatusCode == StatusOK ? 1:-1;
        std::string strPost;
        response.toJsonString(strPost);
        // save result to file
        string file_name = response.opt_id + ".json";
        string file_path = GlobalTypedef::export_path + file_name;
        ofstream outfile;
        outfile.open(file_path);
        outfile << strPost;
        outfile.close();
        // update access log
        apiUtil->update_access_log(response.StatusCode, response.StatusMsg, response.opt_id, state, response.ansNum, 0, file_path);
        // send result to callback
        if (!request.callback.empty()) 
        {
            std::string strResponse;
            HttpUtil::Post(request.callback, strPost, strResponse);
            SLOG_DEBUG("async query callback: " + request.callback + ", response: " + strResponse);
        }
        else
        {
            SLOG_DEBUG("async query no callback");
        }
        Task::TaskManager::finishTask(stoull(response.opt_id), file_name);
    }

    void ApiHandler::query(shared_ptr<APIUtil>& apiUtil, const MessageQueryRequest& request, MessageQueryResponse& response, const query_call& cb, bool format_check)
    {
        try
        {
            if (!query_check(apiUtil, request, response))
                return;
            std::string db_name = request.db_name;
            std::string username = request.username;
            std::string sparql = repalce_pfn_query(request.sparql, response);
            // check db_name paramter
            StatusCode statusCode;
            std::string msg;

            shared_ptr<DatabaseInfo> db_info;
            // check database read lock
            if (!apiUtil->validate_databaseinfo(db_name, db_info, statusCode, msg, true))
            {
                response.StatusMsg = msg;
                response.StatusCode = statusCode;
                return;
            }
            bool is_update = false;
            QueryTree::UpdateType update_type;
            bool update_flag_bool = apiUtil->check_privilege(username, "update", db_name);
            // check update operation
            try
            {
                is_update = db_info->getDatabase()->isUpdate(sparql, update_type);
            }
            catch(const std::exception& e)
            {
                apiUtil->unlock_databaseinfo(db_info);
                response.StatusMsg = "Unknown query type:" + string(e.what());
                response.StatusCode = StatusOperationFailed;
                return;
            }
            FILE *output = NULL;
            ResultSet rs;
            int ret_val;
            long query_time = gs::TimeUtil::timestamp();
            std::string query_start_time;
            try
            {
                if (!is_update)
                    rs.task = Task::TaskManager::addQueryTask(stoull(response.opt_id), "query", db_name, sparql, request.async);
                SLOG_DEBUG("begin query...\n" + sparql);
                rs.setUsername(username);
                query_start_time = gs::TimeUtil::now(NORM_DATETIME_MS_PATTERN);
                ret_val = db_info->getDatabase()->query(sparql, rs, output, update_flag_bool, false, nullptr, nullptr);
                query_time = gs::TimeUtil::timestamp() - query_time;
                // unlock rdlock
                apiUtil->unlock_databaseinfo(db_info);
            } catch (const std::exception &e) {
                apiUtil->unlock_databaseinfo(db_info);
                response.StatusMsg = "Query fail: " + string(e.what());
                response.StatusCode = StatusOperationFailed;
                return;
            }
            string query_time_s = to_string(query_time);
            long rs_ansNum = 0;
            response.threadId = gs::ThreadUtil::getThreadID();

            if (!is_update && (ret_val == -100))
            {
                rs_ansNum = max((long)rs.ansNum - rs.output_offset, 0L);
                long rs_outputlimit = (long)rs.output_limit;
                if (rs_outputlimit != -1)
                {
                    rs_ansNum = min(rs_ansNum, rs_outputlimit);
                }	

                int StatusCode = 0;
                // to void someone downloading all the data file by sparql query on purpose and to protect the data
                // if the ansNum too large, for example, larger than 100000, we limit the return ans.
                if (rs_ansNum > apiUtil->get_max_output_size())
                {
                    if (rs_outputlimit == -1 || rs_outputlimit > apiUtil->get_max_output_size())
                    {
                        rs_outputlimit = apiUtil->get_max_output_size();
                        rs.output_limit = rs_outputlimit;
                    }
                }
                response.StatusCode = StatusOK;
                response.StatusMsg = "success";
                response.ansNum = rs_ansNum;
                response.outputLimit = rs_outputlimit;
                response.queryTime = query_time_s;
                if (format_check)
                {
                    query_format_response_data(apiUtil, request, response, rs);
                }
                else
                {
                    rs.to_JSON(response.query_json);
                }
            }
            else if (is_update)
            {
                rs_ansNum = ret_val;
                SLOG_DEBUG("update query returns true. update num " + to_string(ret_val));
                response.StatusCode = StatusOK;
                response.StatusMsg = "update query returns true.";
                response.ansNum = rs_ansNum;
                response.queryTime = query_time_s;
                response.isUpdate = true;
            }
            else
            {
                msg = "query returns false.";
                SLOG_DEBUG(msg);
                response.StatusMsg = msg;
                response.StatusCode = StatusOperationFailed;   
            }
            // add callback task for query log start
            std::shared_ptr<DBQueryLogInfo> query_log_ptr = std::make_shared<DBQueryLogInfo>(query_start_time, request.remote_ip, sparql, 
                response.ansNum, request.format, response.fileName, response.StatusCode, response.StatusMsg, query_time, db_name);
            cb(query_log_ptr);
            // release ResultSet
            rs.release();
            return;
        }
        catch (const std::exception &e)
        {
            response.StatusMsg = "Query fail: " + string(e.what());
            response.StatusCode = StatusOperationFailed;
            return;
        }
    }
}