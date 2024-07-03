/*=============================================================================
# Filename: QueryTree.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-03-13
# Description: implement functions in QueryTree.h
=============================================================================*/

#include "QueryTree.h"

using namespace std;

GroupPattern::GroupPattern(const GroupPattern& that)
{
	sub_group_pattern = that.sub_group_pattern;
	group_pattern_resultset_minimal_varset = that.group_pattern_resultset_minimal_varset;
	group_pattern_resultset_maximal_varset = that.group_pattern_resultset_maximal_varset;
	group_pattern_subject_object_maximal_varset = that.group_pattern_subject_object_maximal_varset;
	group_pattern_predicate_maximal_varset = that.group_pattern_predicate_maximal_varset;
}

GroupPattern& GroupPattern::operator=(const GroupPattern& that)
{
	sub_group_pattern = that.sub_group_pattern;
	group_pattern_resultset_minimal_varset = that.group_pattern_resultset_minimal_varset;
	group_pattern_resultset_maximal_varset = that.group_pattern_resultset_maximal_varset;
	group_pattern_subject_object_maximal_varset = that.group_pattern_subject_object_maximal_varset;
	group_pattern_predicate_maximal_varset = that.group_pattern_predicate_maximal_varset;

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

/**
	Add a triple to this group graph pattern.

	@param _pattern the triple to add.
*/
void GroupPattern::addOnePattern(Pattern _pattern)
{
	this->sub_group_pattern.push_back(SubGroupPattern(SubGroupPattern::Pattern_type));
	this->sub_group_pattern.back().pattern = _pattern;
}

/**
	Add a nested group graph pattern to this group graph pattern.
*/
void GroupPattern::addOneGroup()
{
	this->sub_group_pattern.emplace_back(SubGroupPattern(SubGroupPattern::Group_type));
}

/**
	Get the nested group graph pattern at the back of this group graph pattern.
	If the component at the back of this group graph pattern is not a nested group
	graph pattern, throw an exception.

	@return the requested group graph pattern.
*/
GroupPattern& GroupPattern::getLastGroup()
{
	if (sub_group_pattern.size() == 0 \
		|| sub_group_pattern.back().type != SubGroupPattern::Group_type)
		throw "GroupPattern::getLastGroup failed";

	return sub_group_pattern.back().group_pattern;
}

/**
	Add a UNION to this group graph pattern.
*/
void GroupPattern::addOneGroupUnion()
{
	this->sub_group_pattern.push_back(SubGroupPattern(SubGroupPattern::Union_type));
}

/**
	Add a group graph pattern to the UNION at the back of this group graph pattern.
	If the component at the back of this group graph pattern is not a UNION, throw 
	an exception.
*/
void GroupPattern::addOneUnion()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Union_type)
		throw "GroupPattern::addOneUnion failed";

	this->sub_group_pattern.back().unions.push_back(GroupPattern());
}

/**
	Get the group graph pattern at the back of the UNION at the back of this
	group graph pattern.
	If the component at the back of this group graph pattern is not a UNION, throw an 
	exception.

	@return the requested group graph pattern.
*/
GroupPattern& GroupPattern::getLastUnion()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Union_type || this->sub_group_pattern.back().unions.empty())
		throw "GroupPattern::getLastUnion failed";

	return this->sub_group_pattern.back().unions.back();
}

/**
	Add an OPTIONAL or a MINUS to this group graph pattern, according to the parameter _type.
	If _type is neither OPTIONAL nor MINUS, throw an exception.

	@param _type the type of the component to add.
*/
void GroupPattern::addOneOptional(int _type)
{
	SubGroupPattern::SubGroupPatternType type = (SubGroupPattern::SubGroupPatternType)_type;
	if (type != SubGroupPattern::Optional_type && type != SubGroupPattern::Minus_type)
		throw "GroupPattern::addOneOptional failed";

	this->sub_group_pattern.push_back(SubGroupPattern(type));
}

/**
	Get the OPTIONAL or MINUS at the back of this group graph pattern.
	If the component at the back of this group graph pattern is not an OPTIONAL
	or a MINUS, throw an exception.

	@return the requested OPTIONAL or MINUS.
*/
GroupPattern& GroupPattern::getLastOptional()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Optional_type && this->sub_group_pattern.back().type != SubGroupPattern::Minus_type)
		throw "GroupPattern::getLastOptional failed";

	return this->sub_group_pattern.back().optional;
}

/**
	Add a FILTER to this group graph pattern.
*/
void GroupPattern::addOneFilter()
{
	this->sub_group_pattern.push_back(SubGroupPattern(SubGroupPattern::Filter_type));
}

/**
	Get the FILTER at the back of this group graph pattern.
	If the component at the back of this group graph pattern is not a FILTER, throw
	an exception.

	@return the requested FILTER.
*/
// GroupPattern::FilterTree& GroupPattern::getLastFilter()
CompTreeNode& GroupPattern::getLastFilter()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Filter_type)
		throw "GroupPattern::getLastFilter failed";

	return this->sub_group_pattern.back().filter;
}

