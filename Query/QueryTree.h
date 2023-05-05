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

class QueryTreeRelabeler
{
    public:
    std::string suffix;
    Varset excluding;
    void relabel(std::string& str){if(!excluding.findVar(str)) str+=suffix;}
    void relabel_safe(std::string& str){if(str.length()!=0&&str[0]=='?'&&!excluding.findVar(str)) str+=suffix;}
    void relabel(std::vector<std::string>& strs){for(std::vector<std::string>::iterator it=strs.begin(); it!=strs.end(); it++) relabel(*it);}
    void relabel(Varset& var){relabel(var.vars);}
    QueryTreeRelabeler(std::string suf):suffix(suf){}
};


class QueryTree;
class GroupPattern;
class PathArgs;
class CompTreeNode;
class ProjectionVar;
class Order;

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

		void addOneSubquery();
		QueryTree& getLastSubquery();

		void getVarset();

		std::pair<Varset, Varset> checkNoMinusAndOptionalVarAndSafeFilter(Varset occur_varset, Varset ban_varset, bool &check_condition);

		void initPatternBlockid();
		int getRootPatternBlockID(int x);
		void mergePatternBlockID(int x, int y);

		void print(int dep);
		void relabel(QueryTreeRelabeler& qtr);
};

class QueryTree
{
public:
	enum QueryForm {Select_Query, Ask_Query};
	enum ProjectionModifier {Modifier_None, Modifier_Distinct, Modifier_Reduced, Modifier_Count, Modifier_Duplicates};
	enum UpdateType {Not_Update, Insert_Data, Delete_Data, Delete_Where, Insert_Clause, Delete_Clause, Modify_Clause};
	enum VarType {Entity, Predicate, EntityPredicate};
	// typedef ::GroupPattern GroupPattern;
	// typedef ::ProjectionVar ProjectionVar;
	// typedef ::CompTreeNode CompTreeNode;
	// typedef ::Order Order;
	
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

	std::unordered_map<std::string, VarType> var2type;

	//----------------------------------------------------------------------------------------------------------------------------------------------------

	UpdateType update_type;

	//only use patterns
	GroupPattern insert_patterns, delete_patterns;

public:
	QueryTree():
		query_form(Select_Query), projection_modifier(Modifier_None), \
		projection_asterisk(false), offset(0), limit(-1),  \
		singleBGP(false), update_type(Not_Update) {}
	QueryTree(const QueryTree& _qt): query_form(_qt.query_form), projection_modifier(_qt.projection_modifier), \
		projection_asterisk(_qt.projection_asterisk), offset(_qt.offset), limit(_qt.limit), \
		singleBGP(_qt.singleBGP), update_type(_qt.update_type)
	{
		projection = _qt.projection;
		group_by = _qt.group_by;
		order_by = _qt.order_by;
		group_pattern = _qt.group_pattern;
		insert_patterns = _qt.insert_patterns;
		delete_patterns = _qt.delete_patterns;
	}
	QueryTree& operator =(const QueryTree& _qt)
	{
		query_form = _qt.query_form;
		projection_modifier = _qt.projection_modifier;
		projection_asterisk = _qt.projection_asterisk;
		offset = _qt.offset;
		limit = _qt.limit;
		singleBGP = _qt.singleBGP;
		update_type = _qt.update_type;

		projection = _qt.projection;
		group_by = _qt.group_by;
		order_by = _qt.order_by;
		group_pattern = _qt.group_pattern;
		insert_patterns = _qt.insert_patterns;
		delete_patterns = _qt.delete_patterns;

		return *this;
	}

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

	std::unordered_map<std::string, VarType>& getVar2Type();

	void setUpdateType(UpdateType _updatetype);
	UpdateType getUpdateType();
	GroupPattern& getInsertPatterns();
	GroupPattern& getDeletePatterns();

	bool checkWellDesigned();
	bool checkAtLeastOneAggregateFunction();
	bool checkAtLeastOneCompTree();
	bool checkSelectAggregateFunctionGroupByValid();

	void print();
	void relabel(QueryTreeRelabeler& qtr);
	void relabel_full(QueryTreeRelabeler& qtr);

