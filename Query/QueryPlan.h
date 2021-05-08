/*=============================================================================
# Filename: QueryPlan.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "../Util/Util.h"
#include "../Query/BasicQuery.h"
#include "../Database/TableOperator.h"
#include "../Database/ResultTrigger.h"
#include "../Database/PlanTree.h"

#ifndef GSTORELIMITK_QUERY_QUERYPLAN_H_
#define GSTORELIMITK_QUERY_QUERYPLAN_H_

class QueryPlan
{
 public:
  std::shared_ptr<std::vector<OneStepJoin>> join_order_; //join order
  std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> ids_after_join_;
  std::shared_ptr<std::vector<VarDescriptor>> var_descriptors_;
  // Do Before the process begin
  std::shared_ptr<std::vector<std::shared_ptr<OneStepJoinNode>>> constant_generating_lists_;

  QueryPlan(const std::shared_ptr<std::vector<OneStepJoin>>& join_order,
            const std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>>&ids_after_join,
            std::shared_ptr<std::vector<VarDescriptor>> var_infos);

  QueryPlan(BasicQuery *basic_query,KVstore *kv_store,shared_ptr<vector<VarDescriptor>> var_infos);
  QueryPlan()=default;

  static std::shared_ptr<std::vector<std::shared_ptr<OneStepJoinNode>>> OnlyConstFilter(BasicQuery*,KVstore*,std::shared_ptr<std::vector<VarDescriptor>>);

  /* greedy method used in version 0.9 */
  static double ScoreNode(BasicQuery *basic_query, int var);
  static TYPE_ENTITY_LITERAL_ID SelectANode(BasicQuery *basic_query,std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> processed_nodes); //include select the start node and choose next node;

  std::string toString(KVstore* kv_store);

  /*  The return value are:
   *  [0] var_to_position_,
   *  [1] position_to_var
   * */
  std::tuple<std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>>,std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>>>
  PositionIDMappings();

  static shared_ptr<QueryPlan> DefaultBFS(BasicQuery*,KVstore*,std::shared_ptr<std::vector<VarDescriptor>>);

  static tuple<shared_ptr<vector<EdgeInfo>>,shared_ptr<vector<EdgeConstantInfo>>> LinkTwoNode(BasicQuery *basic_query,
                                                      const KVstore *kv_store,
                                                      TYPE_ENTITY_LITERAL_ID added_id,
                                                      TYPE_ENTITY_LITERAL_ID d_already_in_table);

  static OneStepJoin LinkWithPreviousNodes(BasicQuery *basic_query,
                                           const KVstore *kv_store,
                                           TYPE_ENTITY_LITERAL_ID added_id,
                                           const std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> &table_ids);
  static std::shared_ptr<OneStepJoinNode> FilterNodeOnConstantEdge(BasicQuery *basic_query,
                                           KVstore *kv_store,
                                           TYPE_ENTITY_LITERAL_ID target_node);
  static OneStepJoin FilterFirstNode(BasicQuery *basic_query, KVstore *kv_store, TYPE_ENTITY_LITERAL_ID start_node,
                                     const shared_ptr<vector<VarDescriptor>> &var_list);
  static void ProcessPredicateAndSatellites(BasicQuery *basic_query,
                                     KVstore *kv_store,
                                     vector<bool> &vars_used_vec,
                                     int graph_var_num,
                                     unsigned int pre_var_num,
                                     std::shared_ptr<std::vector<OneStepJoin>> &join_order,
                                     std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> &ids_after_join);
  static tuple<EdgeInfo,EdgeConstantInfo> WrapEdgeInfo_i_th_Edge(BasicQuery *basic_query,
                           const KVstore *kv_store,
                           TYPE_ENTITY_LITERAL_ID added_id,int edge_id);
};

#endif //GSTORELIMITK_QUERY_QUERYPLAN_H_
