/*
 * SPARQLquery.h
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#ifndef SPARQLQUERY_H_
#define SPARQLQUERY_H_

#include "BasicQuery.h"


class SPARQLquery{
private:
	vector<BasicQuery*> query_vec;
	vector < vector<string> >query_var_vec;
public:

	SPARQLquery();
	~SPARQLquery();

	void addQueryVar(const string& _var);

	void addQueryVarVec();

	vector<string>& getQueryVarVec(int _var_vec_id);

	void addTriple(const Triple& _triple);

	void addBasicQuery(BasicQuery* _basic_q);

	void addBasicQuery();

	const int getBasicQueryNum();

	BasicQuery& getBasicQuery(int _basic_query_id);

	void encodeQuery(KVstore* _p_kv_store);

	vector<BasicQuery*>& getBasicQueryVec();


	void print(ostream& _out_stream);
	std::string triple_str();
	std::string candidate_str();
	std::string result_str();
	std::string to_str();



	public:
	struct Element
	{
		/*
		enum Type { Variable, Literal, IRI };
		enum SubType { None, CustomLanguage, CustomType };
		Type type;
		SubType subType;
		std::string subTypeValue;
		*/
		std::string value;
		Element(const std::string& _value):value(_value){}
	};

	struct Pattern
	{
		Element subject,predicate,object;
		Pattern(const Element &_subject, const Element &_predicate,const Element &_object):subject(_subject), predicate(_predicate), object(_object){}
	};

	struct FilterTree
	{
		enum FilterTree_Type
		{
			Or, And, Equal, NotEqual, Less, LessOrEqual, Greater, GreaterOrEqual, Plus, Minus, Mul, Div,
			Not, UnaryPlus, UnaryMinus, Literal, Variable, IRI, Function, ArgumentList,
			Builtin_str, Builtin_lang, Builtin_langmatches, Builtin_datatype, Builtin_bound, Builtin_sameterm,
			Builtin_isiri, Builtin_isblank, Builtin_isliteral, Builtin_regex, Builtin_in
		};

		FilterTree_Type type;
		FilterTree* parg1,*parg2;
		std::string arg1, arg2;
		//std::string arg1Type, arg2Type;
		explicit FilterTree():parg1(NULL), parg2(NULL){}
		~FilterTree()
		{
			if (parg1 != NULL)	delete parg1;
			if (parg2 != NULL)	delete parg2;
		}
	};

	class PatternGroup
	{
		public:
		std::vector<Pattern> patterns;
		std::vector<FilterTree> filters;
		std::vector<PatternGroup> optionals;
		std::vector<std::vector<PatternGroup> > unions;
		bool hasVar;

		public:
		PatternGroup():hasVar(false){}
		void addOnePattern(Pattern _pattern);
		void addOneFilterTree();
		FilterTree& getLastFilterTree();
		void addOneOptional();
		PatternGroup& getLastOptional();
		void addOneGroupUnion();
		void addOneUnion();
		PatternGroup& getLastUnion();
	};

	class TempResult
	{
		public:
		std::vector<std::string> var;
		std::vector< std::vector<int> > res;
		TempResult()
		{}
		TempResult(const TempResult &t):var(t.var), res(t.res)
		{}
	};

	class EvaPlanEle
	{
		private:
			char type;
			void * p;
		public:
			EvaPlanEle(char _type, void *_p = NULL):type(_type), p(_p){}
			char getType()
			{	return type;	}
			void * getPointer()
			{	return p;	}
	};
	std::vector<EvaPlanEle>	evaPlan;
	std::stack<TempResult *>	evaStack;

	/*
	enum ProjectionModifier { Modifier_None, Modifier_Distinct, Modifier_Reduced, Modifier_Count, Modifier_Duplicates };
	struct Order
	{
		unsigned id;
		bool descending;
	};
	 */

	private:
	std::vector<std::string> projections;
	PatternGroup patterngroup;

	/*
	ProjectionModifier projectionModifier;
	std::vector<Order> order;
	unsigned limit;
	*/

	public:
	void addOneProjection(std::string _projection);
	int getProjectionsNum();
	std::vector<std::string>& getProjections();
	PatternGroup& getPatternGroup();
};


#endif /* SPARQLQUERY_H_ */
