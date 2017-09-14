/*=============================================================================
# Filename: GeneralEvaluation.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-05-05
# Description: 
=============================================================================*/

#ifndef _QUERY_GENERALEVALUATION_H
#define _QUERY_GENERALEVALUATION_H

#include "../VSTree/VSTree.h"
#include "../KVstore/KVstore.h"
#include "../StringIndex/StringIndex.h"
#include "../Database/Strategy.h"
#include "../Parser/QueryParser.h"
#include "../Util/Triple.h"
#include "../Util/Util.h"
#include "SPARQLquery.h"
#include "QueryTree.h"
#include "Varset.h"
#include "RegexExpression.h"
#include "TempResult.h"
#include "QueryCache.h"
#include "ResultSet.h"

class GeneralEvaluation
{
	private:
		QueryParser query_parser;
		QueryTree query_tree;
		VSTree *vstree;
		KVstore *kvstore;
		StringIndex *stringindex;
		Strategy strategy;
		QueryCache *query_cache;

		TYPE_TRIPLE_NUM *pre2num;
		TYPE_PREDICATE_ID limitID_predicate;
		TYPE_ENTITY_LITERAL_ID limitID_literal;
		TYPE_ENTITY_LITERAL_ID limitID_entity;

	public:
		GeneralEvaluation(VSTree *_vstree, KVstore *_kvstore, StringIndex *_stringindex, QueryCache *_query_cache, TYPE_TRIPLE_NUM *_pre2num, TYPE_PREDICATE_ID _limitID_predicate, TYPE_ENTITY_LITERAL_ID _limitID_literal, TYPE_ENTITY_LITERAL_ID _limitID_entity):
			vstree(_vstree), kvstore(_kvstore), stringindex(_stringindex), query_cache(_query_cache), pre2num(_pre2num), limitID_predicate(_limitID_predicate), limitID_literal(_limitID_literal), limitID_entity(_limitID_entity), temp_result(NULL)
		{
		}

		bool parseQuery(const std::string &_query);
		QueryTree& getQueryTree();

		bool doQuery();

	private:
		TempResultSet *temp_result;

		struct EvaluationStackStruct
		{
			QueryTree::GroupPattern group_pattern;
			TempResultSet *result;
		};
		std::vector<EvaluationStackStruct> rewriting_evaluation_stack;

	public:
		TempResultSet* semanticBasedQueryEvaluation(QueryTree::GroupPattern &group_pattern);

		bool expanseFirstOuterUnionGroupPattern(QueryTree::GroupPattern &group_pattern, std::deque<QueryTree::GroupPattern> &queue);
		TempResultSet* rewritingBasedQueryEvaluation(int dep);

		void getFinalResult(ResultSet &ret_result);
		void releaseResult();

		void prepareUpdateTriple(QueryTree::GroupPattern &update_pattern, TripleWithObjType *&update_triple, unsigned &update_triple_num);
};

#endif // _QUERY_GENERALEVALUATION_H

