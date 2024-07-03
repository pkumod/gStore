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

    bool FileHelper::compressExportZip(const std::string& dst_path, const std::string& zip_path)
    {
        SLOG_CODE("compressFile->start:"<<zip_path);
        zipFile zfile = zipOpen64(zip_path.c_str(), APPEND_STATUS_CREATE);
        if (zfile == nullptr)
        {
            return false;
        }
        int zip64 = isLargeFile(dst_path.c_str());
        if (zipOpenNewFileInZip64(zfile, dst_path.c_str(), nullptr, nullptr, 0 , nullptr , 0, nullptr, Z_DEFLATED, Z_DEFLATED, zip64) != ZIP_OK)
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
        SLOG_CODE("compressFile->success, err:"<<err);
        free(read_buf);
        zipClose(zfile, nullptr);
        return err == ZIP_OK ? true : false;    
    }

    ///////UnCompressZip///////
    UnCompressZip::UnCompressZip(const std::string& zip_path, const std::string& dir_path)
        :m_zip_path_(zip_path), m_dirpath_(dir_path)
    {
    }

    UnCompressZip::~UnCompressZip()
    {
    }

    void UnCompressZip::addFileInfo(const std::string& file_path, unsigned long long size)
    {
        m_files_info_.insert(std::make_pair(file_path, size));
    }

    string UnCompressZip::getMaxFilePath()const
    {
        std::pair<std::string, unsigned long long> max("", 0); 
        for (const auto& m : m_files_info_)
        {
            if (m.second > max.second)
                max = m;
        }
        return max.first;
    }

    void UnCompressZip::getFileList(std::vector<std::string>& files, const std::string& except)const
    {
        for (const auto& m : m_files_info_)
        {
            if (m.first != except)
                files.push_back(m.first);
        }
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
        SLOG_CODE("unCompress start:file_path"<<file_path<<"size:"<<file_info.uncompressed_size);
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
                SLOG_CODE("UnCompress file unzCloseCurrentFile error:"<<file_path);
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

    int GzipHelper::compress(const std::string *data, void *compress_data, size_t &compress_size)
    {
        z_stream c_stream;
        int err = 0;
        unsigned int size = data->size();
        if (data && size > 0)
        {
            c_stream.zalloc = (alloc_func)0;
            c_stream.zfree  = (free_func)0;
            c_stream.opaque = (voidpf)0;
            if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAXWBITS+GZIPENCODING, 8, Z_DEFAULT_STRATEGY) != Z_OK)
                return -1;
            c_stream.avail_in = size;
            c_stream.avail_out= size;//Generally smaller than the encrypted string
            c_stream.next_in  = (Bytef *)data->c_str();
            c_stream.next_out = (Bytef *)compress_data;
            while (c_stream.avail_in != 0 && c_stream.total_out < size) 
            {
                if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK)
                    return -1;
            }
            if (c_stream.avail_in != 0)
                return c_stream.avail_in;
            for (;;)
            {
                if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END)
                    break;
                if (err != Z_OK)
                    return -1;
            }
            if (deflateEnd(&c_stream) != Z_OK)
                return -1;
            compress_size = c_stream.total_out;
            return 0;
        }
        return -1;
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
        if (!Util::dir_exist(sourcePath))
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
        SLOG_ERROR("compressFile dir ->start:" << dir_path << "  dir_name:" << dir_name);
        
        int ret { ZIP_ERRNO };
        zipFile zf = zipOpen(zipPath.c_str(), APPEND_STATUS_CREATE);
        if (contain_base)    
            AddDirToZip(zf, dir_name);
        vector<string> files;
        Util::dir_filepaths(dir_path, files);
        for (string rdf_file : files)
        {
            std::string zip_path = rdf_file;
            if (!contain_base)
                zip_path = rdf_file.substr(dir_path.size()+1);
            else if (pos2 != std::string::npos)
                zip_path = rdf_file.substr(pos2+1);
            if (Util::dir_exist(rdf_file))
            {
                SLOG_ERROR("compress dir:" << rdf_file);
                AddDirToZip(zf, zip_path);
            }
            else
            {
                SLOG_ERROR("compress file:" << rdf_file);
                AddFileToZip(zf, zip_path, rdf_file);
            }
        }
        ret = zipClose(zf, NULL);
        SLOG_ERROR("compressFile dir ->end:"<< zipPath << "error:" << ret);
        return ZIP_OK == ret;
    }
}