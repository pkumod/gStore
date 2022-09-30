/*=============================================================================
# Filename: Util.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-16 10:43
# Description: 
1. firstly written by liyouhuan, modified by zengli
2. common macros, functions, classes, etc
# Notice: we only talk about sub-graph isomorphism in the essay, however, in
# this system, the homomorphism is supported.(which means that multiple variables
in the sparql query can point to the same node in data graph)
=============================================================================*/

#ifndef _UTIL_UTIL_H
#define _UTIL_UTIL_H
#define TOPK_SUPPORT
//basic macros and types are defined here, including common headers 

#include <stdio.h>
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
#include <sys/stat.h>
//Below are for boost
//Added for the json-example
#define BOOST_SPIRIT_THREADSAFE
//#include <boost/spirit.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

//Added for the default_resource example
//#include <boost/filesystem.hpp>
//#include <boost/regex.hpp>
//#include <boost/thread/thread.hpp>
//#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/utility/string_ref.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <unordered_set>
#include <random>
#include <type_traits>


//Added for __gnu_parallel::sort
#include <omp.h>
#include <parallel/algorithm>
#include "../tools/rapidjson/document.h"
#include "../tools/rapidjson/prettywriter.h"  
#include "../tools/rapidjson/writer.h"
#include "../tools/rapidjson/stringbuffer.h"
#include "INIParser.h"

#include "Latch.h"
#include "Slog.h"

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
//#define DEBUG_PRECISE 1		all information
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

using TableContent = std::list<std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>>>;
using TableContentShardPtr = std::shared_ptr<std::list<std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>>>>;
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

//===================================================================================================================

/******** all static&universal constants and fucntions ********/
class Util
{
public:
	//static int triple_num;
	//static int pre_num;
	//static int entity_num;
	//static int literal_num;

	static const unsigned MB = 1048576;
	static const unsigned GB = 1073741824;
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

	static std::string gserver_port_file;
	static std::string gserver_port_swap;
	static std::string gserver_log;
	//NOTICE: for endpoints, just set to 1 minute
#ifdef SPARQL_ENDPOINT
	static const int gserver_query_timeout = 60; // Timeout of gServer's query (in seconds)
#else
	static const int gserver_query_timeout = 10000; // Timeout of gServer's query (in seconds)
#endif
	
	static std::string backup_path;
	static const long gserver_backup_interval = 120;
	static const long gserver_backup_time = 72000; // Default backup time (UTC)

	static std::string getThreadID();
	static int memUsedPercentage();
	static int memoryLeft();
	static int compare(const char* _str1, unsigned long _len1, const char* _str2, unsigned long _len2);
	static int string2int(std::string s);
	static std::string int2string(long n);
	static std::string string_replace(std::string rec, const std::string src, const std::string des);
	static bool is_number(std::string s);
	static char* itoa(int num, char* str, int radix);
	//string2str: s.c_str()
	//str2string: string(str)
	static int compIIpair(int _a1, int _b1, int _a2, int _b2);
	static std::string showtime();
	static int cmp_int(const void* _i1, const void* _i2);
	static int cmp_unsigned(const void* _i1, const void* _i2);
	static bool parallel_cmp_unsigned(unsigned _i1, unsigned _i2);
	static void sort(unsigned*& _id_list, unsigned _list_len);
	static unsigned bsearch_int_uporder(unsigned _key, const unsigned* _array, unsigned _array_num);
	static bool bsearch_preid_uporder(TYPE_PREDICATE_ID _preid, unsigned* _pair_idlist, unsigned _list_len);
	static unsigned bsearch_vec_uporder(unsigned _key, const std::vector<unsigned>* _vec);
	static std::string result_id_str(std::vector<unsigned*>& _v, int _var_num);
	static bool dir_exist(const std::string _dir);
    static bool file_exist(const std::string _file);
	static bool create_dir(const std:: string _dir);
	static bool create_dirs(const std:: string _dirs);
	static bool create_file(const std::string _file);

