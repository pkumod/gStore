/*
 * Bstr.cpp
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#include "Bstr.h"

Bstr::Bstr(const char* _str, const int _len)
{
	this->len = _len;
	this->str = new char[len+1];
	memcpy(this->str, _str, sizeof(char)*_len);
	this->str[_len]='\0';
}
bool Bstr::operator > (const Bstr& _b_str)
{

	return true;
}
bool Bstr::operator < (const Bstr& _b_str)
{
	return true;
}
bool Bstr::operator == (const Bstr& _b_str)
{
	return true;
}
bool Bstr::read(FILE* _fp)
{
	return true;
}
int  Bstr::write(FILE* _fp)
{
	return 0;
}

