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
 * @param k , the goal of this algorithm
 */
DPB::DynamicTrie::DynamicTrie(int depth,int k ):depth_(depth),k_(k) {
  this->root = newEntry(k);
}

void DPB::DynamicTrie::deleteEntry(DPB::TrieEntry *trie_entry, int depth) {
  if(depth != this->depth_-1)
    for(int i=0;i<this->k_;i++)
      if(trie_entry->nexts[i] != nullptr)
        deleteEntry(trie_entry->nexts[i],depth+1);

  delete[] trie_entry;
}

DPB::DynamicTrie::~DynamicTrie() {
  // recursive delete
  deleteEntry(this->root,0);
}

/**
 * Insert a sequence and return the pointer to the TrieEntry
 * @param seq the inserted sequence
 * @return  the pointer to the TrieEntry
 */
DPB::TrieEntry* DPB::DynamicTrie::insert(const DPB::sequence &seq) {
  TrieEntry* p = this->root;
  TrieEntry* p_next;
  bool new_created = false;
  for(int i=0;i<depth_;i++)
  {
    p_next = p->nexts[seq[i]];
    if(p_next == NULL) {
      p_next = newEntry(this->k_);
      p->nexts[seq[i]] = p_next;
      new_created = true;
    }
    p = p_next;
  }
  if(new_created) {
    auto father_count = static_cast<size_t>(std::count_if(seq.begin(),
                                     seq.end(),
                                     [](decltype(*seq.begin()) x) { return x != 0; })
    );
    p_next->count = father_count;
  }
  return p_next;
}

/**
 * detect if a sequence have been inserted into the Trie
 * If so, return whether all its parents have been popped
 * @param seq
 * @return  whether all its parents have been popped
 */
bool DPB::DynamicTrie::detect(const DPB::sequence &seq) {
  auto p_next = this->insert(seq);
  p_next->count-- ;
  return p_next->count==0;
}


