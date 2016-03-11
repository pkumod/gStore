/*=============================================================================
# Filename: Util.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:46
# Description: achieve functions in Util.h
=============================================================================*/

#include "Util.h"
using namespace std;

FILE* Util::logsfp;            //assigned by Storage

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
Util::int2string(int n)
{
	string s;
	stringstream ss;
	ss<<n;
	ss>>s;
	return s;
}

void
Util::showtime()
{
	fputs("\n\n", logsfp);
	time_t now;
	time(&now);
	fputs(ctime(&now), logsfp);
}

