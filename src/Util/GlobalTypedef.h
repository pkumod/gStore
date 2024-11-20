#pragma once
#include <regex>
#include <cstdio>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
//DBL_MAX is contained in the header below
#include <float.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <regex.h>
#include <locale.h>
#include <assert.h>
#include <libgen.h>
#include <signal.h>

#include <sys/time.h>
#include <chrono>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <unistd.h>

//NOTICE:below are restricted to C++, C files should not include(maybe nested) this header!
#include <bitset>
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include <map>
#include <set>
#include <stack>
#include <queue>
#include <deque>
#include <vector>
#include <list>
#include <iterator>
#include <algorithm>
#include <functional>
#include <utility>
#include <new>

//NOTICE:below are libraries need to link
#include <thread>    //only for c++11 or greater versions
#include <atomic> 
#include <mutex> 
#include <condition_variable> 
#include <future> 
#include <memory> 
#include <stdexcept> 
#include <pthread.h> 
#include <math.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <exception>
#include <sys/sysinfo.h>
#include <sys/statfs.h>
//Added for __gnu_parallel::sort
#include <omp.h>
#include <parallel/algorithm>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <type_traits>

#include "Slog.h"

#define TOPK_SUPPORT
#define thread_num 1
//below is used to control if using the parallelable sort()
//#define PARALLEL_SORT 1

//NOTICE: hpp is different from static library(*.a) or dynamic library(*.so)
//It places the implementations totally in header file, hpp = *.h + *.cpp

//NOTICE: use below to forbid the warnings in third-part library
//#pragma warning(push)
//#pragma warning(disable:4009)
//#include <***>
//#pragma warning(pop) 

//===================================================================================================================

//if used as only-read application(like sparql endpoint)
//#define ONLY_READ 1
//#define SPARQL_ENDPOINT 1
#ifdef SPARQL_ENDPOINT
#ifndef ONLY_READ
#define ONLY_READ 1
#endif
#endif

//WARN: when running in parallism, please modify the limits of system
//http://www.cnblogs.com/likehua/p/3831331.html
//http://blog.csdn.net/xyang81/article/details/52779229
//
//if use pthread and lock
#define THREAD_ON 1			
//if use stream module if result is too large than memory can hold
#define STREAM_ON 1			
//when used as C/S, if output query result in the server port: default not(you can see the result in the client)
//#define OUTPUT_QUERY_RESULT 1
#define SERVER_SEND_JSON 1
//if to use readline library for console(open by default)
#define READLINE_ON	1
//if to use multiple strategy for answering queries
#define MULTI_INDEX 1
//#define SO2P 1
//#define USE_GROUP_INSERT 1
//#define USE_GROUP_DELETE 1

//indicate that in debug mode
//#define DEBUG_JOIN      
//#define DEBUG_STREAM
//#define DEBUG_PRECISE 1		//all information
//#define DEBUG_KVSTORE 1		//in KVstore
//#define DEBUG_VSTREE 1	//in Database 
//#define DEBUG_LRUCACHE 1
//#define DEBUG_DATABASE 1	//in Database
//#define DEBUG_VLIST 1
//
//

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

#ifdef DEBUG_VLIST
#ifndef DEBUG
#define DEBUG
#endif
#endif

#ifndef DEBUG
//#define DEBUG
#endif

#define xfree(x) free(x); x = NULL;

//for cpu and memory info
#define VMRSS_LINE 22
#define PROCESS_ITEM 14

//===================================================================================================================

//NOTICE:include Util.h and below in each main function
//(the beginning position)
//#ifdef DEBUG
//	Util util;
//#endif

typedef unsigned(*HashFunction)(const char*);
//NOTICE:hash functions for int are not so many, so we represent int by a 4-byte stringinstead
//(not totally change int to string, which is costly)
//http://www.cppblog.com/aurain/archive/2010/07/06/119463.html
//http://blog.csdn.net/mycomputerxiaomei/article/details/7641221
//http://kb.cnblogs.com/page/189480/

//type for the triple num
//NOTICE: this should use unsigned (triple num may > 2500000000)
typedef unsigned long long TYPE_TRIPLE_NUM;
//NOTICE: we do not use long long because it will consume more spaces in pre2num of Database
//For single machines, we aim to support 4.2B triples, and that's enough
//typedef long long TYPE_TRIPLE_NUM;
//TODO: use long if need to run 5B dataset

