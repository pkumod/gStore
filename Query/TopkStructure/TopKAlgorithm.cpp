//
// Created by 37584 on 2021/3/31.
//

#include "TopKAlgorithm.h"

/*
 * choose a root node and count its layer num
 */
void TopKAlgorithm::GenerateTree(BasicQuery *basic_query, KVstore *kv_store,QueryTree::CompTreeNode tree_root_node) {


  auto var_num = basic_query->getVarNum();
  int max_layer = 0,max_root_id;

  for(int root_id = 1;root_id<var_num;root_id++)
  {
    auto result = this->CountLayer(basic_query,root_id);
    if(result>max_layer)
    {
      max_layer = result;
      max_root_id = root_id;
    }
  }
  // go with max root id
  // generate a tree that contains all nodes in tree_root_node
  //

  this->root_id_ = max_root_id;
  auto looked_nodes = set<TYPE_ENTITY_LITERAL_ID>();
  auto layer_node = make_shared<set<TYPE_ENTITY_LITERAL_ID>>();
  for(int i_th_edge = 0; i_th_edge<basic_query->getVarDegree(this->root_id_); i_th_edge++)
  {
    layer_node->insert(basic_query->getEdgeNeighborID(this->root_id_, i_th_edge));
  }


  looked_nodes.insert(layer_node->begin(),layer_node->end());

  auto root_descendants = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();
  root_descendants->insert(root_descendants->end(),layer_node->begin(),layer_node->end());
  (*tree_descendant)[this->root_id_] = root_descendants;

  auto root_r = QueryPlan::FilterNodeOnConstantEdge(basic_query,kv_store,this->root_id_);

  while(!layer_node->empty()) {
    auto next_layer_nodes = make_shared<set<TYPE_ENTITY_LITERAL_ID>>();
    for(const auto& t_id :*layer_node)
    {
      shared_ptr<OneStepJoin> node_plan = nullptr;
      bool have_constant_edge = false;
      auto t_descendant = make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();

      vector<bool> edge_dealed(false,basic_query->getVarDegree(this->root_id_));
      for (int i_th_edge = 0; i_th_edge < basic_query->getVarDegree(this->root_id_); i_th_edge++)
      {

        auto new_node = basic_query->getEdgeNeighborID(t_id, i_th_edge);

        // This edge is a non-tree edge
        if (looked_nodes.find(new_node) != looked_nodes.end()) continue;

        // Generate A query Plan here
        auto descendant_id = new_node;

        auto edge_id = basic_query->getEdgeID(t_id, i_th_edge);
        auto triple = basic_query->getTriple(edge_id);
        auto pre_id = triple.getPredicate();

        // Don't consider constant edge
        if(triple.getSubject().at(0)!='?' || triple.getObject().at(0)!='?' ) {
          edge_dealed[i_th_edge] = true;
          have_constant_edge = true;
          continue;
        }

        edge_dealed[i_th_edge] = true;
        next_layer_nodes->insert(descendant_id);
        t_descendant->push_back(descendant_id);

      }

      if(have_constant_edge)
        this->constant_generating_lists_->push_back(QueryPlan::FilterNodeOnConstantEdge(basic_query, kv_store, t_id));

      auto check_join_info = make_shared<vector<EdgeInfo>>();
      auto check_join_constant_info = make_shared<vector<EdgeConstantInfo>>();

      for(const auto& descendant_id:*t_descendant)
      {
        (*this->parent_map_)[descendant_id] = t_id;
        auto link_result = QueryPlan::LinkTwoNode(basic_query,kv_store,t_id,descendant_id);
        auto t_descendant_edge_info = get<0>(link_result);
        auto t_descendant_edge_const_info = get<1>(link_result);
        check_join_info->insert(check_join_info->end(), t_descendant_edge_info->begin(), t_descendant_edge_info->end());
        check_join_constant_info->insert(check_join_constant_info->end(), t_descendant_edge_const_info->begin(), t_descendant_edge_const_info->end());
      }


      auto one_step_join_ptr = make_shared<OneStepJoin>();
      one_step_join_ptr->join_type_ = OneStepJoin::JoinType::JoinNode;
      auto one_step_join_node = make_shared<OneStepJoinNode>();
      one_step_join_node->node_to_join_ = t_id;
      one_step_join_node->edges_ = check_join_info;
      one_step_join_node->edges_constant_info_ = check_join_constant_info;
      one_step_join_ptr->join_node_ = one_step_join_node;

      (*tree_excution_plan_map_)[t_id] = one_step_join_ptr;
      looked_nodes.insert(layer_node->begin(), layer_node->end());

      // 还需要 edge check 的内容
      for (int i_th_edge = 0; i_th_edge < basic_query->getVarDegree(root_id_); i_th_edge++)
      {
        if(edge_dealed[i_th_edge])
          continue;

        // Now Add edge check

        // tuple<EdgeInfo,EdgeConstantInfo>
        auto edge_constraint_info = QueryPlan::WrapEdgeInfo_i_th_Edge(basic_query,kv_store,t_id,i_th_edge);

        auto one_step_join_ptr = make_shared<OneStepJoin>();

        one_step_join_ptr->join_type_ = OneStepJoin::JoinType::EdgeCheck;
        auto one_step_join_node = make_shared<OneStepJoinNode>();
        one_step_join_node->node_to_join_ = t_id;

        auto edge_info_vec_ptr = make_shared<vector<EdgeInfo>>();
        edge_info_vec_ptr->push_back(get<0>(edge_constraint_info));
        one_step_join_node->edges_ = edge_info_vec_ptr;

        auto edge_const_info_vec_ptr = make_shared<vector<EdgeConstantInfo>>();
        edge_const_info_vec_ptr->push_back(get<1>(edge_constraint_info));
        one_step_join_node->edges_constant_info_ =  edge_const_info_vec_ptr;

        one_step_join_ptr->edge_filter_ = one_step_join_node;
      }


      // 最后把树结构信息更新上去
      (*tree_descendant)[t_id] = t_descendant;
    }
    layer_node = next_layer_nodes;
  }


}

