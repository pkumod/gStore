/*=============================================================================
# Filename: TBstr.cpp
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:35
# Description: achieve functions in TBstr.h
=============================================================================*/

#include "TBstr.h"
using namespace std;

TBstr::TBstr()
{
	this->length = 0;
	this->str = NULL;
}

TBstr::TBstr(char* _str, unsigned _len)
{
	this->length = _len;
	this->str = _str;
}

bool 
TBstr::operator > (const TBstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res == 1)
		return true;
	else
		return false;
}

bool 
TBstr::operator < (const TBstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res == -1)
		return true;
	else
		return false;
}

bool
TBstr::operator == (const TBstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res == 0)
		return true;
	else
		return false;
}

bool
TBstr::operator <= (const TBstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res <= 0)
		return true;
	else
		return false;
}

bool
TBstr::operator >= (const TBstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res >= 0)
		return true;
	else
		return false;
}

bool
TBstr::operator != (const TBstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res != 0)
		return true;
	else
		return false;
}

unsigned
TBstr::getLen() const
{
	return length;
}

void
TBstr::setLen(unsigned _len)
{
	this->length = _len;
}

char*
TBstr::getStr() const
{
	return str;
}

void 
TBstr::setStr(char* _str)
{
	this->str = _str;
}

void
TBstr::copy(const TBstr* _bp)
{
	this->length = _bp->getLen();
	this->str = (char*)malloc(this->length);
	memcpy(this->str, _bp->getStr(), this->length);
}

void
TBstr::clear()	
{
	this->str = NULL;
	this->length = 0;
}

void
TBstr::release()
{
	free(this->str);	//ok to be null, do nothing
	clear();
}

TBstr::~TBstr()	
{	//avoid mutiple delete
	release();
}

void
TBstr::print(string s) const
{
#ifdef DEBUG
	Util::showtime();
	fputs("Class TBstr\n", Util::logsfp);
	fputs("Message: ", Util::logsfp);
	fputs(s.c_str(), Util::logsfp);
	fputs("\n", Util::logsfp);
	if(s == "BSTR")
	{	//total information, providing accurate debugging
		fprintf(Util::logsfp, "length: %u\t the string is:\n", this->length);
		unsigned i;
		for(i = 0; i < this->length; ++i)
			fputc(this->str[i], Util::logsfp);
		fputs("\n", Util::logsfp);
	}
	else if(s == "bstr")
	{	//only length information, needed when string is very long
		fprintf(Util::logsfp, "length: %u\n", this->length);
	}
	else;
#endif
}

