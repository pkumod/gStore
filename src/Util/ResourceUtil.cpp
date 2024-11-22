#include "ResourceUtil.h"

namespace gutil {
    
    ResourceUtil::ResourceUtil() {}
    
    ResourceUtil::~ResourceUtil() {}

    const char* ResourceUtil::get_cpu_items(const char* buffer, unsigned int item)
    {
        const char* p = buffer;

        int len = strlen(buffer);
        unsigned int count = 0;
        for (int i = 0; i < len; i++)
        {
            if (' ' == *p)
            {
                count++;
                if (count == item)
                {
                    p++;
                    break;
                }
            }
            p++;
        }
        return p;
    }

    inline uint64_t ResourceUtil::get_cpu_total()
    {
        // different mode cpu occupy time
        uint64_t user_time;
        uint64_t nice_time;
        uint64_t system_time;
        uint64_t idle_time;
    
        FILE* fd;
        char buff[1024] = { 0 };
    
        fd = fopen("/proc/stat", "r");
        if (nullptr == fd)
            return 0;
    
        fgets(buff, sizeof(buff), fd);
        char name[64] = { 0 };
        sscanf(buff, "%s %ld %ld %ld %ld", name, &user_time, &nice_time, &system_time, &idle_time);
        fclose(fd);
    
        return (user_time + nice_time + system_time + idle_time);
    }

    inline uint64_t ResourceUtil::get_cpu_proc(int pid)
    {
        // get specific pid cpu use time
        uint32_t tmp_pid;
        uint64_t utime;  // user time
        uint64_t stime;  // kernel time
        uint64_t cutime; // all user time
        uint64_t cstime; // all dead time
    
        char file_name[64] = { 0 };
        FILE* fd;
        char line_buff[1024] = { 0 };
        sprintf(file_name, "/proc/%d/stat", pid);
    
        fd = fopen(file_name, "r");
        if (nullptr == fd)
            return 0;
    
        fgets(line_buff, sizeof(line_buff), fd);
    
        sscanf(line_buff, "%u", &tmp_pid);
        const char* q = ResourceUtil::get_cpu_items(line_buff, PROCESS_ITEM);
        sscanf(q, "%ld %ld %ld %ld", &utime, &stime, &cutime, &cstime);
        fclose(fd);
    
        return (utime + stime + cutime + cstime);
    }

    int32_t ResourceUtil::memoryLeft() 
    {
        FILE* fp = fopen("/proc/meminfo", "r");
        if(fp == NULL)
            return 0;
        char str[20], tail[3];
        uint32_t num, avail = 0, free = 0, buffer = 0, cache = 0;		//WARN:unsigned,memory cant be too large!
        while (fscanf(fp, "%s%u%s", str, &num, tail) != EOF)
        {
            if(strcmp(str, "MemAvailable:") == 0)
                avail = num;
            if(strcmp(str, "MemFree:") == 0)
                free = num;
            if(strcmp(str, "Buffers:") == 0)
                buffer = num;
            if(strcmp(str, "Cached:") == 0)
                cache = num;
        }

        if (avail == 0)
            avail = free + buffer + cache;

        fclose(fp);
        // KB to MB
        return avail >> 10;    
    }

    int32_t ResourceUtil::memUsedPercentage()
    {
        FILE* fp = fopen("/proc/meminfo", "r");
        if(fp == NULL)
            return 0;
        char str[20], tail[3];
        uint32_t t, sum, used = 0;		//WARN:unsigned,memory cant be too large!
        fscanf(fp, "%s%u%s", str, &sum, tail);      //MemTotal, KB
        fscanf(fp, "%s%u%s", str, &used, tail);		//MemFree
        fscanf(fp, "%s%u%s", str, &t, tail);
        if(strcmp(str, "MemAvailable") == 0)
        {
            used = t;
        }
        used = sum - used;
        fclose(fp);
        return (int)(used * 100.0 / sum);
    }

