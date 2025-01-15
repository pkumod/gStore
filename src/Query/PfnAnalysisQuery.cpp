
/*
 * @Author: hexuejiang
 * @Date: 2025-1-6 9:52:50
 * @LastEditTime: 2025-1-10 11:50:20
 * @LastEditors: hexuejiang 1632802996@qq.com
 * @Description: pfn query
 * @Sparql: SELECT (PFN.xxx({\"src\":\"<Alice>\", \"dst\": \"<Bob>\", \"direct\":\"true\", \"predSet\":[\"<喜欢>\",\"<关注>\"]}) AS ?z) WHERE {}
 * @function name: xxx
 * @param: {\"src\":\"<Alice>\", \"dst\": \"<Bob>\", \"direct\":\"true\", \"predSet\":[\"<喜欢>\",\"<关注>\"]}
 */
#include "GeneralEvaluation.h"
#include "../Pfn/GAnalysis.h"
#include "../Database/TaskManager.h"

typedef bool (*pfn_analysis_name)(GAnalysis&, const std::string&, std::string&);

std::string GeneralEvaluation::pfnQuery(const std::string& pfn_name, const std::string& pfn_params, const std::string& username)
{
    try
	{
        SLOG_TRACE("pfnQuery pfn_name:" << pfn_name << " pfn_params:" << pfn_params);
        // notice pfn_params parse check, do not delete
        nlohmann::json json = nlohmann::json::parse(pfn_params);
        std::string sofile =  getPfnSoFile(username, pfn_name);
        std::string result = "pfn_type" + excutePfnSoFile(pfn_name, sofile, pfn_params);
        return result;
    }
    catch (const std::exception &e)
    {
		string content = "run dynamic function fail: " + string(e.what());
        SLOG_ERROR(content);
		throw runtime_error(content);
    }
	catch (...)
	{
		string content = "run dynamic function fail: unknown error";
		SLOG_ERROR(content);
		throw runtime_error(content);
	}
}

void GeneralEvaluation::pfnConvertVar(std::vector<std::pair<std::string, vector<int>>>& varList, std::set<std::string>& param_jsons, nlohmann::json& param_json, int pos)
{
	int size = varList.size();
	if (pos >= size)
		return;
	for (auto& m : varList[pos].second)
	{
        task_event.checkOpCancel();
		std::string vid_str = kvstore->getStringByID(m);
		if (vid_str.empty())
			continue;
		param_json[varList[pos].first] = vid_str;
		if (pos == size -1)
		{
			std::string json_str = param_json.dump();
			if (param_jsons.find(json_str) != param_jsons.end())
				continue;
			param_jsons.insert(json_str);
		}
		else
		{
			pfnConvertVar(varList, param_jsons, param_json, pos+1);
		}
	}
}

std::string GeneralEvaluation::pfnQueryByVar(const std::string& pfn_name, const std::string& pfn_params, const std::string& username, TempResult &result0, int begin, int end, int result0_id_cols)
{
	try
	{
        SLOG_TRACE("pfnQueryByVar pfn_name:" << pfn_name << " pfn_params:" << pfn_params);
        nlohmann::json json = nlohmann::json::parse(pfn_params);
        std::string sofile =  getPfnSoFile(username, pfn_name);

		std::vector<std::pair<std::string, vector<int>>> varList;
		for (auto& m : json.items())
		{
			if (m.value().is_string() && result0.getAllVarset().findVar(m.value()))
			{
				std::string var_value = JsonUtil::jsonParam(json, m.key());
				int var2temp = Varset(var_value).mapTo(result0.getAllVarset())[0];
				SLOG_CORE("pfn var param:" << m.key());
				if (var2temp >= result0_id_cols)
					SLOG_ERROR("[ERROR] src must be an entity!");
				else
				{
					std::pair<std::string, vector<int>> varL;
					varL.first = m.key();
					for (int j = begin; j <= end; j++)
					{
						if (result0.result[j].id[var2temp] != INVALID)
							varL.second.push_back(result0.result[j].id[var2temp]);
					}
					if (varL.second.size() > 0)
                    {
                        SLOG_CORE("var size:" << varL.second.size());
                        varList.push_back(varL);
                    }
				}
			}
		}

		std::set<std::string> param_jsons;
		nlohmann::json param_json = json;
		pfnConvertVar(varList, param_jsons, param_json, 0);

        SLOG_CORE("pfn var param size:" << param_jsons.size());

		nlohmann::json result_s = nlohmann::json::array();
		for (auto&m : param_jsons)
		{
            task_event.checkOpCancel();
            std::string result_str = excutePfnSoFile(pfn_name, sofile, m);
			try
			{
				nlohmann::json result_json = nlohmann::json::parse(result_str);
				result_s.push_back(result_json);
			}
			catch (nlohmann::json::exception& e)
			{
				result_s.push_back(result_str);
			}
		}
		return  "pfn_type" + result_s.dump();
	}
	catch (const std::exception &e)
    {
		string content = "run dynamic function fail: " + string(e.what());
        SLOG_ERROR(content);
		throw runtime_error(content);
    }
	catch (...)
	{
		string content = "run dynamic function fail: unknown error";
		SLOG_ERROR(content);
		throw runtime_error(content);
	}
}

