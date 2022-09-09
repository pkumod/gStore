#include "workflow/StringUtil.h"
#include "workflow/WFFacilities.h"

#include "grpc_content.h"

using namespace grpc;


std::string ContentType::to_str(enum content_type type)
{
    switch (type)
    {
#define CTM(name, string, suffix) case name: return #string;
        CONTENT_TYPES_MAP(CTM)
#undef CTM
        default:
            return "<unknown>";
    }
}

enum content_type ContentType::to_enum(const std::string &content_type_str)
{
    if (content_type_str.empty())
    {
        return CONTENT_TYPE_NONE;
    }
#define CTM(name, string, suffix) \
    if (StringUtil::start_with(content_type_str.c_str(), #string)) { \
        return name; \
    }
    CONTENT_TYPES_MAP(CTM)
#undef CTM
    return CONTENT_TYPE_UNDEFINED;
}

std::string ContentType::to_str_by_suffix(const std::string &str)
{
    if (str.empty())
    {
        return "";
    }
#define CTM(name, string, suffix) \
    if (str == #suffix) { \
        return #string; \
    }
    CONTENT_TYPES_MAP(CTM)
#undef CTM
    return "";
}

enum content_type ContentType::to_enum_by_suffix(const std::string &str)
{
    if (str.empty()) {
        return CONTENT_TYPE_NONE;
    }
#define CTM(name, string, suffix) \
    if (str == #suffix) { \
        return name; \
    }
    CONTENT_TYPES_MAP(CTM)
#undef CTM
    return CONTENT_TYPE_UNDEFINED;
}

std::map<std::string, std::string> UrlEncode::parse_post_body(const std::string &body)
{
    std::map<std::string, std::string> map;

    if (body.empty())
        return map;

    std::vector<std::string> arr = StringUtil::split(body, '&');

    if (arr.empty())
        return map;

    for (const auto &ele: arr)
    {
        if (ele.empty())
            continue;

        std::vector<std::string> kv = StringUtil::split(ele, '=');
        size_t kv_size = kv.size();
        std::string &key = kv[0];

        if (key.empty() || map.count(key) > 0)
            continue;

        if (kv_size == 1)
        {
            map.emplace(std::move(key), "");
            continue;
        }

        std::string &val = kv[1];

        if (val.empty())
            map.emplace(std::move(key), "");
        else
            map.emplace(std::move(key), std::move(val));
    }
    return map;
}

Timestamp Timestamp::now()
{
    uint64_t timestamp = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
    return Timestamp(timestamp);
}

std::string Timestamp::to_format_str() const
{
    return to_format_str("%Y-%m-%d %X");
}

std::string Timestamp::to_format_str(const char *fmt) const
{
    std::time_t time = micro_sec_since_epoch_ / k_micro_sec_per_sec;  // ms --> s
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), fmt);
    return ss.str();
}