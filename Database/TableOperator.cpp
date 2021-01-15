/*=============================================================================
# Filename: TableOperator.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "../Util/Util.h"
#include "TableOperator.h"
using namespace std;

/**
 * join two IntermediateResult together,
 * the result table consist of
 *  [table_a][other part of table_b]
 * @param result_a
 * @param result_b
 * @param public_variables : vars appears in table_a and table_b
 * @return
 */
shared_ptr<IntermediateResult> IntermediateResult::JoinTwoStructure(shared_ptr<IntermediateResult> result_a,
    shared_ptr<IntermediateResult> result_b,shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> public_variables) {

  set<TYPE_ENTITY_LITERAL_ID> public_variables_set;
  for(auto id:*public_variables)
    public_variables_set.insert(id);

  decltype(result_a->position_to_id_) r_position_to_id;
  for(auto a_kv:*result_a->position_to_id_)
  {
    auto already_filled_size = r_position_to_id->size();
    (*(r_position_to_id))[already_filled_size] = a_kv.second;
  }

  for(auto b_kv:*result_b->position_to_id_)
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
};

bool EdgeInTableInfo::ConstantToVar(shared_ptr<EdgeInfo> edge_info) {
  switch (edge_info->join_method_) {
    case s2p:
    case s2o:
      return this->s_constant_;
    case p2s:
    case p2o:
      return this->p_constant_;
    case o2s:
    case o2p:
      return this->o_constant_;
    case so2p:
      return this->s_constant_ && this->o_constant_;
    case sp2o:
      return this->s_constant_ && this->p_constant_;
    case po2s:
      return this->p_constant_ && this->o_constant_;
  }
  return false;
}
