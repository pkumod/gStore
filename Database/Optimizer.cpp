/*=============================================================================
# Filename: TableOperator.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "Optimizer.h"

QueryPlan::QueryPlan(shared_ptr<vector<OneStepJoin>> join_order, shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> ids_after_join) {
  // Copy Construction function of Vector
  // To avoid join_order_ and ids_after_join_ be adjusted by outer env
  this->join_order_ = make_shared<vector<OneStepJoin>>(*join_order);
  this->ids_after_join_ = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>(*ids_after_join);
}

Optimizer::Optimizer(KVstore *kv_store,
                     VSTree *vs_tree,
                     TYPE_TRIPLE_NUM *pre2num,
                     TYPE_TRIPLE_NUM *pre2sub,
                     TYPE_TRIPLE_NUM *pre2obj,
                     TYPE_PREDICATE_ID limitID_predicate,
                     TYPE_ENTITY_LITERAL_ID limitID_literal,
                     TYPE_ENTITY_LITERAL_ID limitID_entity,
                     bool is_distinct,
                     shared_ptr<Transaction> txn,
                     SPARQLquery &sparql_query,
                     shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> order_by_list,
                     TYPE_ENTITY_LITERAL_ID limit_num):
                     kv_store_(kv_store),vstree_(vs_tree),pre2num_(pre2num),
                     pre2sub_(pre2obj),pre2obj_(pre2obj),limitID_predicate_(limitID_predicate),
                     limitID_literal_(limitID_literal),limitID_entity_(limitID_entity),
                     is_distinct_(is_distinct),txn_(txn),order_by_list_(order_by_list),limit_num_(limit_num){

  this->current_basic_query_ = -1; // updated by result_list.size()
  this->basic_query_list_= make_shared<vector<shared_ptr<BasicQuery>>>();

  auto basic_query_vector = sparql_query.getBasicQueryVec();
  for(auto basic_query_pointer :basic_query_vector)
  {
    auto basic_query_shared_pointer = make_shared<BasicQuery>(*basic_query_pointer);
    this->basic_query_list_->push_back(basic_query_shared_pointer);
  }

  this->candidate_plans_ = make_shared<vector<tuple<shared_ptr<BasicQuery>, shared_ptr<vector<QueryPlan>>>>>(basic_query_list_->size());

  this->execution_plan_=make_shared<vector<QueryPlan>>();
  this->result_list_=make_shared<vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>(); // vector<unsigned*>* result_list;


  // TODO: join_cache_ & cardinality_cache_ not implemented yet.
  this->join_cache_ = make_shared<vector<map<shared_ptr<BasicQuery>,vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>>>(); // map(sub-structure, result_list)
  this->cardinality_cache_ = make_shared<vector<map<shared_ptr<BasicQuery>,shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>>(); // map(sub-structure, cardinality), not in statistics

}

// Inherit from Join::join_two
bool Optimizer::JoinOneStep(shared_ptr<QueryPlan> query_plan, shared_ptr<IntermediateResult> intermediate_result, shared_ptr<ResultTrigger> result_trigger) {
  return false;
}

/**
 *
 * @param one_step_join_node_
 * @param intermediate_result
 * @param rollback_trigger
 * @return
 */
