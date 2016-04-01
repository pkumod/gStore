/*=============================================================================
# Filename: GeneralEvaluation.h
# Author: Jiaqi, Chen
# Mail: 1181955272@qq.com
# Last Modified: 2016-03-02 20:33
# Description: 
=============================================================================*/

#ifndef _QUERY_GENERALEVALUATION_H
#define _QUERY_GENERALEVALUATION_H

#include "../Util/Util.h"
#include "../Parser/QueryParser.h"
#include "QueryTree.h"
#include "SPARQLquery.h"
#include "Varset.h"
#include "../Database/Database.h"
#include "../KVstore/KVstore.h"
#include "RegexExpression.h"

class GeneralEvaluation
{
private:
	QueryParser _query_parser;
	QueryTree _query_tree;
	SPARQLquery _sparql_query;
	std::vector <Varset> _sparql_query_varset;
	KVstore *kvstore;

public:
	explicit GeneralEvaluation(KVstore *_kvstore):kvstore(_kvstore){}
	QueryParser& getQueryParser();
	QueryTree& getQueryTree();
	SPARQLquery& getSPARQLQuery();
	std::vector< std::vector< std::string > > getSPARQLQueryVarset();

	bool parseQuery(const std::string &query);

	void getVarset(QueryTree::PatternGroup &patterngroup);
	void getBasicQuery(QueryTree::PatternGroup &patterngroup);

	class FilterExistsPatternGroupResultSetRecord;

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

			void mapFilterTree2Varset(QueryTree::FilterTree& filter, Varset &v);
			void doFilter(QueryTree::FilterTree &filter, FilterExistsPatternGroupResultSetRecord &filter_exists_patterngroup_resultset_record, TempResult &r, KVstore *kvstore);
			void getFilterString(int* x, QueryTree::FilterTree::FilterTreeChild &child, string &str, KVstore *kvstore);
			bool matchFilterTree(int* x, QueryTree::FilterTree& filter, FilterExistsPatternGroupResultSetRecord &filter_exists_patterngroup_resultset_record, KVstore *kvstore);

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

			void doFilter(QueryTree::FilterTree& filter, FilterExistsPatternGroupResultSetRecord &filter_exists_patterngroup_resultset_record, TempResultSet &r, KVstore *kvstore);

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

	std::vector<EvaluationUnit>	evaluationPlan;

	void generateEvaluationPlan(QueryTree::PatternGroup &patterngroup);
	void dfsJoinableResultGraph(int x, vector < pair<char, int> > &node_info, vector < vector<int> > &edge, QueryTree::PatternGroup &patterngroup);

	std::stack<TempResultSet*>	evaluationStack;

	class FilterExistsPatternGroupResultSetRecord
	{
		public:
			std::vector<TempResultSet*> resultset;
			std::vector< std::vector<Varset> > common;
			std::vector< std::vector< std::pair< std::vector<int>, std::vector<int> > > > common2resultset;
	} filter_exists_patterngroup_resultset_record;

	int countFilterExistsPatternGroup(QueryTree::FilterTree& filter);
	void doEvaluationPlan();

	void getFinalResult(ResultSet& result_str);
};

#endif // _QUERY_GENERALEVALUATION_H

