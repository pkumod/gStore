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
#include "ResultTrigger.h"

using namespace std;
class QueryPlan
{
  shared_ptr<vector<OneStepJoin>> join_order_; //join order
  shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> ids_after_join_;

  QueryPlan(shared_ptr<vector<OneStepJoin>> ,shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>);

  QueryPlan(BasicQuery*,KVstore*);
};

enum class BasicQueryStrategy{
  Normal,
  Special// method 1-5
};
/*
1. Optimizer类在generalevaluation里面初始化，并且在generalevaluation调用do_query()
----------
 2. do_query()首先判断handler0或者handler1-5
3. 在handler0里面首先调用enum_query_plan()，按照BFS或者DFS生成所有执行计划
4. 生成完执行计划后调用choose_exec_plan()选择一个计划执行
5. 在choose_exec_plan()中每一个执行计划需要层次调用cost_model()->cardinality_estimator()->update_cardinality_cache()
 ----------
6. 按照点的顺序首先需要对起始点调用gen_filter获得该点的候选集 [done]
    而后的每一个点调用join_one_step()
7. 在每一步的join_one_step()中需要根据queryplan调用三种基本join之一，同时需要update中间结果，并且每一步的开始需要check一下中间结果能不能利用
8. 每一步都做完之后，copytoResult()
*/

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

  tuple<bool,shared_ptr<IntermediateResult>> GenerateColdCandidateList(vector<shared_ptr<EdgeInfo>>,vector<shared_ptr<EdgeConstantInfo>>);
  tuple<bool,shared_ptr<IntermediateResult>> JoinANode(shared_ptr<OneStepJoinNode>,shared_ptr<IntermediateResult>);
  tuple<bool,shared_ptr<IntermediateResult>> JoinTwoTable(shared_ptr<OneStepJoinTable>,shared_ptr<IntermediateResult>,shared_ptr<IntermediateResult>);
  tuple<bool,shared_ptr<IntermediateResult>> EdgeConstraintFilter(shared_ptr<EdgeInfo>, EdgeConstantInfo, shared_ptr<IntermediateResult>);
  tuple<bool,shared_ptr<IntermediateResult>> FilterAVariableOnIDList(shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>,TYPE_ENTITY_LITERAL_ID ,shared_ptr<IntermediateResult>);


  shared_ptr<IntermediateResult> NormalJoin(shared_ptr<BasicQuery>,shared_ptr<QueryPlan>);
  bool update_cardinality_cache(shared_ptr<BasicQuery>,vector<map<shared_ptr<BasicQuery>,unsigned*> >);
  bool enum_query_plan(vector<shared_ptr<BasicQuery>>, KVstore* kvstore, bool _is_topk_query);// Join::multi_join() BFS
  bool choose_exec_plan(vector<map<shared_ptr<BasicQuery>, QueryPlan*> > _candidate_plans,
                        vector<QueryPlan> _execution_plan);     //TODO: DP

  // change exec_plan if |real cardinality - estimated cardinality| > eps
  bool choose_exec_plan(vector<map<shared_ptr<BasicQuery>, QueryPlan*> > _candidate_plans,
                        map<shared_ptr<BasicQuery>,vector<unsigned*> > _current_result,  vector<QueryPlan>);
  //TODO: re-choose plan in every iteration



  BasicQueryStrategy ChooseStrategy(BasicQuery*);
  bool execution(vector<shared_ptr<BasicQuery>>, vector<QueryPlan>, vector<unsigned*> _result_list);

  static void UpdateIDList(shared_ptr<IDList>, unsigned*, unsigned,bool);

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