//
// Created by Yuqi Zhou on 2021/7/10.
//

#include "DPBTopKUtil.h"


/**
 * Build Iterators for top k search.
 * @return the final FRIterator
 */
FRIterator *DPBUtil::BuildIteratorTree(const shared_ptr<TopKSearchPlan> tree_search_plan, TopKUtil::Env *env){

  auto root_plan = tree_search_plan->tree_root_;
  auto root_var = root_plan->var_id;

  shared_ptr<IDList> root_candidate_ids(nullptr);

  // from top to down, build the structure
  if(env->id_caches->find(root_var)!=env->id_caches->end())
    root_candidate_ids = (*env->id_caches)[root_var];
  else
  {
    auto property_info = env->bgp_query->GetOccurPosition(root_var);
    auto is_literal = get<1>(property_info);
    root_candidate_ids = GetAllSubObjId(env->kv_store,env->limitID_entity,
                                        env->limitID_literal,is_literal);
  }

  set<TYPE_ENTITY_LITERAL_ID> root_candidate;
  auto coefficient_it = env->coefficients->find(env->bgp_query->get_var_name_by_id((root_var)));
  bool has_coefficient = coefficient_it != env->coefficients->end();
  double coefficient = has_coefficient? (*coefficient_it).second:0.0;

  // calculating scores
  shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,double>> node_score = nullptr;

  // build
  for (auto root_id: *root_candidate_ids->getList()) {
    root_candidate.insert(root_id);
  }
#ifdef TOPK_DEBUG_INFO
  cout<<"ROOT has"<< root_candidate.size()<<" ids"<<endl;
#endif
  if(has_coefficient)
    node_score = GetChildNodeScores(coefficient,root_candidate, false, nullptr, nullptr,
                                    nullptr,env);

  // each node's descendents are OW first and FRs last
  std::vector<std::map<TYPE_ENTITY_LITERAL_ID ,std::shared_ptr<FRIterator>>> descendents_FRs;

  std::vector<std::map<TYPE_ENTITY_LITERAL_ID, // parent id
           std::pair<std::shared_ptr<OWIterator>, // its OW
                     NodeOneChildVarPredicatesPtr>>> // predicate correspond to the OW
  descendents_OWs;

  descendents_FRs.reserve(root_plan->descendents_fr_num_);
  descendents_OWs.reserve(root_plan->descendents_ow_num_);

  std::set<TYPE_ENTITY_LITERAL_ID > deleted_root_ids;
  auto descendents_num = root_plan->descendents_.size();
  for (decltype(descendents_num) descendent_i = 0;descendent_i<descendents_num;descendent_i++ )
  {
    auto descendent_plan = root_plan->descendents_[descendent_i];
    auto tree_edges_plan = root_plan->tree_edges_[descendent_i];
    auto is_fr = descendent_plan->descendents_.size() !=0;

    if(is_fr) {
      // 分情况讨论，是FR还是OW
      auto descendent_iterators = GenerateFRs(root_var, descendent_plan->var_id, tree_edges_plan,
                                              root_candidate, deleted_root_ids, descendent_plan, env);
      descendents_FRs.push_back(std::move(descendent_iterators));
    }
    else
    {
      auto descendent_iterators = GenerateOWs(root_var, descendent_plan->var_id, tree_edges_plan,
                                              root_candidate, deleted_root_ids, env);
      descendents_OWs.push_back(std::move(descendent_iterators));
    }
  }

  // assembling all FQ into one FR
  // constructing children' FQs
  auto child_fqs = AssemblingFrOw(root_candidate, node_score,env->k,descendents_FRs,descendents_OWs);

  auto empty_pre = make_shared<OnePointPredicateVec>();
  auto fr = new FRIterator();
  // constructing parents' FRs
  for(auto& root_id_fq:child_fqs) {
    auto& root_id = root_id_fq.first;
    auto& fq_pointer = root_id_fq.second;
    fr->Insert(root_id,fq_pointer,empty_pre);
  }

#ifdef TOPK_DEBUG_INFO
  cout<<"ROOT has"<< root_candidate.size()<<"ids "<<", FR has "<<child_fqs.size()<<" FQs"<<endl;
#endif
  return fr;
}

/**
 * Assembling FQ Iterators by OW/FR mappings
 * OW first and FR last
 * @return map<FQ id,pointer to FQIterator>
 */