tuple<bool,shared_ptr<IntermediateResult>> Optimizer::JoinANode(shared_ptr<OneStepJoinNode> one_step_join_node_, shared_ptr<IntermediateResult> intermediate_result,shared_ptr<ResultTrigger> rollback_trigger) {


  auto new_intermediate_table = make_shared<IntermediateResult>(intermediate_result->id_to_position_,
                                                                intermediate_result->position_to_id_,
                                                                intermediate_result->dealed_triple_);

  auto new_id_position = intermediate_result->id_to_position_->size();
  (*(new_intermediate_table->id_to_position_))[one_step_join_node_->node_to_join_] = new_id_position;
  (*(new_intermediate_table->position_to_id_))[new_id_position] = one_step_join_node_->node_to_join_;

  auto edges_info = one_step_join_node_->edges_;

  /* : each record */
  for (auto record_iterator = intermediate_result->values_->begin();
       record_iterator != intermediate_result->values_->end();
       record_iterator++) {
    auto record_candidate_list = make_shared<IDList>();
    auto record_candidate_prepared = false;
    /* for each edge */
    for (auto edge_info:*edges_info) {
      if (record_candidate_prepared && record_candidate_list->empty())
        continue;
      TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
      TYPE_ENTITY_LITERAL_ID edge_list_len;
      /* through this edge get candidate */
      switch (edge_info.join_method_) {
        case s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
          auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info.s_];
          auto s_var_id_this_record = (**record_iterator)[s_var_position];
          this->kv_store_->getpreIDlistBysubID(s_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case s2o: {
          auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info.s_];
          auto s_var_id_this_record = (**record_iterator)[s_var_position];
          this->kv_store_->getobjIDlistBysubID(s_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case p2s: {
          auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info.p_];
          auto p_var_id_this_record = (**record_iterator)[p_var_position];
          this->kv_store_->getsubIDlistBypreID(p_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case p2o: {
          auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info.p_];
          auto p_var_id_this_record = (**record_iterator)[p_var_position];
          this->kv_store_->getobjIDlistBypreID(p_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case o2s: {
          auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info.o_];
          auto o_var_id_this_record = (**record_iterator)[o_var_position];
          this->kv_store_->getsubIDlistByobjID(o_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case o2p: {
          auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info.o_];
          auto o_var_id_this_record = (**record_iterator)[o_var_position];
          this->kv_store_->getpreIDlistByobjID(o_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case so2p: {
          auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info.s_];
          auto s_var_id_this_record = (**record_iterator)[s_var_position];
          auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info.o_];
          auto o_var_id_this_record = (**record_iterator)[o_var_position];
          this->kv_store_->getpreIDlistBysubIDobjID(s_var_id_this_record,
                                                    o_var_id_this_record,
                                                    edge_candidate_list,
                                                    edge_list_len,
                                                    true,
                                                    this->txn_);
          break;
        }
        case sp2o: {
          auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info.s_];
          auto s_var_id_this_record = (**record_iterator)[s_var_position];
          auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info.p_];
          auto p_var_id_this_record = (**record_iterator)[p_var_position];
          this->kv_store_->getobjIDlistBysubIDpreID(s_var_id_this_record,
                                                    p_var_id_this_record,
                                                    edge_candidate_list,
                                                    edge_list_len,
                                                    true,
                                                    this->txn_);
          break;
        }
        case po2s: {
          auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info.o_];
          auto o_var_id_this_record = (**record_iterator)[o_var_position];
          auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info.p_];
          auto p_var_id_this_record = (**record_iterator)[p_var_position];
          this->kv_store_->getsubIDlistByobjIDpreID(o_var_id_this_record,
                                                    p_var_id_this_record,
                                                    edge_candidate_list,
                                                    edge_list_len,
                                                    true,
                                                    this->txn_);
          break;
        }
      }

      Optimizer::UpdateIDList(record_candidate_list,
                              edge_candidate_list,
                              edge_list_len,
                              record_candidate_prepared);
      delete [] edge_candidate_list;
      /* If candidate is not prepared, then we ues the first edge as candidate */
      record_candidate_prepared = true;

    }

    /* write to the new table */
    auto record = *record_iterator;
    for (auto new_element:*(record_candidate_list->getList())) {
      decltype(record) new_record(record);
      new_record->push_back(new_element);
      new_intermediate_table->values_->push_back(new_record);
    }
  }


  /* if result is too large, roll back */
  if (rollback_trigger->RollbackTrigger(new_intermediate_table->values_->size()))
  {
    return make_tuple(false,new_intermediate_table);
  }
  else
  {
    return make_tuple(true,new_intermediate_table);
  }
}


/**
 * join two table with vars in one_step_join_table
 * @param one_step_join_table: define join vars
 * @param table_a
 * @param table_b
 * @param result_trigger : if the result size is preferred
 * @return
 */
