//
// Created by 37584 on 2021/3/28.
//
#include "../../Util/Util.h"
#include "./TopKStructure.h"
#include "../../Query/IDList.h"
#include "../../KVstore/KVstore.h"
#ifndef GSTORELIMITK_QUERY_TOPKSTRUCTURE_TWIGPATTERNTOPK_H_
#define GSTORELIMITK_QUERY_TOPKSTRUCTURE_TWIGPATTERNTOPK_H_

/**
 * This Class is the implement of
 *  Gang, G. , and R. Chirkova . "Efficient algorithms for exact ranked twig-pattern matching over graphs."
 *  Acm Sigmod International Conference on Management of Data ACM, 2008.
 */

int ParentNum(NodeSequencePtr ori_sequence_)
{
  int t = 0;
  for(const auto& position_ele:*ori_sequence_)
    if(position_ele>1)
      t++;
  return t;
}

class NodeIterator{
 public:
  // the 'k' in the global 'top-k'
  int k_;
  virtual void GetFirst(){};
  virtual void GetNext(){};
  /**
   * @param element_i: the element index
   * @return returns the element[i].cost - element[i-1].cost
   */
  virtual double delta_cost(int element_i){return 0;};

 protected:
  NodeIterator() = default;
  int descendant_size_;
  std::vector<FRIPoolElementPtr> i_pool_;
};

class DynamicTrie{
 private:
  class TrieEntry{
   public:
    std::vector<std::shared_ptr<TrieEntry>> pointers_;
    union content{
      int c_; // element id in this layer
      int counter_; // the last layer, used to record how many times its parent appeared
    }body;
    TrieEntry()=default;
    TrieEntry(int c){
      this->body.c_ = c;
    };
  };
  std::shared_ptr<TrieEntry> trie_entry_ptr_;
  int invalid_occurence_ = -1;
 public:
  int& insert(std::shared_ptr<LatticeSequenceWithScore> seq);
  int& search(std::shared_ptr<LatticeSequenceWithScore> seq);
  bool detect(std::shared_ptr<LatticeSequenceWithScore> seq);
};

class FQiterator{

 public:
  TYPE_ENTITY_LITERAL_ID node_v_; // the node id in KVstore
  double node_cost_;
  int k_;
  int descendant_size_;
  std::vector<SequencePoolPtr> i_pools_;
  SequencePoolPtr e_pool_;

  MinMaxHeapLatticeSeq queue_;

  std::vector<std::shared_ptr<NodeIterator>> descendants_;
  DynamicTrie trie_;

  FQiterator(TYPE_ENTITY_LITERAL_ID node_v,int k):node_v_(node_v),k_(k){};
  FQiterator(TYPE_ENTITY_LITERAL_ID node_v):node_v_(node_v){};

  FQiterator(){
  };
  double delta_cost(int descendant_i,int element_i);
  void GetFirst();
  void GetNext();
  void AddDescendant(shared_ptr<NodeIterator> descent_it);
  bool nextIPoolElement(int descendant_i,int element_i);
  FQEPoolElementPtr Convert(LatticeSequenceWithScorePtr lattice);
};


/*
 * A FRiterator has some FQiterator as children
 * */
class FRiterator:public NodeIterator{

  FRIPoolQueue queue_;
  std::vector<std::shared_ptr<FQiterator>> FQ_descendants_;
  DynamicTrie trie_;

 public:
  FRiterator(): NodeIterator(), queue_(){};

  void ReserveSpace(int capcity){this->FQ_descendants_.reserve(capcity);};
  void AddFQ(std::shared_ptr<FQiterator> FQ_it){this->FQ_descendants_.push_back(FQ_it);};
  void GetFirst() override;

  void GetNext() override;

  bool nextEPoolElement(int descendant_i,int element_i);
  void Insert(std::shared_ptr<FQiterator> FQ_node);
  double delta_cost(int descendant_i,int element_i);

};

class OneWayIterator:public NodeIterator{

 public:
  OneWayIterator(KVstore kv_store,shared_ptr<IDList>);
  OneWayIterator(std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> ids,std::shared_ptr<std::vector<double>> scores);
  void GetFirst() override;

  void GetNext() override;
  double delta_cost(int element_i);

};
#endif //GSTORELIMITK_QUERY_TOPKSTRUCTURE_TWIGPATTERNTOPK_H_
