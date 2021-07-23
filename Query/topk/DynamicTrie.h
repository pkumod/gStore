//
// Created by Yuqi Zhou on 2021/7/4.
//

#ifndef TOPK_DPB_DYNAMICTRIE_H_
#define TOPK_DPB_DYNAMICTRIE_H_
#include "../../Util/Util.h"
#include "Pool.h"

namespace DPB {

/**
 * The basic unit of the DynamicTrie.
 * If we insert two sequences into the Trie: 1-1-1, 1-2-1:
 * the trie will have 6 TrieEntry:
 * root
 * |
 * 1
 * | \
 * 1 2
 * | |
 * 1 1
 * the upper 4 TrieEntry has filed 'nexts' with size k
 * the bottom 2 TrieEntry has filed 'count'
 * count means how many its parents has been met
 */
union TrieEntry {
  TrieEntry** nexts;
  size_t count;
};

class DynamicTrie {
 private:
  TrieEntry* root;
  int depth_;
  int k_;
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
