//
// Created by Yuqi Zhou on 2021/8/30.
//

#include "TopKSearchPlan.h"

using namespace std;

/**
 * count the tree depth regarding the root_id as root.
 * @note must be a tree, not a graph with loops
 * @param neighbours the tree structure
 * @param root_id root
 * @param total_vars_num the node number of the tree
 * @return the tree depth
 */
std::size_t TopKSearchPlan::CountDepth(map<TYPE_ENTITY_LITERAL_ID, vector<TYPE_ENTITY_LITERAL_ID>>& neighbours,
                                       TYPE_ENTITY_LITERAL_ID root_id,
                                       std::size_t total_vars_num) {
  auto vars_used_vec = new bool[total_vars_num];
  auto vars_depth = new std::size_t [total_vars_num];
  memset(vars_used_vec,0,total_vars_num);
  memset(vars_depth,0,total_vars_num);
  vars_used_vec[root_id] = true;
  vars_depth[root_id]= 1;
  std::stack<int> explore_id;
  explore_id.push(root_id);

  while(!explore_id.empty())
  {
    auto now_id = explore_id.top();
    for(const auto& child_id: neighbours[now_id])
    {
      if(vars_used_vec[child_id])
        continue;
      vars_used_vec[child_id] = true;
      vars_depth[child_id] = vars_depth[now_id] + 1;
    }
    explore_id.pop();
  }

  auto max_depth = *std::max_element(vars_depth,vars_depth+total_vars_num);
  delete[] vars_used_vec;
  delete[] vars_depth;

  return max_depth;
}

