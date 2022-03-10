//
// Created by Yuqi Zhou on 2021/9/11.
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
                                          bool minus_signed)
{

  if(comp_tree_node->children.size()==0)
  {
    if(minus_signed)
      coefficients[comp_tree_node->val] = -1.0;
    else
      coefficients[comp_tree_node->val] = 1.0;
#ifdef TOPK_DEBUG_INFO
    std::cout<<"Node:"<<comp_tree_node->val<<" 1.0"<<std::endl;
#endif
    return;
  }
#ifdef TOPK_DEBUG_INFO
  std::cout<<"Node:"<<comp_tree_node->val<<" ";
  if(comp_tree_node->lchild!=nullptr)
    cout<<comp_tree_node->lchild->val<<" ";
  if(comp_tree_node->rchild!=nullptr)
    cout<<comp_tree_node->rchild->val<<std::endl;
#endif
  // if both of the child are leaves
  // e.g
  // case    A                B
  //         *                +
  //      /     \          /     \
  //    ?x      1.0      ?x       ?y
  if( comp_tree_node->children.size()==2&&
      comp_tree_node->children[0].children.size()==0&&
      comp_tree_node->children[1].children.size()==0)
  {
    // case B
    if(comp_tree_node->oprt == "+" ||comp_tree_node->oprt == "-"  )
    {
      GetVarCoefficientsTreeNode(&(comp_tree_node->children[0]), coefficients,minus_signed);
      GetVarCoefficientsTreeNode(&(comp_tree_node->children[1]), coefficients,comp_tree_node->oprt == "-");
      return;
    }

    if (comp_tree_node->children[0].val.at(0) == '?')
    {
      auto &val_string = comp_tree_node->children[1].val;//.substr(1);
      coefficients[comp_tree_node->children[0].val] = get<1>(Util::checkGetNumericLiteral(val_string));
      if(minus_signed)
        coefficients[comp_tree_node->children[0].val]  = -coefficients[comp_tree_node->children[0].val] ;
    }
    else // 0.1 * ?x
    {
      auto &val_string = comp_tree_node->children[0].val;
      coefficients[comp_tree_node->children[1].val] = get<1>(Util::checkGetNumericLiteral(val_string));
      if(minus_signed)
        coefficients[comp_tree_node->children[1].val]  = - coefficients[comp_tree_node->children[1].val];
    }

    return;
  }

  // the left should be either a leaf or a triangle
  if(comp_tree_node->children.size()>=1) {
    if(comp_tree_node->children.size()==2)
      GetVarCoefficientsTreeNode(&(comp_tree_node->children[0]), coefficients, minus_signed);
    else // case expression: " -?x "
      GetVarCoefficientsTreeNode(&(comp_tree_node->children[0]), coefficients, comp_tree_node->oprt=="-");
  }

  if(comp_tree_node->children.size()==2)
    GetVarCoefficientsTreeNode(&(comp_tree_node->children[1]), coefficients,comp_tree_node->oprt=="-");

}

/**
 * Suppose the SPARQL is simple, base unit is like '0.1 * ?x'
 * And the expression is like the sum of base units
 * @param order
 * @return
 */
std::shared_ptr<std::map<std::string,double>> TopKUtil::getVarCoefficients(QueryTree::Order order)
{
#ifdef TOPK_DEBUG_INFO
  order.comp_tree_root->print(0);
#endif
  auto r= make_shared<std::map<std::string,double>>();
  GetVarCoefficientsTreeNode(&order.comp_tree_root, *r);

#ifdef TOPK_DEBUG_INFO
  std::cout<<"VarCoefficients:"<<std::endl;
  for(const auto& pair:*r)
  {
    std::cout<<pair.first<<"\t"<<pair.second<<std::endl;
  }
#endif
  return r;
}