	static std::string getTimeName();
	static std::string getTimeString();
    static std::string getTimeString2();
    static int getRandNum();
	static std::string get_folder_name(const std::string path, const std::string db_name);
	static std::string get_backup_time(const std::string path, const std::string db_name);
	static long get_cur_time();
	static int get_time();
	static int str2time();
	static std::string get_date_time();
    static std::string get_date_day();
	static std::string get_timestamp();
	static time_t time_to_stamp(std::string time);
	static bool save_to_file(const char*, const std::string _content);
	static bool isValidPort(std::string);
	static bool isValidIP(std::string);
	static std::string node2string(const char* _raw_str);
	static long read_backup_time();

    static std::string replace_all(std::string _content,const std::string oldtext,const std::string newtext);

	static bool is_literal_ele(TYPE_ENTITY_LITERAL_ID id);
	static bool is_entity_ele(TYPE_ENTITY_LITERAL_ID id);
	static bool isEntity(const std::string& _str);
	static bool isLiteral(const std::string& _str);

	static unsigned removeDuplicate(unsigned*, unsigned);
	static void Csync(FILE* _fp);

	static std::string getQueryFromFile(const char* _file_path); 
	static std::string getSystemOutput(std::string cmd);
	static std::string getExactPath(const char* path);
	static std::string getItemsFromDir(std::string path);
	static void logging(std::string _str);
	static void empty_file(const char* _fname);
	static unsigned ceiling(unsigned _val, unsigned _base);

	// Below are some useful hash functions for string
	static unsigned simpleHash(const char *_str);
	static unsigned APHash(const char *_str);
	static unsigned BKDRHash(const char *_str);
	static unsigned DJBHash(const char *_str);
	static unsigned ELFHash(const char *_str);
	static unsigned DEKHash(const char* _str);
	static unsigned BPHash(const char* _str);
	static unsigned FNVHash(const char* _str);
	static unsigned HFLPHash(const char* _str);
	static unsigned HFHash(const char* _str);
	static unsigned JSHash(const char *_str);
	static unsigned PJWHash(const char *_str);
	static unsigned RSHash(const char *_str);
	static unsigned SDBMHash(const char *_str);
	static unsigned StrHash(const char* _str);
	static unsigned TianlHash(const char* _str);

	static const unsigned HashNum = 16;
	static HashFunction hash[];

	static double logarithm(double _a, double _b);
	static void intersect(unsigned*& _id_list, unsigned& _id_list_len, const unsigned* _list1, unsigned _len1, const unsigned* _list2, unsigned _len2);

	static char* l_trim(char *szOutput, const char *szInput);
	static char* r_trim(char *szOutput, const char *szInput);
	static char* a_trim(char *szOutput, const char * szInput);

	static void split(const std::string str, const std::string pat, std::vector<std::string>& res);
	//NOTICE: this function must be called at the beginning of executing!
	Util();
	~Util();
	static std::string profile;
	//NOTICE: this function must be called out of any Database to config the basic settings
	//You can call it by Util util in the first of your main program
	//Another way is to build a GstoreApplication program, and do this configure in the initialization of the application
	static bool configure();  //read init.conf and set the parameters for this system
	static bool configure_new(); //read gstore.ini and set the parameters for this system
	static bool setGlobalConfig(INIParser& parser, string rootname, string keyname);
	static string getConfigureValue(string keyname);
	static bool config_setting();
	static bool config_advanced();
	static bool config_debug();

	//static bool gStore_mode;
	static std::map<std::string, std::string> global_config;
	//static std::string db_home;
	
	//sort functions for qsort
	static int _spo_cmp(const void* _a, const void* _b);
	static bool parallel_spo_cmp(int* _a, int* _b);
	static int _ops_cmp(const void* _a, const void* _b);
	static bool parallel_ops_cmp(int* _a, int* _b);
	static int _pso_cmp(const void* _a, const void* _b);
	static bool parallel_pso_cmp(int* _a, int* _b);
	//sort functions for sort on ID_TUPLE
	static bool spo_cmp_idtuple(const ID_TUPLE& a, const ID_TUPLE& b);
	static bool ops_cmp_idtuple(const ID_TUPLE& a, const ID_TUPLE& b);
	static bool pso_cmp_idtuple(const ID_TUPLE& a, const ID_TUPLE& b);
	static bool equal(const ID_TUPLE& a, const ID_TUPLE& b);

	static std::string tmp_path;
	// this are for debugging
	//to build logs-system, each class: print() in time 
	static std::string debug_path;
	static FILE* debug_kvstore;				
	static FILE* debug_database;
	static FILE* debug_vstree;

