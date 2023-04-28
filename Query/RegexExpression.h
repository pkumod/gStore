/*=============================================================================
# Filename: RegexExpression.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-03-02 20:40
# Description: 
=============================================================================*/

#include "../Util/Util.h"

#ifndef _QUERY_REGEXEXPRESSION_H
#define _QUERY_REGEXEXPRESSION_H

class RegexExpression
{
	private:
		std::regex pattern;

	public:
		bool compile(std::string &pRegexStr, std::string &flag)
		{
			regex::flag_type flags;
			for (int i = 0; i < (int)flag.length(); i++)
				if (flag[i] == 'i')
					flags = regex::icase;

			try {
				pattern.assign(pRegexStr, flags);
				return true;
			} catch (regex_error& e) {
				return false;
			}
		}

		bool match(std::string &pText)
		{
			try {
				bool ret = regex_search(pText, pattern);
				return ret;
			} catch (regex_error& e) {
				return false;
			}
		}
};

#endif // _QUERY_REGEXEXPRESSION_H
