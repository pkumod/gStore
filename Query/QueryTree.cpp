/*=============================================================================
# Filename: QueryTree.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-03-13
# Description: implement functions in QueryTree.h
=============================================================================*/

#include "QueryTree.h"

using namespace std;

QueryTree::GroupPattern::GroupPattern(const QueryTree::GroupPattern& that)
{
	sub_group_pattern = that.sub_group_pattern;
	group_pattern_resultset_minimal_varset = that.group_pattern_resultset_minimal_varset;
	group_pattern_resultset_maximal_varset = that.group_pattern_resultset_maximal_varset;
	group_pattern_subject_object_maximal_varset = that.group_pattern_subject_object_maximal_varset;
	group_pattern_predicate_maximal_varset = that.group_pattern_predicate_maximal_varset;
}

QueryTree::GroupPattern& QueryTree::GroupPattern::operator=(const QueryTree::GroupPattern& that)
{
	sub_group_pattern = that.sub_group_pattern;
	group_pattern_resultset_minimal_varset = that.group_pattern_resultset_minimal_varset;
	group_pattern_resultset_maximal_varset = that.group_pattern_resultset_maximal_varset;
	group_pattern_subject_object_maximal_varset = that.group_pattern_subject_object_maximal_varset;
	group_pattern_predicate_maximal_varset = that.group_pattern_predicate_maximal_varset;

	return *this;
}

/**
	Get varset of the sub-FilterTree rooted at this FilterTreeNode.

	@param varset the output varset.
*/
void QueryTree::GroupPattern::FilterTree::FilterTreeNode::getVarset(Varset &varset)
{
	for (int i = 0; i < (int)this->child.size(); i++)
	{
		if (this->child[i].node_type == FilterTreeChild::String_type && this->child[i].str[0] == '?')
			varset.addVar(this->child[i].str);
		if (this->child[i].node_type == FilterTreeChild::Tree_type)
			this->child[i].node.getVarset(varset);
	}
}

/**
	Set pos and isel of each leaf node of the sub-FilterTree rooted at this FilterTreeNode.
	pos is the position of the leaf node's variable in the parameter varset. isel indicates
	whether the leaf node's variable is in the variable entity_literal_varset.

	@param varset the query's complete varset.
	@param entity_literal_varset the varset that includes all subjects and objects in the query.
*/
void QueryTree::GroupPattern::FilterTree::FilterTreeNode::mapVarPos2Varset(Varset &varset, Varset &entity_literal_varset)
{
	if (this->oper_type == Not_type)
	{
		this->child[0].node.mapVarPos2Varset(varset, entity_literal_varset);
	}
	else if (this->oper_type == Or_type || this->oper_type == And_type)
	{
		this->child[0].node.mapVarPos2Varset(varset, entity_literal_varset);
		this->child[1].node.mapVarPos2Varset(varset, entity_literal_varset);
	}
	else if (Equal_type <= this->oper_type && this->oper_type <= GreaterOrEqual_type)
	{
		if (this->child[0].node_type == FilterTreeChild::Tree_type)
			this->child[0].node.mapVarPos2Varset(varset, entity_literal_varset);
		else if (this->child[0].node_type == FilterTreeChild::String_type && this->child[0].str[0] == '?')
		{
			this->child[0].pos = Varset(this->child[0].str).mapTo(varset)[0];
			this->child[0].isel = entity_literal_varset.findVar(this->child[0].str);
		}

		if (this->child[1].node_type == FilterTreeChild::Tree_type)
			this->child[1].node.mapVarPos2Varset(varset, entity_literal_varset);
		else if (this->child[1].node_type == FilterTreeChild::String_type && this->child[1].str[0] == '?')
		{
			this->child[1].pos = Varset(this->child[1].str).mapTo(varset)[0];
			this->child[1].isel = entity_literal_varset.findVar(this->child[1].str);
		}
	}
	else if (this->oper_type == Builtin_regex_type ||
			 this->oper_type == Builtin_str_type ||
			 this->oper_type == Builtin_isiri_type ||
			 this->oper_type == Builtin_isuri_type ||
			 this->oper_type == Builtin_isliteral_type ||
			 this->oper_type == Builtin_isnumeric_type ||
			 this->oper_type == Builtin_lang_type ||
			 this->oper_type == Builtin_langmatches_type ||
			 this->oper_type == Builtin_bound_type ||
			 this->oper_type == Builtin_in_type ||
			 this->oper_type == Builtin_datatype_type ||
			 this->oper_type == Builtin_ucase_type ||
			 this->oper_type == Builtin_lcase_type ||
			 this->oper_type == Builtin_year_type ||
			 this->oper_type == Builtin_month_type ||
			 this->oper_type == Builtin_day_type ||
			 this->oper_type == Builtin_abs_type)
	{
		if (this->child[0].node_type == FilterTreeChild::Tree_type)
			this->child[0].node.mapVarPos2Varset(varset, entity_literal_varset);
		else if (this->child[0].node_type == FilterTreeChild::String_type && this->child[0].str[0] == '?')
		{
			this->child[0].pos = Varset(this->child[0].str).mapTo(varset)[0];
			this->child[0].isel = entity_literal_varset.findVar(this->child[0].str);
		}
	}
	else if (this->oper_type == Builtin_contains_type || this->oper_type == Builtin_strstarts_type)
	{
		if (this->child[0].node_type == FilterTreeChild::Tree_type)
			this->child[0].node.mapVarPos2Varset(varset, entity_literal_varset);
		else if (this->child[0].node_type == FilterTreeChild::String_type && this->child[0].str[0] == '?')
		{
			this->child[0].pos = Varset(this->child[0].str).mapTo(varset)[0];
			this->child[0].isel = entity_literal_varset.findVar(this->child[0].str);
		}

		if (this->child[1].node_type == FilterTreeChild::Tree_type)
			this->child[1].node.mapVarPos2Varset(varset, entity_literal_varset);
		else if (this->child[1].node_type == FilterTreeChild::String_type && this->child[1].str[0] == '?')
		{
			this->child[1].pos = Varset(this->child[1].str).mapTo(varset)[0];
			this->child[1].isel = entity_literal_varset.findVar(this->child[1].str);
		}
	}
}