	static void init_backuplog();
	static int add_backuplog(std::string db_name);
	static int delete_backuplog(std::string db_name);
	static int update_backuplog(std::string db_name, std::string parameter, std::string value);
	static std::string query_backuplog(std::string db_name, std::string parameter);
	static void search_backuplog(std::vector<std::string> &res, std::string parameter, std::string value);
	static bool has_record_backuplog(std::string db_name);

	static long int get_timestamp(std::string& line);
	static std::string stamp2time(int timestamp);
	static std::vector<std::string> GetFiles(const char *src_dir, const char *ext);
	static std::pair<bool, double> checkGetNumericLiteral(std::string&);
	static std::string getArgValue(int argc, char* argv[], std::string argname,std::string argname2, std::string default_value="");
    static bool checkPort(int port, std::string p_name = "");
    static std::string md5(const string& text);
    // static void formatPrint(std::string content, std::string type = "INFO");
    static std::string urlEncode(const std::string& str);
    static std::string urlDecode(const std::string& str);
    static std::string get_cur_path();

    //the function of string
    static bool iscontain(const string& _parent,const string& _child);
 private:
	
private:
	static bool isValidIPV4(std::string);
	static bool isValidIPV6(std::string);
};

//===================================================================================================================

/**
 * A list of unsigned
 */
class BlockInfo
{
public:
    // starts from 1
	unsigned num;
	BlockInfo* next;
	BlockInfo()
	{
      num = 0;
		next = NULL;
	}
	BlockInfo(unsigned _num)
	{
      num = _num;
		next = NULL;
	}

	/**
	 * create BlockInfo
	 * @param _num the number
	 * @param _bp the 'next' pointer
	 */
	BlockInfo(unsigned _num, BlockInfo* _bp)
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

// md5 class
class MD5
{
    #define S11 7
    #define S12 12
    #define S13 17
    #define S14 22
    #define S21 5
    #define S22 9
    #define S23 14
    #define S24 20
    #define S31 4
    #define S32 11
    #define S33 16
    #define S34 23
    #define S41 6
    #define S42 10
    #define S43 15
    #define S44 21
    public:
        typedef unsigned int size_type; // must be 32bit

        // default ctor, just initailize
        MD5()
        {
            init();
        }

        // nifty shortcut ctor, compute MD5 for string and finalize it right away
        MD5(const std::string &text)
        {
            init();
            update(text.c_str(), text.length());
            finalize();
        }

        // MD5 block update operation. Continues an MD5 message-digest
        // operation, processing another message block
        void update(const unsigned char input[], size_type length)
        {
            // compute number of bytes mod 64
            size_type index = count[0] / 8 % blocksize;

            // Update number of bits
            if ((count[0] += (length << 3)) < (length << 3))
                count[1]++;
            count[1] += (length >> 29);

            // number of bytes we need to fill in buffer
            size_type firstpart = 64 - index;

            size_type i;

            // transform as many times as possible.
            if (length >= firstpart)
            {
                // fill buffer first, transform
                memcpy(&buffer[index], input, firstpart);
                transform(buffer);

                // transform chunks of blocksize (64 bytes)
                for (i = firstpart; i + blocksize <= length; i += blocksize)
                transform(&input[i]);

                index = 0;
            }
            else
                i = 0;

            // buffer remaining input
            memcpy(&buffer[index], &input[i], length-i);
        }

        // for convenience provide a verson with signed char
        void update(const char *input, size_type length)
        {
            update((const unsigned char*)input, length);
        }

        // MD5 finalization. Ends an MD5 message-digest operation, writing the
        // the message digest and zeroizing the context.
        MD5& finalize()
        {
            static unsigned char padding[64] = {
                0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            };

            if (!finalized) {
                // Save number of bits
                unsigned char bits[8];
                encode(bits, count, 8);

                // pad out to 56 mod 64.
                size_type index = count[0] / 8 % 64;
                size_type padLen = (index < 56) ? (56 - index) : (120 - index);
                update(padding, padLen);

                // Append length (before padding)
                update(bits, 8);

                // Store state in digest
                encode(digest, state, 16);

                // Zeroize sensitive information.
                memset(buffer, 0, sizeof buffer);
                memset(count, 0, sizeof count);

                finalized=true;
            }

            return *this;
        }

