/*=============================================================================
# Filename: SPARQLquery.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2021-07-29 09:58 CST (Yue Pang)
# Description: implement functions in SPARQLquery.h
=============================================================================*/

#include "SPARQLquery.h"
#include "../Database/Database.h"

using namespace std;

SPARQLquery::SPARQLquery(const string& _query)
{

}

SPARQLquery::SPARQLquery()
{

}

SPARQLquery::SPARQLquery(const SPARQLquery &other):query_union(other.query_union),query_var(other.query_var) {

}

SPARQLquery::~SPARQLquery()
{
	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		delete this->query_union[i];
	}
}

void 
SPARQLquery::addQueryVar(const string& _var)
{
	query_var.push_back(_var);
}

const int 
SPARQLquery::getQueryVarNum()
{
	return query_var.size();
}

const vector<string>& 
SPARQLquery::getQueryVar()const
{
	return this->query_var;
}

const string& 
SPARQLquery::getQueryVar(int _id)
{
	return this->query_var.at(_id);
}

void 
SPARQLquery::addTriple(const Triple& _triple){
	int last_i = query_union.size()-1;
	query_union[last_i]->addTriple(_triple);
}

const int 
SPARQLquery::getBasicQueryNum()
{
	return this->query_union.size();
}

void 
SPARQLquery::encodeQuery(KVstore* _p_kv_store)
{
	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		(this->query_union[i])->encodeBasicQuery(_p_kv_store, this->query_var);
	}
}

void 
SPARQLquery::encodeQuery(KVstore* _p_kv_store, vector< vector<string> > sparql_query_varset)
{
	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		(this->query_union[i])->encodeBasicQuery(_p_kv_store, sparql_query_varset[i]);
	}
}

BasicQuery& 
SPARQLquery::getBasicQuery(int basic_query_id)
{
	return *(query_union[basic_query_id]);
}

void 
SPARQLquery::addBasicQuery(){
	query_union.push_back(new BasicQuery(""));
}

void 
SPARQLquery::addBasicQuery(BasicQuery* _basic_q)
{
	this->query_union.push_back(_basic_q);
}

vector<BasicQuery*>& 
SPARQLquery::getBasicQueryVec()
{
	return this->query_union;
}

void 
SPARQLquery::print(ostream& _out_stream){
	int k=getQueryVarNum();
	cout<<"QueryVar "<<k<<":"<<endl;
	for (int i=0;i<k;i++){
		cout<<getQueryVar(i)<<endl;
	}
	k=getBasicQueryNum();
	cout<<"Block "<<k<<" in total."<<endl;
	for (int i=0;i<k;i++){
		cout<<"Block "<<i<<endl;
		getBasicQuery(i).print(_out_stream);
	}
}

string 
SPARQLquery::triple_str()
{
	stringstream _ss;

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

string 
SPARQLquery::candidate_str()
{
	stringstream _ss;

	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		_ss << "bq" << i << " :" << this->query_union[i]->candidate_str() << endl;
	}

	return _ss.str();
}

string 
SPARQLquery::result_str()
{
	stringstream _ss;

	for(unsigned i = 0; i < this->query_union.size(); i ++)
	{
		_ss << "bq" << i << " :" << this->query_union[i]->result_str() << endl;
	}

	return _ss.str();
}

string 
SPARQLquery::to_str()
{
	stringstream _ss;

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
int SPARQLquery::GetLimit() const {
  return this->limit_;
}
void SPARQLquery::SetLimit(int limit) {
  this->limit_ = limit;
}

shared_ptr<vector<SPARQLquery::OrderedBy>> SPARQLquery::GetOrderedByVec(){
  return this->ordered_by_vec;
}

void SPARQLquery::SetOrderedByVec(const shared_ptr<vector<OrderedBy>> &ordered_by_vec) {
  this->ordered_by_vec = ordered_by_vec;
}

