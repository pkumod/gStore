//
// Created by Yuqi Zhou on 2021/7/10.
//

#include "TopKUtil.h"


std::size_t TopKTreeSearchPlan::CountDepth(std::map<int, set<int>>& neighbours,
                                           TYPE_ENTITY_LITERAL_ID root_id,
                                           std::size_t total_vars_num) {
  bool*  vars_used_vec = new bool[total_vars_num];
  std::size_t *  vars_depth = new std::size_t [total_vars_num];
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


/**
 * Choose A shallowest tree to do top-k query
 *
 * @param basic_query
 * @param statistics
 */
TopKTreeSearchPlan::TopKTreeSearchPlan(BasicQuery *basic_query, Statistics *statistics, QueryTree::Order expression,
                                       shared_ptr<map<TYPE_ENTITY_LITERAL_ID,shared_ptr<IDList>>> id_caches)
{
  auto total_vars_num = basic_query->getVarNum();

  auto var_descriptors_ = std::make_shared<std::vector<VarDescriptor>>();
  // When id < total_var_num, the var in 'var_infos' maps exactly the id in BasicQuery
  for(int i = 0;i<total_vars_num; i++) {
    var_descriptors_->emplace_back(VarDescriptor::VarType::EntityType ,basic_query->getVarName(i),basic_query);
  }

  std::map<int,set<int>> neighbours;
  for(int i = 0;i< total_vars_num ; i++)
  {

    neighbours[i] = set<int>();
    auto degree = basic_query->getVarDegree(i);

#ifdef TOPK_DEBUG_INFO
    std::cout<<"["<<i<<"]:";
#endif

    for(int j=0;j<degree;j++) {
      auto nei_id = basic_query->getEdgeNeighborID(i, j);
      if(nei_id<0)
        continue;

#ifdef TOPK_DEBUG_INFO
        std::cout<<nei_id<<" ";
#endif
      neighbours[i].insert(nei_id);
    }

#ifdef TOPK_DEBUG_INFO
    std::cout<<std::endl;
#endif

  }
  int min_score_root = -1;
  auto min_score = DBL_MAX;
  for(int i = 0;i< total_vars_num ; i++)
  {
    // need_retrieve = degree > 1
    // so this line check so-called satellites nodes
    if((*var_descriptors_)[i].IsSatellite())
      continue;
    if(id_caches->size()&&
      id_caches->find((*var_descriptors_)[i].basic_query_id)==id_caches->end())
      continue;
    // count the its depth if regard node i as root
    auto tree_depth = this->CountDepth(neighbours, i, total_vars_num);
    auto candidates_size =  (*id_caches)[(*var_descriptors_)[i].basic_query_id]->size();
    double score = tree_depth * candidates_size;
    if(score<min_score)
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
  memset(vars_used_vec,0,sizeof(vars_used_vec));
  auto root_id = min_score_root;
  vars_used_vec[root_id] = true;
  std::stack<TopKTreeNode*> explore_id;
  TopKTreeNode* r = new TopKTreeNode;
  r->var_id = root_id;
  explore_id.push(r);

  while(!explore_id.empty())
  {
    auto now_node = explore_id.top();
    explore_id.pop();
#ifdef TOPK_DEBUG_INFO
    cout << "pop " << now_node->var_id<<" children num:"<<neighbours[now_node->var_id].size()<<endl;
#endif

    for(const auto& child_id: neighbours[now_node->var_id])
    {
      if(vars_used_vec[child_id])
        continue;
      vars_used_vec[child_id] = true;
      auto child_tree = new TopKTreeNode;
      child_tree->var_id = child_id;
      now_node->descendents_.push_back(child_tree);
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
#ifdef TOPK_DEBUG_INFO
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
  if(comp_tree_node->lchild->lchild==NULL&&comp_tree_node->lchild->rchild==NULL&&
      comp_tree_node->rchild->lchild==NULL&&comp_tree_node->rchild->rchild==NULL)
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
std::map<std::string,double> TopKUtil::getVarCoefficients(QueryTree::Order order)
{
  stringstream ss;
  std::map<std::string,double> r;
  GetVarCoefficientsTreeNode(order.comp_tree_root, r, ss);

#ifdef TOPK_DEBUG_INFO
  //std::cout<<"TreeStructure:"<<std::endl;
  //order.comp_tree_root->print(0);
  std::cout<<"VarCoefficients:"<<std::endl;
  for(const auto& pair:r)
  {
    std::cout<<pair.first<<"\t"<<pair.second<<std::endl;
  }
#endif
  return std::move(r);
}

void TopKUtil::UpdateIDList(const shared_ptr<IDList>& valid_id_list, unsigned* id_list, unsigned id_list_len,bool id_list_prepared)
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

FRIterator *TopKUtil::BuildIteratorTree(const shared_ptr<TopKTreeSearchPlan> &tree_search_plan,Env *env){

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
  for (auto descendent_plan:root_plan->descendents_) {
    auto descendent_iterators = GenerateIteratorNode(root_var, descendent_plan->var_id, root_candidate,deleted_root_ids, descendent_plan, env);
    descendents_FRs.push_back(std::move(descendent_iterators));
  }

  // assembling all FQ into one FR
  // constructing children' FQs
  auto child_fqs = AssemblingFrOw(root_candidate, node_score,env->k,descendents_FRs);

  auto fr = new FRIterator();
  // constructing parents' FRs
  for(auto root_id:child_fqs) {
      fr->Insert(env->k,root_id.second);
  }
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
#ifdef TOPK_DEBUG_INFO
  cout<<"FQ["<<fq_id<<"], the min score are "<<fq->pool_[0].cost<<endl;
#endif
  }
  return id_fqs;
}

/**
 * extend one edge in the query graph
 * parent - children - descendants
 *
 * @param parent_var
 * @param child_var
 * @param parent_var_candidates
 * @param child_tree_node
 * @param env
 * @return each parent id and their corresponding FRs
 */
std::map<TYPE_ENTITY_LITERAL_ID ,OrderedList*> TopKUtil::GenerateIteratorNode(int parent_var,int child_var,
                                                                              std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,
                                                                              std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
                                                                              TopKTreeNode *child_tree_node,Env *env) {
  std::map<TYPE_ENTITY_LITERAL_ID ,OrderedList*> parents_FRs;
  auto child_type_num = child_tree_node->descendents_.size();
  auto coefficient_it = env->coefficients->find(env->basic_query->getVarName(child_var));
  bool has_coefficient = coefficient_it != env->coefficients->end();
  double coefficient = has_coefficient ? (*coefficient_it).second : 0.0;

  int separator_num;

  // [0,separator_num) are the predicates parent to child
  // [separator_num,end) are the predicates from child to parent
  std::vector<TYPE_PREDICATE_ID> predicates;

  // parent -> child
  auto parent_degree = env->basic_query->getVarDegree(parent_var);
  for (int edge_i =0;edge_i<parent_degree;edge_i++) {
    if(env->basic_query->getEdgeNeighborID(parent_var,edge_i)!=child_var)
      continue;
    if(env->basic_query->getEdgeType(parent_var,edge_i) == Util::EDGE_IN)
      continue;
    auto edge_predicate = env->basic_query->getEdgePreID(parent_var, edge_i);
    predicates.push_back(edge_predicate);
  }
  separator_num = predicates.size();

  // child  -> parent
  auto child_degree = env->basic_query->getVarDegree(child_var);
  for (int edge_i =0;edge_i<child_degree;edge_i++) {
    if(env->basic_query->getEdgeNeighborID(child_var,edge_i)!=parent_var)
      continue;
    if(env->basic_query->getEdgeType(child_var,edge_i) == Util::EDGE_IN)
      continue;
    auto edge_predicate = env->basic_query->getEdgePreID(child_var, edge_i);
    predicates.push_back(edge_predicate);
  }

  std::set<TYPE_ENTITY_LITERAL_ID> deleted_parent_ids_this_child;

  // record the mapping between parent and children
  std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID >> parent_child;
  std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID >> child_parent;

  std::set<TYPE_ENTITY_LITERAL_ID> child_candidates;
  for (auto it = parent_var_candidates.begin(); it != parent_var_candidates.end(); it++) {
    auto parent_id = *it;
    auto record_candidate_list = make_shared<IDList>();
    auto record_candidate_prepared = false;

    for (int i = 0; i < predicates.size(); i++) {
      auto predicate_id = predicates[i];
      TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
      TYPE_ENTITY_LITERAL_ID edge_list_len;
      if (i < separator_num) {
        if (predicate_id != -1)
          env->kv_store->getobjIDlistBysubIDpreID(parent_id,
                                                  predicate_id,
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  env->txn);
        else
          env->kv_store->getobjIDlistBysubID(parent_id,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             env->txn);
      } else {
        if (predicate_id != -1)
          env->kv_store->getsubIDlistByobjIDpreID(parent_id,
                                                  predicate_id,
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  env->txn);
        else
          env->kv_store->getobjIDlistBysubID(parent_id,
                                             edge_candidate_list,
                                             edge_list_len,
                                             true,
                                             env->txn);
      }
#ifdef TOPK_DEBUG_INFO
      cout<<"direction[";
      if(i < separator_num)
        cout<<"parent -> child]";
        else
        cout<<"child -> parent]";
      cout<<"  parent id:"<<parent_id<<" "<<env->kv_store->getEntityByID(parent_id)<<" predicate id "<<predicate_id<<" "<<
      env->kv_store->getPredicateByID(predicate_id);
#endif
      UpdateIDList(record_candidate_list,
                   edge_candidate_list,
                   edge_list_len,
                   record_candidate_prepared);
#ifdef TOPK_DEBUG_INFO
      cout<<"result size "<< record_candidate_list->size()<<endl;
#endif
      delete[] edge_candidate_list;
      record_candidate_prepared = true;
    }

    if(env->id_caches->find(child_var)!=env->id_caches->end())
    {
      auto caches_ptr = (*(env->id_caches->find(child_var))).second;
      record_candidate_list->intersectList(caches_ptr->getList()->data(),caches_ptr->size());
    }
#ifdef TOPK_DEBUG_INFO
    cout<<"final result size "<< record_candidate_list->size()<<endl;
#endif
    if (record_candidate_list->empty()) {
#ifdef TOPK_DEBUG_INFO
      cout<<"parent var["<<parent_var<<"] delete "<< parent_id<<endl;
#endif
      deleted_parent_ids_this_child.insert(parent_id);
      continue;
    }

    // write into the relationship
    parent_child[parent_id] = std::set<TYPE_ENTITY_LITERAL_ID>();
    auto p_it = parent_child.find(parent_id);
    for (auto child_id:*record_candidate_list->getList()) {
      p_it->second.insert(child_id);
      AddRelation(child_id, parent_id, child_parent);
      child_candidates.insert(child_id);
    }

  }

  for (auto deleted_id:deleted_parent_ids_this_child) {
    parent_var_candidates.erase(deleted_id);
    deleted_parents.insert(deleted_id);
  }
#ifdef TOPK_DEBUG_INFO
  if(has_coefficient)
  cout<<" var has coefficient:"<<coefficient<<endl;
#endif
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
#ifdef TOPK_DEBUG_INFO
  cout<<"var["<<child_var<<"] has no child, constructing OW iterators"<<endl;
#endif
    std::vector<TYPE_ENTITY_LITERAL_ID> children_ids;
    std::vector<double> children_scores;
    for(auto parent_id:parent_var_candidates)
    {
#ifdef TOPK_DEBUG_INFO
      cout<<"parent var["<<parent_id<<"] has "<<parent_child[parent_id].size()<<" child, its OW "<<endl;
#endif
      auto ow = new OWIterator();
      for(auto child_id:parent_child[parent_id])
      {
#ifdef TOPK_DEBUG_INFO
        cout<<" "<<(*node_score)[child_id];
#endif
        children_ids.push_back(child_id);
        children_scores.push_back((*node_score)[child_id]);
      }
#ifdef TOPK_DEBUG_INFO
      cout<<endl;
      //cout<<parent_id<<"'s OW ["<<child_var<<"]";
#endif
      ow->Insert(env->k, children_ids, children_scores);
      parents_FRs[parent_id] = ow;

      children_ids.clear();
      children_scores.clear();
    }
    return std::move(parents_FRs);
  }

  // Return FR iterators
  std::set<TYPE_ENTITY_LITERAL_ID > deleted_children;

  std::vector<std::map<TYPE_ENTITY_LITERAL_ID ,OrderedList*>> descendents_FRs;
  descendents_FRs.reserve(child_type_num);

  for (auto descendent_plan:child_tree_node->descendents_) {
    auto descendent_iterators = GenerateIteratorNode(child_var, descendent_plan->var_id, child_candidates,deleted_children, descendent_plan, env);
    descendents_FRs.push_back(std::move(descendent_iterators));
  }


  // constructing children' FQs
  auto child_fqs = AssemblingFrOw(child_candidates, node_score,env->k,descendents_FRs);

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

  // constructing parents' FRs
  for(auto parent_id:parent_var_candidates) {
    auto fr = new FRIterator();
    parents_FRs[parent_id] = fr;
    for(auto child_id:parent_child[parent_id])
      fr->Insert(env->k,child_fqs[child_id]);
  }

  return std::move(parents_FRs);

}