        // return hex representation of digest as string
        std::string hexdigest() const
        {
            if (!finalized)
                return "";

            char buf[33];
            for (int i=0; i<16; i++)
                sprintf(buf+i*2, "%02x", digest[i]);
            buf[32]=0;

            return std::string(buf);
        }

        std::string md5() const
        {
            return hexdigest();
        }

        friend std::ostream& operator<<(std::ostream& out, MD5 md5)
        {
            return out << md5.hexdigest();
        }
    private:

        typedef unsigned char uint1; //  8bit
        typedef unsigned int uint4;  // 32bit
        enum {blocksize = 64}; // VC6 won't eat a const static int here

        bool finalized;
        uint1 buffer[blocksize]; // bytes that didn't fit in last 64 byte chunk
        uint4 count[2];   // 64bit counter for number of bits (lo, hi)
        uint4 state[4];   // digest so far
        uint1 digest[16]; // the result

        void init()
        {
            finalized=false;

            count[0] = 0;
            count[1] = 0;

            // load magic initialization constants.
            state[0] = 0x67452301;
            state[1] = 0xefcdab89;
            state[2] = 0x98badcfe;
            state[3] = 0x10325476;
        }

        static void decode(uint4 output[], const uint1 input[], size_type len)
        {
            for (unsigned int i = 0, j = 0; j < len; i++, j += 4)
                output[i] = ((uint4)input[j]) | (((uint4)input[j+1]) << 8) |
                (((uint4)input[j+2]) << 16) | (((uint4)input[j+3]) << 24);
        }

        static void encode(uint1 output[], const uint4 input[], size_type len)
        {
            for (size_type i = 0, j = 0; j < len; i++, j += 4) {
                output[j] = input[i] & 0xff;
                output[j+1] = (input[i] >> 8) & 0xff;
                output[j+2] = (input[i] >> 16) & 0xff;
                output[j+3] = (input[i] >> 24) & 0xff;
            }
        }

        // low level logic operations
        static inline uint4 F(uint4 x, uint4 y, uint4 z)
        {
            return (x & y) | ((~x) & z);
        }
        static inline uint4 G(uint4 x, uint4 y, uint4 z)
        {
            return (x&z) | (y&~z);
        }
        static inline uint4 H(uint4 x, uint4 y, uint4 z)
        {
            return x^y^z;
        }
        static inline uint4 I(uint4 x, uint4 y, uint4 z)
        {
            return y ^ (x | ~z);
        }
        static inline uint4 rotate_left(uint4 x, int n)
        {
            return (x << n) | (x >> (32-n));
        }
        // FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
        // Rotation is separate from addition to prevent recomputation.
        static inline void FF(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
        {
            a = rotate_left(a+ F(b,c,d) + x + ac, s) + b;
        }
        static inline void GG(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
        {
            a = rotate_left(a + G(b,c,d) + x + ac, s) + b;
        }
        static inline void HH(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
        {
            a = rotate_left(a + H(b,c,d) + x + ac, s) + b;
        }
        static inline void II(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac)
        {
            a = rotate_left(a + I(b,c,d) + x + ac, s) + b;
        }

        void transform(const uint1 block[blocksize])
        {
            uint4 a = state[0], b = state[1], c = state[2], d = state[3], x[16];
            decode (x, block, blocksize);

            /* Round 1 */
            FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
            FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
            FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
            FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
            FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
            FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
            FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
            FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
            FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
            FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
            FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
            FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
            FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
            FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
            FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
            FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

            /* Round 2 */
            GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
            GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
            GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
            GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
            GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
            GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
            GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
            GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
            GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
            GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
            GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
            GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
            GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
            GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
            GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
            GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

            /* Round 3 */
            HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
            HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
            HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
            HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
            HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
            HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
            HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
            HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
            HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
            HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
            HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
            HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
            HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
            HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
            HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
            HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

            /* Round 4 */
            II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
            II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
            II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
            II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
            II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
            II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
            II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
            II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
            II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
            II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
            II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
            II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
            II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
            II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
            II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
            II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

            state[0] += a;
            state[1] += b;
            state[2] += c;
            state[3] += d;

            // Zeroize sensitive information.
            memset(x, 0, sizeof x);
        }
};
#endif //_UTIL_UTIL_H

