/*=============================================================================
# Filename: Util.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-16 10:43
# Description:
1. firstly written by liyouhuan, modified by zengli
2. achieve functions in Util.h
=============================================================================*/

#include "Util.h"

using namespace std;
//==================================================================================================================
//configure() to config the basic options of gStore system
//==================================================================================================================

pthread_rwlock_t backuplog_lock;

// #define BACKUP_PATH "./backups"

//database home directory, which is an absolute path by config
//TODO:everywhere using database, the prefix should be it
//string Util::db_home = ".";

//false:single true:distribute
//bool Util::gstore_mode = false;

//control the debug information
//string Util::debug_level = "simple";

//database placed in which path
//string Util::db_path = ".";

//the suffix to be added to database name
//string Util::db_suffix = ".db";

//the maxium buffer size assigned to gStore system
//string Util::buffer_maxium = "100"; //the unit is GB

//the maxium thread num assigned to gStore system
//string Util::thread_maxium = "1000";

//if record logs in gStore system(to be recoverable or faster)
//string Util::operation_logs = "true";

//==================================================================================================================

//NOTICE:used in Database, Join and Strategy
//int Util::triple_num = 0;
//int Util::pre_num = 0;
//int Util::entity_num = 0;
//int Util::literal_num = 0;

//QUERY: assign all in Util()?
//BETTER:assigned in KVstore, not one tree?
FILE* Util::debug_kvstore = NULL;            //used by KVstore
FILE* Util::debug_database = NULL;			 //used by Database
FILE* Util::debug_vstree = NULL;			 //used by VSTree

string Util::gserver_port_file = "bin/.gserver_port";
string Util::gserver_port_swap = "bin/.gserver_port.swap";
string Util::gserver_log = "logs/gserver.log";

//set hash table
HashFunction Util::hash[] = { Util::simpleHash, Util::APHash, Util::BKDRHash, Util::DJBHash, Util::ELFHash, \
	Util::DEKHash, Util::BPHash, Util::FNVHash, Util::HFLPHash, Util::HFHash, Util::JSHash, \
	Util::PJWHash, Util::RSHash, Util::SDBMHash, Util::StrHash, Util::TianlHash, NULL};

//remove spaces in the left
char*
Util::l_trim(char* szOutput, const char* szInput)
{
    assert(szInput != NULL);
    assert(szOutput != NULL);
    assert(szOutput != szInput);
    for   (; *szInput != '\0' && isspace(*szInput); ++szInput);
    return strcpy(szOutput, szInput);
}

//remove spaces in the right
char*
Util::r_trim(char *szOutput, const char* szInput)
{
    char *p = NULL;
    assert(szInput != NULL);
    assert(szOutput != NULL);
    assert(szOutput != szInput);
    strcpy(szOutput, szInput);
    for(p = szOutput + strlen(szOutput) - 1; p >= szOutput && isspace(*p); --p);
    *(++p) = '\0';
    return szOutput;
}

//remove spaces in the two sides
char*
Util::a_trim(char * szOutput, const char * szInput)
{
    char *p = NULL;
    assert(szInput != NULL);
    assert(szOutput != NULL);
    l_trim(szOutput, szInput);
    for   (p = szOutput + strlen(szOutput) - 1; p >= szOutput && isspace(*p); --p);
    *(++p) = '\0';
    return szOutput;
}

