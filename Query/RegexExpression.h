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

class RegexExpression {
  private:
  regex_t oRegex;

  public:
  ~RegexExpression()
  {
    regfree(&oRegex);
  }

  inline bool compile(std::string& pRegexStr, std::string& flag)
  {
    int flags = 0;
    for (int i = 0; i < (int)flag.length(); i++)
      if (flag[i] == 'i')
        flags |= REG_ICASE;

    int nErrCode = regcomp(&oRegex, pRegexStr.c_str(), flags);
    return (nErrCode == 0);
  }

  inline bool match(std::string& pText)
  {
    int nErrCode = regexec(&oRegex, pText.c_str(), 0, NULL, 0);
    return (nErrCode == 0);
  }
};

#endif // _QUERY_REGEXEXPRESSION_H
