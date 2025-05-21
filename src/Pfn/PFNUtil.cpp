#include "PFNUtil.h"

PFNUtil::PFNUtil() 
{
    pthread_rwlock_init(&pfn_data_lock, NULL);
    pfn_base_path = Util::getExactPath(Util::getConfigureValue("pfn_base_path").c_str()) + "/";
    pfn_cpp_path = pfn_base_path + "cpp/";
    pfn_lib_path = pfn_base_path + "lib/";
}

PFNUtil::~PFNUtil() 
{
    pthread_rwlock_destroy(&pfn_data_lock);
}

std::string PFNUtil::fun_cppcheck(std::string username, struct PFNInfo *fun_info)
{
    string report_detail = "";
    string file_name = fun_info->funName;
    string cpp_path = pfn_cpp_path + username;
    string report_path = cpp_path + "/report.txt";
    string check_file_path = cpp_path + "/" + file_name + "_temp.cpp";
    string cppcheck = "cppcheck -j 10 --force suppress=missingIncludeSystem --template=\"[line:{line}]:({severity}) {message}\" --output-file="+report_path+" "+check_file_path;     
    string lookAtfile = "cat " + check_file_path;
    ofstream fout(check_file_path.c_str());
    if (fout.is_open())
    {
        string content;
        content = PFNUtil::fun_build_source_data(fun_info, false);
        fout << content;
        fout.close();
    } else {
        throw new runtime_error("cannot write to file " + check_file_path);
    }
    system(lookAtfile.c_str());
    system(cppcheck.c_str());
    ifstream cppcheck_fin(report_path.c_str());
    if(cppcheck_fin.is_open()){
        string data = "";
        while(getline(cppcheck_fin,data)){
            report_detail += data;
            report_detail += "\r\n";
        }
    }
    cppcheck_fin.close();
    FileUtil::removePath(report_path);
    FileUtil::removePath(check_file_path);
    return report_detail;
}

void PFNUtil::fun_query(const string &fun_name, const string &fun_status, const string &username, std::shared_ptr<PFNInfos> pfn_infos)
{
    string cpp_path = pfn_base_path + "cpp/" + username;
    string json_file_path = cpp_path + "/data.json";
    if (FileUtil::fileExists(json_file_path) == false)
    {
        return;
    }
    ifstream in;
    string line;
    string temp_str;
    pthread_rwlock_rdlock(&pfn_data_lock);
    in.open(json_file_path.c_str(), ios::in);
    string match_name = "\"funName\":\"" + fun_name + "\"";
    string match_state = "\"funStatus\":\"" + fun_status + "\"";
    PFNInfo *temp_ptr;
    while (getline(in, line))
    {
        if (!fun_name.empty() && line.find(match_name) == string::npos)
        {
            continue;
        }
        if (!fun_status.empty() && line.find(match_state) == string::npos)
        {
            continue;
        }
        pfn_infos->addPFNInfo(line);
    }
    in.close();
    pthread_rwlock_unlock(&pfn_data_lock);
}

void PFNUtil::fun_create(const string &username, struct PFNInfo *pfn_info)
{
    string content;
    string file_name = pfn_info->funName;
    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    string cpp_path = pfn_cpp_path + username;
    FileUtil::createDirs(cpp_path);
    string file_path = cpp_path + "/" + file_name + ".cpp";
    SLOG_CORE("file_path: " + file_path);
    if (FileUtil::fileExists(file_path))
    {
        throw std::invalid_argument("function name " + pfn_info->funName + " already exists");
    }
    // cppcheck start
    string report_detail = "";
    // report_detail = fun_cppcheck(username, pfn_infos);
    // cppcheck end
    
    // save fun in file
    if (report_detail.size() == 0)
    {
        ofstream fout(file_path.c_str());
        if (fout)
        {
            content = PFNUtil::fun_build_source_data(pfn_info, true);
            SLOG_CORE("fun_build_source_data success");
            fout << content;
            fout.close();
        } 
        else 
        {
            SLOG_ERROR("open file error: " + file_path);
            throw new runtime_error("cannot write to file " + file_path);
        }
        // save method info to json file
        pfn_info->funStatus = "1";
        PFNUtil::fun_write_json_file(username, pfn_info, "1");
    }
    else
    {
        throw std::runtime_error(report_detail);
    }
}

