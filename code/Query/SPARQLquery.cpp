/*=============================================================================
# Filename: SPARQLquery.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-31 19:15
# Description: 
=============================================================================*/

#include "SPARQLquery.h"
#include "../Parser/DBparser.h"
#include "../Database/Database.h"

SPARQLquery::SPARQLquery(const string& _query)
{

}

SPARQLquery::SPARQLquery()
{

}

SPARQLquery::~SPARQLquery()
{
	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		delete this->query_union[i];
	}
}

void SPARQLquery::addQueryVar(const string& _var)
{
	query_var.push_back(_var);
}

const int SPARQLquery::getQueryVarNum()
{
	return query_var.size();
}

const vector<string>& SPARQLquery::getQueryVar()const
{
	return this->query_var;
}

const string& SPARQLquery::getQueryVar(int _id)
{
	return this->query_var.at(_id);
}

void SPARQLquery::addTriple(const Triple& _triple){
	int last_i = query_union.size()-1;
	query_union[last_i]->addTriple(_triple);
}

const int SPARQLquery::getBasicQueryNum()
{
	return this->query_union.size();
}

void SPARQLquery::encodeQuery(KVstore* _p_kv_store)
{
	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		(this->query_union[i])->encodeBasicQuery(_p_kv_store, this->query_var);
	}
}

BasicQuery& SPARQLquery::getBasicQuery(int basic_query_id)
{
	return *(query_union[basic_query_id]);
}

void SPARQLquery::addBasicQuery(){
	query_union.push_back(new BasicQuery(""));
}

void SPARQLquery::addBasicQuery(BasicQuery* _basic_q)
{
	this->query_union.push_back(_basic_q);
}

vector<BasicQuery*>& SPARQLquery::getBasicQueryVec()
{
	return this->query_union;
}

void SPARQLquery::print(ostream& _out_stream){
	int k=getQueryVarNum();
	std::cout<<"QueryVar "<<k<<":"<<std::endl;
	for (int i=0;i<k;i++){
		std::cout<<getQueryVar(i)<<std::endl;
	}
	k=getBasicQueryNum();
	std::cout<<"Block "<<k<<" in total."<<std::endl;
	for (int i=0;i<k;i++){
		std::cout<<"Block "<<i<<std::endl;
		getBasicQuery(i).print(_out_stream);
	}
}

std::string SPARQLquery::triple_str()
{
	std::stringstream _ss;

	_ss << "varNum:" << this->query_var.size() << endl;
	for(unsigned i = 0; i < this->query_var.size(); i ++)
	{
		_ss << this->query_var[i] << "\t";
	}
	_ss << endl;

	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		_ss << "bq" << i << " :" << this->query_union[i]->triple_str() << endl;
	}

	return _ss.str();
}

std::string SPARQLquery::candidate_str()
{
	std::stringstream _ss;

	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		_ss << "bq" << i << " :" << this->query_union[i]->candidate_str() << endl;
	}

	return _ss.str();
}

std::string SPARQLquery::result_str()
{
	std::stringstream _ss;

	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		_ss << "bq" << i << " :" << this->query_union[i]->result_str() << endl;
	}

	return _ss.str();
}

std::string SPARQLquery::to_str()
{
	std::stringstream _ss;

	_ss << "varNum:" << this->query_var.size() << endl;
	for(unsigned i = 0; i < this->query_var.size(); i ++)
	{
		_ss << this->query_var[i] << "\t";
	}
	_ss << endl;

	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		_ss << "bq" << i << " :\n" << this->query_union[i]->to_str() << endl;
	}

	return _ss.str();
}

