#include "ApiProvider.h"

namespace server
{
    void ApiHandler::funquery(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageFunQueryRequest& request, server::MessageFunQueryResponse& response)
    {
        try
        {
            PFNInfo funInfo = request.funInfo;
            std::string username = request.username;
            std::shared_ptr<PFNInfos> pfn_infos = std::make_shared<PFNInfos>();
            pfnUtil->fun_query(funInfo.funName, funInfo.funStatus, username, pfn_infos);
            response.list = pfn_infos->getPFNInfoList();
            response.StatusMsg = "success";
        }
        catch (const std::exception &e)
        {
            std::string error = "Fun query fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }

    void ApiHandler::funcudb(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageFunCudbRequest& request, server::MessageFunCudbResponse& response)
    {
        std::string type = request.type;
        std::string msg;
        if (apiUtil->check_param_value("type", type, msg) == false)
        {
            response.Error(StatusParamIsIllegal, msg);
            return;
        }
        PFNInfo pfn_info = request.funInfo;
        if (pfn_info.empty())
        {
            msg =  "Neither funName nor funBody can be empty!";
            response.Error(StatusParamIsIllegal, msg);
            return;
        }
        std::string username = request.username;
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
                string result = pfnUtil->fun_build(username, pfn_info.funName);
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

    void ApiHandler::funreview(shared_ptr<APIUtil>& apiUtil, shared_ptr<PFNUtil>& pfnUtil, server::MessageFunReviewRequest& request, server::MessageFunReviewResponse& response)
    {
        try
        {
            std::string msg;
            struct PFNInfo pfn_info = request.funInfo;
            if (pfn_info.empty())
            {
                msg =  "Neither funName nor funBody can be empty!";
                response.Error(StatusParamIsIllegal, msg);
                return;
            }
            std::string username = request.username;
            pfnUtil->fun_review(username, &pfn_info);
            string content = pfn_info.funBody;
            gs::StringUtil::url_encode(content);
            response.StatusMsg = "success";
            response.body = content;
        }
        catch (const std::exception &e)
        {
            std::string error = "Function review fail: " + string(e.what());
            response.Error(StatusOperationFailed, error);
        }
    }
}