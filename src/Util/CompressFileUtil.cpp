#include "CompressFileUtil.h"

namespace CompressUtil
{   
    bool FileHelper::isFileDir(const std::string &filename)
    {
        if (filename.empty())
            return false;
        if (filename.back() == '/')
            return true;
        return false;
    }

    ZipCode FileHelper::foreachZip(const std::string& zip_path, const foreach_cb& cb)
    {
        unzFile unfile = unzOpen64(zip_path.c_str());
        if (unfile == nullptr)
            return UnZipFound;
  
        uLong i = 0;
        unz_global_info64 global_info;
        int err = 0;
        if (unzGetGlobalInfo64(unfile, &global_info) != UNZ_OK)
        {
            unzClose(unfile);
            return UnZipError;
        }
        if (global_info.number_entry <= 0)
        {
            unzClose(unfile);
            return UnZipFileEmptyDir;
        }

        ZipCode error = UnZipOK;
        int file_count = 0;
        for (i = 0; i < global_info.number_entry; ++i)
        {
            char filename[MAXFILENAME];
            unz_file_info64 file_info;
            
            err = unzGetCurrentFileInfo64(unfile, &file_info, filename, MAXFILENAME, NULL, 0, NULL, 0);
            if (err != UNZ_OK)
            {
                error = UnZipGetInfoError;
                break;
            }

            if (!FileHelper::isFileDir(filename))
            {
                if (!cb(filename))
                {
                    unzClose(unfile);
                    return UnZipNotSupported;
                }
                ++file_count;
            }

            if ((i+1) < global_info.number_entry)
            {
                err = unzGoToNextFile(unfile);
                if (err!=UNZ_OK)
                {
                    error = UnZipError;
                    break;
                }
            }
        }
        
        unzClose(unfile);
        if (error != UnZipOK)
            return error;
        return file_count==0? UnZipFileEmptyDir:UnZipOK;
    }

    int FileHelper::isLargeFile(const char* filename)
    {
        //suported > 4g file
        int large_4g = 0;
        ZPOS64_T pos = 0;
        FILE* pFile  = fopen64(filename, "rb");
        if (pFile != NULL)
        {
            fseeko64(pFile, 0, SEEK_END);
            pos = (ZPOS64_T)ftello64(pFile);
            if (pos >= 0xffffffff)
            {
                large_4g = 1;
            }
            fclose(pFile);
        }
        return large_4g;
    }

    bool FileHelper::compressExportZip(const std::string& dst_path, const std::string& zip_path, bool contain_base)
    {
        SLOG_CORE("compressFile->start:"<<zip_path);
        zipFile zfile = zipOpen64(zip_path.c_str(), APPEND_STATUS_CREATE);
        if (zfile == nullptr)
        {
            return false;
        }
        int zip64 = isLargeFile(dst_path.c_str());
        std::string new_file_name = dst_path;
        if (!contain_base)
        {
            new_file_name = FileUtil::fileName(dst_path);
        }
        if (zipOpenNewFileInZip64(zfile, new_file_name.c_str(), nullptr, nullptr, 0 , nullptr , 0, nullptr, Z_DEFLATED, Z_DEFLATED, zip64) != ZIP_OK)
        {
            zipClose(zfile, nullptr);
            return false;
        }
        FILE *fin = fopen(dst_path.c_str(), "rb");
        if (fin == nullptr)
        {
            zipCloseFileInZip(zfile);
            zipClose(zfile, nullptr);
            return false;
        }
        void* read_buf = nullptr;
        read_buf = (void*)malloc(WRITEBUFFERSIZE);
        if (read_buf == nullptr)
        {
            fclose(fin);
            zipCloseFileInZip(zfile);
            zipClose(zfile, nullptr);
            return false;
        }
        size_t size_read = 0;
        int err = ZIP_OK;
        do
        {
            err = ZIP_OK;
            size_read = fread(read_buf, 1, WRITEBUFFERSIZE, fin);
            if (size_read < WRITEBUFFERSIZE)
            {
                if (feof(fin) == 0)
                {
                    err = ZIP_ERRNO;
                    break;
                }
            }

            if (size_read > 0)
            {
                err = zipWriteInFileInZip(zfile, read_buf, (unsigned)size_read);
            }

        }while(err == ZIP_OK && size_read > 0);

        if (fin)
            fclose(fin);
        if (err < 0)
            err = ZIP_ERRNO;
        else
            err = zipCloseFileInZip(zfile);
        SLOG_CORE("compressFile->success, err:"<<err);
        free(read_buf);
        zipClose(zfile, nullptr);
        return err == ZIP_OK ? true : false;    
    }

    ///////UnCompressZip///////
    UnCompressZip::UnCompressZip(const std::string& zip_path, const std::string& dir_path)
        :m_zip_path_(zip_path), m_dirpath_(dir_path)
    {
        if (!FileUtil::dirExists(dir_path))
        {
            FileUtil::createDirs(dir_path);
        }
    }

    UnCompressZip::~UnCompressZip()
    {
    }

