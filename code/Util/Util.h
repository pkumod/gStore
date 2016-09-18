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
#include <ctype.h>
#include <time.h>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <set>
#include <stack>
#include <vector>
#include <algorithm>
#include <functional>

#define STREAM_ON 1			
#define READLINE_ON	1
//indicate that in debug mode
//#define DEBUG 1				//set this if any debug, except DEBUG_PRECISE
//#define DEBUG_PRECISE 1		//all information
//#define DEBUG_KVSTORE 1		//in KVstore
//#define DEBUG_DATABASE 1	//in Database
//#define DEBUG_VSTREE 1	//in Database 

//NOTICE:include Util.h and below in each main function
//(the beginning position)
//#ifdef DEBUG
//	Util util;
//#endif

/******** all static&universal constants and fucntions ********/
class Util
{
public:
	//initial transfer buffer size in Tree/ and Stream/
	static const unsigned TRANSFER_SIZE = 1 << 20;	//1M

	static std::string tmp_path;
	// this are for debugging
	//to build logs-system, each class: print() in time 
	static std::string debug_path;
	static FILE* debug_kvstore;				
	static FILE* debug_database;

	static int memUsedPercentage();
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

	Util();
	~Util();
};

#endif //_UTIL_UTIL_H

