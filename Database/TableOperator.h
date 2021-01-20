/*=============================================================================
# Filename: TableOperator.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "../Util/Util.h"

#ifndef GSTORELIMITK_DATABASE_TABLEOPERATOR_H_
#define GSTORELIMITK_DATABASE_TABLEOPERATOR_H_

/* only consider extend 1 node per step */
enum class JoinMethod{s2p,s2o,p2s,p2o,o2s,o2p,so2p,sp2o,po2s};

class IntermediateResult{
 private:
 public:
  shared_ptr<list<shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>>>> values_;
  shared_ptr<map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>> id_to_position_;
  shared_ptr<map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>> position_to_id_;
  shared_ptr<vector<bool>> dealed_triple_;
  IntermediateResult(shared_ptr<map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>> id_to_position,
                     shared_ptr<map<TYPE_ENTITY_LITERAL_ID,TYPE_ENTITY_LITERAL_ID>> position_to_id,
                     shared_ptr<vector<bool>> dealed_triple);
  IntermediateResult();
  static shared_ptr<IntermediateResult> JoinTwoStructure(shared_ptr<IntermediateResult> result_a,shared_ptr<IntermediateResult> result_b,
      shared_ptr< vector<TYPE_ENTITY_LITERAL_ID> > public_variables);
};


class EdgeInfo{
 public:
  TYPE_ENTITY_LITERAL_ID s_;
  TYPE_ENTITY_LITERAL_ID p_;
  TYPE_ENTITY_LITERAL_ID o_;
  EdgeInfo(TYPE_ENTITY_LITERAL_ID s,TYPE_ENTITY_LITERAL_ID p,TYPE_ENTITY_LITERAL_ID o,JoinMethod method):
      s_(s),o_(o),p_(p),join_method_(method){
  }
  JoinMethod join_method_;
  TYPE_ENTITY_LITERAL_ID getVarToFilter();
};

class EdgeConstantInfo{
 public:
  bool s_constant_;
  bool p_constant_;
  bool o_constant_;
  EdgeConstantInfo(bool s_constant, bool p_constant, bool o_constant):
      s_constant_(s_constant), p_constant_(p_constant), o_constant_(o_constant){
  }
  bool ConstantToVar(EdgeInfo edge_info);
};

/* Extend One Table, add a new Node.
 * The Node can have a candidate list */
class OneStepJoinNode{
 public:
  TYPE_ENTITY_LITERAL_ID node_to_join_;
  shared_ptr<vector<EdgeInfo>> edges_;
  shared_ptr<vector<EdgeConstantInfo>> edges_constant_info_;
  void ChangeOrder(shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> already_in){
    struct ConstantNumberInfo
    {
      int constant_number;
      int already_in_number;
      int old_index;
    };
    set<TYPE_ENTITY_LITERAL_ID> in_vars;
    for(auto in_id:*already_in)
      in_vars.insert(in_id);

    vector<ConstantNumberInfo> number_info_vec;

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
};


/* Join Two Table on Public Variables*/
struct OneStepJoinTable{
  shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> public_variables_;
  OneStepJoinTable(shared_ptr<vector<TYPE_ENTITY_LITERAL_ID>> public_variables):public_variables_(public_variables){};
  OneStepJoinTable(){this->public_variables_=make_shared<vector<TYPE_ENTITY_LITERAL_ID>>();};
};

class OneStepJoin{
 public:
  shared_ptr<OneStepJoinNode> join_node_;
  shared_ptr<OneStepJoinTable> join_table_;
  shared_ptr<OneStepJoinNode> edge_filter_; // GenFilter & EdgeCheck use this filed
  enum class JoinType{JoinNode,JoinTable,GenFilter,EdgeCheck} join_type_;
};

/**
 * Requirements  from cpp reference:
 * For all a, comp(a,a)==false
 * If comp(a,b)==true then comp(b,a)==false
 * if comp(a,b)==true and comp(b,c)==true then comp(a,c)==true
 */
struct IndexedRecordResultCompare
{
  bool operator()(const vector<TYPE_ENTITY_LITERAL_ID> &a, const vector<TYPE_ENTITY_LITERAL_ID> &b)
  {
    for(auto i = 0;i<a.size();i++)
    {
      if(a[i]<b[i])
        return true;
      else if(a[i]>b[i])
        return false;
    }
    /* a==b */
    return false;
  }
};

#endif //GSTORELIMITK_DATABASE_TABLEOPERATOR_H_
