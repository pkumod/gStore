/*
 * Bstr.h
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#ifndef BSTR_H_
#define BSTR_H_
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
using namespace std;
class Bstr{
private:
	char* str;
	int len;
public:
	Bstr(const char* _str, const int _len);
	bool operator > (const Bstr& _b_str);
	bool operator < (const Bstr& _b_str);
	bool operator == (const Bstr& _b_str);
	bool read(FILE* _fp);
	int write(FILE* _fp);
};


#endif /* BSTR_H_ */
