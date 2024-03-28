#include "PFNUtil.h"

PFNUtil::PFNUtil() 
{
    pthread_rwlock_init(&pfn_data_lock, NULL);
    pfn_base_path = Util::getConfigureValue("pfn_base_path");
}

PFNUtil::~PFNUtil() 
{
    pthread_rwlock_destroy(&pfn_data_lock);
}

std::string PFNUtil::fun_cppcheck(std::string username, struct PFNInfo *fun_info)
{
    string report_detail = "";
    string file_name = fun_info->getFunName();
    string pfn_cpp_path = pfn_base_path + "cpp/" + username;
    string report_path = pfn_cpp_path + "/report.txt";
    string check_file_path = pfn_cpp_path + "/" + file_name + "_temp.cpp";
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
    Util::remove_path(report_path);
    Util::remove_path(check_file_path);
    return report_detail;
}

void PFNUtil::fun_query(const string &fun_name, const string &fun_status, const string &username, struct PFNInfos *pfn_infos)
{
    string pfn_cpp_path = pfn_base_path + "cpp/" + username;
    string json_file_path = pfn_cpp_path + "/data.json";
    if (Util::file_exist(json_file_path) == false)
    {
        return;
    }
    ifstream in;
    string line;
    string temp_str;
    pthread_rwlock_rdlock(&pfn_data_lock);
    in.open(json_file_path.c_str(), ios::in);
    PFNInfo *temp_ptr;
    while (getline(in, line))
    {
        temp_ptr = new PFNInfo(line);
        if (!fun_name.empty() && temp_ptr->getFunName().find(fun_name) == string::npos)
        {
            continue;
        }
        if (!fun_status.empty() && temp_ptr->getFunStatus() != fun_status)
        {
            continue;
        }
        pfn_infos->addPFNInfo(line);
        delete temp_ptr;
        temp_ptr = NULL;
    }
    in.close();
    pthread_rwlock_unlock(&pfn_data_lock);
}

void PFNUtil::fun_create(const string &username, struct PFNInfo *pfn_info)
{
    string content;
    string file_name;
    file_name = pfn_info->getFunName();
    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    string pfn_cpp_path = pfn_base_path + "cpp/" + username;
    Util::create_dirs(pfn_cpp_path);
    string file_path = pfn_cpp_path + "/" + file_name + ".cpp";
    #if defined(DEBUG)
    SLOG_DEBUG("file_path: " + file_path);
    #endif
    if (Util::file_exist(file_path))
    {
        throw std::invalid_argument("function name " + pfn_info->getFunName() + " already exists");
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
            #if defined(DEBUG)
            SLOG_DEBUG("fun_build_source_data success");
            #endif
            fout << content;
            fout.close();
        } 
        else 
        {
            SLOG_ERROR("open file error: " + file_path);
            throw new runtime_error("cannot write to file " + file_path);
        }
        // save method info to json file
        pfn_info->setFunStatus("1");
        PFNUtil::fun_write_json_file(username, pfn_info, "1");
    }
    else
    {
        throw std::runtime_error(report_detail);
    }
}

void PFNUtil::fun_update(const std::string &username, struct PFNInfo *pfn_infos)
{
    string content;
    string file_name;
    file_name = pfn_infos->getFunName();

    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    string pfn_cpp_path = pfn_base_path + "cpp/" + username;
    string file_path = pfn_cpp_path + "/" + file_name + ".cpp";
    if (Util::file_exist(file_path) == false)
    {
        throw std::invalid_argument("function name " + pfn_infos->getFunName() + " not exists");
    }
    // cpp check start
    string report_detail = "";
    // report_detail = fun_cppcheck(username, pfn_infos);
    // cpp check end
    
    // save fun in file
    if (report_detail.size() == 0)
    {
        ofstream fout(file_path.c_str());
        if (fout) 
        {
            content = PFNUtil::fun_build_source_data(pfn_infos, true);
            #if defined(DEBUG)
            SLOG_DEBUG("fun_build_source_data success");
            #endif
            fout << content;
            fout.close();
        } 
        else 
        {
            SLOG_ERROR("open file error: " + file_path);
            throw new runtime_error("cannot write to file " + file_path);
        }
        // save method info to json file
        pfn_infos->setFunStatus("1");
        PFNUtil::fun_write_json_file(username, pfn_infos, "2");
    }
    else
    {
        throw std::runtime_error(report_detail);
    }
}

void PFNUtil::fun_delete(const std::string &username, struct PFNInfo *pfn_infos)
{
     PFNUtil::fun_write_json_file(username, pfn_infos, "3");
}

