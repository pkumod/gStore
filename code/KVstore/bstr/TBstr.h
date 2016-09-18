/*=============================================================================
# Filename: TBstr.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:33
# Description: class declaration for TBstr(used to store arbitary string) 
=============================================================================*/

#ifndef _T_BSTR_H
#define _T_BSTR_H

#include "../util/Util.h"

class TBstr
{
private:
	char* str;		//pointers consume 8 byte in 64-bit system
	unsigned length;
public:
	TBstr();
	TBstr(char* _str, unsigned _len);
	bool operator > (const TBstr& _bstr);
	bool operator < (const TBstr& _bstr);
	bool operator == (const TBstr& _bstr);
	bool operator <= (const TBstr& _bstr);
	bool operator >= (const TBstr& _bstr);
	bool operator != (const TBstr& _bstr);
	unsigned getLen() const;
	void setLen(unsigned _len);
	char* getStr() const;
	void setStr(char* _str);		//reuse a TBstr
	void release();					//release memory
	void clear();					//set str/length to 0
	void copy(const TBstr* _bp);
	~TBstr();
	void print(std::string s) const;		//DEBUG
};

#endif

