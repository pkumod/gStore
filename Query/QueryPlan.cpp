/*=============================================================================
# Filename: QueryPlan.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "QueryPlan.h"


QueryPlan::QueryPlan(const shared_ptr<vector<StepOperation>>& join_order, const shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>& ids_after_join, shared_ptr<vector<OldVarDescriptor>> var_infos) {
  // Copy Construction function of Vector
  // To avoid join_order_ and ids_after_join_ be adjusted by outer env
  this->join_order_ = make_shared<vector<StepOperation>>(*join_order);
  this->ids_after_join_ = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>(*ids_after_join);
  this->var_descriptors_=std::move(var_infos);
}

/**
 * a default query plan based on node degree. Used in Depth first search
 *
 * |core var|
 *      greedy choose the maximum : degree * |edges with chosen nodes|
 * |pre var|
 * |satellites check|
 */
QueryPlan::QueryPlan(BasicQuery *basic_query,KVstore *kv_store,shared_ptr<vector<OldVarDescriptor>> var_infos) {
  /* total_var_num: the number of
   * 1 .selected vars
   * 2. not selected  but degree > 1  vars
   * no predicate counted
  */
  this->constant_generating_lists_ = make_shared<vector<shared_ptr<FeedOneNode>>>();
  auto total_var_num = basic_query->getTotalVarNum();
  vector<bool> vars_used_vec(total_var_num,false);
  // When id < total_var_num, the var in 'var_infos' maps exactly the id in BasicQuery
  for(int i = 0;i<total_var_num; i++) {
    var_infos->emplace_back(OldVarDescriptor::VarType::EntityType ,basic_query->getVarName(i),basic_query);
  }

  auto graph_var_num = basic_query->getVarNum();
  auto pre_var_num = basic_query->getPreVarNum();
  auto selected_var_num = basic_query->getSelectVarNum();

  this->join_order_ = make_shared<vector<StepOperation>>();
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

  auto one_step_join = FilterFirstNode(basic_query, kv_store, max_i_basic_queryID,var_infos);
  this->join_order_->push_back(one_step_join);

  // add the max to be the first
  vars_used_vec[max_i_basic_queryID] = true;
  this->ids_after_join_->push_back(max_i_basic_queryID);


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
      if (j_score > max_score) {
        max_j_basicqueryID = j;
        max_score = j_score;
      }
    }

    // add the max to selected vars
    vars_used_vec[max_j_basicqueryID] = true;
    this->ids_after_join_->push_back(max_j_basicqueryID);

    auto join_new_node_plan = LinkWithPreviousNodes(basic_query, kv_store, max_j_basicqueryID ,ids_after_join_);
    this->join_order_->push_back(join_new_node_plan);


    auto constant_filtering = FilterNodeOnConstantEdge(basic_query, kv_store, max_j_basicqueryID);
    constant_generating_lists_->push_back(constant_filtering);

  }

  ProcessPredicateAndSatellites(basic_query, kv_store, vars_used_vec, graph_var_num, pre_var_num,this->join_order_,this->ids_after_join_);

}

/*
 *  Node weight in the greedy method
 */
double QueryPlan::ScoreNode(BasicQuery *basic_query, int var){
  unsigned degree = basic_query->getVarDegree(var);
	unsigned size = basic_query->getCandidateSize(var);
	double wt = 1;
	for(unsigned i = 0; i < degree; i++) {
		int edge_id = basic_query->getEdgeID(var, i);
		int nei_id = basic_query->getEdgeNeighborID(var, i);
		if(nei_id < 0 || nei_id >= basic_query->getVarNum() || basic_query->isSatelliteInJoin(nei_id)) {
		  continue;
		}
		//CHECK:if the pre id is valid (0<=p<limit_predicateID)
		// TYPE_PREDICATE_ID pid = basic_query->getEdgePreID(var, i);
		// if(pid < 0 || pid >= this->limitID_predicate)
		// {
		// 	continue;
		// }
		//wt += 10000 / (double)(this->pre2num[pid]+1); // neighbours
	}
		wt += 1000000 / ((double)size+1);

	return wt;
}

/**
 *  Greedy method to choose the next node for processing through scores
 * @param basic_query   BasicQueryPointer
 * @param processed_nodes  NodeIDs which are processed by join
 * @return
 */
