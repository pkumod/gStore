//
// Created by Yuqi Zhou on 2022/11/9.
//

#ifndef GSTORECODECN_QUERY_FILTERPLAN_H_
#define GSTORECODECN_QUERY_FILTERPLAN_H_

#include "../Database/TableOperator.h"
#include "../Database/PlanTree.h"

class FilterPlan {
 public:
  static std::shared_ptr<std::vector<std::shared_ptr<AffectOneNode>>> OnlyConstFilter(std::shared_ptr<BGPQuery> bgp_query,
  KVstore *kv_store);

  static std::shared_ptr<std::vector<std::shared_ptr<AffectOneNode>>> PredicateFilter(std::shared_ptr<BGPQuery> bgp_query,
  KVstore *kv_store);


  static shared_ptr<AffectOneNode> FilterNodeOnConstantEdge(shared_ptr<BGPQuery> bgp_query,
                                                            KVstore *kv_store,
                                                            TYPE_ENTITY_LITERAL_ID target_node);

  static shared_ptr<AffectOneNode> FilterNodeOnConstantPredicate(shared_ptr<BGPQuery> bgp_query,
                                                                 KVstore *kv_store,
                                                                 TYPE_ENTITY_LITERAL_ID target_node);

};

#endif //GSTORECODECN_QUERY_FILTERPLAN_H_
