#include "TimeUtil.h"

namespace gutil 
{
    time_t TimeUtil::timestamp()
    {
        std::time_t now = std::time(0);
        struct tm *tm_now = std::localtime(&now);
        return mktime(tm_now);
    }

    std::string TimeUtil::timestamp_str()
    {
        time_t ts = timestamp();
        return to_string(ts);
    }

    /**
     * current time, default format: YYYYmmddHHMMSS
     */
    std::string TimeUtil::now(std::string format)
    {
        char time_str[32];
        time_t timep;
        time(&timep);
        strftime(time_str, sizeof(time_str), format.c_str(), localtime(&timep));
        return string(time_str);
    }

    /**
     * current date, default format: YYYYmmdd
     */
    std::string TimeUtil::today(std::string format)
    {
        char time_str[20];
        time_t timep;
        time(&timep);
        strftime(time_str, sizeof(time_str), format.c_str(), localtime(&timep));
        return string(time_str);
    }

    /**
     * format timestamp to string, default format: YYYY-mm-dd HH:MM:SS
     */
    std::string TimeUtil::format(time_t timestamp, std::string format)
    {
        struct tm *tm_now = std::localtime(&timestamp);
        char time_str[32];
        strftime(time_str, sizeof(time_str), format.c_str(), tm_now);
        return string(time_str);
    }

    /**
     * parse string to timestamp, default format: YYYY-mm-dd HH:MM:SS
     */
    time_t TimeUtil::parse(const std::string& str, std::string format)
    {
        struct tm tm_now = {0};
        strptime(str.c_str(), format.c_str(), &tm_now);
        return mktime(&tm_now);
    }
}