void PFNUtil::fun_update(const std::string &username, struct PFNInfo *pfn_info)
{
    string content;
    string file_name = pfn_info->funName;

    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    string cpp_path = pfn_cpp_path + username;
    string file_path = cpp_path + "/" + file_name + ".cpp";
    if (FileUtil::fileExists(file_path) == false)
    {
        throw std::invalid_argument("function name " + pfn_info->funName + " does not exist");
    }
    // cpp check start
    string report_detail = "";
    // report_detail = fun_cppcheck(username, pfn_info);
    // cpp check end
    
    // save fun in file
    if (report_detail.size() == 0)
    {
        ofstream fout(file_path.c_str());
        if (fout) 
        {
            content = PFNUtil::fun_build_source_data(pfn_info, true);
            SLOG_CORE("fun_build_source_data success");
            fout << content;
            fout.close();
        } 
        else 
        {
            SLOG_ERROR("open file error: " + file_path);
            throw new runtime_error("cannot write to file " + file_path);
        }
        // save method info to json file
        pfn_info->funStatus = "1";
        PFNUtil::fun_write_json_file(username, pfn_info, "2");
    }
    else
    {
        throw std::runtime_error(report_detail);
    }
}

void PFNUtil::fun_delete(const std::string &username, struct PFNInfo *pfn_info)
{
    PFNUtil::fun_parse_from_name(username, pfn_info->funName, pfn_info);
    PFNUtil::fun_write_json_file(username, pfn_info, "3");
}

string PFNUtil::fun_build(const std::string &username, const std::string fun_name)
{
    string file_name = fun_name;
    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    string cpp_path = pfn_cpp_path + username;
    string lib_path = pfn_lib_path + username;
    string sourceFile = cpp_path + "/" + file_name + ".cpp";
    if (!FileUtil::fileExists(sourceFile))
    {
        throw std::invalid_argument("Function source file dose not exist");
    }
    // get function info from json file
    PFNInfo *fun_info = new PFNInfo();
    string json_str;
    PFNUtil::fun_parse_from_name(username, fun_name, fun_info);

    //create a tmp file
    string last_time = gutil::TimeUtil::now();
    string md5str = Util::md5(last_time);
    string targetDir = lib_path + "/.tmp";
    FileUtil::createDirs(targetDir);
    string targetFile = targetDir + "/lib" + file_name + md5str + ".so";
    string logFile = targetDir + "/lib" + file_name + md5str + ".out";
    FileUtil::removePath(targetFile);
    string pfn_lib = pfn_base_path + "lib/";
    string pfn_include = pfn_base_path + "include/";
    string libaray = pfn_lib + "libganalysis.so";
    string cmd = "g++ -std=c++17 -fPIC " + sourceFile + " -shared -o " + targetFile + " " + libaray + " -L " + pfn_lib + " -llog4cplus" + " -I" + pfn_include + " 2>" + logFile;
    SLOG_CORE("fun_build g++:" << cmd);
    int status;
    status = system(cmd.c_str());
    string error_msg = "";
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
        // update function status to 2
        fun_info->funStatus = "2";
        //delete old so
        string oldLibPrefix = "lib" + file_name;
        vector<std::string> oldLibFiles = PFNUtil::get_files(lib_path.c_str(), oldLibPrefix);
        for (std::string oldLibFile : oldLibFiles)
        {
            SLOG_CORE("delete old so file: " << oldLibFile);
            std::string oldLibPath = lib_path + "/" + oldLibFile;
            FileUtil::removePath(oldLibPath);
        }
        // mv the new into using Path
        std::string new_so_path = lib_path + "/lib" + file_name + md5str + ".so";
        FileUtil::movePath(targetFile, new_so_path);
    }
    else if (FileUtil::fileExists(logFile))
    {
        // update function status to 3
        fun_info->funStatus = "3";
        ifstream errorFile(logFile.c_str());
        string line;
        stringstream strStream;
        while (getline(errorFile, line, '\n'))
        {
            strStream << line;
        }
        error_msg = strStream.str();
    }
    else
    {
        // update function status to 3
        fun_info->funStatus = "3";
        error_msg = "unknown error";
    }

    // update function status and last_time
    fun_info->lastTime = last_time;
    PFNUtil::fun_write_json_file(username, fun_info, "2");
    // delete
    delete fun_info;
    fun_info = NULL;
    FileUtil::removePath(logFile);
    // has error_msg
    if (error_msg.size() > 0)
    {
        return error_msg;
    }
    return "";
}