/**
	Add a BIND to this group graph pattern.
*/
void GroupPattern::addOneBind()
{
	this->sub_group_pattern.push_back(SubGroupPattern(SubGroupPattern::Bind_type));
}

/**
	Get the BIND at the back of this group graph pattern.
	If the component at the back of this group graph pattern is not a BIND, throw
	an exception.

	@return the requested BIND.
*/
GroupPattern::Bind& GroupPattern::getLastBind()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Bind_type)
		throw "GroupPattern::getLastBind failed";

	return this->sub_group_pattern.back().bind;
}

void GroupPattern::addOneSubquery()
{
	this->sub_group_pattern.emplace_back(SubGroupPattern::Subquery_type);
}


QueryTree& GroupPattern::getLastSubquery()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Subquery_type)
		throw "GroupPattern::getLastSubquery failed";

	return this->sub_group_pattern.back().subquery;
}

/**
	Recursively compute the varset of each component of this group graph pattern.
*/
void GroupPattern::getVarset()
{
	for (int i = 0; i < (int)this->sub_group_pattern.size(); i++)
		if (sub_group_pattern[i].type == SubGroupPattern::Group_type)
		{
			sub_group_pattern[i].group_pattern.getVarset();

			group_pattern_resultset_minimal_varset += sub_group_pattern[i].group_pattern.group_pattern_resultset_minimal_varset;
			group_pattern_resultset_maximal_varset += sub_group_pattern[i].group_pattern.group_pattern_resultset_maximal_varset;
			group_pattern_subject_object_maximal_varset += sub_group_pattern[i].group_pattern.group_pattern_subject_object_maximal_varset;
			group_pattern_predicate_maximal_varset += sub_group_pattern[i].group_pattern.group_pattern_predicate_maximal_varset;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Pattern_type)
		{
			if (this->sub_group_pattern[i].pattern.subject.value[0] == '?')
			{
				this->sub_group_pattern[i].pattern.varset.addVar(this->sub_group_pattern[i].pattern.subject.value);
				this->sub_group_pattern[i].pattern.subject_object_varset.addVar(this->sub_group_pattern[i].pattern.subject.value);
				this->group_pattern_subject_object_maximal_varset.addVar(this->sub_group_pattern[i].pattern.subject.value);
			}
			if (this->sub_group_pattern[i].pattern.predicate.value[0] == '?')
			{
				this->sub_group_pattern[i].pattern.varset.addVar(this->sub_group_pattern[i].pattern.predicate.value);
				this->group_pattern_predicate_maximal_varset.addVar(this->sub_group_pattern[i].pattern.predicate.value);
			}
			if (this->sub_group_pattern[i].pattern.object.value[0] == '?')
			{
				this->sub_group_pattern[i].pattern.varset.addVar(this->sub_group_pattern[i].pattern.object.value);
				this->sub_group_pattern[i].pattern.subject_object_varset.addVar(this->sub_group_pattern[i].pattern.object.value);
				this->group_pattern_subject_object_maximal_varset.addVar(this->sub_group_pattern[i].pattern.object.value);
			}
			this->group_pattern_resultset_minimal_varset += this->sub_group_pattern[i].pattern.varset;
			this->group_pattern_resultset_maximal_varset += this->sub_group_pattern[i].pattern.varset;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Union_type)
		{
			Varset minimal_varset;

			for (int j = 0; j < (int)this->sub_group_pattern[i].unions.size(); j++)
			{
				this->sub_group_pattern[i].unions[j].getVarset();
				if (j == 0)
					minimal_varset = this->sub_group_pattern[i].unions[j].group_pattern_resultset_minimal_varset;
				else
					minimal_varset = minimal_varset * this->sub_group_pattern[i].unions[j].group_pattern_resultset_minimal_varset;
				this->group_pattern_resultset_maximal_varset += this->sub_group_pattern[i].unions[j].group_pattern_resultset_maximal_varset;
				this->group_pattern_subject_object_maximal_varset += this->sub_group_pattern[i].unions[j].group_pattern_subject_object_maximal_varset;
				this->group_pattern_predicate_maximal_varset += this->sub_group_pattern[i].unions[j].group_pattern_predicate_maximal_varset;
			}

			this->group_pattern_resultset_minimal_varset += minimal_varset;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Optional_type)
		{
			this->sub_group_pattern[i].optional.getVarset();

			this->group_pattern_resultset_maximal_varset += this->sub_group_pattern[i].optional.group_pattern_resultset_maximal_varset;
			this->group_pattern_subject_object_maximal_varset += this->sub_group_pattern[i].optional.group_pattern_subject_object_maximal_varset;
			this->group_pattern_predicate_maximal_varset += this->sub_group_pattern[i].optional.group_pattern_predicate_maximal_varset;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Minus_type)
		{
			this->sub_group_pattern[i].optional.getVarset();
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Filter_type)
			this->sub_group_pattern[i].filter.varset = this->sub_group_pattern[i].filter.getVarset();
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Bind_type)
		{
			this->sub_group_pattern[i].bind.varset = Varset(this->sub_group_pattern[i].bind.var);
			this->group_pattern_resultset_minimal_varset += this->sub_group_pattern[i].bind.varset;
			this->group_pattern_resultset_maximal_varset += this->sub_group_pattern[i].bind.varset;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Subquery_type)
		{
			this->sub_group_pattern[i].subquery.getGroupPattern().getVarset();
			this->group_pattern_resultset_minimal_varset += sub_group_pattern[i].subquery.getGroupPattern().group_pattern_resultset_minimal_varset;
			this->group_pattern_resultset_maximal_varset += sub_group_pattern[i].subquery.getGroupPattern().group_pattern_resultset_maximal_varset;
			this->group_pattern_subject_object_maximal_varset += sub_group_pattern[i].subquery.getGroupPattern().group_pattern_subject_object_maximal_varset;
			this->group_pattern_predicate_maximal_varset += sub_group_pattern[i].subquery.getGroupPattern().group_pattern_predicate_maximal_varset;
		}
}

