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
using namespace rapidjson;
//==================================================================================================================
//configure() to config the basic options of gStore system
//==================================================================================================================

//string Util::profile = "../init.conf";
string Util::profile = "init.conf";

map<string, string> Util::global_config;
pthread_rwlock_t backuplog_lock;

#define BACKUP_PATH "./backups"
#define BACKUP_LOG_PATH "./backup.json"
#define BACKUP_LOG_TMEP_PATH "./temp.json"
#define DEFALUT_BACKUP_INTERVAL "600" //hour
#define DEFALUT_BACKUP_TIMER "600" //hour

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

//string Util::tmp_path = "../.tmp/";
//string Util::debug_path = "../.debug/";
string Util::tmp_path = ".tmp/";
string Util::debug_path = ".debug/";

//QUERY: assign all in Util()?
//BETTER:assigned in KVstore, not one tree?
FILE* Util::debug_kvstore = NULL;            //used by KVstore
FILE* Util::debug_database = NULL;			 //used by Database
FILE* Util::debug_vstree = NULL;			 //used by VSTree

string Util::gserver_port_file = "bin/.gserver_port";
string Util::gserver_port_swap = "bin/.gserver_port.swap";
string Util::gserver_log = "logs/gserver.log";

string Util::backup_path = "backups/";

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
    const unsigned len = 505;
    char *buf, *c;
    char buf_i[len], buf_o[len];
    FILE *fp = NULL;
	char keyname[len];
	char keyval[len];

	//initialize the settings
	Util::global_config["gstore_mode"] = "single";
	//NOTICE+BETTER+TODO:use macro is better to avoid too many judging on this variable(add a DEBUG macro at the outer)
	Util::global_config["debug_level"] = "simple";
	Util::global_config["log_mode"] = "0";
	Util::global_config["db_home"] = ".";
	Util::global_config["db_suffix"] = ".db";
	Util::global_config["buffer_maxium"] = "100";
	Util::global_config["thread_maxium"] = "1000";
	//TODO:to be recoverable
	Util::global_config["operation_logs"] = "true";

#ifdef DEBUG
	fprintf(stderr, "profile: %s\n", profile.c_str());
#endif
    if((fp = fopen(profile.c_str(), "r")) == NULL)  //NOTICE: this is not a binary file
    {
#ifdef DEBUG
        fprintf(stderr, "openfile [%s] error [%s]\n", profile.c_str(), strerror(errno));
#endif
        return false;
    }
    fseek(fp, 0, SEEK_SET);

    while(!feof(fp) && fgets(buf_i, len, fp) != NULL)
    {
		//fprintf(stderr, "buffer: %s\n", buf_i);
        Util::l_trim(buf_o, buf_i);
        if(strlen(buf_o) <= 0)
            continue;
        buf = NULL;
        buf = buf_o;
		if(buf[0] == '#')
		{
			continue;
		}
		else if(buf[0] == '[') 
		{
			continue;
		} 
		if((c = (char*)strchr(buf, '=')) == NULL)
			continue;
		memset(keyname, 0, sizeof(keyname));
		sscanf(buf, "%[^=|^ |^\t]", keyname);
#ifdef DEBUG
				//fprintf(stderr, "keyname: %s\n", keyname);
#endif
		sscanf(++c, "%[^\n]", keyval);
		char *keyval_o = (char *)calloc(strlen(keyval) + 1, sizeof(char));
		if(keyval_o != NULL) 
		{
			Util::a_trim(keyval_o, keyval);
#ifdef DEBUG
			//fprintf(stderr, "keyval: %s\n", keyval_o);
#endif
			if(keyval_o && strlen(keyval_o) > 0)
			{
				//strcpy(keyval, keyval_o);
				global_config[string(keyname)] = string(keyval_o);
			}
			xfree(keyval_o);
		}
	}

    fclose(fp);
	//display all settings here
	/*cout<<"the current settings are as below: "<<endl;
	cout<<"key : value"<<endl;
	cout<<"------------------------------------------------------------"<<endl;
	for(map<string, string>::iterator it = global_config.begin(); it != global_config.end(); ++it)
	{
		cout<<it->first<<" : "<<it->second<<endl;
	}
	cout<<endl;*/

	return true;
	//return Util::config_setting() && Util::config_debug() && Util::config_advanced();
}

// bool
// Util::config_debug()
// {
//     const unsigned len1 = 100;
    // const unsigned len2 = 505;
	// char AppName[] = "setting";
    // char KeyName[] = "mode";
	// char appname[len1], keyname[len1];
    // char KeyVal[len1];
    // char *buf, *c;
    // char buf_i[len1], buf_o[len1];
    // FILE *fp = NULL;
    // int status = 0; // 1 AppName 2 KeyName