TYPE_ENTITY_LITERAL_ID QueryPlan::SelectANode(BasicQuery *basic_query,std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> processed_nodes) {
  int max_id = -1;
  double max_score = 0;
  if (processed_nodes->empty()) {
    for (int i = 0; i < basic_query->getVarNum(); ++i) {
      if (!basic_query->isSatelliteInJoin(i)) {
        double tmp_score = ScoreNode(basic_query, i);
        if (tmp_score >= max_score) {
          max_score = tmp_score;
          max_id = i;
        }
      }
    }
  } else {
    for (auto node =  processed_nodes->begin(); node != processed_nodes->end(); node++){
      int var_id = *node;
      int var_degree = basic_query->getVarDegree(var_id);
      for (int i = 0; i < var_degree; i++){
        int nei_id = basic_query->getEdgeNeighborID(var_id,i);
        if (!basic_query->isSatelliteInJoin(nei_id)) {
          double tmp_score = ScoreNode(basic_query, nei_id);
          if (tmp_score >= max_score) {
            max_score = tmp_score;
            max_id = nei_id;
          }
        }
      }
    }
  }
  return max_id;
}


/**
 *  A simpled strategy which don't consider predicate variable
 * @param basic_query   BasicQueryPointer
 * @param kv_store      KVStorePointer
 * @param var_infos     the var info list which we will fill in this function
 * @return
 */
shared_ptr<QueryPlan> QueryPlan::DefaultBFS(BasicQuery *basic_query,KVstore *kv_store,shared_ptr<vector<OldVarDescriptor>> var_infos) {
  auto r = make_shared<QueryPlan>();
  r->constant_generating_lists_ = make_shared<vector<shared_ptr<FeedOneNode>>>();
  auto total_var_num = basic_query->getTotalVarNum();
  vector<bool> vars_used_vec(total_var_num,false);
  // When id < total_var_num, the var in 'var_infos' maps exactly the id in BasicQuery
  for(int i = 0;i<total_var_num; i++) {
    var_infos->emplace_back(OldVarDescriptor::VarType::EntityType ,basic_query->getVarName(i),basic_query);
  }

  auto graph_var_num = basic_query->getVarNum();
  auto pre_var_num = basic_query->getPreVarNum();
  auto selected_var_num = basic_query->getSelectVarNum();

  r->join_order_ = make_shared<vector<StepOperation>>();
  r->ids_after_join_ = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();

  r->var_descriptors_ = var_infos;

  int start_node = SelectANode(basic_query, r->ids_after_join_);

  // for the first node, always using constant to generate candidate

  auto one_step_join = FilterFirstNode(basic_query, kv_store, start_node, r->var_descriptors_);
  r->join_order_->push_back(one_step_join);

  vars_used_vec[start_node] = true;
  r->ids_after_join_->push_back(start_node);


  // Loop for r->var_descriptors_->size()-1 times
  for(int _ = 1; _ < r->var_descriptors_->size(); _++) {
    TYPE_ENTITY_LITERAL_ID node_to_add = SelectANode(basic_query, r->ids_after_join_);

    // add the max to selected vars
    vars_used_vec[node_to_add] = true;
    r->ids_after_join_->push_back(node_to_add);

    auto join_new_node_plan = LinkWithPreviousNodes(basic_query, kv_store, node_to_add , r->ids_after_join_);
    r->join_order_->push_back(join_new_node_plan);


    auto constant_filtering = FilterNodeOnConstantEdge(basic_query, kv_store, node_to_add);
    r->constant_generating_lists_->push_back(constant_filtering);

  }

  ProcessPredicateAndSatellites(basic_query, kv_store, vars_used_vec, graph_var_num, pre_var_num,r->join_order_,r->ids_after_join_);
  return r;
}

/**
 * Deal with Satellites vars.
 * First Satellites vars(entity)
 * Next Predicate
 * Finally vars with a predicate var linking other parts of the graph
 * @param basic_query       BasicQueryPointer
 * @param kv_store          KVStorePointer
 * @param vars_used_vec     The list recording which vars have been used
 * @param graph_var_num     .
 * @param pre_var_num       .
 * @param join_order        The join record structure we need to fill
 * @param ids_after_join    The ids in table
 */