TopKSearchPlan::TopKSearchPlan(shared_ptr<BGPQuery> bgp_query, KVstore *kv_store,
                               Statistics *statistics, QueryTree::Order expression,
                               shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> id_caches)
{
  this->total_vars_num_ = bgp_query->get_total_var_num();
  TYPE_ENTITY_LITERAL_ID CONSTANT = -1;

// all the information
// a neighbour may have more than one edge
  map<TYPE_ENTITY_LITERAL_ID ,vector<TYPE_ENTITY_LITERAL_ID>> neighbours;
  map<TYPE_ENTITY_LITERAL_ID,vector<vector<bool>>> predicates_constant;
  map<TYPE_ENTITY_LITERAL_ID,vector<vector<TYPE_ENTITY_LITERAL_ID>>> predicates_ids;
  map<TYPE_ENTITY_LITERAL_ID,vector<vector<TopKUtil::EdgeDirection>>> directions;

// constructing the information structure
  for(decltype(total_vars_num_) i = 0; i< total_vars_num_ ; i++)
  {
    map<TYPE_ENTITY_LITERAL_ID,size_t> neighbour_position;

    neighbours[i] = decltype(neighbours.begin()->second)();
    predicates_constant[i] = decltype(predicates_constant.begin()->second)();
    predicates_ids[i] = decltype(predicates_ids.begin()->second)();
    directions[i] = decltype(directions.begin()->second)();

    auto var_descriptor = bgp_query->get_vardescrip_by_index(i);
    if(var_descriptor->var_type_ == VarDescriptor::VarType::Predicate)
      continue;

    auto &edge_index_list = var_descriptor->so_edge_index_;
    auto degree = bgp_query->get_var_degree(i);
    auto v_name = bgp_query->get_var_name_by_id(i);
    for(auto edge_id:edge_index_list) {
      decltype(i) nei_id;
      auto &triple = bgp_query->get_triple_by_index(edge_id);
      decltype(triple.object) nei_name;
      TopKUtil::EdgeDirection direction=TopKUtil::EdgeDirection::NoEdge;
      if(triple.subject==v_name) {
        nei_name = triple.object;
        direction = TopKUtil::EdgeDirection::OUT;
      }
      else {
        nei_name = triple.subject;
        direction = TopKUtil::EdgeDirection::IN;
      }
      if(nei_name[0]=='?')
        nei_id = CONSTANT;

      if(nei_id==CONSTANT)
        continue;

      TYPE_ENTITY_LITERAL_ID predicates_id = CONSTANT;
      auto predicate_constant = triple.predicate[0]!='?';
      if(predicate_constant)
        predicates_id = kv_store->getIDByPredicate(triple.predicate);
      else
        predicates_id = bgp_query->get_var_id_by_name(triple.predicate);

      size_t neighbour_pos;
      if(neighbour_position.find(nei_id)==neighbour_position.end())
      {
        neighbours[i].push_back(nei_id);
        neighbour_pos = neighbour_position.size();
        neighbour_position[nei_id] = neighbour_pos;

        predicates_constant[i].emplace_back();
        predicates_ids[i].emplace_back();
        directions[i].emplace_back();
      }


      predicates_constant[i][neighbour_pos].push_back(predicate_constant);
      predicates_ids[i][neighbour_pos].push_back(predicates_id);
      directions[i][neighbour_pos].push_back(direction);
    }

#ifdef TOPK_DEBUG_INFO
    std::cout<<std::endl;
#endif

  }
  int min_score_root = -1;
  auto min_score = DBL_MAX;

// get the root id by arg min( candidates * tree depth )
// if all nodes don't have candidates, choose the min tree depth
  for(decltype(total_vars_num_)  i = 0; i< total_vars_num_ ; i++)
  {
    auto var_descriptor = bgp_query->get_vardescrip_by_index(i);
    if(var_descriptor->var_type_ == VarDescriptor::VarType::Predicate)
      continue;

    if(id_caches->size()&&id_caches->find(i)==id_caches->end())
      continue;

// count the its depth if regard node i as root
    auto tree_depth = this->CountDepth(neighbours, i, total_vars_num_);
    auto candidates_size = id_caches->find(i)->second->size();
    double score;
    if(id_caches->size()>0)
      score = tree_depth * candidates_size;
    else
      score = tree_depth;

    if (score < min_score)
    {
      min_score_root = i;
      min_score = score;
    }
  }

#ifdef TOPK_DEBUG_INFO
  cout << "chosen root node:" << min_score_root << endl;
#endif

// construct the query tree
  bool*  vars_used_vec = new bool[total_vars_num_];
  memset(vars_used_vec, 0, total_vars_num_*sizeof(bool));
  auto root_id = min_score_root;
  vars_used_vec[root_id] = true;
  std::stack<TopKTreeNode*> explore_id;

  TopKTreeNode* r = new TopKTreeNode;
  r->var_id = root_id;
  explore_id.push(r);

  vector<StepOperation> edges_after_;
  while(!explore_id.empty()) {
    auto now_node = explore_id.top();
    auto now_id = now_node->var_id;
    explore_id.pop();
#ifdef TOPK_DEBUG_INFO
    cout << "pop " << now_node->var_id << " children num:" << neighbours[now_node->var_id].size();
    if (neighbours[now_node->var_id].size()) {
      cout<<"[";
      for(auto child_id:neighbours[now_node->var_id])
        cout<<child_id<<" ";
      cout<<"]";
    }
    cout<<endl;
#endif

    auto child_num = neighbours[now_id].size();
    for(decltype(child_num) child_i =0;child_i < child_num;child_i++)
    {
      auto child_id= neighbours[now_id][child_i];

// below are vectors
      auto two_var_edges_num = predicates_ids.size();
      auto two_var_predicate_ids = predicates_ids[now_id][child_i];
      auto two_var_predicate_constants = predicates_constant[now_id][child_i];
      auto two_var_directions = directions[now_id][child_i];

      if(vars_used_vec[child_id])
      {
        for(decltype(two_var_edges_num) edge_i =0;edge_i<two_var_edges_num;edge_i++)
        {
          auto predicate_id = two_var_predicate_ids[edge_i];
          auto predicate_constant =  two_var_predicate_constants[edge_i];
          auto direction =  two_var_directions[edge_i];
// leaves the edge to process after the main procedure
          EdgeInfo edge_info;
          EdgeConstantInfo edge_constant_info(false,predicate_constant, false);
          if(direction == TopKUtil::EdgeDirection::IN)
          {
            if(predicate_constant)
              edge_info = EdgeInfo(child_id,predicate_id,now_id,JoinMethod::po2s);
            else
              edge_info = EdgeInfo(child_id,predicate_id,now_id,JoinMethod::o2ps);
          }
          else // TopKUtil::EdgeDirection::OUT
          {
            if(predicate_constant)
              edge_info = EdgeInfo(now_id,predicate_id,child_id,JoinMethod::sp2o);
            else
              edge_info = EdgeInfo(now_id,predicate_id,child_id,JoinMethod::s2po);
          }

// add this edge to the postponed edges set
          edges_after_.emplace_back();
          auto &step_op = edges_after_.back();
          if(predicate_constant)
          {
            step_op.join_type_  = StepOperation::JoinType::EdgeCheck;
            step_op.edge_filter_->node_to_join_ = child_id;
            step_op.edge_filter_->edges_->push_back(edge_info);
            step_op.edge_filter_->edges_constant_info_->push_back(edge_constant_info);
          }
          else
          {
            step_op.join_type_  = StepOperation::JoinType::JoinTwoNodes;
            if(direction == TopKUtil::EdgeDirection::IN) // o2ps
            {
              step_op.join_two_node_->node_to_join_1_ =edge_info.p_;
              step_op.join_two_node_->node_to_join_2_ =edge_info.s_;
            }
            else // s2po
            {
              step_op.join_two_node_->node_to_join_1_ =edge_info.p_;
              step_op.join_two_node_->node_to_join_2_ =edge_info.o_;
            }
            step_op.join_two_node_->edges_ = edge_info;
            step_op.join_two_node_->edges_constant_info_ = edge_constant_info;
          }
        }
        continue;
      }
      vars_used_vec[child_id] = true;
      auto child_tree = new TopKTreeNode;
      child_tree->var_id = child_id;
      now_node->descendents_.push_back(child_tree);
      auto tree_edge_ptr = make_shared<TopKUtil::TreeEdge>();
      tree_edge_ptr->predicate_constant_ = std::move(two_var_predicate_constants);
      tree_edge_ptr->predicate_ids_ = std::move(two_var_predicate_ids);
      tree_edge_ptr->directions_ = std::move(two_var_directions);
      now_node->tree_edges_.push_back(tree_edge_ptr);
      tree_edge_ptr->ChangeOrder();
#ifdef TOPK_DEBUG_INFO
      cout << "push " << child_tree->var_id<<endl;
#endif
      explore_id.push(child_tree);
    }
  }
  delete[] vars_used_vec;
  this->tree_root_ = r;
  this->AdjustOrder();
}

