/*=============================================================================
# Filename: GeneralEvaluation.cpp
# Author: Jiaqi, Chen
# Mail: 1181955272@qq.com
# Last Modified: 2016-03-02 20:35
# Description: implement functions in QueryTree.h
=============================================================================*/

#include "QueryTree.h"

using namespace std;

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

void QueryTree::addProjectionVar(std::string _projection)
{
	this->projection.addVar(_projection);
}

int QueryTree::getProjectionNum()
{
	return (int)this->projection.varset.size();
}

Varset& QueryTree::getProjection()
{
	return this->projection;
}

void QueryTree::setProjectionAsterisk()
{
	this->projection_asterisk = true;
}

bool QueryTree::checkProjectionAsterisk()
{
	return this->projection_asterisk;
}

void QueryTree::addOrder(std::string &_var, bool _descending)
{
	this->order.push_back(Order(_var, _descending));
}

std::vector<QueryTree::Order>& QueryTree::getOrder()
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

QueryTree::PatternGroup& QueryTree::getPatternGroup()
{
	return this->patterngroup;
}

void QueryTree::PatternGroup::addOnePattern(Pattern _pattern)
{
	this->patterns.push_back(_pattern);
}

void QueryTree::PatternGroup::addOneGroupUnion()
{
	this->unions.push_back(vector<PatternGroup>());
}

void QueryTree::PatternGroup::addOneUnion()
{
	int n = (int)this->unions.size();
	this->unions[n - 1].push_back(PatternGroup());
}

QueryTree::PatternGroup& QueryTree::PatternGroup::getLastUnion()
{
	int n = (int)this->unions.size();
	int m = (int)this->unions[n - 1].size();
	return this->unions[n - 1][m - 1];
}

void QueryTree::PatternGroup::addOneOptionalOrMinus(char _type)
{
	this->optionals.push_back(OptionalOrMinusPatternGroup((int)this->patterns.size() - 1, (int)this->unions.size() - 1, _type));
}

QueryTree::PatternGroup& QueryTree::PatternGroup::getLastOptionalOrMinus()
{
	int n = (int)this->optionals.size();
	return this->optionals[n - 1].patterngroup;
}

void QueryTree::PatternGroup::addOneFilterTree()
{
	this->filters.push_back(FilterTree());
	this->filter_exists_patterngroups.push_back(vector<PatternGroup>());
}

QueryTree::FilterTree& QueryTree::PatternGroup::getLastFilterTree()
{
	return this->filters[(int)(this->filters.size()) - 1];
}

void QueryTree::PatternGroup::addOneExistsGroupPattern()
{
	int n = (int)this->filter_exists_patterngroups.size();
	this->filter_exists_patterngroups[n - 1].push_back(PatternGroup());
}

QueryTree::PatternGroup& QueryTree::PatternGroup::getLastExistsGroupPattern()
{
	int n = (int)this->filter_exists_patterngroups.size();
	int m = (int)this->filter_exists_patterngroups[n - 1].size();
	return this->filter_exists_patterngroups[n - 1][m - 1];
}

void QueryTree::PatternGroup::initPatternBlockid()
{
	for (int i = 0; i < (int)this->patterns.size(); i++)
		this->pattern_blockid.push_back(i);
}

int QueryTree::PatternGroup::getRootPatternBlockid(int x)
{
	if (this->pattern_blockid[x] == x)	return x;
	this->pattern_blockid[x] = getRootPatternBlockid(this->pattern_blockid[x]);
	return this->pattern_blockid[x];
}

void QueryTree::PatternGroup::mergePatternBlockid(int x, int y)
{
	int px = getRootPatternBlockid(x);
	int py = getRootPatternBlockid(y);
	this->pattern_blockid[px] = py;
}

