//
// Created by Yuqi Zhou on 2021/7/10.
//

#include "TopKUtil.h"

/**
 * count the tree depth regarding the root_id as root.
 * @note must be a tree, not a graph with loops
 * @param neighbours the tree structure
 * @param root_id root
 * @param total_vars_num the node number of the tree
 * @return the tree depth
 */
std::size_t TopKTreeSearchPlan::CountDepth(map<TYPE_ENTITY_LITERAL_ID,vector<TYPE_ENTITY_LITERAL_ID>>& neighbours,
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


  delete[] vars_used_vec;
  delete[] vars_depth;

  return *std::max_element(vars_depth,vars_depth+total_vars_num);
}

TopKTreeSearchPlan::TopKTreeSearchPlan(shared_ptr<BGPQuery> bgp_query, KVstore kv_store,
                                       Statistics *statistics, QueryTree::Order expression,
                                       shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> id_caches)
{
  auto total_vars_num = bgp_query->get_total_var_num();
  TYPE_ENTITY_LITERAL_ID CONSTANT = -1;

  // all the information
  // a neighbour may have more than one edge
  map<TYPE_ENTITY_LITERAL_ID ,vector<TYPE_ENTITY_LITERAL_ID>> neighbours;
  map<TYPE_ENTITY_LITERAL_ID,vector<vector<bool>>> predicates_constant;
  map<TYPE_ENTITY_LITERAL_ID,vector<vector<TYPE_ENTITY_LITERAL_ID>>> predicates_ids;
  map<TYPE_ENTITY_LITERAL_ID,vector<vector<TopKUtil::EdgeDirection>>> directions;

  // constructing the information structure
  for(decltype(total_vars_num) i = 0;i< total_vars_num ; i++)
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
        predicates_id = kv_store.getIDByPredicate(triple.predicate);
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
  for(decltype(total_vars_num)  i = 0;i< total_vars_num ; i++)
  {
    auto var_descriptor = bgp_query->get_vardescrip_by_index(i);
    if(var_descriptor->var_type_ == VarDescriptor::VarType::Predicate)
      continue;

    if(id_caches->size()&&id_caches->find(i)==id_caches->end())
      continue;

    // count the its depth if regard node i as root
    auto tree_depth = this->CountDepth(neighbours, i, total_vars_num);
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
  bool*  vars_used_vec = new bool[total_vars_num];
  memset(vars_used_vec,0,total_vars_num*sizeof(bool));
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
}



TopKTreeSearchPlan::~TopKTreeSearchPlan() {
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
std::string TopKTreeSearchPlan::DebugInfo() {
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

void TopKUtil::CalculatePosVarMappingNode(TopKTreeNode* top_k_tree_node,
                                          shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> pos_var_mapping)
{
  auto now_position = pos_var_mapping->size();
  (*pos_var_mapping)[now_position] = top_k_tree_node->var_id;
  for(auto descendent:top_k_tree_node->descendents_)
  {
    CalculatePosVarMappingNode(descendent,pos_var_mapping);
  }
}

shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>
TopKUtil::CalculatePosVarMapping(shared_ptr<TopKTreeSearchPlan> search_plan)
{
  auto r = make_shared<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>();
  CalculatePosVarMappingNode(search_plan->tree_root_,r);
  return r;
}


double TopKUtil::GetScore(string &v, stringstream &ss)
{
  double double_v;
  ss<<v;
  ss>>double_v;
  ss.clear();
  ss.str("");
#ifdef SHOW_SCORE
  std::cout<<"TopKUtil::GetScore str:"<<v<<" double "<<double_v<<std::endl;
#endif
  return double_v;
}

void TopKUtil::GetVarCoefficientsTreeNode(QueryTree::CompTreeNode *comp_tree_node,
                                          std::map<std::string,double>& coefficients,
                                          stringstream &ss)
{
#ifdef TOPK_DEBUG_INFO
  std::cout<<"Node:"<<comp_tree_node->val<<" "<<comp_tree_node->lchild->val<<" "<<comp_tree_node->rchild->val<<std::endl;
#endif
  // if both of the child are leaves
  if(comp_tree_node->lchild->lchild==nullptr&&comp_tree_node->lchild->rchild==nullptr&&
      comp_tree_node->rchild->lchild==nullptr&&comp_tree_node->rchild->rchild==nullptr)
  {

    if (comp_tree_node->lchild->val.at(0) == '?') // ?x * 0.1
    {
      auto val_string = comp_tree_node->rchild->val.substr(1);
      coefficients[comp_tree_node->lchild->val] = GetScore(val_string,ss);
    }
    else // 0.1 * ?x
    {
      auto val_string = comp_tree_node->lchild->val.substr(1);
      coefficients[comp_tree_node->rchild->val] = GetScore(val_string,ss);;
    }
  }
  else
  {
    GetVarCoefficientsTreeNode(comp_tree_node->lchild, coefficients, ss);
    GetVarCoefficientsTreeNode(comp_tree_node->rchild, coefficients, ss);
  }
}

/**
 * Suppose the SPARQL is simple, base unit is like '0.1 * ?x'
 * And the expression is like the sum of base units
 * @param order
 * @return
 */
std::shared_ptr<std::map<std::string,double>> TopKUtil::getVarCoefficients(QueryTree::Order order)
{
  stringstream ss;
  auto r= make_shared<std::map<std::string,double>>();
  GetVarCoefficientsTreeNode(order.comp_tree_root, *r, ss);

#ifdef TOPK_DEBUG_INFO

  std::cout<<"VarCoefficients:"<<std::endl;
  for(const auto& pair:*r)
  {
    std::cout<<pair.first<<"\t"<<pair.second<<std::endl;
  }
#endif
  return r;
}

void TopKUtil::UpdateIDList(shared_ptr<IDList> valid_id_list, unsigned* id_list, unsigned id_list_len,bool id_list_prepared)
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

void TopKUtil::UpdateIDListWithAppending(shared_ptr<IDListWithAppending> ids_appending, unsigned* id_list,
                                         unsigned id_list_len,unsigned  one_record_len,
                                         bool id_list_prepared,unsigned main_key_position)
{
  if(id_list_prepared)
    ids_appending->Intersect(id_list, id_list_len,one_record_len,main_key_position);
  else
    ids_appending->Init(id_list,id_list_len,one_record_len,main_key_position);
}

void TopKUtil::FreeGlobalIterators(std::shared_ptr<std::vector<std::shared_ptr<std::set<OrderedList*>>>> global_iterators)
{
  for(auto layer_iterators:*global_iterators)
  {
    if(layer_iterators->size())
    {
      auto iterator_type =  (*layer_iterators->begin())->Type();
      switch (iterator_type)
      {
        case OrderedListType::FR:
          for(auto iterator:*layer_iterators)
          {
            delete (FRIterator*)iterator;
          }
          break;
        case OrderedListType::OW:
          for(auto iterator:*layer_iterators)
          {
            delete (OWIterator*)iterator;
          }
          break;
        case OrderedListType::FQ:
          for(auto iterator:*layer_iterators)
          {
            delete (FQIterator*)iterator;
          }
          break;
      };
    }
  }
}

/**
 * Build Iterators for top k search.
 * @return the final FRIterator
 */
FRIterator *TopKUtil::BuildIteratorTree(const shared_ptr<TopKTreeSearchPlan> tree_search_plan,Env *env){

  auto root_plan = tree_search_plan->tree_root_;
  auto root_var = root_plan->var_id;
  auto child_type_num = root_plan->descendents_.size();

  // from top to down, build the structure
  auto root_candidate_ids = (*env->id_caches)[root_var];

  set<TYPE_ENTITY_LITERAL_ID> root_candidate;
  auto coefficient_it = env->coefficients->find(env->basic_query->getVarName(root_var));
  bool has_coefficient = coefficient_it != env->coefficients->end();
  double coefficient = has_coefficient? (*coefficient_it).second:0.0;

  // calculating scores
  std::map<TYPE_ENTITY_LITERAL_ID,double>* node_score = nullptr;

  // build
  for (auto root_id: *root_candidate_ids->getList()) {
    root_candidate.insert(root_id);
  }
#ifdef TOPK_DEBUG_INFO
  cout<<"ROOT has"<< root_candidate.size()<<" ids"<<endl;
#endif
  if(has_coefficient)
  {
    node_score = new std::map<TYPE_ENTITY_LITERAL_ID,double>();
    for (auto root_id:root_candidate) {
      string v = env->kv_store->getLiteralByID(root_id).substr(1);
      (*node_score)[root_id] = coefficient*GetScore(v, *env->ss);
    }
  }

  std::vector<std::map<TYPE_ENTITY_LITERAL_ID ,OrderedList*>> descendents_FRs;
  descendents_FRs.reserve(child_type_num);

  std::set<TYPE_ENTITY_LITERAL_ID > deleted_root_ids;
  auto descendents_num = root_plan->descendents_.size();
  for (decltype(descendents_num) descendent_i = 0;descendent_i<descendents_num;descendent_i++ )
  {
    auto descendent_plan = root_plan->descendents_[descendent_i];
    auto tree_edges_plan = root_plan->tree_edges_[descendent_i];
    auto descendent_iterators = GenerateIteratorNode(root_var, descendent_plan->var_id,tree_edges_plan,
                                                     root_candidate,deleted_root_ids, descendent_plan, env);
    descendents_FRs.push_back(std::move(descendent_iterators));
  }

  // assembling all FQ into one FR
  // constructing children' FQs
  auto child_fqs = AssemblingFrOw(root_candidate, node_score,env->k,descendents_FRs);

  auto layer_child_fqs = make_shared<set<OrderedList*>>();
  env->global_iterators->push_back(layer_child_fqs);

  auto fr = new FRIterator();
  // constructing parents' FRs
  for(auto root_id:child_fqs) {
    auto fq_pointer = root_id.second;
      fr->Insert(env->k,fq_pointer);
      layer_child_fqs->insert(fq_pointer);
  }
  auto layer_iterators = make_shared<set<OrderedList*>>();
  layer_iterators->insert(fr);
  env->global_iterators->push_back(layer_iterators);

#ifdef TOPK_DEBUG_INFO
  cout<<"ROOT has"<< root_candidate.size()<<"ids "<<", FR has "<<child_fqs.size()<<" FQs"<<endl;
#endif
  return fr;
}

/**
 * equals mapping[x].add(y), add some initial part
 * @param x
 * @param y
 * @param mapping
 */
void inline TopKUtil::AddRelation(TYPE_ENTITY_LITERAL_ID x,
                                  TYPE_ENTITY_LITERAL_ID y,
                                  map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID>> &mapping) {
  auto xit = mapping.find(x);
  if(xit==mapping.end())
    mapping[x] = std::set<TYPE_ENTITY_LITERAL_ID>();
  mapping[x].insert(y);
}

/**
 * Assembling FQ Iterators by FR mappings
 */
std::map<TYPE_ENTITY_LITERAL_ID,FQIterator*> inline TopKUtil::AssemblingFrOw(set<TYPE_ENTITY_LITERAL_ID> &fq_ids,
                                                                             std::map<TYPE_ENTITY_LITERAL_ID,double>* node_scores, int k,
                                                                             vector<std::map<TYPE_ENTITY_LITERAL_ID, OrderedList *>> &descendents_FRs) {
  int child_type_num = descendents_FRs.size();
  std::map<TYPE_ENTITY_LITERAL_ID,FQIterator*> id_fqs;
  for(auto fq_id:fq_ids)
  {
    double score=0.0;
    if(node_scores!= nullptr)
      score = (*node_scores)[fq_id];
    auto fq = new FQIterator(k,fq_id,child_type_num,score);
    // assembling fq
    for(int i =0;i<child_type_num;i++)
      fq->Insert(descendents_FRs[i][fq_id]);
    id_fqs[fq_id] = fq;
    fq->TryGetNext(k);
#ifdef SHOW_SCORE
  cout<<"FQ["<<fq_id<<"], the min score are "<<fq->pool_[0].cost<<endl;
#endif
  }
  return id_fqs;
}

/**
 * extend one edge in the query graph
 * parent - children - descendants
 * This function only deals with a parent - children relation in the query
 * graph. Generate children and the call GenerateIteratorNode again.
 * @param parent_var
 * @param child_var
 * @param parent_var_candidates
 * @param child_tree_node
 * @param env
 * @return each parent id and their corresponding FRs
 */
std::map<TYPE_ENTITY_LITERAL_ID ,OrderedList*> TopKUtil::GenerateIteratorNode(int parent_var,int child_var,std::shared_ptr<TopKUtil::TreeEdge> tree_edges_,
                                                                              std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,
                                                                              std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
                                                                              TopKTreeNode *child_tree_node,Env *env) {
  std::map<TYPE_ENTITY_LITERAL_ID ,OrderedList*> parents_FRs;
  auto child_type_num = child_tree_node->descendents_.size();
  auto coefficient_it = env->coefficients->find(env->basic_query->getVarName(child_var));
  bool has_coefficient = coefficient_it != env->coefficients->end();
  double coefficient = has_coefficient ? (*coefficient_it).second : 0.0;
  auto layer_child_iterators = make_shared<set<OrderedList*>>();
  env->global_iterators->push_back(layer_child_iterators);


  std::set<TYPE_ENTITY_LITERAL_ID> deleted_parent_ids_this_child;

  // record the mapping between parent and children
  std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID >> parent_child;
  std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID >> child_parent;

  std::set<TYPE_ENTITY_LITERAL_ID> child_candidates;

  auto &predicates_constant = tree_edges_->predicate_constant_;
  auto &predicate_ids = tree_edges_->predicate_ids_;
  auto &directions = tree_edges_->directions_;

  for (auto it = parent_var_candidates.begin(); it != parent_var_candidates.end(); it++)
  {
    auto parent_id = *it;
    auto id_list_ptr = make_shared<IDList>();

    // because we arrange constants edge first, we can use IDList
    // first and then use IDListWithAppending later when meeting
    // variable predicate.
    auto id_list_prepared = false;

    shared_ptr<IDListWithAppending> appending_list_ptr = nullptr;
    auto appending_list_prepared = false;

    auto two_var_edges_num = predicate_ids.size();
    decltype(two_var_edges_num) i;

    for (i = 0; i < two_var_edges_num; i++)
    {
      // encounter a case where we must use IDListWithAppending
      // need to transfer to IDListWithAppending
      if(!appending_list_prepared && !predicates_constant[i])
      {
        if(id_list_prepared)
          appending_list_ptr = make_shared<IDListWithAppending>(*id_list_ptr);
        appending_list_prepared = true;
      }

      TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
      TYPE_ENTITY_LITERAL_ID edge_list_len;

      // use IDList
      if (predicates_constant[i]){
        if(directions[i] == EdgeDirection::OUT)
        {
          env->kv_store->getobjIDlistBysubIDpreID(parent_id,
                                                  predicate_ids[i],
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  env->txn);
        }
        else if(directions[i] == EdgeDirection::IN)
        {
          env->kv_store->getsubIDlistByobjIDpreID(parent_id,
                                                  predicate_ids[i],
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  env->txn);
        }
        else
          throw string("unknown EdgeDirection");
        UpdateIDList(id_list_ptr,
                     edge_candidate_list,
                     edge_list_len,
                     id_list_prepared);

        if(!id_list_prepared)
        {
          if(env->id_caches->find(child_var)!=env->id_caches->end())
          {
            auto caches_ptr = (*(env->id_caches->find(child_var))).second;
            id_list_ptr->intersectList(caches_ptr->getList()->data(), caches_ptr->size());
          }
        }

        id_list_prepared =true;
      }
      else // use IDListWithAppending
      {
        if(directions[i] == EdgeDirection::OUT)
        {
          env->kv_store->getpreIDobjIDlistBysubID(parent_id,
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  env->txn);
          UpdateIDListWithAppending(appending_list_ptr,edge_candidate_list,edge_list_len,
                                    2,appending_list_prepared,1);
        }
        else if(directions[i] == EdgeDirection::IN)
        {
          env->kv_store->getpreIDsubIDlistByobjID(parent_id,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             env->txn);
          UpdateIDListWithAppending(appending_list_ptr,edge_candidate_list,edge_list_len,
                                    2,appending_list_prepared,1);
        }
        else
          throw string("unknown EdgeDirection");

        if(!appending_list_prepared)
        {
          if(env->id_caches->find(child_var)!=env->id_caches->end())
          {
            auto caches_ptr = (*(env->id_caches->find(child_var))).second;
            appending_list_ptr->Intersect(caches_ptr->getList()->data(), caches_ptr->size(),1,0);
          }
        }

        appending_list_prepared = true;
      }
      delete[] edge_candidate_list;

    }

    if (id_list_ptr->empty()) {
      deleted_parent_ids_this_child.insert(parent_id);
      continue;
    }

    // write into the relationship
    parent_child[parent_id] = std::set<TYPE_ENTITY_LITERAL_ID>();
    auto p_it = parent_child.find(parent_id);
    for (auto child_id:*id_list_ptr->getList()) {
      p_it->second.insert(child_id);
      AddRelation(child_id, parent_id, child_parent);
      child_candidates.insert(child_id);
    }

  }

  for (auto deleted_id:deleted_parent_ids_this_child) {
    parent_var_candidates.erase(deleted_id);
    deleted_parents.insert(deleted_id);
  }
  // calculate children's scores
  std::map<TYPE_ENTITY_LITERAL_ID,double>* node_score = nullptr;
  if(has_coefficient)
  {
    node_score = new std::map<TYPE_ENTITY_LITERAL_ID,double>();
    for (auto child_id:child_candidates) {
      string v = env->kv_store->getLiteralByID(child_id).substr(1);
      (*node_score)[child_id] = coefficient*GetScore(v, *env->ss);
    }
  }

  // Return OW iterators
  if (child_type_num == 0)
  {
#ifdef SHOW_SCORE
  cout<<"var["<<child_var<<"] has no child, constructing OW iterators"<<endl;
#endif
    std::vector<TYPE_ENTITY_LITERAL_ID> children_ids;
    std::vector<double> children_scores;
    for(auto parent_id:parent_var_candidates)
    {
#ifdef SHOW_SCORE
      cout<<"parent var["<<parent_id<<"] has "<<parent_child[parent_id].size()<<" child, its OW "<<endl;
#endif
      auto ow = new OWIterator();
      for(auto child_id:parent_child[parent_id])
      {
#ifdef SHOW_SCORE
        cout<<"["<<child_id<<"]"<<" "<<(*node_score)[child_id];
#endif
        children_ids.push_back(child_id);
        children_scores.push_back((*node_score)[child_id]);
      }
#ifdef SHOW_SCORE
      cout<<endl;
      //cout<<parent_id<<"'s OW ["<<child_var<<"]";
#endif
      ow->Insert(env->k, children_ids, children_scores);
      parents_FRs[parent_id] = ow;
      layer_child_iterators->insert(ow);
      children_ids.clear();
      children_scores.clear();
    }
    return std::move(parents_FRs);
  }

  // Return FR iterators
  std::set<TYPE_ENTITY_LITERAL_ID > deleted_children;

  std::vector<std::map<TYPE_ENTITY_LITERAL_ID ,OrderedList*>> descendents_FRs;
  descendents_FRs.reserve(child_type_num);

  auto descendents_num = child_tree_node->descendents_.size();

  for (decltype(descendents_num) descendent_i=0;descendent_i<descendents_num;descendent_i++)
  {
    auto descendent_plan = child_tree_node->descendents_[descendent_i];
    auto descendent_tree_edges_plan = child_tree_node->tree_edges_[descendent_i];
    auto descendent_iterators = GenerateIteratorNode(child_var,
                                                     descendent_plan->var_id,
                                                     descendent_tree_edges_plan,
                                                     child_candidates,
                                                     deleted_children,
                                                     descendent_plan, env);
    descendents_FRs.push_back(std::move(descendent_iterators));
  }


  // constructing children' FQs
  auto child_fqs = AssemblingFrOw(child_candidates, node_score,env->k,descendents_FRs);

  for(const auto& id_fq_pair:child_fqs)
  {
    layer_child_iterators->insert(id_fq_pair.second);
  }


  // calculate which parent to be deleted
  for(auto deleted_child_id:deleted_children)
  {
    for(auto parent_id:child_parent[deleted_child_id])
    {
      parent_child[parent_id].erase(deleted_child_id);
      if(parent_child[parent_id].size()==0)
      {
        deleted_parents.insert(parent_id);
        parent_var_candidates.erase(parent_id);
      }
    }
  }

  for(auto deleted_parent:deleted_parents)
    parent_var_candidates.erase(deleted_parent);


  // constructing parents' FRs
  // also add predicate information into FRs
  for(auto parent_id:parent_var_candidates) {
    auto fr = new FRIterator();
    parents_FRs[parent_id] = fr;
    for(auto child_id:parent_child[parent_id])
      fr->Insert(env->k,child_fqs[child_id]);
    fr->TryGetNext(env->k);
  }

  return std::move(parents_FRs);

}

/**
 * Reorder the TreeEdge structure, making
 * edges with constant edges pop first
 */
void TopKUtil::TreeEdge::ChangeOrder() {
  // [0,const_end) stores constant edge
  // [const_end,end) stores variable edge
  size_t const_end = 0;
  auto edges_num = predicate_constant_.size();
  for(decltype(edges_num) i=0;i<edges_num;i++)
  {
    // const
    if(predicate_constant_[i] == true)
    {
      // exchange i with const_end
      if(i!=const_end)
      {
        std::swap(predicate_constant_[i],predicate_constant_[const_end]);
        std::swap(predicate_ids_[i],predicate_ids_[const_end]);
        std::swap(directions_[i],directions_[const_end]);
      }
      const_end++;
    }
  }
}