	void setSingleBGP(bool val);
	bool getSingleBGP();
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
			Element(const Element& _ele): value(_ele.value){}
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
	void relabel(QueryTreeRelabeler& qtr){
		qtr.relabel(varset);
		qtr.relabel(subject_object_varset);
		qtr.relabel_safe(subject.value);
		qtr.relabel_safe(predicate.value);
		qtr.relabel_safe(object.value);
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
	std::vector<std::string> neg_pred_set;
	int k;
	float confidence;
	int retNum;
	std::vector<double> misc;	// Miscellaneous args (first introduced for PageRank)
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
	void relabel(QueryTreeRelabeler& qtr){
		qtr.relabel(varset);
		qtr.relabel_safe(val);
		for(std::vector<CompTreeNode>::iterator it=children.begin(); it!=children.end(); it++)
			it->relabel(qtr);
	};
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
	//@@ Bind(const Bind& _b): str(_b.str), var(_b.var), varset(_b.varset){}
	void relabel(QueryTreeRelabeler& qtr){qtr.relabel(var); qtr.relabel(varset);}
};

class GroupPattern::SubGroupPattern
{
public:
	enum SubGroupPatternType{Group_type, Pattern_type, Union_type, Optional_type, Minus_type, Filter_type, Bind_type, Subquery_type};
	SubGroupPatternType type;

	Pattern pattern;	// triplesBlock

	// graphPatternNotTriples
	GroupPattern group_pattern;
	std::vector<GroupPattern> unions;	// groupOrUnionGraphPattern
	GroupPattern optional;	// optionalGraphPattern, minusGraphPattern
	// FilterTree filter;	// filter
	CompTreeNode filter;
	Bind bind;	// bind
	QueryTree subquery;

	SubGroupPattern(SubGroupPatternType _type):type(_type){}
	SubGroupPattern(const SubGroupPattern& _sgp):type(_sgp.type)
	{
		pattern = _sgp.pattern;
		group_pattern = _sgp.group_pattern;
		unions = _sgp.unions;
		optional = _sgp.optional;
		filter = _sgp.filter;
		bind = _sgp.bind;
		subquery = _sgp.subquery;
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
		subquery = _sgp.subquery;

		return *this;
	}
	void relabel(QueryTreeRelabeler& qtr);
};

class ProjectionVar
{
public:
	enum AggregateType {None_type, Count_type, Sum_type, Min_type, Max_type, Avg_type, Groupconcat_type,
		simpleCyclePath_type, simpleCycleBoolean_type, cyclePath_type, cycleBoolean_type, 
		shortestPath_type, shortestPathLen_type, kHopReachable_type, kHopEnumerate_type, 
		kHopReachablePath_type, ppr_type, triangleCounting_type, closenessCentrality_type,
		bfsCount_type, pr_type, sssp_type, sssplen_type, labelProp_type, wcc_type, clusterCoeff_type,
		maximumKplex_type, coreTruss_type,
		CompTree_type, Contains_type, Custom_type, PFN_type, 
		kHopCount_type, kHopNeighbor_type, shortestPathCount_type, louvain_type};
	AggregateType aggregate_type;

	std::string separator;	// For GROUP_CONCAT

	std::string var, aggregate_var;
	bool distinct;

	PathArgs path_args;

	CompTreeNode comp_tree_root;

	std::vector<std::string> func_args;
	std::string custom_func_name;

	ProjectionVar():aggregate_type(None_type), distinct(false){}
	//@@ ProjectionVar(const ProjectionVar& _pv):aggregate_type(_pv.aggregate_type), var(_pv.var), aggregate_var(_pv.aggregate_var), distinct(_pv.distinct), path_args(_pv.path_args), comp_tree_root(_pv.comp_tree_root), builtin_args(_pv.builtin_args){}
	void relabel(QueryTreeRelabeler& qtr){
		// if(aggregate_type != ProjectionVar::None_type) qtr.relabel(var);
	}
	void relabel_full(QueryTreeRelabeler& qtr){
		// qtr.relabel(aggregate_var);
		// if(aggregate_type != ProjectionVar::None_type) qtr.relabel(var);
	}
};

class Order
{
public:
	std::string var;	// Don't remove for backward compatibility
	CompTreeNode comp_tree_root;	// Support expression
	bool descending;
	Order(bool _descending);
	void relabel(QueryTreeRelabeler& qtr){qtr.relabel(var);comp_tree_root.relabel(qtr);}
};

#endif // _QUERY_QUERYTREE_H