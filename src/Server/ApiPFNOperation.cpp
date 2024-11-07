#include "ApiProvider.h"

namespace server
{
    void ApiHandler::funquery(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageFunQueryResponse& response, rapidjson::Document& json_data)
    {
        try
        {
            struct PFNInfo pfn_info;
            if (hasJsonParam(json_data, "funInfo"))
            {
                rapidjson::Value &fun_info = json_data["funInfo"];
                pfnUtil->build_PFNInfo(fun_info, &pfn_info);
            }
            std::string username =  jsonParam(json_data, "username");
            struct PFNInfos *pfn_infos = new PFNInfos();
            pfnUtil->fun_query(pfn_info.getFunName(), pfn_info.getFunStatus(), username, pfn_infos);
            vector<struct PFNInfo> list = pfn_infos->getPFNInfoList();
            size_t count = list.size();

            nlohmann::json info;
            for (size_t i = 0; i < count; i++)
            {
                PFNInfo pfn_info = list[i];
                pfn_info.toJSON(info);
                response.list.push_back(info);
            }
            response.StatusMsg = "success";
        }
        catch (const std::exception &e)
        {
            std::string error = "Fun query fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::funcudb(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageFunCudbResponse& response, rapidjson::Document& json_data)
    {
        std::string type = jsonParam(json_data, "type");
        std::string msg;
        if (apiUtil->check_param_value("type", type, msg) == false)
        {
            response.Error(StatusParamIsIllegal, msg);
            return;
        }
        if (hasJsonParam(json_data, "funInfo") == false)
        {
            msg =  "the value of funInfo can not be empty!";
            response.Error(StatusParamIsIllegal, msg);
            return;
        }
        std::string username = jsonParam(json_data, "username");
        struct PFNInfo pfn_info;
        rapidjson::Value &fun_info = json_data["funInfo"];
        pfnUtil->build_PFNInfo(fun_info, &pfn_info);
        if (type == "1")
        {
            try
            {
                pfnUtil->fun_create(username, &pfn_info);
                response.StatusMsg = "Function create success.";
            }
            catch(const std::exception& e)
            {
                msg = "Function create fail: " + string(e.what());
                response.Error(StatusOperationFailed, msg);
            }
        }
        else if (type == "2")
        {
            try
            {
                pfnUtil->fun_update(username, &pfn_info);
                response.StatusMsg = "Function update success.";
            }
            catch(const std::exception& e)
            {
                msg = "Function update fail: " + string(e.what());
                response.Error(StatusOperationFailed, msg);
            }
        }
        else if (type == "3")
        {
            try
            {
                pfnUtil->fun_delete(username, &pfn_info);
                response.StatusMsg = "Function delete success.";
            }
            catch(const std::exception& e)
            {
                msg = "Function delete fail: " + string(e.what());
                response.Error(StatusOperationFailed, msg);
            }
        }
        else if (type == "4")
        {
            try
            {
                string result = pfnUtil->fun_build(username, pfn_info.getFunName());
                if (result == "")
                {
                    response.StatusMsg = "Function build success.";
                }
                else
                {
                    response.Error(StatusOperationFailed, result);
                }
            }
            catch(const std::exception& e)
            {
                msg = "Function build fail: " + string(e.what());
                response.Error(StatusOperationFailed, msg);
            }
        }
        else
        {
            msg = "The type is invalid, please look up the api document.";
            response.Error(StatusParamIsIllegal, msg);
        }
    }

    void ApiHandler::funreview(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageReviewResponse& response, rapidjson::Document& json_data)
    {
        try
        {
            std::string msg;
            if (hasJsonParam(json_data, "funInfo") == false)
            {
                msg =  "the value of funInfo can not be empty!";
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            std::string username = jsonParam(json_data, "username");
            struct PFNInfo pfn_info;
            rapidjson::Value &fun_info = json_data["funInfo"];
            pfnUtil->build_PFNInfo(fun_info, &pfn_info);
            pfnUtil->fun_review(username, &pfn_info);
            string content = pfn_info.getFunBody();
            content = gutil::StringUtil::url_encode(content);
            response.StatusMsg = "success";
            response.result = content;
        }
        catch (const std::exception &e)
        {
            std::string error = "Function review fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}