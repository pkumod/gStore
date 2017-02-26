/*=============================================================================
# Filename: QueryTree.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-07-14
# Description: implement functions in QueryTree.h
=============================================================================*/

#include "QueryTree.h"

using namespace std;

void QueryTree::GroupPattern::FilterTreeNode::getVarset(Varset &varset)
{
	for (int i = 0; i < (int)this->child.size(); i++)
	{
		if (this->child[i].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::String_type && this->child[i].arg[0] == '?')
			varset.addVar(this->child[i].arg);
		if (this->child[i].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::Tree_type)
			this->child[i].node.getVarset(varset);
	}
}

void QueryTree::GroupPattern::FilterTreeNode::print(vector<GroupPattern> &exist_grouppatterns, int dep)
{
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Not_type)	printf("!");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Builtin_regex_type)	printf("REGEX");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Builtin_str_type)	printf("STR");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Builtin_lang_type)		printf("LANG");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Builtin_langmatches_type)		printf("LANGMATCHES");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Builtin_bound_type)		printf("BOUND");

	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Builtin_in_type)
	{
		printf("(");
		if (this->child[0].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::String_type)	printf("%s", this->child[0].arg.c_str());
		if (this->child[0].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::Tree_type)	this->child[0].node.print(exist_grouppatterns, dep);
		printf(" IN (");
		for (int i = 1; i < (int)this->child.size(); i++)
		{
			if (i != 1)	printf(" , ");
			if (this->child[i].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::String_type)	printf("%s", this->child[i].arg.c_str());
			if (this->child[i].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::Tree_type)	this->child[i].node.print(exist_grouppatterns, dep);
		}
		printf("))");

		return;
	}

	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Builtin_exists_type)
	{
		printf("EXISTS");
		exist_grouppatterns[this->exists_grouppattern_id].print(dep);

		return;
	}

	printf("(");

	if ((int)this->child.size() >= 1)
	{
		if (this->child[0].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::String_type)	printf("%s", this->child[0].arg.c_str());
		if (this->child[0].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::Tree_type)	this->child[0].node.print(exist_grouppatterns, dep);
	}

	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Or_type)	printf(" || ");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::And_type)	printf(" && ");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Equal_type)	printf(" = ");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::NotEqual_type)	printf(" != ");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Less_type)	printf(" < ");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::LessOrEqual_type)	printf(" <= ");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Greater_type)	printf(" > ");
	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::GreaterOrEqual_type)	printf(" >= ");

	if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Builtin_regex_type || this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Builtin_langmatches_type)	printf(", ");

	if ((int)this->child.size() >= 2)
	{
		if (this->child[1].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::String_type)	printf("%s", this->child[1].arg.c_str());
		if (this->child[1].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::Tree_type)	this->child[1].node.print(exist_grouppatterns, dep);
	}

	if ((int)this->child.size() >= 3)
	{
		if (this->oper_type == QueryTree::GroupPattern::FilterTreeNode::Builtin_regex_type && this->child[2].node_type == QueryTree::GroupPattern::FilterTreeNode::FilterTreeChild::String_type)
			printf(", %s", this->child[2].arg.c_str());
	}

	printf(")");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void QueryTree::GroupPattern::addOnePattern(Pattern _pattern)
{
	this->patterns.push_back(_pattern);
}

void QueryTree::GroupPattern::addOneGroupUnion()
{
	this->unions.push_back(GroupPatternUnions((int)this->patterns.size() - 1));
}

void QueryTree::GroupPattern::addOneUnion()
{
	int n = (int)this->unions.size();
	this->unions[n - 1].grouppattern_vec.push_back(GroupPattern());
}

QueryTree::GroupPattern& QueryTree::GroupPattern::getLastUnion()
{
	int n = (int)this->unions.size();
	int m = (int)this->unions[n - 1].grouppattern_vec.size();
	return this->unions[n - 1].grouppattern_vec[m - 1];
}

void QueryTree::GroupPattern::addOneOptionalOrMinus(char _type)
{
	this->optionals.push_back(OptionalOrMinusGroupPattern((int)this->patterns.size() - 1, (int)this->unions.size() - 1, _type));
}

QueryTree::GroupPattern& QueryTree::GroupPattern::getLastOptionalOrMinus()
{
	int n = (int)this->optionals.size();
	return this->optionals[n - 1].grouppattern;
}

