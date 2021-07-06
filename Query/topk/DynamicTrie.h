//
// Created by Yuqi Zhou on 2021/7/4.
//

#ifndef TOPK_DPB_DYNAMICTRIE_H_
#define TOPK_DPB_DYNAMICTRIE_H_
#include "../../Util/Util.h"
class DynamicTrie {
 private:
  int depth_;
 public:
  void insert(const std::vector<unsigned int> &seq);
  bool detect(const std::vector<unsigned int> &seq);
};

#endif //TOPK_DPB_DYNAMICTRIE_H_
