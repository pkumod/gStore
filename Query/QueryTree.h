/*=============================================================================
# Filename: GeneralEvaluation.cpp
# Author: Jiaqi, Chen
# Mail: 1181955272@qq.com
# Last Modified: 2016-03-02 20:35
# Description: 
=============================================================================*/

#ifndef _QUERY_QUERYTREE_H
#define _QUERY_QUERYTREE_H

#include "../Util/Util.h"
#include "Varset.h"

class QueryTree
{
	public:
		QueryTree():query_form(Select_Query), projection_modifier(Modifier_None), projection_asterisk(false), offset(0), limit(-1){}

		enum	QueryForm {Select_Query, Ask_Query};

		enum ProjectionModifier { Modifier_None, Modifier_Distinct, Modifier_Reduced, Modifier_Count, Modifier_Duplicates };

		class Element
		{
			public:
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

		class Pattern
		{
			public:
				Element subject, predicate, object;
				Pattern(const Element _subject, const Element _predicate,const Element _object):subject(_subject), predicate(_predicate), object(_object){}
		};

		class FilterTree
		{
			public:
				enum FilterTree_Type
				{
					None_type, Or_type, And_type, Equal_type, NotEqual_type, Less_type, LessOrEqual_type, Greater_type, GreaterOrEqual_type,
					Plus_type, Minus_type, Mul_type, Div_type,	Not_type, UnaryPlus_type, UnaryMinus_type, Literal_type, Variable_type, IRI_type,
					Function_type, ArgumentList_type,Builtin_str_type, Builtin_lang_type, Builtin_langmatches_type, Builtin_datatype_type, Builtin_bound_type,
					Builtin_sameterm_type,Builtin_isiri_type, Builtin_isblank_type, Builtin_isliteral_type, Builtin_regex_type, Builtin_in_type, Builtin_exists_type
				};
				FilterTree_Type type;

				class FilterTreeChild
				{
					public:
						FilterTreeChild():type(' '), ptr(NULL), arg(""), pos(-1){}
						FilterTreeChild(const FilterTreeChild &ftc):type(ftc.type), ptr(NULL), arg(ftc.arg), pos(ftc.pos)
						{
							if (ftc.ptr != NULL)
							{
								ptr = new FilterTree();
								*ptr = *ftc.ptr;
							}
						}
						~FilterTreeChild(){	if (ptr != NULL)	delete ptr;		}

						char type;
						FilterTree *ptr;
						std::string arg;
						int pos;
				};
				std::vector<FilterTreeChild> child;
				int exists_patterngroup_id;

				FilterTree():type(None_type), exists_patterngroup_id(-1){};
				FilterTree(const FilterTree &ft):type(ft.type), child(ft.child), exists_patterngroup_id(ft.exists_patterngroup_id)	{}
		};

		class PatternGroup
		{
			public:
				class OptionalOrMinusPatternGroup;

				std::vector<Pattern> patterns;
				std::vector<std::vector<PatternGroup> > unions;
				std::vector<OptionalOrMinusPatternGroup> optionals;

				std::vector<FilterTree> filters;
				std::vector< std::vector<PatternGroup> > filter_exists_patterngroups;

				std::vector<Varset> pattern_varset;
				Varset patterngroup_varset;

				std::vector<int> pattern_blockid;

				void addOnePattern(Pattern _pattern);

				void addOneGroupUnion();
				void addOneUnion();
				PatternGroup& getLastUnion();

				void addOneOptionalOrMinus(char _type);
				PatternGroup& getLastOptionalOrMinus();

				void addOneFilterTree();
				FilterTree& getLastFilterTree();
				void addOneExistsGroupPattern();
				PatternGroup& getLastExistsGroupPattern();

				void initPatternBlockid();
				int getRootPatternBlockid(int x);
				void mergePatternBlockid(int x, int y);
		};

		class PatternGroup::OptionalOrMinusPatternGroup
		{
			public:
				PatternGroup patterngroup;
				int lastpattern, lastunions;
				char type;
				OptionalOrMinusPatternGroup(int _lastpattern, int _lastunions, char _type):
					patterngroup(PatternGroup()), lastpattern(_lastpattern), lastunions(_lastunions), type(_type){}
		};

		class Order
		{
			public:
				std::string var;
				bool descending;
				Order(std::string &_var, bool _descending):var(_var), descending(_descending){}
		};


		private:
			QueryForm query_form;
			ProjectionModifier projection_modifier;
			Varset projection;
			bool projection_asterisk;
			std::vector<Order> order;
			int offset, limit;

			PatternGroup patterngroup;

		public:
			void setQueryForm(QueryForm _queryform);
			QueryForm getQueryForm();
			void setProjectionModifier(ProjectionModifier _projection_modifier);
			ProjectionModifier getProjectionModifier();
			void addProjectionVar(std::string _projection);
			int getProjectionNum();
			Varset& getProjection();
			void setProjectionAsterisk();
			bool checkProjectionAsterisk();
			void addOrder(std::string &_var, bool _descending);
			std::vector<Order>& getOrder();
			void setOffset(int _offset);
			int getOffset();
			void setLimit(int _limit);
			int getLimit();

			PatternGroup& getPatternGroup();
};

#endif // _QUERY_QUERYTREE_H