void TopKUtil::UpdateIDList(shared_ptr<IDList> valid_id_list, unsigned* id_list, unsigned id_list_len, bool id_list_prepared)
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
                                        unsigned id_list_len, unsigned  one_record_len,
                                        bool id_list_prepared, unsigned main_key_position)
{
  if(id_list_prepared)
    ids_appending->Intersect(id_list, id_list_len,one_record_len,main_key_position);
  else
    ids_appending = make_shared<IDListWithAppending>(id_list, id_list_len, one_record_len, main_key_position);
}

/**
 * equals mapping[x].add(y), add some initial part
 * @param x
 * @param y
 * @param mapping
 */
void TopKUtil::AddRelation(TYPE_ENTITY_LITERAL_ID x,
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
    if(!delete_it)
      (*node_score)[child_id] = coefficient *get<1>(check_result);
    if(parent_child!= nullptr && delete_it)
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

std::set<TYPE_ENTITY_LITERAL_ID> // child_candidates
TopKUtil::ExtendTreeEdge(std::set<TYPE_ENTITY_LITERAL_ID>& parent_var_candidates, int child_var,
                        std::set<TYPE_ENTITY_LITERAL_ID>& deleted_parents,
                        std::map<TYPE_ENTITY_LITERAL_ID, shared_ptr<IDListWithAppending>  > &parent_child,
                        std::map<TYPE_ENTITY_LITERAL_ID, std::set<TYPE_ENTITY_LITERAL_ID> > &child_parent,
                        std::shared_ptr<TopKPlanUtil::TreeEdge> tree_edges_,
                        Env *env) {
  std::set<TYPE_ENTITY_LITERAL_ID> child_candidates;
  std::set<TYPE_ENTITY_LITERAL_ID> deleted_parent_ids_this_child;
  auto &predicates_constant = tree_edges_->predicate_constant_;
  auto &predicate_ids = tree_edges_->predicate_ids_;
  auto &directions = tree_edges_->directions_;

  for (auto it = parent_var_candidates.begin(); it != parent_var_candidates.end(); it++) {
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
    for (i = 0; i < two_var_edges_num; i++) {
      // early terminate
      if (id_list_prepared && id_list_ptr->size() == 0)
        break;
      if (appending_list_prepared && appending_list_ptr->Size() == 0)
        break;

      // encounter a case where we must use IDListWithAppending
      // need to transfer to IDListWithAppending
      if (!appending_list_prepared && !predicates_constant[i])
        if (id_list_prepared) {
          appending_list_ptr = make_shared<IDListWithAppending>(*id_list_ptr);
          appending_list_prepared = true;
        }

      TYPE_ENTITY_LITERAL_ID *edge_candidate_list;
      TYPE_ENTITY_LITERAL_ID edge_list_len;

#ifdef TOPK_DEBUG_TREE_EXTEND_INFO
      cout << "\t \t " <<env->kv_store->getEntityByID(parent_id)<<" ";
      if(predicates_constant[i])
      cout<<"-"<<env->kv_store->getPredicateByID(predicate_ids[i]);
      else
        cout<<"-"<<"?";
      if(directions[i] == TopKPlanUtil::EdgeDirection::OUT)
        cout<<"->";
      else
        cout<<"<-";
      cout<<" "<<env->bgp_query->get_var_name_by_id(child_var)<<endl;
#endif

      // use IDList
      if (predicates_constant[i]) {
        if (directions[i] == TopKPlanUtil::EdgeDirection::OUT) {
          env->kv_store->getobjIDlistBysubIDpreID(parent_id,
                                                  predicate_ids[i],
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  env->txn);
        } else if (directions[i] == TopKPlanUtil::EdgeDirection::IN) {
          env->kv_store->getsubIDlistByobjIDpreID(parent_id,
                                                  predicate_ids[i],
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  env->txn);
        } else
          throw string("unknown EdgeDirection");
        UpdateIDList(id_list_ptr,
                     edge_candidate_list,
                     edge_list_len,
                     id_list_prepared);

        if (!id_list_prepared) {
          if (env->id_caches->find(child_var) != env->id_caches->end()) {
            auto caches_ptr = (*(env->id_caches->find(child_var))).second;
            id_list_ptr->intersectList(caches_ptr->getList()->data(), caches_ptr->size());
          }
        }

        id_list_prepared = true;
      } else // use IDListWithAppending
      {
        if (directions[i] == TopKPlanUtil::EdgeDirection::OUT) {
          env->kv_store->getpreIDobjIDlistBysubID(parent_id,
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  env->txn);
          UpdateIDListWithAppending(appending_list_ptr, edge_candidate_list, edge_list_len / 2,
                                    2, appending_list_prepared, 1);
        } else if (directions[i] == TopKPlanUtil::EdgeDirection::IN) {
          env->kv_store->getpreIDsubIDlistByobjID(parent_id,
                                                  edge_candidate_list,
                                                  edge_list_len,
                                                  true,
                                                  env->txn);
          UpdateIDListWithAppending(appending_list_ptr, edge_candidate_list, edge_list_len / 2,
                                    2, appending_list_prepared, 1);
        } else
          throw string("unknown EdgeDirection");

        if (!appending_list_prepared) {
          if (env->id_caches->find(child_var) != env->id_caches->end()) {
            auto caches_ptr = (*(env->id_caches->find(child_var))).second;
            appending_list_ptr->Intersect(caches_ptr->getList()->data(), caches_ptr->size(), 1, 0);
          }
        }

        appending_list_prepared = true;
      }
      delete[] edge_candidate_list;

    }

    // deleting the wrong parent
    if (appending_list_prepared) {
      if (appending_list_ptr->Empty()) {
        deleted_parent_ids_this_child.insert(parent_id);
        continue;
      }
    } else if (id_list_prepared) {
      if (id_list_ptr->empty()) {
        deleted_parent_ids_this_child.insert(parent_id);
        continue;
      }
    } else {
      deleted_parent_ids_this_child.insert(parent_id);
      continue;
    }

    if (!appending_list_prepared)
      appending_list_ptr = make_shared<IDListWithAppending>(*id_list_ptr);
    // write into the relationship
    parent_child[parent_id] = appending_list_ptr;

    auto child_id_it = appending_list_ptr->contents_->cbegin();
    auto child_id_end = appending_list_ptr->contents_->cend();
    while (child_id_it != child_id_end) {
      AddRelation(child_id_it->first, parent_id, child_parent);
      child_candidates.insert(child_id_it->first);
      child_id_it++;
    }
  }

  for (auto deleted_id:deleted_parent_ids_this_child) {
    parent_var_candidates.erase(deleted_id);
    deleted_parents.insert(deleted_id);
  }
  auto child_id_caches_it = env->id_caches->find(child_var);
  if(child_id_caches_it!=env->id_caches->end())
  {
    deleted_parent_ids_this_child.clear();
    auto child_id_caches = child_id_caches_it->second;
    auto child_id_begin_it = child_id_caches->begin();
    auto child_id_end_it = child_id_caches->end();
    set<TYPE_ENTITY_LITERAL_ID> deleted_children;
    for (const auto &child_parents_pair: child_parent) {
      auto child_id = child_parents_pair.first;
      if (std::binary_search(child_id_begin_it, child_id_end_it, child_id))
        continue;
      deleted_children.insert(child_id);
      for (auto parent_id : child_parents_pair.second) {
        parent_child[parent_id]->Erase(child_id);
        if (parent_child[parent_id]->Empty())
          deleted_parent_ids_this_child.insert(parent_id);
      }
    }

    for (auto deleted_child : deleted_children)
      child_parent.erase(deleted_child);
  }

  for (auto deleted_id:deleted_parent_ids_this_child) {
    parent_var_candidates.erase(deleted_id);
    deleted_parents.insert(deleted_id);
  }

  return child_candidates;
}
