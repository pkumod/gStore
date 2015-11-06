/*=============================================================================
# Filename: Stream.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-20 13:18
# Description: 
1. stream buffer for medium results, store/write one record at a time
2. dynamicly change method(memory/disk) according to the memory usage of system
3. each Stream instance is asociated with one result-like object, and one file
4. functions using this class should implement getTriple... to operate on different records
=============================================================================*/

#ifndef _UTIL_STREAM_H
#define _UTIL_STREAM_H

#include "Util.h"
#include "../Bstr/Bstr.h"

//================================================================================
//when stream is in memory, use this struct
//struct ListNode
//{
//TYPE Record
//PONINTER next
//}
//===============================================================================

class Stream
{
private:
	//FILE* fp;
	void* fp;	//FILE* for disk or unsigned*/Bstr* when in memory
	char* cur;  //only need when in Mem
	Bstr transfer;
	unsigned transfer_size;
	std::string file_name;
	bool inMem;
	bool isBstr;
	//bool type; //false:Bstr, true:unsigned
public:
	Stream();
	Stream(bool _type);
	void reset();
	bool open(bool _isBstr = true);
	bool write(const Bstr* _bp);
	bool write(unsigned _id);
	//bool write(void* _vp);
	bool write(const char* _str, unsigned _len);
	//void* read();
	const Bstr* read();
	unsigned read(bool _type);
	~Stream();
};

#endif //_UTIL_STREAM_H