TopKSearchPlan::~TopKSearchPlan() {
  std::stack<TopKTreeNode*> to_delete;
  to_delete.push(this->tree_root_);
  while(!to_delete.empty())
  {
    auto tree_node = to_delete.top();
    to_delete.pop();
    for(const auto child: tree_node->descendents_)
    {
      to_delete.push(child);
    }
    delete tree_node;
  }
}

std::string TopKSearchPlan::DebugInfo() {
  stringstream ss;
  std::string result;
  std::stack<decltype(this->tree_root_)> h;
  h.push(this->tree_root_);
  while(!h.empty())
  {
    auto t = h.top();
    h.pop();
    string id_str;
    ss<<t->var_id;
    ss>>id_str;
    ss.clear();
    ss.str("");

    string children_str;
    for(auto &child_ptr:t->descendents_)
    {
      h.push(child_ptr);
      std::string child_str;
      ss<<child_ptr->var_id;
      ss>>child_str;
      ss.clear();
      ss.str("");
      children_str += child_str + " ";
    }
    result += id_str + "(" + children_str + ")\n";

  }
  return result;
}

/**
 * Change The order so that each node's descendents are OW first and FRs last
 * Making Order Retrieve easier
 */
void TopKSearchPlan::AdjustOrder() {

  // count fr and ow information
  bool is_fr[this->total_vars_num_+1];
  for(size_t i = 0;i<this->total_vars_num_+1;i++)
    is_fr[i] = false;

  stack<TopKTreeNode*> node_to_visit;
  node_to_visit.push(this->tree_root_);

  while(!node_to_visit.empty())
  {
    auto top = node_to_visit.top();
    top->descendents_fr_num_ =0 ;
    top->descendents_ow_num_ =0 ;
    if(top->descendents_.size()!=0)
      is_fr[top->var_id] = true;
    node_to_visit.pop();
    for(auto child_p:top->descendents_)
      node_to_visit.push(child_p);
  }


  // now to adjust the order
  node_to_visit.push(this->tree_root_);

  while(!node_to_visit.empty())
  {
    auto top = node_to_visit.top();

    // [0,const_end) stores constant edge
    // [const_end,end) stores variable edge
    size_t ow_end = 0;
    auto descendents_num = top->descendents_.size();
    for(decltype(descendents_num) i=0;i<descendents_num;i++)
    {
      // move it to the left part
      if(is_fr[i] == false)
      {
        // exchange i with const_end
        if(i!=ow_end)
          std::swap(top->descendents_[i],top->descendents_[ow_end]);
        ow_end++;
      }
    }

    top->descendents_ow_num_ = ow_end;
    top->descendents_fr_num_ = descendents_num - ow_end;

    node_to_visit.pop();
    for(auto child_p:top->descendents_) {
      if(is_fr[child_p->var_id])
        node_to_visit.push(child_p);
    }
  }
}
