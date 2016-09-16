/*
 * SPARQLquery.cpp
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */

#include "SPARQLquery.h"
#include "../Parser/DBparser.h"
#include "../Database/Database.h"


SPARQLquery::SPARQLquery()
{

}

SPARQLquery::~SPARQLquery()
{
	for(int i = 0; i < this->query_vec.size(); i ++)
	{
		delete this->query_vec[i];
	}
}

void SPARQLquery::addQueryVar(const string& _var)
{
	int n = (int)this->query_var_vec.size();
	std::vector<std::string>::iterator i = find(this->query_var_vec[n - 1].begin(), this->query_var_vec[n - 1].end(), _var);
	if (i == this->query_var_vec[n - 1].end())	this->query_var_vec[n - 1].push_back(_var);
}

void SPARQLquery::addQueryVarVec()
{
	this->query_var_vec.push_back(vector <string> ());
}

vector<string>& SPARQLquery::getQueryVarVec(int _var_vec_id)
{
	return this->query_var_vec[_var_vec_id];
}

void SPARQLquery::addTriple(const Triple& _triple){
	int last_i = query_vec.size()-1;
	query_vec[last_i]->addTriple(_triple);
}

const int SPARQLquery::getBasicQueryNum()
{
	return this->query_vec.size();
}

void SPARQLquery::encodeQuery(KVstore* _p_kv_store)
{
	for(int i = 0; i < this->query_vec.size(); i ++)
	{
		(this->query_vec[i])->encodeBasicQuery(_p_kv_store, this->query_var_vec[i]);
	}
}

BasicQuery& SPARQLquery::getBasicQuery(int basic_query_id)
{
	return *(query_vec[basic_query_id]);
}

void SPARQLquery::addBasicQuery()
{
	query_vec.push_back(new BasicQuery(""));
}

void SPARQLquery::addBasicQuery(BasicQuery* _basic_q)
{
	this->query_vec.push_back(_basic_q);
}

vector<BasicQuery*>& SPARQLquery::getBasicQueryVec()
{
	return this->query_vec;
}

void SPARQLquery::print(ostream& _out_stream)
{
	int n = getBasicQueryNum();
	std::cout << "Block " << n << " in total." << std::endl;
	for (int i = 0; i < n; i++)
	{
		std::cout<<"Block "<< i << std::endl;
		int m = this->query_var_vec[i].size();
		std::cout << "QueryVar "<< m <<":"<< std::endl;
		for (int j = 0; j < m; j++)
			std::cout << this->query_var_vec[i][j] << " ";
		std::cout << std::endl;
		getBasicQuery(i).print(_out_stream);
	}
}

std::string SPARQLquery::triple_str()
{
	std::stringstream _ss;

	for(int i = 0; i < this->query_vec.size(); i ++)
	{
		_ss << "varVec" << i << "varNum:" << this->query_var_vec[i].size() << endl;
		for (int j = 0; j < this->query_var_vec[i].size(); j++)
			_ss << this->query_var_vec[i][j] << "\t";
		_ss << endl;
		_ss << "bq" << i << " :" << this->query_vec[i]->triple_str() << endl;
	}

	return _ss.str();
}

std::string SPARQLquery::candidate_str()
{
	std::stringstream _ss;

	for(int i = 0; i < this->query_vec.size(); i ++)
	{
		_ss << "bq" << i << " :" << this->query_vec[i]->candidate_str() << endl;
	}

	return _ss.str();
}

std::string SPARQLquery::result_str()
{
	std::stringstream _ss;

	for(int i = 0; i < this->query_vec.size(); i ++)
	{
		_ss << "bq" << i << " :" << this->query_vec[i]->result_str() << endl;
	}

	return _ss.str();
}

std::string SPARQLquery::to_str()
{
	std::stringstream _ss;

	for(int i = 0; i < this->query_vec.size(); i ++)
	{
		_ss << "varVec" << i << "varNum:" << this->query_var_vec[i].size() << endl;
		for (int j = 0; j < this->query_var_vec[i].size(); j++)
			_ss << this->query_var_vec[i][j] << "\t";
		_ss << endl;
		_ss << "bq" << i << " :\n" << this->query_vec[i]->to_str() << endl;
	}

	return _ss.str();
}

//----------------------------------------------------------------------------------------------------------------------------------------------------

void SPARQLquery::addOneProjection(std::string _projection)
{
	this->projections.push_back(_projection);
}

int SPARQLquery::getProjectionsNum()
{
	return this->projections.size();
}

std::vector<std::string>& SPARQLquery::getProjections()
{
	return this->projections;
}

SPARQLquery::PatternGroup& SPARQLquery::getPatternGroup()
{
	return this->patterngroup;
}

void SPARQLquery::PatternGroup::addOnePattern(Pattern _pattern)
{
	if (_pattern.subject.value[0] == '?' || _pattern.object.value[0] == '?')		this->hasVar = true;
	this->patterns.push_back(_pattern);
}

void SPARQLquery::PatternGroup::addOneFilterTree()
{
	this->filters.push_back(FilterTree());
}

SPARQLquery::FilterTree& SPARQLquery::PatternGroup::getLastFilterTree()
{
	return this->filters[(int)(this->filters.size()) - 1];
}

void SPARQLquery::PatternGroup::addOneOptional()
{
	this->optionals.push_back(PatternGroup());
}

SPARQLquery::PatternGroup& SPARQLquery::PatternGroup::getLastOptional()
{
	return this->optionals[(int)(this->optionals.size()) - 1];
}

void SPARQLquery::PatternGroup::addOneGroupUnion()
{
	this->unions.push_back(std::vector<PatternGroup>());
}

void SPARQLquery::PatternGroup::addOneUnion()
{
	int n = this->unions.size();
	this->unions[n - 1].push_back(PatternGroup());
}

SPARQLquery::PatternGroup& SPARQLquery::PatternGroup::getLastUnion()
{
	int n = this->unions.size();
	int m = this->unions[n - 1].size();
	return this->unions[n - 1][m - 1];
}