    void UnCompressZip::addFileInfo(const std::string& file_path, unsigned long long size)
    {
        m_files_info_.insert(std::make_pair(file_path, size));
    }

    void UnCompressZip::getMaxFilePath(std::pair<std::string, unsigned long long>& max)const
    {
        // get max file
        auto max_iter = std::max_element(
            m_files_info_.begin(), 
            m_files_info_.end(), 
            [&](const std::pair<std::string, unsigned long long>& a, const std::pair<std::string, unsigned long long>& b) {
                return a.second < b.second;
        });
        max.first = max_iter->first;
        max.second = max_iter->second;
    }

    void UnCompressZip::getFileList(std::vector<std::string>& files, const std::string& except)const
    {
        for (const auto& m : m_files_info_)
        {
            if (m.first != except)
                files.push_back(m.first);
        }
    }

    void UnCompressZip::getFileList(std::map<std::string, unsigned long long>& uncompress_files)const
    {
        for (const auto& m : m_files_info_)
            uncompress_files.insert(m);
    }

    bool UnCompressZip::doExtractCurrentFile(unzFile unfile)
    {
        if (unfile == nullptr)
            return false;

        char filename[MAXFILENAME];
        int err = UNZ_OK;
        unz_file_info64 file_info;
        err = unzGetCurrentFileInfo64(unfile, &file_info, filename, MAXFILENAME, NULL, 0, NULL, 0);
        if (err!=UNZ_OK)
            return false;

        std::string file_path = getDirPath() + "/" + filename;
        SLOG_CORE("uncompresse file:" << file_path <<", size:" << file_info.uncompressed_size);
        if (FileHelper::isFileDir(filename))
        {
            mkdir(file_path.c_str(), 0775);
            unzCloseCurrentFile(unfile);
            return true;
        }
        else
        {
            err = unzOpenCurrentFile(unfile);
            if (err!=UNZ_OK)
            {
                unzCloseCurrentFile(unfile);
                return false;
            }
            FILE *out = fopen(file_path.c_str(), "wb");
            if (out == nullptr)
            {
                unzCloseCurrentFile(unfile);
                return false;
            }

            void* read_buffer = (void*)malloc(WRITEBUFFERSIZE);
            if (read_buffer == nullptr)
            {
                fflush(out);
                fclose(out);
                unzCloseCurrentFile(unfile);
                return false;
            }
            do
            {
                err = unzReadCurrentFile(unfile, read_buffer, WRITEBUFFERSIZE);
                if (err < 0)
                    break;
                if (err > 0 && fwrite(read_buffer, (unsigned)err, 1, out)!=1)
                    break;
            }
            while(err > 0);
            addFileInfo(file_path, file_info.uncompressed_size);
            fflush(out);
            fclose(out);
            if (err == UNZ_OK)
                err = unzCloseCurrentFile(unfile);
            else
                unzCloseCurrentFile(unfile);
            if (err != UNZ_OK)
                SLOG_ERROR("UnCompress file unzCloseCurrentFile error:"<<file_path);
            free(read_buffer);
        }
        return err == UNZ_OK ? true : false;
    }

    ZipCode UnCompressZip::unCompress()
    {
        unzFile unfile = unzOpen64(m_zip_path_.c_str());
        if (unfile == nullptr)
            return UnZipFound;

        uLong i = 0;
        unz_global_info64 global_info;
        int err = 0;
        if (unzGetGlobalInfo64(unfile, &global_info) != UNZ_OK)
        {
            unzClose(unfile);
            return UnZipError;
        }
        for (i = 0; i < global_info.number_entry; ++i)
        {
            if (!doExtractCurrentFile(unfile))
            {
                unzClose(unfile);
                return UnZipWriteError;
            }
            if ((i+1) < global_info.number_entry)
            {
                err = unzGoToNextFile(unfile);
                if (err!=UNZ_OK)
                {
                    unzClose(unfile);
                    return UnZipError;
                }
            }
        }
        unzClose(unfile);
        return UnZipOK;
    }

    int GzipHelper::compress(const void *buf, const size_t& buf_size, void *compress_data, size_t &compress_size)
    {
        z_stream c_stream;
        int err = Z_DATA_ERROR;
        if (buf && buf_size > 0)
        {
            c_stream.zalloc = Z_NULL;
            c_stream.zfree  = Z_NULL;
            c_stream.opaque = Z_NULL;
            c_stream.avail_in = buf_size;
            c_stream.next_in  = (Bytef *)buf;
            c_stream.avail_out= buf_size * 2;
            c_stream.next_out = (Bytef *)compress_data;
            err = deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
            if (err != Z_OK)
            {
                return err;
            }
            err = deflate(&c_stream, Z_FINISH);
            if (err != Z_STREAM_END)
            {
                deflateEnd(&c_stream);
                return err;
            }
            deflateEnd(&c_stream);
            compress_size = c_stream.total_out;
            return Z_OK;
        }
        return err;
    }

