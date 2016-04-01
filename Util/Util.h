/*=============================================================================
# Filename: Util.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-16 10:43
# Description: 
1. firstly written by liyouhuan, modified by zengli
2. common macros, functions, classes, etc
=============================================================================*/

#ifndef _UTIL_UTIL_H
#define _UTIL_UTIL_H

/* basic macros and types are defined here, including common headers */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include <locale.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

//NOTICE:below are restricted to C++, C files should not include(maybe nested) this header!
#include <bitset>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <map>
#include <set>
#include <stack>
#include <vector>
#include <list>
#include <iterator>
#include <algorithm>
#include <functional>
#include <utility>

//NOTICE:below are libraries need to link
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>

#define STREAM_ON 1			
#define READLINE_ON	1

//indicate that in debug mode
//#define DEBUG_STREAM
//#define DEBUG_PRECISE 1		all information
//#define DEBUG_KVSTORE 1		//in KVstore
//#define DEBUG_VSTREE 1	//in Database 
//#define DEBUG_DATABASE 1	//in Database
//#define DEBUG_JOIN      

#ifdef DEBUG_PRECISE
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifdef DEBUG_KVSTORE
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifdef DEBUG_VSTREE
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifdef DEBUG_DATABASE
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifdef DEBUG_JOIN
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifndef DEBUG
//#define DEBUG
#endif

//NOTICE:include Util.h and below in each main function
//(the beginning position)
//#ifdef DEBUG
//	Util util;
//#endif

typedef unsigned(*HashFunction)(const char*, unsigned);
//NOTICE:hash functions for int are not so many, so we represent int by a 4-byte stringinstead
//(not totally change int to string, which is costly)
//http://www.cppblog.com/aurain/archive/2010/07/06/119463.html
//http://blog.csdn.net/mycomputerxiaomei/article/details/7641221
//http://kb.cnblogs.com/page/189480/

/******** all static&universal constants and fucntions ********/
class Util
{
public:
	static const unsigned MB = 1048576;
	static const unsigned GB = 1073741824;
	static const int TRIPLE_NUM_MAX = 1000*1000*1000;
	//In order to differentiate the sub-part and literal-part of object
	//let subid begin with 0, while literalid begins with LITERAL_FIRST_ID 
	//used in Database and Join
	static const int LITERAL_FIRST_ID = 1000*1000*1000;
	//initial transfer buffer size in Tree/ and Stream/
	static const unsigned TRANSFER_SIZE = 1 << 20;	//1M

	static std::string tmp_path;
	// this are for debugging
	//to build logs-system, each class: print() in time 
	static std::string debug_path;
	static FILE* debug_kvstore;				
	static FILE* debug_database;

	static int memUsedPercentage();
	static int memoryLeft();
	static int compare(const char* _str1, unsigned _len1, const char* _str2, unsigned _len2); //QUERY(how to use default args)
	static int string2int(std::string s);
	static std::string int2string(long n);
	//string2str: s.c_str()
	//str2string: string(str)
	static std::string showtime();
	static int cmp_int(const void* _i1, const void* _i2);
	static void sort(int*& _id_list, int _list_len);
	static int bsearch_int_uporder(int _key,int* _array,int _array_num);
	static bool bsearch_preid_uporder(int _preid, int* _pair_idlist, int _list_len);
	static int bsearch_vec_uporder(int _key, const std::vector<int>& _vec);
	static std::string result_id_str(std::vector<int*>& _v, int _var_num);
	static bool dir_exist(const std::string _dir);
	static bool create_dir(const std:: string _dir);
	static long get_cur_time();
	static bool save_to_file(const char*, const std::string _content);

	static std::string getQueryFromFile(const char* _file_path); 
	static std::string getSystemOutput(std::string cmd);
	static std::string getExactPath(const char* path);
	static void logging(std::string _str);

	// Below are some useful hash functions for string
	// NOTICE:the string is general type and maybe very large, so length
	// should as parameter(caller may use Bstr to avoid compute the length each time)
	static unsigned simpleHash(const char *_str, unsigned _len);
	static unsigned APHash(const char *_str, unsigned _len);
	static unsigned BKDRHash(const char *_str, unsigned _len);
	static unsigned DJBHash(const char *_str, unsigned _len);
	static unsigned ELFHash(const char *_str, unsigned _len);
	static unsigned DEKHash(const char* _str, unsigned _len);
	static unsigned BPHash(const char* _str, unsigned _len);
	static unsigned FNVHash(const char* _str, unsigned _len);
	static unsigned HFLPHash(const char* _str, unsigned _len);
	static unsigned HFHash(const char* _str, unsigned _len);
	static unsigned JSHash(const char *_str, unsigned _len);
	static unsigned PJWHash(const char *_str, unsigned _len);
	static unsigned RSHash(const char *_str, unsigned _len);
	static unsigned SDBMHash(const char *_str, unsigned _len);
	static unsigned StrHash(const char* _str, unsigned _len);
	static unsigned TianlHash(const char* _str, unsigned _len);

	static double logarithm(double _a, double _b);

	Util();
	~Util();
};

#endif //_UTIL_UTIL_H

