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

#include "GlobalTypedef.h"
#include "JsonUtil.h"
#include "TimeUtil.h"
#include "NodeUtil.h"
#include "StringUtil.h"
#include "IdUtil.h"
#include "FileUtil.h"

//basic macros and types are defined here, including common headers 
#include "INIParser.h"

#include "Latch.h"
#include "MD5.h"
#include "PrettyPrint.h"
using namespace gs;


//===================================================================================================================

/******** all static&universal constants and fucntions ********/
class Util
{
public:
	
	static std::string gserver_port_file;
	static std::string gserver_port_swap;
	static std::string gserver_log;
	//NOTICE: for endpoints, just set to 1 minute
#ifdef SPARQL_ENDPOINT
	static const int gserver_query_timeout = 60; // Timeout of gServer's query (in seconds)
#else
	static const int gserver_query_timeout = 10000; // Timeout of gServer's query (in seconds)
#endif
	
	static const long gserver_backup_interval = 120;
	static const long gserver_backup_time = 72000; // Default backup time (UTC)

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
	// static bool bsearch_preid_uporder(TYPE_PREDICATE_ID _preid, unsigned* _pair_idlist, unsigned _list_len);
	static unsigned bsearch_vec_uporder(unsigned _key, const std::vector<unsigned>* _vec);
	static std::string result_id_str(std::vector<unsigned*>& _v, int _var_num);

	static std::string get_backup_time(const std::string& path, const std::string& separator="_");
	static bool isValidPort(std::string);
	static bool isValidIP(std::string);
	static long read_backup_time();

    static std::string clear_angle_brackets(std::string _str);

	static bool is_literal_ele(TYPE_ENTITY_LITERAL_ID id);
	static bool is_entity_ele(TYPE_ENTITY_LITERAL_ID id);
	static bool is_triple_term_ele(TYPE_ENTITY_LITERAL_ID id);
	static bool isEntity(const std::string& _str);
	static bool isLiteral(const std::string& _str);
	static bool isTripleTerm(const std::string& _str);

	static unsigned removeDuplicate(unsigned*, unsigned);

	static std::string getQueryFromFile(const char* _file_path); 
	static std::string getSystemOutput(std::string cmd);
	static std::string getExactPath(const char* path);
	static std::string getItemsFromDir(std::string path);

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

	static void split(const std::string& str, const std::string& pat, std::vector<std::string>& res);
	static string getStrValue(const std::string& str);
	//NOTICE: this function must be called at the beginning of executing!
	Util();
	~Util();
	
	//NOTICE: this function must be called out of any Database to config the basic settings
	//You can call it by Util util in the first of your main program
	//Another way is to build a GstoreApplication program, and do this configure in the initialization of the application
	static bool configure();  //read conf.ini and set the parameters for this system
	static bool setGlobalConfig(INIParser& parser, string rootname, string keyname, string default_value="");
	static string getConfigureValue(const std::string& keyname, string default_value="");
	static int32_t getConfigureIntValue(const std::string& keyname, int32_t default_value=0);
	static uint64_t getConfigureLongValue(const std::string& keyname, uint64_t  default_value=0);
	// static bool config_advanced();
	// static bool config_debug();

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
	static std::pair<bool, double> checkGetNumericLiteral(std::string&);
	static std::string getArgValue(int argc, char* argv[], std::string argname,std::string argname2, std::string default_value="");
    static std::string md5(const string& text);
    static void printConsole(std::vector<std::string> &headers, std::vector<std::vector<std::string>> &rows);
    static void printFile(std::vector<std::string> &headers, std::vector<std::vector<std::string>> &rows);
    static std::string currentPath();
private:
	static bool isValidIPV4(std::string);
	static bool isValidIPV6(std::string);
};
//===================================================================================================================
#endif //_UTIL_UTIL_H