void QueryPlan::ProcessPredicateAndSatellites(BasicQuery *basic_query,
                                              KVstore *kv_store,
                                              vector<bool> &vars_used_vec,
                                              int graph_var_num,
                                              unsigned int pre_var_num,std::shared_ptr<std::vector<StepOperation>> &join_order,
                                              std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> &ids_after_join) {

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
      {
        join_method = JoinMethod::po2s;
        predicate_id = kv_store->getIDByPredicate(triple_string_type.getPredicate());
      }
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
      else if(p_constant) {
        join_method = JoinMethod::sp2o;
        predicate_id = kv_store->getIDByPredicate(triple_string_type.getPredicate());
      }
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

    StepOperation one_step_join;
    one_step_join.join_type_ = StepOperation::JoinType::EdgeCheck;
    auto one_step_join_node = make_shared<FeedOneNode>();
    one_step_join_node->node_to_join_ = satellite_id;
    one_step_join_node->edges_ = check_edge_info;
    one_step_join_node->edges_constant_info_ = check_edge_constant_info;
    one_step_join_node->ChangeOrder(ids_after_join);
    one_step_join.edge_filter_ = one_step_join_node;
    join_order->push_back(one_step_join);

    // add the max to selected vars
    vars_used_vec[satellite_id] = true;
    ids_after_join->push_back(satellite_id);
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
    StepOperation one_step_join;
    one_step_join.join_type_ = StepOperation::JoinType::JoinNode;
    auto one_step_join_node = make_shared<FeedOneNode>();
    one_step_join_node->node_to_join_ = pre_id_Table;
    one_step_join_node->edges_ = predicate_edge_info;
    one_step_join_node->edges_constant_info_ = predicate_edge_constant_info;
    one_step_join_node->ChangeOrder(ids_after_join);
    one_step_join.join_node_ = one_step_join_node;
    join_order->push_back(one_step_join);
    ids_after_join->push_back(pre_id_Table);
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

    StepOperation one_step_join;
    one_step_join.join_type_ = StepOperation::JoinType::EdgeCheck;
    auto one_step_join_node = make_shared<FeedOneNode>();
    one_step_join_node->node_to_join_ = left_id;
    one_step_join_node->edges_ = check_edge_info;
    one_step_join_node->edges_constant_info_ = check_edge_constant_info;
    one_step_join_node->ChangeOrder(ids_after_join);
    one_step_join.edge_filter_ = one_step_join_node;
    join_order->push_back(one_step_join);

    // add the max to selected vars
    vars_used_vec[left_id] = true;
    ids_after_join->push_back(left_id);
  }
}

/**
 * Using constant edge to intersect candidate list for a cold start
 * @param basic_query   BasicQueryPointer
 * @param kv_store      KVStorePointer
 * @param start_id      the first id
 * @param var_list      the Var Descriptor list
 * @return a generating plan
 */
