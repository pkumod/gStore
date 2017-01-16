/*=============================================================================
# Filename: QueryTree.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-07-14
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
			query_form(Select_Query), update_type(Not_Update), projection_modifier(Modifier_None), projection_asterisk(false), offset(0), limit(-1){}

		enum QueryForm {Select_Query, Ask_Query};
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
								Element(const std::string &_value):
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
				Varset grouppattern_subject_object_maximal_varset, grouppattern_predicate_maximal_varset;

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

				bool checkOnlyUnionOptionalFilterNoExists();
				std::pair<Varset, Varset> checkOptionalGroupPatternVarsAndSafeFilter(Varset occur , Varset ban, bool &check_condition);

				void initPatternBlockid();
				int getRootPatternBlockID(int x);
				void mergePatternBlockID(int x, int y);

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
				enum FilterOperationType
				{
					None_type, Or_type, And_type, Equal_type, NotEqual_type, Less_type, LessOrEqual_type, Greater_type, GreaterOrEqual_type,
					Plus_type, Minus_type, Mul_type, Div_type,	Not_type, UnaryPlus_type, UnaryMinus_type, Literal_type, Variable_type, IRI_type,
					Function_type, ArgumentList_type,Builtin_str_type, Builtin_lang_type, Builtin_langmatches_type, Builtin_datatype_type, Builtin_bound_type,
					Builtin_sameterm_type,Builtin_isiri_type, Builtin_isblank_type, Builtin_isliteral_type, Builtin_regex_type, Builtin_in_type, Builtin_exists_type
				};
				FilterOperationType oper_type;

				class FilterTreeChild;

				std::vector<FilterTreeChild> child;
				int exists_grouppattern_id;

				FilterTreeNode():
					oper_type(None_type), exists_grouppattern_id(-1){}

				void getVarset(Varset &varset);

				void print(std::vector<GroupPattern> &exist_grouppatterns, int dep);
		};

		class GroupPattern::FilterTreeNode::FilterTreeChild
		{
			public:
				enum FilterTreeChildNodeType {None_type, Tree_type, String_type};
				FilterTreeChildNodeType node_type;

				FilterTreeNode node;
				std::string arg;
				int pos;
				bool isel;

				FilterTreeChild():
					node_type(None_type), pos(-1), isel(true){}
		};

		class GroupPattern::FilterTreeRoot
		{
			public:
				FilterTreeNode root;
				Varset varset;
				bool done;
				FilterTreeRoot():done(false){}
		};

		class ProjectionVar
		{
			public:
				enum	AggregateType{None_type, Count_type, Sum_type, Min_type, Max_type, Avg_type};
				AggregateType aggregate_type;

				std::string var, aggregate_var;
				bool distinct;

				ProjectionVar():aggregate_type(None_type), distinct(false){}
		};

		class Order
		{
			public:
				std::string var;
				bool descending;
				Order(std::string &_var, bool _descending):
					var(_var), descending(_descending){}
		};

		enum	UpdateType {Not_Update, Insert_Data, Delete_Data, Delete_Where, Insert_Clause, Delete_Clause, Modify_Clause};

		private:
			QueryForm query_form;
			ProjectionModifier projection_modifier;
			std::vector<ProjectionVar> projection;
			Varset projection_useful_varset;
			bool projection_asterisk;
			std::vector<Order> order;
			int offset, limit;

			GroupPattern grouppattern;

			//----------------------------------------------------------------------------------------------------------------------------------------------------

			UpdateType update_type;

			//only use patterns
			GroupPattern insert_patterns, delete_patterns;

		public:
			void setQueryForm(QueryForm _queryform);
			QueryForm getQueryForm();
			void setProjectionModifier(ProjectionModifier _projection_modifier);
			ProjectionModifier getProjectionModifier();
			void addProjectionVar();
			ProjectionVar& getLastProjectionVar();
			std::vector<ProjectionVar>& getProjection();
			std::vector<std::string> getProjectionVar();
			void addProjectionUsefulVar(std::string &_var);
			Varset& getProjectionUsefulVar();
			void setProjectionAsterisk();
			bool checkProjectionAsterisk();
			bool checkSelectCompatibility();
			bool atLeastOneAggregateFunction();
			void addOrder(std::string &_var, bool _descending);
			std::vector<Order>& getOrder();
			void setOffset(int _offset);
			int getOffset();
			void setLimit(int _limit);
			int getLimit();

			GroupPattern& getGroupPattern();

			void setUpdateType(UpdateType _updatetype);
			UpdateType getUpdateType();
			GroupPattern& getInsertPatterns();
			GroupPattern& getDeletePatterns();

			bool checkWellDesigned();

			void print();
};

#endif // _QUERY_QUERYTREE_H

