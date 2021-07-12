//
// Created by Yuqi Zhou on 2021/7/10.
//

#ifndef GSTORELIMITK_QUERY_TOPK_TOPKUTIL_H_
#define GSTORELIMITK_QUERY_TOPK_TOPKUTIL_H_

#include "../../Util/Util.h"
#include "../../KVstore/KVstore.h"
#include "./ElementList.h"
#include "../../Query/SPARQLquery.h"
#include "../../Query/BasicQuery.h"
#include "../../Database/Statistics.h"
#include "../../Query/QueryTree.h"
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

void getVarCoefficientsTreeNode(QueryTree::CompTreeNode *comp_tree_node,
                                          std::map<std::string,double>& coefficients,
                                          stringstream &ss);
std::map<std::string,double> getVarCoefficients(QueryTree::Order order);
OrderedList* BuildIteratorTree(KVstore* kv_store,BasicQuery* basic_query,
                              const shared_ptr<TopKTreeSearchPlan> &tree_search_plan,
                               std::map<TYPE_ENTITY_LITERAL_ID,OrderedList*>& iterators,
                               std::map<TYPE_ENTITY_LITERAL_ID,TreeResultSet*>& elements_lists,
                               std::map<std::string,double>& coefficients);
}
#endif //GSTORELIMITK_QUERY_TOPK_TOPKUTIL_H_
