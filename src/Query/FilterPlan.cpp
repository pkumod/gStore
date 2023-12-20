//
// Created by Yuqi Zhou on 2022/11/9.
//

#include "FilterPlan.h"

/**
 * If the chosen one have a constant edge, that is ,
 *  neighbour and predicate are both constant
 * @param bgp_query
 * @param kv_store      KVStorePointer
 * @param target_node   the node needed to check constant edge
 * @return the filtering plan
 */
shared_ptr<AffectOneNode> FilterPlan::FilterNodeOnConstantEdge(shared_ptr<BGPQuery> bgp_query,
                                                               KVstore *kv_store,
                                                               TYPE_ENTITY_LITERAL_ID target_node) {
  auto check_edge_info = make_shared<vector<EdgeInfo>>();
  auto check_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();

  auto var_descriptor = bgp_query->get_vardescrip_by_id(target_node);
  auto j_degree = var_descriptor->degree_;
  auto& edge_ids = var_descriptor->so_edge_index_;
  for (unsigned int i_th_edge = 0; i_th_edge < j_degree; i_th_edge++) {
    TYPE_ENTITY_LITERAL_ID sid, oid, pid;
    JoinMethod join_method;
    auto edge_id = edge_ids[i_th_edge];
    auto triple_string_type = bgp_query->get_triple_by_index(edge_id);

    bool s_constant = triple_string_type.getSubject().at(0) != '?';
    bool p_constant = triple_string_type.getPredicate().at(0) != '?';
    bool o_constant = triple_string_type.getObject().at(0) != '?';

    int const_number = 0;
    if (s_constant) const_number++;
    if (p_constant) const_number++;
    if (o_constant) const_number++;


    if (const_number == 2) {
      if (!s_constant) {
        sid = target_node;
        pid = kv_store->getIDByPredicate(triple_string_type.getPredicate());
        oid = kv_store->getIDByString(triple_string_type.getObject());
        join_method = JoinMethod::po2s;
      }
      if (!p_constant) {
        sid = kv_store->getIDByString(triple_string_type.getSubject());
        pid = target_node;
        oid = kv_store->getIDByString(triple_string_type.getObject());
        join_method = JoinMethod::so2p;
      }
      if (!o_constant) {
        sid = kv_store->getIDByString(triple_string_type.getSubject());
        pid = kv_store->getIDByPredicate(triple_string_type.getPredicate());
        oid = target_node;
        join_method = JoinMethod::sp2o;
      }
      check_edge_info->emplace_back(sid, pid, oid, join_method);
      check_edge_constant_info->emplace_back(s_constant, p_constant, o_constant);
    }
    else if(const_number == 1 && (!p_constant))
    {
      if (!s_constant) {
        sid = target_node;
        oid = kv_store->getIDByString(triple_string_type.getObject());
        join_method = JoinMethod::o2s;
      }
      if (!o_constant) {
        sid = kv_store->getIDByString(triple_string_type.getSubject());
        oid = target_node;
        join_method = JoinMethod::s2o;
      }
      check_edge_info->emplace_back(sid, pid, oid, join_method);
      check_edge_constant_info->emplace_back(s_constant, p_constant, o_constant);
    }
  }

  auto one_step_join_node = make_shared<AffectOneNode>();
  one_step_join_node->node_to_join_ = target_node;
  one_step_join_node->edges_ = check_edge_info;
  one_step_join_node->edges_constant_info_ = check_edge_constant_info;
  return one_step_join_node;
}

/**
 * If the chosen one have a constant predicate, that is ,
 * 1. predicate is constant
 * 2. no matter its neighbour is constant (in this case, it will generate a plan to filter target_node
 * with this constant edge) or variable
 * @param bgp_query
 * @param kv_store      KVStorePointer
 * @param target_node   the node needed to check constant edge
 * @return the filtering plan
 */