/**
	Print the sub-FilterTree rooted at this FilterTreeNode.

	@param dep the depth of this FilterTreeNode.
*/
void QueryTree::GroupPattern::FilterTree::FilterTreeNode::print(int dep)
{
	if (this->oper_type == Not_type)					printf("!");
	if (this->oper_type == Builtin_regex_type)			printf("REGEX");
	if (this->oper_type == Builtin_str_type)			printf("STR");
	if (this->oper_type == Builtin_isiri_type)			printf("ISIRI");
	if (this->oper_type == Builtin_isuri_type)			printf("ISURI");
	if (this->oper_type == Builtin_isliteral_type)		printf("ISLITERAL");
	if (this->oper_type == Builtin_isnumeric_type)		printf("ISNUMERIC");
	if (this->oper_type == Builtin_lang_type)			printf("LANG");
	if (this->oper_type == Builtin_langmatches_type)	printf("LANGMATCHES");
	if (this->oper_type == Builtin_bound_type)			printf("BOUND");
	if (this->oper_type == Builtin_simpleCycle_type)	printf("simpleCycleBoolean");
	if (this->oper_type == Builtin_cycle_type)			printf("cycleBoolean");
	if (this->oper_type == Builtin_sp_type)				printf("shortestPathLen");
	if (this->oper_type == Builtin_khop_type)			printf("kHopReachable");
	if (this->oper_type == Builtin_datatype_type)		printf("DATATYPE");
	if (this->oper_type == Builtin_contains_type)		printf("CONTAINS");
	if (this->oper_type == Builtin_ucase_type)			printf("UCASE");
	if (this->oper_type == Builtin_lcase_type)			printf("LCASE");
	if (this->oper_type == Builtin_strstarts_type)		printf("STRSTARTS");
	if (this->oper_type == Builtin_now_type)			printf("NOW");
	if (this->oper_type == Builtin_year_type)			printf("YEAR");
	if (this->oper_type == Builtin_month_type)    		printf("MONTH");
	if (this->oper_type == Builtin_day_type)			printf("DAY");
	if (this->oper_type == Builtin_abs_type)			printf("ABS");

	if (this->oper_type == Builtin_in_type)
	{
		printf("(");
		if (this->child[0].node_type == FilterTreeChild::String_type)	printf("%s", this->child[0].str.c_str());
		if (this->child[0].node_type == FilterTreeChild::Tree_type)		this->child[0].node.print(dep);
		printf(" IN (");
		for (int i = 1; i < (int)this->child.size(); i++)
		{
			if (i != 1)	printf(", ");
			if (this->child[i].node_type == FilterTreeChild::String_type)	printf("%s", this->child[i].str.c_str());
			if (this->child[i].node_type == FilterTreeChild::Tree_type)		this->child[i].node.print(dep);
		}
		printf("))");

		return;
	}

	if (this->oper_type == Builtin_simpleCycle_type || this->oper_type == Builtin_cycle_type
		|| this->oper_type == Builtin_sp_type || this->oper_type == Builtin_khop_type)
	{
		printf("(");

		printf("%s, ", this->child[0].path_args.src.c_str());
		printf("%s, ", this->child[0].path_args.dst.c_str());
		if (this->oper_type == Builtin_simpleCycle_type || this->oper_type == Builtin_cycle_type)
		{
			if (this->child[0].path_args.directed)
				printf("true, ");
			else
				printf("false, ");
		}
		if (this->oper_type == Builtin_khop_type)
			printf("%d, ", this->child[0].path_args.k);
		printf("{");
		for (int i = 0; i < this->child[0].path_args.pred_set.size(); i++)
		{
			printf("%s", this->child[0].path_args.pred_set[i].c_str());
			if (i != this->child[0].path_args.pred_set.size() - 1)
				printf(", ");
		}
		printf("}");
		if (this->oper_type == Builtin_khop_type)
			printf(", %f", this->child[0].path_args.confidence);

		printf(")");

		return;
	}

	printf("(");

	if ((int)this->child.size() >= 1)
	{
		if (this->child[0].node_type == FilterTreeChild::String_type)	printf("%s", this->child[0].str.c_str());
		if (this->child[0].node_type == FilterTreeChild::Tree_type)		this->child[0].node.print(dep);
	}

	if (this->oper_type == Or_type)				printf(" || ");
	if (this->oper_type == And_type)			printf(" && ");
	if (this->oper_type == Equal_type)			printf(" = ");
	if (this->oper_type == NotEqual_type)		printf(" != ");
	if (this->oper_type == Less_type)			printf(" < ");
	if (this->oper_type == LessOrEqual_type)	printf(" <= ");
	if (this->oper_type == Greater_type)		printf(" > ");
	if (this->oper_type == GreaterOrEqual_type)	printf(" >= ");

	if (this->oper_type == Builtin_regex_type || this->oper_type == Builtin_langmatches_type \
		|| this->oper_type == Builtin_contains_type || this->oper_type == Builtin_strstarts_type)
		printf(", ");

	if ((int)this->child.size() >= 2)
	{
		if (this->child[1].node_type == FilterTreeChild::String_type)	printf("%s", this->child[1].str.c_str());
		if (this->child[1].node_type == FilterTreeChild::Tree_type)		this->child[1].node.print(dep);
	}

	if ((int)this->child.size() >= 3)
	{
		if (this->oper_type == FilterTreeNode::Builtin_regex_type && this->child[2].node_type == FilterTreeChild::String_type)
			printf(", %s", this->child[2].str.c_str());
	}

	printf(")");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

/**
	Add a triple to this group graph pattern.

	@param _pattern the triple to add.
*/
void QueryTree::GroupPattern::addOnePattern(Pattern _pattern)
{
	this->sub_group_pattern.push_back(SubGroupPattern(SubGroupPattern::Pattern_type));
	this->sub_group_pattern.back().pattern = _pattern;
}

/**
	Add a nested group graph pattern to this group graph pattern.
*/
void QueryTree::GroupPattern::addOneGroup()
{
	this->sub_group_pattern.emplace_back(SubGroupPattern(SubGroupPattern::Group_type));
}

/**
	Get the nested group graph pattern at the back of this group graph pattern.
	If the component at the back of this group graph pattern is not a nested group
	graph pattern, throw an exception.

	@return the requested group graph pattern.
*/
QueryTree::GroupPattern& QueryTree::GroupPattern::getLastGroup()
{
	if (sub_group_pattern.size() == 0 \
		|| sub_group_pattern.back().type != SubGroupPattern::Group_type)
		throw "QueryTree::GroupPattern::getLastGroup failed";

	return sub_group_pattern.back().group_pattern;
}

/**
	Add a UNION to this group graph pattern.
*/
void QueryTree::GroupPattern::addOneGroupUnion()
{
	this->sub_group_pattern.push_back(SubGroupPattern(SubGroupPattern::Union_type));
}

/**
	Add a group graph pattern to the UNION at the back of this group graph pattern.
	If the component at the back of this group graph pattern is not a UNION, throw 
	an exception.
*/
void QueryTree::GroupPattern::addOneUnion()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Union_type)
		throw "QueryTree::GroupPattern::addOneUnion failed";

	this->sub_group_pattern.back().unions.push_back(GroupPattern());
}

