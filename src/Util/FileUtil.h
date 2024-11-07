#pragma once
#include "GlobalTypedef.h"

namespace gutil
{
    class FileUtil
    {
    public:
        bool readFile(const std::string& path, std::string& content);
        bool writeFile(const std::string& path, const std::string& content);
        bool fileExists(const std::string& path);
        static void Csync(FILE* _fp);
    };
}