/*=============================================================================
# Filename: QueryPlan.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "../Util/Util.h"
#include "../Query/BasicQuery.h"
#include "../Database/TableOperator.h"
#include "../Database/ResultTrigger.h"

#ifndef GSTORELIMITK_QUERY_QUERYPLAN_H_
#define GSTORELIMITK_QUERY_QUERYPLAN_H_

class QueryPlan
{
 public:
  std::shared_ptr<std::vector<OneStepJoin>> join_order_; //join order
  std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> ids_after_join_;
  std::shared_ptr<std::vector<VarDescriptor>> var_descriptors_;

  QueryPlan(std::shared_ptr<std::vector<OneStepJoin>> ,std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>>,std::shared_ptr<std::vector<VarDescriptor>>);

  QueryPlan(BasicQuery*,KVstore*,std::shared_ptr<std::vector<VarDescriptor>>);
};

#endif //GSTORELIMITK_QUERY_QUERYPLAN_H_
