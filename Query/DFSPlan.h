/*=============================================================================
# Filename: DFSPlan.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/
#ifndef GSTORELIMITK_QUERY_QUERYPLAN_H_
#define GSTORELIMITK_QUERY_QUERYPLAN_H_

#include "../Database/TableOperator.h"
#include "../Database/PlanTree.h"

class DFSPlan
{
 public:
  std::shared_ptr<std::vector<std::shared_ptr<StepOperation>>> join_order_; //join order
  DFSPlan(Tree_node* root_node);

  DFSPlan(PlanTree* plan_tree);
  DFSPlan()=default;
  void PreTravel(Tree_node *node);

};



#endif //GSTORELIMITK_QUERY_QUERYPLAN_H_
