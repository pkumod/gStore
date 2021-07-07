//
// Created by Yuqi Zhou on 2021/7/4.
//

#ifndef TOPK_DPB_DYNAMICTRIE_H_
#define TOPK_DPB_DYNAMICTRIE_H_
#include "../../Util/Util.h"
#include "Pool.h"

namespace DPB {

union TrieEntry {
  TrieEntry* next;
  size_t count;
};

class DynamicTrie {
 private:
  TrieEntry* root;
  int depth_;
 public:
  explicit DynamicTrie(int depth);
  ~DynamicTrie();
  void insert(const sequence &seq);
  bool detect(const sequence &seq);
};



}
#endif //TOPK_DPB_DYNAMICTRIE_H_
