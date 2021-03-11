/*=============================================================================
# Filename: QueryPlan.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "QueryPlan.h"

QueryPlan::QueryPlan(shared_ptr<vector<OneStepJoin>> join_order, shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> ids_after_join,shared_ptr<vector<VarDescriptor>> var_infos) {
  // Copy Construction function of Vector
  // To avoid join_order_ and ids_after_join_ be adjusted by outer env
  this->join_order_ = make_shared<vector<OneStepJoin>>(*join_order);
  this->ids_after_join_ = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>(*ids_after_join);
  this->var_descriptors_=var_infos;
}

/**
 * a default query plan based on node degree
 *
 * |core var|
 *      greedy choose the maximum : degree * |edges with chosen nodes|
 * |pre var|
 * |satellites check|
 */
QueryPlan::QueryPlan(BasicQuery *basic_query,KVstore *kv_store,shared_ptr<vector<VarDescriptor>> var_infos) {
  QueryPlan* q = this;
  /* total_var_num: the number of
   * 1 .selected vars
   * 2. not selected  but degree > 1  vars
   * no predicate counted
  */
  auto total_var_num = basic_query->getTotalVarNum();
  vector<bool> vars_used_vec(total_var_num,false);
  // When id < total_var_num, the var in 'var_infos' maps exactly the id in BasicQuery
  for(int i = 0;i<total_var_num; i++) {
   var_infos->emplace_back(VarDescriptor::VarType::EntityType ,basic_query->getVarName(i),basic_query);
  }

  auto graph_var_num = basic_query->getVarNum();
  auto pre_var_num = basic_query->getPreVarNum();
  auto selected_var_num = basic_query->getSelectVarNum();

  this->join_order_ = make_shared<vector<OneStepJoin>>();
  this->ids_after_join_ = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();

  this->var_descriptors_ = var_infos;

  int max_i_basic_queryID = -1;
  int max_degree = -1;

  // select the first node to be the max degree
  for(int i = 0;i<this->var_descriptors_->size(); i++)
  {
    // need_retrieve = degree > 1
    // so this line check so-called satellites nodes
    if((*var_descriptors_)[i].IsSatellite())
      continue;
    if ( (*var_descriptors_)[i].degree_ > max_degree) {
      max_i_basic_queryID = i;
      max_degree = (*var_descriptors_)[i].degree_;
    }
  }

  // for the first node, always using constant to generate candidate
  {
    auto generate_edge_info = make_shared<vector<EdgeInfo>>();
    auto generate_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
    for(int i_th_edge=0; i_th_edge<(*var_descriptors_)[max_i_basic_queryID].degree_; i_th_edge++)
    {
      auto edge_id =  basic_query->getEdgeID(max_i_basic_queryID, i_th_edge);
      auto nei_id = basic_query->getEdgeNeighborID(max_i_basic_queryID, i_th_edge);
      auto predicate_id = basic_query->getEdgePreID(max_i_basic_queryID, i_th_edge);
      auto triple_string_type = basic_query->getTriple(edge_id);

      JoinMethod join_method;
      auto j_name = basic_query->getVarName(max_i_basic_queryID);

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
        if(nei_id == -1)
          nei_id = kv_store->getIDByString(triple_string_type.getObject());
        generate_edge_info->emplace_back(max_i_basic_queryID, predicate_id, nei_id, join_method);
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
        if(nei_id == -1)
          nei_id = kv_store->getIDByString(triple_string_type.getSubject());
        generate_edge_info->emplace_back(nei_id, predicate_id, max_i_basic_queryID, join_method);
      }
      generate_edge_constant_info->emplace_back(s_constant,p_constant,o_constant);
    }
    OneStepJoin one_step_join;
    one_step_join.join_type_ = OneStepJoin::JoinType::GenFilter;
    auto one_step_join_node = make_shared<OneStepJoinNode>();
    one_step_join_node->node_to_join_ = max_i_basic_queryID;
    one_step_join_node->edges_ = generate_edge_info;
    one_step_join_node->edges_constant_info_ = generate_edge_constant_info;
    one_step_join.edge_filter_ = one_step_join_node;
    this->join_order_->push_back(one_step_join);

    // add the max to be the first
    vars_used_vec[max_i_basic_queryID] = true;
    this->ids_after_join_->push_back(max_i_basic_queryID);
  }

  // Loop for this->var_descriptors_->size()-1 times
  for(int _ = 1; _ < this->var_descriptors_->size(); _++) {
    TYPE_ENTITY_LITERAL_ID max_j_basicqueryID = -1;
    int max_score = -1;
    for (int j = 0; j < total_var_num; j++) {
      printf("j=%d\n", j);
      // satellites
      if ((*var_descriptors_)[j].IsSatellite())
        continue;
      if (vars_used_vec[j])
        continue;
      TYPE_ENTITY_LITERAL_ID edges_with_selected_vars_count = 0;
      for (auto selected_var:*this->ids_after_join_)
        edges_with_selected_vars_count += basic_query->getEdgeIndex(selected_var, j).size();
      int j_score = edges_with_selected_vars_count * (*var_descriptors_)[j].degree_;
      printf("j_score = edges_with_selected_vars_count * vars_degree_vec[j] \n");
      printf("j:%d j_score:%d edges_with_selected_vars_count:%d vars_degree_vec_j:%d\n",
             j,
             j_score,
             edges_with_selected_vars_count,
             (*var_descriptors_)[j].degree_);
      cout << endl;
      if (j_score > max_score) {
        max_j_basicqueryID = j;
        max_score = j_score;
      }
    }

    auto join_edge_info = make_shared<vector<EdgeInfo>>();
    auto join_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();

    cout << " choose [" << max_j_basicqueryID << "] as next node" << endl;

    // add the max to selected vars
    vars_used_vec[max_j_basicqueryID] = true;
    this->ids_after_join_->push_back(max_j_basicqueryID);

    // Get Edge Info with previous nodes to Generate Query Plan
    // If the edge has constant predicate and a variable neighbour , then add a edge check
    {
      for (auto selected_var:*this->ids_after_join_) {
        auto i_th_edge_orders = basic_query->getEdgeIndex(max_j_basicqueryID, selected_var);
        for (auto i_th_edge:i_th_edge_orders) {
          auto edge_id = basic_query->getEdgeID(max_j_basicqueryID, i_th_edge);
          auto nei_id = basic_query->getEdgeNeighborID(max_j_basicqueryID, i_th_edge);
          auto predicate_id = basic_query->getEdgePreID(max_j_basicqueryID, i_th_edge);
          auto triple_string_type = basic_query->getTriple(edge_id);
          cout << "\tedge[" << edge_id << "] \t neighbour_id=" << nei_id << " \t predicate_id=" << predicate_id << endl;
          cout << "\t %s:" << triple_string_type.toString() << endl;
          JoinMethod join_method;
          auto j_name = basic_query->getVarName(max_j_basicqueryID);

          bool s_constant = triple_string_type.getSubject().at(0) != '?';
          bool p_constant = triple_string_type.getPredicate().at(0) != '?';
          bool o_constant = triple_string_type.getObject().at(0) != '?';
          // max_j_basicqueryID is subject
          if (j_name == triple_string_type.getSubject()) {
            if (p_constant)
              join_method = JoinMethod::po2s;
            else
              join_method = JoinMethod::o2s;
            join_edge_info->emplace_back(max_j_basicqueryID, predicate_id, nei_id, join_method);
          } else { // max_j_basicqueryID is object
            if (s_constant)
              join_method = JoinMethod::sp2o;
            else
              join_method = JoinMethod::s2o;
            join_edge_info->emplace_back(nei_id, predicate_id, max_j_basicqueryID, join_method);
          }

          join_edge_constant_info->emplace_back(s_constant, p_constant, o_constant);
        }
      }

      OneStepJoin one_step_join;
      one_step_join.join_type_ = OneStepJoin::JoinType::JoinNode;
      auto one_step_join_node = make_shared<OneStepJoinNode>();
      one_step_join_node->node_to_join_ = max_j_basicqueryID;
      one_step_join_node->edges_ = join_edge_info;
      one_step_join_node->edges_constant_info_ = join_edge_constant_info;
      one_step_join_node->ChangeOrder(this->ids_after_join_);
      one_step_join.join_node_ = one_step_join_node;
      this->join_order_->push_back(one_step_join);

    }

    // If the chosen one have a constant edge, that is , neighbour and predicate are both constant
    {
      auto check_edge_info = make_shared<vector<EdgeInfo>>();
      auto check_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();

      auto j_degree = basic_query->getVarDegree(max_j_basicqueryID);
      for (int i_th_edge = 0; i_th_edge < j_degree; i_th_edge++) {
        TYPE_ENTITY_LITERAL_ID sid,oid,pid;
        JoinMethod join_method;
        auto edge_id = basic_query->getEdgeID(max_j_basicqueryID, i_th_edge);
        auto nei_id = basic_query->getEdgeNeighborID(max_j_basicqueryID, i_th_edge);
        auto predicate_id = basic_query->getEdgePreID(max_j_basicqueryID, i_th_edge);
        auto triple_string_type = basic_query->getTriple(edge_id);

        bool s_constant = triple_string_type.getSubject().at(0)!='?';
        bool p_constant = triple_string_type.getPredicate().at(0)!='?';
        bool o_constant = triple_string_type.getObject().at(0)!='?';

        int const_number = 0;
        if (s_constant) const_number++;
        if (p_constant) const_number++;
        if (o_constant) const_number++;
        if (const_number != 2)
          continue;


        if (!s_constant) {
          sid = max_j_basicqueryID;
          pid = predicate_id;
          oid = kv_store->getIDByString(triple_string_type.getObject());
          join_method = JoinMethod::po2s;
        }
        if (!p_constant) {
          sid = kv_store->getIDByString(triple_string_type.getSubject());;
          pid = max_j_basicqueryID;
          oid = kv_store->getIDByString(triple_string_type.getObject());
          join_method = JoinMethod::so2p;
        }
        if (!o_constant) {
          sid = kv_store->getIDByString(triple_string_type.getSubject());
          pid = predicate_id;
          oid = max_j_basicqueryID;
          join_method = JoinMethod::po2s;
        }
        check_edge_info->emplace_back(sid,pid,oid,join_method);
        check_edge_constant_info->emplace_back(s_constant, p_constant,o_constant);
      }
      OneStepJoin one_step_join;
      one_step_join.join_type_ = OneStepJoin::JoinType::EdgeCheck;
      auto one_step_join_node = make_shared<OneStepJoinNode>();
      one_step_join_node->node_to_join_ = max_j_basicqueryID;
      one_step_join_node->edges_ = check_edge_info;
      one_step_join_node->edges_constant_info_ = check_edge_constant_info;
      one_step_join_node->ChangeOrder(this->ids_after_join_);
      one_step_join.edge_filter_ = one_step_join_node;
      this->join_order_->push_back(one_step_join);
    }
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
    one_step_join_node->ChangeOrder(this->ids_after_join_);
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
    one_step_join_node->ChangeOrder(this->ids_after_join_);
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
    one_step_join_node->ChangeOrder(this->ids_after_join_);
    one_step_join.edge_filter_ = one_step_join_node;
    this->join_order_->push_back(one_step_join);

    // add the max to selected vars
    vars_used_vec[left_id] = true;
    this->ids_after_join_->push_back(left_id);
  }
}