/**
	If check_condition is initially true, check whether the query is well-designed
	(no nested group graph pattern, no MINUS, no OPTIONAL, safe filter) and store the 
	result in check_condition.

	@param occur_varset ?
	@param ban_varset ?
	@return pair of occur varset and ban varset.
*/
pair<Varset, Varset> GroupPattern::checkNoMinusAndOptionalVarAndSafeFilter(Varset occur_varset, Varset ban_varset, bool &check_condition)
{
	if (!check_condition)	return make_pair(Varset(), Varset());

	Varset new_ban_varset;

	for (int i = 0; i < (int)this->sub_group_pattern.size(); i++)
		if (!check_condition)	break;
		else if (sub_group_pattern[i].type == SubGroupPattern::Group_type)
			check_condition = false;
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Pattern_type)
		{
			if (this->sub_group_pattern[i].pattern.varset.hasCommonVar(ban_varset))
				check_condition = false;

			occur_varset += this->sub_group_pattern[i].pattern.varset;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Union_type)
		{
			Varset sub_occur_varset, sub_ban_varset;

			for (int j = 0; j < (int)this->sub_group_pattern[i].unions.size(); j++)
			{
				pair<Varset, Varset> sub_return_varset =
					this->sub_group_pattern[i].unions[j].checkNoMinusAndOptionalVarAndSafeFilter(occur_varset, ban_varset, check_condition);

				if (j == 0)
					sub_occur_varset = sub_return_varset.first;
				else
					sub_occur_varset = sub_occur_varset * sub_return_varset.first;

				sub_ban_varset += sub_return_varset.second;
			}

			new_ban_varset += sub_ban_varset;
			occur_varset += sub_occur_varset;
			ban_varset += new_ban_varset;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Optional_type)
		{
			pair<Varset, Varset> sub_return_varset =
				this->sub_group_pattern[i].optional.checkNoMinusAndOptionalVarAndSafeFilter(Varset(), ban_varset, check_condition);

			//occur before
			if (occur_varset.hasCommonVar(sub_return_varset.second))
				check_condition = false;

			new_ban_varset += sub_return_varset.second;
			new_ban_varset += this->sub_group_pattern[i].optional.group_pattern_resultset_maximal_varset - occur_varset;
			occur_varset += sub_return_varset.first;
			ban_varset += new_ban_varset;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Minus_type)
		{
			check_condition = false;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Filter_type)
		{
			if (!this->sub_group_pattern[i].filter.varset.belongTo(occur_varset))
				check_condition = false;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Bind_type)
		{
			if (this->sub_group_pattern[i].bind.varset.hasCommonVar(ban_varset))
				check_condition = false;

			occur_varset += this->sub_group_pattern[i].bind.varset;
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Subquery_type)
		{
			check_condition = false;
		}

	return make_pair(occur_varset, new_ban_varset);
}

/**
	Initialize the blockid of triples in this group graph pattern.
*/
void GroupPattern::initPatternBlockid()
{
	for (int i = 0; i < (int)this->sub_group_pattern.size(); i++)
		if (this->sub_group_pattern[i].type == SubGroupPattern::Pattern_type)
			this->sub_group_pattern[i].pattern.blockid = i;
}

/**
	Get the blockid of the root triple of the triple indexed by x.
	
	@param x the index of this triple.
	@return the requested blockid.
*/
int GroupPattern::getRootPatternBlockID(int x)
{
	if (this->sub_group_pattern[x].type != SubGroupPattern::Pattern_type)
		throw "GroupPattern::getRootPatternBlockID failed";

	if (this->sub_group_pattern[x].pattern.blockid == x)
		return x;
	this->sub_group_pattern[x].pattern.blockid = this->getRootPatternBlockID(this->sub_group_pattern[x].pattern.blockid);

	return this->sub_group_pattern[x].pattern.blockid;
}

/**
	Merge the two triple blocks containing the triples indexed by x and y.
	
	@param x the index of a triple to merge.
	@param y the index of another triple to merge.
*/
void GroupPattern::mergePatternBlockID(int x, int y)
{
	int px = this->getRootPatternBlockID(x);
	int py = this->getRootPatternBlockID(y);
	this->sub_group_pattern[px].pattern.blockid = py;
}