string PFNUtil::fun_build(const std::string &username, const std::string fun_name)
{
    string file_name = fun_name;
    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
    string pfn_cpp_path = pfn_base_path + "cpp/" + username;
    string pfn_lib_path = pfn_base_path + "lib/" + username;
    string sourceFile = pfn_cpp_path + "/" + file_name + ".cpp";
    if (!Util::file_exist(sourceFile))
    {
        throw std::invalid_argument("function source file is not exist");
    }
    // get function info from json file
    PFNInfo *fun_info = new PFNInfo();
    string json_str;
    PFNUtil::fun_parse_from_name(username, fun_name, fun_info);

    //create a temp file
    string last_time = Util::get_timestamp();
    string md5str = Util::md5(last_time);
    string targetDir = pfn_lib_path + "/.temp";
    Util::create_dirs(targetDir);
    string targetFile = targetDir + "/lib" + file_name + md5str + ".so";
    string logFile = targetDir + "/lib" + file_name + md5str + ".out";
    Util::remove_path(targetFile);
    string libaray = "lib/libgpathqueryhandler.so lib/libgcsr.so";
    string cmd = "g++ -std=c++11 -fPIC " + sourceFile + " -shared -o " + targetFile + " " + libaray + " 2>" + logFile;
    int status;
    status = system(cmd.c_str());
    string error_msg = "";
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
    {
        // update function status to 2
        fun_info->setFunStatus("2");
        //delete old so
        string rmOldSo = "rm -f " + pfn_lib_path +"/lib" + file_name + "*.so";
        system(rmOldSo.c_str());
        //mv the new into using Path
        string mvCmd = "mv " +  targetFile + " " + pfn_lib_path;
        system(mvCmd.c_str());
    }
    else if (Util::file_exist(logFile))
    {
        // update function status to 3
        fun_info->setFunStatus("3");
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
        fun_info->setFunStatus("3");
        error_msg = "unknown error";
    }

    // update function status and last_time
    fun_info->setLastTime(last_time);
    PFNUtil::fun_write_json_file(username, fun_info, "2");
    // delete
    delete fun_info;
    fun_info = NULL;
    Util::remove_path(logFile);
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
    pfn_info->setFunBody(content);
}

std::string PFNUtil::fun_build_source_data(struct PFNInfo * fun_info, bool has_header)
{
    const string fun_name = fun_info->getFunName();
    const string fun_args = fun_info->getFunArgs();
    const string fun_subs = Util::urlDecode(fun_info->getFunSubs());
    string fun_body =  Util::urlDecode(fun_info->getFunBody());
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
        #if defined(DEBUG)
        SLOG_DEBUG("fun header:\n" + PFNUtil::pfn_include_header);
        #endif
        _buf << PFN_HEADER;
    }
    
    if (fun_subs.length() > 0)
    {
        _buf << fun_subs << '\n';
    }

    _buf << "extern \"C\" string " + fun_name;
    #if defined(DEBUG)
    SLOG_DEBUG("fun_args: " + fun_args);
    #endif
    if (fun_args == "1") // int uid, int vid, bool directed, vector<int> pred_set
    {
        _buf << "(std::vector<int> iri_set, bool directed, std::vector<int> pred_set, PathQueryHandler* queryUtil)\n";
    }
    else if (fun_args == "2") // int uid, int vid, bool directed, int k, vector<int> pred_set
    {
        _buf << "(std::vector<int> iri_set, bool directed, int k, std::vector<int> pred_set, PathQueryHandler* queryUtil)\n";
    }
    else
    {
        throw std::runtime_error("the fun_args " + fun_args + " not match: {\"1\", \"2\"}");
    }
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
    string pfn_cpp_path = pfn_base_path + "cpp/" + username;
    string pfn_lib_path = pfn_base_path + "lib/" + username;
    string json_file_path = pfn_cpp_path + "/data.json";
    std::string fun_name = fun_info->getFunName(); 
    std::string json_str = fun_info->toJSON();
    if (operation == "1") // create
    {
        pthread_rwlock_wrlock(&pfn_data_lock);
        if (!Util::file_exist(json_file_path))
        {
            Util::create_dirs(pfn_cpp_path);
            Util::create_file(json_file_path);
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
        PFNInfo *fun_info_tmp;
        while (getline(in, line))
        {
            fun_info_tmp = new PFNInfo(line);
            temp_name = fun_info_tmp->getFunName();
            if (temp_name == fun_name)
            {
                if (operation == "2") // update
                {
                    _buf << json_str << '\n';
                }
                else // remove
                {
                    // rename cpp file and remove so file
                    string file_name = fun_name;
                    std::transform(file_name.begin(), file_name.end(), file_name.begin(), ::tolower);
                    string sourcePath = pfn_cpp_path + "/" + file_name + ".cpp";
                    string libPath = pfn_lib_path + "/lib" + file_name + "*.so";
                    Util::remove_path(sourcePath);
                    Util::remove_path(libPath);
                }
            }
            else
            {
                _buf << line << '\n';
            }
            delete fun_info_tmp;
            fun_info_tmp = NULL;
        }
        in.close();
        line = _buf.str();
        string temp_path = pfn_cpp_path + "/temp.json";
        string back_path = pfn_cpp_path + "/back.json";
        ofstream out(temp_path.c_str());
        if (!out.is_open())
        {
            pthread_rwlock_unlock(&pfn_data_lock);
            throw std::runtime_error("open function json temp file error.");
        }
        out << line;
        out.close();
        // mv pfn/cpp/{username}/data.json pfn/cpp/{username}/back.json
        cmd = "mv -f " + json_file_path + " " + back_path;
        int status;
        status = system(cmd.c_str());
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
        {
            // mv pfn/cpp/{username}/temp.json pfn/cpp/{username}/data.json
            cmd = "mv -f " + temp_path + " " + json_file_path;
            status = system(cmd.c_str());
            #if defined(DEBUG)
            SLOG_DEBUG(cmd);
            #endif
            if (WIFEXITED(status) && WEXITSTATUS(status) == 0)
            {
                // remove old json file
                Util::remove_path(back_path);
                pthread_rwlock_unlock(&pfn_data_lock);
                #if defined(DEBUG)
                SLOG_DEBUG(cmd);
                #endif
            }
            else // recover back.json to data.json
            {
                cmd = "mv -f " + back_path + " " + json_file_path;
                system(cmd.c_str());
                pthread_rwlock_unlock(&pfn_data_lock);
                throw std::runtime_error("save function info to json file error, status code:" + status);
            }
        }
        else
        {
            pthread_rwlock_unlock(&pfn_data_lock);
            throw std::runtime_error("save function info to json file error, status code:" + status);
        }
    }
    else
    {
        throw std::invalid_argument("save function info to json file error, no match operation:" + operation);
    }
}

