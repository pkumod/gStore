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

/* This structure is designed for
  selecting limit-k elements */
struct TreeResultSet{
  TYPE_ENTITY_LITERAL_ID node_id_;
  // estimated children number, may over-estimated
  // 0 means it is the leaf node
  size_t child_num_;
  // fathers only useful during bottom to top refinement
  std::set<TreeResultSet*> fathers_;
  std::set<TreeResultSet*> child_elements_;
};

/* This structure is designed for
  selecting top-k min element */
class OrderedList{
 public:
  TreeResultSet* tree_result_set_;// only useful during constructing and refine
  DPB::Pool pool_;
  virtual OrderedListType Type(){return OrderedListType::UnDefined;};
  virtual void TryGetNext(int k)=0;
};

class FRIterator:public OrderedList {
 private:
  minmax::MinMaxHeap<DPB::element> queue_;
 public:
  FRIterator() = default;
  OrderedListType Type() override {return OrderedListType::FR;};
  void TryGetNext(int k) override;



  // Insert a bulk of FQ iterator, all the same type
  void Insert(int k,OrderedList* FQ_iterator);

  // Insert a bulk of FQ iterator, all the same type
  void Insert(int k,const std::vector<OrderedList*>& FQ_iterators);
  void Insert(int k,const std::set<OrderedList*>& FQ_iterators);

  double DeltaCost(OrderedList* node_pointer, int index);
  bool NextEPoolElement(int k, OrderedList* node_pointer, unsigned int index);
};

class OWIterator: public OrderedList{
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
class FQIterator: public OrderedList{
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
  void Insert(OrderedList* FR_OW_iterator);
  bool NextEPoolElement(int k, OrderedList* node_pointer, unsigned int index);
  double DeltaCost(OrderedList* FR_OW_iterator, int index);
};

#endif //TOPK_DPB_ORDEREDLIST_H_