/**
	Print this group graph pattern.
	
	@param dep the depth of this group graph pattern.
*/
void GroupPattern::print(int dep)
{
	std::string code_print;
	for (int t = 0; t < dep; t++)	code_print += "\t";
	SLOG_CODE(code_print << "{");

	for (int i = 0; i < (int)this->sub_group_pattern.size(); i++)
		if (sub_group_pattern[i].type == SubGroupPattern::Group_type)
			sub_group_pattern[i].group_pattern.print(dep + 1);
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Pattern_type)
		{
			code_print = "";
			for (int t = 0; t <= dep; t++)	code_print += "\t";
			SLOG_CODE(code_print<<this->sub_group_pattern[i].pattern.subject.value
				<<"\t"<<this->sub_group_pattern[i].pattern.predicate.value
				<<"\t"<<this->sub_group_pattern[i].pattern.object.value);
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Union_type)
		{
			for (int j = 0; j < (int)this->sub_group_pattern[i].unions.size(); j++)
			{
				if (j != 0)
				{
					code_print = "";
					for (int t = 0; t <= dep; t++)	code_print += "\t";
					SLOG_CODE("UNION");
				}
				this->sub_group_pattern[i].unions[j].print(dep + 1);
			}
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Optional_type || this->sub_group_pattern[i].type == SubGroupPattern::Minus_type)
		{
			code_print = "";
			for (int t = 0; t <= dep; t++)	code_print += "\t";
			if (this->sub_group_pattern[i].type == SubGroupPattern::Optional_type)	SLOG_CODE(code_print<<"OPTIONAL");
			if (this->sub_group_pattern[i].type == SubGroupPattern::Minus_type)	SLOG_CODE(code_print<<"MINUS");
			this->sub_group_pattern[i].optional.print(dep + 1);
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Filter_type)
		{
			code_print = "";
			for (int t = 0; t <= dep; t++)	code_print += "\t";
			SLOG_CODE(code_print<<"FILTER\t");
			this->sub_group_pattern[i].filter.print(dep + 1);
			SLOG_CODE("");
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Bind_type)
		{
			code_print = "";
			for (int t = 0; t <= dep; t++)	code_print += "\t";
			SLOG_CODE(code_print<<"BIND(");
			this->sub_group_pattern[i].bind.bindExpr.print(dep + 1);
			SLOG_CODE("AS\t"<<this->sub_group_pattern[i].bind.var);
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Subquery_type)
		{
			code_print = "";
			for (int t = 0; t <= dep; t++)	code_print += "\t";
			SLOG_CODE(code_print);
            this->sub_group_pattern[i].subquery.getResultProjectionVarset().print();
            SLOG_CODE("<<");
			this->sub_group_pattern[i].subquery.print();
            SLOG_CODE(">>");
		}

	code_print = "";
	for (int t = 0; t < dep; t++)	code_print += "\t";
	SLOG_CODE("}");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

/**
	Set the query's form (SELECT or ASK).
	
	@param _queryform the query's form.
*/
void QueryTree::setQueryForm(QueryForm _queryform)
{
	this->query_form = _queryform;
}

/**
	Get the query's form (SELECT or ASK).
	
	@return the query's form.
*/
QueryTree::QueryForm QueryTree::getQueryForm()
{
	return this->query_form;
}

/**
	Set the query's projection modifier (REDUCED or DISTINCT).
	
	@param _projection_modifier the query's projection modifier.
*/
void QueryTree::setProjectionModifier(ProjectionModifier _projection_modifier)
{
	this->projection_modifier = _projection_modifier;
}

/**
	Get the query's projection modifier (REDUCED or DISTINCT).
	
	@return the query's projection modifier.
*/
QueryTree::ProjectionModifier QueryTree::getProjectionModifier()
{
	return this->projection_modifier;
}

/**
	Add a SELECT variable.
*/
void QueryTree::addProjectionVar()
{
	this->projection.push_back(ProjectionVar());
}

/**
	Get the last SELECT variable.
	
	@return the last SELECT variable.
*/
ProjectionVar& QueryTree::getLastProjectionVar()
{
	return this->projection.back();
}

/**
	Get the vector of all SELECT variables.
	
	@return the vector of all SELECT variables.
*/
vector<ProjectionVar>& QueryTree::getProjection()
{
	return this->projection;
}

/**
	Get the varset of all SELECT variables.
	In the case of expressionAsVar, get the variable inside the aggregate function.
	
	@return the varset of all SELECT variables.
*/
Varset QueryTree::getProjectionVarset()
{
	Varset varset;

	for (int i = 0; i < (int)this->projection.size(); i++)
		varset.addVar(this->projection[i].var);

	return varset;
}

/**
	Get the varset of all SELECT variables.
	In the case of expressionAsVar, get the variable following AS.
	
	@return the varset of all SELECT variables.
*/
Varset QueryTree::getResultProjectionVarset()
{
	Varset varset;

	for (int i = 0; i < (int)this->projection.size(); i++) {
		if (this->projection[i].aggregate_type == ProjectionVar::None_type)
			varset.addVar(this->projection[i].var);
		else {
			if (this->projection[i].aggregate_var != "*")
				varset += this->projection[i].comp_tree_root.getVarset();
			varset += this->projection[i].var;
		}
	}

	return varset;
}

