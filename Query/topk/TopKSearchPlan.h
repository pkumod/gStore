//
// Created by Yuqi Zhou on 2021/8/30.
//
#include "../../Util/Util.h"
#include "../../KVstore/KVstore.h"
#include "DPB/OrderedList.h"
#include "../../Query/SPARQLquery.h"
#include "../../Query/BasicQuery.h"
#include "../../Database/Statistics.h"
#include "../../Query/QueryTree.h"
#include "../../Query/IDList.h"
#include "../../Database/TableOperator.h"
#include "../../Database/PlanGenerator.h"
#ifndef GSTOREGDB_QUERY_TOPK_TOPKSEARCHPLAN_H_
#define GSTOREGDB_QUERY_TOPK_TOPKSEARCHPLAN_H_

// #define TOPK_DEBUG_INFO
#define TOPK_DEBUG_RESULT_INFO
//#define TOPK_DEBUG_TREE_EXTEND_INFO
namespace TopKPlanUtil{
enum class EdgeDirection{IN,OUT,NoEdge};

// something like FeedTwoNodes, but this one is only used in
// Top-K searching
struct TreeEdge{
  std::vector<bool> predicate_constant_;
  std::vector<TYPE_ENTITY_LITERAL_ID> predicate_ids_;
  std::vector<TopKPlanUtil::EdgeDirection> directions_;
  void ChangeOrder();
};

}


// #define SHOW_SCORE
// only Vars
struct TopKTreeNode{
  int var_id;
  std::vector<std::shared_ptr<TopKPlanUtil::TreeEdge>> tree_edges_;
  std::vector<TopKTreeNode*> descendents_;
  size_t descendents_fr_num_;
  size_t descendents_ow_num_;
};

/**
 * The General Top-k Search Plan Structure:
 * 1. filtering all entity vars
 * 2. build top-k iterators
 * 3. get top-k results
 * 3.1 check non-tree edges
 * 3.2 fill selected predicate vars
 */
class TopKSearchPlan {
 private:
  // all the information
  map<TYPE_ENTITY_LITERAL_ID,vector<TYPE_ENTITY_LITERAL_ID>> neighbours_;
  map<TYPE_ENTITY_LITERAL_ID,vector<vector<bool>>> predicates_constant_;
  map<TYPE_ENTITY_LITERAL_ID,vector<vector<TYPE_ENTITY_LITERAL_ID>>> predicates_ids_;
  map<TYPE_ENTITY_LITERAL_ID,vector<vector<TopKPlanUtil::EdgeDirection>>> directions_;
  // The Edges that left behind
  // It can be used when enumerating , use non tree edges to make sure correctness
  StepOperation non_tree_edges_;
  bool is_cycle_graph_;
  std::size_t total_vars_num_;
  static std::size_t CountDepth(map<TYPE_ENTITY_LITERAL_ID,vector<TYPE_ENTITY_LITERAL_ID>> &neighbours, TYPE_ENTITY_LITERAL_ID root_id, std::size_t total_vars_num);
  void AdjustOrder();
  bool walk(set<int> &possible_vars,set<int> &walk_pass_vars,vector<int> &result_cycle);

  void DeleteEdge(TYPE_ENTITY_LITERAL_ID a,TYPE_ENTITY_LITERAL_ID b);
  bool CutCycle(shared_ptr<BGPQuery> bgp_query, KVstore *kv_store, Statistics *statistics,
                shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> id_caches);

 public:
  explicit TopKSearchPlan(shared_ptr<BGPQuery> bgp_query, KVstore *kv_store, Statistics *statistics,
                          const QueryTree::Order&,shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> id_caches);
  void GetPlan(shared_ptr<BGPQuery> bgp_query, KVstore *kv_store, Statistics *statistics, const QueryTree::Order& expression,
               shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> id_caches);
  // The first tree to search
  TopKTreeNode* tree_root_;
  // Recursive delete
  ~TopKSearchPlan();
  StepOperation& GetNonTreeEdges(){return this->non_tree_edges_;};
  std::vector<int> FindCycle();
  bool SuggestTopK();
  void DebugInfo(shared_ptr<BGPQuery> bgp_query, KVstore *kv_store);
  bool HasCycle() {return this->is_cycle_graph_;};
};

#endif //GSTOREGDB_QUERY_TOPK_TOPKSEARCHPLAN_H_
