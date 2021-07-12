//
// Created by Yuqi Zhou on 2021/7/10.
//

#include "TopKUtil.h"


std::size_t TopKTreeSearchPlan::count_depth(std::map<int, set<int>>& neighbours,
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
TopKTreeSearchPlan::TopKTreeSearchPlan(BasicQuery *basic_query, Statistics *statistics, QueryTree::Order expression)
{
  auto total_vars_num = basic_query->getVarNum();

  auto var_descriptors_ = std::make_shared<std::vector<VarDescriptor>>();
  // When id < total_var_num, the var in 'var_infos' maps exactly the id in BasicQuery
  for(int i = 0;i<total_vars_num; i++) {
    var_descriptors_->emplace_back(VarDescriptor::VarType::EntityType ,basic_query->getVarName(i),basic_query);
  }

  std::map<int,set<int>> neighbours;
  int max_depth_root = -1;
  std::size_t max_depth = 0;
  for(int i = 0;i< total_vars_num ; i++)
  {
    // need_retrieve = degree > 1
    // so this line check so-called satellites nodes
    if((*var_descriptors_)[i].IsSatellite())
      continue;
    // count the its depth if regard node i as root
    auto tree_depth = this->count_depth(neighbours,i,total_vars_num);
    if(tree_depth>max_depth)
    {
      max_depth_root = i;
      max_depth = tree_depth;
    }
  }

  // construct the query tree
  bool*  vars_used_vec = new bool[total_vars_num];
  auto root_id = max_depth_root;
  vars_used_vec[root_id] = true;
  std::stack<TopKTreeNode*> explore_id;
  TopKTreeNode* r = new TopKTreeNode;
  r->var_id = root_id;
  explore_id.push(r);

  while(!explore_id.empty())
  {
    auto now_node = explore_id.top();

    for(const auto& child_id: neighbours[now_node->var_id])
    {
      if(vars_used_vec[child_id])
        continue;
      vars_used_vec[child_id] = true;
      auto child_tree = new TopKTreeNode;
      child_tree->var_id = child_id;
      now_node->descendents_.push_back(child_tree);
    }
    explore_id.pop();
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

    for(const auto child: tree_node->descendents_)
    {
      to_delete.push(child);
    }
    to_delete.pop();
    delete tree_node;
  }
}

void TopKUtil::getVarCoefficientsTreeNode(QueryTree::CompTreeNode *comp_tree_node,
                                                                  std::map<std::string,double>& coefficients,
                                                                  stringstream &ss)
{
  // if both of the child are leaves
  if(comp_tree_node->lchild->lchild==NULL&&
      comp_tree_node->lchild->lchild==NULL&&
      comp_tree_node->lchild->lchild==NULL&&
      comp_tree_node->lchild->lchild==NULL)
  {
    if (comp_tree_node->lchild->val.at(0) == '?') // ?x * 0.1
    {
      double coef;
      ss<<comp_tree_node->rchild->val;
      ss>>coef;
      ss.clear();
      coefficients[comp_tree_node->lchild->val] = coef;
    }
    else // 0.1 * ?x
    {
      double coef;
      ss<<comp_tree_node->lchild->val;
      ss>>coef;
      ss.clear();
      coefficients[comp_tree_node->rchild->val] = coef;
    }
  }
  else
  {
    getVarCoefficientsTreeNode(comp_tree_node->lchild,coefficients,ss);
    getVarCoefficientsTreeNode(comp_tree_node->rchild,coefficients,ss);
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
  getVarCoefficientsTreeNode(order.comp_tree_root,r,ss);
  return std::move(r);
}

OrderedList *TopKUtil::BuildIteratorTree(KVstore* kv_store,BasicQuery* basic_query,
                                         const shared_ptr<TopKTreeSearchPlan> &tree_search_plan,
                                         map<TYPE_ENTITY_LITERAL_ID, OrderedList *> &iterators,
                                         map<TYPE_ENTITY_LITERAL_ID, TreeResultSet *> &elements_lists,
                                         map<std::string, double> &coefficients) {
  return nullptr;
}