/**
	Get the group graph pattern at the back of the UNION at the back of this
	group graph pattern.
	If the component at the back of this group graph pattern is not a UNION, throw an 
	exception.

	@return the requested group graph pattern.
*/
QueryTree::GroupPattern& QueryTree::GroupPattern::getLastUnion()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Union_type || this->sub_group_pattern.back().unions.empty())
		throw "QueryTree::GroupPattern::getLastUnion failed";

	return this->sub_group_pattern.back().unions.back();
}

/**
	Add an OPTIONAL or a MINUS to this group graph pattern, according to the parameter _type.
	If _type is neither OPTIONAL nor MINUS, throw an exception.

	@param _type the type of the component to add.
*/
void QueryTree::GroupPattern::addOneOptional(int _type)
{
	SubGroupPattern::SubGroupPatternType type = (SubGroupPattern::SubGroupPatternType)_type;
	if (type != SubGroupPattern::Optional_type && type != SubGroupPattern::Minus_type)
		throw "QueryTree::GroupPattern::addOneOptional failed";

	this->sub_group_pattern.push_back(SubGroupPattern(type));
}

/**
	Get the OPTIONAL or MINUS at the back of this group graph pattern.
	If the component at the back of this group graph pattern is not an OPTIONAL
	or a MINUS, throw an exception.

	@return the requested OPTIONAL or MINUS.
*/
QueryTree::GroupPattern& QueryTree::GroupPattern::getLastOptional()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Optional_type && this->sub_group_pattern.back().type != SubGroupPattern::Minus_type)
		throw "QueryTree::GroupPattern::getLastOptional failed";

	return this->sub_group_pattern.back().optional;
}

