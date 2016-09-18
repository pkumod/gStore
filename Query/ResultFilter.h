/*=============================================================================
# Filename: ResultFilter.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-05-03 15:36
# Description:
=============================================================================*/

#ifndef _QUERY_RESULTFILTER_H
#define _QUERY_RESULTFILTER_H

#include "SPARQLquery.h"
#include "../Util/Util.h"

class ResultFilter
{
	private:
		static const int MAX_SIZE = 1048576;
		inline int hash(int x)
		{
			return ((x & (MAX_SIZE - 1)) * 17) & (MAX_SIZE - 1);
		}

		std::map<std::string, std::vector<int> > hash_table;

	public:
		void addVar(std::string var);
		std::vector<int>* findVar(std::string var);
		void change(SPARQLquery& query, int value);
		void candFilter(SPARQLquery& query);
};


#endif // _QUERY_RESULTFILTER_H

