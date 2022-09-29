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

		class CompTreeNode;
		
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

				GroupPattern() { }
				GroupPattern(const GroupPattern& that);
				GroupPattern& operator=(const GroupPattern& that);
				~GroupPattern() { }

				void addOnePattern(Pattern _pattern);

				void addOneGroup();
				GroupPattern& getLastGroup();
				
				void addOneGroupUnion();
				void addOneUnion();
				GroupPattern& getLastUnion();

				void addOneOptional(int _type);
				GroupPattern& getLastOptional();

				void addOneFilter();
				CompTreeNode& getLastFilter();

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
				bool kleene;	// Denote if the predicate is a Kleene closure
				Varset varset, subject_object_varset;
				int blockid;

				Pattern():kleene(false), blockid(-1){}
				Pattern(const Element _subject, const Element _predicate, const Element _object, bool _kleene=false):
					subject(_subject), predicate(_predicate), object(_object), kleene(_kleene), blockid(-1){}
				Pattern(const Pattern& _pattern)
				{
					subject.value = _pattern.subject.value;
					predicate.value = _pattern.predicate.value;
					object.value = _pattern.object.value;
					varset = _pattern.varset;
					subject_object_varset = _pattern.subject_object_varset;
					blockid = _pattern.blockid;
					kleene = _pattern.kleene;
				}

				Pattern& operator = (const Pattern &_pattern)
				{
					subject.value = _pattern.subject.value;
					predicate.value = _pattern.predicate.value;
					object.value = _pattern.object.value;
					varset = _pattern.varset;
					subject_object_varset = _pattern.subject_object_varset;
					blockid = _pattern.blockid;
					kleene = _pattern.kleene;
					
					return *this;
				}

				bool operator < (const Pattern &x) const
				{
					if (this->subject.value != x.subject.value)
						return this->subject.value < x.subject.value;
					if (this->predicate.value != x.predicate.value)
						return this->predicate.value < x.predicate.value;
					return (this->object.value < x.object.value);
				}

				bool operator == (const Pattern &x) const
				{
					if (this->subject.value == x.subject.value \
						&& this->predicate.value == x.predicate.value \
						&& this->object.value == x.object.value && this->kleene == x.kleene)
						return true;
					return false;
				}
		};

		class PathArgs
		{
		public:
			std::string src, dst, fun_name;
			bool directed;
			std::vector<std::string> iri_set;
			std::vector<std::string> vert_set;
			std::vector<std::string> pred_set;
			int k;
			float confidence;
			int retNum;
		};

		class CompTreeNode
		{
		public:
			std::string oprt;	// operator
			// CompTreeNode *lchild;
			// CompTreeNode *rchild;
			std::vector<CompTreeNode> children;	// child nodes
			std::string val;	// variable, or literal followed by datatype suffix
			PathArgs path_args;
			Varset varset;
			bool done;

			CompTreeNode(): done(false) {}
			// CompTreeNode(const CompTreeNode& that);
			// CompTreeNode& operator=(const CompTreeNode& that);
			// ~CompTreeNode();
			void print(int dep);	// Print subtree rooted at this node
			Varset getVarset();
			CompTreeNode(const CompTreeNode& that)
			{
				oprt = that.oprt;
				children = that.children;
				val = that.val;
				path_args = that.path_args;
				varset = that.varset;
				done = that.done;
			}
			CompTreeNode& operator = (const CompTreeNode& that)
			{
				oprt = that.oprt;
				children = that.children;
				val = that.val;
				path_args = that.path_args;
				varset = that.varset;
				done = that.done;

				return *this;
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
							Builtin_regex_type, Builtin_in_type, Builtin_exists_type,
							Builtin_contains_type, Builtin_ucase_type, Builtin_lcase_type, Builtin_strstarts_type,
							Builtin_now_type, Builtin_year_type, Builtin_month_type, Builtin_day_type,
							Builtin_abs_type,
							Builtin_simpleCycle_type, Builtin_cycle_type, Builtin_sp_type, Builtin_khop_type
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

					PathArgs path_args;

					FilterTreeChild():node_type(None_type), pos(-1), isel(true){}
			};

		class GroupPattern::Bind
		{
			public:
				Bind(){}
				Bind(const std::string &_var):var(_var){}
				// std::string str, var;
				CompTreeNode bindExpr;
				std::string var;
				Varset varset;
		};

		class GroupPattern::SubGroupPattern
		{
			public:
				enum SubGroupPatternType{Group_type, Pattern_type, Union_type, Optional_type, Minus_type, Filter_type, Bind_type};
				SubGroupPatternType type;

				Pattern pattern;	// triplesBlock

				// graphPatternNotTriples
				GroupPattern group_pattern;
				std::vector<GroupPattern> unions;	// groupOrUnionGraphPattern
				GroupPattern optional;	// optionalGraphPattern, minusGraphPattern
				// FilterTree filter;	// filter
				CompTreeNode filter;
				Bind bind;	// bind

				SubGroupPattern(SubGroupPatternType _type):type(_type){}
				SubGroupPattern(const SubGroupPattern& _sgp):type(_sgp.type)
				{
					pattern = _sgp.pattern;
					group_pattern = _sgp.group_pattern;
					unions = _sgp.unions;
					optional = _sgp.optional;
					filter = _sgp.filter;
					bind = _sgp.bind;
				}
				SubGroupPattern& operator =(const SubGroupPattern& _sgp)
				{
					type = _sgp.type;
					pattern = _sgp.pattern;
					group_pattern = _sgp.group_pattern;
					unions = _sgp.unions;
					optional = _sgp.optional;
					filter = _sgp.filter;
					bind = _sgp.bind;

					return *this;
				}
		};

		class ProjectionVar
		{
			public:
				enum AggregateType {None_type, Count_type, Sum_type, Min_type, Max_type, Avg_type, Groupconcat_type,
					simpleCyclePath_type, simpleCycleBoolean_type, cyclePath_type, cycleBoolean_type, 
					shortestPath_type, shortestPathLen_type, kHopReachable_type, kHopEnumerate_type, 
					kHopReachablePath_type, ppr_type, triangleCounting_type, closenessCentrality_type,
					bfsCount_type,
					CompTree_type, Contains_type, Custom_type,
					PFN_type};
				AggregateType aggregate_type;

				std::string separator;	// For GROUP_CONCAT

				std::string var, aggregate_var;
				bool distinct;

				PathArgs path_args;

				CompTreeNode comp_tree_root;

				std::vector<std::string> func_args;
				std::string custom_func_name;

				ProjectionVar():aggregate_type(None_type), distinct(false){}
		};

		class Order
		{
			public:
				std::string var;	// Don't remove for backward compatibility
				CompTreeNode comp_tree_root;	// Support expression
				bool descending;
				// Order(std::string &_var, bool _descending): var(_var), descending(_descending) { comp_tree_root = new CompTreeNode(); }
				Order(bool _descending);
				// Order(const Order& that);
				// Order& operator=(const Order& that);
				// ~Order();
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

			bool singleBGP;

			//----------------------------------------------------------------------------------------------------------------------------------------------------

			UpdateType update_type;

			//only use patterns
			GroupPattern insert_patterns, delete_patterns;

		public:
			QueryTree():
				query_form(Select_Query), projection_modifier(Modifier_None), \
				projection_asterisk(false), offset(0), limit(-1), update_type(Not_Update), \
				singleBGP(false) {}

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

			// void addOrderVar(std::string &_var, bool _descending);
			void addOrderVar(bool _descending);
			std::vector<Order>& getOrderVarVector();
			Order& getLastOrderVar();
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

			void setSingleBGP(bool val);
			bool getSingleBGP();
};

#endif // _QUERY_QUERYTREE_H

