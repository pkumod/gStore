/*=============================================================================
# Filename: Executor.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
# Last Modified:  2021/8/10.
=============================================================================*/
#ifndef GSTOREGDB_DATABASE_EXECUTOR_H_
#define GSTOREGDB_DATABASE_EXECUTOR_H_

// #include "../Query/SPARQLquery.h"
#include "../Query/IDList.h"
#include "TableOperator.h"
// #include "../Query/DFSPlan.h"
#include "../Query/QueryTree.h"
#include "Join.h"

namespace gstore
{

using namespace std;

#define EXECUTOR_DEBUG_INFO

template <typename Container> // we can make this generic for any container [1]
struct container_hash{
  std::size_t operator()(Container const& c) const {
    return boost::hash_range(c.begin(), c.end());
  }
};

/**
 * Operation for Table is doing here.
 * any operation to write id_pos/pos_id map should be in here.
 * IntermediateResult is the standard format Executor communicates
 * with others
 */
class Executor {
 private:
  KVstore *kv_store_;
  std::shared_ptr<Transaction> txn_;
  TYPE_PREDICATE_ID limitID_predicate_;
  TYPE_ENTITY_LITERAL_ID limitID_literal_;
  TYPE_ENTITY_LITERAL_ID limitID_entity_;


  tuple<bool,TableContentShardPtr> OneEdgeConstraintFilter(EdgeInfo edge_info,
                                                           EdgeConstantInfo edge_table_info,
                                                           TableContentShardPtr table_content_ptr,
                                                           PositionValueSharedPtr id_pos_mapping,
                                                           bool remain_old);

  tuple<bool,TableContentShardPtr> FilterAVariableOnIDList(shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> candidate_list,
                                                           TYPE_ENTITY_LITERAL_ID var_id,
                                                           TableContentShardPtr table_content_ptr,
                                                           PositionValueSharedPtr id_posing_mapping,
                                                           bool remain_old);

  TableContentShardPtr ConvertToTable(std::shared_ptr<IDList> id_list, size_t max_output = NO_LIMIT_OUTPUT);

  bool AddConstantCandidates(EdgeInfo edge_info,TYPE_ENTITY_LITERAL_ID targetID,bool distinct,IDCachesSharePtr id_caches);

  std::tuple<bool, TableContentShardPtr> GetAllSubObjId(bool need_literal,size_t max_output);
  std::tuple<bool, TableContentShardPtr> GetAllPreId(size_t max_output);

  std::shared_ptr<IDList> ExtendRecordOneNode(shared_ptr<AffectOneNode> one_step_join_node_,
                                              PositionValueSharedPtr id_pos_mapping,
                                              IDCachesSharePtr id_caches,
                                              TYPE_ENTITY_LITERAL_ID new_id,
                                              bool distinct,
                                              list<vector<TYPE_ENTITY_LITERAL_ID>>::iterator record_iterator) const;

  std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> ExtendRecordTwoNode(shared_ptr<AffectTwoNode> one_step_join_node_,
                                                                           PositionValueSharedPtr id_pos_mapping,
                                                                           IDCachesSharePtr id_caches,
                                                                           TYPE_ENTITY_LITERAL_ID new_id1,
                                                                           TYPE_ENTITY_LITERAL_ID new_id2,
                                                                           list<vector<TYPE_ENTITY_LITERAL_ID>>::iterator record_iterator) const;

  shared_ptr<IDList> CandidatesWithConstantEdge(shared_ptr<vector<EdgeInfo>> edge_info_vector,bool distinct) const;

 public:
  static constexpr auto NO_LIMIT_OUTPUT = static_cast<size_t>(-1);

  Executor(KVstore *kv_store,std::shared_ptr<Transaction> txn,TYPE_PREDICATE_ID limitID_predicate,
           TYPE_ENTITY_LITERAL_ID limitID_literal,TYPE_ENTITY_LITERAL_ID limitID_entity):
           kv_store_(kv_store),txn_(txn),limitID_predicate_(limitID_predicate),limitID_literal_(limitID_literal),
           limitID_entity_(limitID_entity){};

  void CheckedInsert(TableContentShardPtr& new_records,
                     size_t &max_output,
                     bool remain_old,
                     vector<TYPE_ENTITY_LITERAL_ID>& inserted_output);
  tuple<bool, IntermediateResult> AffectANode(IntermediateResult old_table,
                                              const IDCachesSharePtr id_caches,
                                              bool node_added_into_table,
                                              bool distinct,
                                              bool remain_old,
                                              size_t max_output,
                                              const shared_ptr<AffectOneNode> feed_plan);

  std::tuple<bool,IntermediateResult> InitialTableOneNode(std::shared_ptr<AffectOneNode> feed_plan,
                                                          bool is_entity,
                                                          bool is_literal,
                                                          bool is_predicate,
                                                          bool distinct,
                                                          size_t max_output,
                                                          IDCachesSharePtr id_caches);

  std::tuple<bool,IntermediateResult> InitialTableTwoNode(std::shared_ptr<AffectTwoNode> join_plan,
                                                          size_t max_output,
                                                          IDCachesSharePtr id_caches);

  std::tuple<bool, IntermediateResult> JoinTwoNode(shared_ptr<AffectTwoNode> join_two_node_,
                                                   IntermediateResult old_table,
                                                   IDCachesSharePtr id_caches,
                                                   bool remain_old,
                                                   size_t max_output);

  tuple<bool,IntermediateResult> JoinTable(shared_ptr<JoinTwoTable> join_plan,
                                           IntermediateResult table_a,
                                           IntermediateResult table_b,
                                           bool remain_old);

  tuple<bool,IntermediateResult> ANodeEdgesConstraintFilter(shared_ptr<AffectOneNode> check_plan,
                                                            IntermediateResult old_table,
                                                            bool remain_old) ;

  bool CacheConstantCandidates(shared_ptr<AffectOneNode> one_step, bool distinct, IDCachesSharePtr id_caches);

  static void UpdateIDList(const std::shared_ptr<IDList>& valid_id_list, unsigned* id_list, unsigned id_list_len,bool id_list_prepared);

  bool UpdateIDCache(shared_ptr<AffectOneNode> affect_one_node, IDCachesSharePtr id_caches, bool distinct);

  std::tuple<bool,IntermediateResult> GetAllTriple(size_t max_output, std::shared_ptr<AffectOneNode> feed_one_node);
};
} // end namespace gstore
#endif //GSTOREGDB_DATABASE_EXECUTOR_H_