shared_ptr<AffectOneNode> FilterPlan::FilterNodeOnConstantPredicate(shared_ptr<BGPQuery> bgp_query,
                                                                    KVstore *kv_store,
                                                                    TYPE_ENTITY_LITERAL_ID target_node){
  auto check_edge_info = make_shared<vector<EdgeInfo>>();
  auto check_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();

  auto var_descriptor = bgp_query->get_vardescrip_by_id(target_node);
  auto& target_var_name = var_descriptor->var_name_;
  auto j_degree = var_descriptor->degree_;
  auto& edge_ids = var_descriptor->so_edge_index_;
  for (unsigned int i_th_edge = 0; i_th_edge < j_degree; i_th_edge++) {
    TYPE_ENTITY_LITERAL_ID sid, oid, pid;
    JoinMethod join_method;
    auto edge_id = edge_ids[i_th_edge];
    auto triple_string_type = bgp_query->get_triple_by_index(edge_id);

    bool s_constant = triple_string_type.getSubject().at(0) != '?';
    bool p_constant = triple_string_type.getPredicate().at(0) != '?';
    bool o_constant = triple_string_type.getObject().at(0) != '?';

    if(!p_constant)
      continue;

    int const_number = 0;
    if (s_constant) const_number++;
    if (p_constant) const_number++;
    if (o_constant) const_number++;


    if (const_number == 2) {
      if (!s_constant) { // po2s
        sid = target_node;
        pid = kv_store->getIDByPredicate(triple_string_type.getPredicate());
        oid = kv_store->getIDByString(triple_string_type.getObject());
        join_method = JoinMethod::po2s;
      }
      if (!o_constant) { // sp2o
        sid = kv_store->getIDByString(triple_string_type.getSubject());
        pid = kv_store->getIDByPredicate(triple_string_type.getPredicate());
        oid = target_node;
        join_method = JoinMethod::sp2o;
      }
      check_edge_info->emplace_back(sid, pid, oid, join_method);
      check_edge_constant_info->emplace_back(s_constant, p_constant, o_constant);
    }
    else if(const_number == 1 ) // only p is constant
    {
      pid = kv_store->getIDByPredicate(triple_string_type.getPredicate());
      if (triple_string_type.getSubject() == target_var_name) // subject is the target
      {
        sid = target_node;
        join_method = JoinMethod::p2s;
      }
      else { // object is the target
        oid = target_node;
        join_method = JoinMethod::p2o;
      }
      check_edge_info->emplace_back(sid, pid, oid, join_method);
      check_edge_constant_info->emplace_back(s_constant, p_constant, o_constant);
    }
  }

  auto one_step_join_node = make_shared<AffectOneNode>();
  one_step_join_node->node_to_join_ = target_node;
  one_step_join_node->edges_ = check_edge_info;
  one_step_join_node->edges_constant_info_ = check_edge_constant_info;
  return one_step_join_node;
}

std::shared_ptr<std::vector<std::shared_ptr<AffectOneNode>>>
FilterPlan::OnlyConstFilter(std::shared_ptr<BGPQuery> bgp_query,
                            KVstore *kv_store) {
  if (bgp_query->get_triple_num() == 1) return make_shared<vector<shared_ptr<AffectOneNode>>>();
  auto result = make_shared<FilterPlan>();
  auto constant_generating_lists = make_shared<vector<shared_ptr<AffectOneNode>>>();
  auto total_var_num = bgp_query->get_total_var_num();

// select the first node to be the max degree
  for(decltype(total_var_num) i = 0;i<total_var_num; i++)
  {
    auto var_id = bgp_query->get_vardescrip_by_index(i)->id_;
    if( (!bgp_query->is_var_selected_by_id(var_id)) && bgp_query->get_var_degree_by_id(var_id) == 1)
      continue;
    if(bgp_query->get_vardescrip_by_index(i)->var_type_==VarDescriptor::VarType::Predicate)
      continue;
    auto constant_filtering = FilterNodeOnConstantEdge(bgp_query, kv_store, var_id);
    if(!constant_filtering->edges_->empty())
      constant_generating_lists->push_back(constant_filtering);
  }
  return constant_generating_lists;
}


std::shared_ptr<std::vector<std::shared_ptr<AffectOneNode>>>
FilterPlan::PredicateFilter(std::shared_ptr<BGPQuery> bgp_query,
                            KVstore *kv_store) {
  auto result = make_shared<FilterPlan>();
  auto constant_predicate_generating_lists = make_shared<vector<shared_ptr<AffectOneNode>>>();
  auto total_var_num = bgp_query->get_total_var_num();

// select the first node to be the max degree
  for(decltype(total_var_num) i = 0;i<total_var_num; i++)
  {
    auto var_id = bgp_query->get_vardescrip_by_index(i)->id_;
    if(bgp_query->get_vardescrip_by_index(i)->var_type_==VarDescriptor::VarType::Predicate)
      continue;
    auto predicate_filtering = FilterNodeOnConstantPredicate(bgp_query, kv_store, var_id);
    if(!predicate_filtering->edges_->empty())
      constant_predicate_generating_lists->push_back(predicate_filtering);
  }
  return constant_predicate_generating_lists;
}
