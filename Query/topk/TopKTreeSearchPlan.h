//
// Created by Yuqi Zhou on 2021/7/10.
//

#ifndef GSTORELIMITK_QUERY_TOPK_TOPKTREESEARCHPLAN_H_
#define GSTORELIMITK_QUERY_TOPK_TOPKTREESEARCHPLAN_H_

#include "../../Util/Util.h"
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


#endif //GSTORELIMITK_QUERY_TOPK_TOPKTREESEARCHPLAN_H_