/**
	Add a FILTER to this group graph pattern.
*/
void QueryTree::GroupPattern::addOneFilter()
{
	this->sub_group_pattern.push_back(SubGroupPattern(SubGroupPattern::Filter_type));
}

/**
	Get the FILTER at the back of this group graph pattern.
	If the component at the back of this group graph pattern is not a FILTER, throw
	an exception.

	@return the requested FILTER.
*/
// QueryTree::GroupPattern::FilterTree& QueryTree::GroupPattern::getLastFilter()
QueryTree::CompTreeNode& QueryTree::GroupPattern::getLastFilter()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Filter_type)
		throw "QueryTree::GroupPattern::getLastFilter failed";

	return this->sub_group_pattern.back().filter;
}

/**
	Add a BIND to this group graph pattern.
*/
void QueryTree::GroupPattern::addOneBind()
{
	this->sub_group_pattern.push_back(SubGroupPattern(SubGroupPattern::Bind_type));
}

/**
	Get the BIND at the back of this group graph pattern.
	If the component at the back of this group graph pattern is not a BIND, throw
	an exception.

	@return the requested BIND.
*/
QueryTree::GroupPattern::Bind& QueryTree::GroupPattern::getLastBind()
{
	if (this->sub_group_pattern.back().type != SubGroupPattern::Bind_type)
		throw "QueryTree::GroupPattern::getLastBind failed";

	return this->sub_group_pattern.back().bind;
}

/**
	Recursively compute the varset of each component of this group graph pattern.
*/
void QueryTree::GroupPattern::getVarset()
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
}

/**
	If check_condition is initially true, check whether the query is well-designed
	(no nested group graph pattern, no MINUS, no OPTIONAL, safe filter) and store the 
	result in check_condition.

	@param occur_varset ?
	@param ban_varset ?
	@return pair of occur varset and ban varset.
*/
pair<Varset, Varset> QueryTree::GroupPattern::checkNoMinusAndOptionalVarAndSafeFilter(Varset occur_varset, Varset ban_varset, bool &check_condition)
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

	return make_pair(occur_varset, new_ban_varset);
}

