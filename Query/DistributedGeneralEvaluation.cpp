/*=============================================================================
# Filename: GeneralEvaluation.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-03-02 20:35
# Description: implement functions in GeneralEvaluation.h
# modified by zengli
=============================================================================*/

#include "DistributedGeneralEvaluation.h"
#include <mpi.h>

using namespace std;

void DistributedGeneralEvaluation::doQuery(const string &_query, int argc, char * argv[])
{
    long tv_begin = Util::get_cur_time();

    if (!this->parseQuery(_query))
        return;
    long tv_parse = Util::get_cur_time();
    cout << "after Parsing, used " << (tv_parse - tv_begin) << "ms." << endl;

    this->query_tree.getGroupPattern().getVarset();

	//if(this->query_tree.getQueryForm() == QueryTree::Select_Query && this->query_tree.checkWellDesigned())
	//{
	//	cout << "=================" << endl;
	//	cout << "||well-designed||" << endl;
	//	cout << "=================" << endl;

	//	this->expansion_evaluation_stack.clear();
	//	this->expansion_evaluation_stack.reserve(100);
	//	this->expansion_evaluation_stack.push_back(ExpansionEvaluationStackUnit());
	//	this->expansion_evaluation_stack[0].grouppattern = this->query_tree.getGroupPattern();
	//	this->queryRewriteEncodeRetrieveJoin(0, this->result_filter);

	//	this->semantic_evaluation_result_stack.push(this->expansion_evaluation_stack[0].result);
	//}
	//else
	{
		cout << "=====================" << endl;
		cout << "||not well-designed||" << endl;
		cout << "=====================" << endl;

		this->getBasicQuery(this->query_tree.getGroupPattern());

		//vector< vector<string> > tmp;
		//tmp.push_back(this->query_tree.getProjection());
		//this->sparql_query.encodeQuery(this->kvstore, this->getProjection());
		this->sparql_query.encodeQuery(this->kvstore, this->getSPARQLQueryVarset());
		cout << "sparqlSTR:\t" << this->sparql_query.to_str() << endl;
		long tv_encode = Util::get_cur_time();
		cout << "after Encode, used " << (tv_encode - tv_parse) << "ms." << endl;

		//NOTICE: use this strategy instead of default filter-join way
		Strategy stra(this->kvstore, this->vstree);
		stra.handle(this->sparql_query);
		long tv_handle = Util::get_cur_time();
		cout << "after handle, used " << (tv_handle - tv_encode) << "ms." << endl;

		
		//this->vstree->retrieve(this->sparql_query);
		//long tv_retrieve = Util::get_cur_time();
		//cout << "after Retrieve, used " << (tv_retrieve - tv_encode) << "ms." << endl;

		//Join *join = new Join(kvstore);
		//join->join_sparql(this->sparql_query);
		//delete join;
		//long tv_join = Util::get_cur_time();
		//cout << "after Join, used " << (tv_join - tv_retrieve) << "ms." << endl;
		
		this->generateEvaluationPlan(this->query_tree.getGroupPattern());
		this->doEvaluationPlan();
		long tv_postpro = Util::get_cur_time();
		cout << "after Postprocessing, used " << (tv_postpro - tv_handle) << "ms." << endl;
	}

	long tv_bfget = Util::get_cur_time();
	this->getFinalResult(this->result_set);
	long tv_final = Util::get_cur_time();
	cout << "after getFinalResult, used " << (tv_final - tv_bfget) << "ms." << endl;

	//BETTER:output final size and total time at last
    cout << "Total time used: " << (tv_final - tv_begin) << "ms." << endl;
}