    int GzipHelper::unCompress(const char * data, int size, char *uncompress_data, size_t uncompress_size)
    {
        z_stream strm;
        strm.zalloc = (alloc_func)0;
        strm.zfree  = (free_func)0;
        strm.opaque = (voidpf)0;
        strm.avail_in = size;
        strm.avail_out= uncompress_size;
        strm.next_in  = (Bytef *)data;
        strm.next_out = (Bytef *)uncompress_data;
        int err = -1;
        int ret = -1;
        err = inflateInit2(&strm, MAXWBITS+GZIPENCODING);
        if (err == Z_OK)
        {
            err = inflate(&strm, Z_FINISH);
            if (err == Z_STREAM_END)
            {
                ret = strm.total_out;
            }
            else
            {
                inflateEnd(&strm);
                return err;
            }
        }
        else
        {
            inflateEnd(&strm);
            return err;
        }
        inflateEnd(&strm);
        return err;
    }

    bool CompressZip::AddFileToZip(zipFile zf, const std::string& relativeInZip, const std::string& sourcePath)
    {
        FILE* fps{ NULL };
        int err{ ZIP_ERRNO };
        bool ret{ false };
        zip_fileinfo zi { 0 };
        memset(&zi, 0, sizeof(zip_fileinfo));
        std::string newFileName{ relativeInZip };
        err = zipOpenNewFileInZip(zf, newFileName.c_str(), &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
        if (ZIP_OK != err)
        {
            SLOG_ERROR("open path file error:" << relativeInZip);
            return false;
        }
        ret = InnerWriteFileToZip(zf, sourcePath);
        err = zipCloseFileInZip(zf);
        return ret && (ZIP_OK == err);
    }


    bool CompressZip::AddDirToZip(zipFile zf, const std::string& relative)
    {
        zip_fileinfo zi{ 0 };
        memset(&zi, 0, sizeof(zip_fileinfo));
        int ret{ ZIP_ERRNO };
        std::string newRelative { relative + "/" };
        ret = zipOpenNewFileInZip(zf, newRelative.c_str(), &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
        if (ZIP_OK != ret)
        {
            SLOG_ERROR("open path dir error:" << relative);
            return false;
        }
        ret = zipCloseFileInZip(zf);
        return ret == ZIP_OK;
    }

    bool CompressZip::InnerWriteFileToZip(zipFile zf, const std::string& path)
    {
        FILE* fin = fopen(path.c_str(), "rb");
        if (NULL == fin)
        {
            SLOG_ERROR("open write path error:" << path);
            return false;
        }
        void* read_buf = nullptr;
        read_buf = (void*)malloc(WRITEBUFFERSIZE);
        if (read_buf == nullptr)
        {
            SLOG_ERROR("error:compress memery not enough");
            fclose(fin);
            return false;
        }
        int err = ZIP_OK;
        size_t size_read = 0;
        do
        {
            err == ZIP_OK;
            size_read = fread(read_buf, 1, WRITEBUFFERSIZE, fin);
            if (size_read < WRITEBUFFERSIZE)
            {
                if (feof(fin) == 0)
                {
                    err = ZIP_ERRNO;
                    break;
                }
            }

            if (size_read > 0)
            {
                err = zipWriteInFileInZip(zf, read_buf, (unsigned)size_read);
            }

        }while(err == ZIP_OK && size_read > 0);
        fclose(fin);
        return ZIP_OK == err;
    }

    bool CompressZip::compressDirExportZip(const std::string& sourcePath, const std::string& zipPath, bool contain_base)
    {
        if (!FileUtil::dirExists(sourcePath))
        {
            SLOG_ERROR("compressFile dir not exist");
            return false;
        }
        std::string dir_path = sourcePath;
        std::string::size_type pos1 = sourcePath.find_last_not_of("/");
        if (pos1 != std::string::npos)
            dir_path = sourcePath.substr(0, pos1+1);
        std::string::size_type pos2 = dir_path.find_last_of("/");
        std::string dir_name = dir_path;
        if (pos2 != std::string::npos)
            dir_name = dir_path.substr(pos2+1);
        SLOG_CORE("compressFile dir ->start:" << dir_path << "  dir_name:" << dir_name);
        
        int ret { ZIP_ERRNO };
        zipFile zf = zipOpen(zipPath.c_str(), APPEND_STATUS_CREATE);
        if (contain_base)    
            AddDirToZip(zf, dir_name);
        vector<string> files;
        FileUtil::dir_filepaths(dir_path, files);
        for (string rdf_file : files)
        {
            std::string zip_path = rdf_file;
            if (!contain_base)
                zip_path = rdf_file.substr(dir_path.size()+1);
            else if (pos2 != std::string::npos)
                zip_path = rdf_file.substr(pos2+1);
            if (FileUtil::dirExists(rdf_file))
            {
                SLOG_CORE("compress dir:" << rdf_file);
                AddDirToZip(zf, zip_path);
            }
            else
            {
                SLOG_CORE("compress file:" << rdf_file);
                AddFileToZip(zf, zip_path, rdf_file);
            }
        }
        ret = zipClose(zf, NULL);
        SLOG_CORE("compress end: " << zipPath << ", zip close stsatus: " << ret);
        return ZIP_OK == ret;
    }
}