bool
Util::configure()
{
    INIParser ini_parser;
    ini_parser.ReadINI(GlobalTypedef::profile);
    // system
    // Util::setGlobalConfig(ini_parser, "system", "version");
    // Util::setGlobalConfig(ini_parser, "system", "product_name");
    // Util::setGlobalConfig(ini_parser, "system", "website");
    Util::setGlobalConfig(ini_parser, "system", "db_home", "./dbhome/");
    Util::setGlobalConfig(ini_parser, "system", "db_suffix", ".db");
    Util::setGlobalConfig(ini_parser, "system", "root_username", "root");
    Util::setGlobalConfig(ini_parser, "system", "root_password", "123456");
    Util::setGlobalConfig(ini_parser, "system", "system_username", "system");
    Util::setGlobalConfig(ini_parser, "system", "pfn_base_path", "./pfn/");
    Util::setGlobalConfig(ini_parser, "system", "licensetype", "opensource");
    Util::setGlobalConfig(ini_parser, "system", "min_memory", "512");
    Util::setGlobalConfig(ini_parser, "system", "min_million_disk", "50");
    Util::setGlobalConfig(ini_parser, "system", "min_million_memory", "20");
    Util::setGlobalConfig(ini_parser, "system", "resource_check", "off");
    Util::setGlobalConfig(ini_parser, "system", "check_point", "on");
    Util::setGlobalConfig(ini_parser, "system", "build_multi_thread", "on");
    Util::setGlobalConfig(ini_parser, "system", "build_schema", "off");
    // server
    Util::setGlobalConfig(ini_parser, "server", "deamon", "off");
    Util::setGlobalConfig(ini_parser, "server", "port", "9000");
    Util::setGlobalConfig(ini_parser, "server", "tcp_port", "9100");
    Util::setGlobalConfig(ini_parser, "server", "thread_num", "30");
    Util::setGlobalConfig(ini_parser, "server", "max_database_num", "100");
    Util::setGlobalConfig(ini_parser, "server", "max_user_num", "1000");
    Util::setGlobalConfig(ini_parser, "server", "max_output_size", "100000");
    Util::setGlobalConfig(ini_parser, "server", "ip_allow_path");
    Util::setGlobalConfig(ini_parser, "server", "ip_deny_path", "conf/ipDeny.config");
    // log
    Util::setGlobalConfig(ini_parser, "log", "log_mode", "conf/slog.properties");
    Util::setGlobalConfig(ini_parser, "log", "querylog_mode", "1");
    Util::setGlobalConfig(ini_parser, "log", "querylog_path", "logs/endpoint/");
    Util::setGlobalConfig(ini_parser, "log", "querylog_days", "30");
    Util::setGlobalConfig(ini_parser, "log", "accesslog_mode", "1");
    Util::setGlobalConfig(ini_parser, "log", "accesslog_path", "logs/ipaccess/");
    Util::setGlobalConfig(ini_parser, "log", "accesslog_days", "30");
    // backup
    Util::setGlobalConfig(ini_parser, "backup", "backup_path", "./backups/");
    Util::setGlobalConfig(ini_parser, "backup", "max_backups", "3");
    // upload
    Util::setGlobalConfig(ini_parser, "upload", "upload_path", "./upload/");
    Util::setGlobalConfig(ini_parser, "upload", "upload_max_body_size", "1073741824");
    Util::setGlobalConfig(ini_parser, "upload", "upload_allow_extensions", "nt|ttl|n3|rdf|txt");
    Util::setGlobalConfig(ini_parser, "upload", "upload_allow_compress_packages", "zip");
    // cluster
    Util::setGlobalConfig(ini_parser, "cluster", "cluster_on", "off");
    Util::setGlobalConfig(ini_parser, "cluster", "cluster_role");
    Util::setGlobalConfig(ini_parser, "cluster", "cluster_node"); 
    Util::setGlobalConfig(ini_parser, "cluster", "cluster_heartbeat", "3"); 
    Util::setGlobalConfig(ini_parser, "cluster", "cluster_relpy_timeout", "5"); 
    Util::setGlobalConfig(ini_parser, "cluster", "cluster_data_path", "cluster/"); 

    // create db_home
    string temp_str = GlobalTypedef::global_config["db_home"];
    gs::StringUtil::append(temp_str, '/');
    GlobalTypedef::global_config["db_home"] = temp_str;
    FileUtil::createDirs(temp_str);
    
    // create backup_path
    temp_str = Util::getConfigureValue("backup_path");
    gs::StringUtil::append(temp_str, '/');
    GlobalTypedef::global_config["backup_path"] = temp_str;
    FileUtil::createDirs(temp_str);

    // create export_path
    temp_str = GlobalTypedef::export_path;
    FileUtil::createDirs(temp_str);

    // create pfn_base_path
    temp_str = GlobalTypedef::global_config["pfn_base_path"];
    gs::StringUtil::append(temp_str, '/');
    GlobalTypedef::global_config["pfn_base_path"] = temp_str;
    FileUtil::createDirs(temp_str + "cpp");
    FileUtil::createDirs(temp_str + "lib");

    // create upload_path
    temp_str = GlobalTypedef::global_config["upload_path"];
    gs::StringUtil::append(temp_str, '/');
    GlobalTypedef::global_config["upload_path"] = temp_str;
    FileUtil::createDirs(temp_str);

    // create logs path
    temp_str = GlobalTypedef::global_config["querylog_path"];
    gs::StringUtil::append(temp_str, '/');
    GlobalTypedef::global_config["querylog_path"] = temp_str;
    FileUtil::createDirs(temp_str);

    temp_str = GlobalTypedef::global_config["accesslog_path"];
    gs::StringUtil::append(temp_str, '/');
    GlobalTypedef::global_config["accesslog_path"] = temp_str;
    FileUtil::createDirs(temp_str);

    temp_str = GlobalTypedef::global_config["queryresult_path"];
    gs::StringUtil::append(temp_str, '/');
    GlobalTypedef::global_config["queryresult_path"] = temp_str;
    FileUtil::createDirs(temp_str);
    
    // create cluster path
    if (GlobalTypedef::global_config["cluster_on"] == "on")
    {        
        temp_str = GlobalTypedef::global_config["cluster_data_path"];
        gs::StringUtil::append(temp_str, '/');
        GlobalTypedef::global_config["cluster_data_path"] = temp_str;
        FileUtil::createDirs(temp_str);
    }

    // init slog
    string log_mode = Util::getConfigureValue("log_mode");
	if (gs::FileUtil::fileExists(log_mode))
	{
		log4cplus::PropertyConfigurator::doConfigure(LOG4CPLUS_TEXT(log_mode));
	}
	else
	{
		log4cplus::BasicConfigurator::doConfigure();
		SLOG_INFO("log config file not exist, use default config");
	}
    if (GlobalTypedef::isEnabledFor(log4cplus::TRACE_LOG_LEVEL))
    {
        vector<std::string> headers = {"name", "value"};
        PrettyPrint pp(headers);
        std::vector<std::vector<std::string>> rows;
        for (map<string, string>::iterator it = GlobalTypedef::global_config.begin(); it != GlobalTypedef::global_config.end(); ++it)
        {
            pp.addRow({it->first, it->second});
        }
        stringstream _ss;
        _ss << "configuration params: " << endl;
        pp.print(_ss);
        SLOG_CORE(_ss.str());
    }
    return true;
}

bool Util::setGlobalConfig(INIParser& parser, string rootname, string keyname, string default_value)
{
    string value = parser.GetValue(rootname, keyname);
    if(value.empty()==false)
        GlobalTypedef::global_config[keyname] = StringUtil::replace_all(value,"\"","");
    else
        GlobalTypedef::global_config[keyname] = default_value;
    return true;
}

string Util::getConfigureValue(const std::string& keyname, string default_value)
{
    map<string, string>::iterator iter = GlobalTypedef::global_config.find(keyname);
	if (iter != GlobalTypedef::global_config.end())
	{
		return iter->second;
	}
	return default_value;
}

int32_t Util::getConfigureIntValue(const std::string& keyname, int32_t default_value)
{
    string value = Util::getConfigureValue(keyname);
    if (!value.empty()) 
    {
        return stoi(value);
    }
    return default_value;
}

uint64_t Util::getConfigureLongValue(const std::string& keyname, uint64_t default_value)
{
    string value = Util::getConfigureValue(keyname);
    if (!value.empty()) 
    {
        return stoll(value);
    }
    return default_value;
}

Util::Util()
{
    Util::configure();
#ifdef DEBUG_KVSTORE
    if(this->debug_kvstore == NULL)
    {
        string s = this->debug_path + "kv.log";
        this->debug_kvstore = fopen(s.c_str(), "w+");
        if(this->debug_kvstore == NULL)
        {
            cerr << "open error: kv.log\n";
            this->debug_kvstore = stderr;
        }
    }
#endif
#ifdef DEBUG_DATABASE
    if(this->debug_database == NULL)
    {
        string s = this->debug_path + "db.log";
        this->debug_database = fopen(s.c_str(), "w+");
        if(this->debug_database == NULL)
        {
            cerr << "open error: db.log\n";
            this->debug_database = stderr;
        }
    }
#endif
#ifdef DEBUG_VSTREE
    if(this->debug_vstree == NULL)
    {
        string s = this->debug_path + "vs.log";
        this->debug_vstree = fopen(s.c_str(), "w+");
        if(this->debug_vstree == NULL)
        {
            cerr << "open error: vs.log\n";
            this->debug_vstree = stderr;
        }
    }
#endif
}

Util::~Util()
{
#ifdef DEBUG_KVSTORE
    fclose(this->debug_kvstore);	//NULL is ok, just like free(NULL)
    this->debug_kvstore = NULL;
#endif
#ifdef DEBUG_DATABASE
    fclose(this->debug_database);	//NULL is ok, just like free(NULL)
    this->debug_database = NULL;
#endif
}

bool
Util::is_literal_ele(TYPE_ENTITY_LITERAL_ID _id)
{
    return _id >= GlobalTypedef::LITERAL_FIRST_ID;
}

bool 
Util::is_entity_ele(TYPE_ENTITY_LITERAL_ID id) 
{
	return id < GlobalTypedef::LITERAL_FIRST_ID;
}

