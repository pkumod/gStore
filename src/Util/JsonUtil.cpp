#include "JsonUtil.h"

namespace gutil
{

    JsonUtil::JsonUtil() {}
    JsonUtil::~JsonUtil() {}

    std::string JsonUtil::to_json_string(const nlohmann::json &json)
    {
        return json.dump();
    }

    std::string JsonUtil::jsonParam(const nlohmann::json &json, const std::string &key, const std::string &default_val)
    {
        if (json.contains(key))
        {
            if (json[key].is_string())
            {
                return json[key].get<std::string>();
            }
            else if (json[key].is_boolean())
            {
                return "true";
            }
        }
        return default_val;
    }

    int32_t JsonUtil::jsonParam(const nlohmann::json &json, const std::string &key, const int32_t &default_val)
    {
        if (json.contains(key))
        {
            if (json[key].is_number_integer())
            {
                return json[key].get<int32_t>();
            }
            else if (json[key].is_string())
            {
                std::string value = json[key].get<std::string>();
                if (value.empty())
                    return 0;
                return std::stoi(value);
            }
        }
        return default_val;
    }

    uint32_t JsonUtil::jsonParam(const nlohmann::json &json, const std::string &key, const uint32_t &default_val)
    {
        if (json.contains(key))
        {
            if (json[key].is_number_unsigned())
            {
                return json[key].get<uint32_t>();
            }
            else if (json[key].is_string())
            {
                uint32_t max = std::numeric_limits<uint32_t>::max();
                std::string value = json[key].get<std::string>();
                if (value.empty())
                    return 0;
                int64_t val = std::stoll(value);
                if (val > max)
                {
                    return default_val;
                }
                return val;
            }
        }
        return default_val;
    }

    int64_t JsonUtil::jsonParam(const nlohmann::json &json, const std::string &key, const int64_t &default_val)
    {
        if (json.contains(key))
        {
            if (json[key].is_number())
            {
                return json[key].get<int64_t>();
            }
            else if (json[key].is_string())
            {
                int64_t max = std::numeric_limits<int64_t>::max();
                std::string value = json[key].get<std::string>();
                if (value.empty())
                    return 0;
                uint64_t val = std::stoll(value);
                if (val > max)
                {
                    return default_val;
                }
                return val;
            }
        }
        return default_val;
    }

    uint64_t JsonUtil::jsonParam(const nlohmann::json &json, const std::string &key, const uint64_t &default_val)
    {
        if (json.contains(key))
        {
            if (json[key].is_number())
            {
                return json[key].get<uint64_t>();
            }
            else if (json[key].is_string())
            {
                std::string value = json[key].get<std::string>();
                if (value.empty())
                    return 0;
                return std::stoul(value);
            }
        }
        return default_val;
    }

    bool JsonUtil::jsonBoolParam(const nlohmann::json &json, const std::string &key, const bool &default_val)
    {
        if (json.contains(key))
        {
            if (json[key].is_boolean())
            {
                return json[key].get<bool>();
            }
            else if (json[key].is_string())
            {
                std::string value = json[key].get<std::string>();
                return value == "true" || value == "1";
            }
            else if (json[key].is_number())
            {
                int value = json[key].get<int>();
                return value == 1;
            }
        }
        return default_val;
    }

    void JsonUtil::jsonArrayParam(const nlohmann::json& json, const std::string &key, std::vector<std::string> &vec)
    {
        if (json.contains(key))
        {
            if (json[key].is_array())
            {
                for (auto &item : json[key])
                {
                    vec.push_back(item.get<std::string>());
                }
            }
        }
    }

    bool JsonUtil::hasJsonParam(const nlohmann::json &json, const std::string &key)
    {
        return json.contains(key);
    }

    bool JsonUtil::accept(const std::string &json_str)
    {
        return nlohmann::json::accept(json_str);
    }
    bool JsonUtil::parse(const std::string &json_str, nlohmann::json &json)
    {
        try
        {
            json = nlohmann::json::parse(json_str);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
}