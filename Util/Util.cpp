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

//TODO:add config and use absolute path here
//NOTICE:relative to the position of you when executing
//cd bin;./gconsole and bin/gconsole are different
string Util::tmp_path = ".tmp/";
string Util::debug_path = ".debug/";
//QUERY: assign all in Util()?
//BETTER:assigned in KVstore, not one tree?
FILE* Util::debug_kvstore = NULL;            //assigned by KVstore/Storage
FILE* Util::debug_database = NULL;			 //assigned by Database

Util::Util()
{
#ifdef DEBUG_KVSTORE
	if(this->debug_kvstore == NULL)
	{
		string s = this->debug_path + "kv.log";
		this->debug_kvstore = fopen(s.c_str(), "w+");
		if(this->debug_kvstore == NULL)
		{
			printf("open error: kv.log\n");
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
			printf("open error: db.log\n");
			this->debug_database = stderr;
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
	unsigned t, sum, unuse = 0;		//WARN:unsigned,memory cant be too large!
	fscanf(fp, "%s%u%s", str, &sum, tail);       //MemTotal, KB
	fscanf(fp, "%s%u%s", str, &unuse, tail);		//MemFree
	fscanf(fp, "%s%u%s", str, &t, tail);
	if(strcmp(str, "MemAvailable") == 0)
	{
		unuse = t;
	}
	fclose(fp);
	return unuse / Util::MB;
}

int 
Util::cmp_int(const void* _i1, const void* _i2)
{
	return *(int*)_i1  -  *(int*)_i2;
}

void 
Util::sort(int*& _id_list, int _list_len)
{
	qsort(_id_list, _list_len, sizeof(int), Util::cmp_int);
}

int 
Util::bsearch_int_uporder(int _key,int* _array,int _array_num)
{
	if (_array_num == 0) 
	{
		return -1;
	}
	if (_array == NULL) 
	{
		return -1;
	}
	int _first = _array[0];
	int _last = _array[_array_num - 1];

	if (_last == _key) 
	{
		return _array_num - 1;
	}

	if (_last < _key || _first > _key) 
	{
		return -1;
	}

	int low = 0;
	int high = _array_num - 1;

	int mid;
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
	return -1;
}

bool 
Util::bsearch_preid_uporder(int _preid, int* _pair_idlist, int _list_len)
{
	if(_list_len == 0)
	{
		return false;
	}
	int pair_num = _list_len / 2;
	int _first = _pair_idlist[2*0 + 0];
	int _last = _pair_idlist[2*(pair_num-1) + 0];

	if(_preid == _last)
	{
		return true;
	}

	bool not_find = (_last < _preid || _first > _preid);
	if(not_find)
	{
		return false;
	}

	int low = 0;
	int high = pair_num - 1;
	int mid;

	while (low <= high) 
	{
		mid = (high - low) / 2 + low;
		if (_pair_idlist[2*mid + 0] == _preid) 
		{
			return true;
		}

		if (_pair_idlist[2*mid + 0] > _preid) 
		{
			high = mid - 1;
		} else 
		{
			low = mid + 1;
		}
	}

	return false;
}

int 
Util::bsearch_vec_uporder(int _key, const vector<int>& _vec)
{
	int tmp_size = _vec.size();
	if (tmp_size == 0) 
	{
		return -1;
	}

	int _first = _vec[0];
	int _last = _vec[tmp_size - 1];

	if (_key == _last) 
	{
		return tmp_size - 1;
	}

	bool not_find = (_last < _key || _first > _key);
	if (not_find) 
	{
		return -1;
	}

	int low = 0;
	int high = tmp_size - 1;
	int mid;

	while (low <= high) 
	{
		mid = (high - low) / 2 + low;
		if (_vec[mid] == _key) 
		{
			return mid;
		}

		if (_vec[mid] > _key) 
		{
			high = mid - 1;
		} 
		else 
		{
			low = mid + 1;
		}
	}
	return -1;
}

string 
Util::result_id_str(vector<int*>& _v, int _var_num)
{
	stringstream _ss;

	for(unsigned i = 0; i < _v.size(); i ++)
	{
		int* _p_int = _v[i];
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
	return (opendir(_dir.c_str()) != NULL);
}

bool 
Util::create_dir(const  string _dir)
{
	if(! Util::dir_exist(_dir))
	{
		mkdir(_dir.c_str(), 0755);
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
Util::compare(const char* _str1, unsigned _len1, const char* _str2, unsigned _len2)
{
	int ifswap = 1;		//1 indicate: not swapped
	if(_len1 > _len2)
	{
		const char* str = _str1;
		_str1 = _str2;
		_str2 = str;
		unsigned len = _len1;
		_len1 = _len2;
		_len2 = len;
		ifswap = -1;
	}
	unsigned i;
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
	for(i = 0; i < _len1; ++i)
	{	//ASCII: 0~127 but c: 0~255(-1) all transfered to unsigned char when comparing
		if((unsigned char)_str1[i] < (unsigned char)_str2[i])
			return -1 * ifswap;
		else if((unsigned char)_str1[i] > (unsigned char)_str2[i])
			return 1 * ifswap;
		else;
	}
	if(i == _len2)
		return 0;
	else
		return -1 * ifswap;
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
    printf("file to open: %s\n", _file_path);
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
Util::getSystemOutput(string cmd)
{
	string ans = "";
	string file = Util::tmp_path;
	file += "ans.txt";
	cmd += " > ";
	cmd += file;
	cerr << cmd << endl;
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
	getline(fin, ans);
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
    fputs(_str.c_str(), Util::debug_database);
    fflush(Util::debug_database);
#endif
}

unsigned
Util::BKDRHash(const char *_str, unsigned _len)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int key = 0;

	for(unsigned i = 0; i < _len; ++i)
    {
        key = key * seed + _str[i];
    }

    return (key & 0x7FFFFFFF);
}

unsigned 
Util::simpleHash(const char *_str, unsigned _len)
{
    unsigned int key;
    unsigned char *p;

    for(key = 0, p = (unsigned char *)_str; *p; p++)
        key = 31 * key + *p;

    return (key & 0x7FFFFFFF);
}

unsigned 
Util::RSHash(const char *_str, unsigned _len)
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
Util::JSHash(const char *_str, unsigned _len)
{
    unsigned int key = 1315423911;

    while (*_str)
    {
        key ^= ((key << 5) + (*_str++) + (key >> 2));
    }

    return (key & 0x7FFFFFFF);
}

unsigned 
Util::PJWHash(const char *_str, unsigned _len)
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
Util::ELFHash(const char *_str, unsigned _len)
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
Util::SDBMHash(const char *_str, unsigned _len)
{
    unsigned int key = 0;

    while (*_str)
    {
        key = (*_str++) + (key << 6) + (key << 16) - key;
    }

    return (key & 0x7FFFFFFF);
}

unsigned 
Util::DJBHash(const char *_str, unsigned _len)
{
    unsigned int key = 5381;
    while (*_str) {
        key += (key << 5) + (*_str++);
    }
    return (key & 0x7FFFFFFF);
}

unsigned 
Util::APHash(const char *_str, unsigned _len)
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
Util::DEKHash(const char* _str, unsigned _len)
{
    unsigned int hash = strlen(_str);
    unsigned int i    = 0;

    for(i = 0; _str[i] != '\0'; _str++, i++) {
        hash = ((hash << 5) ^ (hash >> 27)) ^ (*_str);
    }
    return hash;
}

unsigned 
Util::BPHash(const char* _str, unsigned _len)
{
    unsigned int hash = 0;
    unsigned int i    = 0;
    for(i = 0; _str[i] != '\0'; _str++, i++) {
        hash = hash << 7 ^ (*_str);
    }

    return hash;
}

unsigned 
Util::FNVHash(const char* _str, unsigned _len)
{
    const unsigned int fnv_prime = 0x811C9DC5;
    unsigned int hash      = 0;
    unsigned int i         = 0;

    for(i = 0; _str[i] != '\0'; _str++, i++) {
        hash *= fnv_prime;
        hash ^= (*_str);
    }

    return hash;
}

unsigned 
Util::HFLPHash(const char* _str, unsigned _len)
{
    unsigned int n=0;
    char* b=(char*)&n;
	unsigned int len = strlen(_str);
    for(unsigned i=0; i < len; ++i) 
	{
        b[i%4]^=_str[i];
    }
    return n%len;
}

unsigned 
Util::HFHash(const char* _str, unsigned _len)
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
Util::StrHash(const char* _str, unsigned _len)
{
    register unsigned int   h;
    register unsigned char *p;
    for(h=0,p=(unsigned char *)_str; *p; p++) {
        h=31*h+*p;
    }

    return h;

}

unsigned 
Util::TianlHash(const char* _str, unsigned _len)
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