bool 
Util::isEntity(const std::string& _str)
{
	if(_str[0] == '<')
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool 
Util::isLiteral(const std::string& _str)
{
	if(_str[0] == '"')
	{
		return true;
	}
	else
	{
		return false;
	}
}


//NOTICE: require that the list is ordered
unsigned
Util::removeDuplicate(unsigned* _list, unsigned _len)
{
	if (_list == NULL || _len == 0) {
		return 0;
	}
	unsigned valid = 0, limit = _len - 1;
	for(unsigned i = 0; i < limit; ++i)
	{
		if(_list[i] != _list[i+1])
		{
			_list[valid++] = _list[i];
		}
	}
	_list[valid++] = _list[limit];

	return valid;
}

int
Util::cmp_int(const void* _i1, const void* _i2)
{
    return *(int*)_i1  -  *(int*)_i2;
}

int
Util::cmp_unsigned(const void* _i1, const void* _i2)
{
	unsigned t1 = *(unsigned*)_i1;
	unsigned t2 = *(unsigned*)_i2;
	if(t1 > t2)
	{
		return 1;
	}
	else if(t1 == t2)
	{
		return 0;
	}
	else //t1 < t2
	{
		return -1;
	}
}

bool
Util::parallel_cmp_unsigned(unsigned _i1, unsigned _i2)
{
	return _i1 < _i2;
}

void
Util::sort(unsigned*& _id_list, unsigned _list_len)
{
    #ifndef PARALLEL_SORT
	std::qsort(_id_list, _list_len, sizeof(unsigned), Util::cmp_unsigned);
    #else
    omp_set_num_threads(thread_num);
    __gnu_parallel::sort(_id_list, _id_list + _list_len, Util::parallel_cmp_unsigned);
    #endif
}

unsigned
Util::bsearch_int_uporder(unsigned _key, const unsigned* _array, unsigned _array_num)
{
    if (_array_num == 0)
    {
        //return -1;
		return INVALID;
    }
    if (_array == NULL)
    {
        //return -1;
		return INVALID;
    }

    unsigned _first = _array[0];
    unsigned _last = _array[_array_num - 1];

    if (_last == _key)
    {
        return _array_num - 1;
    }

    if (_last < _key || _first > _key)
    {
        //return -1;
		return INVALID;
    }

    unsigned low = 0;
    unsigned high = _array_num - 1;

    unsigned mid;
    while (low <= high)
    {
        mid = (high - low) / 2 + low;
        if (_array[mid] == _key)
        {
            return mid;
        }
        if (_array[mid] > _key)
        {
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }

    //return -1;
	return INVALID;
}

// bool
// Util::bsearch_preid_uporder(TYPE_PREDICATE_ID _preid, unsigned* _pair_idlist, unsigned _list_len)
// {
//     if(_list_len == 0)
//     {
//         return false;
//     }
// 	//NOTICE: if list len > 0, then it must >= 2, so pair num >= 1
//     unsigned pair_num = _list_len / 2;
//     unsigned _first = _pair_idlist[2*0 + 0];
//     unsigned _last = _pair_idlist[2*(pair_num-1) + 0];
//
//     if(_preid == _last)
//     {
//         return true;
//     }
//
//     bool not_find = (_last < _preid || _first > _preid);
//     if(not_find)
//     {
//         return false;
//     }
//
//     unsigned low = 0;
//     unsigned high = pair_num - 1;
//     unsigned mid;
//
//     while(low <= high)
//     {
//         mid = (high - low) / 2 + low;
//         if(_pair_idlist[2*mid + 0] == _preid)
//         {
//             return true;
//         }
//
//         if(_pair_idlist[2*mid + 0] > _preid)
//         {
//             high = mid - 1;
//         }
// 		else
//         {
//             low = mid + 1;
//         }
//     }
//
//     return false;
// }

unsigned
Util::bsearch_vec_uporder(unsigned _key, const vector<unsigned>* _vec)
{
    unsigned tmp_size = _vec->size();
    if (tmp_size == 0)
    {
        //return -1;
		return INVALID;
    }

    unsigned _first = (*_vec)[0];
    unsigned _last = (*_vec)[tmp_size - 1];

    if (_key == _last)
    {
        return tmp_size - 1;
    }

    bool not_find = (_last < _key || _first > _key);
    if (not_find)
    {
        //return -1;
		return INVALID;
    }

    unsigned low = 0;
    unsigned high = tmp_size - 1;
    unsigned mid;

    while (low <= high)
    {
        mid = (high - low) / 2 + low;
        if ((*_vec)[mid] == _key)
        {
            return mid;
        }

        if ((*_vec)[mid] > _key)
        {
            high = mid - 1;
        }
        else
        {
            low = mid + 1;
        }
    }

    //return -1;
	return INVALID;
}

string
Util::result_id_str(vector<unsigned*>& _v, int _var_num)
{
    stringstream _ss;

	unsigned size = _v.size();
    for(unsigned i = 0; i < size; ++i)
    {
        unsigned* _p_int = _v[i];
        _ss << "[";
        for(int j = 0; j < _var_num-1; j ++)
        {
            _ss << _p_int[j] << ",";
        }
        _ss << _p_int[_var_num-1] << "]\t";
    }

    return _ss.str();
}



string
Util::get_backup_time(const std::string& path, const std::string& separator)
{
    string::size_type position;
    position = path.find(separator);

    if(position == std::string::npos)
        return "";
    // format: yyyyMMddHHmmss
    string time_str = path.substr(position + 1);
    if(time_str.size() != 14 && !is_number(time_str))
        return "";
    time_t timestamp = TimeUtil::parse(time_str, PURE_DATETIME_PATTERN);
    return TimeUtil::format(timestamp);
}

bool
Util::is_number(string s)
{
	if (s.empty()) return false;
    string::size_type pos = 0;
    for(; pos < s.size(); pos++){
        if(!isdigit(s[pos])) return false;
    }
    return true;
}

//NOTICE: there does not exist itoa() function in Linux, atoi() is included in stdlib.h
//itoa() is not a standard C function, and it is only used in Windows.
//However, there do exist a function called sprintf() in standard library which can replace itoa()
//char str[255];
//sprintf(str, "%x", 100); //将100转为16进制表示的字符串
char* 
Util::itoa(int num, char* str, int radix) //the last parameter means the number's radix: decimal, or octal formats
{
	//index table
	char index[]="0123456789ABCDEF";
	unsigned unum;
	int i=0,j,k;
	if(radix==10&&num<0)  //negative in decimal
	{
		unum=(unsigned)-num;
		str[i++]='-';
	}
	else unum=(unsigned)num;
	do{
		str[i++]=index[unum%(unsigned)radix];
		unum/=radix;
	}while(unum);
	str[i]='\0';
	//reverse order
	if(str[0]=='-')k=1;
	else k=0;
	char temp;
	for(j=k;j<=(i-1)/2;j++)
	{
		temp = str[j];
		str[j] = str[i-1+k-j];
		str[i-1+k-j] = temp;
	}
	return str;
}


string
Util::showtime()
{
//	fputs("\n\n", logsfp);
    time_t now;
    time(&now);
//	fputs(ctime(&now), logsfp);
    return string("\n\n") + ctime(&now);
}

string
Util::getQueryFromFile(const char* _file_path)
{
#ifdef DEBUG_PRECISE
    cerr << "file to open: " << _file_path <<endl;
#endif
    char buf[10000];
    std::string query_file;

    ifstream fin(_file_path);
    if(!fin)
    {
        printf("can not open: %s\n", _file_path);
        return "";
    }

    memset(buf, 0, sizeof(buf));
    stringstream _ss;
    while(!fin.eof())
    {
        fin.getline(buf, 9999);
        _ss << buf << "\n";
    }
    fin.close();

    return _ss.str();
}

string
Util::getItemsFromDir(string _path)
{
	DIR* dp = NULL;
	struct dirent* entry;
	string ret = "";
	if((dp = opendir(_path.c_str())) == NULL)
	{
		fprintf(stderr, "error opening directory!\n");
	}
	else
	{
		while((entry = readdir(dp)) != NULL)
		{
#ifdef DEBUG_PRECISE
			fprintf(stderr, "%s\n", entry->d_name);
#endif
			string name= string(entry->d_name);
			int len = name.length();
			if(len <= 3)
			{
				continue;
			}

			//BETTER: change to use db_suffix instead
			if(name.substr(len-3, 3) == ".db")
			{
				string tmp = name.substr(0, len-3);
				if(ret == "")
					ret = tmp;
				else
					ret = ret + "  " + tmp;
			}
		}
		closedir(dp);
	}
#ifdef DEBUG_PRECISE
	fprintf(stderr, "OUT getItemsFromDir\n");
#endif
	return ret;
}

//NOTICE: system() is implemented by fork() and exec(), the latter will change the whole control flow, 
//so it must be used with fork()
//The fork() will copy the whole process's image, including the heap and stack.
//But in UNIX, this copy is only logical, in practice it is copy-on-write, which will save a lot memory usage.
//Another function is vfork(), which won't copy a lot of things
//
//http://www.cnblogs.com/wuchanming/p/3784862.html
//http://www.cnblogs.com/sky-heaven/p/4687489.html
//
//BETTER: sysytem() may have some risks, using popen() instead?
string
Util::getSystemOutput(string cmd)
{
    string ans = "";
    string file = GlobalTypedef::tmp_path;
    file += "ans.txt";
    cmd += " > ";
    cmd += file;
    //cerr << cmd << endl;
    int ret = system(cmd.c_str());
    cmd = "rm -rf " + file;
    if(ret < 0)
    {
        fprintf(stderr, "system call failed:%s\n", cmd.c_str());
        // system(cmd.c_str());
        FileUtil::removePath(file);
        return "";
    }

    ifstream fin(file.c_str());
    if(!fin)
    {
        cerr << "getSystemOutput: Fail to open : " << file << endl;
        return "";
    }

	string temp;
	getline(fin, temp);
	while(!fin.eof())
	{
		//cout<<"system line"<<endl;
		if(ans == "")
			ans = temp;
		else
			ans = ans + "\n" + temp;
		getline(fin, temp);
	}
    fin.close();
    //FILE *fp = NULL;
    //if((fp = fopen(file.c_str(), "r")) == NULL)
    //{
    //fprintf(stderr, "unbale to open file: %s\n", file.c_str());
    //}
    //else
    //{
    //char *ans = (char *)malloc(100);
    //fgets(path, 100, fp);
    //char *find = strchr(path, '\n');
    //if(find != NULL)
    //*find = '\0';
    //fclose(fp);
    //}
    // system(cmd.c_str());
    FileUtil::removePath(file);
	//cerr<<"ans: "<<ans<<endl;
    return ans;
}

// Get the exact file path from given string: ~, ., symbol links
string
Util::getExactPath(const char *str)
{
    // string cmd = "realpath ";
    // cmd += string(str);

    // return getSystemOutput(cmd);
    struct stat st;    
    if (lstat(str, &st) == -1)
    {
        return "";
    }
    char real_path[PATH_MAX];
    if (S_ISLNK(st.st_mode))
    {
        ssize_t num_bytes = readlink(str, real_path, PATH_MAX - 1);
        if (num_bytes == -1)
        {
            return "";
        }
        real_path[num_bytes] = '\0';
    }
    if (realpath(str, real_path) == nullptr)
    {
        return "";
    }
    return real_path;
}

unsigned
Util::BKDRHash(const char *_str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int key = 0;

    //for(unsigned i = 0; i < i; ++i)
	while(*_str)
    {
        //key = key * seed + _str[i];
		key = key * seed + *(_str++);
    }

    return (key & 0x7FFFFFFF);
}

unsigned
Util::simpleHash(const char *_str)
{
    unsigned int key;
    unsigned char *p;

    for(key = 0, p = (unsigned char *)_str; *p; p++)
        key = 31 * key + *p;

    return (key & 0x7FFFFFFF);
}

unsigned
Util::RSHash(const char *_str)
{
    unsigned int b = 378551;
    unsigned int a = 63689;
    unsigned int key = 0;

    while (*_str)
    {
        key = key * a + (*_str++);
        a *= b;
    }

    return (key & 0x7FFFFFFF);
}

unsigned
Util::JSHash(const char *_str)
{
    unsigned int key = 1315423911;

    while (*_str)
    {
        key ^= ((key << 5) + (*_str++) + (key >> 2));
    }

    return (key & 0x7FFFFFFF);
}

unsigned
Util::PJWHash(const char *_str)
{
    unsigned int bits_in_unsigned_int = (unsigned int)(sizeof(unsigned int) * 8);
    unsigned int three_quarters = (unsigned int)((bits_in_unsigned_int * 3) / 4);
    unsigned int one_eighth = (unsigned int)(bits_in_unsigned_int / 8);

    unsigned int high_bits = (unsigned int)(0xFFFFFFFF) << (bits_in_unsigned_int - one_eighth);
    unsigned int key = 0;
    unsigned int test = 0;

    while (*_str)
    {
        key = (key << one_eighth) + (*_str++);
        if ((test = key & high_bits) != 0)
        {
            key = ((key ^ (test >> three_quarters)) & (~high_bits));
        }
    }

    return (key & 0x7FFFFFFF);
}

unsigned
Util::ELFHash(const char *_str)
{
    unsigned int key = 0;
    unsigned int x  = 0;

    while (*_str)
    {
        key = (key << 4) + (*_str++);
        if ((x = key & 0xF0000000L) != 0)
        {
            key ^= (x >> 24);
            key &= ~x;
        }
    }

    return (key & 0x7FFFFFFF);
}

unsigned
Util::SDBMHash(const char *_str)
{
    unsigned int key = 0;

    while (*_str)
    {
        key = (*_str++) + (key << 6) + (key << 16) - key;
    }

    return (key & 0x7FFFFFFF);
}

unsigned
Util::DJBHash(const char *_str)
{
    unsigned int key = 5381;
    while (*_str) {
        key += (key << 5) + (*_str++);
    }
    return (key & 0x7FFFFFFF);
}

unsigned
Util::APHash(const char *_str)
{
    unsigned int key = 0;
    int i;

    for (i=0; *_str; i++)
    {
        if ((i & 1) == 0)
        {
            key ^= ((key << 7) ^ (*_str++) ^ (key >> 3));
        }
        else
        {
            key ^= (~((key << 11) ^ (*_str++) ^ (key >> 5)));
        }
    }

    return (key & 0x7FFFFFFF);
}

unsigned
Util::DEKHash(const char* _str)
{
    unsigned int hash = strlen(_str);
    for(; *_str; _str++) 
	{
        hash = ((hash << 5) ^ (hash >> 27)) ^ (*_str);
    }
    return hash;
}

unsigned
Util::BPHash(const char* _str)
{
    unsigned int hash = 0;
    for(; *_str; _str++) 
	{
        hash = hash << 7 ^ (*_str);
    }

    return hash;
}

unsigned
Util::FNVHash(const char* _str)
{
    const unsigned int fnv_prime = 0x811C9DC5;
    unsigned int hash = 0;

    for(; *_str; _str++) 
	{
        hash *= fnv_prime;
        hash ^= (*_str);
    }

    return hash;
}

unsigned
Util::HFLPHash(const char* _str)
{
    unsigned int n = 0;
    char* b = (char*)&n;
    unsigned int len = strlen(_str);
    for(unsigned i = 0; i < len; ++i)
    {
        b[i%4] ^= _str[i];
    }
    return n%len;
}

unsigned
Util::HFHash(const char* _str)
{
    int result=0;
    const char* ptr = _str;
    int c;
    unsigned int len = strlen(_str);
    for(int i=1; (c=*ptr++); i++)
        result += c*3*i;
    if (result<0)
        result = -result;
    return result%len;
}

unsigned
Util::StrHash(const char* _str)
{
    unsigned int h;
    unsigned char *p;
    for(h = 0, p = (unsigned char *)_str; *p; p++) 
	{
        h = 31 * h + *p;
    }

    return h;

}

unsigned
Util::TianlHash(const char* _str)
{
    unsigned long urlHashValue=0;
    int ilength=strlen(_str);
    int i;
    unsigned char ucChar;
    if(!ilength)  {
        return 0;
    }
    if(ilength<=256)  {
        urlHashValue=16777216*(ilength-1);
    } else {
        urlHashValue = 42781900080;
    }
    if(ilength<=96) {
        for(i=1; i<=ilength; i++) {
            ucChar = _str[i-1];
            if(ucChar<='Z'&&ucChar>='A')  {
                ucChar=ucChar+32;
            }
            urlHashValue+=(3*i*ucChar*ucChar+5*i*ucChar+7*i+11*ucChar)%1677216;
        }
    } else  {
        for(i=1; i<=96; i++)
        {
            ucChar = _str[i+ilength-96-1];
            if(ucChar<='Z'&&ucChar>='A')
            {
                ucChar=ucChar+32;
            }
            urlHashValue+=(3*i*ucChar*ucChar+5*i*ucChar+7*i+11*ucChar)%1677216;
        }
    }

    return urlHashValue;
}

//NOTICE: the time of log() and sqrt() in C can be seen as constant

//NOTICE:_b must >= 1
double
Util::logarithm(double _a, double _b)
{
    //REFRENCE: http://blog.csdn.net/liyuanbhu/article/details/8997850
    //a>0 != 1; b>0 (b>=2 using log/log10/change, 1<b<2 using log1p, b<=1?)
    if(_a <= 1 || _b < 1)
        return -1.0;
    double under = log(_a);
    if(_b == 1)
        return 0.0;
    else if(_b < 2)
        return log1p(_b - 1) / under;
    else //_b >= 2
        return log(_b) / under;
    return -1.0;
}


void
Util::intersect(unsigned*& _id_list, unsigned& _id_list_len, const unsigned* _list1, unsigned _len1, const unsigned* _list2, unsigned _len2)
{
	vector<unsigned> res;
	//cout<<"intersect prevar: "<<_len1<<"   "<<_len2<<endl;
	if(_list1 == NULL || _len1 == 0 || _list2 == NULL || _len2 == 0)
	{
		_id_list = NULL;
		_id_list_len = 0;
	}

	//when size is almost the same, intersect O(n)
	//when one size is small ratio, search in the larger one O(mlogn)
	//
	//n>0 m=nk(0<k<1) 
	//compare n(k+1) and nklogn: k0 = log(n/2)2 requiring that n>2
	//k<=k0 binary search; k>k0 intersect
	int method = -1; //0: intersect 1: search in list1 2: search in list2
	unsigned n = _len1;
	double k = 0;
	if(n < _len2)
	{
		k = (double)n / (double)_len2;
		n = _len2;
		method = 2;
	}
	else
	{
		k = (double)_len2 / (double)n;
		method = 1;
	}
	if(n <= 2)
		method = 0;
	else
	{
		double limit = Util::logarithm(n/2, 2);
		if(k > limit)
			method = 0;
	}

	switch(method)
	{
	case 0:
	{   //this bracket is needed if vars are defined in case
		unsigned id_i = 0;
		unsigned num = _len1;
		for(unsigned i = 0; i < num; ++i)
		{
			unsigned can_id = _list1[i];
			while((id_i < _len2) && (_list2[id_i] < can_id))
			{
				id_i ++;
			}

			if(id_i == _len2)
			{
				break;
			}

			if(can_id == _list2[id_i])
			{
				res.push_back(can_id);
				id_i ++;
			}
		}
		break;
	}
	case 1:
	{
		for(unsigned i = 0; i < _len2; ++i)
		{
			if(Util::bsearch_int_uporder(_list2[i], _list1, _len1) != INVALID)
				res.push_back(_list2[i]);
		}
		break;
	}
	case 2:
	{
		unsigned m = _len1, i;
		for(i = 0; i < m; ++i)
		{
			unsigned t = _list1[i];
			if(Util::bsearch_int_uporder(t, _list2, _len2) != INVALID)
				res.push_back(t);
		}
		break;
	}
	default:
		cerr << "no such method in Util::intersect()" << endl;
		break;
	}

	_id_list_len = res.size();

	if (_id_list_len == 0) {
		_id_list = NULL;
	}
	else {
		_id_list = new unsigned[_id_list_len];
		for (unsigned i = 0; i < _id_list_len; ++i)
			_id_list[i] = res[i];
	}
	delete[] _list1;
	delete[] _list2;
}

int
Util::compIIpair(int _a1, int _b1, int _a2, int _b2)
{
	if(_a1 == _a2 && _b1 == _b2)
		return 0;
	else if(_a1 < _a2 || (_a1 == _a2 && _b1 <= _b2))
		return -1;
	else
		return 1;
}

bool 
Util::isValidPort(string str) 
{
	//valid port number: 0 - 65535
	if(str.length() < 1 || str.length() > 5) 
	{
		return false;
	}

	unsigned i;
	for(i = 0; i < str.length(); i++) 
	{
		if(str[i] < '0' || str[i] > '9') 
		{
			return false;
		}
	}

	int port = stoi(str);
	if(port < 0 || port>65535) 
	{
		return false;
	}

	return true;
}

bool 
Util::isValidIP(string str) 
{
	if(str == "localhost") 
	{
		return true;
	}
	return (Util::isValidIPV4(str) || Util::isValidIPV6(str));
}

bool 
Util::isValidIPV4(string str) 
{
	regex_t reg;
	char pattern[] = "^(([01]?[0-9][0-9]?|2[0-4][0-9]|25[0-5])\\.){3}([01]?[0-9][0-9]?|2[0-4][0-9]|25[0-5])$";
	regcomp(&reg, pattern, REG_EXTENDED | REG_NOSUB);
	regmatch_t pm[1];
	int status = regexec(&reg, str.c_str(), 1, pm, 0);
	regfree(&reg);
	if(status == REG_NOMATCH) 
	{
		return false;
	}
	return true;
}

bool 
Util::isValidIPV6(string str) 
{
	//TO BE IMPLEMENTED
	return false;
}

//TODO: change these compare functions from int to unsigned, but take care of the returned values
int 
Util::_spo_cmp(const void* _a, const void* _b) 
{
	int** _p_a = (int**)_a;
	int** _p_b = (int**)_b;

	int _sub_id_a = (*_p_a)[0];
	int _sub_id_b = (*_p_b)[0];
	if (_sub_id_a != _sub_id_b) {
		return _sub_id_a - _sub_id_b;
	}

	int _pre_id_a = (*_p_a)[1];
	int _pre_id_b = (*_p_b)[1];
	if (_pre_id_a != _pre_id_b) {
		return _pre_id_a - _pre_id_b;
	}

	int _obj_id_a = (*_p_a)[2];
	int _obj_id_b = (*_p_b)[2];
	if (_obj_id_a != _obj_id_b) {
		return _obj_id_a - _obj_id_b;
	}

	return 0;
}

bool
Util::parallel_spo_cmp(int* _a, int* _b)
{
        int _sub_id_a = _a[0];
        int _sub_id_b = _b[0];
        if (_sub_id_a != _sub_id_b) {
                return _sub_id_a < _sub_id_b;
        }

        int _pre_id_a = _a[1];
        int _pre_id_b = _b[1];
        if (_pre_id_a != _pre_id_b) {
                return _pre_id_a < _pre_id_b;
        }

        int _obj_id_a = _a[2];
        int _obj_id_b = _b[2];
        if (_obj_id_a != _obj_id_b) {
                return _obj_id_a < _obj_id_b;
        }

        return 0;
}

int 
Util::_ops_cmp(const void* _a, const void* _b) 
{
	int** _p_a = (int**)_a;
	int** _p_b = (int**)_b;

	int _obj_id_a = (*_p_a)[2];
	int _obj_id_b = (*_p_b)[2];
	if (_obj_id_a != _obj_id_b) {
		return _obj_id_a - _obj_id_b;
	}

	int _pre_id_a = (*_p_a)[1];
	int _pre_id_b = (*_p_b)[1];
	if (_pre_id_a != _pre_id_b) {
		return _pre_id_a - _pre_id_b;
	}

	int _sub_id_a = (*_p_a)[0];
	int _sub_id_b = (*_p_b)[0];
	if (_sub_id_a != _sub_id_b) {
		return _sub_id_a - _sub_id_b;
	}

	return 0;
}

bool
Util::parallel_ops_cmp(int* _a, int* _b)
{
        int _obj_id_a = _a[2];
        int _obj_id_b = _b[2];
        if (_obj_id_a != _obj_id_b) {
                return _obj_id_a < _obj_id_b;
        }

        int _pre_id_a = _a[1];
        int _pre_id_b = _b[1];
        if (_pre_id_a != _pre_id_b) {
                return _pre_id_a < _pre_id_b;
        }

        int _sub_id_a = _a[0];
        int _sub_id_b = _b[0];
        if (_sub_id_a != _sub_id_b) {
                return _sub_id_a < _sub_id_b;
        }

        return 0;
}

int 
Util::_pso_cmp(const void* _a, const void* _b) 
{
	int** _p_a = (int**)_a;
	int** _p_b = (int**)_b;

	int _pre_id_a = (*_p_a)[1];
	int _pre_id_b = (*_p_b)[1];
	if (_pre_id_a != _pre_id_b) {
		return _pre_id_a - _pre_id_b;
	}

	int _sub_id_a = (*_p_a)[0];
	int _sub_id_b = (*_p_b)[0];
	if (_sub_id_a != _sub_id_b) {
		return _sub_id_a - _sub_id_b;
	}

	int _obj_id_a = (*_p_a)[2];
	int _obj_id_b = (*_p_b)[2];
	if (_obj_id_a != _obj_id_b) {
		return _obj_id_a - _obj_id_b;
	}

	return 0;
}

bool
Util::parallel_pso_cmp(int* _a, int* _b)
{
        int _pre_id_a = _a[1];
        int _pre_id_b = _b[1];
        if (_pre_id_a != _pre_id_b) {
                return _pre_id_a < _pre_id_b;
        }

        int _sub_id_a = _a[0];
        int _sub_id_b = _b[0];
        if (_sub_id_a != _sub_id_b) {
                return _sub_id_a < _sub_id_b;
        }

        int _obj_id_a = _a[2];
        int _obj_id_b = _b[2];
        if (_obj_id_a != _obj_id_b) {
                return _obj_id_a < _obj_id_b;
        }

        return 0;
}

bool 
Util::spo_cmp_idtuple(const ID_TUPLE& a, const ID_TUPLE& b)
{
	if(a.subid != b.subid)
	{
		return a.subid < b.subid;
	}

	if(a.preid != b.preid)
	{
		return a.preid < b.preid;
	}

	if(a.objid != b.objid)
	{
		return a.objid < b.objid;
	}

	//all are equal, no need to sort this two
	return false;
}

bool 
Util::ops_cmp_idtuple(const ID_TUPLE& a, const ID_TUPLE& b)
{
	if(a.objid != b.objid)
	{
		return a.objid < b.objid;
	}

	if(a.preid != b.preid)
	{
		return a.preid < b.preid;
	}

	if(a.subid != b.subid)
	{
		return a.subid < b.subid;
	}

	//all are equal, no need to sort this two
	return false;
}

bool 
Util::pso_cmp_idtuple(const ID_TUPLE& a, const ID_TUPLE& b)
{
	if(a.preid != b.preid)
	{
		return a.preid < b.preid;
	}

	if(a.subid != b.subid)
	{
		return a.subid < b.subid;
	}

	if(a.objid != b.objid)
	{
		return a.objid < b.objid;
	}

	//all are equal, no need to sort this two
	return false;
}

bool 
Util::equal(const ID_TUPLE& a, const ID_TUPLE& b)
{
	if(a.subid == b.subid && a.preid == b.preid && a.objid == b.objid)
	{
		return true;
	}
	return false;
}

//require that _base>=1
unsigned 
ceiling(unsigned _val, unsigned _base)
{
	//WARN: we donot check overflow here
	return (_val+_base-1) / _base * _base;
}

long 
Util::read_backup_time() 
{
	return Util::gserver_backup_time;
}

std::string
Util::clear_angle_brackets(std::string _str)
{
    size_t len = _str.size();
    if (_str[0] == '<' && _str[len-1] == '>')
    {
        _str = _str.substr(1, len-2);
    } 
    else if (_str[0] == '<') 
    {
        _str = _str.substr(1, len-1);
    }
    else if (_str[len-1] == '>') 
    {
        _str = _str.substr(0, len-2);
    }
    return _str;
}

void
Util::split(const std::string& str, const std::string& pat, std::vector<std::string>& res){
    std::string token;
    size_t start = 0;
    size_t end;
    size_t pat_length = pat.length();
    while ((end = str.find(pat, start)) != std::string::npos)
    {
        token = str.substr(start, end-start);
        // remove lr blank chars
        token.erase(0, token.find_first_not_of(' '));
        token.erase(token.find_last_not_of(' ') + 1);
        res.push_back(token);
        start = end + pat_length;
    }
    // push last token
    token = str.substr(start);
    token.erase(0, token.find_first_not_of(' '));
    token.erase(token.find_last_not_of(' ') + 1);
    if(!token.empty()) 
    {
        res.push_back(token);
    }
}

string Util::getStrValue(const std::string& str)
{
    if (str.size() >= 3)
    {
        return str.substr(1, str.size()-2);
    }
    return str;
}

void 
Util::init_backuplog()
{
    // pthread_rwlock_wrlock(&backuplog_lock);
    // FILE* fp = fopen(BACKUP_LOG_PATH, "w");

    // Document document;
    // document.SetObject();
    // Document::AllocatorType &allocator = document.GetAllocator();

    // document.AddMember("db_name", StringRef(GlobalTypedef::system_db.c_str()), allocator);
    // document.AddMember("backup_timer", DEFALUT_BACKUP_INTERVAL, allocator);
    // StringBuffer buffer;
    // PrettyWriter<StringBuffer> writer(buffer);
    // document.Accept(writer);
    // string rec = buffer.GetString();
    // rec = Util::string_replace(rec, "\n", "");
    // rec = Util::string_replace(rec, "    ", "");
    // rec.push_back('\n');
    // fputs(rec.c_str(), fp);

    // fclose(fp);
    // pthread_rwlock_unlock(&backuplog_lock);
}

int 
Util::add_backuplog(string db_name)
{
    // if(db_name == GlobalTypedef::system_db){
    //     SLOG_ERROR("system can not be duplicated");
    //     return -1;
    // }
    // if(has_record_backuplog(db_name)) return 1;
    // pthread_rwlock_wrlock(&backuplog_lock);
    // FILE* fp = fopen(BACKUP_LOG_PATH, "a");
    // Document document;
    // document.SetObject();
    // Document::AllocatorType &allocator = document.GetAllocator();

    // string time = gs::TimeUtil::now(NORM_DATETIME_PATTERN);

    // document.AddMember("db_name", StringRef(db_name.c_str()), allocator);
    // document.AddMember("backup_interval", DEFALUT_BACKUP_INTERVAL, allocator);
    // document.AddMember("last_backup_time", StringRef(time.c_str()), allocator);
    // document.AddMember("is_backup", "false", allocator);
    // StringBuffer buffer;
    // PrettyWriter<StringBuffer> writer(buffer);
    // document.Accept(writer);
    // string rec = buffer.GetString();
    // rec = Util::string_replace(rec, "\n", "");
    // rec = Util::string_replace(rec, "    ", "");
    // rec.push_back('\n');
    // fputs(rec.c_str(), fp);

    // fclose(fp);
    // pthread_rwlock_unlock(&backuplog_lock);
    return 0;
}

int 
Util::delete_backuplog(string db_name)
{
    // if(db_name == GlobalTypedef::system_db){
    //     SLOG_ERROR("system can not be deleted!");
    //     return -1;
    // }
    // pthread_rwlock_wrlock(&backuplog_lock);
    // FILE* fp = fopen(BACKUP_LOG_PATH, "r");
    // FILE* fp1 = fopen(BACKUP_LOG_TMEP_PATH, "w");
    // char readBuffer[0xffff];
    int ret = 1;
    // while(fgets(readBuffer, 1024, fp)) {
    //     string rec = readBuffer;
    //     StringStream is(readBuffer);
    //     Document d;
    //     d.ParseStream(is);
    //     if(d["db_name"].GetString() == db_name){
    //         ret = 0;
    //         continue;
    //     }
    //     fputs(readBuffer, fp1);
    // }
    // fclose(fp);
    // fclose(fp1);
    // FileUtil::removePath(BACKUP_LOG_PATH);
    // string cmd = "mv ";
    // cmd += BACKUP_LOG_TMEP_PATH;
    // cmd += ' ';
    // cmd += BACKUP_LOG_PATH;
    // system(cmd.c_str());
    // pthread_rwlock_unlock(&backuplog_lock);
    return ret;
}

int 
Util::update_backuplog(string db_name, string parameter, string value)
{
    // if(parameter == "db_name"){
    //    SLOG_ERROR("parameter can not be db_name!");
    //     return -1;
    // }
    // pthread_rwlock_wrlock(&backuplog_lock);
    // FILE* fp = fopen(BACKUP_LOG_PATH, "r");
    // FILE* fp1 = fopen(BACKUP_LOG_TMEP_PATH, "w");
    // char readBuffer[0xffff];
    int ret = 0;
    // while(fgets(readBuffer, 1024, fp)){
    //     string rec = readBuffer;
    //     StringStream is(readBuffer);
    //     Document d;
    //     d.ParseStream(is);
    //     if(d["db_name"].GetString() != db_name){
    //         fputs(readBuffer, fp1);
    //         continue;
    //     }
    //     if(d.HasMember(parameter.c_str())){
    //         Value& S = d[parameter.c_str()];
    //         S.SetString(value.c_str(), value.length());
    //         StringBuffer buffer;
    //         Writer<StringBuffer> writer(buffer);
    //         d.Accept(writer);
    //         string line = buffer.GetString();
    //         line.push_back('\n');
    //         fputs(line.c_str(), fp1);
    //     }
    //     else{
    //         fputs(readBuffer, fp1);
    //         SLOG_ERROR("wrong parameter!");
    //         ret = 1;
    //     }
    // }
    // fclose(fp);
    // fclose(fp1);
    // FileUtil::removePath(BACKUP_LOG_PATH);
    // string cmd = "mv ";
    // cmd += BACKUP_LOG_TMEP_PATH;
    // cmd += ' ';
    // cmd += BACKUP_LOG_PATH;
    // system(cmd.c_str());
    // pthread_rwlock_unlock(&backuplog_lock);
    return ret;
}

string 
Util::query_backuplog(string db_name, string parameter)
{
    // pthread_rwlock_rdlock(&backuplog_lock);
    // FILE* fp = fopen(BACKUP_LOG_PATH, "r");
    // char readBuffer[0xffff];
    // while(fgets(readBuffer, 1024, fp)){
    //     string rec = readBuffer;
    //     StringStream is(readBuffer);
    //     Document d;
    //     d.ParseStream(is);
    //     if(d["db_name"].GetString() != db_name) continue;
    //     if(d.HasMember(parameter.c_str())){
    //         fclose(fp);
    //         pthread_rwlock_unlock(&backuplog_lock);
    //         return d[parameter.c_str()].GetString();

    //     }
    //     else{
    //         SLOG_ERROR("wrong parameter!");
    //     }
    // }
    // fclose(fp);
    // pthread_rwlock_unlock(&backuplog_lock);
    return " ";
}

void 
Util::search_backuplog(vector<string> &res, string parameter, string value)
{
    // pthread_rwlock_rdlock(&backuplog_lock);
    // FILE* fp = fopen(BACKUP_LOG_PATH, "r");
    // char readBuffer[0xffff];
    // while(fgets(readBuffer, 1024, fp)) {
    //     string rec = readBuffer;
    //     StringStream is(readBuffer);
    //     Document d;
    //     d.ParseStream(is);
    //     if(d.HasMember(parameter.c_str()) && d[parameter.c_str()].GetString() == value)
    //         res.push_back(d["db_name"].GetString());
    // }
    // fclose(fp);
    // pthread_rwlock_unlock(&backuplog_lock);
}

bool 
Util::has_record_backuplog(string db_name)
{
    // pthread_rwlock_rdlock(&backuplog_lock);
    // if(db_name == GlobalTypedef::system_db) 
    //     return true;
    // FILE* fp = fopen(BACKUP_LOG_PATH, "r");
    // char readBuffer[0xffff];
    // while(fgets(readBuffer, 1024, fp)) {
    //     string rec = readBuffer;
    //     StringStream is(readBuffer);
    //     Document d;
    //     d.ParseStream(is);
    //     if (d["db_name"].GetString() == db_name){
    //         SLOG_DEBUG(rec);
    //         pthread_rwlock_unlock(&backuplog_lock);
    //         return true;
    //     }
    // }
    // fclose(fp);
    // pthread_rwlock_unlock(&backuplog_lock);

    return false;
}

long int 
Util::get_timestamp(string& line)
{
    int n = line.length() - 2;
    long int timestamp = 0;
    int i = 0;
    while (isdigit(line[n]))
    {
        timestamp = pow(10, i) * (line[n] - '0') + timestamp;
        i++;
        n--;
    }
    line = line.substr(0, line.length() - i - 2);
    return timestamp;
}

/*!
 * @brief		get the param value from command
 * @param[in]	argc:the length of argc
 * @param[in]	argv:the param list
 * @param[in]   argname:the abbreviation name of param (the format is "-"+argname,e.g. -db)
 * @param[in]   argname2:the full name of param(the format is "--"+argname,e.g. --database)
 * @param[in]   default_value:the default value of the param while the param is not exist.
 * @return 		the value of param
*/
std::string Util::getArgValue(int argc, char* argv[], std::string argname,std::string argname2, std::string default_value)
{

  for (int i = 0; i < argc; i++)
  {
    if ((argv[i] == "-" + argname)||(argv[i]=="--"+argname2))
    {
      if (i + 1 >= argc)
      {
        return "";
      }
      else
      {
        return argv[i + 1];
      }

    }

  }
  //cout << argname << " is not exist,using the default value:" << default_value << endl;
  return default_value;
}

pair<bool, double> Util::checkGetNumericLiteral(string &literal)
{
  if (literal.size() <= 2)
    return make_pair(false, 0);
  if (literal[0] != '"')
    return make_pair(false, 0);

  auto right_quotation_pos =  literal.rfind('"');

  if (right_quotation_pos == 1 || right_quotation_pos == string::npos)
    return make_pair(false, 0);

  const char* last_s = literal.c_str() + right_quotation_pos;
  const char* start = literal.c_str() + 1;
  char* end_ptr;
  double v = std::strtod(start, &end_ptr);
  bool success = end_ptr == last_s;
  return make_pair(success,v);
}

std::string Util::md5(const string& text)
{
    MD5 _md5(text);
    return _md5.toStr();
}

void Util::printConsole(std::vector<std::string> &headers, std::vector<std::vector<std::string>> &rows)
{
    std::cout.flush();
    PrettyPrint pp(headers);
    for(auto row: rows)
        pp.addRow(row);
    pp.print(std::cout);
}

void Util::printFile(std::vector<std::string> &headers, std::vector<std::vector<std::string>> &rows)
{
    PrettyPrint pp(headers);
    for(auto row: rows)
        pp.addRow(row);
    stringstream ss;
    ss << "\n";
    pp.print(ss);
    if (GlobalTypedef::_logger.getAllAppenders().size() == 0)
    {
        Util::configure();
    }
    if(GlobalTypedef::isEnabledFor(log4cplus::INFO_LOG_LEVEL))
    {
        SLOG_INFO(ss.str());
    } 
    else if (GlobalTypedef::isEnabledFor(log4cplus::DEBUG_LOG_LEVEL))
    {
        SLOG_DEBUG(ss.str());
    }
}

std::string Util::currentPath()
{
    std::filesystem::path cur_path = std::filesystem::current_path();
    return cur_path.string();
}