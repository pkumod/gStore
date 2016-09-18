/*=============================================================================
# Filename: QueryTree.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
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
		QueryTree():
			query_form(Select_Query), projection_modifier(Modifier_None), projection_asterisk(false), offset(0), limit(-1){}

		enum	QueryForm {Select_Query, Ask_Query};
		enum ProjectionModifier {Modifier_None, Modifier_Distinct, Modifier_Reduced, Modifier_Count, Modifier_Duplicates};

		class GroupPattern
		{
			public:
				class Pattern
				{
					public:
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
								Element(const std::string& _value):
									value(_value){}
						};
						Element subject, predicate, object;
						Varset varset;
						Pattern(const Element _subject, const Element _predicate,const Element _object):subject(_subject), predicate(_predicate), object(_object){}
				};


				class GroupPatternUnions;
				class OptionalOrMinusGroupPattern;
				class FilterTreeNode;
				class FilterTreeRoot;

				std::vector<Pattern> patterns;
				std::vector<GroupPatternUnions> unions;
				std::vector<OptionalOrMinusGroupPattern> optionals;

				std::vector<FilterTreeRoot> filters;
				std::vector<std::vector<GroupPattern> > filter_exists_grouppatterns;

				Varset grouppattern_resultset_minimal_varset, grouppattern_resultset_maximal_varset;

				std::vector<int> pattern_blockid;

				void addOnePattern(Pattern _pattern);

				void addOneGroupUnion();
				void addOneUnion();
				GroupPattern& getLastUnion();

				void addOneOptionalOrMinus(char _type);
				GroupPattern& getLastOptionalOrMinus();

				void addOneFilterTree();
				FilterTreeNode& getLastFilterTree();
				void addOneExistsGroupPattern();
				GroupPattern& getLastExistsGroupPattern();

				void getVarset();
				int getVarNum();

				bool checkOnlyUnionOptionalFilterNoExists();
				std::pair<Varset, Varset> checkOptionalGroupPatternVarsAndSafeFilter(Varset occur , Varset ban, bool &check_condition);

				void initPatternBlockid();
				int getRootPatternBlockid(int x);
				void mergePatternBlockid(int x, int y);

				void print(int dep);
		};

		class GroupPattern::GroupPatternUnions
		{
			public:
				std::vector<GroupPattern> grouppattern_vec;
				int lastpattern;
				GroupPatternUnions(int _lastpattern):
					lastpattern(_lastpattern){}
		};

		class GroupPattern::OptionalOrMinusGroupPattern
		{
			public:
				GroupPattern grouppattern;
				int lastpattern, lastunions;
				char type;
				OptionalOrMinusGroupPattern(int _lastpattern, int _lastunions, char _type):
					grouppattern(GroupPattern()), lastpattern(_lastpattern), lastunions(_lastunions), type(_type){}
		};

		class GroupPattern::FilterTreeNode
		{
			public:
				enum FilterType
				{
					None_type, Or_type, And_type, Equal_type, NotEqual_type, Less_type, LessOrEqual_type, Greater_type, GreaterOrEqual_type,
					Plus_type, Minus_type, Mul_type, Div_type,	Not_type, UnaryPlus_type, UnaryMinus_type, Literal_type, Variable_type, IRI_type,
					Function_type, ArgumentList_type,Builtin_str_type, Builtin_lang_type, Builtin_langmatches_type, Builtin_datatype_type, Builtin_bound_type,
					Builtin_sameterm_type,Builtin_isiri_type, Builtin_isblank_type, Builtin_isliteral_type, Builtin_regex_type, Builtin_in_type, Builtin_exists_type
				};
				FilterType type;

				class FilterTreeChild;

				std::vector<FilterTreeChild> child;
				int exists_grouppattern_id;

				FilterTreeNode():
					type(None_type), exists_grouppattern_id(-1){}

				void getVarset(Varset &varset);

				void print(std::vector<GroupPattern> &exist_grouppatterns, int dep);
		};

		class GroupPattern::FilterTreeNode::FilterTreeChild
		{
			public:
				FilterTreeChild():
					type(' '), pos(-1){}

				char type;
				FilterTreeNode node;
				std::string arg;
				int pos;
		};

		class GroupPattern::FilterTreeRoot
		{
			public:
				FilterTreeNode root;
				Varset varset;
		};

		class Order
		{
			public:
				std::string var;
				bool descending;
				Order(std::string &_var, bool _descending):
					var(_var), descending(_descending){}
		};


		private:
			QueryForm query_form;
			ProjectionModifier projection_modifier;
			Varset projection;
			bool projection_asterisk;
			std::vector<Order> order;
			int offset, limit;

			GroupPattern grouppattern;

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

			GroupPattern& getGroupPattern();

			bool checkWellDesigned();

			void print();
};

#endif // _QUERY_QUERYTREE_H

