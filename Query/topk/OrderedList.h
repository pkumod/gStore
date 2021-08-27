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
using NodeOneChildVarPredicates = std::map<TYPE_ENTITY_LITERAL_ID ,OnePointPredicatePtr>;
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
  virtual ~OrderedList(){};
};

class FRIterator:public OrderedList {
 private:
  NodeOneChildVarPredicatesPtr type_predicates_;
  minmax::MinMaxHeap<DPB::element> queue_;
  std::map<TYPE_ENTITY_LITERAL_ID,std::shared_ptr<OrderedList>> fqs_map_;
 public:
  FRIterator() = default;
  virtual ~FRIterator(){};
  OrderedListType Type() override {return OrderedListType::FR;};
  void TryGetNext(unsigned int k) override;

  // Insert a bulk of FQ iterator, all the same type
  void Insert(unsigned int k,
              TYPE_ENTITY_LITERAL_ID fq_id,
              std::shared_ptr<OrderedList> FQ_iterator,
              OnePointPredicatePtr predicates_vec);

  static double DeltaCost(std::shared_ptr<OrderedList> node_pointer, int index);
  static bool NextEPoolElement(unsigned int k, std::shared_ptr<OrderedList> node_pointer, unsigned int index);
  virtual void GetResult(int i_th,std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record) override;
};

class OWIterator: public OrderedList{
 private:
  // sorting right after building, so don't need a heap to get top-k
  // the top-k result already in the pool
 public:
  OWIterator() = default;
  virtual ~OWIterator(){};
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
  std::vector<std::shared_ptr<OrderedList>> fr_ow_iterators_;
  std::vector<std::vector<unsigned int>> seq_list_;
  DPB::ePool e_pool_;
  DPB::DynamicTrie dynamic_trie_;
  std::vector<NodeOneChildVarPredicatesPtr> types_predicates_;
  TYPE_ENTITY_LITERAL_ID node_id_;
 public:

  virtual ~FQIterator(){};
  void AddOneTypePredicate(NodeOneChildVarPredicatesPtr p){this->types_predicates_.push_back(p);}
  explicit FQIterator(int k,TYPE_ENTITY_LITERAL_ID node_id,int child_type_num,double node_score):
      node_score_(node_score), dynamic_trie_(child_type_num,k),node_id_(node_id)
  {this->fr_ow_iterators_.reserve(child_type_num);};
  void TryGetNext(unsigned int k) override;
  TYPE_ENTITY_LITERAL_ID GetNodeID(){return this->node_id_;};

  void Insert(std::vector<std::shared_ptr<OrderedList>> FR_OW_iterators);
  void Insert(std::shared_ptr<OrderedList> FR_OW_iterator);
  static bool NextEPoolElement(unsigned int k, std::shared_ptr<OrderedList> node_pointer, unsigned int index);
  static double DeltaCost(std::shared_ptr<OrderedList> FR_OW_iterator, int index);
  virtual void GetResult(int i_th,std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record) override;
  virtual OrderedListType Type() override {return OrderedListType::FQ;};
};

#endif //TOPK_DPB_ORDEREDLIST_H_
