//
// Created by Yuqi Zhou on 2021/7/4.
//

#ifndef TOPK_DPB_POOL_H_
#define TOPK_DPB_POOL_H_
#include "../../Util/Util.h"

namespace dpb {

struct elements{
  TYPE_ENTITY_LITERAL_ID node;
  unsigned int index;
  double cost;
};

using  Pool = std::vector<elements>;

};

#endif //TOPK_DPB_POOL_H_
