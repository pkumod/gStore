/*=============================================================================
# Filename: QueryPlan.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "QueryPlan.h"

QueryPlan::QueryPlan(PlanTree *plan_tree) {
  // Do A depth first search
  auto root = plan_tree->root_node;
  stack<decltype(root)> q;
  q.push(root);
  while(!q.empty())
  {
    auto top_node = q.top();
    if(top_node->node->op_type_ == StepOperation::StepOpType::Filter)
      this->constant_generating_lists_->push_back(top_node->node->join_node_);
    else
      this->join_order_->push_back(top_node->node);
    if(top_node->left_node!= nullptr)
      q.push(top_node->left_node);
    if(top_node->right_node!= nullptr)
      q.push(top_node->right_node);
    q.pop();
  }
}

void QueryPlan::PreTravel(Tree_node *node)
{
  if(node->left_node!= nullptr)
    PreTravel(node->left_node);
  if(node->right_node!= nullptr)
    PreTravel(node->right_node);
  this->join_order_->push_back(node->node);
}

QueryPlan::QueryPlan(Tree_node *root_node) {
  this->join_order_= make_shared<std::vector<shared_ptr<StepOperation>>>();
  this->constant_generating_lists_= make_shared<vector<std::shared_ptr<AffectOneNode>>>();
  PreTravel(root_node);
}

