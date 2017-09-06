/*=============================================================================
# Filename: QueryTree.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-03-13
# Description: 
=============================================================================*/

#ifndef _QUERY_QUERYTREE_H
#define _QUERY_QUERYTREE_H

#include "../Util/Util.h"
#include "Varset.h"

class QueryTree
{
	public:
		enum QueryForm {Select_Query, Ask_Query};
		enum ProjectionModifier {Modifier_None, Modifier_Distinct, Modifier_Reduced, Modifier_Count, Modifier_Duplicates};

		class GroupPattern
		{
			public:
				class Pattern;
				class FilterTree;
				class Bind;
				class SubGroupPattern;

				std::vector<SubGroupPattern> sub_group_pattern;

				Varset group_pattern_resultset_minimal_varset, group_pattern_resultset_maximal_varset;
				Varset group_pattern_subject_object_maximal_varset, group_pattern_predicate_maximal_varset;

				void addOnePattern(Pattern _pattern);

				void addOneGroupUnion();
				void addOneUnion();
				GroupPattern& getLastUnion();

				void addOneOptional(int _type);
				GroupPattern& getLastOptional();

				void addOneFilter();
				FilterTree& getLastFilter();

				void addOneBind();
				Bind& getLastBind();

				void getVarset();

				std::pair<Varset, Varset> checkNoMinusAndOptionalVarAndSafeFilter(Varset occur_varset, Varset ban_varset, bool &check_condition);

				void initPatternBlockid();
				int getRootPatternBlockID(int x);
				void mergePatternBlockID(int x, int y);

				void print(int dep);
		};

		class GroupPattern::Pattern
		{
			public:
				class Element
				{
					public:
						//enum Type { Variable, Literal, IRI };
						//enum SubType { None, CustomLanguage, CustomType };
						//Type type;
						//SubType subType;
						//std::string subTypeValue;

						std::string value;
						Element(){}
						Element(const std::string &_value):value(_value){}
				};
				Element subject, predicate, object;
				Varset varset, subject_object_varset;
				int blockid;

				Pattern():blockid(-1){}
				Pattern(const Element _subject, const Element _predicate, const Element _object):
					subject(_subject), predicate(_predicate), object(_object), blockid(-1){}

				bool operator < (const Pattern &x) const
				{
					if (this->subject.value != x.subject.value)
						return this->subject.value < x.subject.value;
					if (this->predicate.value != x.predicate.value)
						return this->predicate.value < x.predicate.value;
					return (this->object.value < x.object.value);
				}
		};

		class GroupPattern::FilterTree
		{
			public:
				class FilterTreeNode
				{
					public:
						enum FilterOperationType
						{
							None_type, Or_type, And_type, Equal_type, NotEqual_type, Less_type, LessOrEqual_type, Greater_type, GreaterOrEqual_type,
							Plus_type, Minus_type, Mul_type, Div_type, Not_type, UnaryPlus_type, UnaryMinus_type, Literal_type, Variable_type, IRI_type,
							Function_type, ArgumentList_type, Builtin_str_type, Builtin_lang_type, Builtin_langmatches_type, Builtin_datatype_type, Builtin_bound_type,
							Builtin_sameterm_type, Builtin_isiri_type, Builtin_isuri_type, Builtin_isblank_type, Builtin_isliteral_type, Builtin_isnumeric_type,
							Builtin_regex_type, Builtin_in_type, Builtin_exists_type
						};
						FilterOperationType oper_type;

						class FilterTreeChild;

						std::vector<FilterTreeChild> child;

						FilterTreeNode():oper_type(None_type){}

						void getVarset(Varset &varset);
						void mapVarPos2Varset(Varset &varset, Varset &entity_literal_varset);

						void print(int dep);
				};

				FilterTreeNode root;
				Varset varset;
				bool done;
				FilterTree():done(false){}
		};

			class GroupPattern::FilterTree::FilterTreeNode::FilterTreeChild
			{
				public:
					enum FilterTreeChildNodeType {None_type, Tree_type, String_type};
					FilterTreeChildNodeType node_type;

					FilterTreeNode node;
					std::string str;
					int pos;
					bool isel;

					FilterTreeChild():node_type(None_type), pos(-1), isel(true){}
			};

		class GroupPattern::Bind
		{
			public:
				Bind(){}
				Bind(const std::string &_str, const std::string &_var):str(_str), var(_var){}
				std::string str, var;
				Varset varset;
		};

		class GroupPattern::SubGroupPattern
		{
			public:
				enum SubGroupPatternType{Pattern_type, Union_type, Optional_type, Minus_type, Filter_type, Bind_type};
				SubGroupPatternType type;

				Pattern pattern;
				std::vector<GroupPattern> unions;
				GroupPattern optional;
				FilterTree filter;
				Bind bind;

				SubGroupPattern(SubGroupPatternType _type):type(_type){}
				SubGroupPattern(const SubGroupPattern& _sgp):type(_sgp.type)
				{
					pattern = _sgp.pattern;
					unions = _sgp.unions;
					optional = _sgp.optional;
					filter = _sgp.filter;
					bind = _sgp.bind;
				}
		};

		class ProjectionVar
		{
			public:
				enum AggregateType{None_type, Count_type, Sum_type, Min_type, Max_type, Avg_type};
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
				Order(std::string &_var, bool _descending):var(_var), descending(_descending){}
		};

		enum UpdateType {Not_Update, Insert_Data, Delete_Data, Delete_Where, Insert_Clause, Delete_Clause, Modify_Clause};

		private:
			QueryForm query_form;

			ProjectionModifier projection_modifier;
			std::vector<ProjectionVar> projection;
			bool projection_asterisk;

			Varset group_by;
			std::vector<Order> order_by;
			int offset, limit;

			GroupPattern group_pattern;

			//----------------------------------------------------------------------------------------------------------------------------------------------------

			UpdateType update_type;

			//only use patterns
			GroupPattern insert_patterns, delete_patterns;

		public:
			QueryTree():
				query_form(Select_Query), projection_modifier(Modifier_None), projection_asterisk(false), offset(0), limit(-1), update_type(Not_Update){}

			void setQueryForm(QueryForm _queryform);
			QueryForm getQueryForm();

			void setProjectionModifier(ProjectionModifier _projection_modifier);
			ProjectionModifier getProjectionModifier();

			void addProjectionVar();
			ProjectionVar& getLastProjectionVar();
			std::vector<ProjectionVar>& getProjection();
			Varset getProjectionVarset();
			Varset getResultProjectionVarset();

			void setProjectionAsterisk();
			bool checkProjectionAsterisk();

			void addGroupByVar(std::string &_var);
			Varset& getGroupByVarset();

			void addOrderVar(std::string &_var, bool _descending);
			std::vector<Order>& getOrderVarVector();
			Varset getOrderByVarset();

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
			bool checkAtLeastOneAggregateFunction();
			bool checkSelectAggregateFunctionGroupByValid();

			void print();
};

#endif // _QUERY_QUERYTREE_H

