/*=============================================================================
# Filename: QueryPlan.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/
#ifndef GSTORELIMITK_QUERY_QUERYPLAN_H_
#define GSTORELIMITK_QUERY_QUERYPLAN_H_

#include "../Database/TableOperator.h"
#include "../Database/PlanTree.h"

class QueryPlan
{
 public:
  std::shared_ptr<std::vector<std::shared_ptr<StepOperation>>> join_order_; //join order
  // Do Before the process begin
  std::shared_ptr<std::vector<std::shared_ptr<AffectOneNode>>> constant_generating_lists_;

  QueryPlan(Tree_node* root_node);

  QueryPlan(PlanTree* plan_tree);
  QueryPlan()=default;
  void PreTravel(Tree_node *node);


};



#endif //GSTORELIMITK_QUERY_QUERYPLAN_H_
