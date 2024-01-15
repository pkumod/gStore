/*=============================================================================
# Filename: TableOperator.h
# Author: Yuqi Zhou
# Mail: zhouyuqi@pku.edu.cn
=============================================================================*/
#ifndef GSTORELIMITK_DATABASE_RESULTTRIGGER_H_
#define GSTORELIMITK_DATABASE_RESULTTRIGGER_H_

#include "../Util/Util.h"


class ResultTrigger{
  int estimate_upper_bound;
  int estimate_lower_bound;
 public:
  bool RollbackTrigger(size_t new_size) {return false;}
};


#endif //GSTORELIMITK_DATABASE_RESULTTRIGGER_H_
