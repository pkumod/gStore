/*
 * util.h
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#ifndef UTIL_H_
#define UTIL_H_
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<sys/time.h>
#include<vector>
#include<string>
#include<dirent.h>
#include<sstream>
#include<fstream>
using namespace std;

class util{
public:
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
};


#endif /* UTIL_H_ */
