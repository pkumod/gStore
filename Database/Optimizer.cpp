/*=============================================================================
# Filename: Optimizer.cpp
# Author: Yuqi Zhou, Linglin Yang
# Mail: zhouyuqi@pku.edu.cn, fyulingi@gmail.com
=============================================================================*/

#include "Optimizer.h"

Optimizer::Optimizer(KVstore *kv_store,
                     Statistics *statistics,
                     TYPE_TRIPLE_NUM *pre2num,
                     TYPE_TRIPLE_NUM *pre2sub,
                     TYPE_TRIPLE_NUM *pre2obj,
                     TYPE_PREDICATE_ID limitID_predicate,
                     TYPE_ENTITY_LITERAL_ID limitID_literal,
                     TYPE_ENTITY_LITERAL_ID limitID_entity,
                     shared_ptr<Transaction> txn
):
    kv_store_(kv_store), statistics(statistics), pre2num_(pre2num),
    pre2sub_(pre2obj),pre2obj_(pre2obj),limitID_predicate_(limitID_predicate),
    limitID_literal_(limitID_literal),limitID_entity_(limitID_entity),
    txn_(std::move(txn))
{
/*
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
*/

  // TODO: join_cache_ & cardinality_cache_ not implemented yet.
  this->join_cache_ = make_shared<vector<map<BasicQuery*,vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>>>(); // map(sub-structure, result_list)
  this->cardinality_cache_ = make_shared<vector<map<BasicQuery*,shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>>(); // map(sub-structure, cardinality), not in statistics
  this->var_descriptors_ = make_shared<vector<VarDescriptor>>();
}


/**
 * generate a var candidate based on its constant edge/neighbour
 * @param edge_info_vector
 * @param edge_table_info_vector
 * @return
 */
tuple<bool, TableContentShardPtr> Optimizer::GenerateColdCandidateList(const shared_ptr<vector<EdgeInfo>>& edge_info_vector,
                                                                       const shared_ptr<vector<EdgeConstantInfo>>& edge_table_info_vector) {

  if(edge_info_vector->empty())
    return make_tuple(false, nullptr);


  auto var_to_filter = (*edge_info_vector)[0].getVarToFilter();

  assert(edge_info_vector->size()==edge_table_info_vector->size());

  auto id_candidate = CandidatesWithConstantEdge(edge_info_vector);

  auto result = make_shared<TableContent>();
  auto id_candidate_vec = id_candidate->getList();
  for(auto var_id: *id_candidate_vec)
  {
    auto record = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();
    record->push_back(var_id);
    result->push_back(record);
  }
  return make_tuple(true,result);
}