/**
	Mark the query as SELECT *.
*/
void QueryTree::setProjectionAsterisk()
{
	this->projection_asterisk = true;
}

/**
	Check if the query is SELECT *.
*/
bool QueryTree::checkProjectionAsterisk()
{
	return this->projection_asterisk;
}

/**
	Add a GROUP BY variable.
	
	@param _var the GROUP BY variable to add.
*/
void QueryTree::addGroupByVar(string &_var)
{
	this->group_by.addVar(_var);
}

/**
	Get the varset of all GROUP BY variables.
	
	@return the varset of all GROUP BY variables.
*/
Varset& QueryTree::getGroupByVarset()
{
	return this->group_by;
}

/**
	The constructor of class Order.
	@param _descending the boolean indicating whether the order should be descending.
*/
Order::Order(bool _descending)
{
	descending = _descending;
	// comp_tree_root = new CompTreeNode();
}

/**
	The copy constructor of class Order.
	@param that the object to copy from.
*/
// Order::Order(const Order& that)
// {
// 	var = that.var;
// 	comp_tree_root = new CompTreeNode();
// 	*comp_tree_root = *(that.comp_tree_root);
// 	descending = that.descending;
// }

/**
	The copy assignment operator of class Order.
	@param that the object to copy from.
*/
// Order& Order::operator=(const Order& that)
// {
// 	CompTreeNode *local_root = new CompTreeNode();
// 	// If the above statement throws,
//     // the object is still in the same state as before.
//     // None of the following statements will throw an exception :)
//     *local_root = *(that.comp_tree_root);
//     delete comp_tree_root;
//     comp_tree_root = local_root;
//     var = that.var;
//     descending = that.descending;
//     return *this;
// }

/**
	The destructor of class Order.
*/
// Order::~Order()
// {
// 	delete comp_tree_root;
// }

/**
	Add a ORDER BY variable, and mark if it is DESC or ASC.
	
	@param _var the ORDER BY variable to add. [ARCHAIC: now use CompTreeNode to incorporate expressions]
	@param _descending the boolean indicating whether the order should be descending.
*/
// void QueryTree::addOrderVar(string &_var, bool _descending)
// {
// 	this->order_by.push_back(Order(_var, _descending));
// }
void QueryTree::addOrderVar(bool _descending)
{
	this->order_by.push_back(Order(_descending));
}

/**
	Get the vector of all ORDER BY variables.
	
	@return the vector of all ORDER BY variables.
*/
vector<Order>& QueryTree::getOrderVarVector()
{
	return this->order_by;
}

Order& QueryTree::getLastOrderVar()
{
	return order_by.back();
}

/**
	Get the varset of all ORDER BY variables.
	
	@return the varset of all ORDER BY variables.
*/
Varset QueryTree::getOrderByVarset()
{
	Varset varset;

	for (int i = 0; i < (int)this->order_by.size(); i++)
	{
		// if (this->order_by[i].var != "")
		// 	varset.addVar(this->order_by[i].var);
		varset += order_by[i].comp_tree_root.getVarset();
	}

	return varset;
}

/**
	Set OFFSET number of query.
	
	@param _offset OFFSET number of query.
*/
void QueryTree::setOffset(int _offset)
{
	this->offset = _offset;
}

/**
	Get OFFSET number of query.
	
	@return OFFSET number of query.
*/
int QueryTree::getOffset()
{
	return this->offset;
}

/**
	Set LIMIT number of query.
	
	@param _offset LIMIT number of query.
*/
void QueryTree::setLimit(int _limit)
{
	this->limit = _limit;
}

/**
	Get LIMIT number of query.
	
	@return LIMIT number of query.
*/
int QueryTree::getLimit()
{
	return this->limit;
}

/**
	Get the outermost group graph pattern of this query.
	
	@return the outermost group graph pattern of this query.
*/
GroupPattern& QueryTree::getGroupPattern()
{
	return this->group_pattern;
}

std::unordered_map<std::string, QueryTree::VarType>& QueryTree::getVar2Type()
{
	return this->var2type;
}

/**
	Set the update type of this query.
*/
void QueryTree::setUpdateType(UpdateType _updatetype)
{
	this->update_type = _updatetype;
}

/**
	Get the update type of this query.
	
	@return the update type of this query.
*/
QueryTree::UpdateType QueryTree::getUpdateType()
{
	return this->update_type;
}

/**
	Get the insert patterns of this query.
	
	@return the insert patterns of this query.
*/
GroupPattern& QueryTree::getInsertPatterns()
{
	return this->insert_patterns;
}

/**
	Get the delete patterns of this query.
	
	@return the delete patterns of this query.
*/
GroupPattern& QueryTree::getDeletePatterns()
{
	return this->delete_patterns;
}

