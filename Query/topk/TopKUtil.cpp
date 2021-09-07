//
// Created by Yuqi Zhou on 2021/7/10.
//

#include "TopKUtil.h"

void TopKUtil::CalculatePosVarMappingNode(TopKTreeNode* top_k_tree_node,
                                          shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>> pos_var_mapping)
{
  auto now_position = pos_var_mapping->size();
  (*pos_var_mapping)[now_position] = top_k_tree_node->var_id;
  auto descendent_num = top_k_tree_node->descendents_.size();
  auto ow_num = top_k_tree_node->descendents_ow_num_;
  decltype(descendent_num) i = 0;
  for(;i<ow_num;i++) {
    auto ow_descendent = top_k_tree_node->descendents_[i];
    auto tree_edges_const = top_k_tree_node->tree_edges_[i]->predicate_constant_;
    auto tree_edges_id = top_k_tree_node->tree_edges_[i]->predicate_ids_;
    auto edge_size = tree_edges_const.size();
    for(decltype(edge_size) j =0 ;j<edge_size;j++)
      if(!tree_edges_const[j])
        (*pos_var_mapping)[pos_var_mapping->size()] = tree_edges_id[j];
    CalculatePosVarMappingNode(ow_descendent, pos_var_mapping);
  }
  for(;i<descendent_num;i++) {
    auto fr_descendent = top_k_tree_node->descendents_[i];
    auto tree_edges_const = top_k_tree_node->tree_edges_[i]->predicate_constant_;
    auto tree_edges_id = top_k_tree_node->tree_edges_[i]->predicate_ids_;
    auto edge_size = tree_edges_const.size();
    for(decltype(edge_size) j =0 ;j<edge_size;j++)
      if(!tree_edges_const[j])
        (*pos_var_mapping)[pos_var_mapping->size()] = tree_edges_id[j];
    CalculatePosVarMappingNode(fr_descendent, pos_var_mapping);
  }

}

shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>
TopKUtil::CalculatePosVarMapping(shared_ptr<TopKSearchPlan> search_plan)
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

  if(comp_tree_node->lchild==nullptr&&comp_tree_node->rchild==nullptr)
  {
    coefficients[comp_tree_node->val] = 1.0;
#ifdef TOPK_DEBUG_INFO
    std::cout<<"Node:"<<comp_tree_node->val<<" 1.0"<<std::endl;
    return;
#endif
  }
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
    for(unsigned i = 0; i < id_list_len; i++)
      valid_id_list->addID(id_list[i]);
  }
}

void TopKUtil::UpdateIDListWithAppending(shared_ptr<IDListWithAppending> &ids_appending, unsigned* id_list,
                                         unsigned id_list_len,unsigned  one_record_len,
                                         bool id_list_prepared,unsigned main_key_position)
{
  if(id_list_prepared)
    ids_appending->Intersect(id_list, id_list_len,one_record_len,main_key_position);
  else
    ids_appending = make_shared<IDListWithAppending>(id_list, id_list_len, one_record_len, main_key_position);
}

/**
 * Build Iterators for top k search.
 * @return the final FRIterator
 */
