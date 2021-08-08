//
// Created by Yuqi Zhou on 2021/7/10.
//

#ifndef GSTORELIMITK_QUERY_TOPK_TOPKUTIL_H_
#define GSTORELIMITK_QUERY_TOPK_TOPKUTIL_H_

#include "../../Util/Util.h"
#include "../../KVstore/KVstore.h"
#include "./OrderedList.h"
#include "../../Query/SPARQLquery.h"
#include "../../Query/BasicQuery.h"
#include "../../Database/Statistics.h"
#include "../../Query/QueryTree.h"
#include "../../Query/IDList.h"
#include "../../Database/TableOperator.h"

#define TOPK_DEBUG_INFO
// #define SHOW_SCORE
// only Vars
struct TopKTreeNode{
  int var_id;
  std::vector<TopKTreeNode*> descendents_;
};

/**
 * The General Top-k Search Plan Structure:
 * 1. filtering all entity vars
 * 2. build top-k iterators
 * 3. get top-k results
 * 3.1 check non-tree edges
 * 3.2 fill selected predicate vars
 */
class TopKTreeSearchPlan {
 private:
  std::size_t CountDepth(std::map<int, set<int>>& neighbours, TYPE_ENTITY_LITERAL_ID root_id, std::size_t total_vars_num);
 public:
  // Choose A shortest tree to do top-k
  explicit TopKTreeSearchPlan(BasicQuery* basic_query, Statistics *statistics, QueryTree::Order expression,
                              shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> id_caches);
  // The first tree to search
  TopKTreeNode* tree_root_;
  // Recursive delete
  ~TopKTreeSearchPlan();
  // The Edges that left behind
  // It can be used in two ways:
  // 1 . when filtering, use non tree edges to early filter
  // 2 . when enumerating , use non tree edges to make sure correctness
  std::vector<OneStepJoin> non_tree_edges;
  // The predicate been selected,
  // We process these vars when all the entity vars have been filled
  std::vector<OneStepJoin> selected_predicate_edges;

  std::string DebugInfo();
};

namespace TopKUtil{

void CalculatePosVarMappingNode(TopKTreeNode* top_k_tree_node,shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> pos_var_mapping);
shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> CalculatePosVarMapping(shared_ptr<TopKTreeSearchPlan> search_plan);

double GetScore(string &v, stringstream &ss);
void GetVarCoefficientsTreeNode(QueryTree::CompTreeNode *comp_tree_node,
                                std::map<std::string,double>& coefficients,
                                stringstream &ss);
std::shared_ptr<std::map<std::string,double>> getVarCoefficients(QueryTree::Order order);

struct Env{
  KVstore *kv_store;
  BasicQuery *basic_query;
  shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> id_caches;
  int k;
  // store the OrderedList that have been allocated , and delete them in the end of top-k function
  std::shared_ptr<std::vector<std::shared_ptr<std::set<OrderedList*>>>> global_iterators;
  // store the non-tree edges to speed up result enumeration
  std::shared_ptr<std::vector<std::map<TYPE_ENTITY_LITERAL_ID,std::set<TYPE_ENTITY_LITERAL_ID> >>> non_tree_edges_lists_;
  std::shared_ptr<map<std::string, double>> coefficients;
  shared_ptr<Transaction> txn;
  std::shared_ptr<stringstream> ss;
};

std::map<TYPE_ENTITY_LITERAL_ID,FQIterator*>  inline AssemblingFrOw(set<TYPE_ENTITY_LITERAL_ID> &fq_ids,
                                                                    std::map<TYPE_ENTITY_LITERAL_ID,double>* node_scores, int k,
                                                                    vector<std::map<TYPE_ENTITY_LITERAL_ID, OrderedList *>> &descendents_FRs);

void inline AddRelation(TYPE_ENTITY_LITERAL_ID x,TYPE_ENTITY_LITERAL_ID y, std::map<TYPE_ENTITY_LITERAL_ID ,std::set<TYPE_ENTITY_LITERAL_ID >>& mapping);

std::map<TYPE_ENTITY_LITERAL_ID ,OrderedList*>  GenerateIteratorNode(int parent_var,int child_var,std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,
                                                                     std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
                                                                TopKTreeNode *child_tree_node,Env *env);

FRIterator* BuildIteratorTree(const shared_ptr<TopKTreeSearchPlan> &tree_search_plan,Env *env);

void UpdateIDList(const shared_ptr<IDList>& valid_id_list, unsigned* id_list, unsigned id_list_len,bool id_list_prepared);

void FreeGlobalIterators(std::shared_ptr<std::vector<std::shared_ptr<std::set<OrderedList*>>>> global_iterators);
}
#endif //GSTORELIMITK_QUERY_TOPK_TOPKUTIL_H_