/**
	Check whether the query is well-designed. If so, can use rewriting-based
	query evaluation.
	
	@return true if the query is well-designed, false otherwise.
*/
bool QueryTree::checkWellDesigned()
{
	bool check_condition = true;
	this->group_pattern.checkNoMinusAndOptionalVarAndSafeFilter(Varset(), Varset(), check_condition);
	for (auto proj : projection)
	{
		// TODO: reconsider these
		if (proj.aggregate_type == ProjectionVar::cyclePath_type
			|| proj.aggregate_type == ProjectionVar::cycleBoolean_type
			|| proj.aggregate_type == ProjectionVar::shortestPath_type
			|| proj.aggregate_type == ProjectionVar::shortestPathLen_type
			|| proj.aggregate_type == ProjectionVar::kHopReachable_type
			|| proj.aggregate_type == ProjectionVar::kHopEnumerate_type
			|| proj.aggregate_type == ProjectionVar::CompTree_type)
		{
			check_condition = false;
			break;
		}
	}
	// BIND must respect original order
	// Kleene closure respects order for pruning (not guaranteed to be optimal)
	for (size_t i = 0; i < group_pattern.sub_group_pattern.size(); i++)
	{
		if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Bind_type \
		|| (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Pattern_type \
			&& group_pattern.sub_group_pattern[i].pattern.kleene))
		{
			check_condition = false;
			break;
		}
	}
	return check_condition;
}

/**
	Check whether there is at least one aggregate function in the SELECT clause.
	
	@return true if there is at least one aggregate function, false otherwise.
*/
bool QueryTree::checkAtLeastOneAggregateFunction()
{
	// CompTrees are not aggregates
	for (int i = 0; i < (int)this->projection.size(); i++)
		if (this->projection[i].aggregate_type != ProjectionVar::None_type \
			&& this->projection[i].aggregate_type != ProjectionVar::CompTree_type)
			return true;

	return false;
}

bool QueryTree::checkAtLeastOneCompTree()
{
	for (int i = 0; i < (int)this->projection.size(); i++)
		if (this->projection[i].aggregate_type == ProjectionVar::CompTree_type)
			return true;

	return false;
}

/**
	Check whether aggregates in the SELECT clause and GROUP BY variables match.
	
	@return true if aggregates in the SELECT clause and GROUP BY variables match,
	false otherwise.
*/
bool QueryTree::checkSelectAggregateFunctionGroupByValid()
{
	if (this->checkAtLeastOneAggregateFunction() && this->group_by.empty())
	{
		for (int i = 0; i < (int)this->projection.size(); i++)
			if (this->projection[i].aggregate_type == ProjectionVar::None_type)
				return false;
	}

	if (!this->group_by.empty())
	{
		for (int i = 0; i < (int)this->projection.size(); i++)
			if (this->projection[i].aggregate_type == ProjectionVar::None_type && !this->group_by.findVar(this->projection[i].var))
				return false;
	}

	return true;
}

