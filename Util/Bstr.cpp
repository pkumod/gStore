/*=============================================================================
# Filename: Bstr.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-16 13:18
# Description: achieve functions in Bstr.h
=============================================================================*/

#include "Bstr.h"

using namespace std;

//default construct function
Bstr::Bstr()
{
	this->length = 0;
	this->str = NULL;
}

Bstr::Bstr(const char* _str, unsigned long _len, bool _nocopy)
{
	//WARN: if need a string .please add '\0' in your own!
	this->length = _len;

	//if(_nocopy)
	//{
		//this->str = _str; //not valid:const char* -> char*
		//return;
	//}

	//NOTICE: we decide to use new/delete in global area
	//this->str = (char*)malloc(_len);
	this->str = new char[_len];
	memcpy(this->str, _str, sizeof(char) * _len);
	//this->str[_len]='\0';
}

//Bstr::Bstr(char* _str, unsigned _len)
//{
//	this->length = _len;
//	this->str = _str;
//}

//copy construct function
Bstr::Bstr(const Bstr& _bstr)
{
	//DEBUG:if copy memory here
	this->length = _bstr.length;
	this->str = _bstr.str;
}

// assign function for class
// Warning: Please make sure that this->str==nullptr or its memory has been freed.
Bstr& Bstr::operator=(const Bstr& _bstr)
{
	if(this != &_bstr) {
		this->length = _bstr.length;
		this->str = _bstr.str;
	}
	return *this;
};

void Bstr::assignCopy(const Bstr& _bstr)
{
	this->length = _bstr.length;
	this->str = _bstr.str;
}

bool 
Bstr::operator > (const Bstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res == 1)
		return true;
	else
		return false;
}

bool 
Bstr::operator < (const Bstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res == -1)
		return true;
	else
		return false;
}

bool
Bstr::operator == (const Bstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res == 0)
		return true;
	else
		return false;
}

bool
Bstr::operator <= (const Bstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res <= 0)
		return true;
	else
		return false;
}

bool
Bstr::operator >= (const Bstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res >= 0)
		return true;
	else
		return false;
}

bool
Bstr::operator != (const Bstr& _bstr)
{
	int res = Util::compare(this->str, this->length, _bstr.str, _bstr.length);
	if(res != 0)
		return true;
	else
		return false;
}

unsigned long
Bstr::getLen() const
{
//WARN: we should not include too complicate logic here!!!!

	//NOTICE: this is for VList
	//if(this->isBstrLongList())
	////if(this->str == NULL)
	//{
		//return 0;
	//}

	return length;
}

void
Bstr::setLen(unsigned long _len)
{
	this->length = _len;
}

char*
Bstr::getStr() const
{
	return str;
}

void 
Bstr::setStr(char* _str)
{
	this->str = _str;
}

void
Bstr::copy(const Bstr* _bp)
{
	this->length = _bp->getLen();
	this->str = new char[this->length];
	memcpy(this->str, _bp->getStr(), sizeof(char) * this->length);
}

void Bstr::copy(const Bstr* _bp, bool del)
{
	if (del && this->str != nullptr)
		delete [] this->str;
	this->length = _bp->getLen();
	this->str = new char[this->length];
	memcpy(this->str, _bp->getStr(), sizeof(char) * this->length);
}

void
Bstr::copy(const char* _str, unsigned long _len)
{
	this->length = _len;
	//this->str = (char*)malloc(this->length);
	this->str = new char[this->length];
	memcpy(this->str, _str, this->length);
}

void
Bstr::clear()	
{
	this->str = nullptr;
	this->length = 0;
}

void
Bstr::release()
{
	//free(this->str);	//ok to be null, do nothing
	if (this->str)
	{
		delete[] this->str;
		clear();
	}
}

Bstr::~Bstr()	
{	//avoid mutiple delete
	release();
}

void
Bstr::print(string s) const
{
//TODO: add a new debug file in Util(maybe a total?)
//#ifdef DEBUG
//	Util::showtime();
//	fputs("Class Bstr\n", Util::logsfp);
//	fputs("Message: ", Util::logsfp);
//	fputs(s.c_str(), Util::logsfp);
//	fputs("\n", Util::logsfp);
//	if(s == "BSTR")
//	{	//total information, providing accurate debugging
//		fprintf(Util::logsfp, "length: %u\t the string is:\n", this->length);
//		unsigned i;
//		for(i = 0; i < this->length; ++i)
//			fputc(this->str[i], Util::logsfp);
//		fputs("\n", Util::logsfp);
//	}
//	else if(s == "bstr")
//	{	//only length information, needed when string is very long
//		fprintf(Util::logsfp, "length: %u\n", this->length);
//	}
//	else;
//#endif
}

bool
Bstr::isBstrLongList() const
{
	return this->str == NULL;
}

