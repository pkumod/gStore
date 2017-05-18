/*=============================================================================
# Filename: Varset.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-03-02 20:35
# Description:
=============================================================================*/

#ifndef _QUERY_VARSET_H
#define _QUERY_VARSET_H

#include "../Util/Util.h"

class Varset
{
	public:
		std::vector <std::string> varset;
		Varset(){};
		Varset(std::string & _var);
		Varset(std::vector<std::string> & _varset);

		bool findVar(std::string& _var);
		void addVar(std::string& _var);

		Varset operator + (Varset& x);
		Varset operator * (Varset& x);
		Varset operator - (Varset& x);
		bool operator ==(Varset &x);
		bool hasCommonVar(Varset &x);
		bool belongTo(Varset &x);

		std::vector <int> mapTo(Varset& x);

		void print();
};

#endif // _QUERY_VARSET_H