std::string GeneralEvaluation::getPfnSoFile(const std::string& username, const std::string& fun_name)
{
    string pfn_base_path = Util::getConfigureValue("pfn_base_path");
    string pfn_file_path = pfn_base_path + "cpp/" + username;
    string pfn_lib_path =  pfn_base_path + "lib/" + username;
    string json_file_path = pfn_file_path + "/data.json";
    // #if defined(DEBUG)
    SLOG_CORE("lib path: " << pfn_lib_path);
    SLOG_CORE("json path: " << json_file_path);
    // #endif
    ifstream in;
    in.open(json_file_path, ios::in);
    if (!in.is_open())
    {
        throw runtime_error("open function json file error.");
    }
    string line;
    string md5Str;
    string fun_args, fun_status, last_time;
    bool isMatch = false;
    string fun_return = "";
    string match_name = "\"funName\":\""+fun_name+"\"";
    while (getline(in, line))
    {
        if (line.find(match_name) != std::string::npos)
        {
            if (!nlohmann::json::accept(line))
            {
                throw runtime_error("function '" + fun_name + "' json file format error");
            }
            nlohmann::json doc = nlohmann::json::parse(line);
            if (doc.contains("funStatus"))
                doc.at("funStatus").get_to(fun_status);
            if (fun_status != "2")
            {
                throw runtime_error("abort function '" + fun_name + "' not compile yet");
            }
            if (doc.contains("funReturn"))
                doc.at("funReturn").get_to(fun_return);
            if (doc.contains("lastTime"))
            {
                doc.at("lastTime").get_to(last_time);
                md5Str = Util::md5(last_time);
            }
            isMatch = true;
            break;
        }
    }
    in.close();
    if (!isMatch)
        throw runtime_error("abort function '" + fun_name + "' is not exist");

    string fileName = fun_name;
    std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
    std::string soFile = pfn_lib_path + "/lib" + fileName + md5Str + ".so";
    return soFile;
}

std::string GeneralEvaluation::excutePfnSoFile(const std::string& fun_name, const std::string& soFile, const std::string& pfn_params)
{
    void* handle = dlopen(soFile.c_str(), RTLD_LAZY);
    if (!handle)
        throw runtime_error("abort load so file error:" + string(dlerror()));

    pfn_analysis_name p_fun = (pfn_analysis_name)dlsym(handle, fun_name.c_str());
    char *error;
    if ((error = dlerror()) != NULL)
    {
        dlclose(handle);
        throw runtime_error("cannot load symbol '" + fun_name + "': " + string(error));
    }
    
    GAnalysis ganalysis(kvstore, pqHandler->getCSRHandler());
    std::string result;
    try
	{
        bool success = p_fun(ganalysis, pfn_params, result);
    }
    catch (const std::exception &e)
    {
		string content = "run dynamic function fail: " + string(e.what());
        SLOG_ERROR(content);
		throw runtime_error(content);
    }
	catch (...)
	{
		string content = "run dynamic function fail: unknown error";
		SLOG_ERROR(content);
		throw runtime_error(content);
	}
    dlclose(handle);
    SLOG_CORE("result: " + result);
    return result;
}