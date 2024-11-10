#pragma once
#include "GlobalTypedef.h"

namespace gutil
{
    class FileUtil
    {
    public:
        static bool readFile(const std::string& path, std::string& content);
        static bool createFile(const std::string& path);
        static bool writeLine(const std::string& path, const std::string& line);
        static bool writeLines(const std::string& path, const vector<std::string>& lines);
        static bool fileExists(const std::string& path);
        static void Csync(FILE* _fp);
    };
}