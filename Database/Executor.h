/*=============================================================================
# Filename: Executor.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
# Last Modified:  2021/8/10.
=============================================================================*/
#ifndef GSTOREGDB_DATABASE_EXECUTOR_H_
#define GSTOREGDB_DATABASE_EXECUTOR_H_
#include "../Util/Util.h"
#include "../Query/SPARQLquery.h"
#include "../Query/BasicQuery.h"
#include "../Query/IDList.h"
#include "../KVstore/KVstore.h"
#include "../VSTree/VSTree.h"
#include "TableOperator.h"
#include "ResultTrigger.h"
#include "../Query/QueryPlan.h"
#include "../Query/QueryTree.h"
#include "Join.h"
#include "./Statistics.h"
#include "./PlanTree.h"
#include "./PlanGenerator.h"

using namespace std;

template <typename Container> // we can make this generic for any container [1]
struct container_hash{
  std::size_t operator()(Container const& c) const {
    return boost::hash_range(c.begin(), c.end());
  }
};

class Executor {
 private:
  KVstore *kv_store_;
  std::shared_ptr<Transaction> txn_;
  TYPE_PREDICATE_ID limitID_predicate_;
  TYPE_ENTITY_LITERAL_ID limitID_literal_;
  TYPE_ENTITY_LITERAL_ID limitID_entity_;
 public:
  Executor(KVstore *kv_store,std::shared_ptr<Transaction> txn,TYPE_PREDICATE_ID limitID_predicate,
           TYPE_ENTITY_LITERAL_ID limitID_literal,TYPE_ENTITY_LITERAL_ID limitID_entity):
           kv_store_(kv_store),txn_(txn),limitID_predicate_(limitID_predicate),limitID_literal_(limitID_literal),
           limitID_entity_(limitID_entity){};

  std::tuple<bool,TableContentShardPtr> GenerateColdCandidateList(const std::shared_ptr<std::vector<EdgeInfo>>& edge_info_vector,
                                                             const std::shared_ptr<std::vector<EdgeConstantInfo>>& edge_table_info_vector);

  std::tuple<bool,TableContentShardPtr> JoinANode(const std::shared_ptr<FeedOneNode>& one_step_join_node_,
                                             const TableContentShardPtr& table_content_ptr,
                                             const PositionValueSharedPtr& id_pos_mapping,
                                             const IDCachesSharePtr& id_caches);

  std::tuple<bool,TableContentShardPtr> JoinTwoNode(const std::shared_ptr<FeedTwoNode> join_two_node_,
                                                  const TableContentShardPtr& table_content_ptr,
                                                  const PositionValueSharedPtr& id_pos_mapping,
                                                  const IDCachesSharePtr& id_caches);

  std::tuple<bool,PositionValueSharedPtr,TableContentShardPtr> JoinTable(const std::shared_ptr<JoinTwoTable>& one_step_join_table,
                                                                    const TableContentShardPtr& table_a,
                                                                    const PositionValueSharedPtr& table_a_id_pos,
                                                                    const PositionValueSharedPtr& table_a_pos_id,
                                                                    const TableContentShardPtr& table_b,
                                                                    const PositionValueSharedPtr& table_b_id_pos,
                                                                    const PositionValueSharedPtr& table_b_pos_id);
  std::tuple<bool,TableContentShardPtr> ANodeEdgesConstraintFilter(const std::shared_ptr<FeedOneNode>&,
      TableContentShardPtr,const PositionValueSharedPtr&,const IDCachesSharePtr&);
  std::tuple<bool,TableContentShardPtr> OneEdgeConstraintFilter(EdgeInfo, EdgeConstantInfo, const TableContentShardPtr&,const PositionValueSharedPtr&,const IDCachesSharePtr&);
  std::tuple<bool,TableContentShardPtr> FilterAVariableOnIDList(const std::shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>&,TYPE_ENTITY_LITERAL_ID ,const TableContentShardPtr&,const PositionValueSharedPtr&);
  std::shared_ptr<IntermediateResult> NormalJoin(std::shared_ptr<BasicQuery>,std::shared_ptr<QueryPlan>);
  bool CacheConstantCandidates(const std::shared_ptr<FeedOneNode>& one_step, const IDCachesSharePtr& id_caches);
  bool AddConstantCandidates(EdgeInfo edge_info,EdgeConstantInfo edge_table_info,TYPE_ENTITY_LITERAL_ID targetID, const IDCachesSharePtr& id_caches);
  std::tuple<bool, TableContentShardPtr> getAllSubObjID(bool need_literal);

  static void UpdateIDList(const std::shared_ptr<IDList>& valid_id_list, unsigned* id_list, unsigned id_list_len,bool id_list_prepared);

  std::shared_ptr<IDList> ExtendRecordOneNode(const shared_ptr<FeedOneNode> &one_step_join_node_,
                                              const PositionValueSharedPtr &id_pos_mapping,
                                              const IDCachesSharePtr &id_caches,
                                              TYPE_ENTITY_LITERAL_ID new_id,
                                              list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>::iterator &record_iterator) const;

  std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> ExtendRecordTwoNode(const shared_ptr<FeedTwoNode> one_step_join_node_,
                                              const PositionValueSharedPtr &id_pos_mapping,
                                              const IDCachesSharePtr &id_caches,
                                              TYPE_ENTITY_LITERAL_ID new_id1,
                                              TYPE_ENTITY_LITERAL_ID new_id2,
                                              list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>::iterator &record_iterator) const;


  shared_ptr<IDList> CandidatesWithConstantEdge(const shared_ptr<vector<EdgeInfo>> &edge_info_vector) const;


};

#endif //GSTOREGDB_DATABASE_EXECUTOR_H_
