/*=============================================================================
# Filename: TableOperator.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/

#include "../Util/Util.h"

#ifndef GSTORELIMITK_DATABASE_TABLEOPERATOR_H_
#define GSTORELIMITK_DATABASE_TABLEOPERATOR_H_

/* Extend One Table, add a new Node */
struct OneStepExtend{
  TYPE_ENTITY_LITERAL_ID s_;
  TYPE_ENTITY_LITERAL_ID p_;
  TYPE_ENTITY_LITERAL_ID o_;
  enum JoinMethod{s2po,s2p,s2o,p2so,p2s,p2o,o2sp,o2s,o2p,so2p,sp2o,po2s} join_method_;
  OneStepExtend(TYPE_ENTITY_LITERAL_ID s,TYPE_ENTITY_LITERAL_ID p,TYPE_ENTITY_LITERAL_ID o,JoinMethod method):
      s_(s),o_(o),p_(p),join_method_(method){
  }
};

/* Join Two Table on Public Variables*/
struct OneStepJoin{
  }
};

#endif //GSTORELIMITK_DATABASE_TABLEOPERATOR_H_
