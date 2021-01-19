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

/**
 * a default query plan based on node degree
 * greedy choose the maximun : degree * |edges with chosen nodes|
 */
QueryPlan::QueryPlan(BasicQuery *basic_query,KVstore *kv_store) {
  /* total_var_num: the number of
   * 1 .selected vars
   * 2. not selected  but degree>1  vars
   * no predicate counted*/
  auto total_var_num = basic_query->getTotalVarNum();
  auto graph_var_num = basic_query->getVarNum();
  auto pre_var_num = basic_query->getPreVarNum();
  this->join_order_ = make_shared<vector<OneStepJoin>>();
  this->ids_after_join_ = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();

  TYPE_ENTITY_LITERAL_ID max_i = -1;
  TYPE_ENTITY_LITERAL_ID max_degree = -1;
  vector<bool> vars_used_vec(total_var_num+pre_var_num,false);
  vector<int> vars_degree_vec(total_var_num,0);


  for(int i = 1;i<total_var_num; i++)
    vars_degree_vec[i] = basic_query->getVarDegree(i);

  // select the first node to be the max degree
  for(int i = 1;i<total_var_num; i++)
  {
    if(basic_query->if_need_retrieve(i))
      continue;
    if (vars_degree_vec[i] > max_degree) {
      max_i = 1;
      max_degree = vars_degree_vec[i];
    }
  }
  // for the first node, always using constant to generate candidate
  {
    auto generate_edge_info = make_shared<vector<EdgeInfo>>();
    auto generate_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
    for(int i_th_edge=0;i_th_edge<vars_degree_vec[max_i];i_th_edge++)
    {
      auto edge_id =  basic_query->getEdgeID(max_i,i_th_edge);
      auto nei_id = basic_query->getEdgeNeighborID(max_i,i_th_edge);
      auto predicate_id = basic_query->getEdgePreID(max_i,i_th_edge);
      auto triple_string_type = basic_query->getTriple(edge_id);

      JoinMethod join_method;
      auto j_name = basic_query->getVarName(max_i);

      bool s_constant = triple_string_type.getSubject().at(0)!='?';
      bool p_constant = triple_string_type.getPredicate().at(0)!='?';
      bool o_constant = triple_string_type.getObject().at(0)!='?';
      // max_j is subject
      if(j_name==triple_string_type.getSubject())
      {
        if(p_constant)
        {
          if(o_constant)
            join_method = JoinMethod::po2s;
          else
            join_method = JoinMethod::p2s;
        }
        else if(o_constant)
          join_method = JoinMethod::o2s;
        else
          continue;
        generate_edge_info->emplace_back(max_i,predicate_id,nei_id,join_method);
      }
      else{ // max_j is object
        if(p_constant) {
          if(s_constant)
            join_method = JoinMethod::sp2o;
          else
            join_method = JoinMethod::p2o;
        }
        else if(s_constant)
          join_method = JoinMethod::s2o;
        else
          continue;
        generate_edge_info->emplace_back(nei_id,predicate_id,max_i,join_method);
      }
      generate_edge_constant_info->emplace_back(s_constant,p_constant,o_constant);
    }
    OneStepJoin one_step_join;
    one_step_join.join_type_ = OneStepJoin::JoinType::GenFilter;
    auto one_step_join_node = make_shared<OneStepJoinNode>();
    one_step_join_node->node_to_join_ = max_i;
    one_step_join_node->edges_ = generate_edge_info;
    one_step_join_node->edges_constant_info_ = generate_edge_constant_info;
    one_step_join.edge_filter_ = one_step_join_node;
    this->join_order_->push_back(one_step_join);

    // add the max to be the first
    vars_used_vec[max_i] = true;
    this->ids_after_join_->push_back(max_i);
  }




  for(TYPE_ENTITY_LITERAL_ID _ = 1; _ <graph_var_num; _++)
  {
    TYPE_ENTITY_LITERAL_ID max_j = -1;
    TYPE_ENTITY_LITERAL_ID max_score = -1;
    for(int j = 1;j<total_var_num; j++)
    {
      if(basic_query->if_need_retrieve(j))
        continue;
      if(vars_used_vec[j])
        continue;
      TYPE_ENTITY_LITERAL_ID edges_with_selected_vars_count = 0;
      for(auto selected_var:*this->ids_after_join_)
        edges_with_selected_vars_count += basic_query->getEdgeIndex(selected_var,j).size();
      auto j_score = edges_with_selected_vars_count * vars_degree_vec[j];
      if(j_score>max_score)
      {
        max_j = j;
        max_score = j_score;
      }
    }

    auto join_edge_info = make_shared<vector<EdgeInfo>>();
    auto join_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();

    // Get Edge Info with previous nodes to Generate Query Plan
    for(auto selected_var:*this->ids_after_join_)
    {
      auto i_th_edge_orders = basic_query->getEdgeIndex(selected_var, max_j);
      for(auto i_th_edge:i_th_edge_orders)
      {
        auto edge_id =  basic_query->getEdgeID(max_j,i_th_edge);
        auto nei_id = basic_query->getEdgeNeighborID(max_j,i_th_edge);
        auto predicate_id = basic_query->getEdgePreID(max_j,i_th_edge);
        auto triple_string_type = basic_query->getTriple(edge_id);

        JoinMethod join_method;
        auto j_name = basic_query->getVarName(max_j);

        bool s_constant = triple_string_type.getSubject().at(0)!='?';
        bool p_constant = triple_string_type.getPredicate().at(0)!='?';
        bool o_constant = triple_string_type.getObject().at(0)!='?';
        // max_j is subject
        if(j_name==triple_string_type.getSubject())
        {
          if(p_constant)
            join_method = JoinMethod::po2s;
          else
            join_method = JoinMethod::o2s;
          join_edge_info->emplace_back(max_j,predicate_id,nei_id,join_method);
        }
        else{ // max_j is object
          if(s_constant)
            join_method = JoinMethod::sp2o;
          else
            join_method = JoinMethod::s2o;
          join_edge_info->emplace_back(nei_id,predicate_id,max_j,join_method);
        }
        join_edge_constant_info->emplace_back(s_constant,p_constant,o_constant);
      }
    }

    OneStepJoin one_step_join;
    one_step_join.join_type_ = OneStepJoin::JoinType::JoinNode;
    auto one_step_join_node = make_shared<OneStepJoinNode>();
    one_step_join_node->node_to_join_ = max_j;
    one_step_join_node->edges_ = join_edge_info;
    one_step_join_node->edges_constant_info_ = join_edge_constant_info;
    one_step_join.edge_filter_ = one_step_join_node;
    this->join_order_->push_back(one_step_join);

    // add the max to selected vars
    vars_used_vec[max_j] = true;
    this->ids_after_join_->push_back(max_j);
  }

  vector<TYPE_ENTITY_LITERAL_ID> leave_behind_satellite_vec;
  // now we deal with nodes which 1. no selected 2. only one degree
  // also called satellite var
  for(TYPE_ENTITY_LITERAL_ID satellite_id = 0; satellite_id <graph_var_num;satellite_id++)
  {
    if(!basic_query->isSatelliteInJoin(satellite_id))
      continue;
    // because it has and only has one edge , and its neighbour

    auto check_edge_info = make_shared<vector<EdgeInfo>>();
    auto check_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();

    auto edge_id = basic_query->getEdgeID(satellite_id,0);
    auto nei_id = basic_query->getEdgeNeighborID(satellite_id,0);
    auto predicate_id = basic_query->getEdgePreID(satellite_id,0);
    auto triple_string_type = basic_query->getTriple(edge_id);

    JoinMethod join_method;
    auto satellite_name = basic_query->getVarName(satellite_id);

    bool s_constant = triple_string_type.getSubject().at(0)!='?';
    bool p_constant = triple_string_type.getPredicate().at(0)!='?';
    bool o_constant = triple_string_type.getObject().at(0)!='?';
    // satellite_id is subject
    if(satellite_name==triple_string_type.getSubject())
    {
      if(o_constant) // then satellite_id link to other part of BGP through predicate in triple_string_type
      {
        // degree(predicate) > 1
        // if so, we postpone it after pre vars and all filled
        leave_behind_satellite_vec.push_back(satellite_id);
        continue;
      }
      else if(p_constant) // ?o already in table,if not, the BGP is not Connective
        join_method = JoinMethod::po2s;
      else
      {
        // ?s ?p ?o and ?p links to the other part
        if(basic_query->isSatelliteInJoin(nei_id)) {
          continue;
        }
        else // ?s ?p ?o and ?o links to the other part
          join_method = JoinMethod::o2s;
      }
      check_edge_info->emplace_back(satellite_id,predicate_id,nei_id,join_method);
    }
    else{ // satellite_id is object
      if(s_constant) {
        // same reason as conditions ahead
        leave_behind_satellite_vec.push_back(satellite_id);
        continue;
      }// ?s already in table
      else if(p_constant)
        join_method = JoinMethod::sp2o;
      else // ?s ?p ?o and ?p links to the other part
      {
        // ?s ?p ?o and ?p links to the other part
        // no need to check this edge
        if(basic_query->isSatelliteInJoin(nei_id)) {
          continue;
        }
        else // ?s ?p ?o and ?s links to the other part
          join_method = JoinMethod::s2o;
      }
      check_edge_info->emplace_back(nei_id,predicate_id,satellite_id,join_method);
    }
    check_edge_constant_info->emplace_back(s_constant,p_constant,o_constant);

    OneStepJoin one_step_join;
    one_step_join.join_type_ = OneStepJoin::JoinType::EdgeCheck;
    auto one_step_join_node = make_shared<OneStepJoinNode>();
    one_step_join_node->node_to_join_ = satellite_id;
    one_step_join_node->edges_ = check_edge_info;
    one_step_join_node->edges_constant_info_ = check_edge_constant_info;
    one_step_join.edge_filter_ = one_step_join_node;
    this->join_order_->push_back(one_step_join);

    // add the max to selected vars
    vars_used_vec[satellite_id] = true;
    this->ids_after_join_->push_back(satellite_id);
  }

  auto pre_var_id_mappings = make_shared<map<string ,TYPE_ENTITY_LITERAL_ID>>();

  // deal with predicate
  // only deals with [ degree > 1 or selected ] predicates
  for(TYPE_ENTITY_LITERAL_ID predicate_id = 0; predicate_id <pre_var_num;predicate_id++)
  {
    auto pre_var = basic_query->getPreVarByID(predicate_id);
    if((!pre_var.selected) && pre_var.triples.size() <= 1)
      continue;
    TYPE_ENTITY_LITERAL_ID pre_id_Table;
    TYPE_ENTITY_LITERAL_ID s_id,o_id;
    auto predicate_edge_info = make_shared<vector<EdgeInfo>>();
    auto predicate_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
    for(auto edge_id:pre_var.triples)
    {
      auto triple_string_type = basic_query->getTriple(edge_id);

      JoinMethod join_method;

      bool s_constant = triple_string_type.getSubject().at(0)!='?';
      bool o_constant = triple_string_type.getObject().at(0)!='?';
      if(s_constant&&o_constant)
      {
        join_method = JoinMethod::so2p;
        s_id = kv_store->getIDByEntity(triple_string_type.getSubject());
        o_id = kv_store->getIDByString(triple_string_type.getObject());
      }
      else if( (!s_constant) &&o_constant)
      {
        auto s_var_id = basic_query->getIDByVarName(triple_string_type.getSubject());
        bool s_left = false;
        for(auto left_id:leave_behind_satellite_vec)
          if(left_id==s_var_id)
            s_left = true;
        if(s_left)
          join_method = JoinMethod::o2p;
        else
          join_method = JoinMethod::so2p;
        s_id = basic_query->getIDByVarName(triple_string_type.getSubject());
        o_id = kv_store->getIDByString(triple_string_type.getObject());
      }
      else if(s_constant && (!o_constant))
      {
        auto o_var_id = basic_query->getIDByVarName(triple_string_type.getObject());
        bool o_left = false;
        for(auto left_id:leave_behind_satellite_vec)
          if(left_id==o_var_id)
           o_left = true;
        if(o_left)
          join_method = JoinMethod::s2p;
        else
          join_method = JoinMethod::so2p;
        s_id = kv_store->getIDByEntity(triple_string_type.getSubject());
        o_id = basic_query->getIDByVarName(triple_string_type.getObject());
      }
      else // ?s ?p ?o
      {
        s_id = basic_query->getIDByVarName(triple_string_type.getSubject());
        bool s_left = false;
        o_id = basic_query->getIDByVarName(triple_string_type.getObject());
        bool o_left = false;

        // ?s ?p ?o and ?p links to other part
        if( (!basic_query->if_need_retrieve(s_id)) && (!basic_query->if_need_retrieve(o_id)))
          continue;
        for(auto left_id:leave_behind_satellite_vec) {
          if (left_id == s_id)
            s_left = true;
          if(left_id == o_id)
            o_left = true;
        }
      }

      if(pre_var_id_mappings->find(pre_var.name)!=pre_var_id_mappings->end())
        pre_id_Table = pre_var_id_mappings->operator[](pre_var.name);
      else
      {
        pre_id_Table = graph_var_num + 1 + pre_var_id_mappings->size();
        (*pre_var_id_mappings)[pre_var.name] = pre_id_Table;
      }
      predicate_edge_info->emplace_back(s_id,pre_id_Table,o_id,join_method);
      predicate_edge_constant_info->emplace_back(s_constant,false,o_constant);
    }
    OneStepJoin one_step_join;
    one_step_join.join_type_ = OneStepJoin::JoinType::JoinNode;
    auto one_step_join_node = make_shared<OneStepJoinNode>();
    one_step_join_node->node_to_join_ = pre_id_Table;
    one_step_join_node->edges_ = predicate_edge_info;
    one_step_join_node->edges_constant_info_ = predicate_edge_constant_info;
    one_step_join.join_node_ = one_step_join_node;
    this->join_order_->push_back(one_step_join);
    this->ids_after_join_->push_back(pre_id_Table);
  }

  // we do not forget the left behind
  // leave_behind_satellite must have a variable var
  for(auto left_id:leave_behind_satellite_vec)
  {

    // because it has and only has one edge , and its neighbour

    auto check_edge_info = make_shared<vector<EdgeInfo>>();
    auto check_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();

    auto left_name = basic_query->getVarName(left_id);
    auto edge_id = basic_query->getEdgeID(left_id,0);
    auto triple_string_type = basic_query->getTriple(edge_id);

    JoinMethod join_method;

    bool s_constant = triple_string_type.getSubject().at(0)!='?';
    bool o_constant = triple_string_type.getObject().at(0)!='?';

    TYPE_ENTITY_LITERAL_ID s_id,o_id;
    auto pre_id_table = (*pre_var_id_mappings)[triple_string_type.getPredicate()];
    // no need to check
    if( (!s_constant) && (!o_constant))
      continue;
    // satellite_id is subject
    if(left_name==triple_string_type.getSubject())
    {
      // o must be constant
      join_method = JoinMethod::po2s;
      s_id = basic_query->getIDByVarName(left_name);
      o_id = kv_store->getIDByString(triple_string_type.getObject());

    }
    else{ // satellite_id is object
      join_method = JoinMethod::sp2o;
      o_id = basic_query->getIDByVarName(left_name);
      s_id = kv_store->getIDByString(triple_string_type.getSubject());
    }
    check_edge_info->emplace_back(s_id,pre_id_table,o_id,join_method);
    check_edge_constant_info->emplace_back(s_constant, false,o_constant);

    OneStepJoin one_step_join;
    one_step_join.join_type_ = OneStepJoin::JoinType::EdgeCheck;
    auto one_step_join_node = make_shared<OneStepJoinNode>();
    one_step_join_node->node_to_join_ = left_id;
    one_step_join_node->edges_ = check_edge_info;
    one_step_join_node->edges_constant_info_ = check_edge_constant_info;
    one_step_join.edge_filter_ = one_step_join_node;
    this->join_order_->push_back(one_step_join);

    // add the max to selected vars
    vars_used_vec[left_id] = true;
    this->ids_after_join_->push_back(left_id);
  }

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

/**
 * generate a var candidate based on its constant edge/neighbour
 * @param edge_info_vector
 * @param edge_table_info_vector
 * @return
 */
tuple<bool, shared_ptr<IntermediateResult>> Optimizer::GenerateColdCandidateList(vector<shared_ptr<EdgeInfo>> edge_info_vector,
                                                                                 vector<shared_ptr<EdgeConstantInfo>> edge_table_info_vector) {

  if(edge_info_vector.empty())
    return make_tuple(false, nullptr);


  auto var_to_filter = (edge_info_vector[0])->getVarToFilter();
  auto id_candidate = make_shared<IDList>();

  assert(edge_info_vector.size()==edge_table_info_vector.size());

  for(int i = 0;i<edge_info_vector.size();i++)
  {
    auto edge_info = edge_info_vector[i];
    TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
    TYPE_ENTITY_LITERAL_ID this_edge_list_len;
    switch (edge_info->join_method_) {
      case JoinMethod::s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
        auto s_var_constant_id = edge_info->s_;
        this->kv_store_->getpreIDlistBysubID(s_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::s2o: {
        auto s_var_constant_id = edge_info->s_;
        this->kv_store_->getobjIDlistBysubID(s_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::p2s: {
        auto p_var_constant_id = edge_info->p_;
        this->kv_store_->getsubIDlistBypreID(p_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::p2o: {
        auto p_var_constant_id = edge_info->p_;
        this->kv_store_->getobjIDlistBypreID(p_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::o2s: {
        auto o_var_constant_id = edge_info->o_;
        this->kv_store_->getsubIDlistByobjID(o_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::o2p: {
        auto o_var_constant_id = edge_info->o_;
        this->kv_store_->getpreIDlistByobjID(o_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::so2p: {
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
      case JoinMethod::sp2o: {
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
      case JoinMethod::po2s: {
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

    UpdateIDList(id_candidate,edge_candidate_list,this_edge_list_len,i > 0);
  }

  auto result = make_shared<IntermediateResult>();
  (*(result->id_to_position_))[var_to_filter] = 0;
  (*(result->position_to_id_))[0] = var_to_filter;
  auto id_candidate_vec = id_candidate->getList();
  auto result_content = result->values_;
  for(auto var_id: *id_candidate_vec)
  {
    auto record = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();
    record->push_back(var_id);
    result_content->push_back(record);
  }
  return make_tuple(true,result);
}




/**
 * add a node to the table
 * @param one_step_join_node_ Describe how to join a node
 * @param intermediate_result The Result in previous step
 * @return bool: the function is done; IntermediateResult: the new IntermediateResult
 */
tuple<bool,shared_ptr<IntermediateResult>> Optimizer::JoinANode(shared_ptr<OneStepJoinNode> one_step_join_node_, shared_ptr<IntermediateResult> intermediate_result) {


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
          auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info.s_];
          auto s_var_id_this_record = (**record_iterator)[s_var_position];
          this->kv_store_->getpreIDlistBysubID(s_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case JoinMethod::s2o: {
          auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info.s_];
          auto s_var_id_this_record = (**record_iterator)[s_var_position];
          this->kv_store_->getobjIDlistBysubID(s_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case JoinMethod::p2s: {
          auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info.p_];
          auto p_var_id_this_record = (**record_iterator)[p_var_position];
          this->kv_store_->getsubIDlistBypreID(p_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case JoinMethod::p2o: {
          auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info.p_];
          auto p_var_id_this_record = (**record_iterator)[p_var_position];
          this->kv_store_->getobjIDlistBypreID(p_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case JoinMethod::o2s: {
          auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info.o_];
          auto o_var_id_this_record = (**record_iterator)[o_var_position];
          this->kv_store_->getsubIDlistByobjID(o_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case JoinMethod::o2p: {
          auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info.o_];
          auto o_var_id_this_record = (**record_iterator)[o_var_position];
          this->kv_store_->getpreIDlistByobjID(o_var_id_this_record,
                                               edge_candidate_list,
                                               edge_list_len,
                                               true,
                                               this->txn_);
          break;
        }
        case JoinMethod::so2p: {
          TYPE_ENTITY_LITERAL_ID s_var_position,s_var_id_this_record;
          if(edge_constant_info.s_constant_)
            s_var_id_this_record = edge_info.s_;
          else {
            auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info.s_];
            s_var_id_this_record = (**record_iterator)[s_var_position];
          }

          TYPE_ENTITY_LITERAL_ID o_var_position,o_var_id_this_record;
          if(edge_constant_info.o_constant_)
            o_var_id_this_record = edge_info.o_;
          else {
            auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info.o_];
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
          TYPE_ENTITY_LITERAL_ID s_var_position,s_var_id_this_record;
          if(edge_constant_info.s_constant_)
            s_var_id_this_record = edge_info.s_;
          else {
            auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info.s_];
            s_var_id_this_record = (**record_iterator)[s_var_position];
          }

          TYPE_ENTITY_LITERAL_ID p_var_position,p_var_id_this_record;
          if(edge_constant_info.p_constant_)
            p_var_id_this_record = edge_info.p_;
          else {
            auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info.p_];
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
          TYPE_ENTITY_LITERAL_ID o_var_position,o_var_id_this_record;
          if(edge_constant_info.o_constant_)
            o_var_id_this_record = edge_info.o_;
          else {
            auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info.o_];
            o_var_id_this_record = (**record_iterator)[o_var_position];
          }

          TYPE_ENTITY_LITERAL_ID p_var_position,p_var_id_this_record;
          if(edge_constant_info.p_constant_)
            p_var_id_this_record = edge_info.p_;
          else {
            auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info.p_];
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


  return make_tuple(true,new_intermediate_table);

}


/**
 * join two table with vars in one_step_join_table
 * @param one_step_join_table: define join vars
 * @param table_a
 * @param table_b
 * @return
 */
tuple<bool, shared_ptr<IntermediateResult>> Optimizer::JoinTwoTable(shared_ptr<OneStepJoinTable> one_step_join_table,
                                                                    shared_ptr<IntermediateResult> table_a,
                                                                    shared_ptr<IntermediateResult> table_b) {
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

  return make_tuple(true,result_table);

}

/**
 * for a table T[a,b,c] if we want to check if ?a always have a edge (?a,p,o)
 * If T is nullptr,
 * @param edge_info
 * @param edge_table_info
 * @param intermediate_result
 * @return
 */
tuple<bool, shared_ptr<IntermediateResult>> Optimizer::EdgeConstraintFilter(shared_ptr<EdgeInfo> edge_info,
                                                                            EdgeConstantInfo edge_table_info,
                                                                            shared_ptr<IntermediateResult> intermediate_result) {
  TYPE_ENTITY_LITERAL_ID var_to_filter= edge_info->getVarToFilter();

  if(edge_table_info.ConstantToVar(edge_info))
  {

    TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
    TYPE_ENTITY_LITERAL_ID this_edge_list_len;
    switch (edge_info->join_method_) {
      case JoinMethod::s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
        auto s_var_constant_id = edge_info->s_;
        this->kv_store_->getpreIDlistBysubID(s_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::s2o: {
        auto s_var_constant_id = edge_info->s_;
        this->kv_store_->getobjIDlistBysubID(s_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::p2s: {
        auto p_var_constant_id = edge_info->p_;
        this->kv_store_->getsubIDlistBypreID(p_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::p2o: {
        auto p_var_constant_id = edge_info->p_;
        this->kv_store_->getobjIDlistBypreID(p_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::o2s: {
        auto o_var_constant_id = edge_info->o_;
        this->kv_store_->getsubIDlistByobjID(o_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::o2p: {
        auto o_var_constant_id = edge_info->o_;
        this->kv_store_->getpreIDlistByobjID(o_var_constant_id,
                                             edge_candidate_list,
                                             this_edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::so2p: {
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
      case JoinMethod::sp2o: {
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
      case JoinMethod::po2s: {
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
    return FilterAVariableOnIDList(validate_list,var_to_filter,intermediate_result);
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
      case JoinMethod::s2p: { // Because if we don't add a pair of '{}', the editor will report a error of redefinition
        auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info->s_];
        auto s_var_id_this_record = (**record_iterator)[s_var_position];
        this->kv_store_->getpreIDlistBysubID(s_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);

        break;
      }
      case JoinMethod::s2o: {
        auto s_var_position = (*(intermediate_result->id_to_position_))[edge_info->s_];
        auto s_var_id_this_record = (**record_iterator)[s_var_position];
        this->kv_store_->getobjIDlistBysubID(s_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::p2s: {
        auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info->p_];
        auto p_var_id_this_record = (**record_iterator)[p_var_position];
        this->kv_store_->getsubIDlistBypreID(p_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::p2o: {
        auto p_var_position = (*(intermediate_result->id_to_position_))[edge_info->p_];
        auto p_var_id_this_record = (**record_iterator)[p_var_position];
        this->kv_store_->getobjIDlistBypreID(p_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::o2s: {
        auto o_var_position = (*(intermediate_result->id_to_position_))[edge_info->o_];
        auto o_var_id_this_record = (**record_iterator)[o_var_position];
        this->kv_store_->getsubIDlistByobjID(o_var_id_this_record,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             this->txn_);
        break;
      }
      case JoinMethod::o2p: {
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
      case JoinMethod::so2p: {
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
      case JoinMethod::sp2o: {
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
      case JoinMethod::po2s: {
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

  return make_tuple(true,new_intermediate_table);

}

tuple<bool, shared_ptr<IntermediateResult>> Optimizer::FilterAVariableOnIDList(shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> candidate_list,
                                                                               TYPE_ENTITY_LITERAL_ID var_id,
                                                                               shared_ptr<IntermediateResult> intermediate_result) {

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

  return make_tuple(true,new_intermediate_table);

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

BasicQueryStrategy Optimizer::ChooseStrategy(BasicQuery *basic_query) {
  return BasicQueryStrategy::Normal;
}

shared_ptr<IntermediateResult> Optimizer::NormalJoin(shared_ptr<BasicQuery>, shared_ptr<QueryPlan>) {
  return shared_ptr<IntermediateResult>();
}


bool Optimizer::do_query(SPARQLquery &sparql_query) {
  for(auto basic_query_pointer:sparql_query.getBasicQueryVec())
  {
    switch (this->ChooseStrategy(basic_query_pointer)) {
      case BasicQueryStrategy::Normal:



        break;


      case BasicQueryStrategy::Special:
        break;
    };
  }
  return true;
}



