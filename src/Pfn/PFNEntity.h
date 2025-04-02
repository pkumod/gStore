#pragma once
#include <string>
#include <vector>
#include "nlohmann/json.hpp"

using namespace std;

struct PFNInfo
{
public:
    std::string funName;
    std::string funDesc;
    std::string funBody;
    std::string funSubs;
    std::string funStatus;
    std::string lastTime;
public:
    PFNInfo() {}
    PFNInfo(const nlohmann::json& json)
    {
        if(json.is_string())
        {
            string json_str = json.get<string>();
            fromJSON(json_str, *this);
        }
        else
        {
            if (json.contains("funName"))
                json.at("funName").get_to(funName);
            if (json.contains("funDesc"))
                json.at("funDesc").get_to(funDesc);
            if (json.contains("funBody"))
                json.at("funBody").get_to(funBody); 
            if (json.contains("funSubs"))
                json.at("funSubs").get_to(funSubs);
            if (json.contains("funStatus"))
                json.at("funStatus").get_to(funStatus);
            if (json.contains("lastTime"))
                json.at("lastTime").get_to(lastTime);
        }
    }
    bool toJSON(nlohmann::json& json)
    {
        {
            try
            {
                json["funName"] = funName;
                json["funDesc"] = funDesc;
                json["funBody"] = funBody;
                json["funSubs"] = funSubs;
                json["funStatus"] = funStatus;
                json["lastTime"] = lastTime;
                return true;
            }
            catch (...)
            {
                return false;
            }
        }
    }

    static bool fromJSON(const std::string& json_str, struct PFNInfo &pfn_info)
    {
        try
        {
            nlohmann::json json = nlohmann::json::parse(json_str);
            if (json.contains("funName"))
                json.at("funName").get_to(pfn_info.funName);
            if (json.contains("funDesc"))
                json.at("funDesc").get_to(pfn_info.funDesc);
            if (json.contains("funBody"))
                json.at("funBody").get_to(pfn_info.funBody);
            if (json.contains("funSubs"))
                json.at("funSubs").get_to(pfn_info.funSubs);
            if (json.contains("funStatus"))
                json.at("funStatus").get_to(pfn_info.funStatus);
            if (json.contains("lastTime"))
                json.at("lastTime").get_to(pfn_info.lastTime);
            return true;
        }
        catch(const std::exception& e)
        {
            return false;
        }
    }

    void copyFrom(const struct PFNInfo &other)
    {
        this->funName = other.funName;
        this->funDesc = other.funDesc;
        this->funBody = other.funBody;
        this->funSubs = other.funSubs;
        this->funStatus = other.funStatus;
        this->lastTime = other.lastTime;
    }

    bool empty()
    {
        return funName.empty() && funBody.empty();
    }
};


struct PFNInfos
{
private:
    int totalSize;
    int totalPage;
    vector<struct PFNInfo> list;
public:
    PFNInfos() 
    {
        totalSize = 0;
        totalPage = 0;
    }
    PFNInfos(int _totalSize, int _totalPage)
    {
        totalSize = _totalSize;
        totalPage = _totalPage;
    }
    void setTotalSize(int _totalSize)
    {
        totalSize = _totalSize;
    }
    void setTotalPage(int _totalPage)
    {
        totalPage = _totalPage;
    }
    int getTotalSize() 
    {
        return totalSize;
    }
    int getTotalPage()
    {
        return totalPage;
    }
    void addPFNInfo(const string & json_str)
    {
        PFNInfo item;
        if (PFNInfo::fromJSON(json_str, item))
            list.push_back(item);
    }
    vector<struct PFNInfo> getPFNInfoList()
    {
        return list;
    }
};