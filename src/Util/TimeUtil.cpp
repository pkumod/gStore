#include "TimeUtil.h"

namespace gutil 
{
    int64_t TimeUtil::timestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
        return ms.count();
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
        bool with_ms = false;
        if (format == NORM_DATETIME_MS_PATTERN)
        {
            format = NORM_DATETIME_PATTERN;
            with_ms = true;
        } 
        else if (format == PURE_DATETIME_MS_PATTERN)
        {
            format = PURE_DATETIME_PATTERN;
            with_ms = true;
        }
        char time_str[64];
        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&t);
        strftime(time_str, sizeof(time_str), format.c_str(), &tm);
        std::stringstream ss;
        ss << time_str;
        if (with_ms)
        {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            ss << "." << std::setw(3) << std::setfill('0') << ms.count();
        }
        return ss.str();
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
        if (format == NORM_DATETIME_MS_PATTERN)
        {
            format = NORM_DATE_PATTERN;
        }
        if (format == PURE_DATETIME_MS_PATTERN)
        {
            format = PURE_DATETIME_PATTERN;
        }
        std::tm tm_now = *std::localtime(&timestamp);
        char time_str[32];
        strftime(time_str, sizeof(time_str), format.c_str(), &tm_now);
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