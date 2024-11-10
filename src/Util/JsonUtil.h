#pragma once
#include "GlobalTypedef.h"
#include "nlohmann/json.hpp"

namespace gutil
{
    class JsonUtil
    {
    private:
        /* data */
    public:
        JsonUtil();
        ~JsonUtil();

        // nlohmann
        static std::string to_json_string(const nlohmann::json& json);
        static std::string jsonParam(const nlohmann::json& json, const std::string &key, const std::string& default_val = "");
        static int32_t jsonParam(const nlohmann::json& json, const std::string &key, const int32_t &default_val);
        static uint32_t jsonParam(const nlohmann::json& json, const std::string &key, const uint32_t &default_val);
        static int64_t jsonParam(const nlohmann::json& json, const std::string &key, const int64_t &default_val);
        static uint64_t jsonParam(const nlohmann::json& json, const std::string &key, const uint64_t &default_val);
        static bool jsonBoolParam(const nlohmann::json& json, const std::string &key, const bool &default_val);
        static bool hasJsonParam(const nlohmann::json& json, const std::string &key);
        static bool accept(const std::string &json_str);
        static bool parse(const std::string &json_str, nlohmann::json &json);
    };
}