/**
	Initialize the blockid of triples in this group graph pattern.
*/
void QueryTree::GroupPattern::initPatternBlockid()
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
int QueryTree::GroupPattern::getRootPatternBlockID(int x)
{
	if (this->sub_group_pattern[x].type != SubGroupPattern::Pattern_type)
		throw "QueryTree::GroupPattern::getRootPatternBlockID failed";

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
void QueryTree::GroupPattern::mergePatternBlockID(int x, int y)
{
	int px = this->getRootPatternBlockID(x);
	int py = this->getRootPatternBlockID(y);
	this->sub_group_pattern[px].pattern.blockid = py;
}

/**
	Print this group graph pattern.
	
	@param dep the depth of this group graph pattern.
*/
void QueryTree::GroupPattern::print(int dep)
{
	for (int t = 0; t < dep; t++)	printf("\t");	printf("{\n");

	for (int i = 0; i < (int)this->sub_group_pattern.size(); i++)
		if (sub_group_pattern[i].type == SubGroupPattern::Group_type)
			sub_group_pattern[i].group_pattern.print(dep + 1);
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Pattern_type)
		{
			for (int t = 0; t <= dep; t++)	printf("\t");
			printf("%s\t%s\t%s.\n",	this->sub_group_pattern[i].pattern.subject.value.c_str(),
									this->sub_group_pattern[i].pattern.predicate.value.c_str(),
									this->sub_group_pattern[i].pattern.object.value.c_str());
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Union_type)
		{
			for (int j = 0; j < (int)this->sub_group_pattern[i].unions.size(); j++)
			{
				if (j != 0)
				{
					for (int t = 0; t <= dep; t++)	printf("\t");	printf("UNION\n");
				}
				this->sub_group_pattern[i].unions[j].print(dep + 1);
			}
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Optional_type || this->sub_group_pattern[i].type == SubGroupPattern::Minus_type)
		{
			for (int t = 0; t <= dep; t++)	printf("\t");
			if (this->sub_group_pattern[i].type == SubGroupPattern::Optional_type)	printf("OPTIONAL\n");
			if (this->sub_group_pattern[i].type == SubGroupPattern::Minus_type)	printf("MINUS\n");
			this->sub_group_pattern[i].optional.print(dep + 1);
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Filter_type)
		{
			for (int t = 0; t <= dep; t++)	printf("\t");	printf("FILTER\t");
			this->sub_group_pattern[i].filter.print(dep + 1);
			printf("\n");
		}
		else if (this->sub_group_pattern[i].type == SubGroupPattern::Bind_type)
		{
			for (int t = 0; t <= dep; t++)	printf("\t");
			printf("BIND(");
			this->sub_group_pattern[i].bind.bindExpr.print(dep + 1);
			printf("AS\t%s)", this->sub_group_pattern[i].bind.var.c_str());
			// printf("BIND(%s\tAS\t%s)", this->sub_group_pattern[i].bind.str.c_str(), this->sub_group_pattern[i].bind.var.c_str());
			printf("\n");
		}
		else
			printf("ERROR in QueryTree::GroupPattern sub_group_pattern element type\n");

	for (int t = 0; t < dep; t++)	printf("\t");	printf("}\n");
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
QueryTree::ProjectionVar& QueryTree::getLastProjectionVar()
{
	return this->projection.back();
}

/**
	Get the vector of all SELECT variables.
	
	@return the vector of all SELECT variables.
*/
vector<QueryTree::ProjectionVar>& QueryTree::getProjection()
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

	for (int i = 0; i < (int)this->projection.size(); i++)
		if (this->projection[i].aggregate_type == ProjectionVar::None_type)
			varset.addVar(this->projection[i].var);
		else if (this->projection[i].aggregate_var != "*")
			varset.addVar(this->projection[i].aggregate_var);

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
QueryTree::Order::Order(bool _descending)
{
	descending = _descending;
	// comp_tree_root = new CompTreeNode();
}

/**
	The copy constructor of class Order.
	@param that the object to copy from.
*/
// QueryTree::Order::Order(const QueryTree::Order& that)
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
// QueryTree::Order& QueryTree::Order::operator=(const QueryTree::Order& that)
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
// QueryTree::Order::~Order()
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
vector<QueryTree::Order>& QueryTree::getOrderVarVector()
{
	return this->order_by;
}

QueryTree::Order& QueryTree::getLastOrderVar()
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
QueryTree::GroupPattern& QueryTree::getGroupPattern()
{
	return this->group_pattern;
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
QueryTree::GroupPattern& QueryTree::getInsertPatterns()
{
	return this->insert_patterns;
}

/**
	Get the delete patterns of this query.
	
	@return the delete patterns of this query.
*/
QueryTree::GroupPattern& QueryTree::getDeletePatterns()
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
	for (int i = 0; i < (int)this->projection.size(); i++)
		if (this->projection[i].aggregate_type != ProjectionVar::None_type)
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
	for (int j = 0; j < 80; j++)			printf("=");	printf("\n");

	if (this->update_type == Not_Update)
	{
		if (this->query_form == Select_Query)
		{
			if (singleBGP)
				printf("Single BGP\n");
			printf("SELECT");
			if (this->projection_modifier == Modifier_Distinct)
				printf(" DISTINCT");
			printf("\n");

			printf("Var: \t");
			for (int i = 0; i < (int)this->projection.size(); i++)
			{
				if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::None_type)
					printf("%s\t", this->projection[i].var.c_str());
				else
				{
					printf("(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::Count_type)
						printf("COUNT(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::Sum_type)
						printf("SUM(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::Min_type)
						printf("MIN(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
						printf("MAX(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
						printf("AVG(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::simpleCyclePath_type)
						printf("simpleCyclePath(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::simpleCycleBoolean_type)
						printf("simpleCycleBoolean(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::cyclePath_type)
						printf("cyclePath(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::cycleBoolean_type)
						printf("cycleBoolean(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::shortestPath_type)
						printf("shortestPath(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::shortestPathLen_type)
						printf("shortestPathLen(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::kHopReachable_type)
						printf("kHopReachable(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::kHopEnumerate_type)
						printf("kHopEnumerate(");
					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::CompTree_type)
					{
						cout << endl;
						projection[i].comp_tree_root.print(0);
					}
					
					if (this->projection[i].distinct)
						printf("DISTINCT ");

					if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::Count_type
						|| this->projection[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
						|| this->projection[i].aggregate_type == QueryTree::ProjectionVar::Min_type
						|| this->projection[i].aggregate_type == QueryTree::ProjectionVar::Max_type
						|| this->projection[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
						printf("%s", this->projection[i].aggregate_var.c_str());
					else
					{
						printf("%s, ", this->projection[i].path_args.src.c_str());
						printf("%s, ", this->projection[i].path_args.dst.c_str());
						if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::simpleCyclePath_type
							|| this->projection[i].aggregate_type == QueryTree::ProjectionVar::simpleCycleBoolean_type
							|| this->projection[i].aggregate_type == QueryTree::ProjectionVar::cyclePath_type
							|| this->projection[i].aggregate_type == QueryTree::ProjectionVar::cycleBoolean_type)
						{
							if (this->projection[i].path_args.directed)
								printf("true, ");
							else
								printf("false, ");
						}
						else if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::kHopReachable_type
							|| this->projection[i].aggregate_type == QueryTree::ProjectionVar::kHopEnumerate_type)
							printf("%d, ", this->projection[i].path_args.k);
						printf("{");
						for (int j = 0; j < this->projection[i].path_args.pred_set.size(); j++)
						{
							printf("%s", this->projection[i].path_args.pred_set[j].c_str());
							if (j != this->projection[i].path_args.pred_set.size() - 1)
								printf(", ");
						}
						printf("}");
						if (this->projection[i].aggregate_type == QueryTree::ProjectionVar::kHopReachable_type
							|| this->projection[i].aggregate_type == QueryTree::ProjectionVar::kHopEnumerate_type)
							printf(", %f", this->projection[i].path_args.confidence);
					}

					printf(") AS %s)\t", this->projection[i].var.c_str());
				}
			}
			if (this->projection_asterisk && !this->checkAtLeastOneAggregateFunction())
				printf("*");
			printf("\n");
		}
		else printf("ASK\n");

		printf("GroupPattern:\n");
		this->group_pattern.print(0);

		if (this->query_form == Select_Query)
		{
			if (!this->group_by.empty())
			{
				printf("GROUP BY\t");

				for (int i = 0; i < (int)this->group_by.vars.size(); i++)
					printf("%s\t", this->group_by.vars[i].c_str());

				printf("\n");
			}

			if (!this->order_by.empty())
			{
				printf("ORDER BY\t");

				for (int i = 0; i < (int)this->order_by.size(); i++)
				{
					if (!this->order_by[i].descending)	printf("ASC(");
					else printf("DESC(");
					// printf("%s)\t", this->order_by[i].var.c_str());
					order_by[i].comp_tree_root.print(0);
					printf(")\t");
				}
				printf("\n");
			}
			if (this->offset != 0)
				printf("OFFSET\t%d\n", this->offset);
			if (this->limit != -1)
				printf("LIMIT\t%d\n", this->limit);
		}
	}
	else
	{
		printf("UPDATE\n");
		if (this->update_type == Delete_Data || this->update_type == Delete_Where ||
				this->update_type == Delete_Clause || this->update_type == Modify_Clause)
		{
			printf("Delete:\n");
			this->delete_patterns.print(0);
		}
		if (this->update_type == Insert_Data || this->update_type == Insert_Clause || this->update_type == Modify_Clause)
		{
			printf("Insert:\n");
			this->insert_patterns.print(0);
		}
		if (this->update_type == Delete_Where || this->update_type == Insert_Clause ||
				this->update_type == Delete_Clause || this->update_type == Modify_Clause)
		{
			printf("GroupPattern:\n");
			this->group_pattern.print(0);
		}
	}

	for (int j = 0; j < 80; j++)			printf("=");	printf("\n");
}

/**
	The constructor of class CompTreeNode.
*/
// QueryTree::CompTreeNode::CompTreeNode()
// {
// 	lchild = NULL;
// 	rchild = NULL;
// }

/**
	The copy constructor of class CompTreeNode.
	@param that the object to copy from.
*/
// QueryTree::CompTreeNode::CompTreeNode(const QueryTree::CompTreeNode& that)
// {
// 	oprt = that.oprt;
// 	if (that.lchild)
// 	{
// 		lchild = new QueryTree::CompTreeNode();
// 		*lchild = *(that.lchild);
// 	}
// 	else
// 		lchild = NULL;
// 	if (that.rchild)
// 	{
// 		rchild = new QueryTree::CompTreeNode();
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
// QueryTree::CompTreeNode& QueryTree::CompTreeNode::operator=(const QueryTree::CompTreeNode& that)
// {
// 	if (that.lchild)
// 	{
// 		QueryTree::CompTreeNode *local_lchild = new QueryTree::CompTreeNode();
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
// 		QueryTree::CompTreeNode *local_rchild = new QueryTree::CompTreeNode();
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
// QueryTree::CompTreeNode::~CompTreeNode()
// {
// 	if (lchild)
// 		delete lchild;
// 	if (rchild)
// 		delete rchild;
// }

void QueryTree::CompTreeNode::print(int dep)
{
	if (children.empty())
	{
		for (size_t i = 0; i < dep; i++)
			cout << '\t';
		cout << "Value: " << val << endl;
	}
	else
	{
		for (size_t i = 0; i < dep; i++)
			cout << '\t';
		cout << "Operator " << oprt << endl;
		for (size_t i = 0; i < children.size(); i++)
		{
			for (size_t i = 0; i < dep; i++)
				cout << '\t';
			cout << "child[" << i << "]:" << endl;
			children[i].print(dep + 1);
		}
	}

	// if (!lchild && !rchild)
	// {
	// 	for (int i = 0; i < dep; i++)
	// 		cout << '\t';
	// 	cout << "Value: " << val << endl;
	// }
	// else
	// {
	// 	for (int i = 0; i < dep; i++)
	// 		cout << '\t';
	// 	cout << "Operator " << oprt << endl;
	// 	if (lchild)
	// 	{
	// 		for (int i = 0; i < dep; i++)
	// 			cout << '\t';
	// 		cout << "lchild:" << endl;
	// 		lchild->print(dep + 1);
	// 	}
	// 	if (rchild)
	// 	{
	// 		for (int i = 0; i < dep; i++)
	// 			cout << '\t';
	// 		cout << "rchild:" << endl;
	// 		rchild->print(dep + 1);
	// 	}
	// }
}

Varset QueryTree::CompTreeNode::getVarset()
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

	// if (lchild && rchild)
	// 	return lchild->getVarset() + rchild->getVarset();
	// if (lchild)
	// 	return lchild->getVarset();
	// if (rchild)
	// 	return rchild->getVarset();
	// // !lchild && !rchild
	// if (val[0] == '?')
	// 	return Varset(val);
}

void QueryTree::setSingleBGP(bool val)
{
	singleBGP = val;
}

bool QueryTree::getSingleBGP()
{
	return singleBGP;
}