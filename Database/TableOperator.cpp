/*=============================================================================
# Filename: TableOperator.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "../Util/Util.h"
#include "TableOperator.h"
using namespace std;

std::string JoinMethodToString(JoinMethod x){
  switch (x) {
    case JoinMethod::s2p: return "s2p";
    case JoinMethod::s2o: return "s2o";
    case JoinMethod::p2s: return "p2s";
    case JoinMethod::p2o: return "p2o";
    case JoinMethod::o2s: return "o2s";
    case JoinMethod::o2p: return "o2p";
    case JoinMethod::so2p: return "so2p";
    case JoinMethod::sp2o: return "sp2o";
    case JoinMethod::po2s: return "po2s";
  }
  return "Not a JoinMethod instance";
};


/**
 * join two IntermediateResult together,
 * the result table consist of
 *  [table_a][other part of table_b]
 * @param result_a
 * @param result_b
 * @param public_variables : vars appears in table_a and table_b
 * @return
 */

shared_ptr<IntermediateResult>
IntermediateResult::JoinTwoStructure(shared_ptr<IntermediateResult> result_a,
    shared_ptr<IntermediateResult> result_b,shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> public_variables) {

  set<TYPE_ENTITY_LITERAL_ID> public_variables_set;
  for(auto id:*public_variables)
    public_variables_set.insert(id);

  decltype(result_a->position_to_var_des_) r_position_to_id;
  for(auto a_kv:*result_a->position_to_var_des_)
  {
    auto already_filled_size = r_position_to_id->size();
    (*(r_position_to_id))[already_filled_size] = a_kv.second;
  }

  for(auto b_kv:*result_b->position_to_var_des_)
  {
    if (public_variables_set.find(b_kv.second)!=public_variables_set.end())
      continue;
    auto already_filled_size = r_position_to_id->size();
    (*(r_position_to_id))[already_filled_size] = b_kv.second;
  }

  decltype(r_position_to_id) r_id_to_position;
  for(auto kv:*r_position_to_id)
  {
    (*r_id_to_position)[kv.second] = kv.first;
  }
  auto r_dealed_triple = make_shared<vector<bool>>();
  return make_shared<IntermediateResult>(r_id_to_position,r_position_to_id,r_dealed_triple);
}

IntermediateResult::IntermediateResult(
    shared_ptr<map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>> id_to_position,
shared_ptr<map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>> position_to_id,
shared_ptr<vector<bool>> dealed_triple){
  this->values_ = make_shared<list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>();
  this->var_des_to_position_ = make_shared<map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>(*id_to_position);
  this->position_to_var_des_ = make_shared<map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>(*position_to_id);
  this->dealed_triple_ = make_shared<vector<bool>>(*dealed_triple);
}

IntermediateResult::IntermediateResult(){
  this->values_ = make_shared<list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>>();
  this->var_des_to_position_ = make_shared<map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>();
  this->position_to_var_des_ = make_shared<map<TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID>>();
  this->dealed_triple_ = make_shared<vector<bool>>();
};


bool EdgeConstantInfo::ConstantToVar(EdgeInfo edge_info) {
  switch (edge_info.join_method_) {
    case JoinMethod::s2p:
    case JoinMethod::s2o:
      return this->s_constant_;
    case JoinMethod::p2s:
    case JoinMethod::p2o:
      return this->p_constant_;
    case JoinMethod::o2s:
    case JoinMethod::o2p:
      return this->o_constant_;
    case JoinMethod::so2p:
      return this->s_constant_ && this->o_constant_;
    case JoinMethod::sp2o:
      return this->s_constant_ && this->p_constant_;
    case JoinMethod::po2s:
      return this->p_constant_ && this->o_constant_;
  }
  return false;
}

TYPE_ENTITY_LITERAL_ID EdgeInfo::getVarToFilter() {
  TYPE_ENTITY_LITERAL_ID var_to_filter;
  switch (this->join_method_) {
    case JoinMethod::s2p:
      var_to_filter = this->p_;
      break;

    case JoinMethod::s2o:
      var_to_filter = this->o_;
      break;

    case JoinMethod::p2s:
      var_to_filter = this->s_;
      break;

    case JoinMethod::p2o:
      var_to_filter = this->o_;
      break;

    case JoinMethod::o2s:
      var_to_filter = this->s_;
      break;

    case JoinMethod::o2p:
      var_to_filter = this->p_;
      break;

    case JoinMethod::so2p:
      var_to_filter = this->p_;
      break;

    case JoinMethod::sp2o:
      var_to_filter = this->o_;
      break;

    case JoinMethod::po2s:
      var_to_filter = this->s_;
      break;
  }
  return var_to_filter;
}

string EdgeInfo::toString() {

  stringstream ss;
  ss<<" EdgeInfo: \t s["<<this->s_<<"] \t p["<<this->p_<<"]"<<" \t o["<<this->o_<<"]";
  string t;
  ss>>t;
  return t;
}

std::string EdgeConstantInfo::toString() {
  stringstream ss;
  vector<string> wrong_right = {"variable","constant"};
  ss<<" EdgeConstantInfo: \t s["<<wrong_right[this->s_constant_]
    <<"] \t p["<<this->p_constant_<<"]"
    <<" \t o["<<this->o_constant_<<"]";
  string t;
  ss >> t;
  return t;
}

/**
 * change order to better produce candidate
 * Top: already in and constant
 * Last: not in and variable
 * @param already_in
 */
void OneStepJoinNode::ChangeOrder(std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> already_in){
  struct ConstantNumberInfo
  {
    int constant_number;
    int already_in_number;
    int old_index;
  };
  std::set<TYPE_ENTITY_LITERAL_ID> in_vars;
  for(auto in_id:*already_in)
    in_vars.insert(in_id);

  std::vector<ConstantNumberInfo> number_info_vec;

  for(auto i =0;i<this->edges_->size();i++)
  {
    ConstantNumberInfo t;
    t.already_in_number = t.constant_number = 0;
    t.old_index = i;

    if((*this->edges_constant_info_)[i].s_constant_)
      t.constant_number += 1;
    if((*this->edges_constant_info_)[i].p_constant_)
      t.constant_number += 1;
    if((*this->edges_constant_info_)[i].o_constant_)
      t.constant_number += 1;

    if(in_vars.find((*this->edges_)[i].s_) != in_vars.end())
      t.already_in_number += 1;
    if(in_vars.find((*this->edges_)[i].p_) != in_vars.end())
      t.already_in_number += 1;
    if(in_vars.find((*this->edges_)[i].o_) != in_vars.end())
      t.already_in_number += 1;

    number_info_vec.push_back(t);
  }

  /* decreasing order */
  sort(number_info_vec.begin(),number_info_vec.end(),[](const ConstantNumberInfo& a,const ConstantNumberInfo& b){
    return ( (a.already_in_number+1) * (a.constant_number + 1)) >  ((b.already_in_number+1)* (b.constant_number + 1));
  });

  auto new_edge_info = make_shared<vector<EdgeInfo>>();
  auto new_edge_constant_info = make_shared<vector<EdgeConstantInfo>>();
  for(auto& number_info:number_info_vec)
  {
    new_edge_info->push_back((*this->edges_)[number_info.old_index]);
    new_edge_constant_info->push_back((*this->edges_constant_info_)[number_info.old_index]);
  }

  this->edges_ = new_edge_info;
  this->edges_constant_info_ = new_edge_constant_info;

}