void QueryTree::GroupPattern::addOneFilterTree()
{
	this->filters.push_back(FilterTreeRoot());
	this->filter_exists_grouppatterns.push_back(vector<GroupPattern>());
}

QueryTree::GroupPattern::FilterTreeNode& QueryTree::GroupPattern::getLastFilterTree()
{
	return this->filters[(int)(this->filters.size()) - 1].root;
}

void QueryTree::GroupPattern::addOneExistsGroupPattern()
{
	int n = (int)this->filter_exists_grouppatterns.size();
	this->filter_exists_grouppatterns[n - 1].push_back(GroupPattern());
}

QueryTree::GroupPattern& QueryTree::GroupPattern::getLastExistsGroupPattern()
{
	int n = (int)this->filter_exists_grouppatterns.size();
	int m = (int)this->filter_exists_grouppatterns[n - 1].size();
	return this->filter_exists_grouppatterns[n - 1][m - 1];
}

void QueryTree::GroupPattern::getVarset()
{
	for (int i = 0; i < (int)this->patterns.size(); i++)
	{
		if (this->patterns[i].subject.value[0] == '?')
		{
			this->patterns[i].varset.addVar(this->patterns[i].subject.value);
			this->grouppattern_subject_object_maximal_varset.addVar(this->patterns[i].subject.value);
		}
		if (this->patterns[i].predicate.value[0] == '?')
		{
			this->patterns[i].varset.addVar(this->patterns[i].predicate.value);
			this->grouppattern_predicate_maximal_varset.addVar(this->patterns[i].predicate.value);
		}
		if (this->patterns[i].object.value[0] == '?')
		{
			this->patterns[i].varset.addVar(this->patterns[i].object.value);
			this->grouppattern_subject_object_maximal_varset.addVar(this->patterns[i].object.value);
		}
		this->grouppattern_resultset_minimal_varset = this->grouppattern_resultset_minimal_varset + this->patterns[i].varset;
		this->grouppattern_resultset_maximal_varset = this->grouppattern_resultset_maximal_varset + this->patterns[i].varset;
	}

	for (int i = 0; i < (int)this->unions.size(); i++)
	{
		Varset minimal_varset;
		for (int j = 0; j < (int)this->unions[i].grouppattern_vec.size(); j++)
		{
			this->unions[i].grouppattern_vec[j].getVarset();
			if (j == 0)
				minimal_varset = minimal_varset + this->unions[i].grouppattern_vec[j].grouppattern_resultset_minimal_varset;
			else
				minimal_varset = minimal_varset * this->unions[i].grouppattern_vec[j].grouppattern_resultset_minimal_varset;
			this->grouppattern_resultset_maximal_varset = this->grouppattern_resultset_maximal_varset + this->unions[i].grouppattern_vec[j].grouppattern_resultset_maximal_varset;
			this->grouppattern_subject_object_maximal_varset = this->grouppattern_subject_object_maximal_varset + this->unions[i].grouppattern_vec[j].grouppattern_subject_object_maximal_varset;
			this->grouppattern_predicate_maximal_varset = this->grouppattern_predicate_maximal_varset + this->unions[i].grouppattern_vec[j].grouppattern_predicate_maximal_varset;
		}
		this->grouppattern_resultset_minimal_varset = this->grouppattern_resultset_minimal_varset + minimal_varset;
	}

	for (int i = 0; i < (int)this->optionals.size(); i++)
	{
		this->optionals[i].grouppattern.getVarset();
		if (this->optionals[i].type == 'o')
		{
			this->grouppattern_resultset_maximal_varset = this->grouppattern_resultset_maximal_varset + this->optionals[i].grouppattern.grouppattern_resultset_maximal_varset;
			this->grouppattern_subject_object_maximal_varset = this->grouppattern_subject_object_maximal_varset + this->optionals[i].grouppattern.grouppattern_subject_object_maximal_varset;
			this->grouppattern_predicate_maximal_varset = this->grouppattern_predicate_maximal_varset + this->optionals[i].grouppattern.grouppattern_predicate_maximal_varset;
		}
	}

	for (int i = 0; i < (int)this->filters.size(); i++)
	{
		this->filters[i].root.getVarset(this->filters[i].varset);
	}

	for(int i = 0; i < (int)this->filter_exists_grouppatterns.size(); i++)
		for (int j = 0; j < (int)this->filter_exists_grouppatterns[i].size(); j++)
		{
			this->filter_exists_grouppatterns[i][j].getVarset();
		}
}

