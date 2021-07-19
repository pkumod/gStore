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

// only Vars
struct TopKTreeNode{
  int var_id;
  std::vector<TopKTreeNode*> descendents_;
};

class TopKTreeSearchPlan {
 private:
  std::size_t count_depth(std::map<int,set<int>>& neighbours,TYPE_ENTITY_LITERAL_ID root_id,std::size_t total_vars_num);
 public:
  // Choose A shortest tree to do top-k
  explicit TopKTreeSearchPlan(BasicQuery* basic_query, Statistics *statistics, QueryTree::Order expression);
  // The first tree to search
  TopKTreeNode* tree_root_;
  // Recursive delete
  ~TopKTreeSearchPlan();
  // The Edges that left behind
  std::vector<std::pair<int,int>> postponed_edges_;
};

namespace TopKUtil{

void CalculatePosVarMappingNode(TopKTreeNode* top_k_tree_node,shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> pos_var_mapping);
shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> CalculatePosVarMapping(shared_ptr<TopKTreeSearchPlan> search_plan);

double GetScore(string &v, stringstream &ss);
void GetVarCoefficientsTreeNode(QueryTree::CompTreeNode *comp_tree_node,
                                std::map<std::string,double>& coefficients,
                                stringstream &ss);
std::map<std::string,double> getVarCoefficients(QueryTree::Order order);

struct Env{
  KVstore *kv_store;
  BasicQuery *basic_query;
  shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> id_caches;
  int k;
  std::vector<std::map<TYPE_ENTITY_LITERAL_ID,OrderedList*>> *global_iterators;
  map<std::string, double> *coefficients;
  shared_ptr<Transaction> txn;
  stringstream *ss;
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


}
#endif //GSTORELIMITK_QUERY_TOPK_TOPKUTIL_H_
