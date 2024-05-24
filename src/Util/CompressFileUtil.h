# pragma once

#include <iostream>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <functional>
#include <map>
#include <vector>
#include "minizip/unzip.h"
#include "minizip/zip.h"
#include "Util.h"

#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (512)
#define MAXWBITS 15
#define GZIPENCODING 16
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

    class GzipHelper
    {
        public:
        static int compress(const std::string *data, void *compress_data, size_t &compress_size);
        static int unCompress(const char * data, int size, char *uncompress_data, size_t uncompress_size);
    };

    class CompressZip
    {
        public:
        CompressZip(){}
        ~CompressZip(){}
        bool compressDirExportZip(const std::string& sourcePath, const std::string& zipPath, bool contain_base = false);
        bool AddFileToZip(zipFile zf, const std::string& relative, const std::string& sourcePath);
        bool AddDirToZip(zipFile zf, const std::string& relative);
        bool InnerWriteFileToZip(zipFile zf, const std::string& path);
    };
}