StepOperation QueryPlan::FilterFirstNode(BasicQuery *basic_query, KVstore *kv_store,
                                     TYPE_ENTITY_LITERAL_ID start_id, const shared_ptr<vector<OldVarDescriptor>> &var_list) {

  StepOperation one_step_join;
  auto generate_edge_info = make_shared<vector<EdgeInfo>>();
  auto generate_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
  for(int i_th_edge=0; i_th_edge<(*var_list)[start_id].degree_; i_th_edge++)
  {
    auto edge_id =  basic_query->getEdgeID(start_id, i_th_edge);
    auto nei_id = basic_query->getEdgeNeighborID(start_id, i_th_edge);
    auto predicate_id = basic_query->getEdgePreID(start_id, i_th_edge);
    auto triple_string_type = basic_query->getTriple(edge_id);

    JoinMethod join_method;
    auto j_name = basic_query->getVarName(start_id);

    bool s_constant = triple_string_type.getSubject().at(0)!='?';
    bool p_constant = triple_string_type.getPredicate().at(0)!='?';
    bool o_constant = triple_string_type.getObject().at(0)!='?';
    // max_j is subject
    if(j_name==triple_string_type.getSubject())
    {
      if(p_constant)
      {
        predicate_id = kv_store->getIDByPredicate(triple_string_type.getPredicate());
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
      generate_edge_info->emplace_back(start_id, predicate_id, nei_id, join_method);
    }
    else{ // max_j is object
      if(p_constant) {
        predicate_id = kv_store->getIDByPredicate(triple_string_type.getPredicate());
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
      generate_edge_info->emplace_back(nei_id, predicate_id, start_id, join_method);
    }
    generate_edge_constant_info->emplace_back(s_constant,p_constant,o_constant);
  }
  one_step_join.join_type_ = StepOperation::JoinType::GenerateCandidates;
  auto one_step_join_node = make_shared<FeedOneNode>();
  one_step_join_node->node_to_join_ = start_id;
  one_step_join_node->edges_ = generate_edge_info;
  one_step_join_node->edges_constant_info_ = generate_edge_constant_info;
  one_step_join.edge_filter_ = one_step_join_node;
  return one_step_join;
}

/**
 * If the chosen one have a constant edge, that is ,
 * 1. neighbour and predicate are both constant , or
 * 2. have a not-selected predicate var and a constant neighbour
 * @param basic_query   BasicQueryPointer
 * @param kv_store      KVStorePointer
 * @param target_node   the node needed to check constant edge
 * @return the filtering plan
 */
shared_ptr<FeedOneNode> QueryPlan::FilterNodeOnConstantEdge(BasicQuery *basic_query,
                                                            KVstore *kv_store,
                                                            TYPE_ENTITY_LITERAL_ID target_node) {
  auto check_edge_info = make_shared<vector<EdgeInfo>>();
  auto check_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();

  auto j_degree = basic_query->getVarDegree(target_node);
  for (int i_th_edge = 0; i_th_edge < j_degree; i_th_edge++) {
    TYPE_ENTITY_LITERAL_ID sid, oid, pid;
    JoinMethod join_method;
    auto edge_id = basic_query->getEdgeID(target_node, i_th_edge);
    auto nei_id = basic_query->getEdgeNeighborID(target_node, i_th_edge);
    auto predicate_id = basic_query->getEdgePreID(target_node, i_th_edge);
    auto triple_string_type = basic_query->getTriple(edge_id);

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

  auto one_step_join_node = make_shared<FeedOneNode>();
  one_step_join_node->node_to_join_ = target_node;
  one_step_join_node->edges_ = check_edge_info;
  one_step_join_node->edges_constant_info_ = check_edge_constant_info;
  return one_step_join_node;
}

/**
 * If the chosen one have a constant edge, that is ,
 *  neighbour and predicate are both constant
 * @param bgp_query
 * @param kv_store      KVStorePointer
 * @param target_node   the node needed to check constant edge
 * @return the filtering plan
 */
shared_ptr<FeedOneNode> QueryPlan::FilterNodeOnConstantEdge(shared_ptr<BGPQuery> bgp_query,
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

  auto one_step_join_node = make_shared<FeedOneNode>();
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
shared_ptr<FeedOneNode> QueryPlan::FilterNodeOnConstantPredicate(shared_ptr<BGPQuery> bgp_query,
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

  auto one_step_join_node = make_shared<FeedOneNode>();
  one_step_join_node->node_to_join_ = target_node;
  one_step_join_node->edges_ = check_edge_info;
  one_step_join_node->edges_constant_info_ = check_edge_constant_info;
  return one_step_join_node;
}

/**
 *  Get Edge Info with previous nodes to Generate Query Plan
 *  predicate can be constant/variable, neighbour should have been in the table
 * @param basic_query   BasicQueryPointer
 * @param kv_store      KVStorePointer
 * @param added_id      the node to add
 * @param table_ids     the ids which already in table
 * @return A JOIN Node query plan
 */
StepOperation QueryPlan::LinkWithPreviousNodes(BasicQuery *basic_query,
                                           const KVstore *kv_store,
                                           TYPE_ENTITY_LITERAL_ID added_id,
                                           const std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> &table_ids) {
  auto join_edge_info = make_shared<vector<EdgeInfo>>();
  auto join_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
  for (auto selected_var:*table_ids) {
    if(selected_var == added_id) continue;
    auto i_th_edge_orders = basic_query->getEdgeIndex(added_id, selected_var);
    auto result = LinkTwoNode(basic_query, kv_store, added_id, selected_var);
    auto r_edge_info = get<0>(result);
    auto r_edge_constant_info = get<1>(result);
    join_edge_info->insert(join_edge_info->end(),r_edge_info->begin(),r_edge_info->end());
    join_edge_constant_info->insert(join_edge_constant_info->end(),r_edge_constant_info->begin(),r_edge_constant_info->end());
  }

  StepOperation one_step_join;
  one_step_join.join_type_ = StepOperation::JoinType::JoinNode;
  auto one_step_join_node = make_shared<FeedOneNode>();
  one_step_join_node->node_to_join_ = added_id;
  one_step_join_node->edges_ = join_edge_info;
  one_step_join_node->edges_constant_info_ = join_edge_constant_info;
  one_step_join_node->ChangeOrder(table_ids);
  one_step_join.join_node_ = one_step_join_node;
  return one_step_join;
}

/**
 *
 * @param basic_query BasicQuery Instance Pointer
 * @param kv_store
 * @param added_id
 * @param id_already_in_table
 * @return Edge Info list & its corresponding constant info
 */
tuple<shared_ptr<vector<EdgeInfo>>,shared_ptr<vector<EdgeConstantInfo>>>
    QueryPlan::LinkTwoNode(BasicQuery *basic_query,
                            const KVstore *kv_store,
                            TYPE_ENTITY_LITERAL_ID added_id,
                            TYPE_ENTITY_LITERAL_ID id_already_in_table) {
  TYPE_ENTITY_LITERAL_ID selected_var = id_already_in_table;
  auto join_edge_info = make_shared<vector<EdgeInfo>>();
  auto join_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
  auto i_th_edge_orders = basic_query->getEdgeIndex(added_id, selected_var);
  for (auto i_th_edge:i_th_edge_orders) {
    auto i_th_edge_result = WrapEdgeInfo_i_th_Edge(basic_query, kv_store, added_id, i_th_edge);
    join_edge_info->push_back(get<0>(i_th_edge_result));
    join_edge_constant_info->push_back(get<1>(i_th_edge_result));
  }
  return make_tuple(join_edge_info,join_edge_constant_info);
}


tuple<EdgeInfo,EdgeConstantInfo>
QueryPlan::WrapEdgeInfo_i_th_Edge(BasicQuery *basic_query,
                             const KVstore *kv_store,
                             TYPE_ENTITY_LITERAL_ID added_id,
                             int i_th_edge) {
  auto edge_id = basic_query->getEdgeID(added_id, i_th_edge);
  auto nei_id = basic_query->getEdgeNeighborID(added_id, i_th_edge);
  auto predicate_id = basic_query->getEdgePreID(added_id, i_th_edge);
  auto triple_string_type = basic_query->getTriple(edge_id);
  EdgeInfo edge_info;
  EdgeConstantInfo edge_constant_info;
  JoinMethod join_method;
  auto j_name = basic_query->getVarName(added_id);

  bool s_constant = triple_string_type.getSubject().at(0) != '?';
  bool p_constant = triple_string_type.getPredicate().at(0) != '?';
  bool o_constant = triple_string_type.getObject().at(0) != '?';
  // added_id is subject
  if (j_name == triple_string_type.getSubject()) {
    if (p_constant) {
      join_method = JoinMethod::po2s;
      predicate_id = kv_store->getIDByPredicate(triple_string_type.getPredicate());
    } else
      join_method = JoinMethod::o2s;
    edge_info = EdgeInfo(added_id, predicate_id, nei_id, join_method);
  } else { // added_id is object
    if (p_constant) {
      join_method = JoinMethod::sp2o;
      predicate_id = kv_store->getIDByPredicate(triple_string_type.getPredicate());
    } else
      join_method = JoinMethod::s2o;
    edge_info = EdgeInfo(nei_id, predicate_id, added_id, join_method);
  }

  edge_constant_info = EdgeConstantInfo(s_constant, p_constant, o_constant);
  return make_tuple(edge_info,edge_constant_info);
}

std::string QueryPlan::toString(KVstore* kv_store) {
  stringstream ss;
  ss<<"QueryPlan:\n";
  ss<<"join order size "<<this->join_order_->size()<<endl;
  for(int i=0;i<this->join_order_->size();i++)
  {
    auto step_n = (*this->join_order_)[i];

    ss << "\tstep[" << i << "]: " << StepOperation::JoinTypeToString(step_n.join_type_) << " \t ";
    shared_ptr<FeedOneNode> step_descriptor;
    switch (step_n.join_type_) {
      case StepOperation::JoinType::JoinNode:
        step_descriptor = step_n.join_node_;
        break;
      case StepOperation::JoinType::GenerateCandidates:
      case StepOperation::JoinType::EdgeCheck:
        step_descriptor = step_n.edge_filter_;
        break;
    }
    ss<<" node["<<step_descriptor->node_to_join_<<"]\n";

    // describe edge info
    for(int i =0;i<step_descriptor->edges_->size();i++)
      ss<< "\t \t "<<EdgeToString(kv_store,(*step_descriptor->edges_)[i],(*step_descriptor->edges_constant_info_)[i])<<"\n";
  }
  return ss.str();
}
std::tuple<std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>,
           std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>> QueryPlan::PositionIDMappings() {

  auto var_to_position = make_shared<map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>();
  auto position_to_var = make_shared<map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>();

  for(const auto &one_step: *this->join_order_)
  {
    bool node_added = false;
    TYPE_ENTITY_LITERAL_ID node_i = -1;
    switch (one_step.join_type_) {
      case  StepOperation::JoinType::JoinNode:
        node_i = one_step.join_node_->node_to_join_;
        node_added =true;
        break;
      case  StepOperation::JoinType::GenerateCandidates:
        node_i = one_step.edge_filter_->node_to_join_;
        node_added =true;
        break;
      case  StepOperation::JoinType::JoinTable: break;
      case  StepOperation::JoinType::EdgeCheck : break;
    }
    if(node_added)
    {
      auto map_size = var_to_position->size();
      (*var_to_position)[node_i] = map_size;
      (*position_to_var)[map_size] = node_i;
    }
  }

  return make_tuple(var_to_position,position_to_var);
}

std::shared_ptr<std::vector<std::shared_ptr<FeedOneNode>>> QueryPlan::OnlyConstFilter(BasicQuery *basic_query,
                                                                                      KVstore *kv_store,
                                                                                      std::shared_ptr<std::vector<OldVarDescriptor>> var_infos) {
 return make_shared<std::vector<std::shared_ptr<FeedOneNode>>>();
}

std::shared_ptr<std::vector<std::shared_ptr<FeedOneNode>>>
QueryPlan::OnlyConstFilter(std::shared_ptr<BGPQuery> bgp_query,
                           KVstore *kv_store) {
  auto result = make_shared<QueryPlan>();
  auto constant_generating_lists = make_shared<vector<shared_ptr<FeedOneNode>>>();
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


std::shared_ptr<std::vector<std::shared_ptr<FeedOneNode>>>
QueryPlan::PredicateFilter(std::shared_ptr<BGPQuery> bgp_query,
                           KVstore *kv_store) {
  auto result = make_shared<QueryPlan>();
  auto constant_predicate_generating_lists = make_shared<vector<shared_ptr<FeedOneNode>>>();
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

QueryPlan::QueryPlan(PlanTree *plan_tree) {
  // Do A depth first search
  auto root = plan_tree->root_node;
  stack<decltype(root)> q;
  q.push(root);
  while(!q.empty())
  {
    auto top_node = q.top();
    if(top_node->node->join_type_ == StepOperation::JoinType::GenerateCandidates)
      this->constant_generating_lists_->push_back(top_node->node->join_node_);
    else
      this->join_order_->push_back(*(top_node->node));
    if(top_node->left_node!= nullptr)
      q.push(top_node->left_node);
    if(top_node->right_node!= nullptr)
      q.push(top_node->right_node);
    q.pop();
  }
}

void QueryPlan::PreTravel(Tree_node *node)
{
  if(node->left_node!= nullptr)
    PreTravel(node->left_node);
  if(node->right_node!= nullptr)
    PreTravel(node->right_node);
  this->join_order_->push_back(*node->node);
}

QueryPlan::QueryPlan(Tree_node *root_node) {
  this->join_order_=make_shared<std::vector<StepOperation>>();
  this->ids_after_join_=make_shared<std::vector<TYPE_ENTITY_LITERAL_ID>>();
  this->var_descriptors_=make_shared<std::vector<OldVarDescriptor>>();
  this->constant_generating_lists_= make_shared<vector<std::shared_ptr<FeedOneNode>>>();
  PreTravel(root_node);
}

