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
		used += t;
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

