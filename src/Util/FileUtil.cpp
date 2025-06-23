#include "FileUtil.h"

namespace gs
{
    bool FileUtil::readFile(const std::string& path, std::string& content )
    {
        std::ifstream file(path);
        if (!file.is_open())
            return false;
       content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
       file.close();
       return true;
    }

    bool FileUtil::createFile(const std::string& path)
    {
        if (access(path.c_str(), 0) != 0)
        {
            // file permission 0755
            mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
            if (creat(path.c_str(), mode) != 0)
            {
                return false;
            }
        }
        return true;
    }

    bool FileUtil::createDirs(const std::string& path)
    {
        // create all directories in the path
        std::filesystem::path _path(path);
        return std::filesystem::create_directories(_path);
    }

    bool FileUtil::writeLine(const std::string& path, const std::string& line)
    {
        std::ofstream file(path, std::ios_base::app);
        if (!file.is_open())
            return false;
        file << line << std::endl;
        file.flush();
        file.close();
        return true;
    }
    
    bool FileUtil::writeLines(const std::string& path, const std::vector<std::string>& lines)
    {
        std::ofstream file(path, std::ios_base::app);
        if (!file.is_open())
            return false;
        for (std::string line : lines)
        {
            file << line << std::endl;
        }    
        file.flush();
        file.close();
        return true;
    }

    bool FileUtil::fileExists(const std::string& path)
    {
        std::filesystem::path _path(path);
        return std::filesystem::exists(_path) && std::filesystem::is_regular_file(_path);
    }

    bool FileUtil::dirExists(const std::string& path)
    {
        std::filesystem::path _path(path);
        return std::filesystem::exists(_path) && std::filesystem::is_directory(_path);
    }

    bool FileUtil::pathExists(const std::string& path)
    {
        std::filesystem::path _path(path);
        return std::filesystem::exists(_path);
    }

    //is ostream.write() ok to update to disk at once? all add ofstream.flush()?
    //http://bookug.cc/rwbuffer
    //BETTER: add a sync function in Util to support FILE*, fd, and fstream
    void FileUtil::Csync(FILE* _fp)
    {
        //NOTICE: fclose will also do fflush() operation, but not others
        if(_fp == NULL)
        {
            return; 
        }
        //this will update the buffer from user mode to kernel mode
        fflush(_fp);
        //change to Unix fd and use fsync to sync to disk: fileno(stdin)=0
        int fd = fileno(_fp);
        fsync(fd);
        //FILE * fp = fdopen (1, "w+");   //file descriptor to file pointer 
        //NOTICE: disk scheduler also has a small buffer, but there is no matter even if the power is off
        //(UPS for each server to enable the synchronization between scheduler and disk)
    }

    std::string FileUtil::fileName(const std::string &filepath)
    {
        std::string::size_type pos1 = filepath.find_last_not_of("/");
        if (pos1 == std::string::npos)
        {
            return "/";
        }
        std::string::size_type pos2 = filepath.find_last_of("/", pos1);
        if (pos2 == std::string::npos)
        {
            pos2 = 0;
        } else
        {
            pos2++;
        }

        return filepath.substr(pos2, pos1 - pos2 + 1);
    }

    void FileUtil::dir_filepaths(const std::string& _dir, std::vector<std::string> &file_list)
    {
        DIR *dirp = opendir(_dir.c_str());
        if (dirp == NULL)
        {
            if(GlobalTypedef::_logger.getAllAppenders().size() > 0)
            {
                SLOG_WARN("dir is not exist.");
            }
            return;
        }
        struct dirent *dir_entry = NULL;
        std::string file_name;
        struct stat st; 
        while ((dir_entry = readdir(dirp)) != NULL)
        {
            file_name = dir_entry->d_name;
            if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
            {
                continue;
            }
            std::string path;
            if (_dir.back() != '/')
                path = _dir + '/' + file_name;
            else
                path = _dir + file_name;
            stat(path.c_str(), &st);
            file_list.push_back(path);
            if (S_ISDIR(st.st_mode))
                dir_filepaths(path, file_list);
        }
        closedir(dirp);
    }

    void FileUtil::dir_filenames(const std::string& _dir, std::vector<std::string> &file_list, const std::string& _name_contains, const bool& _inlude_path)
    {
        std::string _dir_path = _dir;
        if (_dir_path.back() != '/')
            _dir_path += '/';
        DIR *dirp = opendir(_dir_path.c_str());
        if (dirp == NULL)
        {
            return;
        }
        struct dirent *dir_entry = NULL;
        std::string file_name;
        while ((dir_entry = readdir(dirp)) != NULL)
        {
            file_name = dir_entry->d_name;
            if (strcmp(dir_entry->d_name, ".") == 0 || strcmp(dir_entry->d_name, "..") == 0)
            {
                continue;
            }
            if (!_name_contains.empty() && file_name.find(_name_contains.c_str()) == std::string::npos)
            {
                continue;
            }
            if (_inlude_path)
                file_list.push_back(_dir_path + dir_entry->d_name);
            else
                file_list.push_back(dir_entry->d_name);
        }
        closedir(dirp);
    }

    bool FileUtil::is_file(const std::string& file)
    {
        std::filesystem::path _path(file);
        return std::filesystem::is_regular_file(_path);
    }

    bool FileUtil::is_dir(const std::string& path)
    {
        std::filesystem::path _path(path);
        return std::filesystem::is_directory(_path);
    }

