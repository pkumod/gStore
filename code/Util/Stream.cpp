/*=============================================================================
# Filename:	Stream.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-20 14:10
# Description: achieve functions in Stream.h
=============================================================================*/

#include "Stream.h"

using namespace std;

Stream::Stream()
{
	this->fp = NULL;
	this->transfer.setStr((char*)malloc(Util::TRANSFER_SIZE));
	this->transfer_size = Util::TRANSFER_SIZE;
	//this->type = false;
	this->file_name = "";
	this->inMem = true;
	this->isBstr = true;
	this->cur = NULL;
}

//Stream::Stream(bool _type)
//{
//	this->fp = NULL;
//	this->transfer.setStr((char*)malloc(Util::TRANSFER_SIZE));
//	this->transfer_size = Util::TRANSFER_SIZE;
//	this->type = _type;
//}

void
Stream::reset()
{
	if(!this->inMem)
		fseek((FILE*)this->fp, 0, SEEK_SET);
	else
		cur = (char*)this->fp;
}

bool
Stream::open(bool _isBstr)
{
#ifdef DEBUG_PRECISE
	//printf("Stream:now to open stream\n");
#endif
	if(!isBstr)
		this->isBstr = _isBstr;
#ifdef DEBUG_PRECISE
	//printf("Stream:this is for Bstr\n");
#endif
	if(Util::memUsedPercentage() > 85)
		this->inMem = false;
#ifdef DEBUG_PRECISE
	//printf("Stream:after memUsedPercentage\n");
#endif
	if(this->inMem)
		return true;
	if(this->fp != NULL)
	fclose((FILE*)this->fp);
	this->file_name = Util::tmp_path + Util::int2string(Util::get_cur_time());
	this->file_name += ".dat";
#ifdef DEBUG_PRECISE
	printf("%s\n", this->file_name.c_str());
#endif
	if((this->fp = fopen(this->file_name.c_str(), "w+b")) == NULL)
		return false;
	else
		return true;	
}

bool
Stream::write(const Bstr* _bp)
{
	if(this->inMem)
	{
		char* p = (char*)malloc(sizeof(Bstr) + sizeof(void*));
		Bstr* bp = (Bstr*)p;
		bp->copy(_bp);
		p += sizeof(Bstr);
		*(long*)p = (long)this->fp;
		this->fp = bp;
		return true;
	}

	if(this->fp == NULL)
		return false;
	unsigned len = _bp->getLen();
	fwrite(&len, sizeof(unsigned), 1, (FILE*)this->fp);
	fwrite(_bp->getStr(), sizeof(char), len, (FILE*)this->fp);
	return true;
}

bool
Stream::write(const char* _str, unsigned _len)
{
	if(this->inMem)
	{
		char* p = (char*)malloc(sizeof(Bstr) + sizeof(void*));
		Bstr* bp = (Bstr*)p;
		bp->copy(_str, _len);
		p += sizeof(Bstr);
		*(long*)p = (long)this->fp;
		this->fp = bp;
		return true;
	}

	if(this->fp == NULL)
		return false;
	fwrite(&_len, sizeof(unsigned), 1, (FILE*)this->fp);
	fwrite(_str, sizeof(char), _len, (FILE*)this->fp);
	return true;
}

bool
Stream::write(unsigned _id)
{  
	if(this->inMem)
	{
		char* p = (char*)malloc(sizeof(unsigned) + sizeof(void*));
		unsigned* up = (unsigned*)p;
		*up = _id;
		p += sizeof(unsigned);
		*(long*)p = (long)this->fp;
		this->fp = up;
		return true;
	}

	if(this->fp == NULL)
		return false;
	fwrite(&_id, sizeof(unsigned), 1, (FILE*)this->fp);
	return true;
}

//BETTER:also use transfer?

const Bstr*		
Stream::read()		//default: false, ie. Bstr
{
	if(this->inMem)
	{
		Bstr* ret = (Bstr*)this->cur;	
		this->cur = (char*)*(long*)((char*)this->cur + sizeof(Bstr));
		return ret;
	}

	if(this->fp == NULL)
		return NULL;
	else if(feof((FILE*)this->fp))
		return NULL;		//indicate the end
	unsigned len = 0;
	fread(&len, sizeof(unsigned), 1, (FILE*)this->fp);
	if(len + 1 > this->transfer_size)
	{
		transfer.release();
		transfer.setStr((char*)malloc(len+1));
		this->transfer_size = len + 1;
	}
	fread(transfer.getStr(), sizeof(char), len, (FILE*)this->fp);
	transfer.getStr()[len] = '\0';	//set for string() in KVstore
	transfer.setLen(len);
	return &transfer;
}

unsigned
Stream::read(bool _type)	//default: true, ie. unsigned
{
	if(this->inMem)
	{
		unsigned ret = *(unsigned*)this->cur;	
		this->cur = (char*)*(long*)((char*)this->cur + sizeof(unsigned));
		return ret;
	}

	if(this->fp == NULL)
		return 0;			//QUERY: the id begins from 1?
	else if(feof((FILE*)this->fp))
		return 0;		//indicate the end
	unsigned len = 0;
	fread(&len, sizeof(unsigned), 1, (FILE*)this->fp);
	//if(_type)	//type is unsigned
	//{
	return len;
	//}
}

Stream::~Stream()
{
	if(this->inMem)
	{
		this->cur = (char*)this->fp;
		char* p;
		while(this->cur != NULL)
		{
			if(isBstr)
				p = (char*)*(long*)((char*)this->cur + sizeof(Bstr));
			else
				p = (char*)*(long*)((char*)this->cur + sizeof(unsigned));
			free(this->cur);
			this->cur = p;
		}
		return;
	}

	if(this->fp != NULL)
	{
		fclose((FILE*)this->fp);
		this->fp = NULL;
	}
#ifdef DEBUG_PRECISE
	//printf("file is closed in Stream!\n");
#endif
	//transfer will de deleted as Bstr
	if(!this->file_name.empty())
	{			//to remove the stream file in .tmp
		int status = remove(this->file_name.c_str());
		if(status == 0)
		{
			//printf("Ok to delete the file: %s\n", this->file_name.c_str());
		}
		else
		{
			printf("Unable to delete the file: %s\n", this->file_name.c_str());
			perror("Error");
		}
	}
}

