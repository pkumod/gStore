#pragma once

#include "GlobalTypedef.h"
#include "../Util/Util.h"

namespace gutil
{
    class ResourceUtil
    {
    private:
        //for cpu mem disk
        static const char* get_cpu_items(const char* buffer, unsigned int item);
        static uint64_t get_cpu_total();
        static uint64_t get_cpu_proc(int pid);
    public:
        ResourceUtil();
        ~ResourceUtil();
        static int32_t memoryLeft();
        static int32_t memUsedPercentage();
        static uint64_t get_disk_free();
        static float get_app_cpu_usage(int pid);
        static float get_app_mem_usage(int pid);
        // alloctate memory
        static bool IsEnoughMemory(unsigned triple_num);
        static bool IsEnoughMemoryMb(const size_t& bytes);
        static bool IsEnoughDisk(unsigned triple_num);
    };
}