// 	return true;
// }

// bool
// Util::config_advanced()
// {
//     const unsigned len1 = 100;
    // const unsigned len2 = 505;
	// char AppName[] = "setting";
    // char KeyName[] = "mode";
	// char appname[len1], keyname[len1];
    // char KeyVal[len1];
    // char *buf, *c;
    // char buf_i[len1], buf_o[len1];
    // FILE *fp = NULL;
    // int status = 0; // 1 AppName 2 KeyName
// 	return true;
// }

bool Util::setGlobalConfig(INIParser& parser, string rootname, string keyname)
{
    string value = parser.GetValue(rootname, keyname);
    if(value.empty()==false)
        Util::global_config[keyname] = replace_all(value,"\"","");
    return true;
}

string Util::getConfigureValue(string keyname)
{
    map<string, string>::iterator iter = Util::global_config.find(keyname);
	if (iter != Util::global_config.end())
	{
		    return iter->second;
	}
	return "";
}

bool Util::configure_new()
{
    INIParser ini_parser;
    ini_parser.ReadINI("conf.ini");
    /*string value=ini_parser.GetValue("ghttp", "max_out_limit");
    Util::global_config["max_out_limit"] = value;*/
    Util::setGlobalConfig(ini_parser, "ghttp", "default_port");
    Util::setGlobalConfig(ini_parser, "ghttp", "thread_num");
    Util::setGlobalConfig(ini_parser, "ghttp", "max_database_num");
    Util::setGlobalConfig(ini_parser, "ghttp", "max_user_num");
    Util::setGlobalConfig(ini_parser, "ghttp", "max_output_size");
    Util::setGlobalConfig(ini_parser, "ghttp", "root_username");
    Util::setGlobalConfig(ini_parser, "ghttp", "root_password");
    Util::setGlobalConfig(ini_parser, "ghttp", "system_username");
    Util::setGlobalConfig(ini_parser, "ghttp", "system_path");
    Util::setGlobalConfig(ini_parser, "ghttp", "db_path");
    Util::setGlobalConfig(ini_parser, "ghttp", "backup_path");
    Util::setGlobalConfig(ini_parser, "ghttp", "querylog_mode");
    Util::setGlobalConfig(ini_parser, "ghttp", "querylog_path");
    Util::setGlobalConfig(ini_parser, "ghttp", "accesslog_path");
    Util::setGlobalConfig(ini_parser, "ghttp", "queryresult_log");
    Util::setGlobalConfig(ini_parser, "ghttp", "queryresult_path");
    Util::setGlobalConfig(ini_parser, "ghttp", "pfn_file_path");
    Util::setGlobalConfig(ini_parser, "ghttp", "pfn_lib_path");
    Util::setGlobalConfig(ini_parser, "ghttp", "ip");
    Util::setGlobalConfig(ini_parser, "ghttp", "ip_allow_path");
    Util::setGlobalConfig(ini_parser, "ghttp", "ip_deny_path");
    Util::setGlobalConfig(ini_parser, "system", "version");
    Util::setGlobalConfig(ini_parser, "system", "log_mode");
    Util::setGlobalConfig(ini_parser, "system", "licensetype");
    
    // init slog
    string log_mode = Util::getConfigureValue("log_mode");
    Slog::getInstance().init(log_mode.c_str());
    if(Slog::getInstance()._logger.getLogLevel() == log4cplus::DEBUG_LOG_LEVEL)
    {
        SLOG_DEBUG("the current settings are as below: ");
        SLOG_DEBUG("key : value");
        SLOG_DEBUG("----------------------------------");
        for (map<string, string>::iterator it = Util::global_config.begin(); it != Util::global_config.end(); ++it)
        {
            SLOG_DEBUG(it->first + " : " + it->second);
        }
        SLOG_DEBUG("----------------------------------");
    }
    return true;
}

