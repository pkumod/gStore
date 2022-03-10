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
  return new TrieEntry(k);
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
DPB::DynamicTrie::DynamicTrie(int depth,int k ): depth_(depth), default_k_(k) {
  this->root = newEntry(k);
}

/**
 * Recursive Delete
 * @param trie_entry
 * @param depth
 */
void DPB::DynamicTrie::deleteEntry(DPB::TrieEntry *trie_entry, int depth) {
  for(unsigned int i=0;i<trie_entry->nexts.size();i++)
    if(trie_entry->nexts[i] != nullptr)
      deleteEntry(trie_entry->nexts[i],depth+1);
  delete trie_entry;
}

DPB::DynamicTrie::~DynamicTrie() {
  // recursive delete
  deleteEntry(this->root,0);
}

/**
 * if seq not exist:
 * Insert a sequence and return the pointer to the TrieEntry
 * if exist:
 * return the pointer to the old TrieEntry
 * @param seq the inserted sequence
 * @return  the pointer to the TrieEntry
 */
DPB::TrieEntry* DPB::DynamicTrie::insert(const DPB::sequence &seq) {
  TrieEntry* p = this->root;
  TrieEntry* p_next;
  bool new_created = false;
  for(int i=0;i<depth_;i++)
  {
    auto i_th_element = seq[i];

    // alloc new entries
    while(i_th_element>=p->nexts.size())
    {
      p->nexts.push_back(nullptr);
    }

    p_next = p->nexts[seq[i]];
    if(p_next == nullptr) {
      p_next = newEntry(this->default_k_);
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


