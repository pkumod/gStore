//
// Created by Yuqi Zhou on 2021/7/4.
//

#ifndef TOPK_DPB_DYNAMICTRIE_H_
#define TOPK_DPB_DYNAMICTRIE_H_
#include "../../../Util/Util.h"
#include "Pool.h"

namespace DPB {

/**
 * The basic unit of the DynamicTrie.
 * If we insert two sequences into the Trie: 0-0-0, 0-1-0:
 * the trie will have 6 TrieEntry:
 * root
 * |
 * 0
 * | \
 * 0 1
 * | |
 * 0 0
 * the upper 4 TrieEntry is inner node
 * the bottom 2 TrieEntry if leaf node
 * count means how many its parents has been met
 */
struct TrieEntry {
  std::vector<TrieEntry*> nexts;
  // useful only in leaf node
  size_t count;
  TrieEntry(size_t default_k):nexts(default_k, nullptr),count(-1){};
};

class DynamicTrie {
 private:
  TrieEntry* root;
  int depth_;
  int default_k_;
 public:
  DPB::TrieEntry* newEntry(int k);
  explicit DynamicTrie(int depth,int k );
  ~DynamicTrie();
  void deleteEntry(TrieEntry *trie_entry,int depth);
  TrieEntry* insert(const sequence &seq);
  bool detect(const sequence &seq);
};



}
#endif //TOPK_DPB_DYNAMICTRIE_H_
