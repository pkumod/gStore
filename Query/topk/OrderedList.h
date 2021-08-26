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

using OnePointPredicateVec= std::vector<TYPE_ENTITY_LITERAL_ID>;
using OnePointPredicatePtr = std::shared_ptr<OnePointPredicateVec>;
using NodeOneChildVarPredicates = std::vector<OnePointPredicatePtr>;
using NodeOneChildVarPredicatesPtr = std::shared_ptr<NodeOneChildVarPredicates>;


/* This structure is designed for
  selecting top-k min element */
class OrderedList{
 public:
  DPB::Pool pool_;
  // predicates_vec[i] is the predicates of child[i]
  // FQs and FRs will use this field.
  virtual OrderedListType Type(){return OrderedListType::UnDefined;};
  virtual void TryGetNext(unsigned int k)=0;
  virtual void GetResult(int i_th,std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record)=0;
};

class FRIterator:public OrderedList {
 private:
  NodeOneChildVarPredicatesPtr type_predicates_;
  minmax::MinMaxHeap<DPB::element> queue_;
 public:
  FRIterator() = default;
  OrderedListType Type() override {return OrderedListType::FR;};
  void TryGetNext(unsigned int k) override;

  // Insert a bulk of FQ iterator, all the same type
  void Insert(unsigned int k,OrderedList* FQ_iterator,
              OnePointPredicatePtr predicates_vec);

  double DeltaCost(OrderedList* node_pointer, int index);
  bool NextEPoolElement(unsigned int k, OrderedList* node_pointer, unsigned int index);
  virtual void GetResult(int i_th,std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record) override;
};

class OWIterator: public OrderedList{
 private:
  // sorting right after building, so don't need a heap to get top-k
  // the top-k result already in the pool
 public:
  OWIterator() = default;
  OrderedListType Type() override {return OrderedListType::OW;};
  void TryGetNext(unsigned int k) override;

  // Insert a bulk of gStore Node ids and their scores
  void Insert(unsigned int k,const std::vector<TYPE_ENTITY_LITERAL_ID>& ids, const std::vector<double>& scores);
  virtual void GetResult(int i_th,std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record) override;
};

// FQ may have cost itself
class FQIterator: public OrderedList{
 private:
  double node_score_;
  minmax::MinMaxHeap<DPB::FqElement> queue_;
  std::vector<OrderedList*> FR_OW_iterators;
  std::vector<std::vector<unsigned int>> seq_list_;
  DPB::ePool e_pool_;
  DPB::DynamicTrie dynamic_trie_;
  std::vector<NodeOneChildVarPredicatesPtr> types_predicates_;
 public:
  void AddOneTypePredicate(NodeOneChildVarPredicatesPtr p){this->types_predicates_.push_back(p);}


  TYPE_ENTITY_LITERAL_ID node_id_;
  explicit FQIterator(int k,TYPE_ENTITY_LITERAL_ID node_id,int child_type_num,double node_score):
      node_score_(node_score), dynamic_trie_(child_type_num,k),node_id_(node_id)
  {this->FR_OW_iterators.reserve(child_type_num);};
  OrderedListType Type() override {return OrderedListType::FQ;};
  void TryGetNext(unsigned int k) override;
  // Insert a bulk of FR or OW iterators
  // inserting one certain type each time each time
  // certain type [i] specified by it's the i-th child of its father
  void Insert(std::vector<OrderedList*> FR_OW_iterators);
  void Insert(OrderedList* FR_OW_iterator);
  bool NextEPoolElement(unsigned int k, OrderedList* node_pointer, unsigned int index);
  double DeltaCost(OrderedList* FR_OW_iterator, int index);
  virtual void GetResult(int i_th,std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record) override;
};

#endif //TOPK_DPB_ORDEREDLIST_H_
