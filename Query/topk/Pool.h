//
// Created by Yuqi Zhou on 2021/7/4.
//

#ifndef TOPK_DPB_POOL_H_
#define TOPK_DPB_POOL_H_
#include "../../Util/Util.h"

class OrderedList;

namespace DPB {

struct element{
  union Identity {
    TYPE_ENTITY_LITERAL_ID node;
    OrderedList* pointer;
  };
  Identity identity;
  unsigned int index;
  double cost;
  bool operator<(const element &other){return this->cost < other.cost;}
};

using  Pool = std::vector<element>;
using  sequence =  std::vector<unsigned int>;

// For FQ heap
struct FqElement{
  sequence seq;
  double cost;
  bool operator<(const FqElement &other){return this->cost < other.cost;}
};

using  ePool = std::vector<FqElement>;
};

#endif //TOPK_DPB_POOL_H_