int TopKAlgorithm::CountLayer( BasicQuery *basic_query,int root_id) {

  int layer_count = 1;
  auto looked_nodes = set<TYPE_ENTITY_LITERAL_ID>();
  auto layer_node = make_shared<set<TYPE_ENTITY_LITERAL_ID>>();
  for(int i_th_edge = 0;i_th_edge<basic_query->getVarDegree(root_id);i_th_edge++)
  {
    layer_node->insert(basic_query->getEdgeNeighborID(root_id,i_th_edge));
  }
  if(layer_node->empty())
    return layer_count;

  looked_nodes.insert(layer_node->begin(),layer_node->end());

  while(!layer_node->empty()) {
    layer_count++;
    auto next_layer_nodes = make_shared<set<TYPE_ENTITY_LITERAL_ID>>();
    for(const auto& t_id :*layer_node)
    {
      for (int i_th_edge = 0; i_th_edge < basic_query->getVarDegree(root_id); i_th_edge++)
      {
        auto new_node = basic_query->getEdgeNeighborID(t_id, i_th_edge);
        if (looked_nodes.find(new_node) == looked_nodes.end())
          next_layer_nodes->insert(new_node);
      }
    }
    if (next_layer_nodes->empty())
      return layer_count;

    looked_nodes.insert(layer_node->begin(),layer_node->end());
    layer_node = next_layer_nodes;
  }
  return layer_count;
}

/**
 * If a tiwg doesn't have a CompTreeNode as descendant, cut it, move its edges into edge check clause
 * @param basic_query
 * @param kv_store
 * @param tree_root_node
 */
void TopKAlgorithm::PruneEdge(BasicQuery *basic_query, KVstore *kv_store, QueryTree::CompTreeNode tree_root_node) {

  // the set stores raw var names
  auto comp_node_set = tree_root_node.getCompTreeVarset();
  this->PostorderTraversalPrune(basic_query,comp_node_set,this->root_id_);
}

bool TopKAlgorithm::PostorderTraversalPrune(BasicQuery *basic_query, const Varset& var_set,TYPE_ENTITY_LITERAL_ID node)
{
  // The twig will not pruned
  if(var_set.findVar(basic_query->getVarName(node)))
    return true;

  bool descendant_match_with_var_set = false;
  for(auto descendant_node : *(*this->tree_descendant)[node])
  {
    if(PostorderTraversalPrune(basic_query,var_set,descendant_node))
      descendant_match_with_var_set = true;
  }

  if(descendant_match_with_var_set)
    return true;


  // if all children not matched with var_set, prune this node

  auto node_plan = (*(this->tree_excution_plan_map_))[node];
  this->tree_excution_plan_map_->erase(node);

  (*this->tree_extend_plan_map_)[node] = node_plan;
}

bool TopKAlgorithm::FillFRwithFQ(TYPE_ENTITY_LITERAL_ID FR_q_id, shared_ptr<FRiterator> FR_it ,shared_ptr<IDList> FQ_node_ids)
{
  FR_it->ReserveSpace(FQ_node_ids->size());
  auto tmp_id_pos_map = make_shared<map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>();
  auto descendants = this->tree_descendant->find(FR_q_id)->second;
  auto tmp_table = make_shared<TableContent>();

  for(const auto& FQ_data_id:*FQ_node_ids)
  {
    auto fq_sptr = make_shared<FQiterator>(FQ_data_id,this->k_);
    FR_it->Insert(fq_sptr);

    // FQ's descendants
    for(const auto&descendant_id:*descendants)
    {
      auto record_it = tmp_table->begin();
      auto descend_candidates = this->opt_ptr_->ExtendRecord((*this->tree_excution_plan_map_)[descendant_id]->join_node_,
                                                             tmp_id_pos_map,
                                                             this->id_caches_share_ptr_,
                                                             descendant_id,
                                                             record_it);
      // One Way It
      if( (*this->is_leaf_)[descendant_id])
      {
        auto one_way_it = make_shared<OneWayIterator>(this->kv_store_,descend_candidates);
        fq_sptr->AddDescendant(one_way_it);
      }
      else // Another FR
      {
        auto descendant_FR = make_shared<FRiterator>();
        this->FillFRwithFQ(descendant_id,descendant_FR,descend_candidates);
      }
    }
  }
}




void TopKAlgorithm::DoQueryBasedOnDpB() {

  // start from root id
  // construct dp-b network
  deque<TYPE_ENTITY_LITERAL_ID> nodes;

  auto cold_start_plan = (*(this->tree_excution_plan_map_))[this->root_id_];
  auto root_generate_plan = cold_start_plan->edge_filter_;

  auto root_candidates_vec = this->opt_ptr_->CandidatesWithConstantEdge(root_generate_plan->edges_);

  // generate the root FR-iterator


  // Now create a virtual root node
  while(!nodes.empty())
  {
    auto node_t = nodes.front();
    nodes.pop_front();

  }

}


