#pragma once
#include "GlobalTypedef.h"
#include <filesystem>
namespace gs
{
    class FileUtil
    {
    public:
        static bool readFile(const std::string& path, std::string& content);
        static bool createFile(const std::string& path);
        static bool createDirs(const std::string& path);
        static bool writeLine(const std::string& path, const std::string& line);
        static bool writeLines(const std::string& path, const std::vector<std::string>& lines);
        static bool fileExists(const std::string& path);
        static bool dirExists(const std::string& path);
        static bool pathExists(const std::string& path);
        static void Csync(FILE* _fp);
        static std::string fileName(const std::string &filepath);
        static void dir_filepaths(const std::string& _dir, std::vector<std::string> &file_list);
        static void dir_filenames(const std::string& _dir, std::vector<std::string> &file_list, const std::string& _name_contains = "", const bool& _inlude_path = false);
        static bool is_file(const std::string& _file);
        static bool is_dir(const std::string& _path);
        static std::string fileSuffix(const std::string &filepath);
        static bool removeDir(const std::string& dir_path);
        static bool removeFile(const std::string& file_path);
        static bool removePath(const std::string& path);
        static bool copyFile(const std::string& src_file, const std::string& dst_file);
        static bool copyDir(const std::string& src_dir, const std::string& dst_dir);
        static bool movePath(const std::string& src_path, const std::string& dst_path);
        static std::string parentPath(const std::string& path);
        static std::uint64_t fileSize(const std::string& path);
        static std::uint64_t dirSize(const std::string& path);
        static std::uint64_t fileLines(const std::string& file_path);
        static bool isEmptyDir(const std::string& dir_path);
        static std::string getExactPath(const char *str);
        static bool isSameDir(const std::string& s, const std::string& t);
    };
}