/**
	Print QueryTree.
*/
void QueryTree::print()
{
	std::string code_print;
	for (int j = 0; j < 80; j++)			code_print += "=";
	SLOG_CODE(code_print);

	if (this->update_type == Not_Update)
	{
		if (this->query_form == Select_Query)
		{
			if (singleBGP)
				SLOG_CODE("Single BGP");
			code_print = "SELECT";
			if (this->projection_modifier == Modifier_Distinct)
				code_print += " DISTINCT";
			SLOG_CODE(code_print);

			code_print = "Var: \t";
			for (int i = 0; i < (int)this->projection.size(); i++)
			{
				if (this->projection[i].aggregate_type == ProjectionVar::None_type)
					code_print += this->projection[i].var + "\t";
				else
				{
					code_print +="(";
					if (this->projection[i].aggregate_type == ProjectionVar::Count_type)
						code_print += "COUNT(";
					if (this->projection[i].aggregate_type == ProjectionVar::Sum_type)
						code_print += "SUM(";
					if (this->projection[i].aggregate_type == ProjectionVar::Min_type)
						code_print += "MIN(";
					if (this->projection[i].aggregate_type == ProjectionVar::Max_type)
						code_print += "MAX(";
					if (this->projection[i].aggregate_type == ProjectionVar::Avg_type)
						code_print += "AVG(";
					if (this->projection[i].aggregate_type == ProjectionVar::Sample_type)
						code_print += "SAMPLE(";
					if (this->projection[i].aggregate_type == ProjectionVar::simpleCyclePath_type)
						code_print += "simpleCyclePath(";
					if (this->projection[i].aggregate_type == ProjectionVar::simpleCycleBoolean_type)
						code_print += "simpleCycleBoolean(";
					if (this->projection[i].aggregate_type == ProjectionVar::cyclePath_type)
						code_print += "cyclePath(";
					if (this->projection[i].aggregate_type == ProjectionVar::cycleBoolean_type)
						code_print += "cycleBoolean(";
					if (this->projection[i].aggregate_type == ProjectionVar::shortestPath_type)
						code_print += "shortestPath(";
					if (this->projection[i].aggregate_type == ProjectionVar::shortestPathLen_type)
						code_print += "shortestPathLen(";
					if (this->projection[i].aggregate_type == ProjectionVar::kHopReachable_type)
						code_print += "kHopReachable(";
					if (this->projection[i].aggregate_type == ProjectionVar::kHopEnumerate_type)
						code_print += "kHopEnumerate(";
					if (this->projection[i].aggregate_type == ProjectionVar::CompTree_type)
					{
						SLOG_CODE(code_print);
						code_print = "";
						projection[i].comp_tree_root.print(0);
					}
					
					if (this->projection[i].distinct)
						code_print += "DISTINCT ";

					if (this->projection[i].aggregate_type == ProjectionVar::Count_type
						|| this->projection[i].aggregate_type == ProjectionVar::Sum_type
						|| this->projection[i].aggregate_type == ProjectionVar::Min_type
						|| this->projection[i].aggregate_type == ProjectionVar::Max_type
						|| this->projection[i].aggregate_type == ProjectionVar::Avg_type
						|| this->projection[i].aggregate_type == ProjectionVar::Sample_type)
						code_print += this->projection[i].aggregate_var;
					else
					{
						code_print += this->projection[i].path_args.src;
						code_print += this->projection[i].path_args.dst;
						if (this->projection[i].aggregate_type == ProjectionVar::simpleCyclePath_type
							|| this->projection[i].aggregate_type == ProjectionVar::simpleCycleBoolean_type
							|| this->projection[i].aggregate_type == ProjectionVar::cyclePath_type
							|| this->projection[i].aggregate_type == ProjectionVar::cycleBoolean_type)
						{
							if (this->projection[i].path_args.directed)
								code_print += "true, ";
							else
								code_print += "false, ";
						}
						else if (this->projection[i].aggregate_type == ProjectionVar::kHopReachable_type
							|| this->projection[i].aggregate_type == ProjectionVar::kHopEnumerate_type)
							code_print += std::to_string(this->projection[i].path_args.k);
						code_print += "{";
						for (unsigned j = 0; j < this->projection[i].path_args.pred_set.size(); j++)
						{
							code_print += this->projection[i].path_args.pred_set[j];
							if (j != this->projection[i].path_args.pred_set.size() - 1)
								code_print += ", ";
						}
						code_print += "}";
						if (this->projection[i].aggregate_type == ProjectionVar::kHopReachable_type
							|| this->projection[i].aggregate_type == ProjectionVar::kHopEnumerate_type)
							code_print += std::to_string(this->projection[i].path_args.confidence);
					}

					code_print += ") AS " + this->projection[i].var+ ")\t";
				}
			}
			if (this->projection_asterisk && !this->checkAtLeastOneAggregateFunction())
				code_print += "*";
			SLOG_CODE(code_print);
			code_print = "";
		}
		else SLOG_CODE("ASK");

		SLOG_CODE("GroupPattern:");
		this->group_pattern.print(0);

		if (this->query_form == Select_Query)
		{
			if (!this->group_by.empty())
			{
				code_print = "GROUP BY\t";

				for (int i = 0; i < (int)this->group_by.vars.size(); i++)
					code_print += this->group_by.vars[i] + "\t";

				SLOG_CODE(code_print);
			}

			if (!this->order_by.empty())
			{
				code_print = "ORDER BY\t";

				for (int i = 0; i < (int)this->order_by.size(); i++)
				{
					if (!this->order_by[i].descending)	code_print += "ASC(";
					else code_print += "DESC(";
					// printf("%s)\t", this->order_by[i].var.c_str());
					SLOG_CODE(code_print);
					code_print = "";
					order_by[i].comp_tree_root.print(0);
					code_print = ")\t";
				}
				SLOG_CODE(code_print);
			}
			if (this->offset != 0)
				SLOG_CODE("OFFSET\t" << this->offset);
			if (this->limit != -1)
				SLOG_CODE("LIMIT\t" << this->limit);
		}
	}
	else
	{
		SLOG_CODE("UPDATE");
		if (this->update_type == Delete_Data || this->update_type == Delete_Where ||
				this->update_type == Delete_Clause || this->update_type == Modify_Clause)
		{
			SLOG_CODE("Delete:");
			this->delete_patterns.print(0);
		}
		if (this->update_type == Insert_Data || this->update_type == Insert_Clause || this->update_type == Modify_Clause)
		{
			SLOG_CODE("Insert:");
			this->insert_patterns.print(0);
		}
		if (this->update_type == Delete_Where || this->update_type == Insert_Clause ||
				this->update_type == Delete_Clause || this->update_type == Modify_Clause)
		{
			SLOG_CODE("GroupPattern:");
			this->group_pattern.print(0);
		}
	}

	code_print = "";
	for (int j = 0; j < 80; j++)			code_print +="=";
	SLOG_CODE(code_print);
}

/**
	The constructor of class CompTreeNode.
*/
// CompTreeNode::CompTreeNode()
// {
// 	lchild = NULL;
// 	rchild = NULL;
// }

