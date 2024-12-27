#pragma once
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

namespace gutil 
{
    #define NORM_DATE_PATTERN "%Y-%m-%d"
    #define NORM_DATETIME_PATTERN "%Y-%m-%d %H:%M:%S"
    #define NORM_DATETIME_MS_PATTERN "%Y-%m-%d %H:%M:%S.%sss"
    #define PURE_DATE_PATTERN "%Y%m%d"
    #define PURE_DATETIME_PATTERN "%Y%m%d%H%M%S"
    #define PURE_DATETIME_MS_PATTERN "%Y%m%d%H%M%S.%sss"
    class TimeUtil
    {
    public:
        static int64_t timestamp();
        static std::string timestamp_str();
        static std::string now(std::string format=PURE_DATETIME_PATTERN);
        static std::string today(std::string format=PURE_DATE_PATTERN);
        static std::string format(time_t timestamp, std::string format=NORM_DATETIME_PATTERN);
        static time_t parse(const std::string& str, std::string format=NORM_DATETIME_PATTERN);
    };
}