void PFNUtil::fun_parse_from_name(const std::string& username, const std::string& fun_name, struct PFNInfo *fun_info)
{
    string pfn_cpp_path = pfn_base_path + "cpp/" + username;
    string json_file_path = pfn_cpp_path + "/data.json";
    ifstream in;
    pthread_rwlock_rdlock(&pfn_data_lock);
    in.open(json_file_path.c_str(), ios::in);
    if (!in.is_open())
    {
        pthread_rwlock_unlock(&pfn_data_lock);
        throw std::runtime_error("open function json file error.");
    }
    string line;
    bool isMatch;
    string temp_name;
    PFNInfo* temp_ptr = nullptr;
    isMatch = false;
    while (getline(in, line))
    {
        temp_ptr = new PFNInfo(line);
        temp_name = temp_ptr->getFunName();
        if (temp_name == fun_name)
        {
            isMatch = true;
            break;
        }
        delete temp_ptr;
        temp_ptr = NULL;
    }
    in.close();
    pthread_rwlock_unlock(&pfn_data_lock);
    if (isMatch)
    {
        fun_info->copyFrom(*temp_ptr);
        delete temp_ptr;
        temp_ptr = NULL;
    }
    else
    {
        delete temp_ptr;
        temp_ptr = NULL;
        throw std::invalid_argument("function " + fun_name + " not exists");
    }
}

void PFNUtil::build_PFNInfo(rapidjson::Value &fun_info, struct PFNInfo* pfn_info)
{
	if (fun_info.HasMember("funName") && fun_info["funName"].IsString())
	{
		pfn_info->setFunName(fun_info["funName"].GetString());
	}
	if (fun_info.HasMember("funDesc") && fun_info["funDesc"].IsString())
	{
		pfn_info->setFunDesc(fun_info["funDesc"].GetString());
	}
	if (fun_info.HasMember("funArgs") && fun_info["funArgs"].IsString())
	{
		pfn_info->setFunArgs(fun_info["funArgs"].GetString());
	}
	if (fun_info.HasMember("funBody") && fun_info["funBody"].IsString())
	{
		pfn_info->setFunBody(fun_info["funBody"].GetString());
	}
	if (fun_info.HasMember("funSubs") && fun_info["funSubs"].IsString())
	{
		pfn_info->setFunSubs(fun_info["funSubs"].GetString());
	}
	if (fun_info.HasMember("funStatus") && fun_info["funStatus"].IsString())
	{
		pfn_info->setFunStatus(fun_info["funStatus"].GetString());
	}
	if (fun_info.HasMember("funReturn") && fun_info["funReturn"].IsString())
	{
		pfn_info->setFunReturn(fun_info["funReturn"].GetString());
	}
}