tuple<bool, shared_ptr<IntermediateResult>> Optimizer::JoinTwoTable(shared_ptr<OneStepJoinTable> one_step_join_table,
                                                                    shared_ptr<IntermediateResult> table_a,
                                                                    shared_ptr<IntermediateResult> table_b,
                                                                    shared_ptr<ResultTrigger> result_trigger) {
  if(table_a->values_->size()==0 || table_b->values_->size()==0)
    return make_tuple(false,make_shared<IntermediateResult>());

  auto join_nodes = one_step_join_table->public_variables_;
  /* build index in a
   * cost(a) = a log a + b log a
   * build index i b
   * cost(b) = b log b + a log b
   *
   * if a < b
   * cost(b) - cost(a)
   * = b log b - a log a + a log b - b log a
   * > b log b - a log b + a log b - b log a
   * = b (logb - log a)
   * > 0
   *
   * So, we build index in smaller table
   * */
  auto small_table = table_a->values_->size() < table_b->values_->size()? table_a:table_b;
  auto large_table = table_a->values_->size() > table_b->values_->size()? table_a:table_b;

  auto result_table = IntermediateResult::JoinTwoStructure(small_table,large_table,one_step_join_table->public_variables_);

  auto indexed_result =map<
      /*key*/ vector<TYPE_ENTITY_LITERAL_ID>,
      /*value*/ shared_ptr<vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>,
      /*compare*/ IndexedRecordResultCompare
      >();

  vector<TYPE_ENTITY_LITERAL_ID> common_variables_position_small;
  vector<TYPE_ENTITY_LITERAL_ID> common_variables_position_lager;
  for(auto common_variable:*(one_step_join_table->public_variables_))
  {
    common_variables_position_small.push_back((*(small_table->id_to_position_))[common_variable]);
    common_variables_position_lager.push_back((*(large_table->id_to_position_))[common_variable]);
  }

  auto common_variables_size = one_step_join_table->public_variables_->size();
  for(auto small_record:*(small_table->values_))
  {
    vector<TYPE_ENTITY_LITERAL_ID> result_index(common_variables_size);
    for(auto common_position:common_variables_position_small)
    {
      result_index.push_back((*small_record)[common_position]);
    }
    if(indexed_result.find(result_index)!=indexed_result.end())
    {
      indexed_result[result_index]->push_back(small_record);
    }
    else
    {
      auto tmp_vector = make_shared<vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>();
      tmp_vector->push_back(small_record);
      indexed_result[result_index] = tmp_vector;
    };
  }


  /* Now Do the Matching , first calculate which ids in table large should be add
   * */
  set<TYPE_ENTITY_LITERAL_ID> public_variables;
  for(auto variable_id:*join_nodes)
  {
    public_variables.insert(variable_id);
  }
  vector<TYPE_ENTITY_LITERAL_ID> large_table_inserted_variables_position;
  for(auto b_kv:*large_table->position_to_id_)
  {
    if (public_variables.find(b_kv.second)!=public_variables.end())
      continue;
    large_table_inserted_variables_position.push_back(b_kv.first);
  }

  auto result_contents = result_table->values_;
  /* do the matching */
  for(auto large_record:*(large_table->values_))
  {
    vector<TYPE_ENTITY_LITERAL_ID> result_index(common_variables_size);
    for(auto common_position:common_variables_position_lager)
    {
      result_index.push_back((*large_record)[common_position]);
    }
    /* the index is in the small table */
    if(indexed_result.find(result_index)!=indexed_result.end())
    {
      vector<TYPE_ENTITY_LITERAL_ID> large_record_inserted;
      for(auto large_position:large_table_inserted_variables_position)
      {
        large_record_inserted.push_back((*large_record)[large_position]);
      }
      auto matched_content = indexed_result[result_index];
      for(auto matched_small_record:*matched_content)
      {
        auto result_record = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>(*matched_small_record);
        for(auto large_inserted_element:large_record_inserted)
        {
          result_record->push_back(large_inserted_element);
        }
        result_contents->push_back(result_record);
      }
    }
    /* if not, ignore */
  }

  /* if result is too large, roll back */
  if (result_trigger->RollbackTrigger(result_table->values_->size()))
  {
    return make_tuple(false,result_table);
  }
  else
  {
    return make_tuple(true,result_table);
  }
}

