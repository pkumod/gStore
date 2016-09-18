/*=============================================================================
# Filename: GeneralEvaluation.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-03-02 20:33
# Description: 
=============================================================================*/

#ifndef _QUERY_GENERALEVALUATION_H
#define _QUERY_GENERALEVALUATION_H

//#include "../Database/Database.h"
#include "SPARQLquery.h"
#include "../VSTree/VSTree.h"
#include "../Database/Join.h"
#include "../Database/Strategy.h"
#include "../KVstore/KVstore.h"
#include "../Query/ResultSet.h"
#include "../Query/GeneralEvaluation.h"
#include "../Util/Util.h"
#include "../Parser/QueryParser.h"
#include "QueryTree.h"
#include "Varset.h"
#include "RegexExpression.h"
#include "ResultFilter.h"

class DistributedGeneralEvaluation : public GeneralEvaluation
{
public:
	void doQuery(const std::string &_query, int argc, char * argv[]);
};

#endif // _QUERY_GENERALEVALUATION_H

