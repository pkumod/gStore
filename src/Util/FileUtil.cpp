#include "FileUtil.h"

namespace gutil
{
    bool FileUtil::readFile(const std::string& path, std::string& content )
    {
        std::ifstream file(path);
        if (!file.is_open())
            return false;
       content = string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
       file.close();
       return true;
    }

    bool FileUtil::createFile(const std::string& path)
    {
        std::ofstream file(path);
        if (!file.is_open())
            return false;
        file.close();
        return true;
    }

    bool FileUtil::writeLine(const std::string& path, const std::string& line)
    {
        std::ofstream file(path, std::ios_base::app);
        if (!file.is_open())
            return false;
        file << line << endl;
        file.flush();
        file.close();
        return true;
    }
    
    bool FileUtil::writeLines(const std::string& path, const vector<std::string>& lines)
    {
        std::ofstream file(path, std::ios_base::app);
        if (!file.is_open())
            return false;
        for (std::string line : lines)
        {
            file << line << endl;
        }    
        file.flush();
        file.close();
        return true;
    }

    bool FileUtil::fileExists(const std::string& path)
    {
        std::ifstream file(path);
        return file.good();
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

}