tuple<bool, shared_ptr<IntermediateResult>> Optimizer::EdgeConstraintFilter(shared_ptr<EdgeInfo> edge_info,
                                                                            EdgeInTableInfo edge_table_info,
                                                                            shared_ptr<IntermediateResult> intermediate_result,
                                                                            shared_ptr<ResultTrigger> result_trigger) {
  TYPE_ENTITY_LITERAL_ID var_to_filter;
  switch (edge_info->join_method_) {
    case s2p:
      var_to_filter = edge_info->p_;
      break;

    case s2o:
      var_to_filter = edge_info->o_;
      break;

    case p2s:
      var_to_filter = edge_info->s_;
      break;

    case p2o:
      var_to_filter = edge_info->o_;
      break;

    case o2s:
      var_to_filter = edge_info->s_;
      break;

    case o2p:
      var_to_filter = edge_info->p_;
      break;

    case so2p:
      var_to_filter = edge_info->p_;
      break;

    case sp2o:
      var_to_filter = edge_info->o_;
      break;

    case po2s:
      var_to_filter = edge_info->s_;
      break;

  }


  if(edge_table_info.ConstantToVar(edge_info))
  {

    TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
    TYPE_ENTITY_LITERAL_ID this_edge_list_len;
    switch (edge_info->join_method_) {
      case s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
        auto s_var_constant_id = edge_info->s_;
        this->kv_store_->getpreIDlistBysubID(s_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case s2o: {
        auto s_var_constant_id = edge_info->s_;
        this->kv_store_->getobjIDlistBysubID(s_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case p2s: {
        auto p_var_constant_id = edge_info->p_;
        this->kv_store_->getsubIDlistBypreID(p_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case p2o: {
        auto p_var_constant_id = edge_info->p_;
        this->kv_store_->getobjIDlistBypreID(p_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case o2s: {
        auto o_var_constant_id = edge_info->o_;
        this->kv_store_->getsubIDlistByobjID(o_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case o2p: {
        auto o_var_constant_id = edge_info->o_;
        this->kv_store_->getpreIDlistByobjID(o_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case so2p: {
        auto s_var_constant_id = edge_info->s_;
        auto o_var_constant_id = edge_info->o_;
        this->kv_store_->getpreIDlistBysubIDobjID(s_var_constant_id,
                                                  o_var_constant_id,
                                                  edge_candidate_list,
                                                  this_edge_list_len,
                                                  true,
                                                  this->txn_);
        break;
      }
      case sp2o: {
        auto s_var_constant_id = edge_info->s_;
        auto p_var_constant_id = edge_info->p_;
        this->kv_store_->getobjIDlistBysubIDpreID(s_var_constant_id,
                                                  p_var_constant_id,
                                                  edge_candidate_list,
                                                  this_edge_list_len,
                                                  true,
                                                  this->txn_);
        break;
      }
      case po2s: {
        auto p_var_constant_id = edge_info->p_;
        auto o_var_constant_id = edge_info->o_;
        this->kv_store_->getsubIDlistByobjIDpreID(o_var_constant_id,
                                                  p_var_constant_id,
                                                  edge_candidate_list,
                                                  this_edge_list_len,
                                                  true,
                                                  this->txn_);
        break;
      }
    }
    auto validate_list = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>(edge_candidate_list,edge_candidate_list+this_edge_list_len);
    delete [] edge_candidate_list;
    return FilterAVariableOnIDList(validate_list,var_to_filter,intermediate_result,result_trigger);
  }


  auto new_intermediate_table = make_shared<IntermediateResult>(intermediate_result->id_to_position_,
                                                                intermediate_result->position_to_id_,
                                                                intermediate_result->dealed_triple_);

  auto r_table_content = new_intermediate_table->values_;
  /* : each record */
  for (auto record_iterator = intermediate_result->values_->begin(); record_iterator != intermediate_result->values_->end();record_iterator++) {
    TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
    TYPE_ENTITY_LITERAL_ID edge_list_len;
    switch (edge_info->join_method_) {
      case s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
        auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info->s_];
        auto s_var_id_this_record = (**record_iterator)[s_var_position];
        this->kv_store_->getpreIDlistBysubID(s_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);

        break;
      }
      case s2o: {
        auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info->s_];
        auto s_var_id_this_record = (**record_iterator)[s_var_position];
        this->kv_store_->getobjIDlistBysubID(s_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case p2s: {
        auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info->p_];
        auto p_var_id_this_record = (**record_iterator)[p_var_position];
        this->kv_store_->getsubIDlistBypreID(p_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case p2o: {
        auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info->p_];
        auto p_var_id_this_record = (**record_iterator)[p_var_position];
        this->kv_store_->getobjIDlistBypreID(p_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case o2s: {
        auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info->o_];
        auto o_var_id_this_record = (**record_iterator)[o_var_position];
        this->kv_store_->getsubIDlistByobjID(o_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case o2p: {
        auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info->o_];
        auto o_var_id_this_record = (**record_iterator)[o_var_position];
        this->kv_store_->getpreIDlistByobjID(o_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      /**
       * Notice: s or o may be constant
       */
      case so2p: {
        TYPE_ENTITY_LITERAL_ID  s_var_id_this_record;
        TYPE_ENTITY_LITERAL_ID  o_var_id_this_record;

        if(edge_table_info.s_constant_)
        {
          s_var_id_this_record = edge_info->s_;
        }
        else{
          auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info->s_];
          s_var_id_this_record = (**record_iterator)[s_var_position];
        }

        if(edge_table_info.o_constant_)
        {
          o_var_id_this_record = edge_info->o_;
        }
        else{
          auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info->o_];
          o_var_id_this_record = (**record_iterator)[o_var_position];
        }

        this->kv_store_->getpreIDlistBysubIDobjID(s_var_id_this_record,
                                                  o_var_id_this_record,
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  this->txn_);
        break;
      }
      case sp2o: {
        TYPE_ENTITY_LITERAL_ID  s_var_id_this_record;
        TYPE_ENTITY_LITERAL_ID  p_var_id_this_record;
        if(edge_table_info.s_constant_)
        {
          s_var_id_this_record = edge_info->s_;
        }
        else{
          auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info->s_];
          s_var_id_this_record = (**record_iterator)[s_var_position];
        }

        if(edge_table_info.p_constant_)
        {
          p_var_id_this_record = edge_info->p_;
        }
        else{
          auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info->p_];
          p_var_id_this_record = (**record_iterator)[p_var_position];
        }

        this->kv_store_->getobjIDlistBysubIDpreID(s_var_id_this_record,
                                                  p_var_id_this_record,
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  this->txn_);
        break;
      }
      case po2s: {
        TYPE_ENTITY_LITERAL_ID  p_var_id_this_record;
        TYPE_ENTITY_LITERAL_ID  o_var_id_this_record;

        if(edge_table_info.o_constant_)
        {
          o_var_id_this_record = edge_info->o_;
        }
        else{
          auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info->o_];
          o_var_id_this_record = (**record_iterator)[o_var_position];
        }

        if(edge_table_info.p_constant_)
        {
          p_var_id_this_record = edge_info->p_;
        }
        else{
          auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info->p_];
          p_var_id_this_record = (**record_iterator)[p_var_position];
        }

        this->kv_store_->getsubIDlistByobjIDpreID(o_var_id_this_record,
                                                  p_var_id_this_record,
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  this->txn_);
        break;
      }

    }

    /**
     * Now we get the valid candidate list of var_to_filter
     * do a binary to decide whether the record satisfy this edge
     * */

    auto var_to_filter_position = (*(intermediate_result->id_to_position_))[var_to_filter];
    auto var_to_filter_id_this_record = (**record_iterator)[var_to_filter_position];
    if(binary_search(edge_candidate_list,edge_candidate_list+edge_list_len,var_to_filter_id_this_record))
    {
      r_table_content->push_back(*record_iterator);
    }
    delete [] edge_candidate_list;
  }

  /* if result is too large, roll back */
  if (result_trigger->RollbackTrigger(new_intermediate_table->values_->size()))
  {
    return make_tuple(false,new_intermediate_table);
  }
  else
  {
    return make_tuple(true,new_intermediate_table);
  }
}

tuple<bool, shared_ptr<IntermediateResult>> Optimizer::FilterAVariableOnIDList(shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> candidate_list,
                                                                               TYPE_ENTITY_LITERAL_ID var_id,
                                                                               shared_ptr<IntermediateResult> intermediate_result,
                                                                               shared_ptr<ResultTrigger> result_trigger) {

  auto new_intermediate_table = make_shared<IntermediateResult>(intermediate_result->id_to_position_,
                                                                intermediate_result->position_to_id_,
                                                                intermediate_result->dealed_triple_);
  auto r_table_content = new_intermediate_table->values_;
  auto var_position = (*(intermediate_result->id_to_position_))[var_id];

  /* : each record */
  for (auto record_sp : *(intermediate_result->values_)){
    auto var_to_filter_id_this_record=(*record_sp)[var_position];
    if(binary_search(candidate_list->begin(),candidate_list->end(),var_to_filter_id_this_record))
    {
      r_table_content->push_back(record_sp);
    }
  }

  if (result_trigger->RollbackTrigger(new_intermediate_table->values_->size()))
  {
    return make_tuple(false,new_intermediate_table);
  }
  else
  {
    return make_tuple(true,new_intermediate_table);
  }
}


/**
 * a simple version of Join::update_answer_list
 * @param valid_id_list
 * @param id_list
 * @param id_list_len
 * @param id_list_prepared : valid_id_list is initialed
 */
void
Optimizer::UpdateIDList(shared_ptr<IDList> valid_id_list, unsigned* id_list, unsigned id_list_len,bool id_list_prepared)
{
  if(id_list_prepared)
  {
    valid_id_list->intersectList(id_list, id_list_len);
  }
  else
  {
    valid_id_list->reserve(id_list_len);
    for(int i = 0; i < id_list_len; i++)
      valid_id_list->addID(id_list[i]);
  }
}

