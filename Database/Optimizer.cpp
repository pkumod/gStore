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