/**
	The copy constructor of class CompTreeNode.
	@param that the object to copy from.
*/
// CompTreeNode::CompTreeNode(const CompTreeNode& that)
// {
// 	oprt = that.oprt;
// 	if (that.lchild)
// 	{
// 		lchild = new CompTreeNode();
// 		*lchild = *(that.lchild);
// 	}
// 	else
// 		lchild = NULL;
// 	if (that.rchild)
// 	{
// 		rchild = new CompTreeNode();
// 		*rchild = *(that.rchild);
// 	}
// 	else
// 		rchild = NULL;
// 	val = that.val;
// }

/**
	The copy assignment operator of class CompTreeNode.
	@param that the object to copy from.
*/
// CompTreeNode& CompTreeNode::operator=(const CompTreeNode& that)
// {
// 	if (that.lchild)
// 	{
// 		CompTreeNode *local_lchild = new CompTreeNode();
// 	    *local_lchild = *(that.lchild);
// 	    if (lchild)
// 	    	delete lchild;
// 	    lchild = local_lchild;
// 	}
// 	else if (lchild)
// 	{
// 		delete lchild;
// 		lchild = NULL;
// 	}
// 	if (that.rchild)
// 	{
// 		CompTreeNode *local_rchild = new CompTreeNode();
// 	    *local_rchild = *(that.rchild);
// 	    if (rchild)
// 	    	delete rchild;
// 	    rchild = local_rchild;
// 	}
// 	else if (rchild)
// 	{
// 		delete rchild;
// 		rchild = NULL;
// 	}
//     oprt = that.oprt;
//     val = that.val;
//     return *this;
// }

/**
	The destructor of class CompTreeNode.
*/
// CompTreeNode::~CompTreeNode()
// {
// 	if (lchild)
// 		delete lchild;
// 	if (rchild)
// 		delete rchild;
// }

void CompTreeNode::print(int dep)
{
	if (children.empty())
	{
		std::string code_print;
		for (int i = 0; i < dep; i++)
			code_print += '\t';
		SLOG_CODE(code_print<<"Value: " << val);
	}
	else
	{
		std::string code_print;
		for (int i = 0; i < dep; i++)
			code_print += '\t';
		SLOG_CODE(code_print<<"Operator " << oprt);
		for (size_t i = 0; i < children.size(); i++)
		{
			code_print = "";
			for (int j = 0; j < dep; j++)
				code_print += '\t';
			SLOG_CODE(code_print<<"child[" << i << "]:");
			children[i].print(dep + 1);
		}
	}
}

Varset CompTreeNode::getVarset()
{
	if (children.size() == 0)
	{
		if (val[0] == '?')
			return Varset(val);
		else
			return Varset();
	}
	else
	{
		Varset ret = children[0].getVarset();
		for (size_t i = 1; i < children.size(); i++)
			ret += children[i].getVarset();
		return ret;
	}
}

void QueryTree::setSingleBGP(bool val)
{
	singleBGP = val;
}

bool QueryTree::getSingleBGP()
{
	return singleBGP;
}

void QueryTree::relabel(QueryTreeRelabeler& qtr){
    for(vector<ProjectionVar>::iterator it=projection.begin(); it!=projection.end(); it++)
		it->relabel(qtr);	// Do nothing
    qtr.relabel(group_by);
    for(vector<Order>::iterator it=order_by.begin(); it!=order_by.end(); it++)
		it->relabel(qtr);
    group_pattern.relabel(qtr);
	SLOG_CODE("");
}

void QueryTree::relabel_full(QueryTreeRelabeler& qtr){
    for(vector<ProjectionVar>::iterator it=projection.begin(); it!=projection.end(); it++) it->relabel_full(qtr);
    qtr.relabel(group_by);
    for(vector<Order>::iterator it=order_by.begin(); it!=order_by.end(); it++) it->relabel(qtr);
    group_pattern.relabel(qtr);
}

void GroupPattern::relabel(QueryTreeRelabeler& qtr){
    qtr.relabel(group_pattern_resultset_minimal_varset);
    qtr.relabel(group_pattern_resultset_maximal_varset);
    qtr.relabel(group_pattern_subject_object_maximal_varset);
    qtr.relabel(group_pattern_predicate_maximal_varset);
    for(vector<SubGroupPattern>::iterator it=sub_group_pattern.begin(); it!=sub_group_pattern.end(); it++) it->relabel(qtr);
}

void GroupPattern::SubGroupPattern::relabel(QueryTreeRelabeler& qtr){
    if(type==Group_type) group_pattern.relabel(qtr);
    else if(type==Pattern_type) pattern.relabel(qtr);
    else if(type==Union_type) for(vector<GroupPattern>::iterator it=unions.begin(); it!=unions.end(); it++) it->relabel(qtr);
    else if(type==Optional_type||type==Minus_type) optional.relabel(qtr);
    else if(type==Filter_type) filter.relabel(qtr);
    else if(type==Bind_type) bind.relabel(qtr);
    else if(type==Subquery_type) {
        QueryTreeRelabeler qtr1(qtr.suffix);
		qtr1.excluding = qtr.excluding;
        subquery.relabel_full(qtr1);
    }
}
