//
// Created by Yuqi Zhou on 2021/7/4.
//

#include "DynamicTrie.h"

/**
 * alloc A TrieEntry*, which has k children
 * @param k ,children number
 * @return the pointer to the TrieEntry instance
 */
DPB::TrieEntry* DPB::DynamicTrie::newEntry(int k) {
  auto r = new TrieEntry;
  r->nexts = new TrieEntry*[k];
  memset(r->nexts ,0,k*sizeof(TrieEntry*));
  return r;
}

/**
 * union TrieEntry {
  TrieEntry** nexts;
  size_t count;
};
 */

/**
 * build a DynamicTrie
 * @param depth  the depth of this Trie, the same as child type number of FQ
 * @param k
 */
DPB::DynamicTrie::DynamicTrie(int depth,int k ):depth_(depth),k_(k) {
  this->root = newEntry(k);
}

void DPB::DynamicTrie::deleteEntry(DPB::TrieEntry *trie_entry, int depth) {
  if(depth != this->depth_-1)
    for(int i=0;i<this->k_;i++)
      deleteEntry(trie_entry->nexts[i],depth+1);
  delete[] trie_entry;
}

DPB::DynamicTrie::~DynamicTrie() {
  // recursive delete
  deleteEntry(this->root,0);
}

DPB::TrieEntry* DPB::DynamicTrie::insert(const DPB::sequence &seq) {
  TrieEntry* p = this->root;
  TrieEntry* p_next;
  bool new_created = false;
  for(int i=0;i<depth_-1;i++)
  {
    p_next = p->nexts[seq[i-1]];
    if(p_next == NULL) {
      p_next = newEntry(this->k_);
      new_created = true;
    }
  }
  if(new_created) {
    int father_count = std::count_if(seq.begin(),
                                     seq.end(),
                                     [](decltype(*seq.begin()) x) { return x != 1; }
    );
    p_next->count = father_count;
  }
  return p_next;
}

bool DPB::DynamicTrie::detect(const DPB::sequence &seq) {
  auto p_next = this->insert(seq);
  p_next->count-- ;
  return p_next->count==0;
}


