#pragma once
#include "GlobalTypedef.h"

namespace gutil
{
    class ThreadUtil
    {
    public:
        /**
         * @brief Sleep the thread for a given number of milliseconds.
         * @param ms The number of milliseconds to sleep.
         */
        static void sleep(uint32_t ms)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }

        /**
         * @brief Get the ID of the current thread.
         * @return The ID of the current thread.
         */
        static std::string getThreadID()
        {
            return std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
        }
    };
}