//type for entity/literal ID
typedef unsigned TYPE_ENTITY_LITERAL_ID;
static const TYPE_ENTITY_LITERAL_ID INVALID_ENTITY_LITERAL_ID = UINT_MAX;
//static const TYPE_ENTITY_LITERAL_ID INVALID_ENTITY_LITERAL_ID = -1;
//#define INVALID_ENTITY_LITERAL_ID UINT_MAX

using TableContent = std::list<std::vector<TYPE_ENTITY_LITERAL_ID>>;
using TableContentShardPtr = std::shared_ptr<TableContent>;
using PositionValue = std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>;
using PositionValueSharedPtr = std::shared_ptr<PositionValue>;

//type for predicate ID
typedef int TYPE_PREDICATE_ID;
static const TYPE_PREDICATE_ID INVALID_PREDICATE_ID = -1;
//static const TYPE_PREDICATE_ID INVALID_PREDICATE_ID = -1;
//#define INVALID_PREDICATE_ID -1

//type for block index in IVArray
typedef unsigned TYPE_IVBLOCK_ID;
static const TYPE_IVBLOCK_ID INVALID_BLOCK_ID = 0;

//type for concurrecy control
typedef unsigned long long TYPE_TS;
typedef unsigned long long TYPE_TXN_ID;
typedef unsigned int TYPE_READ_CNT;
const int INVALID_TYPE_ID = 0;
const unsigned long long INVALID_TS = -1;
const unsigned long long INVALID_ID = -1;
const unsigned long long  MAX_TS = -1;

//TODO:typedef several ID types and new a ID module

//TODO:encode entity from low to high, encode literal from high to low(finally select the mid of space as border)

//TODO: what is more, the Block ID in kvstore
//typedef unsigned NODE_ID;

//can use `man limits.h` to see more
static const unsigned int INVALID = UINT_MAX;
//static const int INVALID = -1;
//#define INVALID UINT_MAX

//NOTICE: always use unsigned for query result matrix
//
//NOTICE: if use define, the type is none

typedef struct TYPE_ID_TUPLE
{
	TYPE_ENTITY_LITERAL_ID subid;
//	use int_type for preid
//  TODO: need to check
	TYPE_PREDICATE_ID preid;
	TYPE_ENTITY_LITERAL_ID objid;
	TYPE_ID_TUPLE(TYPE_ENTITY_LITERAL_ID _subid, TYPE_PREDICATE_ID _preid, TYPE_ENTITY_LITERAL_ID _objid): \
		subid(_subid), preid(_preid), objid(_objid)
	{

	}
	TYPE_ID_TUPLE(){}
}ID_TUPLE;

template<typename ... Args>
inline std::string g_format(const std::string& format, Args ... args){
  size_t size = 1 + snprintf(nullptr, 0, format.c_str(), args ...);  // Extra space for \0
  // unique_ptr<char[]> buf(new char[size]);
  char bytes[size];
  snprintf(bytes, size, format.c_str(), args ...);
  return string(bytes);
}

inline std::string g_GetLineDescription(const char* file_name, const char* function, int line){
  return g_format("[%s] %s, line %d",file_name,function,line);
}

#define GetLineDescription() g_GetLineDescription(__FILE__,__FUNCTION__,__LINE__)

class GlobalTypedef {
public:
	static const uint32_t KB = 1024;
	static const uint32_t MB = 1048576;
	static const uint32_t GB = 1073741824;
	//static const int TRIPLE_NUM_MAX = 1000*1000*1000;
	static const TYPE_TRIPLE_NUM TRIPLE_NUM_MAX = (unsigned long long)-1;
	//static const TYPE_TRIPLE_NUM TRIPLE_NUM_MAX = (long long)10000*1000*1000;
	static const char EDGE_IN = 'i';
	static const char EDGE_OUT= 'o';

	//In order to differentiate the sub-part and literal-part of object
	//let subid begin with 0, while literalid begins with LITERAL_FIRST_ID 
	//used in Database and Join
	static const unsigned LITERAL_FIRST_ID = 2 * 1000*1000*1000;
	//static const int LITERAL_FIRST_ID = 2 * 1000*1000*1000;

