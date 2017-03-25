/*=============================================================================
# Filename: GeneralEvaluation.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-09-12
# Description: 
=============================================================================*/

#ifndef _QUERY_GENERALEVALUATION_H
#define _QUERY_GENERALEVALUATION_H

//#include "../Database/Database.h"
#include "SPARQLquery.h"
#include "../VSTree/VSTree.h"
#include "../KVstore/KVstore.h"
#include "../StringIndex/StringIndex.h"
#include "../Database/Join.h"
#include "../Database/Strategy.h"
#include "../Query/ResultSet.h"
#include "../Util/Util.h"
#include "../Parser/QueryParser.h"
#include "QueryTree.h"
#include "Varset.h"
#include "RegexExpression.h"
#include "ResultFilter.h"
#include "../Util/Triple.h"

class GeneralEvaluation
{
	private:
		QueryParser query_parser;
		QueryTree query_tree;
		SPARQLquery sparql_query;
		std::vector <Varset> sparql_query_varset;
		VSTree *vstree;
		KVstore *kvstore;
		TYPE_TRIPLE_NUM* pre2num;
		TYPE_PREDICATE_ID limitID_predicate;
		TYPE_ENTITY_LITERAL_ID limitID_literal;
		StringIndex *stringindex;
		Strategy strategy;
		ResultFilter result_filter;
		bool need_output_answer;

	public:
		explicit GeneralEvaluation(VSTree *_vstree, KVstore *_kvstore, StringIndex *_stringindex, TYPE_TRIPLE_NUM* _pre2num, TYPE_PREDICATE_ID _limitID_predicate, TYPE_ENTITY_LITERAL_ID _limitID_literal):
			vstree(_vstree), kvstore(_kvstore), stringindex(_stringindex), pre2num(_pre2num), limitID_predicate(_limitID_predicate), limitID_literal(_limitID_literal), need_output_answer(false)
		{
		}

		std::vector<std::vector<std::string> > getSPARQLQueryVarset();

		bool parseQuery(const std::string &_query);
		QueryTree& getQueryTree();

		bool doQuery();

		void getBasicQuery(QueryTree::GroupPattern &grouppattern);

		class FilterExistsGroupPatternResultSetRecord;

		class FilterEvaluationMultitypeValue
		{
			public:
				class EffectiveBooleanValue
				{
					public:
						enum EBV {true_value, false_value, error_value};
						EBV value;

						EffectiveBooleanValue():value(error_value){}
						EffectiveBooleanValue(bool _value){	if (_value)	value = true_value;	else value = false_value;	}
						EffectiveBooleanValue(EBV _value):value(_value){}

						EffectiveBooleanValue operator ! ();
						EffectiveBooleanValue operator || (const EffectiveBooleanValue &x);
						EffectiveBooleanValue operator && (const EffectiveBooleanValue &x);
						EffectiveBooleanValue operator == (const EffectiveBooleanValue &x);
						EffectiveBooleanValue operator != (const EffectiveBooleanValue &x);
						EffectiveBooleanValue operator < (const EffectiveBooleanValue &x);
						EffectiveBooleanValue operator <= (const EffectiveBooleanValue &x);
						EffectiveBooleanValue operator > (const EffectiveBooleanValue &x);
						EffectiveBooleanValue operator >= (const EffectiveBooleanValue &x);
				};

				class DateTime
				{
					private:
						std::vector<int> date;
						//year = date[0]
						//month = date[1]
						//day = date[2]
						//hour = date[3]
						//minute = date[4]
						//second = date[5]

					public:
						DateTime(int _year = 0, int _month = 0, int _day = 0, int _hour = 0, int _minute = 0, int _second = 0)
						{
							this->date.reserve(6);
							this->date.push_back(_year);
							this->date.push_back(_month);
							this->date.push_back(_day);
							this->date.push_back(_hour);
							this->date.push_back(_minute);
							this->date.push_back(_second);
						}

						EffectiveBooleanValue operator == (const DateTime &x);
						EffectiveBooleanValue operator != (const DateTime &x);
						EffectiveBooleanValue operator < (const DateTime &x);
						EffectiveBooleanValue operator <= (const DateTime &x);
						EffectiveBooleanValue operator > (const DateTime &x);
						EffectiveBooleanValue operator >= (const DateTime &x);
				};

				enum DataType {rdf_term, iri, literal, xsd_string,
					xsd_boolean, xsd_integer, xsd_decimal, xsd_float, xsd_double,
					xsd_datetime};