shared_ptr<IDList> Optimizer::CandidatesWithConstantEdge(const shared_ptr<vector<EdgeInfo>> &edge_info_vector) const {
  auto id_candidate = make_shared<IDList>();
  for(int i = 0; i<edge_info_vector->size(); i++)
  {
    auto edge_info = (*edge_info_vector)[i];
    TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
    TYPE_ENTITY_LITERAL_ID this_edge_list_len;
    cout<<"edge["<<i<<"] \n\t"<< edge_info.toString() << "\n\t join method:"<<JoinMethodToString(edge_info.join_method_)<<endl;
    switch (edge_info.join_method_) {
      case JoinMethod::s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
        auto s_var_constant_id = edge_info.s_;
        kv_store_->getpreIDlistBysubID(s_var_constant_id,
                                       edge_candidate_list,
                                       this_edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::s2o: {
        auto s_var_constant_id = edge_info.s_;
        kv_store_->getobjIDlistBysubID(s_var_constant_id,
                                       edge_candidate_list,
                                       this_edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::p2s: {
        auto p_var_constant_id = edge_info.p_;
        kv_store_->getsubIDlistBypreID(p_var_constant_id,
                                       edge_candidate_list,
                                       this_edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::p2o: {
        auto p_var_constant_id = edge_info.p_;
        kv_store_->getobjIDlistBypreID(p_var_constant_id,
                                       edge_candidate_list,
                                       this_edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::o2s: {
        auto o_var_constant_id = edge_info.o_;
        kv_store_->getsubIDlistByobjID(o_var_constant_id,
                                       edge_candidate_list,
                                       this_edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::o2p: {
        auto o_var_constant_id = edge_info.o_;
        kv_store_->getpreIDlistByobjID(o_var_constant_id,
                                       edge_candidate_list,
                                       this_edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::so2p: {
        auto s_var_constant_id = edge_info.s_;
        auto o_var_constant_id = edge_info.o_;
        kv_store_->getpreIDlistBysubIDobjID(s_var_constant_id,
                                            o_var_constant_id,
                                            edge_candidate_list,
                                            this_edge_list_len,
                                            true,
                                            txn_);
        break;
      }
      case JoinMethod::sp2o: {
        auto s_var_constant_id = edge_info.s_;
        auto p_var_constant_id = edge_info.p_;
        kv_store_->getobjIDlistBysubIDpreID(s_var_constant_id,
                                            p_var_constant_id,
                                            edge_candidate_list,
                                            this_edge_list_len,
                                            true,
                                            txn_);
        break;
      }
      case JoinMethod::po2s: {
        auto p_var_constant_id = edge_info.p_;
        auto o_var_constant_id = edge_info.o_;
        kv_store_->getsubIDlistByobjIDpreID(o_var_constant_id,
                                            p_var_constant_id,
                                            edge_candidate_list,
                                            this_edge_list_len,
                                            true,
                                            txn_);
        break;
      }
    }
    cout<<"get "<<this_edge_list_len<<" result in this edge "<<endl;
    UpdateIDList(id_candidate,edge_candidate_list,this_edge_list_len,i > 0);
  }
  return id_candidate;
}

/**
 * add a node to the table
 * @param one_step_join_node_ Describe how to join a node
 * @param intermediate_result The Result in previous step
 * @return bool: the function is done; IntermediateResult: the new IntermediateResult
 */
tuple<bool,TableContentShardPtr> Optimizer::JoinANode(const shared_ptr<OneStepJoinNode>& one_step_join_node_,
                                                      const TableContentShardPtr& table_content_ptr,
                                                      const PositionValueSharedPtr& id_pos_mapping,
                                                      const IDCachesSharePtr& id_caches)
{


  TableContentShardPtr new_intermediate_table = make_shared<TableContent>();

  auto new_id = one_step_join_node_->node_to_join_;
  auto new_id_position = (*id_pos_mapping)[new_id];

  auto edges_info = one_step_join_node_->edges_;

  /* : each record */
  for (auto record_iterator = table_content_ptr->begin();
       record_iterator != table_content_ptr->end();
       record_iterator++) {
    shared_ptr<IDList> record_candidate_list = ExtendRecord(one_step_join_node_,
                                                            id_pos_mapping,
                                                            id_caches,
                                                            new_id,
                                                            record_iterator);

    /* write to the new table */
    auto record = *record_iterator;
    for (auto new_element:*(record_candidate_list->getList())) {
      auto new_record = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>(*record);
      new_record->push_back(new_element);
      //cout<<"new_record_len="<<new_record->size()<<endl;
      new_intermediate_table->push_back(std::move(new_record));
    }
  }

  //cout<<"Optimizer::JoinANode result_record_len = "<<new_intermediate_table->values_->front()->size()<<endl;
  return make_tuple(true,new_intermediate_table);

}

shared_ptr<IDList> Optimizer::ExtendRecord(const shared_ptr<OneStepJoinNode> &one_step_join_node_,
                                           const PositionValueSharedPtr &id_pos_mapping,
                                           const IDCachesSharePtr &id_caches,
                                           TYPE_ENTITY_LITERAL_ID new_id,
                                           list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>::iterator &record_iterator) const {

  auto record_candidate_list= make_shared<IDList>();
  auto record_candidate_prepared = false;
  auto edges_info = one_step_join_node_->edges_;
  /* for each edge */
  for (int edge_i = 0;edge_i<edges_info->size();edge_i++) {
    auto edge_info = (*edges_info)[edge_i];
    auto edge_constant_info = one_step_join_node_->edges_constant_info_->operator[](edge_i);
    if (record_candidate_prepared && record_candidate_list->empty())
      continue;
    TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
    TYPE_ENTITY_LITERAL_ID edge_list_len;
    /* through this edge get candidate */
    switch (edge_info.join_method_) {
      case JoinMethod::s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
        /* if s is constant, it is edge constraint check, and should not appear in the function*/
        auto s_var_position = (*id_pos_mapping)[edge_info.s_];
        auto s_var_id_this_record = (**record_iterator)[s_var_position];
        kv_store_->getpreIDlistBysubID(s_var_id_this_record,
                                       edge_candidate_list,
                                       edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::s2o: {
        auto s_var_position = (*id_pos_mapping)[edge_info.s_];
        auto s_var_id_this_record = (**record_iterator)[s_var_position];
        kv_store_->getobjIDlistBysubID(s_var_id_this_record,
                                       edge_candidate_list,
                                       edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::p2s: {
        auto p_var_position = (*id_pos_mapping)[edge_info.p_];
        auto p_var_id_this_record = (**record_iterator)[p_var_position];
        kv_store_->getsubIDlistBypreID(p_var_id_this_record,
                                       edge_candidate_list,
                                       edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::p2o: {
        auto p_var_position = (*id_pos_mapping)[edge_info.p_];
        auto p_var_id_this_record = (**record_iterator)[p_var_position];
        kv_store_->getobjIDlistBypreID(p_var_id_this_record,
                                       edge_candidate_list,
                                       edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::o2s: {
        auto o_var_position = (*id_pos_mapping)[edge_info.o_];
        auto o_var_id_this_record = (**record_iterator)[o_var_position];
        kv_store_->getsubIDlistByobjID(o_var_id_this_record,
                                       edge_candidate_list,
                                       edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::o2p: {
        auto o_var_position = (*id_pos_mapping)[edge_info.o_];
        auto o_var_id_this_record = (**record_iterator)[o_var_position];
        kv_store_->getpreIDlistByobjID(o_var_id_this_record,
                                       edge_candidate_list,
                                       edge_list_len,
                                       true,
                                       txn_);
        break;
      }
      case JoinMethod::so2p: {
        TYPE_ENTITY_LITERAL_ID s_var_position,s_var_id_this_record;
        if(edge_constant_info.s_constant_)
          s_var_id_this_record = edge_info.s_;
        else {
          auto s_var_position = (*id_pos_mapping)[edge_info.s_];
          s_var_id_this_record = (**record_iterator)[s_var_position];
        }

        TYPE_ENTITY_LITERAL_ID o_var_position,o_var_id_this_record;
        if(edge_constant_info.o_constant_)
          o_var_id_this_record = edge_info.o_;
        else {
          auto o_var_position = (*id_pos_mapping)[edge_info.o_];
          o_var_id_this_record = (**record_iterator)[o_var_position];
        }

        kv_store_->getpreIDlistBysubIDobjID(s_var_id_this_record,
                                            o_var_id_this_record,
                                            edge_candidate_list,
                                            edge_list_len,
                                            true,
                                            txn_);
        break;
      }
      case JoinMethod::sp2o: {
        TYPE_ENTITY_LITERAL_ID s_var_position,s_var_id_this_record;
        if(edge_constant_info.s_constant_)
          s_var_id_this_record = edge_info.s_;
        else {
          auto s_var_position = (*id_pos_mapping)[edge_info.s_];
          s_var_id_this_record = (**record_iterator)[s_var_position];
        }

        TYPE_ENTITY_LITERAL_ID p_var_position,p_var_id_this_record;
        if(edge_constant_info.p_constant_)
          p_var_id_this_record = edge_info.p_;
        else {
          auto p_var_position = (*id_pos_mapping)[edge_info.p_];
          p_var_id_this_record = (**record_iterator)[p_var_position];
        }
        kv_store_->getobjIDlistBysubIDpreID(s_var_id_this_record,
                                            p_var_id_this_record,
                                            edge_candidate_list,
                                            edge_list_len,
                                            true,
                                            txn_);
        break;
      }
      case JoinMethod::po2s: {
        TYPE_ENTITY_LITERAL_ID o_var_position,o_var_id_this_record;
        if(edge_constant_info.o_constant_)
          o_var_id_this_record = edge_info.o_;
        else {
          auto o_var_position = (*id_pos_mapping)[edge_info.o_];
          o_var_id_this_record = (**record_iterator)[o_var_position];
        }

        TYPE_ENTITY_LITERAL_ID p_var_position,p_var_id_this_record;
        if(edge_constant_info.p_constant_)
          p_var_id_this_record = edge_info.p_;
        else {
          auto p_var_position = (*id_pos_mapping)[edge_info.p_];
          p_var_id_this_record = (**record_iterator)[p_var_position];
        }

        kv_store_->getsubIDlistByobjIDpreID(o_var_id_this_record,
                                            p_var_id_this_record,
                                            edge_candidate_list,
                                            edge_list_len,
                                            true,
                                            txn_);
        break;
      }
    }

    UpdateIDList(record_candidate_list,
                 edge_candidate_list,
                 edge_list_len,
                 record_candidate_prepared);
    delete [] edge_candidate_list;
    /* If candidate is not prepared, then we ues the first edge as candidate */
    record_candidate_prepared = true;
  }

  if(id_caches->find(new_id)!=id_caches->end())
  {
    auto caches_ptr = (*(id_caches->find(new_id))).second;
    record_candidate_list->intersectList(caches_ptr->getList()->data(),caches_ptr->size());
  }

  return record_candidate_list;
}

/**
 * join two table with vars in one_step_join_table
 * @param one_step_join_table: define join vars
 * @param table_a
 * @param table_b
 * @return new table, columns are made up of
 * [ big table vars ][ small table vars - common vars]
 */
tuple<bool, PositionValueSharedPtr ,TableContentShardPtr> Optimizer:: JoinTwoTable(const shared_ptr<OneStepJoinTable>& one_step_join_table,
                                                                                   const TableContentShardPtr& table_a,
                                                                                   const PositionValueSharedPtr& table_a_id_pos,
                                                                                   const PositionValueSharedPtr& table_a_pos_id,
                                                                                   const TableContentShardPtr& table_b,
                                                                                   const PositionValueSharedPtr& table_b_id_pos,
                                                                                   const PositionValueSharedPtr& table_b_pos_id)  {
  long t1 = Util::get_cur_time();
  if(table_a->empty() || table_b->empty())
    return make_tuple(false,nullptr,make_shared<TableContent>());

  auto new_position_id_mapping = make_shared<PositionValue>();

  auto join_nodes = one_step_join_table->public_variables_;
  /* build index in big table
   * */
  auto& big_table = table_a->size() > table_b->size() ? table_a : table_b;
  auto& big_id_pos = table_a->size() > table_b->size() ? table_a_id_pos : table_b_id_pos;
  auto& big_pos_id = table_a->size() > table_b->size() ? table_a_pos_id : table_b_pos_id;

  auto& small_table = table_a->size() <= table_b->size() ? table_a : table_b;
  auto& small_id_pos = table_a->size() <= table_b->size() ? table_a_id_pos : table_b_id_pos;
  auto& small_pos_id = table_a->size() <= table_b->size() ? table_a_pos_id : table_b_pos_id;

  auto result_table = make_shared<TableContent>();

  auto indexed_result = unordered_map<
      /*key*/ vector<TYPE_ENTITY_LITERAL_ID>,
      /*value*/ shared_ptr<vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>,
      /*hash function*/ container_hash<vector<TYPE_ENTITY_LITERAL_ID>>
  >();
  // # TODO 先建立大表的索引 ，再建立小表的索引
  vector<TYPE_ENTITY_LITERAL_ID> common_variables_position_big;
  vector<TYPE_ENTITY_LITERAL_ID> common_variables_position_small;
  for(auto common_variable:*(one_step_join_table->public_variables_))
  {
    common_variables_position_big.push_back((*big_id_pos)[common_variable]);
    common_variables_position_small.push_back((*small_id_pos)[common_variable]);
  }
  auto &public_vars = one_step_join_table->public_variables_;

  auto big_table_record_len = big_id_pos->size();
  for(int i =0;i<big_table_record_len;i++)
    (*new_position_id_mapping)[i]= (*big_pos_id)[i];

  auto small_table_record_len = small_pos_id->size();
  for(int i =0;i<small_table_record_len;i++) {
    auto small_id = (*small_pos_id)[i];
    if(find(public_vars->begin(),public_vars->end(),small_id)!=public_vars->end())
      continue;
    (*new_position_id_mapping)[new_position_id_mapping->size()] = small_id;
  }

//  cout << "JoinTwoTable::new_mapping = " << new_position_id_mapping->size() << endl;

  auto common_variables_size = one_step_join_table->public_variables_->size();
  for(const auto& big_record:*(big_table))
  {
    vector<TYPE_ENTITY_LITERAL_ID> result_index(common_variables_size);
    for(auto common_position:common_variables_position_big)
    {
      result_index.push_back((*big_record)[common_position]);
    }
    if(indexed_result.find(result_index)!=indexed_result.end())
    {
      indexed_result[result_index]->push_back(big_record);
    }
    else
    {
      auto tmp_vector = make_shared<vector<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>();
      tmp_vector->push_back(big_record);
      indexed_result[result_index] = tmp_vector;
    }
  }


  /* Now Do the Matching , first calculate which ids in table small should be add
   * */
  set<TYPE_ENTITY_LITERAL_ID> public_variables;
  for(auto variable_id:*join_nodes)
  {
    public_variables.insert(variable_id);
  }
  vector<TYPE_ENTITY_LITERAL_ID> small_table_inserted_variables_position;
  for(int i =0;i<small_pos_id->size();i++)
  {
    auto small_node = (*small_pos_id)[i];
    if (public_variables.find(small_node)!=public_variables.end())
      continue;
    small_table_inserted_variables_position.push_back(i);
  }

  auto result_contents = result_table;
  /* do the matching */
  for(const auto& small_record:*small_table)
  {
    vector<TYPE_ENTITY_LITERAL_ID> result_index(common_variables_size);
    for(auto common_position:common_variables_position_small)
    {
      result_index.push_back((*small_record)[common_position]);
    }
    /* the index is in the big table */
    if(indexed_result.find(result_index)!=indexed_result.end())
    {
      vector<TYPE_ENTITY_LITERAL_ID> small_record_inserted;
      for(auto small_position:small_table_inserted_variables_position)
      {
        small_record_inserted.push_back((*small_record)[small_position]);
      }
      auto matched_content = indexed_result[result_index];
      for(const auto& matched_big_record:*matched_content)
      {
        auto result_record = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>(*matched_big_record);
        for(auto small_inserted_element:small_record_inserted)
        {
          result_record->push_back(small_inserted_element);
        }
        result_contents->push_back(result_record);
      }
    }
    /* if not, ignore */
  }
  cout<<"binary join,  result size "<<result_table->size()<<endl;
  long t2 = Util::get_cur_time();
  cout << "binary join used " << (t2-t1) << "ms." <<endl;
  auto first_r = result_table->front();
  return make_tuple(true, new_position_id_mapping, result_table);

}

tuple<bool, TableContentShardPtr> Optimizer::ANodeEdgesConstraintFilter(const shared_ptr<OneStepJoinNode>& one_step_join_table,
                                                                        TableContentShardPtr table_content_ptr,
                                                                        const PositionValueSharedPtr& id_pos_mapping,
                                                                        const IDCachesSharePtr& id_caches) {
  auto edge_info_vec = one_step_join_table->edges_;
  auto edge_constant_info_vec = one_step_join_table->edges_constant_info_;
  for(int i=0;i<one_step_join_table->edges_->size();i++)
  {
    auto edge_info = (*edge_info_vec)[i];
    auto edge_constant_info = (*edge_constant_info_vec)[i];
    auto step_result = this->OneEdgeConstraintFilter(edge_info,edge_constant_info,table_content_ptr,id_pos_mapping,id_caches);
    if(get<0>(step_result))
      table_content_ptr = get<1>(step_result);
  }
  return make_tuple(true,table_content_ptr);
}


/**
 * for a table T[a,b,c] if we want to check if ?a always have a edge (?a,p,o)
 * If T is nullptr,
 * @param edge_info
 * @param edge_table_info
 * @param table_content_ptr
 * @return
 */
tuple<bool, TableContentShardPtr> Optimizer::OneEdgeConstraintFilter(EdgeInfo edge_info,
                                                                     EdgeConstantInfo edge_table_info,
                                                                     const TableContentShardPtr& table_content_ptr,
                                                                     const PositionValueSharedPtr& id_pos_mapping,
                                                                     const IDCachesSharePtr& id_caches) {
  TYPE_ENTITY_LITERAL_ID var_to_filter= edge_info.getVarToFilter();
  if(edge_table_info.ConstantToVar(edge_info))
  {

    TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
    TYPE_ENTITY_LITERAL_ID this_edge_list_len;
    switch (edge_info.join_method_) {
      case JoinMethod::s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
        auto s_var_constant_id = edge_info.s_;
        this->kv_store_->getpreIDlistBysubID(s_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::s2o: {
        auto s_var_constant_id = edge_info.s_;
        this->kv_store_->getobjIDlistBysubID(s_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::p2s: {
        auto p_var_constant_id = edge_info.p_;
        this->kv_store_->getsubIDlistBypreID(p_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::p2o: {
        auto p_var_constant_id = edge_info.p_;
        this->kv_store_->getobjIDlistBypreID(p_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::o2s: {
        auto o_var_constant_id = edge_info.o_;
        this->kv_store_->getsubIDlistByobjID(o_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::o2p: {
        auto o_var_constant_id = edge_info.o_;
        this->kv_store_->getpreIDlistByobjID(o_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::so2p: {
        auto s_var_constant_id = edge_info.s_;
        auto o_var_constant_id = edge_info.o_;
        this->kv_store_->getpreIDlistBysubIDobjID(s_var_constant_id,
                                                  o_var_constant_id,
                                                  edge_candidate_list,
                                                  this_edge_list_len,
                                                  true,
                                                  this->txn_);
        break;
      }
      case JoinMethod::sp2o: {
        auto s_var_constant_id = edge_info.s_;
        auto p_var_constant_id = edge_info.p_;
        this->kv_store_->getobjIDlistBysubIDpreID(s_var_constant_id,
                                                  p_var_constant_id,
                                                  edge_candidate_list,
                                                  this_edge_list_len,
                                                  true,
                                                  this->txn_);
        break;
      }
      case JoinMethod::po2s: {
        auto p_var_constant_id = edge_info.p_;
        auto o_var_constant_id = edge_info.o_;
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
    return FilterAVariableOnIDList(validate_list,var_to_filter,table_content_ptr,id_pos_mapping);
  }


  auto result_table = make_shared<TableContent>();
  /* : each record */
  for (auto record_iterator = table_content_ptr->begin(); record_iterator != table_content_ptr->end();record_iterator++) {
    TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
    TYPE_ENTITY_LITERAL_ID edge_list_len;
    switch (edge_info.join_method_) {
      case JoinMethod::s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
        auto s_var_position = (*id_pos_mapping)[edge_info.s_];
        auto s_var_id_this_record = (**record_iterator)[s_var_position];
        this->kv_store_->getpreIDlistBysubID(s_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);

        break;
      }
      case JoinMethod::s2o: {
        auto s_var_position = (*id_pos_mapping)[edge_info.s_];
        auto s_var_id_this_record = (**record_iterator)[s_var_position];
        this->kv_store_->getobjIDlistBysubID(s_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::p2s: {
        auto p_var_position = (*id_pos_mapping)[edge_info.p_];
        auto p_var_id_this_record = (**record_iterator)[p_var_position];
        this->kv_store_->getsubIDlistBypreID(p_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::p2o: {
        auto p_var_position = (*id_pos_mapping)[edge_info.p_];
        auto p_var_id_this_record = (**record_iterator)[p_var_position];
        this->kv_store_->getobjIDlistBypreID(p_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::o2s: {
        auto o_var_position = (*id_pos_mapping)[edge_info.o_];
        auto o_var_id_this_record = (**record_iterator)[o_var_position];
        this->kv_store_->getsubIDlistByobjID(o_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::o2p: {
        auto o_var_position = (*id_pos_mapping)[edge_info.o_];
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
      case JoinMethod::so2p: {
        TYPE_ENTITY_LITERAL_ID  s_var_id_this_record;
        TYPE_ENTITY_LITERAL_ID  o_var_id_this_record;

        if(edge_table_info.s_constant_)
        {
          s_var_id_this_record = edge_info.s_;
        }
        else{
          auto s_var_position = (*id_pos_mapping)[edge_info.s_];
          s_var_id_this_record = (**record_iterator)[s_var_position];
        }

        if(edge_table_info.o_constant_)
        {
          o_var_id_this_record = edge_info.o_;
        }
        else{
          auto o_var_position = (*id_pos_mapping)[edge_info.o_];
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
      case JoinMethod::sp2o: {
        TYPE_ENTITY_LITERAL_ID  s_var_id_this_record;
        TYPE_ENTITY_LITERAL_ID  p_var_id_this_record;
        if(edge_table_info.s_constant_)
        {
          s_var_id_this_record = edge_info.s_;
        }
        else{
          auto s_var_position = (*id_pos_mapping)[edge_info.s_];
          s_var_id_this_record = (**record_iterator)[s_var_position];
        }

        if(edge_table_info.p_constant_)
        {
          p_var_id_this_record = edge_info.p_;
        }
        else{
          auto p_var_position = (*id_pos_mapping)[edge_info.p_];
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
      case JoinMethod::po2s: {
        TYPE_ENTITY_LITERAL_ID  p_var_id_this_record;
        TYPE_ENTITY_LITERAL_ID  o_var_id_this_record;

        if(edge_table_info.o_constant_)
        {
          o_var_id_this_record = edge_info.o_;
        }
        else{
          auto o_var_position = (*id_pos_mapping)[edge_info.o_];
          o_var_id_this_record = (**record_iterator)[o_var_position];
        }

        if(edge_table_info.p_constant_)
        {
          p_var_id_this_record = edge_info.p_;
        }
        else{
          auto p_var_position = (*id_pos_mapping)[edge_info.p_];
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

    auto var_to_filter_position = (*id_pos_mapping)[var_to_filter];
    auto var_to_filter_id_this_record = (**record_iterator)[var_to_filter_position];
    if(binary_search(edge_candidate_list,edge_candidate_list+edge_list_len,var_to_filter_id_this_record))
    {
      result_table->push_back(*record_iterator);
    }
    delete [] edge_candidate_list;
  }

  return make_tuple(true,result_table);

}

tuple<bool, TableContentShardPtr> Optimizer::FilterAVariableOnIDList(const shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>& candidate_list,
                                                                     TYPE_ENTITY_LITERAL_ID var_id,
                                                                     const TableContentShardPtr& table_content_ptr,
                                                                     const PositionValueSharedPtr& id_posing_mapping) {

  auto new_intermediate_table = make_shared<TableContent>();
  auto var_position = (*id_posing_mapping)[var_id];

  /* : each record */
  for (const auto& record_sp : *table_content_ptr){
    auto var_to_filter_id_this_record=(*record_sp)[var_position];
    if(binary_search(candidate_list->begin(),candidate_list->end(),var_to_filter_id_this_record))
    {
      new_intermediate_table->push_back(record_sp);
    }
  }

  return make_tuple(true,new_intermediate_table);

}

bool Optimizer::CacheConstantCandidates(const shared_ptr<OneStepJoinNode>& one_step, const IDCachesSharePtr& id_caches) {
  auto node_t = one_step->node_to_join_;
  auto edges = one_step->edges_;
  auto edges_constant = one_step->edges_constant_info_;
  for(int i =0 ;i<edges->size();i++) {
    AddConstantCandidates((*edges)[i],(*edges_constant)[i],node_t,id_caches);
  }
  return true;
}

bool Optimizer::AddConstantCandidates(EdgeInfo edge_info,EdgeConstantInfo edge_table_info,TYPE_ENTITY_LITERAL_ID targetID,
                                      const IDCachesSharePtr& id_caches)
{
  TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
  TYPE_ENTITY_LITERAL_ID this_edge_list_len;
  switch (edge_info.join_method_) {
    case JoinMethod::s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
      auto s_var_constant_id = edge_info.s_;
      this->kv_store_->getpreIDlistBysubID(s_var_constant_id,
                                           edge_candidate_list,
                                           this_edge_list_len,
                                           true,
                                           this->txn_);
      break;
    }
    case JoinMethod::s2o: {
      auto s_var_constant_id = edge_info.s_;
      this->kv_store_->getobjIDlistBysubID(s_var_constant_id,
                                           edge_candidate_list,
                                           this_edge_list_len,
                                           true,
                                           this->txn_);
      break;
    }
    case JoinMethod::p2s: {
      auto p_var_constant_id = edge_info.p_;
      this->kv_store_->getsubIDlistBypreID(p_var_constant_id,
                                           edge_candidate_list,
                                           this_edge_list_len,
                                           true,
                                           this->txn_);
      break;
    }
    case JoinMethod::p2o: {
      auto p_var_constant_id = edge_info.p_;
      this->kv_store_->getobjIDlistBypreID(p_var_constant_id,
                                           edge_candidate_list,
                                           this_edge_list_len,
                                           true,
                                           this->txn_);
      break;
    }
    case JoinMethod::o2s: {
      auto o_var_constant_id = edge_info.o_;
      this->kv_store_->getsubIDlistByobjID(o_var_constant_id,
                                           edge_candidate_list,
                                           this_edge_list_len,
                                           true,
                                           this->txn_);
      break;
    }
    case JoinMethod::o2p: {
      auto o_var_constant_id = edge_info.o_;
      this->kv_store_->getpreIDlistByobjID(o_var_constant_id,
                                           edge_candidate_list,
                                           this_edge_list_len,
                                           true,
                                           this->txn_);
      break;
    }
    case JoinMethod::so2p: {
      auto s_var_constant_id = edge_info.s_;
      auto o_var_constant_id = edge_info.o_;
      this->kv_store_->getpreIDlistBysubIDobjID(s_var_constant_id,
                                                o_var_constant_id,
                                                edge_candidate_list,
                                                this_edge_list_len,
                                                true,
                                                this->txn_);
      break;
    }
    case JoinMethod::sp2o: {
      auto s_var_constant_id = edge_info.s_;
      auto p_var_constant_id = edge_info.p_;
      this->kv_store_->getobjIDlistBysubIDpreID(s_var_constant_id,
                                                p_var_constant_id,
                                                edge_candidate_list,
                                                this_edge_list_len,
                                                true,
                                                this->txn_);
      break;
    }
    case JoinMethod::po2s: {
      auto p_var_constant_id = edge_info.p_;
      auto o_var_constant_id = edge_info.o_;
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
  if(id_caches->find(targetID) == id_caches->end())
  {
    auto new_ids = make_shared<IDList>();
    UpdateIDList(new_ids,edge_candidate_list,this_edge_list_len,false);
    (*id_caches)[targetID] = new_ids;
  }
  else
  {
    auto prepared_IDs = (*id_caches->find(targetID)).second;
    UpdateIDList(prepared_IDs,edge_candidate_list,this_edge_list_len,true);
  }
  delete [] edge_candidate_list;

  return true;
}

/**
 * a simple version of Join::update_answer_list
 * @param valid_id_list
 * @param id_list
 * @param id_list_len
 * @param id_list_prepared : valid_id_list is initialed
 */
void
Optimizer::UpdateIDList(const shared_ptr<IDList>& valid_id_list, unsigned* id_list, unsigned id_list_len,bool id_list_prepared)
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

BasicQueryStrategy Optimizer::ChooseStrategy(BasicQuery *basic_query,QueryInfo *query_info) {
  if (!query_info->limit_) {
    if(basic_query->getTripleNum()!=1)
      return BasicQueryStrategy::Normal;
    else
      return BasicQueryStrategy::Special;
  }
  else
  {
    if(query_info->ordered_by_expressions_->size())
      return BasicQueryStrategy::TopK;
    else
      return BasicQueryStrategy::limitK;
  }
}

shared_ptr<IntermediateResult> Optimizer::NormalJoin(shared_ptr<BasicQuery>, shared_ptr<QueryPlan>) {
  return shared_ptr<IntermediateResult>();
}

/* This is One layer, need more*/
tuple<bool, TableContentShardPtr> Optimizer::ExecutionDepthFirst(BasicQuery* basic_query,
                                                                 const shared_ptr<QueryPlan>& query_plan,
                                                                 const QueryInfo& query_info,
                                                                 const PositionValueSharedPtr& id_pos_mapping) {
  auto limit_num = query_info.limit_num_;
  cout<<"Optimizer::ExecutionDepthFirst query_info.limit_num_="<<query_info.limit_num_<<endl;
  auto first_result = this->GenerateColdCandidateList((*query_plan->join_order_)[0].edge_filter_->edges_,
                                                      (*query_plan->join_order_)[0].edge_filter_->edges_constant_info_);

  auto var_candidates_cache = make_shared<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>>();
  auto first_candidates_list = get<1>(first_result);


  auto first_var_candidates_vec = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>(first_candidates_list->size());
  for(const auto& var_can_vec:*first_candidates_list)
    first_var_candidates_vec->push_back((*var_can_vec)[0]);

  if( query_plan->join_order_->size()==1 || first_candidates_list->empty())
    return make_tuple(true, first_candidates_list);

  int now_result = 0;
  int now_offset = 0;
  vector<TableContentShardPtr> result_container;

  cout<<"ExecutionDepthFirst:"<<endl;

  for(auto& constant_generating_step: *(query_plan->constant_generating_lists_))
  {
    CacheConstantCandidates(constant_generating_step,var_candidates_cache);
  };

  cout<<"first_candidates_list.size()="<<first_candidates_list->size()<<endl;
  /* #TODO 特殊情况：只有一个点，这个还没写
   * */
  if(limit_num != -1) {
    while (now_result <= limit_num) {
      auto tmp_result = make_shared<TableContent>();
      tmp_result->push_back(first_candidates_list->front());
      first_candidates_list->pop_front();
      auto first_var_one_point_result =
          this->DepthSearchOneLayer(query_plan, 1, now_result, limit_num, tmp_result, id_pos_mapping,var_candidates_cache);

      if (get<0>(first_var_one_point_result)) {
        auto one_point_inter_result = get<1>(first_var_one_point_result);
        if (!one_point_inter_result->empty())
          result_container.push_back(one_point_inter_result);
        if (now_result >= limit_num)
          break;
        if (now_offset >= first_var_candidates_vec->size())
          break;
      }
      if (first_candidates_list->empty())
        break;
    }
  }
  else
  {
    while (true) {
      auto tmp_result = make_shared<TableContent>();
      tmp_result->push_back(first_candidates_list->front());
      first_candidates_list->pop_front();
      auto first_var_one_point_result =
          this->DepthSearchOneLayer(query_plan, 1, now_result, limit_num, tmp_result, id_pos_mapping,var_candidates_cache);
      if (get<0>(first_var_one_point_result))
      {
        auto one_point_inter_result = get<1>(first_var_one_point_result);
        if (!one_point_inter_result->empty())
          result_container.push_back(one_point_inter_result);
        if (now_offset >= first_var_candidates_vec->size())
          break;
      }
      if (first_candidates_list->empty())
        break;
    }
  }
  cout<<"Optimizer::ExecutionDepthFirst result_container.size()="<<result_container.size()<<endl;
  if(result_container.empty())
    return make_tuple(true,make_shared<TableContent>());
  int counter = 0;
  /* merge result */

  auto final_result = result_container[0];
  for(int i=1;i<result_container.size();i++)
    for(const auto& record:*(result_container[i])) {
      final_result->push_back(record);
      if( ++counter == limit_num)
        break;
    }

  cout<<"Optimizer::ExecutionDepthFirst final_result.size()="<<final_result->size()<<endl;
  return make_tuple(true,final_result);
}

// #TODO, just return list<shared_ptr<vector< ID_TYPE >>>
tuple<bool,TableContentShardPtr> Optimizer::DepthSearchOneLayer(const shared_ptr<QueryPlan>& query_plan,
                                                                int layer_count,
                                                                int &result_number_till_now,
                                                                int limit_number,
                                                                const TableContentShardPtr& table_content_ptr,
                                                                const PositionValueSharedPtr& id_pos_mapping,
                                                                const IDCachesSharePtr& id_caches) {

  auto one_step = (*(query_plan->join_order_))[layer_count];
  tuple<bool,TableContentShardPtr> step_result;
  switch (one_step.join_type_) {
    case OneStepJoin::JoinType::JoinNode: { // 要注意这里的指针会不会传丢掉
      step_result = JoinANode(one_step.join_node_, table_content_ptr,id_pos_mapping,id_caches);
      break;
    }
    case OneStepJoin::JoinType::JoinTable: {
      break;
    }
    case OneStepJoin::JoinType::GenerateCandidates : {
      // Now Only The first element can be GenerateCandidates
      // We have processed it before
      return make_tuple(false, nullptr);
    }
    case OneStepJoin::JoinType::EdgeCheck: {
      step_result = ANodeEdgesConstraintFilter(one_step.edge_filter_, table_content_ptr,id_pos_mapping,id_caches);
      break;
    }
  }
  // If not success, should have returned before

  auto try_result = get<1>(step_result);

  /* go deeper */
  if(try_result->empty())
    return make_tuple(false, nullptr);


  /* deep in bottom */
  if(layer_count + 1 == query_plan->join_order_->size()) {
    //inter_result->values_->front()->size();
    result_number_till_now += get<1>(step_result)->size();
    return step_result;
  }

  auto all_result = make_shared<TableContent>();



  /* fill a node */
  for(const auto& one_result:*try_result) {
    auto table_content_ptr = make_shared<TableContent>();

    table_content_ptr->push_back(one_result);
    auto next_layer_result  =
        this->DepthSearchOneLayer(query_plan, layer_count + 1, result_number_till_now, limit_number, table_content_ptr,id_pos_mapping,id_caches);
    /* if not success */
    if(!get<0>(next_layer_result))
      continue;

    /* record the result */
    for(const auto& next_layer_one_result: *get<1>(next_layer_result)) {
      all_result->push_back(next_layer_one_result);// all_result->values_->front()->size();
      // cout<<"layer["<<layer_count<<"] record size:"<<next_layer_one_result->size()<<endl;
    }
    if(limit_number!=-1) {
      /* examine the number*/
      if (result_number_till_now >= limit_number)
        return make_tuple(true, all_result);
    }
  }
  return make_tuple(true,all_result);

}


tuple<bool,shared_ptr<IntermediateResult>> Optimizer::DoQuery(SPARQLquery &sparql_query,QueryInfo query_info) {

  vector<BasicQuery*> basic_query_vec;
  vector<shared_ptr<QueryPlan>> query_plan_vec;

#ifdef TOPK_DEBUG_INFO
  std::cout<<"Optimizer:: limit used:"<<query_info.limit_<<std::endl;
  std::cout<<"Optimizer::DoQuery limit num:"<<query_info.limit_num_<<std::endl;
#endif
  // We Don't think too complex SPARQL now
  for(auto basic_query_pointer:sparql_query.getBasicQueryVec())
  {

    auto var_candidates_cache = make_shared<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>>();
    shared_ptr<QueryPlan> query_plan;
    auto strategy = this->ChooseStrategy(basic_query_pointer,&query_info);
    if(strategy == BasicQueryStrategy::Normal)
    {

      long t1 =Util::get_cur_time();
      auto const_candidates = QueryPlan::OnlyConstFilter(basic_query_pointer, this->kv_store_, this->var_descriptors_);
      for (auto &constant_generating_step: *const_candidates) {
        CacheConstantCandidates(constant_generating_step, var_candidates_cache);
      };
      long t2 = Util::get_cur_time();
      cout << "get var cache, used " << (t2 - t1) << "ms." << endl;
      this->var_descriptors_->clear();
      long t3 = Util::get_cur_time();

      cout << "id_list.size = " << var_candidates_cache->size() << endl;
      for (auto x : *var_candidates_cache){
        cout << "var[" << x.first << "] = " << basic_query_pointer->getVarName(x.first) << "  , var_candidate list size = " << x.second->size() << endl;
      }
#ifdef FEED_PLAN
      vector<int> node_order = {2,1,0};
      auto best_plan_tree = new PlanTree(node_order);
#else
      auto best_plan_tree = this->get_plan(basic_query_pointer, this->kv_store_, var_candidates_cache);
#endif
      long t4 = Util::get_cur_time();
      cout << "plan get, used " << (t4 - t3) << "ms." << endl;
      best_plan_tree->print(basic_query_pointer);
      auto bfs_result = this->ExecutionBreathFirst(basic_query_pointer,query_info,best_plan_tree->root_node,var_candidates_cache);

      long t5 = Util::get_cur_time();
      cout << "execution, used " << (t5 - t3) << "ms." << endl;

      basic_query_vec.push_back(basic_query_pointer);
      query_plan_vec.push_back(query_plan);
      auto pos_var_mapping = get<1>(bfs_result);

      auto var_pos_mapping = make_shared<PositionValue>();
      for(const auto& pos_var_pair:*pos_var_mapping) {
        (*var_pos_mapping)[pos_var_pair.second] = pos_var_pair.first;
      }

      long t6 = Util::get_cur_time();
      CopyToResult(basic_query_pointer->getResultListPointer(), basic_query_pointer, make_shared<IntermediateResult>(
          var_pos_mapping,pos_var_mapping,get<2>(bfs_result)
      ));
      long t7 = Util::get_cur_time();
      cout << "copy to result, used " << (t7 - t6) <<"ms." <<endl;
      cout << "total execution, used " << (t7 - t1) <<"ms."<<endl;
    }
    else if(strategy ==BasicQueryStrategy::Special){
      printf("BasicQueryStrategy::Special not supported yet\n");
    }
    else if(strategy == BasicQueryStrategy::TopK)
    {
#ifdef TOPK_SUPPORT
      auto const_candidates = QueryPlan::OnlyConstFilter(basic_query_pointer, this->kv_store_, this->var_descriptors_);
      for (auto &constant_generating_step: *const_candidates) {
        CacheConstantCandidates(constant_generating_step, var_candidates_cache);
      };

#ifdef TOPK_DEBUG_INFO
      std::cout<<"Top-k Constant Filtering Candidates Info"<<std::endl;
      for(const auto& pair:*var_candidates_cache)
        std::cout<<"var["<<pair.first<<"]  "<<pair.second->size()<<std::endl;
      std::cout<<"Top-k Constant Filtering Candidates Info End"<<std::endl;
#endif

      auto search_plan = make_shared<TopKTreeSearchPlan>(basic_query_pointer,this->statistics,(*query_info.ordered_by_expressions_)[0],var_candidates_cache);
#ifdef TOPK_DEBUG_INFO
      std::cout<<"Top-k Search Plan"<<std::endl;
      std::cout<<search_plan->DebugInfo()<<std::endl;
#endif
      auto top_k_result = this->ExecutionTopK(basic_query_pointer,search_plan,query_info,var_candidates_cache);
      auto pos_var_mapping = get<1>(top_k_result);
      auto var_pos_mapping = make_shared<PositionValue>();
      for(const auto& pos_var_pair:*pos_var_mapping) {
        (*var_pos_mapping)[pos_var_pair.second] = pos_var_pair.first;
      }

      CopyToResult(basic_query_pointer->getResultListPointer(), basic_query_pointer, make_shared<IntermediateResult>(
          var_pos_mapping,pos_var_mapping,get<2>(top_k_result)
      ));
#endif
    }
    else if(strategy == BasicQueryStrategy::limitK)
    {

    }
  }
  return MergeBasicQuery(sparql_query);
}


tuple<bool, shared_ptr<IntermediateResult>> Optimizer::MergeBasicQuery(SPARQLquery &sparql_query) {
  return tuple<bool, shared_ptr<IntermediateResult>>();
}

/**
 * 调用栈：   Strategy::handle
 *          Strategy::handler0
 *          Join::join_basic
 * @param target
 * @param basic_query
 * @param result, which has already gotten CoreVar, Pre var(selected), but not get satellites
 * @return
 */
bool Optimizer::CopyToResult(vector<unsigned int *> *target,
                             BasicQuery* basic_query,
                             const shared_ptr<IntermediateResult>& result) {

  assert(target->empty());

  int select_var_num = basic_query->getSelectVarNum();
  int core_var_num = basic_query->getRetrievedVarNum();
  int selected_pre_var_num = basic_query->getSelectedPreVarNum();

  // A little different with that in Join::CopyToResult
  // may basic query don't allocate an id for not selected var so selected_pre_var_num = pre_var_num?
  cout << "position to var des size: " << result->position_to_var_des_->size() << endl;
  cout << "totalvar_num: " << basic_query->getTotalVarNum()<<endl;
  cout << "varnum: " << basic_query->getVarNum()<<endl;
  cout << "selected var num: " << select_var_num<<endl;
  cout << "core var num: " << core_var_num<<endl;
  cout << "selected pre var num: " << selected_pre_var_num<<endl;

#ifdef OPTIMIZER_DEBUG_INFO
  for(auto &pos_id:*result->position_to_var_des_)
  {
    cout<<"pos["<<pos_id.first<<"] "<<pos_id.second<<endl;
  }
#endif
//  Linglin Yang fix it to total_var_num,
//  maybe change it to core_var_num + selected_pre_var_num in the future
  if (result->position_to_var_des_->size() != basic_query->getVarNum())
  {
    cout << "terrible error in Optimizer::CopyToResult!" << endl;
    return false;
  }

  shared_ptr<vector<Satellite>> satellites = make_shared<vector<Satellite>>();

  auto record_len = select_var_num + selected_pre_var_num;
  auto record = new unsigned[record_len];

  auto position_id_map_ptr = result->position_to_var_des_;
  auto id_position_map_ptr = result->var_des_to_position_;
  cout<<"fir_var_position"<<basic_query->getSelectedVarPosition((*position_id_map_ptr)[0])<<endl;
  int var_num =  basic_query->getVarNum();
  for (const auto&  record_ptr : *(result->values_))
  {
    int column_index = 0;
    for (; column_index < core_var_num; ++column_index)
    {
      //This is because selected var id is always smaller
      if ( (*position_id_map_ptr)[column_index] < select_var_num)
      {
        int vpos = basic_query->getSelectedVarPosition((*position_id_map_ptr)[column_index]);
        record[vpos] = (*record_ptr)[column_index];
      }
    }

    // below are for selected pre vars
    while (column_index < result->position_to_var_des_->size() )
    {
      //only add selected ones
      // 原句是 int pre_var_id = this->pos2id[i] - this->var_num;
      // 可能有bug
      int pre_var_id = (*position_id_map_ptr)[column_index] - var_num;
      int pre_var_position = basic_query->getSelectedPreVarPosition(pre_var_id);
      if(pre_var_position >= 0)
      {
        record[pre_var_position] = (*record_ptr)[column_index];
      }
      ++column_index;
    }

    bool valid = true;
    //generate satellites when constructing records
    //NOTICE: satellites in join must be selected
    //core vertex maybe not in select
    for (column_index = 0; column_index < core_var_num; ++column_index)
    {
      int id = (*position_id_map_ptr)[column_index];
      unsigned ele = (*record_ptr)[column_index];
      int degree = basic_query->getVarDegree(id);
      for (int j = 0; j < degree; ++j)
      {
        int id2 = basic_query->getEdgeNeighborID(id, j);
        if (basic_query->isSatelliteInJoin(id2) == false)
          continue;

        if(id_position_map_ptr->find(id2)!=id_position_map_ptr->end())
          continue;

        cout << "satellite node: " <<basic_query->getVarName(id2)<<endl;
        unsigned* idlist = nullptr;
        unsigned idlist_len = 0;
        int triple_id = basic_query->getEdgeID(id, j);
        Triple triple = basic_query->getTriple(triple_id);

        TYPE_PREDICATE_ID preid = basic_query->getEdgePreID(id, j);
        if (preid == -2)  //?p
        {
          string predicate = triple.predicate;
          int pre_var_id = basic_query->getPreVarID(predicate);
          //if(this->basic_query->isPreVarSelected(pre_var_id))
          //{
          preid = (*record_ptr)[ (*id_position_map_ptr)[pre_var_id+ var_num]];
          //}
        }
        else if (preid == -1)  //INVALID_PREDICATE_ID
        {
          //ERROR
        }

        char edge_type = basic_query->getEdgeType(id, j);
        if (edge_type == Util::EDGE_OUT)
        {
          this->kv_store_->getobjIDlistBysubIDpreID(ele, preid, idlist, idlist_len, true, this->txn_);
        }
        else
        {
          this->kv_store_->getsubIDlistByobjIDpreID(ele, preid, idlist, idlist_len, true, this->txn_);
        }

        if(idlist_len == 0)
        {
          valid = false;
          break;
        }
        satellites->push_back(Satellite(id2, idlist, idlist_len));

      }
      if(!valid)
      {
        break;
      }
    }

    int size = satellites->size();
    if(valid)
    {
      // Join::cartesian
      Cartesian(0,size,record_len,record,satellites,target,basic_query);
    }

    for (int k = 0; k < size; ++k)
    {
      delete[] (*satellites)[k].idlist;
      //this->satellites[k].idlist = NULL;
    }
    //WARN:use this to avoid influence on the next loop
    satellites->clear();
  }

  delete[] record;


  return false;
}

void
Optimizer::Cartesian(int pos, int end,int record_len,unsigned* record,
                     const shared_ptr<vector<Satellite>>& satellites,
                     vector<unsigned*>* result_list,
                     BasicQuery *basic_query)
{
  if (pos == end)
  {
    auto new_record = new unsigned[record_len];
    memcpy(new_record, record, sizeof(unsigned) * record_len);
    result_list->push_back(new_record);
    return;
  }

  unsigned size = (*satellites)[pos].idlist_len;
  int id = (*satellites)[pos].id;
  int vpos = basic_query->getSelectedVarPosition(id);
  unsigned* list = (*satellites)[pos].idlist;
  for (unsigned i = 0; i < size; ++i)
  {
    record[vpos] = list[i];
    Cartesian(pos + 1, end,record_len,record,satellites,result_list,basic_query);
  }
}




//  Linglin Yang add:


int Optimizer::get_strategy(unsigned int id_cache_size) {
  if(id_cache_size <= SAMPLE_CACHE_MAX){
    return 1;
  } else if(id_cache_size <= 1000){
    return 2;
  } else{
    return 3;
  }
}

////  Copied from Database::exist_triple in Database.cpp
////  If there exist a triple (s_id, p_id, o_id) in db, return true;
////  else, return false
bool Optimizer::check_exist_this_triple(TYPE_ENTITY_LITERAL_ID s_id, TYPE_PREDICATE_ID p_id, TYPE_ENTITY_LITERAL_ID o_id){

  unsigned* _objidlist = nullptr;
  unsigned _list_len = 0;
  bool is_exist = false;
  //get exclusive before update!
  kv_store_->getobjIDlistBysubIDpreID(s_id, p_id, _objidlist, _list_len, true);

  if (Util::bsearch_int_uporder(o_id, _objidlist, _list_len) != INVALID){
    is_exist = true;
  }
  delete[] _objidlist;

  return is_exist;
}



bool Optimizer::check_pass(vector<int> &linked_nei_pos, vector<char> &edge_type,
                           vector<TYPE_PREDICATE_ID> &p_list, vector<unsigned> &last_sample, unsigned this_var_sample){
  for(int i = 0; i < linked_nei_pos.size(); ++i){
    if(edge_type[i] == Util::EDGE_IN){
      if(!check_exist_this_triple(last_sample[linked_nei_pos[i]], p_list[i], this_var_sample)){
        return false;
      }
    } else{
      if(!check_exist_this_triple(this_var_sample, p_list[i], last_sample[linked_nei_pos[i]])){
        return false;
      }
    }

  }
  return true;

}

unsigned long Optimizer::card_estimator(BasicQuery* basicquery, IDCachesSharePtr &id_caches,
                                        map<int, unsigned> &var_to_num_map, map<int, TYPE_ENTITY_LITERAL_ID> &var_to_type_map,
                                        map<int, vector<TYPE_ENTITY_LITERAL_ID >> &var_to_sample_map, map<int, map<int, unsigned >> &s_o_list_average_size,
                                        const vector<int> &last_plan_nodes, int next_join_node, const vector<int> &now_plan_nodes,
                                        vector<map<vector<int>, unsigned long>> &card_cache,
                                        vector<map<vector<int>, vector<vector<unsigned>> >> &sample_cache,
                                        bool use_sample){


  int now_plan_node_num = now_plan_nodes.size();

  if(use_sample == false) {

    if (now_plan_node_num == 2) {

      unsigned long card_estimation = ULONG_MAX;

      unsigned long all_num = statistics->get_type_num_by_type_id(var_to_type_map[next_join_node]) *
          statistics->get_type_num_by_type_id(var_to_type_map[last_plan_nodes[0]]);

      for (int i = 0; i < basicquery->getVarDegree(next_join_node); ++i) {
        if (basicquery->getEdgeNeighborID(next_join_node, i) == last_plan_nodes[0]) {
          char in_or_out = basicquery->getEdgeType(next_join_node, i);
          TYPE_PREDICATE_ID p_id = basicquery->getEdgePreID(next_join_node, i);

          double selectivity = var_to_num_map[next_join_node] * var_to_num_map[last_plan_nodes[0]] / ((double) all_num);

          if(in_or_out == Util::EDGE_IN){
            card_estimation = min(card_estimation,
                                  (unsigned long)(selectivity*statistics->get_type_one_edge_typeid_num_by_id(var_to_type_map[last_plan_nodes[0]], p_id, var_to_type_map[next_join_node])));
          } else{
            card_estimation = min(card_estimation,
                                  (unsigned long)(selectivity*statistics->get_type_one_edge_typeid_num_by_id(var_to_type_map[next_join_node], p_id, var_to_type_map[last_plan_nodes[0]])));
          }
        }
      }

      if(card_estimation == ULONG_MAX){
        cout << "error when estimate cardinality when var_num = 2" << endl;
        exit(-1);
      } else{
        if(card_cache.size() < 1){
          map<vector<int>, unsigned long> two_var_card_map;
          two_var_card_map.insert(make_pair(now_plan_nodes, card_estimation));
          card_cache.push_back(two_var_card_map);

        } else{
          card_cache[0].insert(make_pair(now_plan_nodes, card_estimation));
        }
        return card_estimation;
      }
    }

    if (now_plan_node_num >= 3) {

      unsigned long card_estimation = ULONG_MAX;

      unsigned long last_plan_card = card_cache[now_plan_node_num-3][last_plan_nodes];
      if(now_plan_node_num==3)
        cout << "last_plan_card for " << last_plan_nodes[0] << " " << last_plan_nodes[1] << " " << next_join_node<<"  "<< last_plan_card<<endl;

      for (int i = 0; i < basicquery->getVarDegree(next_join_node); ++i) {
        if (find(last_plan_nodes.begin(), last_plan_nodes.end(),
                 basicquery->getEdgeNeighborID(next_join_node, i)) != last_plan_nodes.end()) {

          char in_or_out = basicquery->getEdgeType(next_join_node, i);
          TYPE_PREDICATE_ID p_id = basicquery->getEdgePreID(next_join_node, i);

          double selectivity = var_to_num_map[next_join_node] * var_to_num_map[basicquery->getEdgeNeighborID(next_join_node, i)] /
              ((double)(statistics->get_type_num_by_type_id(var_to_type_map[next_join_node]) *
                  statistics->get_type_num_by_type_id(var_to_type_map[basicquery->getEdgeNeighborID(next_join_node, i)])));

          if(in_or_out == Util::EDGE_IN){
            card_estimation = min(card_estimation,
                                  (unsigned long)(last_plan_card*selectivity*statistics->get_type_one_edge_typeid_num_by_id(var_to_type_map[basicquery->getEdgeNeighborID(next_join_node, i)], p_id, var_to_type_map[next_join_node])));
          } else{
            card_estimation = min(card_estimation,
                                  (unsigned long)(last_plan_card*selectivity*statistics->get_type_one_edge_typeid_num_by_id(var_to_type_map[next_join_node], p_id, var_to_type_map[basicquery->getEdgeNeighborID(next_join_node, i)])));
          }
        }
      }
      cout << "now card estimation: "<<card_estimation<<endl;

      if(card_estimation == ULONG_MAX){
        cout << "error when estimate cardinality when var_num = 2" << endl;
        exit(-1);
      } else{
        if(card_cache.size() < now_plan_node_num-1){
          map<vector<int>, unsigned long> this_var_num_card_map;
          this_var_num_card_map.insert(make_pair(now_plan_nodes, card_estimation));
          card_cache.push_back(this_var_num_card_map);

        } else{
          card_cache[now_plan_node_num-2].insert(make_pair(now_plan_nodes, card_estimation));
        }
        return card_estimation;
      }
    }

  }
  else{

//        if use sample when estimate card
//		{
//			if (now_plan_node_num == 2) {
//
//				unsigned long card_estimation;
//				vector<vector<unsigned>> this_sample;
//
//				unsigned now_sample_num = 0;
//				unsigned long all_num = var_to_num_map[last_plan_nodes[0]] * var_to_num_map[next_join_node];
//				cout << "all_num:" << var_to_num_map[last_plan_nodes[0]] << " * " << var_to_num_map[next_join_node]
//					 << " = " << all_num << endl;
//
//
//				vector<int> linked_nei_id;
//				vector<char> edge_type;
//				vector<TYPE_PREDICATE_ID> p_list;
//
//				for (int i = 0; i < basicquery->getVarDegree(next_join_node); ++i) {
//					if (basicquery->getEdgeNeighborID(next_join_node, i) == last_plan_nodes[0]) {
//						linked_nei_id.push_back(i);
//						edge_type.push_back(basicquery->getEdgeType(next_join_node, i));
//						p_list.push_back(basicquery->getEdgePreID(next_join_node, i));
//					}
//				}
//
//
//				map<int, int> new_id_pos_map;
//				if (last_plan_nodes[0] < next_join_node) {
//					new_id_pos_map[last_plan_nodes[0]] = 0;
//					new_id_pos_map[next_join_node] = 1;
//				} else {
//					new_id_pos_map[next_join_node] = 0;
//					new_id_pos_map[last_plan_nodes[0]] = 1;
//				}
//
//				for (int i = 0; i < var_to_sample_map[last_plan_nodes[0]].size(); ++i) {
//					for (int j = 0; j < var_to_sample_map[next_join_node].size(); ++j) {
//						bool check_pass = true;
//						for (int k = 0; k < linked_nei_id.size(); ++k) {
//							if (edge_type[k] == Util::EDGE_IN) {
//								if (!check_exist_this_triple(var_to_sample_map[last_plan_nodes[0]][i], p_list[k],
//															 var_to_sample_map[next_join_node][j])) {
//									check_pass = false;
//									break;
//								}
//							} else {
//								if (!check_exist_this_triple(var_to_sample_map[next_join_node][j], p_list[k],
//															 var_to_sample_map[last_plan_nodes[0]][i])) {
//									check_pass = false;
//									break;
//								}
//							}
//						}
//						if (check_pass) {
//							now_sample_num++;
//
//							if (now_sample_num < SAMPLE_CACHE_MAX) {
////
//								vector<unsigned> this_pass_sample(2);
//								this_pass_sample[new_id_pos_map[last_plan_nodes[0]]] = var_to_sample_map[last_plan_nodes[0]][i];
//								this_pass_sample[new_id_pos_map[next_join_node]] = var_to_sample_map[next_join_node][j];
//
//								this_sample.push_back(this_pass_sample);
//							} else {
////							if ((double) rand() / (double) RAND_MAX < 1.0 / now_sample_num)
//								if ((double) rand() / (double) RAND_MAX < SAMPLE_PRO) {
//
////								int index_to_replace = rand() % SAMPLE_CACHE_MAX;
////
////								this_sample[index_to_replace][new_id_pos_map[last_plan_nodes[0]]] = var_to_sample_map[last_plan_nodes[0]][i];
////								this_sample[index_to_replace][new_id_pos_map[next_join_node]] = var_to_sample_map[next_join_node][j];
//
//									vector<unsigned> this_pass_sample(2);
//									this_pass_sample[new_id_pos_map[last_plan_nodes[0]]] = var_to_sample_map[last_plan_nodes[0]][i];
//									this_pass_sample[new_id_pos_map[next_join_node]] = var_to_sample_map[next_join_node][j];
//
//									this_sample.push_back(this_pass_sample);
//
//								}
//
//							}
//
//						}
//
//					}
//				}
//
//				cout << "pass num: " << now_sample_num << endl;
//
//				card_estimation = max(
//						(unsigned long) ((double) (now_sample_num) / var_to_sample_map[last_plan_nodes[0]].size() *
//										 var_to_num_map[last_plan_nodes[0]] / var_to_sample_map[next_join_node].size() *
//										 var_to_num_map[next_join_node]), (unsigned long) 1);
//
//				cout << "estimation is " << card_estimation << endl;
//
//				if (card_cache.size() < 1) {
//					map<vector<int>, unsigned long> this_var_num_card_map;
//					this_var_num_card_map.insert(make_pair(now_plan_nodes, card_estimation));
//					card_cache.push_back(this_var_num_card_map);
//
//					map<vector<int>, vector<vector<unsigned>>> this_var_num_sample_map;
//					this_var_num_sample_map.insert(make_pair(now_plan_nodes, this_sample));
//					sample_cache.push_back(this_var_num_sample_map);
//
//				} else {
//					card_cache[0].insert(make_pair(now_plan_nodes, card_estimation));
//					sample_cache[0].insert(make_pair(now_plan_nodes, this_sample));
//				}
//				return card_estimation;
//
//			}
//
//			if (now_plan_node_num >= 3) {
//
//				if (sample_cache[now_plan_node_num - 3][last_plan_nodes].empty()) {
//					unsigned long card_estimation = 1;
//					if (card_cache.size() < now_plan_node_num - 1) {
//						map<vector<int>, unsigned long> this_var_num_card_map;
//						this_var_num_card_map.insert(make_pair(now_plan_nodes, card_estimation));
//						card_cache.push_back(this_var_num_card_map);
//
//						map<vector<int>, vector<vector<unsigned>>> this_var_num_sample_map;
//						vector<vector<unsigned>> null_sample_vec;
//						this_var_num_sample_map.insert(make_pair(now_plan_nodes, null_sample_vec));
//						sample_cache.push_back(this_var_num_sample_map);
//
//					} else {
//						card_cache[now_plan_node_num - 2].insert(make_pair(now_plan_nodes, card_estimation));
//						vector<vector<unsigned >> null_sample_vec;
//						sample_cache[now_plan_node_num - 2].insert(make_pair(now_plan_nodes, null_sample_vec));
//					}
//					return card_estimation;
//				}
//
//				unsigned now_sample_num = 0;
//				vector<vector<unsigned >> now_sample;
//
//				auto &last_sample = sample_cache[now_plan_node_num - 3][last_plan_nodes];
//				cout << "last_sample size: " << last_sample.size() << endl;
//
//				vector<int> linked_nei_pos;
//				vector<char> edge_type;
//				vector<TYPE_PREDICATE_ID> p_list;
//
//				for (int i = 0; i < basicquery->getVarDegree(next_join_node); ++i) {
//					if (find(last_plan_nodes.begin(), last_plan_nodes.end(),
//							 basicquery->getEdgeNeighborID(next_join_node, i))
//						!= last_plan_nodes.end()) {
//						for (int j = 0; j < last_plan_nodes.size(); ++j) {
//							if (basicquery->getEdgeNeighborID(next_join_node, i) == last_plan_nodes[j]) {
//								linked_nei_pos.push_back(j);
//								break;
//							}
//						}
//
//						edge_type.push_back(basicquery->getEdgeType(next_join_node, i));
//						p_list.push_back(basicquery->getEdgePreID(next_join_node, i));
//					}
//				}
//
//				for (int i = 0; i < last_sample.size(); ++i) {
//					for (int j = 0; j < var_to_sample_map[next_join_node].size(); ++j) {
//						if (check_pass(linked_nei_pos, edge_type, p_list,
//									   last_sample[i], var_to_sample_map[next_join_node][j])) {
//							now_sample_num += 1;
//
//							if (now_sample.size() < SAMPLE_CACHE_MAX) {
////			unsigned *need_insert_sample = new unsigned[this_join_var_num];
//								vector<unsigned> need_insert_sample(now_plan_node_num);
//								for (int k = 0, index = 0; k < now_plan_node_num; ++k) {
//									if (index == now_plan_node_num - 1 || next_join_node < last_plan_nodes[index]) {
//										need_insert_sample[k] = var_to_sample_map[next_join_node][j];
//
//									} else {
//										need_insert_sample[k] = last_sample[i][index++];
//									}
//								}
//
//								now_sample.push_back(need_insert_sample);
//							} else {
////							if ((double) rand() / (double) RAND_MAX < 1.0 / now_sample_num)
//								if ((double) rand() / (double) RAND_MAX < SAMPLE_PRO) {
//
////								int index_to_replace = rand() % SAMPLE_CACHE_MAX;
////
////								for (int k = 0, index = 0; k < now_plan_node_num; ++k) {
////									if (index == now_plan_node_num - 1 || next_join_node < last_plan_nodes[index]) {
////										now_sample[index_to_replace][k] = var_to_sample_map[next_join_node][j];
////									} else {
////										now_sample[index_to_replace][k] = last_sample[i][index++];
////									}
////								}
//									vector<unsigned> need_insert_sample(now_plan_node_num);
//									for (int k = 0, index = 0; k < now_plan_node_num; ++k) {
//										if (index == now_plan_node_num - 1 || next_join_node < last_plan_nodes[index]) {
//											need_insert_sample[k] = var_to_sample_map[next_join_node][j];
//
//										} else {
//											need_insert_sample[k] = last_sample[i][index++];
//										}
//									}
//
//									now_sample.push_back(need_insert_sample);
//
//								}
//							}
//
//						}
//					}
//				}
//
////	todo: card 计算
//
//
//				unsigned long last_plan_card = card_cache[now_plan_node_num - 3][last_plan_nodes];
//
//				unsigned long card_estimation = max((unsigned long) ((double) last_plan_card / last_sample.size() *
//																	 var_to_num_map[next_join_node] /
//																	 var_to_sample_map[next_join_node].size()),
//													(unsigned long) 1);
//
//				if (card_cache.size() < now_plan_node_num - 1) {
//					map<vector<int>, unsigned long> this_var_num_card_map;
//					this_var_num_card_map.insert(make_pair(now_plan_nodes, card_estimation));
//					card_cache.push_back(this_var_num_card_map);
//
//					map<vector<int>, vector<vector<unsigned>>> this_var_num_sample_map;
//					this_var_num_sample_map.insert(make_pair(now_plan_nodes, now_sample));
//					sample_cache.push_back(this_var_num_sample_map);
//
//				} else {
//					card_cache[now_plan_node_num - 2].insert(make_pair(now_plan_nodes, card_estimation));
//					sample_cache[now_plan_node_num - 2].insert(make_pair(now_plan_nodes, now_sample));
//				}
//				return card_estimation;
//			}
//		}

//		add at 5.12




    if(now_plan_node_num == 2) {

      if (card_cache.size()==0 || card_cache[0].find(now_plan_nodes) == card_cache[0].end()) {

        unsigned long card_estimation;
        vector<vector<unsigned>> this_sample;

        unsigned now_sample_num = 0;

        vector<int> linked_nei_id;
        vector<char> edge_type;
        vector<TYPE_PREDICATE_ID> p_list;

        for (int i = 0; i < basicquery->getVarDegree(next_join_node); ++i) {
          if (basicquery->getEdgeNeighborID(next_join_node, i) == last_plan_nodes[0]) {
            linked_nei_id.push_back(i);
            edge_type.push_back(basicquery->getEdgeType(next_join_node, i));
            p_list.push_back(basicquery->getEdgePreID(next_join_node, i));
          }
        }


        map<int, int> new_id_pos_map;
        if (last_plan_nodes[0] < next_join_node) {
          new_id_pos_map[last_plan_nodes[0]] = 0;
          new_id_pos_map[next_join_node] = 1;
        } else {
          new_id_pos_map[next_join_node] = 0;
          new_id_pos_map[last_plan_nodes[0]] = 1;
        }

        unsigned long s_o_list1_total_num = 0;
        unsigned long s_o_list2_total_num = 0;

        if (edge_type[0] == Util::EDGE_IN) {
          for (int i = 0; i < var_to_sample_map[last_plan_nodes[0]].size(); ++i) {
            unsigned *s_o_list = nullptr;
            unsigned s_o_list_len = 0;

            kv_store_->getobjIDlistBysubIDpreID(var_to_sample_map[last_plan_nodes[0]][i], p_list[0],
                                                s_o_list,
                                                s_o_list_len);


            s_o_list1_total_num += s_o_list_len;

            for (int j = 0; j < s_o_list_len; ++j) {
              if (binary_search((*id_caches)[next_join_node]->begin(),
                                (*id_caches)[next_join_node]->end(),
                                s_o_list[j])) {
                int k = 1;
                for (k = 1; k < edge_type.size(); ++k) {
                  if (edge_type[k] == Util::EDGE_IN) {
                    if (!check_exist_this_triple(var_to_sample_map[last_plan_nodes[0]][i],
                                                 p_list[k],
                                                 s_o_list[j])) {
                      break;
                    }
                  } else {
                    if (!check_exist_this_triple(s_o_list[j], p_list[k],
                                                 var_to_sample_map[last_plan_nodes[0]][i])) {
                      break;
                    }
                  }

                }

                if (k == edge_type.size()) { ;
                  ++now_sample_num;
                  if (now_sample_num < SAMPLE_CACHE_MAX) {
//
                    vector<unsigned> this_pass_sample(2);
                    this_pass_sample[new_id_pos_map[last_plan_nodes[0]]] = var_to_sample_map[last_plan_nodes[0]][i];
                    this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

                    this_sample.push_back(this_pass_sample);
                  } else {
//							if ((double) rand() / (double) RAND_MAX < 1.0 / now_sample_num)
                    if ((double) rand() / (double) RAND_MAX < SAMPLE_PRO) {

//								int index_to_replace = rand() % SAMPLE_CACHE_MAX;
//
//								this_sample[index_to_replace][new_id_pos_map[last_plan_nodes[0]]] = var_to_sample_map[last_plan_nodes[0]][i];
//								this_sample[index_to_replace][new_id_pos_map[next_join_node]] = var_to_sample_map[next_join_node][j];

                      vector<unsigned> this_pass_sample(2);
                      this_pass_sample[new_id_pos_map[last_plan_nodes[0]]] = var_to_sample_map[last_plan_nodes[0]][i];
                      this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

                      this_sample.push_back(this_pass_sample);

                    }
                  }
                }

              }
            }

            delete[] s_o_list;


          }

          for (int i = 0; i < var_to_sample_map[next_join_node].size(); ++i) {
            unsigned *s_o_list = nullptr;
            unsigned s_o_list_len = 0;

            kv_store_->getsubIDlistByobjIDpreID(var_to_sample_map[next_join_node][i], p_list[0], s_o_list,
                                                s_o_list_len);
            s_o_list2_total_num += s_o_list_len;

            delete[] s_o_list;


          }

          if (s_o_list_average_size.find(last_plan_nodes[0]) == s_o_list_average_size.end()) {
            map<int, unsigned > this_node_selectivity_map;
            this_node_selectivity_map.insert(
                make_pair(next_join_node, (unsigned )((double) s_o_list1_total_num / var_to_sample_map[last_plan_nodes[0]].size())));
            s_o_list_average_size.insert(make_pair(last_plan_nodes[0], this_node_selectivity_map));
          } else {
            s_o_list_average_size[last_plan_nodes[0]].insert(
                make_pair(next_join_node, (unsigned )((double) s_o_list1_total_num / var_to_sample_map[last_plan_nodes[0]].size())));
          }

          if (s_o_list_average_size.find(next_join_node) == s_o_list_average_size.end()) {
            map<int, unsigned > this_node_selectivity_map;
            this_node_selectivity_map.insert(
                make_pair(last_plan_nodes[0], (unsigned )((double) s_o_list2_total_num / var_to_sample_map[next_join_node].size())));
            s_o_list_average_size.insert(make_pair(next_join_node, this_node_selectivity_map));
          } else {
            s_o_list_average_size[next_join_node].insert(
                make_pair(last_plan_nodes[0], (unsigned )((double) s_o_list2_total_num / var_to_sample_map[next_join_node].size())));
          }

        } else {


          for (int i = 0; i < var_to_sample_map[next_join_node].size(); ++i) {
            unsigned *s_o_list = nullptr;
            unsigned s_o_list_len = 0;

            kv_store_->getobjIDlistBysubIDpreID(var_to_sample_map[next_join_node][i], p_list[0], s_o_list,
                                                s_o_list_len);

            s_o_list1_total_num += s_o_list_len;

            for (int j = 0; j < s_o_list_len; ++j) {
              if (binary_search((*id_caches)[last_plan_nodes[0]]->begin(),
                                (*id_caches)[last_plan_nodes[0]]->end(), s_o_list[j])) {
                int k = 1;
                for (k = 1; k < edge_type.size(); ++k) {
                  if (edge_type[k] == Util::EDGE_IN) {
                    if (!check_exist_this_triple(s_o_list[j], p_list[k],
                                                 var_to_sample_map[next_join_node][i])) {
                      break;
                    }
                  } else {
                    if (!check_exist_this_triple(var_to_sample_map[next_join_node][i], p_list[k],
                                                 s_o_list[j])) {
                      break;
                    }
                  }

                }

                if (k == edge_type.size()) { ;
                  ++now_sample_num;
                  if (now_sample_num < SAMPLE_CACHE_MAX) {
//
                    vector<unsigned> this_pass_sample(2);
                    this_pass_sample[new_id_pos_map[last_plan_nodes[0]]] = s_o_list[j];
                    this_pass_sample[new_id_pos_map[next_join_node]] = var_to_sample_map[next_join_node][i];

                    this_sample.push_back(this_pass_sample);
                  } else {
//							if ((double) rand() / (double) RAND_MAX < 1.0 / now_sample_num)
                    if ((double) rand() / (double) RAND_MAX < SAMPLE_PRO) {

//								int index_to_replace = rand() % SAMPLE_CACHE_MAX;
//
//								this_sample[index_to_replace][new_id_pos_map[last_plan_nodes[0]]] = var_to_sample_map[last_plan_nodes[0]][i];
//								this_sample[index_to_replace][new_id_pos_map[next_join_node]] = var_to_sample_map[next_join_node][j];

                      vector<unsigned> this_pass_sample(2);
                      this_pass_sample[new_id_pos_map[last_plan_nodes[0]]] = s_o_list[j];
                      this_pass_sample[new_id_pos_map[next_join_node]] = var_to_sample_map[next_join_node][i];

                      this_sample.push_back(this_pass_sample);

                    }
                  }
                }

              }
            }

            delete[] s_o_list;
          }

          for (int i = 0; i < var_to_sample_map[last_plan_nodes[0]].size(); ++i) {
            unsigned *s_o_list = nullptr;
            unsigned s_o_list_len = 0;

            kv_store_->getsubIDlistByobjIDpreID(var_to_sample_map[last_plan_nodes[0]][i], p_list[0],
                                                s_o_list,
                                                s_o_list_len);
            s_o_list2_total_num += s_o_list_len;

            delete[] s_o_list;


          }

          if (s_o_list_average_size.find(next_join_node) == s_o_list_average_size.end()) {
            map<int, unsigned > this_node_selectivity_map;
            this_node_selectivity_map.insert(
                make_pair(last_plan_nodes[0], (unsigned )((double) s_o_list1_total_num / var_to_sample_map[next_join_node].size())));
            s_o_list_average_size.insert(make_pair(next_join_node, this_node_selectivity_map));
          } else {
            s_o_list_average_size[next_join_node].insert(
                make_pair(last_plan_nodes[0],(unsigned )((double) s_o_list1_total_num / var_to_sample_map[next_join_node].size())));
          }

          if (s_o_list_average_size.find(last_plan_nodes[0]) == s_o_list_average_size.end()) {
            map<int, unsigned > this_node_selectivity_map;
            this_node_selectivity_map.insert(
                make_pair(next_join_node, (unsigned )((double) s_o_list2_total_num / var_to_sample_map[last_plan_nodes[0]].size())));
            s_o_list_average_size.insert(make_pair(last_plan_nodes[0], this_node_selectivity_map));
          } else {
            s_o_list_average_size[last_plan_nodes[0]].insert(
                make_pair(next_join_node, (unsigned )((double) s_o_list2_total_num / var_to_sample_map[last_plan_nodes[0]].size())));
          }

        }

        card_estimation = max((edge_type[0] == Util::EDGE_IN) ? (unsigned long) ((double) now_sample_num /
            var_to_sample_map[last_plan_nodes[0]].size() *
            var_to_num_map[last_plan_nodes[0]])
                                                              :
                              (unsigned long) ((double) now_sample_num /
                                  var_to_sample_map[next_join_node].size() *
                                  var_to_num_map[next_join_node]), (unsigned long) 1);

        if (card_cache.size() < 1) {
          map<vector<int>, unsigned long> this_var_num_card_map;
          this_var_num_card_map.insert(make_pair(now_plan_nodes, card_estimation));
          card_cache.push_back(this_var_num_card_map);

          map<vector<int>, vector<vector<unsigned>>> this_var_num_sample_map;
          this_var_num_sample_map.insert(make_pair(now_plan_nodes, this_sample));
          sample_cache.push_back(this_var_num_sample_map);

        } else {
          card_cache[0].insert(make_pair(now_plan_nodes, card_estimation));
          sample_cache[0].insert(make_pair(now_plan_nodes, this_sample));
        }
        return var_to_num_map[last_plan_nodes[0]]*s_o_list_average_size[last_plan_nodes[0]][next_join_node];
      } else{
        return var_to_num_map[last_plan_nodes[0]]*s_o_list_average_size[last_plan_nodes[0]][next_join_node];//+var_to_num_map[next_join_node];

      }
    }


    if(now_plan_node_num >=3) {

      if (card_cache.size() < now_plan_node_num - 1 ||
          card_cache[now_plan_node_num - 2].find(now_plan_nodes) == card_cache[now_plan_node_num - 2].end()) {

        unsigned long card_estimation;
        unsigned long last_card_estimation = card_cache[now_plan_node_num - 3][last_plan_nodes];
        vector<vector<unsigned>> this_sample;
        vector<vector<unsigned>> &last_sample = sample_cache[now_plan_node_num - 3][last_plan_nodes];

        if (last_sample.size() != 0) {

          unsigned now_sample_num = 0;

          vector<int> linked_nei_pos;
          vector<char> edge_type;
          vector<TYPE_PREDICATE_ID> p_list;


          for (int i = 0; i < basicquery->getVarDegree(next_join_node); ++i) {
            if (find(last_plan_nodes.begin(), last_plan_nodes.end(),
                     basicquery->getEdgeNeighborID(next_join_node, i))
                != last_plan_nodes.end()) {
              for (int j = 0; j < last_plan_nodes.size(); ++j) {
                if (basicquery->getEdgeNeighborID(next_join_node, i) == last_plan_nodes[j]) {
                  linked_nei_pos.push_back(j);
                  break;
                }
              }

              edge_type.push_back(basicquery->getEdgeType(next_join_node, i));
              p_list.push_back(basicquery->getEdgePreID(next_join_node, i));
            }
          }

          map<int, int> new_id_pos_map;
          for (int i = 0, index = 0, used = 0; i < now_plan_node_num; ++i) {
            if ((index == now_plan_node_num - 1) || (!used && next_join_node < last_plan_nodes[index])) {
              new_id_pos_map[next_join_node] = i;
              used = 1;
            } else {
              new_id_pos_map[last_plan_nodes[index]] = i;
              ++index;
            }
          }



          if (edge_type[0] == Util::EDGE_IN) {
            for (int i = 0; i < last_sample.size(); ++i) {
              unsigned *s_o_list = nullptr;
              unsigned s_o_list_len = 0;

              kv_store_->getobjIDlistBysubIDpreID(last_sample[i][linked_nei_pos[0]], p_list[0], s_o_list,
                                                  s_o_list_len);


              for (int j = 0; j < s_o_list_len; ++j) {
                if (binary_search((*id_caches)[next_join_node]->begin(),
                                  (*id_caches)[next_join_node]->end(),
                                  s_o_list[j])) {
                  int k = 1;
                  for (k = 1; k < edge_type.size(); ++k) {
                    if (edge_type[k] == Util::EDGE_IN) {
                      if (!check_exist_this_triple(last_sample[i][linked_nei_pos[k]], p_list[k],
                                                   s_o_list[j])) {
                        break;
                      }
                    } else {
                      if (!check_exist_this_triple(s_o_list[j], p_list[k],
                                                   last_sample[i][linked_nei_pos[k]])) {
                        break;
                      }
                    }

                  }

                  if (k == edge_type.size()) { ;
                    ++now_sample_num;
                    if (now_sample_num < SAMPLE_CACHE_MAX) {
//
                      vector<unsigned> this_pass_sample(now_plan_node_num);
                      for (int l = 0; l < now_plan_node_num - 1; ++l) {
                        this_pass_sample[new_id_pos_map[last_plan_nodes[l]]] = last_sample[i][l];
                      }
                      this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

                      this_sample.push_back(this_pass_sample);
                    } else {
//							if ((double) rand() / (double) RAND_MAX < 1.0 / now_sample_num)
                      if ((double) rand() / (double) RAND_MAX < SAMPLE_PRO) {

//								int index_to_replace = rand() % SAMPLE_CACHE_MAX;
//
//								this_sample[index_to_replace][new_id_pos_map[last_plan_nodes[0]]] = var_to_sample_map[last_plan_nodes[0]][i];
//								this_sample[index_to_replace][new_id_pos_map[next_join_node]] = var_to_sample_map[next_join_node][j];

                        vector<unsigned> this_pass_sample(now_plan_node_num);
                        for (int l = 0; l < now_plan_node_num - 1; ++l) {
                          this_pass_sample[new_id_pos_map[last_plan_nodes[l]]] = last_sample[i][l];
                        }
                        this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

                        this_sample.push_back(this_pass_sample);

                      }
                    }
                  }

                }
              }

              delete[] s_o_list;


            }
          } else {
            for (int i = 0; i < last_sample.size(); ++i) {
              unsigned *s_o_list = nullptr;
              unsigned s_o_list_len = 0;

              kv_store_->getsubIDlistByobjIDpreID(last_sample[i][linked_nei_pos[0]], p_list[0], s_o_list,
                                                  s_o_list_len);


              for (int j = 0; j < s_o_list_len; ++j) {
                if (binary_search((*id_caches)[next_join_node]->begin(),
                                  (*id_caches)[next_join_node]->end(),
                                  s_o_list[j])) {
                  int k = 1;
                  for (k = 1; k < edge_type.size(); ++k) {
                    if (edge_type[k] == Util::EDGE_IN) {
                      if (!check_exist_this_triple(last_sample[i][linked_nei_pos[k]], p_list[k],
                                                   s_o_list[j])) {
                        break;
                      }
                    } else {
                      if (!check_exist_this_triple(s_o_list[j], p_list[k],
                                                   last_sample[i][linked_nei_pos[k]])) {
                        break;
                      }
                    }

                  }

                  if (k == edge_type.size()) { ;
                    ++now_sample_num;
                    if (now_sample_num < SAMPLE_CACHE_MAX) {
//
                      vector<unsigned> this_pass_sample(now_plan_node_num);
                      for (int l = 0; l < now_plan_node_num - 1; ++l) {
                        this_pass_sample[new_id_pos_map[last_plan_nodes[l]]] = last_sample[i][l];
                      }
                      this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

                      this_sample.push_back(this_pass_sample);
                    } else {
//							if ((double) rand() / (double) RAND_MAX < 1.0 / now_sample_num)
                      if ((double) rand() / (double) RAND_MAX < SAMPLE_PRO) {

//								int index_to_replace = rand() % SAMPLE_CACHE_MAX;
//
//								this_sample[index_to_replace][new_id_pos_map[last_plan_nodes[0]]] = var_to_sample_map[last_plan_nodes[0]][i];
//								this_sample[index_to_replace][new_id_pos_map[next_join_node]] = var_to_sample_map[next_join_node][j];

                        vector<unsigned> this_pass_sample(now_plan_node_num);
                        for (int l = 0; l < now_plan_node_num - 1; ++l) {
                          this_pass_sample[new_id_pos_map[last_plan_nodes[l]]] = last_sample[i][l];
                        }
                        this_pass_sample[new_id_pos_map[next_join_node]] = s_o_list[j];

                        this_sample.push_back(this_pass_sample);

                      }
                    }
                  }

                }
              }

              delete[] s_o_list;


            }

          }

          card_estimation = max(
              (unsigned long) ((double) (now_sample_num) / last_sample.size() * last_card_estimation),
              (unsigned long) 1);

        } else {
          card_estimation = 1;
        }

        if (card_cache.size() < now_plan_node_num - 1) {
          map<vector<int>, unsigned long> this_var_num_card_map;
          this_var_num_card_map.insert(make_pair(now_plan_nodes, card_estimation));
          card_cache.push_back(this_var_num_card_map);

          map<vector<int>, vector<vector<unsigned>>> this_var_num_sample_map;
          this_var_num_sample_map.insert(make_pair(now_plan_nodes, this_sample));
          sample_cache.push_back(this_var_num_sample_map);

        } else {
          card_cache[now_plan_node_num - 2].insert(make_pair(now_plan_nodes, card_estimation));
          sample_cache[now_plan_node_num - 2].insert(make_pair(now_plan_nodes, this_sample));
        }

        vector<int> linked_nei_id;
        for (int i = 0; i < basicquery->getVarDegree(next_join_node); ++i) {
          if (find(last_plan_nodes.begin(), last_plan_nodes.end(), basicquery->getEdgeNeighborID(next_join_node, i))
              != last_plan_nodes.end()) {
            linked_nei_id.push_back(basicquery->getEdgeNeighborID(next_join_node, i));
          }
        }

        unsigned multiple = 1+s_o_list_average_size[linked_nei_id[0]][next_join_node];
        for(auto x : linked_nei_id){
          multiple = min(multiple, s_o_list_average_size[x][next_join_node]+1);
//					multiple += s_o_list_average_size[x][next_join_node];

        }
        return card_cache[now_plan_node_num - 3][last_plan_nodes]*multiple;

      } else{

        vector<int> linked_nei_id;
        for (int i = 0; i < basicquery->getVarDegree(next_join_node); ++i) {
          if (find(last_plan_nodes.begin(), last_plan_nodes.end(), basicquery->getEdgeNeighborID(next_join_node, i))
              != last_plan_nodes.end()) {
            linked_nei_id.push_back(basicquery->getEdgeNeighborID(next_join_node, i));
          }
        }
        unsigned multiple = 1+s_o_list_average_size[linked_nei_id[0]][next_join_node];
        for(auto x : linked_nei_id){
          multiple = min(multiple, s_o_list_average_size[x][next_join_node]+1);
        }

        return card_cache[now_plan_node_num - 3][last_plan_nodes]*multiple;//+var_to_num_map[next_join_node];
      }
    }

  }
}

unsigned long Optimizer::get_card(vector<map<vector<int>, unsigned long>> &card_cache, const vector<int> &nodes){
  return card_cache[nodes.size()-2][nodes];
}

unsigned long Optimizer::cost_model_for_wco(BasicQuery* basicquery, IDCachesSharePtr &id_caches, PlanTree* last_plan,
                                            const vector<int> &last_plan_node, int next_node, const vector<int> &now_plan_node,
                                            map<int, unsigned> &var_to_num_map, map<int, TYPE_ENTITY_LITERAL_ID> &var_to_type_map,
                                            map<int, vector<unsigned>> &var_to_sample_cache, map<int, map<int, unsigned >> &s_o_list_average_size,
                                            vector<map<vector<int>, unsigned long>> &card_cache, vector<map<vector<int>, vector<vector<unsigned>> >> &sample_cache){

  return last_plan->plan_cost + card_estimator(basicquery, id_caches, var_to_num_map, var_to_type_map, var_to_sample_cache,
                                               s_o_list_average_size, last_plan_node, next_node, now_plan_node, card_cache, sample_cache);
}

unsigned long Optimizer::cost_model_for_binary(vector<map<vector<int>, unsigned long>> &card_cache,
                                               const vector<int> &plan_a_nodes, const vector<int> &plan_b_nodes,
                                               PlanTree* plan_a, PlanTree* plan_b){

  unsigned long plan_a_card = get_card(card_cache, plan_a_nodes);
  unsigned long plan_b_card = get_card(card_cache, plan_b_nodes);
  unsigned long min_card = min(plan_a_card, plan_b_card);
  unsigned long max_card = max(plan_a_card, plan_b_card);

  return 2*min_card + max_card + plan_a->plan_cost + plan_b->plan_cost;
}

//    save every nei_id of now_in_plan_node in nei_node
void Optimizer::get_nei_by_subplan_nodes(BasicQuery* basicquery, const vector<int> &need_join_nodes,
                                         const vector<int> &last_plan_node, set<int> &nei_node){
  ;
  for(int node_in_plan : last_plan_node){
    for(int i = 0; i < basicquery->getVarDegree(node_in_plan); ++i){
      if(find(last_plan_node.begin(), last_plan_node.end(), basicquery->getEdgeNeighborID(node_in_plan, i))
          == last_plan_node.end() && basicquery->getEdgeNeighborID(node_in_plan, i) != -1 &&
          find(need_join_nodes.begin(), need_join_nodes.end(), basicquery->getEdgeNeighborID(node_in_plan, i))
              != need_join_nodes.end()){
        nei_node.insert(basicquery->getEdgeNeighborID(node_in_plan, i));
      }
    }
  }
}

void Optimizer::get_join_nodes(BasicQuery* basicquery, const vector<int> &plan_a_nodes, vector<int> &other_nodes, set<int> &join_nodes){
  for(int node : other_nodes){
    for(int i = 0; i < basicquery->getVarDegree(node); ++i){
      if(find(plan_a_nodes.begin(), plan_a_nodes.end(), basicquery->getEdgeNeighborID(node, i))!= plan_a_nodes.end()){
        join_nodes.insert(basicquery->getEdgeNeighborID(node, i));
      }
    }
  }
}

// first node
void Optimizer::considerallscan(BasicQuery* basicquery, IDCachesSharePtr &id_caches,
                                vector<map<vector<int>, list<PlanTree*>>> &plan_cache,
                                map<int, unsigned > &var_to_num_map,
                                map<int, TYPE_ENTITY_LITERAL_ID> &var_to_type_map,
                                map<int, vector<unsigned>> &var_to_sample_cache, vector<int> &need_join_nodes, bool use_sample){

  for(int i = 0 ; i < basicquery->getVarNum(); ++i) {

    if (basicquery->if_need_retrieve(i)) {

      need_join_nodes.push_back(i);
      vector<int> this_node{i};
      PlanTree *new_scan = new PlanTree(i);


      var_to_num_map[i] = (*id_caches)[i]->size();
      new_scan->plan_cost = var_to_num_map[i];

      list<PlanTree *> this_node_plan;
      this_node_plan.push_back(new_scan);

      if (plan_cache.size() < 1) {
        map<vector<int>, list<PlanTree *>> one_node_plan_map;

        one_node_plan_map.insert(make_pair(this_node, this_node_plan));

        plan_cache.push_back(one_node_plan_map);
      } else {
        plan_cache[0].insert(make_pair(this_node, this_node_plan));
      }

      if (use_sample) {

        unsigned sample_num = 0;

        int strategy = get_strategy((*id_caches)[i]->size());

        vector<unsigned> need_insert_vec;

        switch (strategy) {
          case 1:
            for (int index = 0; index < (*id_caches)[i]->size(); ++index) {
              need_insert_vec.push_back((*id_caches)[i]->getID(index));
            }
            break;
          case 2:
            for (int index = 0; index < SAMPLE_CACHE_MAX; ++index) {
              need_insert_vec.push_back((*id_caches)[i]->getID(index));
            }
            for (int index = SAMPLE_CACHE_MAX; index < (*id_caches)[i]->size(); ++index) {
              if ((double) rand() / RAND_MAX < 1.0 / index) {
                int index_to_replace = rand() % SAMPLE_CACHE_MAX;
                need_insert_vec[index_to_replace] = (*id_caches)[i]->getID(index);
              }
            }
            break;
          case 3:
            for (int index = 0; index < SAMPLE_CACHE_MAX; ++index) {
              need_insert_vec.push_back((*id_caches)[i]->getID(index));
            }
            for (int index = SAMPLE_CACHE_MAX; index < 1000; ++index) {
              if ((double) rand() / RAND_MAX < 1.0 / index) {
                int index_to_replace = rand() % SAMPLE_CACHE_MAX;
                need_insert_vec[index_to_replace] = (*id_caches)[i]->getID(index);
              }
            }
            for (unsigned index = 1000; index < (*id_caches)[i]->size(); ++index) {
              if ((double) rand() / RAND_MAX < SAMPLE_PRO) {
                need_insert_vec.push_back((*id_caches)[i]->getID(index));
              }
            }
            break;

        }

        var_to_sample_cache.insert(make_pair(i, need_insert_vec));
      }

      cout << "degree: " << basicquery->getVarDegree(i) << endl;
      for (int j = 0; j < basicquery->getVarDegree(i); ++j) {
        cout << "type pre id: " << statistics->type_pre_id << endl;
        if (basicquery->getEdgePreID(i, j) == statistics->type_pre_id) {

          int triple_id = basicquery->getEdgeID(i, j);
          string type_name = basicquery->getTriple(triple_id).object;
          cout << "type_name: " << type_name << endl;
          var_to_type_map[i] = kv_store_->getIDByEntity(type_name);

        }
      }
    }
  }

}

// add one node, the added node need to be linked by nodes in plan before
void Optimizer::considerwcojoin(BasicQuery* basicquery,IDCachesSharePtr &id_caches,  int node_num, const vector<int> &need_join_nodes,
                                vector<map<vector<int>, list<PlanTree*>>> &plan_cache, map<int, unsigned> &var_to_num_map, map<int, TYPE_ENTITY_LITERAL_ID> &var_to_type_map,
                                map<int, vector<unsigned>> &var_to_sample_cache, map<int, map<int, unsigned >> &s_o_list_average_size,
                                vector<map<vector<int>, unsigned long>> &card_cache, vector<map<vector<int>, vector<vector<unsigned>> >> &sample_cache){
  auto plan_tree_list = plan_cache[node_num - 2];
  for(const auto &last_node_plan : plan_tree_list){
    set<int> nei_node;

    get_nei_by_subplan_nodes(basicquery, need_join_nodes, last_node_plan.first, nei_node);

    PlanTree* last_best_plan = get_best_plan(last_node_plan.first, plan_cache);

    for(int next_node : nei_node) {

      vector<int> new_node_vec(last_node_plan.first);
      new_node_vec.push_back(next_node);
      sort(new_node_vec.begin(), new_node_vec.end());


      PlanTree* new_plan = new PlanTree(last_best_plan, next_node);
      unsigned long cost = cost_model_for_wco(basicquery, id_caches, last_best_plan, last_node_plan.first,
                                              next_node, new_node_vec, var_to_num_map, var_to_type_map, var_to_sample_cache,
                                              s_o_list_average_size,card_cache, sample_cache);
      new_plan->plan_cost = cost;

      if(plan_cache.size() < node_num){
        map<vector<int>, list<PlanTree*>> this_num_node_plan;
        list<PlanTree*> this_nodes_plan;
        this_nodes_plan.push_back(new_plan);
        this_num_node_plan.insert(make_pair(new_node_vec, this_nodes_plan));
        plan_cache.push_back(this_num_node_plan);

      } else{
        if(plan_cache[node_num-1].find(new_node_vec) == plan_cache[node_num-1].end()){
          list<PlanTree*> this_nodes_plan;
          this_nodes_plan.push_back(new_plan);
          plan_cache[node_num-1].insert(make_pair(new_node_vec, this_nodes_plan));
        }else{
          plan_cache[node_num-1][new_node_vec].push_back(new_plan);
        }
      }

    }
  }

}

// not add nodes, but to consider if binaryjoin could decrease cost
void Optimizer::considerbinaryjoin(BasicQuery* basicquery, int node_num,
                                   vector<map<vector<int>, unsigned long>> &card_cache,
                                   vector<map<vector<int>, list<PlanTree*>>> &plan_cache){

  int min_size = 3;
  int max_size = min(min_size, node_num - 2);


  for(const auto &need_considerbinaryjoin_nodes_plan : plan_cache[node_num - 1]){

    unsigned long last_plan_smallest_cost = get_best_plan(need_considerbinaryjoin_nodes_plan.first, plan_cache)->plan_cost;

    for(int small_plan_nodes_num = min_size; small_plan_nodes_num <= max_size; ++ small_plan_nodes_num){
      for(const auto &small_nodes_plan : plan_cache[small_plan_nodes_num-1]){
        if(OrderedVector::contain_sub_vec(need_considerbinaryjoin_nodes_plan.first, small_nodes_plan.first)) {
          vector<int> other_nodes;
          OrderedVector::subtract(need_considerbinaryjoin_nodes_plan.first, small_nodes_plan.first,other_nodes);
          set<int> join_nodes;
          get_join_nodes(basicquery, small_nodes_plan.first, other_nodes, join_nodes);

          if (join_nodes.size() >= 1 && join_nodes.size() + other_nodes.size() < node_num - 1) {

            OrderedVector::vec_set_union(other_nodes, join_nodes);

            if (plan_cache[other_nodes.size() - 1].find(other_nodes) !=
                plan_cache[other_nodes.size() - 1].end()) {

              PlanTree *small_best_plan = get_best_plan(small_nodes_plan.first, plan_cache);
              PlanTree *another_small_best_plan = get_best_plan(other_nodes, plan_cache);

              unsigned long now_cost = cost_model_for_binary(card_cache, small_nodes_plan.first,
                                                             other_nodes, small_best_plan,
                                                             another_small_best_plan);

              if (now_cost < last_plan_smallest_cost) {
//                            build new plan and add to plan_cache
                PlanTree *new_plan = new PlanTree(small_best_plan, another_small_best_plan);
                new_plan->plan_cost = now_cost;
                plan_cache[node_num - 1][need_considerbinaryjoin_nodes_plan.first].push_back(new_plan);
                last_plan_smallest_cost = now_cost;

              }
            }
          }
        }

      }
    }
  }
}


//  enumerate all execution plan, and build cost_cache
int Optimizer::enum_query_plan(BasicQuery* basicquery, KVstore *kvstore, IDCachesSharePtr& id_caches,
                               vector<int> &need_join_nodes,
                               vector<map<vector<int>, list<PlanTree*>>> &plan_cache){

  vector<map<vector<int>, unsigned long>> card_cache;
  vector<map<vector<int>, vector<vector<unsigned>> >> sample_cache;

  map<int, unsigned > var_to_num_map;
  map<int, TYPE_ENTITY_LITERAL_ID > var_to_type_map;
  map<int, vector<unsigned>> var_to_sample_cache;
  map<int, map<int, unsigned >> s_o_list_average_size;


  long t1 = Util::get_cur_time();
  considerallscan(basicquery, id_caches, plan_cache,
                  var_to_num_map, var_to_type_map, var_to_sample_cache, need_join_nodes);
  long t2 = Util::get_cur_time();
  cout << "consider all scan, used " << (t2 - t1) << "ms." << endl;
  for(auto x : var_to_num_map){
    cout << "var: " << basicquery->getVarName(x.first) << ", num: " << x.second << endl;
  }
  for(auto x : var_to_sample_cache){
    cout << "var: " << basicquery->getVarName(x.first) << ", sample num: " << x.second.size() << endl;
  }




  for(int i = 1; i < need_join_nodes.size(); ++i){
    long t3 = Util::get_cur_time();
    considerwcojoin(basicquery, id_caches, i+1, need_join_nodes, plan_cache, var_to_num_map, var_to_type_map, var_to_sample_cache,
                    s_o_list_average_size,card_cache, sample_cache);
    long t4 = Util::get_cur_time();
    cout << "i = " << i << ", considerwcojoin, used " << (t4 - t3) << "ms." << endl;

    if(i >= 4){
//            begin when nodes_num >= 5
      considerbinaryjoin(basicquery, i+1, card_cache, plan_cache);
//      cout << "binary join consider here" << endl;
    }
  }

//	int nodes = 2;
//	for(auto &x:card_cache){
//		cout << "card for " << nodes++ <<" nodes" << endl;
//		for(auto y:x){
//			for(int node:y.first){
//				cout << node << " ";
//			}
//			cout << ", card estimation: " << y.second << endl;
//		}
//	}


  return need_join_nodes.size();
}

PlanTree* Optimizer::get_best_plan(const vector<int> &nodes,
                                   vector<map<vector<int>, list<PlanTree*>>> &plan_cache){

  PlanTree* best_plan;
  unsigned min_cost = ULONG_MAX;

  for(const auto &plan : plan_cache[nodes.size()-1][nodes]){
    if(plan->plan_cost < min_cost){
      best_plan = plan;
    }
  }

  return best_plan;
}

// only used when get best plan for all nodes in basicquery
PlanTree* Optimizer::get_best_plan_by_num(BasicQuery* basicquery, int total_var_num, vector<map<vector<int>, list<PlanTree*>>> &plan_cache){

  PlanTree* best_plan;
  unsigned min_cost = ULONG_MAX	;


//	for(int i =0;i < plan_cache.size();++i){
//		for(auto x:plan_cache[i]){
//			for(int j = 0;j<x.first.size();++j){
//				cout << x.first[j]<< " ";
//			}
//			cout <<endl;
//			for(auto y:x.second){
//				y->print(basicquery);
//			}
//		}
//	}

  int count = 0;
  for(const auto &nodes_plan : plan_cache[total_var_num-1]){
    for(const auto &plan_tree : nodes_plan.second){
//    	plan_tree->print(basicquery);
      count ++;
      if(plan_tree->plan_cost < min_cost){
        best_plan = plan_tree;
        min_cost = plan_tree->plan_cost;
      }
    }
  }
  cout << "during enumerate plans, get " << count << " possible best plans." << endl;
  return best_plan;
}


PlanTree* Optimizer::get_plan(BasicQuery* basicquery, KVstore *kvstore, IDCachesSharePtr& id_caches){

  vector<map<vector<int>, list<PlanTree*>>> plan_cache;
  vector<int> need_join_nodes;

  long t1 = Util::get_cur_time();
  int join_nodes_num = enum_query_plan(basicquery, kvstore, id_caches, need_join_nodes, plan_cache);
  long t2 = Util::get_cur_time();
  cout << "enum query plan, used " << (t2 - t1) << "ms." << endl;

  PlanTree* best_plan = get_best_plan_by_num(basicquery, join_nodes_num, plan_cache);
  long t3 = Util::get_cur_time();
  cout << "get best plan by num, used " << (t2 - t1) << "ms." << endl;

  for(int i = 0; i < basicquery->getVarNum(); ++i){
    if(find(need_join_nodes.begin(), need_join_nodes.end(), i) == need_join_nodes.end()
        && !basicquery->isSatelliteInJoin(i)){
      best_plan = new PlanTree(best_plan, i);
    }
  }

  //  selected, and not need_retrieve
  for(int i = 0; i < basicquery->getSelectVarNum(); ++i){
    if(!basicquery->if_need_retrieve(i)){
      best_plan = new PlanTree(best_plan, i);
    }
  }

  return best_plan;
}



tuple<bool, TableContentShardPtr> Optimizer::getAllSubObjID(bool need_literal)
{
  set<TYPE_ENTITY_LITERAL_ID> ids;
  for (TYPE_PREDICATE_ID i = 0; i < this->limitID_entity_; ++i) {
    auto entity_str = this->kv_store_->getEntityByID(i);
    if(entity_str!="")
      ids.insert(i);
  }
  if(need_literal) {
    for (TYPE_PREDICATE_ID i = Util::LITERAL_FIRST_ID; i < this->limitID_literal_; ++i) {
      auto entity_str = this->kv_store_->getLiteralByID(i);
      if (entity_str != "")
        ids.insert(i);
    }
  }
  auto result = make_shared<TableContent>();
  for(auto var_id: ids)
  {
    auto record = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();
    record->push_back(var_id);
    result->push_back(record);
  }
  return make_tuple(true,result);
}

/**
 *
 * @param basic_query
 * @param query_info
 * @param plan_tree_node
 * @param id_caches
 * @return PositionValueSharedPtr position2var
 */
tuple<bool,PositionValueSharedPtr, TableContentShardPtr> Optimizer::ExecutionBreathFirst(BasicQuery* basic_query,
                                                                                         const QueryInfo& query_info,
                                                                                         Tree_node* plan_tree_node,
                                                                                         IDCachesSharePtr id_caches)
{
  auto limit_num = query_info.limit_num_;
  // leaf node
  if( plan_tree_node->joinType == NodeJoinType::LeafNode)
  {
//    cout<<"ExecutionBreathFirst 0" ;
    auto node_added = plan_tree_node->node_to_join;
//    cout<<" node to join "<<node_added;
    cout<<"leafnode ["<<basic_query->getVarName(node_added)<<"]"<<endl;
    auto id_cache_it = id_caches->find(node_added);
    PositionValueSharedPtr pos_mapping = make_shared<PositionValue>();
    (*pos_mapping)[0] = node_added;
    auto result = make_shared<TableContent>();

    if(id_cache_it!=id_caches->end())
    {
      auto id_candidate_vec = id_cache_it->second->getList();
      for(auto var_id: *id_candidate_vec)
      {
        auto record = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();
        record->push_back(var_id);
        result->push_back(record);
      }
//      cout<<"ExecutionBreathFirst 1"<<endl;
//      cout<<"result size "<<result->size()<<endl;
      return make_tuple(true,pos_mapping,result);
    }
    else // No Constant Constraint,Return All IDs
    {
      auto leaf_var_id = plan_tree_node->node_to_join;
      bool has_in_edge = false;
      for (int edge_i = 0; edge_i < basic_query->getVarDegree(leaf_var_id); edge_i++)
      {
        if(basic_query->getEdgeType(leaf_var_id,edge_i)==Util::EDGE_IN)
        {
          has_in_edge = true;
          break;
        }
      }
      auto all_nodes_table = get<1>(getAllSubObjID(has_in_edge));
      cout<<"result size "<<all_nodes_table->size()<<endl;
      return make_tuple(true,pos_mapping,all_nodes_table);
    }
  }
  else // inner node
  {
    tuple<bool,PositionValueSharedPtr, TableContentShardPtr> left_r;
    tuple<bool,PositionValueSharedPtr, TableContentShardPtr> right_r;
//    cout<<"ExecutionBreathFirst 3"<<endl;
//    cout<<"plan_tree_node->joinType" << NodeJoinTypeStr(plan_tree_node->joinType)<<endl;

    if(plan_tree_node->joinType == NodeJoinType::JoinANode)
    {
      left_r = this->ExecutionBreathFirst(basic_query,query_info,plan_tree_node->left_node,id_caches);
      auto left_table = get<2>(left_r);
      auto left_pos_id_mapping = get<1>(left_r);

      auto left_ids = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();

      auto left_id_position = make_shared<PositionValue>();
      for(const auto& pos_id_pair:* left_pos_id_mapping) {
        left_ids->push_back(pos_id_pair.second);
        (*left_id_position)[pos_id_pair.second]= pos_id_pair.first;
      }
      auto node_to_join = plan_tree_node->right_node->node_to_join;

      cout<<"join node ["<<basic_query->getVarName(node_to_join)<<"]"<<",  "<<endl;
      auto one_step_join = QueryPlan::LinkWithPreviousNodes(basic_query, this->kv_store_, node_to_join,left_ids);
      (*left_pos_id_mapping)[left_pos_id_mapping->size()] = node_to_join;
      (*left_id_position)[node_to_join] = left_pos_id_mapping->size();
      auto join_node = one_step_join.join_node_;
      auto edges = *join_node->edges_;
      auto edge_c = *join_node->edges_constant_info_;
#ifdef TABLE_OPERATOR_DEBUG_INFO
      for(int i=0;i<edges.size();i++)
      {
        std::cout<<"edge["<<i<<"] "<<edges[i].toString()<<std::endl;
        std::cout<<"constant["<<i<<"] "<<edge_c[i].toString()<<std::endl;
      }
#endif
      long t1 = Util::get_cur_time();
      auto step_result = JoinANode(one_step_join.join_node_, left_table,left_id_position,id_caches);

      cout<<"result size "<<get<1>(step_result)->size();
      long t2 = Util::get_cur_time();
      cout<< "  used " << (t2 - t1) << "ms." <<endl;
      return make_tuple(true,left_pos_id_mapping,get<1>(step_result));
    }
    else if(plan_tree_node->joinType == NodeJoinType::JoinTwoTable)
    {

      left_r = this->ExecutionBreathFirst(basic_query,query_info,plan_tree_node->left_node,id_caches);
      auto left_table = get<2>(left_r);
      auto left_pos_id_mapping = get<1>(left_r);


      right_r = this->ExecutionBreathFirst(basic_query,query_info,plan_tree_node->right_node,id_caches);
      auto right_table = get<2>(right_r);
      auto right_pos_id_mapping = get<1>(right_r);

      set<TYPE_ENTITY_LITERAL_ID> public_var_set;
      set<TYPE_ENTITY_LITERAL_ID> right_table_var_id;

      for(const auto& right_pos_id_pair:*right_pos_id_mapping)
        right_table_var_id.insert(right_pos_id_pair.second);

      for(const auto& left_pos_id_pair:*left_pos_id_mapping){
        if(right_table_var_id.find(left_pos_id_pair.second)!=right_table_var_id.end()){
          public_var_set.insert(left_pos_id_pair.second);
        }
      }
//      cout << "public var:" << endl;
//      for(auto x : public_var_set)
//        cout << x << endl;

      auto one_step_join_table = make_shared<OneStepJoinTable>();
      for(const auto public_var:public_var_set)
        one_step_join_table->public_variables_->push_back(public_var);
//      cout<<"ExecutionBreathFirst 5"<<endl;

      auto left_id_pos_mapping = make_shared<PositionValue>();
      auto right_id_pos_mapping = make_shared<PositionValue>();

      for(const auto& pos_id_pair:*left_pos_id_mapping)
        (*left_id_pos_mapping)[pos_id_pair.second] = pos_id_pair.first;

      for(const auto& pos_id_pair:*right_pos_id_mapping)
        (*right_id_pos_mapping)[pos_id_pair.second] = pos_id_pair.first;


      return this->JoinTwoTable(one_step_join_table, left_table, left_id_pos_mapping,left_pos_id_mapping,
                                right_table, right_id_pos_mapping,right_pos_id_mapping);
    }
  }
}



#ifdef TOPK_SUPPORT

tuple<bool,PositionValueSharedPtr, TableContentShardPtr>  Optimizer::ExecutionTopK(BasicQuery* basic_query,
                                                                                   shared_ptr<TopKTreeSearchPlan> &tree_search_plan,
                                                                                   const QueryInfo& query_info,
                                                                                   IDCachesSharePtr id_caches) {

  auto pos_var_mapping = TopKUtil::CalculatePosVarMapping(tree_search_plan);
  auto k = query_info.limit_num_;
  auto first_item = (*query_info.ordered_by_expressions_)[0];
  auto var_coefficients = TopKUtil::getVarCoefficients(first_item);
  // Build Iterator tree
  auto env = new TopKUtil::Env();
  env->kv_store= this->kv_store_;
  env->basic_query = basic_query;
  env->id_caches = id_caches;
  cout<<" Optimizer::ExecutionTopK  env->id_caches "<<  env->id_caches->size()<<endl;
  env->k = query_info.limit_num_;
  env->coefficients= &var_coefficients;
  env->txn = this->txn_;
  env->ss = new stringstream();
  env->global_iterators = new std::vector<std::shared_ptr<std::set<OrderedList*>>>();

  auto root_fr = TopKUtil::BuildIteratorTree(tree_search_plan,env);
  for(int i =1;i<=query_info.limit_num_;i++)
  {
    root_fr->TryGetNext(k);
    if(root_fr->pool_.size()!=i)
      break;
  }

  auto result_list = make_shared<list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>();

#ifdef TOPK_DEBUG_INFO
  cout<<" the top score "<<root_fr->pool_[0].cost<<"  pool size "<<root_fr->pool_.size()<<endl;
  for(const auto& pos_id_pair:*pos_var_mapping)
    {
    cout<<"pos["<<pos_id_pair.first<<"]"<<" var id "<<pos_id_pair.second<<" "<<env->basic_query->getVarName(pos_id_pair.second)<<endl;
    }

#endif

  auto var_num = pos_var_mapping->size();
  for(int i =0;i<root_fr->pool_.size();i++)
  {
    auto record = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();
    record->reserve(var_num);
    root_fr->GetResult(i,record);
    result_list->emplace_back(std::move(record));
  }
#ifdef TOPK_DEBUG_INFO
  auto it = result_list->begin();
  for(int i =0;i<result_list->size();i++)
  {
    auto rec = *it;
    cout<<" record["<<i<<"]";
    for(int j =0;j<rec->size();j++)
      cout<<" "<<(*rec)[j];
    cout<<endl;
    it++;
  }
#endif
  TopKUtil::FreeGlobalIterators(env->global_iterators);
  return std::make_tuple(true,pos_var_mapping, result_list);
}
#endif