    std::string FileUtil::fileSuffix(const std::string &filepath)
    {
        std::string::size_type pos1 = filepath.find_last_of("/");
        if (pos1 == std::string::npos)
        {
            pos1 = 0;
        }
        else
        {
            pos1++;
        }
        std::string file = filepath.substr(pos1, -1);

        std::string::size_type pos2 = file.find_last_of(".");
        if (pos2 == std::string::npos)
        {
            return "";
        }
        return file.substr(pos2 + 1, -1);
    }

    bool FileUtil::removeDir(const std::string& dir_path)
    {
        std::filesystem::path _path(dir_path);
        std::uintmax_t n = std::filesystem::remove_all(_path);
        return n > 0;
    }

    bool FileUtil::removeFile(const std::string& file_path)
    {
        std::filesystem::path _path(file_path);
        return std::filesystem::remove(_path);
    }

    bool FileUtil::removePath(const std::string& path)
    {
        std::filesystem::path _path(path);
        if (std::filesystem::is_directory(_path))
        {
            std::uintmax_t n = std::filesystem::remove_all(_path);
            return n > 0;
        }    
        else
        {
            return std::filesystem::remove(_path);
        }    
    }

    bool FileUtil::copyFile(const std::string& src_file, const std::string& dst_file)
    {
        std::filesystem::path src_path(src_file);
        std::filesystem::path dst_path(dst_file);
        if (!std::filesystem::exists(src_path))
        {
            return false;
        }
        if (std::filesystem::is_regular_file(src_path))
        {
            std::error_code ec;
            std::filesystem::copy_file(src_path, dst_path, std::filesystem::copy_options::overwrite_existing, ec);
            return ec.value() == 0;
        }
        return false;
    }

    bool FileUtil::copyDir(const std::string& src_dir, const std::string& dst_dir)
    {
        std::filesystem::path src_path(src_dir);
        std::filesystem::path dst_path(dst_dir);
        if (!std::filesystem::exists(src_path))
        {
            return false;
        }
        if (std::filesystem::is_directory(src_path))
        {
            SLOG_CORE("copy from " << src_dir << " to "<<dst_dir);
            std::error_code ec;
            std::filesystem::copy(src_path, dst_path, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive, ec);
            return ec.value() == 0;
        }
        return false;
    }

    bool FileUtil::movePath(const std::string& src_path, const std::string& dst_path)
    {
        std::filesystem::path src(src_path);
        std::filesystem::path dst(dst_path);
        if (!std::filesystem::exists(src))
        {
            return false;
        }
        if (std::filesystem::exists(dst))
        {
            return false;
        }
        std::error_code ec;
        std::filesystem::rename(src, dst, ec);
        return ec.value() == 0;
    }

    std::string FileUtil::parentPath(const std::string& path)
    {
        std::filesystem::path _path(path);
        return _path.parent_path().string();
    }

    /**
     * get file size
     * unit: byte
     * @param path
     */
    std::uint64_t FileUtil::fileSize(const std::string& path)
    {
        std::filesystem::path _path(path);
        if (std::filesystem::exists(_path) && std::filesystem::is_regular_file(_path))
        {
            return std::filesystem::file_size(_path);
        }
        return 0;
    }

    /**
     * count the size of a directory
     * unit: byte
     * @param path
     */
    std::uint64_t FileUtil::dirSize(const std::string& path)
    {
        std::filesystem::path _path(path);
        uint64_t size = 0;
        for (const auto& entry : std::filesystem::recursive_directory_iterator(_path)) {
            if (entry.is_regular_file()) {
                size += std::filesystem::file_size(entry.path());
            }
        }
        return size;
    }

    std::uint64_t FileUtil::fileLines(const std::string& file_path)
    {
        std::filesystem::path _path(file_path);
        if (!std::filesystem::exists(_path) || !std::filesystem::is_regular_file(_path))
        {
            return 0;
        }
        std::string cmd = "wc -l " + file_path + " | awk '{print $1}'";
        char _cmd[1024] = {0};
        strcpy(_cmd, cmd.c_str());
        FILE *ptr;
        uint64_t line_count = 0;
        if ((ptr = popen(_cmd, "r")) != NULL)
        {
            char buf[64];
            char *rt = new char[128]{0};
            while (fgets(buf, 64, ptr) != NULL)
            {
                strcat(rt, buf);
                if (strlen(rt) > 64)
                {
                    break;
                }
            }
            pclose(ptr);
            ptr = NULL;
            line_count = strtoul(rt, NULL, 10);
            delete [] rt;
        }
        else
        {
            std::ifstream file(file_path, std::ios::binary);
            uint64_t file_size = std::filesystem::file_size(_path);
            uint64_t block_size = 1024 * 1024 * 1; // 1M
            char buffer[block_size];
            uint64_t size_to_read = 0;
            uint64_t pos  = 0;
            for(; pos  < file_size; pos += block_size)
            {
                size_to_read = std::min(block_size, file_size - pos);
                file.seekg(pos, std::ios::beg);
                file.read(buffer, size_to_read);
                line_count += std::count(buffer, buffer + size_to_read, '\n');
            }
            if ((file_size - block_size) > 0 && buffer[file_size - block_size - 1] != '\n')
            {
                line_count += 1;
            }
            file.close();
        }
        return line_count;
    }

    bool FileUtil::isEmptyDir(const std::string& dir_path)
    {
        return std::filesystem::is_empty(dir_path);
    }
}