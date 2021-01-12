/*=============================================================================
# Filename: Optimizer.h
# Author: Lei Yang, Yuqi Zhou
# Mail: yangop8@pku.edu.cn. zhouyuqi@pku.edu.cn
=============================================================================*/

#ifndef _DATABASE_OPTIMIZER_H
#define _DATABASE_OPTIMIZER_H

#include "../Util/Util.h"
#include "Strategy.h"
#include "../Query/SPARQLquery.h"
#include "../Query/BasicQuery.h"
#include "../Query/IDList.h"
#include "../KVstore/KVstore.h"
#include "TableOperator.h"


class QueryPlan
{

  shared_ptr<vector<OneStepJoin>> join_order_; //join order
  shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> ids_after_join_;

  QueryPlan(shared_ptr<vector<OneStepJoin>> ,shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>);
};

struct IntermediateResult{
  shared_ptr<vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>> values_;
  shared_ptr<map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>> id_to_position_;
  shared_ptr<map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>> position_to_id;
  shared_ptr<vector<bool>> dealed_triple_;
};


class ResultTrigger{
  int estimate_upper_bound;
  int estimate_lower_bound;
};

class Optimizer
{
 public:
  Optimizer();
  Optimizer(KVstore* kv_store, VSTree* vs_tree, TYPE_TRIPLE_NUM* pre2num, TYPE_TRIPLE_NUM* pre2sub,
             TYPE_TRIPLE_NUM* pre2obj, TYPE_PREDICATE_ID limitID_predicate, TYPE_ENTITY_LITERAL_ID limitID_literal,
             TYPE_ENTITY_LITERAL_ID limitID_entity,bool is_distinct, shared_ptr<Transaction> txn,SPARQLquery& sparql_query,
             shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> order_by_list,TYPE_ENTITY_LITERAL_ID limit_num);
  ~Optimizer();
  bool do_query(SPARQLquery&); // the whole process
  IDList gen_filter(unsigned _id, KVstore* kvstore, shared_ptr<TYPE_TRIPLE_NUM*> pre2num,
                    shared_ptr<TYPE_TRIPLE_NUM*> pre2sub, shared_ptr<TYPE_TRIPLE_NUM*> pre2obj, shared_ptr<bool*> dealed_triple); // Strategy::pre_handler()
  unsigned cardinality_estimator(shared_ptr<BasicQuery>, KVstore* kvstore, vector<map<shared_ptr<BasicQuery>,unsigned*> > _cardinality_cache);
  unsigned cost_model(shared_ptr<BasicQuery>, shared_ptr<QueryPlan>); // TODO: other parameters used in cost model

  // all following functions have input and output parameters
  // and return status

  bool JoinOneStep(shared_ptr<QueryPlan>,shared_ptr<IntermediateResult>,shared_ptr<ResultTrigger>);
  // two join ways update current result
  bool vertex_join(shared_ptr<BasicQuery>, map<shared_ptr<BasicQuery>,shared_ptr<vector<unsigned*>>> _current_result,
                   shared_ptr< vector< vector<int> >> _edges, unsigned _id); // Join::join_two()
  bool edge_join(shared_ptr<BasicQuery>, map<shared_ptr<BasicQuery>,shared_ptr<vector<unsigned*>>> _current_result,
                 shared_ptr<vector< vector<int> >> _edges, unsigned _id); // Join::only_pre_filter_after_join()
  // Join::pre_var_handler()


  bool update_cardinality_cache(shared_ptr<BasicQuery>,vector<map<shared_ptr<BasicQuery>,unsigned*> >);
  bool enum_query_plan(vector<shared_ptr<BasicQuery>>, KVstore* kvstore, bool _is_topk_query);// Join::multi_join() BFS
  bool choose_exec_plan(vector<map<shared_ptr<BasicQuery>, QueryPlan*> > _candidate_plans,
                        vector<QueryPlan> _execution_plan);     //TODO: DP

  // change exec_plan if |real cardinality - estimated cardinality| > eps
  bool choose_exec_plan(vector<map<shared_ptr<BasicQuery>, QueryPlan*> > _candidate_plans,
                        map<shared_ptr<BasicQuery>,vector<unsigned*> > _current_result,  vector<QueryPlan>);
  //TODO: re-choose plan in every iteration

  bool execution(vector<shared_ptr<BasicQuery>>, vector<QueryPlan>, vector<unsigned*> _result_list);


 private:
  KVstore* kv_store_;

  shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> order_by_list_; // empty if not using 'orderby'
  TYPE_ENTITY_LITERAL_ID limit_num_; // -1 if not limit result size

  bool is_edge_case; //Strategy 1-6 or Strategy 0
  int current_basic_query_; // updated by result_list.size()

  shared_ptr<vector<shared_ptr<BasicQuery>>> basic_query_list_; //fork from SPARQLQuery, I dont know why
  shared_ptr<vector<tuple<shared_ptr<BasicQuery>, shared_ptr<vector<QueryPlan>>>>> candidate_plans_;
  shared_ptr<vector<QueryPlan>> execution_plan_;
  shared_ptr<vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>> result_list_; // vector<unsigned*>* result_list;

  //TODO: shared_ptr<BasicQuery> may brings wrong matching
  shared_ptr<vector<map<shared_ptr<BasicQuery>,vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>>> join_cache_; // map(sub-structure, result_list)
  shared_ptr<vector<map<shared_ptr<BasicQuery>,shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>> cardinality_cache_; // map(sub-structure, cardinality), not in statistics

  FILE* fp_;
  bool export_flag_;
  bool is_distinct_;
  VSTree* vstree_;
  TYPE_TRIPLE_NUM* pre2num_;
  TYPE_TRIPLE_NUM* pre2sub_;
  TYPE_TRIPLE_NUM* pre2obj_;
  TYPE_PREDICATE_ID limitID_predicate_;
  TYPE_ENTITY_LITERAL_ID limitID_literal_;
  TYPE_ENTITY_LITERAL_ID limitID_entity_;
  shared_ptr<Transaction> txn_;

};

#endif