				DataType datatype;
				string term_value, str_value;
				EffectiveBooleanValue bool_value;
				int int_value;
				float flt_value;
				double dbl_value;
				DateTime dt_value;

				bool isSimpleLiteral();
				void getSameNumericType (FilterEvaluationMultitypeValue &x);
				FilterEvaluationMultitypeValue operator !();
				FilterEvaluationMultitypeValue operator || (FilterEvaluationMultitypeValue &x);
				FilterEvaluationMultitypeValue operator && (FilterEvaluationMultitypeValue &x);
				FilterEvaluationMultitypeValue operator == (FilterEvaluationMultitypeValue &x);
				FilterEvaluationMultitypeValue operator != (FilterEvaluationMultitypeValue &x);
				FilterEvaluationMultitypeValue operator < (FilterEvaluationMultitypeValue &x);
				FilterEvaluationMultitypeValue operator <= (FilterEvaluationMultitypeValue &x);
				FilterEvaluationMultitypeValue operator > (FilterEvaluationMultitypeValue &x);
				FilterEvaluationMultitypeValue operator >= (FilterEvaluationMultitypeValue &x);

				FilterEvaluationMultitypeValue():datatype(rdf_term), int_value(0), flt_value(0), dbl_value(0){}
		};


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
				void doUnion(TempResult &rt);
				void doOptional(std::vector<bool> &binding, TempResult &x, TempResult &rn, TempResult &ra, bool add_no_binding);
				void doMinus(TempResult &x, TempResult &r);
				void doDistinct(TempResult &r);

				void mapFilterTree2Varset(QueryTree::GroupPattern::FilterTreeNode &filter, Varset &v, Varset &entity_literal_varset);
				void doFilter(QueryTree::GroupPattern::FilterTreeNode &filter, FilterExistsGroupPatternResultSetRecord &filter_exists_grouppattern_resultset_record, TempResult &r, StringIndex *stringindex, Varset &entity_literal_varset);
				void getFilterString(QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild &child, FilterEvaluationMultitypeValue &femv, int *row, StringIndex *stringindex);
				FilterEvaluationMultitypeValue matchFilterTree(QueryTree::GroupPattern::FilterTreeNode &filter, FilterExistsGroupPatternResultSetRecord &filter_exists_grouppattern_resultset_record, int *row, StringIndex *stringindex);

				void print();
		};

		class TempResultSet
		{
			public:
				std::vector<TempResult> results;

				void release();

				int findCompatibleResult(Varset &_varset);

				void doJoin(TempResultSet &x, TempResultSet &r);
				void doUnion(TempResultSet &x, TempResultSet &r);
				void doOptional(TempResultSet &x, TempResultSet &r);
				void doMinus(TempResultSet &x, TempResultSet &r);
				void doDistinct(Varset &projection, TempResultSet &r);

				void doFilter(QueryTree::GroupPattern::FilterTreeNode &filter, FilterExistsGroupPatternResultSetRecord &filter_exists_grouppattern_resultset_record, TempResultSet &r, StringIndex *stringindex, Varset &entity_literal_varset);

				void print();
		};

		class EvaluationUnit
		{
			private:
				char type;
				void *p;
			public:
				EvaluationUnit(char _type, void *_p = NULL):type(_type), p(_p){}
				char getType()
				{	return type;	}
				void *getPointer()
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

		int countFilterExistsGroupPattern(QueryTree::GroupPattern::FilterTreeNode &filter);
		void doEvaluationPlan();

		class ExpansionEvaluationStackUnit
		{
			public:
				ExpansionEvaluationStackUnit():result(NULL){}
				QueryTree::GroupPattern grouppattern;
				SPARQLquery sparql_query;
				TempResultSet *result;
		};
		std::vector <ExpansionEvaluationStackUnit> expansion_evaluation_stack;

		bool expanseFirstOuterUnionGroupPattern(QueryTree::GroupPattern &grouppattern, std::deque<QueryTree::GroupPattern> &queue);
		void queryRewriteEncodeRetrieveJoin(int dep);

		bool needOutputAnswer();
		void setNeedOutputAnswer();

		void getFinalResult(ResultSet &result_str);
		void releaseResultStack();

		void prepareUpdateTriple(QueryTree::GroupPattern &update_pattern, TripleWithObjType *&update_triple, unsigned &update_triple_num);
};

#endif // _QUERY_GENERALEVALUATION_H

