//
// Created by Yuqi Zhou on 2021/7/4.
//

#ifndef TOPK_DPB_ORDEREDLIST_H_
#define TOPK_DPB_ORDEREDLIST_H_
#include "../../Util/Util.h"
#include "Pool.h"
#include "MinMaxHeap.hpp"
#include "DynamicTrie.h"
enum class OrderedListType{UnDefined,FR,OW,FQ};

// This structure is designed for
// selectinhg top-k min element
class OrderedList{
 public:
  DPB::Pool pool_;
  virtual OrderedListType Type(){return OrderedListType::UnDefined;};
  virtual void TryGetNext(int k)=0;
};

class FRIterator:OrderedList {
 private:
  minmax::MinMaxHeap<DPB::element> queue_;
 public:
  FRIterator() = default;
  OrderedListType Type() override {return OrderedListType::FR;};
  void TryGetNext(int k) override;

  // Insert a bulk of FQ iterator, all the same type
  void Insert(int k,std::vector<OrderedList*> FQ_iterators);

  double DeltaCost(OrderedList* node_pointer, int index);
  bool NextEPoolElement(int k,OrderedList* node_pointer,unsigned int index);
};

class OWIterator:OrderedList{
 private:
  // sorting right after building, so don't need a heap to get top-k
  // the top-k result already in the pool
 public:
  OWIterator() = default;
  OrderedListType Type() override {return OrderedListType::OW;};
  void TryGetNext(int k)override;

  // Insert a bulk of gStore Node ids and their scores
  void Insert(int k,const std::vector<TYPE_ENTITY_LITERAL_ID>& ids, const std::vector<double>& scores);

};

// FQ may have cost itself
class FQIterator:OrderedList{
 private:
  double node_score_;
  minmax::MinMaxHeap<DPB::FqElement> queue_;
  std::vector<OrderedList*> FR_OW_iterators;
  DPB::ePool e_pool_;
  DPB::DynamicTrie dynamic_trie_;
 public:
  TYPE_ENTITY_LITERAL_ID node_id_;
  explicit FQIterator(int k,TYPE_ENTITY_LITERAL_ID node_id,int child_type_num,double node_score):
  node_id_(node_id), dynamic_trie_(k,child_type_num),node_score_(node_score)
  {this->FR_OW_iterators.reserve(child_type_num);};
  OrderedListType Type() override {return OrderedListType::FQ;};
  void TryGetNext(int k) override;
  // Insert a bulk of FR or OW iterators
  // inserting one certain type each time each time
  // certain type [i] specified by it's the i-th child of its father
  void Insert(std::vector<OrderedList*> FR_OW_iterators);
  bool NextEPoolElement(int k,OrderedList* node_pointer,unsigned int index);
  double DeltaCost(OrderedList* FR_OW_iterator, int index);
};

#endif //TOPK_DPB_ORDEREDLIST_H_
