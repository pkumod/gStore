/*=============================================================================
# Filename: Optimizer.h
# Author: Lei Yang
# Mail: yangop8@pku.edu.cn
=============================================================================*/

#ifndef _DATABASE_OPTIMIZER_H
#define _DATABASE_OPTIMIZER_H 

#include "../Util/Util.h"
#include "Strategy.h"
#include "../Query/SPARQLquery.h"
#include "../Query/BasicQuery.h"
#include "../Query/IDList.h"
#include "../KVstore/KVstore.h"

typedef struct QueryPlan
{
	int* idlist; //join order
	bool* methodlist; // vertex join or edge join
	QueryPlan(int* _idlist, bool* _methodlist)
	{
		this->idlist = _idlist;
		this->methodlist = _methodlist;
	}
}QueryPlan;


class Optimizer
{
public:
    Optimizer();
    Optimizer(KVstore*, Strategy, SPARQLquery&);
    ~Optimizer();
    bool do_query(SPARQLquery&); // the whole process
    bool is_topk_query; // general query or topk query
    bool is_edge_case; //Strategy 1-6 or Strategy 0
    IDList gen_filter(unsigned _id, KVstore* kvstore, TYPE_TRIPLE_NUM* pre2num, 
	TYPE_TRIPLE_NUM* pre2sub, TYPE_TRIPLE_NUM* pre2obj, bool * dealed_triple); // Strategy::pre_handler()
    unsigned cardinality_estimator(BasicQuery*, KVstore* kvstore, vector<map<BasicQuery*,unsigned*> > _cardinality_cache);
    unsigned cost_model(BasicQuery*, QueryPlan); // TODO: other parameters used in cost model
    
    // all following functions have input and output parameters
    // and return status
    
    // two join ways update current result
    bool vertex_join(BasicQuery*, map<BasicQuery*,vector<unsigned*> > _current_result, 
        vector< vector<int> >& _edges, unsigned _id); // Join::join_two()
    bool edge_join(BasicQuery*, map<BasicQuery*,vector<unsigned*> > _current_result, 
        vector< vector<int> >& _edges, unsigned _id); // Join::only_pre_filter_after_join()
        // Join::pre_var_handler()
    

    bool update_cardinality_cache(BasicQuery*,vector<map<BasicQuery*,unsigned*> >);
    bool enum_query_plan(vector<BasicQuery*>, KVstore* kvstore, bool _is_topk_query);// Join::multi_join() BFS
    bool choose_exec_plan(vector<map<BasicQuery*, QueryPlan*> > _candidate_plans, 
        vector<QueryPlan> _execution_plan);     //TODO: DP

    // change exec_plan if |real cardinality - estimated cardinality| > eps
    bool choose_exec_plan(vector<map<BasicQuery*, QueryPlan*> > _candidate_plans, 
        map<BasicQuery*,vector<unsigned*> > _current_result,  vector<QueryPlan>);
    //TODO: re-choose plan in every iteration
    
    bool execution(vector<BasicQuery*>, vector<int*>, vector<bool*>, vector<unsigned*> _result_list);


private:
    Strategy strategy;
    KVstore* kvstore;
    unsigned current_basic_query; // updated by result_list.size()
    vector<BasicQuery*> basic_query_list; //fork from SPARQLQuery, I dont know why
    vector<map<BasicQuery*, QueryPlan*> > candidate_plans;
    vector<QueryPlan> execution_plan;
    vector<unsigned*> result_list; // vector<unsigned*>* result_list;
    vector<map<BasicQuery*,vector<unsigned*> > > join_cache; // map(sub-structure, result_list)
    vector<map<BasicQuery*,unsigned*> > cardinality_cache; // map(sub-structure, cardinality), not in statistics

};

#endif