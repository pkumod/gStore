/*=============================================================================
# Filename: Optimizer.h
# Author: Lei Yang, Yuqi Zhou
# Mail: yangop8@pku.edu.cn. zhouyuqi@pku.edu.cn
=============================================================================*/

#ifndef _DATABASE_OPTIMIZER_H
#define _DATABASE_OPTIMIZER_H

#include "../Query/QueryPlan.h"
#include "./PlanGenerator.h"
#include "./Executor.h"

#ifdef TOPK_SUPPORT
#include "../Query/topk/DPBTopKUtil.h"
#endif // TOPK_SUPPORT
#include <cstring>
#include <climits>
#include <algorithm>
#include <list>
#include <cstdlib>
#include <cmath>
#include <random>
#include "./OptimizerDebug.h"
using namespace std;
using TableContent = list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>;
using TableContentShardPtr = shared_ptr<list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>;
using PositionValue = std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>;
using PositionValueSharedPtr = std::shared_ptr<PositionValue>;
using QueryPlanSharedPtr = shared_ptr<QueryPlan>;

enum class BasicQueryStrategy{
  Normal, // method 0
  Special,// method 1-5
  limitK,
  TopK
};

struct QueryInfo{
  bool limit_;
  int limit_num_;
  bool is_distinct_;
  shared_ptr<vector<Order>> ordered_by_expressions_;
};

class Optimizer
{
 public:

  Optimizer(KVstore* kv_store, TYPE_TRIPLE_NUM* pre2num, TYPE_TRIPLE_NUM* pre2sub,
			TYPE_TRIPLE_NUM* pre2obj, TYPE_TRIPLE_NUM triples_num, TYPE_PREDICATE_ID limitID_predicate,
			TYPE_ENTITY_LITERAL_ID limitID_literal, TYPE_ENTITY_LITERAL_ID limitID_entity, shared_ptr<Transaction> txn);
  ~Optimizer()=default;

  tuple<bool,shared_ptr<IntermediateResult>> DoQuery(SPARQLquery&,QueryInfo); // the whole process
  tuple<bool,bool> DoQuery(std::shared_ptr<BGPQuery> bgp_query,QueryInfo query_info); // the whole process

 private:
  tuple<bool,shared_ptr<IntermediateResult>> MergeBasicQuery(SPARQLquery &sparql_query);

  BasicQueryStrategy ChooseStrategy(std::shared_ptr<BGPQuery> bgp_query,QueryInfo *query_info);

  std::shared_ptr<std::vector<IntermediateResult>> GenerateResultTemplate(shared_ptr<QueryPlan> query_plan);

  tuple<bool,IntermediateResult> ExecutionDepthFirst(shared_ptr<BGPQuery>  bgp_query,
                                                     shared_ptr<QueryPlan> query_plan,
                                                     QueryInfo query_info,
                                                     IDCachesSharePtr id_caches);


  tuple<bool,IntermediateResult> DepthSearchOneLayer(shared_ptr<QueryPlan> query_plan,
                                                     int layer_count,
                                                     int &result_number_till_now,
                                                     int limit_number,
                                                     TableContentShardPtr table_content_ptr,
                                                     IDCachesSharePtr id_caches,
                                                     shared_ptr<vector<IntermediateResult>> table_template);

#ifdef TOPK_SUPPORT
  tuple<bool,IntermediateResult> ExecutionTopK(shared_ptr<BGPQuery> bgp_query, shared_ptr<TopKSearchPlan> &tree_search_plan,
                                                                         const QueryInfo& query_info,IDCachesSharePtr id_caches);
#endif
  bool CopyToResult(shared_ptr<BGPQuery> bgp_query, IntermediateResult result);

  void GetResultListByEdgeInfo(const EdgeInfo &edge_info, unsigned* &result_list_pointer,
							   unsigned &result_list_len, bool distinct);
  bool SpecialOneTuplePlanExecution(PlanTree* plan_tree, shared_ptr<BGPQuery> bgp_query, IDCachesSharePtr &id_caches, bool distinct);

  tuple<bool,IntermediateResult> ExecutionBreathFirst(shared_ptr<BGPQuery> bgp_query,
                                                      QueryInfo query_info,
                                                      Tree_node* plan_tree_node,
                                                      IDCachesSharePtr id_caches);
 private:
  KVstore* kv_store_;

  shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> order_by_list_; // empty if not using 'orderby'
  TYPE_ENTITY_LITERAL_ID limit_num_; // -1 if not limit result size

  TYPE_TRIPLE_NUM* pre2num_;
  TYPE_TRIPLE_NUM* pre2sub_;
  TYPE_TRIPLE_NUM* pre2obj_;
  TYPE_TRIPLE_NUM triples_num_;
  TYPE_PREDICATE_ID limitID_predicate_;
  TYPE_ENTITY_LITERAL_ID limitID_literal_;
  TYPE_ENTITY_LITERAL_ID limitID_entity_;
  shared_ptr<Transaction> txn_;

  gstore::Executor executor_;

  tuple<bool,bool,bool> PrepareInitial(shared_ptr<BGPQuery> bgp_query,
                            shared_ptr<AffectOneNode> join_a_node_plan) const;
  tuple<bool,IntermediateResult> InitialTable(shared_ptr<BGPQuery> &bgp_query,
                                  const IDCachesSharePtr &id_caches,
                                  shared_ptr<StepOperation> &step_operation);
  tuple<bool,IntermediateResult> UpdateCandidates(IDCachesSharePtr &id_caches,
                                      shared_ptr<StepOperation> &step_operation);

  tuple<bool, IntermediateResult> ExtendOneNode(shared_ptr<BGPQuery> &bgp_query,
                                                shared_ptr<StepOperation> &step_operation,
                                                const IDCachesSharePtr &id_caches,
                                                IntermediateResult &left_table);
  tuple<bool, IntermediateResult> ExtendTwoNode(shared_ptr<BGPQuery> &bgp_query,
                                                shared_ptr<StepOperation> &step_operation,
                                                const IDCachesSharePtr &id_caches,
                                                IntermediateResult &left_table);
  tuple<bool, IntermediateResult> TableCheck(shared_ptr<StepOperation> &step_operation,
                                             IntermediateResult &left_table);
  tuple<bool, IntermediateResult> JoinTable(shared_ptr<StepOperation> &step_operation,
                                            IntermediateResult &left_table,
                                            IntermediateResult &right_table);
};


#endif