bool
Util::config_setting()
{
    const unsigned len1 = 100;
    const unsigned len2 = 505;
	char AppName[] = "setting";
    char KeyName[] = "mode";
	char appname[len1], keyname[len1];
    char KeyVal[len1];
    char *buf, *c;
    char buf_i[len2], buf_o[len2];
    FILE *fp = NULL;
    int status = 0; // 1 AppName 2 KeyName

#ifdef DEBUG
	fprintf(stderr, "profile: %s\n", profile.c_str());
#endif
    if((fp = fopen(profile.c_str(), "r")) == NULL)  //NOTICE: this is not a binary file
    {
#ifdef DEBUG
        fprintf(stderr, "openfile [%s] error [%s]\n", profile.c_str(), strerror(errno));
#endif
        return false;
    }
    fseek(fp, 0, SEEK_SET);
	memset(appname, 0, sizeof(appname));
	sprintf(appname,"[%s]", AppName);
#ifdef DEBUG
	fprintf(stderr, "appname: %s\n", appname);
#endif

    while(!feof(fp) && fgets(buf_i, len2, fp) != NULL)
    {
		//fprintf(stderr, "buffer: %s\n", buf_i);
        Util::l_trim(buf_o, buf_i);
        if(strlen(buf_o) <= 0)
            continue;
        buf = NULL;
        buf = buf_o;
		if(buf[0] == '#')
		{
			continue;
		}
        if(status == 0)
        {
            if(strncmp(buf, appname, strlen(appname)) == 0)
            {
#ifdef DEBUG
				fprintf(stderr, "app found!\n");
#endif
                status = 1;
                continue;
            }
        }
        else if(status == 1)
        {
			if(buf[0] == '[') 
			{
				//NOTICE: nested module is not allowed
                break;
            } 
			else 
			{
                if((c = (char*)strchr(buf, '=')) == NULL)
                    continue;
                memset(keyname, 0, sizeof(keyname));
                sscanf(buf, "%[^=|^ |^\t]", keyname);
#ifdef DEBUG
				fprintf(stderr, "keyname: %s\n", keyname);
#endif
                if(strcmp(keyname, KeyName) == 0) 
				{
#ifdef DEBUG
					fprintf(stderr, "key found!\n");
#endif
                    sscanf(++c, "%[^\n]", KeyVal);
                    char *KeyVal_o = (char *)calloc(strlen(KeyVal) + 1, sizeof(char));
                    if(KeyVal_o != NULL) 
					{
                        Util::a_trim(KeyVal_o, KeyVal);
#ifdef DEBUG
						fprintf(stderr, "KeyVal: %s\n", KeyVal_o);
#endif
                        if(KeyVal_o && strlen(KeyVal_o) > 0)
                            strcpy(KeyVal, KeyVal_o);
                        xfree(KeyVal_o);
                    }
                    status = 2;
                    break;
                } 
            }
        }
    }
    fclose(fp);
    //if(found == 2)
        //return(0);
    //else
        //return(-1);
    //fprintf(stderr, "%s\n", KeyVal);
	if(strcmp(KeyVal, "distribute") == 0)
	{
#ifdef DEBUG
		fprintf(stderr, "the gStore will run in distributed mode!\n");
#endif
		//Util::gStore_mode = true;
	}

    return true;   //config success
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

string 
Util::getThreadID()
{
	//thread::id, neither int or long
	auto myid = this_thread::get_id();
	stringstream ss;
	ss << myid;
	return ss.str();
}

int
Util::memUsedPercentage()
{
    FILE* fp = fopen("/proc/meminfo", "r");
    if(fp == NULL)
        return -1;
    char str[20], tail[3];
    unsigned t, sum, used = 0;		//WARN:unsigned,memory cant be too large!
    fscanf(fp, "%s%u%s", str, &sum, tail);       //MemTotal, KB
    fscanf(fp, "%s%u%s", str, &used, tail);		//MemFree
    fscanf(fp, "%s%u%s", str, &t, tail);
    if(strcmp(str, "MemAvailable") == 0)
    {
        //QUERY:what is the relation between MemFree and MemAvailable?
        used = t;
        //scanf("%s%u%s", str, &t, tail);		//Buffers
        //used += t;
        //scanf("%s%u%s", str, &t, tail);		//Cached
        //used += t;
    }
    //else							//Buffers
    //{
    //	scanf("%s%u%s", str, &t, tail);		//Cached
    //	used += t;
    //}
    used = sum - used;
    fclose(fp);
    return (int)(used * 100.0 / sum);
}

int
Util::memoryLeft()
{
    FILE* fp = fopen("/proc/meminfo", "r");
    if(fp == NULL)
        return -1;

    char str[20], tail[3];
    unsigned num, avail = 0, free = 0, buffer = 0, cache = 0;		//WARN:unsigned,memory cant be too large!
    while (fscanf(fp, "%s%u%s", str, &num, tail) != EOF)
    {
        if(strcmp(str, "MemAvailable:") == 0)
        	avail = num;
        if(strcmp(str, "MemFree:") == 0)
        	free = num;
        if(strcmp(str, "Buffers:") == 0)
        	buffer = num;
        if(strcmp(str, "Cached:") == 0)
        	cache = num;
    }

    if (avail == 0)
    	avail = free + buffer + cache;

    fclose(fp);
    return avail / Util::MB;
}

bool
Util::is_literal_ele(TYPE_ENTITY_LITERAL_ID _id)
{
    return _id >= Util::LITERAL_FIRST_ID;
}

bool 
Util::is_entity_ele(TYPE_ENTITY_LITERAL_ID id) 
{
	return id < Util::LITERAL_FIRST_ID;
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
	qsort(_id_list, _list_len, sizeof(unsigned), Util::cmp_unsigned);
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


bool
Util::dir_exist(const string _dir)
{
	DIR* dirptr = opendir(_dir.c_str());
	if(dirptr != NULL)
	{
		closedir(dirptr);
		return true;
	}

	return false;
}

bool Util::file_exist(const string _file)
{
	struct stat buffer;
	return (stat(_file.c_str(), &buffer) == 0);
}

bool
Util::create_dir(const string _dir)
{
    if(! Util::dir_exist(_dir))
    {
        
        mkdir(_dir.c_str(), 0755);
        return true;
    }

    return false;
}

bool
Util::create_dirs(const string _dirs)
{
    if (! Util::dir_exist(_dirs))
    {
        char tmpDirPath[255] = {0};
        size_t len = _dirs.length();
        for (size_t i = 0; i < len; i++)
        {
            tmpDirPath[i] = _dirs[i];
            if (tmpDirPath[i] == '/')
            {
                if (access(tmpDirPath, 0) != 0)
                {
                    mkdir(tmpDirPath, 755);
                }
            }
        }
        if (access(tmpDirPath, 0) != 0)
        {
            mkdir(tmpDirPath, 755);
        }
        return true;
    }
    return false;
}

bool
Util::create_file(const string _file) {
	if (creat(_file.c_str(), 0755) > 0) {
		return true;
	}
	return false;
}

long
Util::get_cur_time()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

string 
Util::get_date_time()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M:%S",localtime(&timep) );
	return tmp;
}

string
Util::get_date_day()
{
	time_t timep;
	time(&timep);
	char tmp[64];
	strftime(tmp, sizeof(tmp), "%Y%m%d",localtime(&timep) );
	return tmp;
}

string
Util::get_timestamp()
{
    string timestamp;
    time_t timep;
    time(&timep);
    char year[5];
    char tmp[64];
    strftime(year, sizeof(year), "%Y", localtime(&timep));
    timestamp += year[2]; timestamp += year[3];
    strftime(tmp, sizeof(tmp), "%m%d%H%M%S",localtime(&timep) );
    timestamp += tmp;
    return timestamp;
}

time_t 
Util::time_to_stamp(string time){
    struct tm* tm = (struct tm*)malloc(sizeof(struct tm));
    strptime(time.c_str() , "%Y-%m-%d %H:%M:%S", tm);
    time_t stamp = mktime(tm);
    return stamp;
}

string
Util::get_backup_time(const string path, const string db_name)
{
    string _db_name = db_name + ".db";
    string::size_type position;
    position = path.find(_db_name);

    if(position == path.npos)
        return "";
    string db_folder = path.substr(position, path.length());
    string timestamp = db_folder.substr(_db_name.length() + 1, db_folder.length());
    string year, month, day, hour, minute, second;
    year = "20" + timestamp.substr(0, 2);
    month = timestamp.substr(2, 2);
    day = timestamp.substr(4, 2);
    hour = timestamp.substr(6, 2);
    minute = timestamp.substr(8, 2);
    second = timestamp.substr(10, 2);
    string time = year + '-' + month + '-' + day + ' ' + hour + ":" + minute + ":" + second;
    return time;
}

string
Util::get_folder_name(const string path, const string db_name){
    string _db_name = db_name + ".db";
    string::size_type position;
    position = path.find(_db_name);

    string db_folder = path.substr(position, path.length());
    return db_folder;
}

bool
Util::save_to_file(const char* _dir, const string _content)
{
    ofstream fout(_dir);

    if (fout.is_open())
    {
        fout << _content;
        fout.close();
    }

    return false;
}

int
Util::compare(const char* _str1, unsigned long _len1, const char* _str2, unsigned long _len2)
{
	int ifswap = 1;		//1 indicate: not swapped
	if (_len1 > _len2)
	{
		const char* str = _str1;
		_str1 = _str2;
		_str2 = str;
		unsigned long len = _len1;
		_len1 = _len2;
		_len2 = len;
		ifswap = -1;
	}
	unsigned long i;
	//DEBUG: if char can be negative, which cause problem when comparing(128+)
	//
	//NOTICE:little-endian-storage, when string buffer poniter is changed to
	//unsigned long long*, the first char is the lowest byte!
	/*
	unsigned long long *p1 = (unsigned long long*)_str1, *p2 = (unsigned long long*)_str2;
	unsigned limit = _len1/8;
	for(i = 0; i < limit; ++i, ++p1, ++p2)
	{
	if((*p1 ^ *p2) == 0)	continue;
	else
	{
	if(*p1 < *p2)	return -1 * ifswap;
	else			return 1 * ifswap;
	}
	}
	for(i = 8 * limit; i < _len1; ++i)
	{
	if(_str1[i] < _str2[i])	return -1 * ifswap;
	else if(_str1[i] > _str2[i])	return 1 * ifswap;
	else continue;
	}
	if(i == _len2)	return 0;
	else	return -1 * ifswap;
	*/
	for (i = 0; i < _len1; ++i)
	{   //ASCII: 0~127 but c: 0~255(-1) all transfered to unsigned char when comparing
		if ((unsigned char)_str1[i] < (unsigned char)_str2[i])
			return -1 * ifswap;
		else if ((unsigned char)_str1[i] > (unsigned char)_str2[i])
			return 1 * ifswap;
		else;
	}
	if (i == _len2)
		return 0;
	else
		return -1 * ifswap;
}

string
Util::string_replace(string rec, const string src, const string des)
{
    string::size_type pos = 0;
    string::size_type a = src.size();
    string::size_type b = des.size();
    while ((pos = rec.find(src, pos)) != string::npos)
    {
        rec.replace(pos, a, des);
        pos += b;
    }
    return rec;
}

bool
Util::is_number(string s)
{
    string::size_type pos = 0;
    for(; pos < s.size(); pos++){
        if(!isdigit(s[pos])) return false;
    }
    return true;
}

int
Util::string2int(string s)
{
    return atoi(s.c_str());
}

string
Util::int2string(long n)
{
    string s;
    stringstream ss;
    ss<<n;
    ss>>s;
    return s;
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
    string file = Util::tmp_path;
    file += "ans.txt";
    cmd += " > ";
    cmd += file;
    //cerr << cmd << endl;
    int ret = system(cmd.c_str());
    cmd = "rm -rf " + file;
    if(ret < 0)
    {
        fprintf(stderr, "system call failed!\n");
        system(cmd.c_str());
        return NULL;
    }

    ifstream fin(file.c_str());
    if(!fin)
    {
        cerr << "getSystemOutput: Fail to open : " << file << endl;
        return NULL;
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
    system(cmd.c_str());
	//cerr<<"ans: "<<ans<<endl;
    return ans;
}

// Get the exact file path from given string: ~, ., symbol links
string
Util::getExactPath(const char *str)
{
    string cmd = "realpath ";
    cmd += string(str);

    return getSystemOutput(cmd);
}

void
Util::logging(string _str)
{
    _str += "\n";
#ifdef DEBUG_DATABASE
    fputs(_str.c_str(), Util::debug_database);
    fflush(Util::debug_database);
#endif

#ifdef DEBUG_VSTREE
    fputs(_str.c_str(), Util::debug_vstree);
    fflush(Util::debug_vstree);
#endif
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
    register unsigned int   h;
    register unsigned char *p;
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

	int port = Util::string2int(str);
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

string 
Util::getTimeName()
{
	//NOTICE: this is another method to get the concrete time
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	string tempTime = asctime(timeinfo);
	for(unsigned i = 0; i < tempTime.length(); i++)
	{
		if(tempTime[i] == ' ')
			tempTime[i] = '_';
	}
	string myTime = tempTime.substr(0, tempTime.length()-1);
	return myTime;
}

string
Util::getTimeString() {
	static const int max = 20; // max length of time string
	char time_str[max];
	time_t timep;
	time(&timep);
	strftime(time_str, max, "%Y%m%d %H:%M:%S", localtime(&timep));
	return string(time_str);
}

string
Util::getTimeString2() {
	static const int max = 20; // max length of time string
	char time_str[max];
	time_t timep;
	time(&timep);
	strftime(time_str, max, "%Y%m%d%H%M%S", localtime(&timep));
	return string(time_str);
}
int
Util::getRandNum()
{
     unsigned seed;  // Random generator seed
    // Use the time function to get a "seed” value for srand
    seed = time(0);
    srand(seed);
    int result=rand();
    return result;
}

bool Util::checkPort(int port, std::string p_name)
{
    stringstream ss;
	ss << port;
	string str_port = ss.str();
	string out_file = ".tmp/port_check";
	string cmd = "netstat -ntlp |grep " + str_port + " > " + out_file;
	system(cmd.c_str());
	struct stat buffer;
	bool file_exist = (stat(out_file.c_str(), &buffer) == 0);
	if (!file_exist)
	{
		return true;
	}
	ifstream inputFile(out_file.c_str());
	string line;
	size_t idx;
	str_port = ":" + str_port;
	bool result = true;
	while (getline(inputFile, line, '\n'))
	{
		idx = line.find(str_port);
		if ((idx != string::npos) && line[idx + str_port.size()] == ' ')
		{
			if (p_name.empty())
				result = false;
			else if (line.find(p_name) == string::npos) // main process restart sub process
				result = false;
			break;
		}
	}
	line = "rm -f " + out_file;
	system(line.c_str());
	return result;
}

//is ostream.write() ok to update to disk at once? all add ofstream.flush()?
//http://bookug.cc/rwbuffer
//BETTER: add a sync function in Util to support FILE*, fd, and fstream
void 
Util::Csync(FILE* _fp)
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

string
Util::node2string(const char* _raw_str) {
	string _output;
	unsigned _first_quote = 0;
	unsigned _last_quote = 0;
	bool _has_quote = false;
	for (unsigned i = 0; _raw_str[i] != '\0'; i++) {
		if (_raw_str[i] == '\"') {
			if (!_has_quote) {
				_first_quote = i;
				_last_quote = i;
				_has_quote = true;
			}
			else {
				_last_quote = i;
			}
		}
	}
	if (_first_quote==_last_quote) {
		_output += _raw_str;
		return _output;
	}
	for (unsigned i = 0; i <= _first_quote; i++) {
		_output += _raw_str[i];
	}
	for (unsigned i = _first_quote + 1; i < _last_quote; i++) {
		switch (_raw_str[i]) {
		case '\n':
			_output += "\\n";
			break;
		case '\r':
			_output += "\\r";
			break;
		case '\t':
			_output += "\\t";
			break;
		case '\"':
			_output += "\\\"";
			break;
		case '\\':
			_output += "\\\\";
			break;
		default:
			_output += _raw_str[i];
		}
	}
	for (unsigned i = _last_quote; _raw_str[i] != 0; i++) {
		_output += _raw_str[i];
	}
	return _output;
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

void
Util::empty_file(const char* _fname)
{
	FILE * fp;
	//NOTICE: if exist, then overwrite and create a empty file
	fp = fopen(_fname, "w"); 
	if(fp == NULL)
	{
		printf("do empty file %s failed\n", _fname);
	}
	else 
	{
		fclose(fp);
	}
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
	ifstream in;
	in.open(Util::profile.c_str(), ios::in);
	if (!in) {
		return Util::gserver_backup_time;
	}
	int buf_size = 512;
	char lbuf[buf_size];
	while (!in.eof()) {
		in.getline(lbuf, buf_size);
		regex_t reg;
		char pattern[] = "^\\s*BackupTime\\s*=\\s*((0|1)[0-9]|2[0-3])[0-5][0-9]\\s*(\\s#.*)?$";
		regcomp(&reg, pattern, REG_EXTENDED | REG_NOSUB);
		regmatch_t pm[1];
		int status = regexec(&reg, lbuf, 1, pm, 0);
		regfree(&reg);
		if (status == REG_NOMATCH) {
			continue;
		}
		else if (status != 0) {
			in.close();
			return Util::gserver_backup_time;
		}
		for (int i = 11; i < buf_size && lbuf[i]; i++) {
			if (lbuf[i] >= '0' && lbuf[i] <= '9') {
				in.close();
				return 36000 * (lbuf[i] - '0') + 3600 * (lbuf[i + 1] - '0')
					+ 600 * (lbuf[i + 2] - '0') + 60 * (lbuf[i + 3] - '0');
			}
		}
	}
	in.close();
	return Util::gserver_backup_time;
}

std::string
Util::replace_all(std::string _content,const std::string oldtext,const std::string newtext)
{
     while(true)   {
            string::size_type  pos(0);
            if(  (pos=_content.find(oldtext))!=string::npos  )
                _content.replace(pos,oldtext.length(),newtext);
            else   break;
        }
    return  _content;
}

void
Util::split(string str, string pattern, vector<string> &res){
    string::size_type pos = 0;
    str += pattern;
    for(int i = 0; i < static_cast<int>(str.size()); i++)
    {
        pos = str.find(pattern, i);
        if(pos < str.size()){
            res.push_back(str.substr(i, pos-i));
            i = pos + pattern.size() - 1;
        }
    }
}

void 
Util::init_backuplog()
{
    pthread_rwlock_wrlock(&backuplog_lock);
    FILE* fp = fopen(BACKUP_LOG_PATH, "w");
    Document document;
    document.SetObject();
    Document::AllocatorType &allocator = document.GetAllocator();

    document.AddMember("db_name", "system", allocator);
    document.AddMember("backup_timer", DEFALUT_BACKUP_INTERVAL, allocator);
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    document.Accept(writer);
    string rec = buffer.GetString();
    rec = Util::string_replace(rec, "\n", "");
    rec = Util::string_replace(rec, "    ", "");
    rec.push_back('\n');
    fputs(rec.c_str(), fp);

    fclose(fp);
    pthread_rwlock_unlock(&backuplog_lock);
}

int 
Util::add_backuplog(string db_name)
{
    if(db_name == "system"){
        SLOG_ERROR("system can not be duplicated");
        return -1;
    }
    if(has_record_backuplog(db_name)) return 1;
    pthread_rwlock_wrlock(&backuplog_lock);
    FILE* fp = fopen(BACKUP_LOG_PATH, "a");
    Document document;
    document.SetObject();
    Document::AllocatorType &allocator = document.GetAllocator();

    string time = Util::get_date_time();

    document.AddMember("db_name", StringRef(db_name.c_str()), allocator);
    document.AddMember("backup_interval", DEFALUT_BACKUP_INTERVAL, allocator);
    document.AddMember("last_backup_time", StringRef(time.c_str()), allocator);
    document.AddMember("is_backup", "false", allocator);
    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    document.Accept(writer);
    string rec = buffer.GetString();
    rec = Util::string_replace(rec, "\n", "");
    rec = Util::string_replace(rec, "    ", "");
    rec.push_back('\n');
    fputs(rec.c_str(), fp);

    fclose(fp);
    pthread_rwlock_unlock(&backuplog_lock);
    return 0;
}

int 
Util::delete_backuplog(string db_name)
{
    if(db_name == "system"){
        SLOG_ERROR("system can not be deleted!");
        return -1;
    }
    pthread_rwlock_wrlock(&backuplog_lock);
    FILE* fp = fopen(BACKUP_LOG_PATH, "r");
    FILE* fp1 = fopen(BACKUP_LOG_TMEP_PATH, "w");
    char readBuffer[0xffff];
    int ret = 1;
    while(fgets(readBuffer, 1024, fp)) {
        string rec = readBuffer;
        StringStream is(readBuffer);
        Document d;
        d.ParseStream(is);
        if(d["db_name"].GetString() == db_name){
            ret = 0;
            continue;
        }
        fputs(readBuffer, fp1);
    }
    fclose(fp);
    fclose(fp1);
    string cmd = "rm ";
    cmd += BACKUP_LOG_PATH;
    system(cmd.c_str());
    cmd = "mv ";
    cmd += BACKUP_LOG_TMEP_PATH;
    cmd += ' ';
    cmd += BACKUP_LOG_PATH;
    system(cmd.c_str());
    pthread_rwlock_unlock(&backuplog_lock);
    return ret;
}

int 
Util::update_backuplog(string db_name, string parameter, string value)
{
    if(parameter == "db_name"){
       SLOG_ERROR("parameter can not be db_name!");
        return -1;
    }
    pthread_rwlock_wrlock(&backuplog_lock);
    FILE* fp = fopen(BACKUP_LOG_PATH, "r");
    FILE* fp1 = fopen(BACKUP_LOG_TMEP_PATH, "w");
    char readBuffer[0xffff];
    int ret = 0;
    while(fgets(readBuffer, 1024, fp)){
        string rec = readBuffer;
        StringStream is(readBuffer);
        Document d;
        d.ParseStream(is);
        if(d["db_name"].GetString() != db_name){
            fputs(readBuffer, fp1);
            continue;
        }
        if(d.HasMember(parameter.c_str())){
            Value& S = d[parameter.c_str()];
            S.SetString(value.c_str(), value.length());
            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            d.Accept(writer);
            string line = buffer.GetString();
            line.push_back('\n');
            fputs(line.c_str(), fp1);
        }
        else{
            fputs(readBuffer, fp1);
            SLOG_ERROR("wrong parameter!");
            ret = 1;
        }
    }
    fclose(fp);
    fclose(fp1);
    string cmd = "rm ";
    cmd += BACKUP_LOG_PATH;
    system(cmd.c_str());
    cmd = "mv ";
    cmd += BACKUP_LOG_TMEP_PATH;
    cmd += ' ';
    cmd += BACKUP_LOG_PATH;
    system(cmd.c_str());
    pthread_rwlock_unlock(&backuplog_lock);
    return ret;
}

string 
Util::query_backuplog(string db_name, string parameter)
{
    pthread_rwlock_rdlock(&backuplog_lock);
    FILE* fp = fopen(BACKUP_LOG_PATH, "r");
    char readBuffer[0xffff];
    while(fgets(readBuffer, 1024, fp)){
        string rec = readBuffer;
        StringStream is(readBuffer);
        Document d;
        d.ParseStream(is);
        if(d["db_name"].GetString() != db_name) continue;
        if(d.HasMember(parameter.c_str())){
            fclose(fp);
            pthread_rwlock_unlock(&backuplog_lock);
            return d[parameter.c_str()].GetString();

        }
        else{
            SLOG_ERROR("wrong parameter!");
        }
    }
    fclose(fp);
    pthread_rwlock_unlock(&backuplog_lock);
    return " ";
}

void 
Util::search_backuplog(vector<string> &res, string parameter, string value)
{
    pthread_rwlock_rdlock(&backuplog_lock);
    FILE* fp = fopen(BACKUP_LOG_PATH, "r");
    char readBuffer[0xffff];
    while(fgets(readBuffer, 1024, fp)) {
        string rec = readBuffer;
        StringStream is(readBuffer);
        Document d;
        d.ParseStream(is);
        if(d.HasMember(parameter.c_str()) && d[parameter.c_str()].GetString() == value)
            res.push_back(d["db_name"].GetString());
    }
    fclose(fp);
    pthread_rwlock_unlock(&backuplog_lock);
}

bool 
Util::has_record_backuplog(string db_name)
{
    pthread_rwlock_rdlock(&backuplog_lock);
    if(db_name == "system") return true;
    FILE* fp = fopen(BACKUP_LOG_PATH, "r");
    char readBuffer[0xffff];
    while(fgets(readBuffer, 1024, fp)) {
        string rec = readBuffer;
        StringStream is(readBuffer);
        Document d;
        d.ParseStream(is);
        if (d["db_name"].GetString() == db_name){
            SLOG_DEBUG(rec);
            pthread_rwlock_unlock(&backuplog_lock);
            return true;
        }
    }
    fclose(fp);
    pthread_rwlock_unlock(&backuplog_lock);

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

string 
Util::stamp2time(int timestamp)
{
    time_t tick = (time_t)timestamp;
    struct tm tm;
    char s[100];
    tm = *localtime(&tick);
    strftime(s, sizeof(s), "%Y-%m-%d %H:%M:%S", &tm);

    return s;
}


//get all specific file type files in a directory
vector<string> 
Util::GetFiles(const char *src_dir, const char *ext)
{
    vector<string> result;
    string directory(src_dir);
    string m_ext(ext);

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
                if (strcmp(d_name.c_str () + d_name.length () - m_ext.length(), m_ext.c_str ()) == 0)
                {
                    result.push_back(string(d_ent->d_name));
                }
            }
        }
    }
 
    // sort the returned files
    std::sort(result.begin(), result.end());
 
    closedir(dir);
    return result;
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
    return _md5.md5();
}

bool Util::iscontain(const string& _parent,const string& _child)
{
 string::size_type idx = _parent.find(_child);
 if(idx != string::npos )
 {
   return true;
 }
 else
 {
   return false;
 }
}

// void Util::formatPrint(std::string content, std::string type)
// {
//     string time = Util::get_date_time();
//     cout << "[" << type << "][" << time << "] " << content << endl;
// }

std::string Util::urlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    unsigned char x;
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) ||
            (str[i] == '-') ||
            (str[i] == '_') ||
            (str[i] == '.') ||
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            x = (unsigned char)str[i] >> 4;
            strTemp += x > 9 ? x + 55 : x + 48;
            x = (unsigned char)str[i] % 16;
            strTemp += x > 9 ? x + 55 : x + 48;
        }
    }
    return strTemp;
}

std::string Util::urlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    unsigned char x;
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+')
            strTemp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            x = (unsigned char)str[++i];
            unsigned char high;
            if (x >= 'A' && x <= 'Z')
                high = x - 'A' + 10;
            else if (x >= 'a' && x <= 'z')
                high = x - 'a' + 10;
            else if (x >= '0' && x <= '9')
                high = x - '0';
            else
                assert(0);

            x = (unsigned char)str[++i];
            unsigned char low;
            if (x >= 'A' && x <= 'Z')
                low = x - 'A' + 10;
            else if (x >= 'a' && x <= 'z')
                low = x - 'a' + 10;
            else if (x >= '0' && x <= '9')
                low = x - '0';

            strTemp += high * 16 + low;
        }
        else
            strTemp += str[i];
    }
    return strTemp;
}

std::string Util::get_cur_path()
{
    char *buffer;
    if((buffer = getcwd(NULL, 0)) == NULL) 
    {
        SLOG_ERROR("get cur path error");
        return "";
    }
    else
    {
        string cur_path = string(buffer);
        SLOG_DEBUG("cur_path: " + cur_path);
        return cur_path;
    }
}