    uint64_t ResourceUtil::get_disk_free()
    {
        char* p = NULL;
        const int len = 256;
        char arr_tmp[len] = {0};
        int n = readlink("/proc/self/exe", arr_tmp, len);
        if (n == -1)
        {
            return 0;
        }
        if (NULL != (p = strrchr(arr_tmp, '/')))
        {
            *p = '\0';
        }
        std::string cur_path = std::string(arr_tmp);
        struct statfs disk_info;
        statfs(cur_path.c_str(), &disk_info);
        // byte num of block
        int64_t block_size = disk_info.f_bsize;
        uint64_t total_size = block_size * disk_info.f_blocks;
        string log_info = "Total_size=" + to_string(total_size) + ", " + to_string(total_size>>10) + "B, " + to_string(total_size>>20) + "MB, " + to_string(total_size>>30) + "GB";
        SLOG_CORE(log_info);
        // free = block_size * free_block_num
        // available = block_size * available_block_num
        // byte to MB
        uint64_t available_disk = (block_size * disk_info.f_bavail) >> 20;
        // return MB
        return available_disk;
    }

    float ResourceUtil::get_app_cpu_usage(int pid)
    {
        unsigned long totalcputime1, totalcputime2;
        unsigned long procputime1, procputime2;
        totalcputime1 = get_cpu_total();
        procputime1 = get_cpu_proc(pid);
        // FIXME: the 200ms is a magic number, works well
        usleep(200000); // sleep 200ms to fetch two time point cpu usage snapshots sample for later calculation
        totalcputime2 = get_cpu_total();
        procputime2 = get_cpu_proc(pid);
        float pcpu = 0.0;
        if (0 != totalcputime2 - totalcputime1)
            pcpu = (procputime2 - procputime1) / float(totalcputime2 - totalcputime1); // float number
        int cpu_num = get_nprocs();
        pcpu *= cpu_num; // should multiply cpu num in multiple cpu machine
        return pcpu;
    }

    float ResourceUtil::get_app_mem_usage(int pid)
    {
        char file_name[64] = {0};
        FILE *fd;
        char line_buff[512] = {0};
        sprintf(file_name, "/proc/%d/status", pid);
        fd = fopen(file_name, "r");
        if (nullptr == fd)
            return 0;
        char name[64];
        int vmrss = 0;
        for (int i = 0; i < VMRSS_LINE - 1; i++)
            fgets(line_buff, sizeof(line_buff), fd);
        fgets(line_buff, sizeof(line_buff), fd);
        sscanf(line_buff, "%s %d", name, &vmrss);
        fclose(fd);
        // cnvert VmRSS from KB to MB
        return vmrss >> 10;
    }

    bool ResourceUtil::IsEnoughMemory(unsigned triple_num)
    {
        if (Util::getConfigureValue("resource_check") != "on")
            return true;
        // uint mb
        unsigned need_count = (triple_num/1000000) > 0 ? (triple_num/1000000) : 1;
        unsigned million_need_memory = atoi(Util::getConfigureValue("min_million_memory").c_str());
        int memory_free = memoryLeft();
        if (memory_free <= need_count*million_need_memory)
        {
            SLOG_WARN("Memory is not enough, need at least memory:" << need_count*million_need_memory << "mb" << " ,current:" << memory_free << "mb");
            return false;
        }
        return true;
    }

    bool ResourceUtil::IsEnoughMemoryMb(const size_t& bytes)
    {
        if (Util::getConfigureValue("resource_check") != "on")
            return true;
        int memory_free = memoryLeft();
        int need_memory = bytes >> 20;
        if (memory_free <= need_memory) {
            SLOG_WARN("Memory is not enough, need at least memory:" << need_memory << "mb" << " ,current:" << memory_free << "mb");
            return false;
        } else {
            return true;
        }
    }

    bool ResourceUtil::IsEnoughDisk(unsigned triple_num)
    {
        if (Util::getConfigureValue("resource_check") != "on")
            return true;
        // uint mb
        unsigned need_count = (triple_num/1000000) > 0 ? (triple_num/1000000) : 1;
        unsigned million_need_disk = atoi(Util::getConfigureValue("min_million_disk").c_str());
        int disk_free = get_disk_free();
        if (disk_free <= need_count*million_need_disk)
        {
            SLOG_WARN("Disk is not enough, need at least disk:" << need_count*million_need_disk << "mb" << " ,disk:" << disk_free << "db");
            return false;
        }
        return true;
    }
}