bool QueryTree::GroupPattern::checkOnlyUnionOptionalFilterNoExists()
{
	for (int i = 0; i < (int)this->unions.size(); i++)
	{
		for (int j = 0; j < (int)this->unions[i].grouppattern_vec.size(); j++)
			if (!this->unions[i].grouppattern_vec[j].checkOnlyUnionOptionalFilterNoExists())
				return false;
	}

	for (int i = 0; i < (int)this->optionals.size(); i++)
	{
		if (this->optionals[i].type != 'o')
			return false;
		if (!this->optionals[i].grouppattern.checkOnlyUnionOptionalFilterNoExists())
			return false;
	}

	for (int i = 0; i < (int)this->filter_exists_grouppatterns.size(); i++)
		if ((int)this->filter_exists_grouppatterns[i].size() != 0)
			return false;

	return true;
}

pair<Varset, Varset> QueryTree::GroupPattern::checkOptionalGroupPatternVarsAndSafeFilter(Varset occur , Varset ban, bool &check_condition)
//return occur varset and ban varset
{
	if (!check_condition)	return make_pair(Varset(), Varset());

	Varset this_ban;

	int lastpattern = -1, lastunions = -1, lastoptional = -1;
	while (check_condition && (lastpattern + 1 < (int)this->patterns.size() || lastunions + 1 < (int)this->unions.size() || lastoptional + 1 < (int)this->optionals.size()))
	{
		if (lastoptional + 1 < (int)this->optionals.size() && this->optionals[lastoptional + 1].lastpattern == lastpattern && this->optionals[lastoptional + 1].lastunions == lastunions)
		//optional
		{
			pair<Varset, Varset> sub_grouppattern_return_varset = this->optionals[lastoptional + 1].grouppattern.checkOptionalGroupPatternVarsAndSafeFilter(Varset(), ban, check_condition);

			if (occur.hasCommonVar(sub_grouppattern_return_varset.second))
				check_condition = false;

			Varset out = this->optionals[lastoptional + 1].grouppattern.grouppattern_resultset_maximal_varset - occur;
			occur = occur + sub_grouppattern_return_varset.first;
			this_ban = this_ban + sub_grouppattern_return_varset.second;
			this_ban = this_ban + out;
			ban = ban + this_ban;

			lastoptional++;
		}
		else if (lastunions + 1 < (int)this->unions.size() && this->unions[lastunions + 1].lastpattern == lastpattern)
		//union
		{
			Varset sub_grouppattern_occur, sub_grouppattern_ban;

			for (int i = 0; i < (int)this->unions[lastunions + 1].grouppattern_vec.size(); i++)
			{
				pair<Varset, Varset> sub_grouppattern_result = this->unions[lastunions + 1].grouppattern_vec[i].checkOptionalGroupPatternVarsAndSafeFilter(occur, ban, check_condition);

				if (i == 0)
					sub_grouppattern_occur = sub_grouppattern_occur + sub_grouppattern_result.first;
				else
					sub_grouppattern_occur = sub_grouppattern_occur * sub_grouppattern_result.first;
				sub_grouppattern_ban = sub_grouppattern_ban + sub_grouppattern_result.second;
			}

			occur = occur + sub_grouppattern_occur;
			this_ban = this_ban + sub_grouppattern_ban;
			ban = ban + this_ban;

			lastunions++;
		}
		else
		//triple pattern
		{
			if (this->patterns[lastpattern + 1].varset.hasCommonVar(ban))
				check_condition = false;

			occur = occur + this->patterns[lastpattern + 1].varset;

			lastpattern++;
		}
	}
	//filter
	for (int i = 0; i < (int)this->filters.size(); i++)
	if (!this->filters[i].varset.belongTo(occur))
	{
		check_condition = false;
		break;
	}

	return make_pair(occur, this_ban);
}

void QueryTree::GroupPattern::initPatternBlockid()
{
	this->pattern_blockid.clear();
	for (int i = 0; i < (int)this->patterns.size(); i++)
		this->pattern_blockid.push_back(i);
}

int QueryTree::GroupPattern::getRootPatternBlockID(int x)
{
	if (this->pattern_blockid[x] == x)	return x;
	this->pattern_blockid[x] = getRootPatternBlockID(this->pattern_blockid[x]);
	return this->pattern_blockid[x];
}

void QueryTree::GroupPattern::mergePatternBlockID(int x, int y)
{
	int px = getRootPatternBlockID(x);
	int py = getRootPatternBlockID(y);
	this->pattern_blockid[px] = py;
}

