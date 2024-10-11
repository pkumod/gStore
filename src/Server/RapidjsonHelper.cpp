#include "ApiProvider.h"

namespace server
{
    std::string to_json_string(const rapidjson::Document& json)
    {
        rapidjson::StringBuffer resBuffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> resWriter(resBuffer);
        json.Accept(resWriter);
        return resBuffer.GetString();
    }

    std::string jsonParam(const rapidjson::Document& json, const std::string &key, const std::string& default_val)
    {
        if (json.HasMember(key.c_str()))
        {
            auto& value = json[key.c_str()];
            if (value.IsString()) {	    
                return value.GetString();
            } else if (value.IsInt()) {
                return std::to_string(value.GetInt());
            } else if (value.IsUint()) {
                return std::to_string(value.GetUint());
            } else if (value.IsInt64()) {
                return std::to_string(value.GetInt64());
            } else if (value.IsUint64()) {
                return std::to_string(value.GetUint64());
            } else if (value.IsDouble()) {
                return std::to_string(value.GetDouble());
            } else if(value.IsFloat()){
                return std::to_string(value.GetFloat());
            }else if (value.IsTrue()) {
                return "true";
            } else if (value.IsFalse()) {
                return "false";
            }
        }
        return default_val;
    }

    int32_t jsonParam(const rapidjson::Document& json, const std::string &key, const int32_t &default_val)
    {
        if (json.HasMember(key.c_str()))
        {
            auto& value = json[key.c_str()];
            if (value.IsInt()) {
                return value.GetInt();
            } else if (value.IsString()) {
                return std::stoi(value.GetString());
            }
        }
        return default_val;
    }

    uint32_t jsonParam(const rapidjson::Document& json, const std::string &key, const uint32_t &default_val)
    {
        if (json.HasMember(key.c_str()))
        {
            auto& value = json[key.c_str()];
            if (value.IsUint()) {
                return value.GetUint();
            } else if (value.IsString()) {
                uint32_t max = std::numeric_limits<uint32_t>::max();
                int64_t val = std::stoll(value.GetString());
                if (val > max) {
                    return default_val;
                }
                return val;
            }
        }
        return default_val;
    }

    int64_t jsonParam(const rapidjson::Document& json, const std::string &key, const int64_t &default_val)
    {
        if (json.HasMember(key.c_str()))
        {
            auto& value = json[key.c_str()];
            if (value.IsInt64()) {
                return value.GetInt64();
            } else if (value.IsString()) {
                int64_t max = std::numeric_limits<int64_t>::max();
                uint64_t val = std::stoll(value.GetString());
                if (val > max) {
                    return default_val;
                }
                return val;
            }
        }
        return default_val;
    }

    uint64_t jsonParam(const rapidjson::Document& json, const std::string &key, const uint64_t &default_val)
    {
        if (json.HasMember(key.c_str()))
        {
            auto& value = json[key.c_str()];
            if (value.IsInt64()) {
                return value.GetInt64();
            } else if (value.IsString()) {
                return std::stoul(value.GetString());
            }
        }
        return default_val;
    }

    bool jsonBoolParam(const rapidjson::Document& json, const std::string &key, const bool &default_val)
    {
        if (json.HasMember(key.c_str())) {
            auto& value = json[key.c_str()];
            if (value.IsBool()) {
                SLOG_DEBUG("json[" + key + "]=" + to_string(value.GetBool()));
                return value.GetBool();
            } else if (value.IsString()) {
                std::string v = value.GetString();
                SLOG_DEBUG("json[" + key + "]=" + v);
                return v == "true" || v == "1";
            } else if (value.IsInt()) {
                SLOG_DEBUG("json[" + key + "]=" + to_string(value.GetInt()));
                return value.GetInt() == 1;
            }
        }
        return default_val;
    }

    bool hasJsonParam(const rapidjson::Document& json, const std::string &key)
    {
        return json.HasMember(key.c_str());
    }
}