FRIterator *TopKUtil::BuildIteratorTree(const shared_ptr<TopKSearchPlan> tree_search_plan, Env *env){

  auto root_plan = tree_search_plan->tree_root_;
  auto root_var = root_plan->var_id;

  // from top to down, build the structure
  auto root_candidate_ids = (*env->id_caches)[root_var];

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
  for(auto root_id_fq:child_fqs) {
    auto root_id = root_id_fq.first;
    auto fq_pointer = root_id_fq.second;
      fr->Insert(root_id,fq_pointer,empty_pre);
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

std::shared_ptr< std::map<TYPE_ENTITY_LITERAL_ID,double>>
TopKUtil::GetChildNodeScores(double coefficient,
                             std::set<TYPE_ENTITY_LITERAL_ID> &child_candidates,
                             bool has_parent,
                             std::set<TYPE_ENTITY_LITERAL_ID>* deleted_parents,
                             std::map<TYPE_ENTITY_LITERAL_ID, shared_ptr<IDListWithAppending>  > *parent_child,
                             std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID> > *child_parent,
                             Env *env)
{
  auto node_score =  make_shared< std::map<TYPE_ENTITY_LITERAL_ID,double>>();
  std::set<TYPE_ENTITY_LITERAL_ID> deleted_children;
  for (auto child_id:child_candidates) {
    bool delete_it = false;
    // this child is not valid,delete it
    if(Util::is_entity_ele(child_id))
    {
      deleted_children.insert(child_id);
      delete_it = true;
      if(!has_parent)
        continue;
    }
    auto literal_string = env->kv_store->getLiteralByID(child_id);
    pair<bool, double> check_result =  Util::checkGetNumericLiteral(literal_string);
    delete_it = !get<0>(check_result);
    (*node_score)[child_id] = coefficient *get<1>(check_result);
    if(delete_it)
    {
      auto &its_parents = (*child_parent)[child_id];
      for(auto parent_id:its_parents)
      {
        auto children_and_predicates = (*parent_child)[parent_id];
        children_and_predicates->Erase(child_id);
        if(children_and_predicates->Size()==0)
        {
          deleted_parents->insert(parent_id);
          parent_child->erase(parent_id);
        }
      }
    }
  }

  for(auto delete_child:deleted_children)
    child_candidates.erase(delete_child);
  return node_score;
}

/**
 * Assembling FQ Iterators by OW/FR mappings
 * OW first and FR last
 * @return map<FQ id,pointer to FQIterator>
 */
std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<FQIterator>>
TopKUtil::AssemblingFrOw(set<TYPE_ENTITY_LITERAL_ID> &fq_ids,
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

std::set<TYPE_ENTITY_LITERAL_ID> // child_candidates
TopKUtil::ExtendTreeEdge(std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,int child_var,
                         std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
                         std::map<TYPE_ENTITY_LITERAL_ID, shared_ptr<IDListWithAppending>  > &parent_child,
                         std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID> > &child_parent,
                         std::shared_ptr<TopKUtil::TreeEdge> tree_edges_,
                         Env *env)
{
  std::set<TYPE_ENTITY_LITERAL_ID> child_candidates;
  std::set<TYPE_ENTITY_LITERAL_ID> deleted_parent_ids_this_child;
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
      // early terminate
      if (id_list_prepared && id_list_ptr->size()==0)
        break;
      if(appending_list_prepared && appending_list_ptr->Size()==0)
        break;

      // encounter a case where we must use IDListWithAppending
      // need to transfer to IDListWithAppending
      if(!appending_list_prepared && !predicates_constant[i])
        if(id_list_prepared) {
          appending_list_ptr = make_shared<IDListWithAppending>(*id_list_ptr);
          appending_list_prepared = true;
        }


      TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
      TYPE_ENTITY_LITERAL_ID edge_list_len;

#ifdef TOPK_DEBUG_INFO
      cout << "\t \t " <<env->kv_store->getEntityByID(parent_id)<<" ";
      if(predicates_constant[i])
      cout<<"-"<<env->kv_store->getPredicateByID(predicate_ids[i]);
      else
        cout<<"-"<<"?";
      if(directions[i] == EdgeDirection::OUT)
        cout<<"->";
      else
        cout<<"<-";
      cout<<" "<<env->bgp_query->get_var_name_by_id(child_var)<<endl;
#endif

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
          UpdateIDListWithAppending(appending_list_ptr,edge_candidate_list,edge_list_len/2,
                                    2,appending_list_prepared,1);
        }
        else if(directions[i] == EdgeDirection::IN)
        {
          env->kv_store->getpreIDsubIDlistByobjID(parent_id,
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  env->txn);
          UpdateIDListWithAppending(appending_list_ptr,edge_candidate_list,edge_list_len/2,
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

    // deleting the wrong parent
    if(appending_list_prepared) {
      if (appending_list_ptr->Empty()) {
        deleted_parent_ids_this_child.insert(parent_id);
        continue;
      }
    }
    else if(id_list_prepared){
      if (id_list_ptr->empty()){
        deleted_parent_ids_this_child.insert(parent_id);
        continue;
      }
    }
    else{
      deleted_parent_ids_this_child.insert(parent_id);
      continue;
    }

    if(!appending_list_prepared)
      appending_list_ptr = make_shared<IDListWithAppending>(*id_list_ptr);
    // write into the relationship
    parent_child[parent_id] = appending_list_ptr;

    auto child_id_it = appending_list_ptr->contents_->cbegin();
    auto child_id_end = appending_list_ptr->contents_->cend();
    while(child_id_it != child_id_end) {
      AddRelation(child_id_it->first, parent_id, child_parent);
      child_candidates.insert(child_id_it->first);
      child_id_it++;
    }
  }

  for (auto deleted_id:deleted_parent_ids_this_child) {
    parent_var_candidates.erase(deleted_id);
    deleted_parents.insert(deleted_id);
  }
  return child_candidates;
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
TopKUtil::GenerateOWs(int parent_var,int child_var,std::shared_ptr<TopKUtil::TreeEdge> tree_edges_,
                      std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,
                      std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
                      Env *env)
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
                                                                               deleted_parents,parent_child,
                                                                               child_parent,tree_edges_,env);

  // calculate children's scores
  std::shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,double>> node_score =nullptr;
  if(has_coefficient)
    node_score = GetChildNodeScores(coefficient,child_candidates,true,&deleted_parents,&parent_child,
                                    &child_parent,env);

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
      children_scores.push_back((*node_score)[child_id]);
      child_it++;
    }
#ifdef SHOW_SCORE
    cout<<endl;
      //cout<<parent_id<<"'s OW ["<<child_var<<"]";
#endif

    ow->Insert( children_ids, children_scores);
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
TopKUtil::GenerateFRs(int parent_var, int child_var, std::shared_ptr<TopKUtil::TreeEdge> tree_edges_,
                      std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates,
                      std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
                      TopKTreeNode *child_tree_node, Env *env) {
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
                                                                               deleted_parents,parent_child,
                                                                               child_parent,tree_edges_,env);

  // calculate children's scores
  shared_ptr<std::map<TYPE_ENTITY_LITERAL_ID,double>> node_score = nullptr;
  if(has_coefficient)
    node_score = GetChildNodeScores(coefficient,child_candidates,true,&deleted_parents,&parent_child,
                                    &child_parent,env);



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

  for(auto deleted_parent:deleted_parents)
    parent_var_candidates.erase(deleted_parent);

  std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<FRIterator>> result;

  // constructing parents' FRs
  // also add predicate information into FRs
  for(auto parent_id:parent_var_candidates) {
    auto fr = make_shared<FRIterator>();
    auto edges_predicates = make_shared<NodeOneChildVarPredicates>();
    for(auto child_id_appending_pair: *(parent_child[parent_id])->contents_)
    {
      auto child_id = child_id_appending_pair.first;
      auto appending_list = child_id_appending_pair.second;
      fr->Insert( child_id,child_fqs[child_id],appending_list);
    }
    result[parent_id] = fr;
    fr->TryGetNext(env->k);
  }

  return result;

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