void QueryTree::GroupPattern::print(int dep)
{
	for (int t = 0; t < dep; t++)	printf("\t");	printf("{\n");

	int lastpattern = -1, lastunions = -1, lastoptional = -1;
	while (lastpattern + 1 < (int)this->patterns.size() || lastunions + 1 < (int)this->unions.size() || lastoptional + 1 < (int)this->optionals.size())
	{
		if (lastoptional + 1 < (int)this->optionals.size() && this->optionals[lastoptional + 1].lastpattern == lastpattern && this->optionals[lastoptional + 1].lastunions == lastunions)
		//optional
		{
			for (int t = 0; t <= dep; t++)	printf("\t");
			if (this->optionals[lastoptional + 1].type == 'o')	printf("OPTIONAL\n");
			if (this->optionals[lastoptional + 1].type == 'm')	printf("MINUS\n");

			this->optionals[lastoptional + 1].grouppattern.print(dep + 1);
			lastoptional++;
		}
		else if (lastunions + 1 < (int)this->unions.size() && this->unions[lastunions + 1].lastpattern == lastpattern)
		//union
		{
			for (int i = 0; i < (int)this->unions[lastunions + 1].grouppattern_vec.size(); i++)
			{
				if (i != 0)
				{
					for (int t = 0; t <= dep; t++)	printf("\t");	printf("UNION\n");
				}
				this->unions[lastunions + 1].grouppattern_vec[i].print(dep + 1);
			}
			lastunions++;
		}
		else
		//triple pattern
		{
			for (int t = 0; t <= dep; t++)	printf("\t");
			printf("%s\t%s\t%s.\n", this->patterns[lastpattern + 1].subject.value.c_str(), this->patterns[lastpattern + 1].predicate.value.c_str(), this->patterns[lastpattern + 1].object.value.c_str());
			lastpattern++;
		}
	}
	//filter
	for (int i = 0; i < (int)this->filters.size(); i++)
	{
		for (int t = 0; t <= dep; t++)	printf("\t");	printf("FILTER\t");
		this->filters[i].root.print(this->filter_exists_grouppatterns[i], dep + 1);
		printf("\n");
	}

	for (int t = 0; t < dep; t++)	printf("\t");	printf("}\n");
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void QueryTree::setQueryForm(QueryForm _queryform)
{
	this->query_form = _queryform;
}

QueryTree::QueryForm QueryTree::getQueryForm()
{
	return this->query_form;
}

void QueryTree::setProjectionModifier(ProjectionModifier _projection_modifier)
{
	projection_modifier = _projection_modifier;
}

QueryTree::ProjectionModifier QueryTree::getProjectionModifier()
{
	return this->projection_modifier;
}

void QueryTree::addProjectionVar()
{
	this->projection.push_back(ProjectionVar());
}

QueryTree::ProjectionVar& QueryTree::getLastProjectionVar()
{
	int n = (int)this->projection.size();
	return this->projection[n - 1];
}

vector<QueryTree::ProjectionVar>& QueryTree::getProjection()
{
	return this->projection;
}

vector<string> QueryTree::getProjectionVar()
{
	vector<string> var_vec;

	for (int i = 0; i < (int)this->projection.size(); i++)
		var_vec.push_back(this->projection[i].var);

	return var_vec;
}

void QueryTree::addProjectionUsefulVar(string &_var)
{
	this->projection_useful_varset.addVar(_var);
}

Varset& QueryTree::getProjectionUsefulVar()
{
	return this->projection_useful_varset;
}

void QueryTree::setProjectionAsterisk()
{
	this->projection_asterisk = true;
}

bool QueryTree::checkProjectionAsterisk()
{
	return this->projection_asterisk;
}

bool QueryTree::checkSelectCompatibility()
{
	bool all_var = true, all_aggregate_function = true;

	if (this->checkProjectionAsterisk())
		return true;

	for (int i = 0; i < (int)this->projection.size(); i++)
	{
		if (this->projection[i].aggregate_type != ProjectionVar::None_type)
			all_var = false;
		if (this->projection[i].aggregate_type == ProjectionVar::None_type)
			all_aggregate_function = false;
	}

	return all_var || all_aggregate_function;
}

bool QueryTree::atLeastOneAggregateFunction()
{
	for (int i = 0; i < (int)this->projection.size(); i++)
		if (this->projection[i].aggregate_type != ProjectionVar::None_type)
			return true;

	return false;
}

void QueryTree::addOrder(string &_var, bool _descending)
{
	this->order.push_back(Order(_var, _descending));
}

vector<QueryTree::Order>& QueryTree::getOrder()
{
	return this->order;
}

void QueryTree::setOffset(int _offset)
{
	this->offset = _offset;
}

int QueryTree::getOffset()
{
	return this->offset;
}

void QueryTree::setLimit(int _limit)
{
	this->limit = _limit;
}

int QueryTree::getLimit()
{
	return this->limit;
}

QueryTree::GroupPattern& QueryTree::getGroupPattern()
{
	return this->grouppattern;
}

void QueryTree::setUpdateType(UpdateType _updatetype)
{
	this->update_type = _updatetype;
}

QueryTree::UpdateType QueryTree::getUpdateType()
{
	return this->update_type;
}

QueryTree::GroupPattern& QueryTree::getInsertPatterns()
{
	return this->insert_patterns;
}

QueryTree::GroupPattern& QueryTree::getDeletePatterns()
{
	return this->delete_patterns;
}

bool QueryTree::checkWellDesigned()
{
	if (!this->getGroupPattern().checkOnlyUnionOptionalFilterNoExists())
		return false;

	bool check_condition = true;
	this->getGroupPattern().checkOptionalGroupPatternVarsAndSafeFilter(Varset(), Varset(), check_condition);
	return check_condition;
}

void QueryTree::print()
{
	for (int j = 0; j < 80; j++)			printf("=");	printf("\n");

	if (this->getUpdateType() == Not_Update)
	{
		if (this->getQueryForm() == Select_Query)
		{
			printf("SELECT");
			if (this->getProjectionModifier() == Modifier_Distinct)
				printf(" DISTINCT");
			printf("\n");

			printf("Var: \t");
			vector<ProjectionVar> &proj = this->getProjection();
			for (int i = 0; i < (int)proj.size(); i++)
			{
				if (proj[i].aggregate_type == QueryTree::ProjectionVar::None_type)
					printf("%s\t", proj[i].var.c_str());
				else
				{
					printf("(");
					if (proj[i].aggregate_type == QueryTree::ProjectionVar::Count_type)
						printf("COUNT(");
					if (proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type)
						printf("SUM(");
					if (proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type)
						printf("MIN(");
					if (proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
						printf("MAX(");
					if (proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
						printf("AVG(");
					if (proj[i].distinct)
						printf("DISTINCT ");
					printf("%s) AS %s)\t", proj[i].aggregate_var.c_str(), proj[i].var.c_str());
				}
			}
			if (this->checkProjectionAsterisk() && !this->atLeastOneAggregateFunction())
				printf("*");
			printf("\n");
		}
		else printf("ASK\n");

		printf("GroupPattern:\n");
		this->getGroupPattern().print(0);

		if (this->getQueryForm() == Select_Query)
		{
			if ((int)this->getOrder().size() > 0)
			{
				printf("ORDER BY\t");
				vector<QueryTree::Order>&order = this->getOrder();
				for (int i = 0; i < (int)order.size(); i++)
				{
					if (!order[i].descending)	printf("ASC(");
					else printf("DESC(");
					printf("%s)\t", order[i].var.c_str());
				}
				printf("\n");
			}
			if (this->getOffset() != 0)
				printf("OFFSET\t%d\n", this->getOffset());
			if (this->getLimit() != -1)
				printf("LIMIT\t%d\n", this->getLimit());
		}
	}
	else
	{
		printf("UPDATE\n");
		if (this->getUpdateType() == Delete_Data || this->getUpdateType() == Delete_Where ||
				this->getUpdateType() == Delete_Clause || this->getUpdateType() == Modify_Clause)
		{
			printf("Delete:\n");
			this->getDeletePatterns().print(0);
		}
		if (this->getUpdateType() == Insert_Data || this->getUpdateType() == Insert_Clause || this->getUpdateType() == Modify_Clause)
		{
			printf("Insert:\n");
			this->getInsertPatterns().print(0);
		}
		if (this->getUpdateType() == Delete_Where || this->getUpdateType() == Insert_Clause ||
				this->getUpdateType() == Delete_Clause || this->getUpdateType() == Modify_Clause)
		{
			printf("GroupPattern:\n");
			this->getGroupPattern().print(0);
		}
	}


	for (int j = 0; j < 80; j++)			printf("=");	printf("\n");
}