std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<FQIterator>>
DPBUtil::AssemblingFrOw(set<TYPE_ENTITY_LITERAL_ID> &fq_ids,
                        std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,double>> node_scores, int k,
                        vector<std::map<TYPE_ENTITY_LITERAL_ID, shared_ptr<FRIterator> >> &descendents_FRs,
                        std::vector<
                             std::map<TYPE_ENTITY_LITERAL_ID, // parent id
                                      std::pair<shared_ptr<OWIterator>, // its OW
                                                NodeOneChildVarPredicatesPtr>>> // predicate correspond to the OW
                         &descendents_OWs) {
  auto ow_size = descendents_OWs.size();
  auto fr_size = descendents_FRs.size();
  auto child_type_num =  ow_size+fr_size;
  std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<FQIterator>> id_fqs;
  for(auto fq_id:fq_ids)
  {
    double score=0.0;
    if(node_scores!= nullptr)
      score = (*node_scores)[fq_id];
    auto fq = make_shared<FQIterator>(k,fq_id,child_type_num,score);
    // assembling fq
    for(decltype(child_type_num) i =0;i<ow_size;i++)
    {
      fq->Insert(descendents_OWs[i][fq_id].first);
      fq->AddOneTypePredicate(descendents_OWs[i][fq_id].second);
    }
    for(decltype(child_type_num) i =0;i<fr_size;i++)
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
 * Generate each parents' ids corresponding OW iterators
 * @param parent_var
 * @param child_var
 * @param tree_edges_
 * @param parent_var_candidates
 * @param deleted_parents
 * @param child_tree_node
 * @param env
 * @return
 */
std::map<TYPE_ENTITY_LITERAL_ID, // parent id
         std::pair<std::shared_ptr<OWIterator>, // its OW
                   NodeOneChildVarPredicatesPtr>> // predicate correspond to the OW
DPBUtil::GenerateOWs(int parent_var, int child_var, std::shared_ptr<TopKPlanUtil::TreeEdge> tree_edges_,
                     std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,
                     std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
                     TopKUtil::Env *env)
{
  auto coefficient_it = env->coefficients->find(env->bgp_query->get_var_name_by_id((child_var)));
  bool has_coefficient = coefficient_it != env->coefficients->end();
  double coefficient = has_coefficient ? (*coefficient_it).second : 0.0;

  // record the mapping between parent and children
  // IDListWithAppending not only records the children
  // but also records the edges between them
  std::map<TYPE_ENTITY_LITERAL_ID, shared_ptr<IDListWithAppending>  > parent_child;
  std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID> > child_parent;

  std::set<TYPE_ENTITY_LITERAL_ID> child_candidates = TopKUtil::ExtendTreeEdge(parent_var_candidates, child_var,
                                                                              deleted_parents, parent_child,
                                                                              child_parent, tree_edges_, env);

  // calculate children's scores
  std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,double>> node_score =nullptr;
  if(has_coefficient)
    node_score = GetChildNodeScores(coefficient,child_candidates,true,&deleted_parents,&parent_child,
                                    &child_parent,env);

  for(auto deleted_parent:deleted_parents)
    parent_var_candidates.erase(deleted_parent);

  // Return OW iterators
#ifdef SHOW_SCORE
  cout<<"var["<<child_var<<"] has no child, constructing OW iterators"<<endl;
#endif
  std::vector<TYPE_ENTITY_LITERAL_ID> children_ids;
  std::vector<double> children_scores;

  std::map<TYPE_ENTITY_LITERAL_ID, // parent id
           std::pair<shared_ptr<OWIterator>, // its OW
                     NodeOneChildVarPredicatesPtr>> // predicate correspond to the OW
                     result;
  for(auto parent_id:parent_var_candidates)
  {
#ifdef SHOW_SCORE
    cout<<"parent var["<<parent_id<<"] has "<<parent_child[parent_id].size()<<" child, its OW "<<endl;
#endif
    auto ow = make_shared<OWIterator>();
    auto child_it = parent_child[parent_id]->contents_->cbegin();
    auto child_end =  parent_child[parent_id]->contents_->cend();
    auto ow_predicates = make_shared<NodeOneChildVarPredicates>();
    while(child_it != child_end)
    {
      const auto child_id = child_it->first;
      auto predicate_vec = child_it->second;
#ifdef SHOW_SCORE
      cout<<"["<<child_id<<"]"<<" "<<(*node_score)[child_id];
#endif
      children_ids.push_back(child_id);
      (*ow_predicates)[child_id] = predicate_vec;
      if(has_coefficient)
        children_scores.push_back((*node_score)[child_id]);
      child_it++;
    }
#ifdef SHOW_SCORE
    cout<<endl;
      //cout<<parent_id<<"'s OW ["<<child_var<<"]";
#endif
    if(has_coefficient)
      ow->Insert( children_ids, children_scores);
    else
      ow->Insert( children_ids);
    result[parent_id] = make_pair(ow,ow_predicates);
    children_ids.clear();
    children_scores.clear();
  }
  return result;

}


/**
 * extend one edge (and the subtree induced) in the query graph
 * parent - children - descendants
 * This function only deals with a parent - children relation in the query
 * graph. Generate children and the call GenerateFRs to prone the
 * generated candidates ,and assemble them as FQ in the end.
 * @param parent_var
 * @param child_var
 * @param parent_var_candidates
 * @param child_tree_node
 * @param env
 * @return each parent id and their children corresponding FRs/OWs
 */
std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<FRIterator>>
DPBUtil::GenerateFRs(int parent_var, int child_var, std::shared_ptr<TopKPlanUtil::TreeEdge> tree_edges_,
                     std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,
                     std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
                     TopKTreeNode *child_tree_node, TopKUtil::Env *env) {
  auto coefficient_it = env->coefficients->find(env->bgp_query->get_var_name_by_id(child_var));
  bool has_coefficient = coefficient_it != env->coefficients->end();
  double coefficient = has_coefficient ? (*coefficient_it).second : 0.0;
  std::set<TYPE_ENTITY_LITERAL_ID> deleted_parent_ids_this_child;

  // record the mapping between parent and children
  // IDListWithAppending not only records the children
  // but also records the edges between them
  std::map<TYPE_ENTITY_LITERAL_ID, shared_ptr<IDListWithAppending>  > parent_child;
  std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID> > child_parent;

  std::set<TYPE_ENTITY_LITERAL_ID> child_candidates = TopKUtil::ExtendTreeEdge(parent_var_candidates, child_var,
                                                                              deleted_parents, parent_child,
                                                                              child_parent, tree_edges_, env);



  // calculate children's scores
  shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,double>> node_score = nullptr;
  if(has_coefficient)
    node_score = GetChildNodeScores(coefficient,child_candidates,true,&deleted_parents,&parent_child,
                                    &child_parent,env);

  for(auto deleted_parent:deleted_parents)
    parent_var_candidates.erase(deleted_parent);

  // Return FR iterators
  std::set<TYPE_ENTITY_LITERAL_ID > deleted_children;

  std::vector<std::map<TYPE_ENTITY_LITERAL_ID ,std::shared_ptr<FRIterator>>> descendents_FRs;
  descendents_FRs.reserve(child_tree_node->descendents_fr_num_);

  std::vector<
      std::map<TYPE_ENTITY_LITERAL_ID, // parent id
               std::pair<std::shared_ptr<OWIterator>, // its OW
                         NodeOneChildVarPredicatesPtr>>> // predicate correspond to the OW
  descendents_OWs;
  descendents_OWs.reserve(child_tree_node->descendents_ow_num_);

  auto descendents_num = child_tree_node->descendents_.size();

  for (decltype(descendents_num) descendent_i=0;descendent_i<descendents_num;descendent_i++)
  {
    auto descendent_plan = child_tree_node->descendents_[descendent_i];
    auto descendent_tree_edges_plan = child_tree_node->tree_edges_[descendent_i];
    auto is_fr = descendent_plan->descendents_.size() !=0;
    if(is_fr) {
      auto descendent_iterators = GenerateFRs(child_var,
                                              descendent_plan->var_id,
                                              descendent_tree_edges_plan,
                                              child_candidates,
                                              deleted_children,
                                              descendent_plan, env);
      descendents_FRs.push_back(std::move(descendent_iterators));
    }
    else
    {
      auto descendent_iterators = GenerateOWs(child_var,
                                              descendent_plan->var_id,
                                              descendent_tree_edges_plan,
                                              child_candidates,
                                              deleted_children,
                                              env);
      descendents_OWs.push_back(std::move(descendent_iterators));
    }
  }

  // constructing children' FQs
  auto child_fqs = AssemblingFrOw(child_candidates, node_score,env->k,descendents_FRs,descendents_OWs);

  // calculate which parent to be deleted
  for(auto deleted_child_id:deleted_children)
  {
    for(auto parent_id:child_parent[deleted_child_id])
    {
      parent_child[parent_id]->Erase(deleted_child_id);
      if(parent_child[parent_id]->MainKeyNum()==0)
      {
        deleted_parents.insert(parent_id);
        parent_var_candidates.erase(parent_id);
      }
    }
  }

  std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<FRIterator>> result;

  // constructing parents' FRs
  // also add predicate information into FRs
  for(auto parent_id:parent_var_candidates) {
    auto fr = make_shared<FRIterator>();
    auto edges_predicates = make_shared<NodeOneChildVarPredicates>();
    for(auto& child_id_appending_pair: *(parent_child[parent_id])->contents_)
    {
      auto& child_id = child_id_appending_pair.first;
      auto& appending_list = child_id_appending_pair.second;
      fr->Insert( child_id,child_fqs[child_id],appending_list);
    }
    result[parent_id] = fr;
    fr->TryGetNext(env->k);
  }

  return result;

}

shared_ptr<IDList>
DPBUtil::GetAllSubObjId(KVstore *kv_store_,
                        unsigned int limitID_entity_,
                        unsigned int limitID_literal_,
                        bool need_literal)
{
  auto result = make_shared<IDList>();
  for (TYPE_PREDICATE_ID i = 0; i < limitID_entity_; ++i) {
    auto entity_str = kv_store_->getEntityByID(i);
    if(entity_str!="")
      result->addID(i);
  }
  if(need_literal) {
    for (TYPE_PREDICATE_ID i = Util::LITERAL_FIRST_ID; i < limitID_literal_; ++i) {
      auto entity_str = kv_store_->getLiteralByID(i);
      if (entity_str != "")
        result->addID(i);
    }
  }
  return result;
}