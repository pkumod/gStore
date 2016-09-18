/*=============================================================================
# Filename: Util.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:45
# Description: common macros, functions, classes, etc.
=============================================================================*/

#ifndef _UTIL_H
#define _UTIL_H
/* basic macros and types are defined here, including common headers */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

//#define DEBUG 1		//indicate that in debug mode

/******** all static&universal constants and fucntions ********/
class Util	
{
public:
	/******** this are for debugging ********/
	//to build logs-system, each class: print() in time 
	static FILE* logsfp;				//file: executing logs

public:
	static int compare(const char* _str1, unsigned _len1, const char* _str2, unsigned _len2); //QUERY(how to use default args)
	static int string2int(std::string s);
	static std::string int2string(int n);
	//string2str: s.c_str()
	//str2string: string(str)
	static void showtime();
};

#endif 