	//initial transfer buffer size in Tree/ and Stream/
	static const unsigned TRANSFER_SIZE = 1 << 20;	//1M
	//NOTICE:the larger the faster, but need to care the memory usage(not use 1<<33, negative)
	//static const unsigned long long MAX_BUFFER_SIZE = 0xffffffff;		//max buffer size in Storage
	static const unsigned long long MAX_BUFFER_SIZE = 1 << 30;
	//static const unsigned long long MAX_BUFFER_SIZE = 0x1ffffffff;		//max buffer size in Storage
	//NOTICE:use smaller if saving space, use larger if to be faster
	//static const unsigned STORAGE_BLOCK_SIZE = 1 << 10;	//fixed size of disk-block in B+ tree storage
	static const unsigned STORAGE_BLOCK_SIZE = 1 << 12;	//fixed size of disk-block in B+ tree storage
	//max block num in kvstore storage, blockNum*blockSize for a B+ tree file should >= 256G
	//static const unsigned MAX_BLOCK_NUM = 1 << 26;
	//DEBUG:maybe the file size will over if the data is too large

	//type of B+ tree
	static const int SS_TREE = 0;
	static const int SI_TREE = 1;
	static const int II_TREE = 2;
	static const int IS_TREE = 3;

	static std::string product_name;
	static std::string product_version;
	static std::string product_website;
	static std::string tmp_path;
	static std::string debug_path;
	static std::string pid_path;
	static std::string system_db;
	static std::string profile;
	static std::string initfile;
	static std::string transaction_log_path;
	static std::string backup_log_path;
	//static bool gStore_mode;
	static std::map<std::string, std::string> global_config;
	static std::string db_home();
	static std::string db_suffix();
	static std::string db_path(const string& db_name);
	static std::string backup_path();
	static std::string upload_path();
	static std::string root_uname();	
	static std::string sys_uname();
	static int32_t backup_interval();
};

/**
 * A list of unsigned
 */
class BlockInfo
{
public:
    // starts from 1
	unsigned num;
	std::shared_ptr<BlockInfo> next;
	BlockInfo()
	{
      num = 0;
		// next = NULL;
	}
	BlockInfo(unsigned _num)
	{
      num = _num;
		// next = NULL;
	}

	/**
	 * create BlockInfo
	 * @param _num the number
	 * @param _bp the 'next' pointer
	 */
	BlockInfo(unsigned _num, std::shared_ptr<BlockInfo> _bp)
	{
    	num = _num;
		next = _bp;
	}
};

class IVBlockInfo
{
public:
	TYPE_IVBLOCK_ID num;			
	IVBlockInfo* next;
	IVBlockInfo()
	{
		num = 0;
		next = NULL;
	}
	IVBlockInfo(TYPE_IVBLOCK_ID _num)
	{
		num = _num;
		next = NULL;
	}
	IVBlockInfo(TYPE_IVBLOCK_ID _num, IVBlockInfo* _bp)
	{
		num = _num;
		next = _bp;
	}
};


//BETTER+TODO:if considering frequent insert/delete, there maybe too many empty positions, too wasteful!
//A method is to divide as groups, set the base for each, not conflict
//Reproducing the array if needed!


/**
 * A raw array of string. A simple wrapping.
 * BETTER+TODO:if considering frequent insert/delete, there maybe too many empty positions,too wasteful! .
 * A method is to divide as groups, set the base for each, not conflict. Reproducing the array if needed!
 */
class Buffer
{
public:
	unsigned size;
	std::string* buffer;
	
	Buffer(unsigned _size)
	{
		this->size = _size;
		this->buffer = new std::string[this->size];
	}
	
	bool set(unsigned _pos, const std::string& _ele)
	{
		if(_pos >= this->size)
		{
			return false;
		}
		//BETTER:check if this position is used, and decide abort or update?
		this->buffer[_pos] = _ele;
		return true;
	}

	bool del(unsigned _pos)
	{
		if(_pos >= this->size)
		{
			return false;
		}
		this->buffer[_pos] = "";
		return true;
	}

	std::string& get(unsigned _pos) const
	{
		return this->buffer[_pos];
	}

	~Buffer()
	{
		delete[] buffer;
	}
};