void PFNUtil::fun_review(const std::string &username, struct PFNInfo *pfn_info)
{
    string content = PFNUtil::fun_build_source_data(pfn_info, true);
    pfn_info->funBody = content;
}

std::string PFNUtil::fun_build_source_data(struct PFNInfo * fun_info, bool has_header)
{
    const string fun_name = fun_info->funName;
    string fun_subs = fun_info->funSubs;
    gutil::StringUtil::url_decode(fun_subs);
    string fun_body = fun_info->funBody;
    gutil::StringUtil::url_decode(fun_body);
    char *fun_body_o = (char *)calloc(fun_body.length() + 1, sizeof(char));
    if(fun_body_o != NULL) 
    {
        Util::a_trim(fun_body_o, fun_body.c_str());
        if(fun_body_o && strlen(fun_body_o) > 0)
            fun_body = string(fun_body_o);
        xfree(fun_body_o);
    }
    stringstream _buf;
    if (has_header)
    {
        _buf << PFN_HEADER;
    }
    
    if (fun_subs.length() > 0)
    {
        _buf << fun_subs << '\n';
    }

    _buf << "extern \"C\" bool " + fun_name;
    _buf << "(GAnalysis& ganalysis, const std::string& params, std::string& result)\n";

    bool add_brace = false;
    if (fun_body[0] != '{')
    {
        _buf << "{\n";
        add_brace = true;
    }
    _buf << fun_body;
    if (add_brace)
    {
        _buf << "\n}";
    }
    return _buf.str();
}

void PFNUtil::fun_write_json_file(const std::string& username, struct PFNInfo *fun_info, std::string operation)
{
    string cpp_path = pfn_cpp_path + username;
    string lib_path = pfn_lib_path + username;
    string json_file_path = cpp_path + "/data.json";
    std::string fun_name = fun_info->funName; 
    nlohmann::json json_data;
    std::string json_str;
    fun_info->toJSON(json_data);
    json_str = json_data.dump();
    if (operation == "1") // create
    {
        pthread_rwlock_wrlock(&pfn_data_lock);
        if (!FileUtil::fileExists(json_file_path))
        {
            FileUtil::createDirs(cpp_path);
            FileUtil::createFile(json_file_path);
        }
        FILE *fp = fopen(json_file_path.c_str(), "a");
        if (fp == NULL)
        {
            pthread_rwlock_unlock(&pfn_data_lock);
            throw std::runtime_error("open function json file error");
        }
        json_str.push_back('\n');
        fprintf(fp, "%s", json_str.c_str());
        fclose(fp);
        pthread_rwlock_unlock(&pfn_data_lock);
    }
    else if (operation == "2" || operation == "3") // update or remove
    {
        ifstream in;
        stringstream _buf;
        string line;
        string cmd;
        string temp_name;
        pthread_rwlock_wrlock(&pfn_data_lock);
        in.open(json_file_path.c_str(), ios::in);
        if (!in.is_open())
        {
            pthread_rwlock_unlock(&pfn_data_lock);
            throw std::runtime_error("open function json file error");
        }
        string match_name = "\"funName\":\""+fun_name+"\"";
        while (getline(in, line))
        {
            if (line.find(match_name) != std::string::npos)
            {
                if (operation == "2") // update
                {
                    _buf << json_str << '\n';
                }
                else // remove
                {
                    PFNInfo fun_info_tmp;
                    if (PFNInfo::fromJSON(line, fun_info_tmp))
                    {
                    // remove cpp and so file
                        string file_name = fun_info_tmp.funName;
                        std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
                        string cppPath = cpp_path + "/" + file_name + ".cpp";
                        SLOG_CORE("remove cpp file: " + cppPath);
                        FileUtil::removePath(cppPath);
                        if (fun_info_tmp.lastTime.empty() == false) 
                        {
                            string md5str = Util::md5(fun_info_tmp.lastTime);
                            string libPath = lib_path + "/lib" + file_name + md5str + ".so";
                            SLOG_CORE("remove lib file: " + libPath);
                            FileUtil::removePath(libPath);
                        }
                    }
                }
            }
            else
            {
                _buf << line << '\n';
            }
        }
        in.close();
        line = _buf.str();
        string temp_path = cpp_path + "/tmp.json";
        string back_path = cpp_path + "/back.json";
        ofstream out(temp_path.c_str());
        if (!out.is_open())
        {
            pthread_rwlock_unlock(&pfn_data_lock);
            throw std::runtime_error("open function json tmp file error.");
        }
        out << line;
        out.close();
        // mv pfn/cpp/{username}/data.json pfn/cpp/{username}/back.json
        if (FileUtil::movePath(json_file_path, back_path))
        {
            // mv pfn/cpp/{username}/tmp.json pfn/cpp/{username}/data.json
            if (FileUtil::movePath(temp_path, json_file_path))
            {
                // remove old json file
                FileUtil::removePath(back_path);
                pthread_rwlock_unlock(&pfn_data_lock);
            }
            else // recover back.json to data.json
            {
                FileUtil::movePath(back_path, json_file_path);
                pthread_rwlock_unlock(&pfn_data_lock);
                throw std::runtime_error("save function info to json file error");
            }
        }
        else
        {
            pthread_rwlock_unlock(&pfn_data_lock);
            throw std::runtime_error("save function info to json file error");
        }
    }
    else
    {
        throw std::invalid_argument("save function info to json file error, no match operation:" + operation);
    }
}

