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
#include "../Util/Util.h"
#include "../Parser/QueryParser.h"
#include "QueryTree.h"
#include "Varset.h"
#include "RegexExpression.h"
#include "ResultFilter.h"

class GeneralEvaluation
{
private:
	QueryParser query_parser;
	QueryTree query_tree;
	SPARQLquery sparql_query;
	std::vector <Varset> sparql_query_varset;
	VSTree *vstree;
	KVstore *kvstore;
	ResultSet &result_set;
	ResultFilter result_filter;
	bool handle(SPARQLquery&);

public:
	explicit GeneralEvaluation(VSTree *_vstree, KVstore *_kvstore, ResultSet &_result_set):
		vstree(_vstree), kvstore(_kvstore), result_set(_result_set){}

	std::vector<std::vector<std::string> > getSPARQLQueryVarset();

	void doQuery(const std::string &_query);
	void doQuery(const std::string &_query, int myRank, std::string &internal_tag_str, string& lpm_str);
	bool parseQuery(const std::string &_query);
	bool onlyParseQuery(const std::string &_query, int& var_num, QueryTree::QueryForm& query_form);

	void getBasicQuery(QueryTree::GroupPattern &grouppattern);

	class FilterExistsGroupPatternResultSetRecord;

	class TempResult
	{
		public:
			Varset var;
			std::vector<int*> res;

			void release();

			static int compareFunc(int *a, std::vector<int> &p, int *b, std::vector<int> &q);
			void sort(int l, int r, std::vector<int> &p);
			int findLeftBounder(std::vector<int> &p, int *b, std::vector<int> &q);
			int findRightBounder(std::vector<int> &p, int *b, std::vector<int> &q);

			void doJoin(TempResult &x, TempResult &r);
			void doOptional(std::vector<bool> &binding, TempResult &x, TempResult &rn, TempResult &ra, bool add_no_binding);
			void doUnion(TempResult &x, TempResult &rt, TempResult &rx);
			void doMinus(TempResult &x, TempResult &r);
			void doDistinct(TempResult &r);

			void mapFilterTree2Varset(QueryTree::GroupPattern::FilterTreeNode& filter, Varset &v);
			void doFilter(QueryTree::GroupPattern::FilterTreeNode &filter, FilterExistsGroupPatternResultSetRecord &filter_exists_grouppattern_resultset_record, TempResult &r, KVstore *kvstore);
			void getFilterString(int* x, QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild &child, string &str, KVstore *kvstore);
			bool matchFilterTree(int* x, QueryTree::GroupPattern::FilterTreeNode& filter, FilterExistsGroupPatternResultSetRecord &filter_exists_grouppattern_resultset_record, KVstore *kvstore);

			void print();
	};

	class TempResultSet
	{
		public:
			std::vector<TempResult> results;

			void release();

			int findCompatibleResult(Varset &_varset);

			void doJoin(TempResultSet &x, TempResultSet &r);
			void doOptional(TempResultSet &x, TempResultSet &r);
			void doUnion(TempResultSet &x, TempResultSet &r);
			void doMinus(TempResultSet &x, TempResultSet &r);
			void doDistinct(Varset &projection, TempResultSet &r);

			void doFilter(QueryTree::GroupPattern::FilterTreeNode& filter, FilterExistsGroupPatternResultSetRecord &filter_exists_grouppattern_resultset_record, TempResultSet &r, KVstore *kvstore);

			void print();
	};

	class EvaluationUnit
	{
		private:
			char type;
			void * p;
		public:
			EvaluationUnit(char _type, void *_p = NULL):type(_type), p(_p){}
			char getType()
			{	return type;	}
			void * getPointer()
			{	return p;	}
	};

	std::vector<EvaluationUnit>	semantic_evaluation_plan;

	void generateEvaluationPlan(QueryTree::GroupPattern &grouppattern);
	void dfsJoinableResultGraph(int x, vector < pair<char, int> > &node_info, vector < vector<int> > &edge, QueryTree::GroupPattern &grouppattern);

	std::stack<TempResultSet*>	semantic_evaluation_result_stack;

	class FilterExistsGroupPatternResultSetRecord
	{
		public:
			std::vector<TempResultSet*> resultset;
			std::vector< std::vector<Varset> > common;
			std::vector< std::vector< std::pair< std::vector<int>, std::vector<int> > > > common2resultset;
	} filter_exists_grouppattern_resultset_record;

	int countFilterExistsGroupPattern(QueryTree::GroupPattern::FilterTreeNode& filter);
	void doEvaluationPlan();

	class ExpansionEvaluationStackUnit
	{
		public:
			QueryTree::GroupPattern grouppattern;
			SPARQLquery sparql_query;
			TempResultSet* result;
	};
	std::vector <ExpansionEvaluationStackUnit> expansion_evaluation_stack;

	bool expanseFirstOuterUnionGroupPattern(QueryTree::GroupPattern &grouppattern, std::deque<QueryTree::GroupPattern> &queue);
	void queryRewriteEncodeRetrieveJoin(int dep, ResultFilter &result_filter);

	void getFinalResult(ResultSet& result_str);
};

#endif // _QUERY_GENERALEVALUATION_H

