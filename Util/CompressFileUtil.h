# pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <functional>
#include "../tools/zlib-1.3/include/unzip.h"
#include "../tools/zlib-1.3/include/zip.h"
#include "../Util/Util.h"

#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (512)
using namespace std;

typedef std::function<bool(std::string)> foreach_cb;

namespace CompressUtil
{
    enum ZipCode
    {
        //unCompress
        UnZipOK = 0,
        UnZipFound,
        UnZipError,
        UnZipNotSupported,
        UnZipGetInfoError,
        UnZipFileEmptyDir,
        UnZipWriteError,
    };

    class FileHelper
    {
        public:
        static int isLargeFile(const char* filename);
        static bool isFileDir(const std::string &filename);
        static bool compressExportZip(const std::string& dst_path, const std::string& zip_path);
        static ZipCode foreachZip(const std::string& zip_path, const foreach_cb& cb);
    };

    class UnCompressZip
    {
        private:
        std::string m_dirpath_;
        std::string m_zip_path_;
        std::map<std::string, unsigned long long> m_files_info_;
        public:
        UnCompressZip(const std::string& zip_path, const std::string& dir_path);
        ~UnCompressZip();
        private:
        void addFileInfo(const std::string& file_path, unsigned long long size);
        const std::string& getDirPath()const{ return m_dirpath_; }
        bool doExtractCurrentFile(unzFile unfile);
        public:
        std::string getMaxFilePath()const; 
        void getFileList(std::vector<std::string>& files, const std::string& except)const;
        ZipCode unCompress();
    };
}