void PFNUtil::fun_parse_from_name(const std::string& username, const std::string& fun_name, struct PFNInfo *fun_info)
{
    string cpp_path = pfn_cpp_path + username;
    string json_path = cpp_path + "/data.json";
    ifstream in;
    pthread_rwlock_rdlock(&pfn_data_lock);
    in.open(json_path.c_str(), ios::in);
    if (!in.is_open())
    {
        pthread_rwlock_unlock(&pfn_data_lock);
        throw std::runtime_error("open function json file error.");
    }
    string line;
    bool isMatch = false;
    string match_name = "\"funName\":\""+fun_name+"\"";
    while (getline(in, line))
    {
        if (line.find(match_name) != std::string::npos)
        {
            isMatch = true;
            break;
        }
    }
    in.close();
    pthread_rwlock_unlock(&pfn_data_lock);
    if (isMatch)
    {
        if(!PFNInfo::fromJSON(line, *fun_info))
        {
            SLOG_ERROR("parse function info from json error: " + line);
            throw std::invalid_argument("parse function(" + fun_name + ") faild.");
        }
    }
    else
    {
        throw std::invalid_argument("function " + fun_name + " not exists");
    }
}

//get all specific file type files in a directory
vector<string> 
PFNUtil::get_files(const char *src_dir, const string& prefix)
{
    vector<string> result;
    string directory(src_dir);

    DIR *dir = opendir(src_dir);
    if ( dir == NULL )
    {
        printf("[ERROR] %s is not a directory or not exist!", src_dir);
        return result;
    }
 
    struct dirent* d_ent = NULL;
 
    char dot[3] = ".";
    char dotdot[6] = "..";
 
    while ( (d_ent = readdir(dir)) != NULL )
    {
        if ( (strcmp(d_ent->d_name, dot) != 0) && (strcmp(d_ent->d_name, dotdot) != 0) )
        {
            if ( d_ent->d_type != DT_DIR)
            {
                string d_name(d_ent->d_name);
                if (d_name.find(prefix) != std::string::npos)
                {
                    result.push_back(d_name);
                }
            }
        }
    }
    closedir(dir);
    return result;
}