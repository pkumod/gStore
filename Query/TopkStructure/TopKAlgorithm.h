#include "../../Util/Util.h"
#include "../../Query/SPARQLquery.h"
#include "../../Database//Optimizer.h"
#include "../../Query/BasicQuery.h"
#include "../../Query/IDList.h"
#include "../../KVstore/KVstore.h"
#include "../../VSTree/VSTree.h"
#include "../TempResult.h"
#include "./twigPatternTopK.h"
#include <unordered_map>

#ifndef GSTORELIMITK_QUERY_TOPKSTRUCTURE_TOPKALGORITHM_H_
#define GSTORELIMITK_QUERY_TOPKSTRUCTURE_TOPKALGORITHM_H_

/**
 * The implement of DP-B algorithm
 * Assumption:
 *  The query Graph is full-connected
 *  No variable is Predicate
 */
class TopKAlgorithm {
  // A SPARQL may be like this:
  // select ?x ?y where{
  // ?x p ?y.
  // ?y p2 ?z
  // ?y p3 ?z
  // } order by (x+y)/3


 private:
  std::shared_ptr<std::map<
      TYPE_ENTITY_LITERAL_ID, // the node
      std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID> // the descendant of the node
      >>> tree_descendant;
  std::shared_ptr<std::vector<std::shared_ptr<OneStepJoinNode>>> constant_generating_lists_;
  std::shared_ptr<std::vector<std::shared_ptr<OneStepJoinNode>>> edge_check_lists_;
  std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<OneStepJoin>>> tree_excution_plan_map_;
  std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>> parent_map_;
  TYPE_ENTITY_LITERAL_ID root_id_;
  std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<OneStepJoin>>> tree_extend_plan_map_;
  std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,bool>> is_leaf_;
  IDCachesSharePtr id_caches_share_ptr_;
  std::shared_ptr<Optimizer> opt_ptr_;
  KVstore kv_store_;
  int k_;

 public:
  // First Construct A Tree out of the Query Graph, and choose a root node which made the layer num minnum
  TopKAlgorithm(){
    // Need a structure which tell us how the tree is organized
    this->tree_descendant = make_shared<map<
        TYPE_ENTITY_LITERAL_ID, // the node
        shared_ptr<vector<TYPE_ENTITY_LITERAL_ID> // the descendant of the node
        >>>();
    this->constant_generating_lists_ = make_shared<vector<shared_ptr<OneStepJoinNode>>>();
    this->edge_check_lists_ = make_shared<vector<shared_ptr<OneStepJoinNode>>>();
    this->tree_excution_plan_map_ = make_shared<map<TYPE_ENTITY_LITERAL_ID, shared_ptr<OneStepJoin>>>();
    this->parent_map_ = make_shared<std::map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>>();
    this->tree_extend_plan_map_ = std::make_shared<std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<OneStepJoin>>>();

  }
  void GenerateTree(BasicQuery* basic_query, KVstore *kv_store,QueryTree::CompTreeNode tree_root_node);

  void PruneEdge(BasicQuery* basic_query, KVstore *kv_store,QueryTree::CompTreeNode tree_root_node);
  int CountLayer( BasicQuery *basic_query,int root_id);

  bool PostorderTraversalPrune(BasicQuery *basic_query,
                               const Varset &var_set,
                               TYPE_ENTITY_LITERAL_ID node);

  bool FillFRwithFQ(TYPE_ENTITY_LITERAL_ID FR_q_id,shared_ptr<FRiterator> FR_it,shared_ptr<IDList> FQ_node_ids);
  void DoQueryBasedOnDpB();

};

#endif //GSTORELIMITK_QUERY_TOPKSTRUCTURE_TOPKALGORITHM_H_
