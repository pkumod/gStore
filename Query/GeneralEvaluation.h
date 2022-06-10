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
#include "../Database/CSR.h"
#include "../Database/Optimizer.h"
#include "../Database/Statistics.h"
#include "../Parser/QueryParser.h"
#include "../Util/Triple.h"
#include "../Util/Util.h"
#include "../Util/EvalMultitypeValue.h"
#include "SPARQLquery.h"
#include "QueryTree.h"
#include "Varset.h"
#include "RegexExpression.h"
#include "TempResult.h"
#include "QueryCache.h"
#include "ResultSet.h"
#include <dlfcn.h>
#include "PathQueryHandler.h"

class GeneralEvaluation
{
	private:
		QueryParser query_parser;
		QueryTree query_tree;
		int well_designed;
		// VSTree *vstree;
		KVstore *kvstore;
		StringIndex *stringindex;
		Strategy strategy;
		shared_ptr<Optimizer> optimizer_;
		QueryCache *query_cache;
		PathQueryHandler *pqHandler;
		CSR *csr;
        Statistics *statistics;
		bool ranked;

		TYPE_TRIPLE_NUM *pre2num;
		TYPE_TRIPLE_NUM *pre2sub;
		TYPE_TRIPLE_NUM *pre2obj;
		TYPE_TRIPLE_NUM triples_num;
		TYPE_PREDICATE_ID limitID_predicate;
		TYPE_ENTITY_LITERAL_ID limitID_literal;
		TYPE_ENTITY_LITERAL_ID limitID_entity;
		shared_ptr<Transaction> txn;

		shared_ptr<BGPQuery> bgp_query_total;
    public:
    	FILE* fp;
    	bool export_flag;

	public:
		GeneralEvaluation(KVstore *_kvstore, Statistics *_statistics, StringIndex *_stringindex, QueryCache *_query_cache, \
			TYPE_TRIPLE_NUM *_pre2num,TYPE_TRIPLE_NUM *_pre2sub, TYPE_TRIPLE_NUM *_pre2obj, \
			TYPE_TRIPLE_NUM _triples_num, TYPE_PREDICATE_ID _limitID_predicate, TYPE_ENTITY_LITERAL_ID _limitID_literal, \
			TYPE_ENTITY_LITERAL_ID _limitID_entity, CSR *_csr, shared_ptr<Transaction> txn = nullptr);

		~GeneralEvaluation();

		bool parseQuery(const std::string &_query);
		QueryTree& getQueryTree();

		bool doQuery();

		void addAllTriples(const QueryTree::GroupPattern &group_pattern);

		void setStringIndexPointer(StringIndex* _tmpsi);
		
	private:
		TempResultSet *temp_result;

		struct EvaluationStackStruct
		{
			QueryTree::GroupPattern group_pattern;
			TempResultSet *result;
			EvaluationStackStruct();
			EvaluationStackStruct(const EvaluationStackStruct& that);
			EvaluationStackStruct& operator=(const EvaluationStackStruct& that);
			~EvaluationStackStruct();
		};
		std::vector<EvaluationStackStruct> rewriting_evaluation_stack;

	public:
		bool expanseFirstOuterUnionGroupPattern(QueryTree::GroupPattern &group_pattern, std::deque<QueryTree::GroupPattern> &queue);

		TempResultSet* queryEvaluation(int dep);

		void getFinalResult(ResultSet &ret_result);
		void releaseResult();

		void prepareUpdateTriple(QueryTree::GroupPattern &update_pattern, TripleWithObjType *&update_triple, TYPE_TRIPLE_NUM &update_triple_num);

	private:
		void loadCSR();
		void prepPathQuery();
		void pathVec2JSON(int src, int dst, const std::vector<int> &v, std::stringstream &ss);

		int constructTriplePattern(QueryTree::GroupPattern& triple_pattern, int dep);
		void getUsefulVarset(Varset& useful, int dep);
		bool checkBasicQueryCache(vector<QueryTree::GroupPattern::Pattern>& basic_query, TempResultSet *sub_result, Varset& useful);
		void fillCandList(SPARQLquery& sparql_query, int dep, vector<vector<string> >& encode_varset);
		void fillCandList(vector<shared_ptr<BGPQuery>>& bgp_query_vec, int dep, vector<vector<string> >& encode_varset);
		void joinBasicQueryResult(SPARQLquery& sparql_query, TempResultSet *new_result, TempResultSet *sub_result, vector<vector<string> >& encode_varset, \
			vector<vector<QueryTree::GroupPattern::Pattern> >& basic_query_handle, long tv_begin, long tv_handle, int dep=0);
		void getAllPattern(const QueryTree::GroupPattern &group_pattern, vector<QueryTree::GroupPattern::Pattern> &vp);
		std::map<std::string, std::string> dynamicFunction(const std::vector<int> &iri_set, bool directed, int k, const std::vector<int> &pred_set, const std::string& fun_name, const std::string& username);

		void kleeneClosure(TempResultSet *temp, TempResult * const tr, const string &subject, const string &predicate, const string &object, int dep);
		void BFS(TempResultSet *temp, int sid, int pred, bool forward, int numCol=2);
};

#endif // _QUERY_GENERALEVALUATION_H

