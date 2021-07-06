//
// Created by Yuqi Zhou on 2021/7/4.
//

#ifndef TOPK_DPB_ORDEREDLIST_H_
#define TOPK_DPB_ORDEREDLIST_H_

enum class OrderedListType{UnDefined,FR,OW};

class OrderedList{
 public:
  virtual OrderedListType Type(){return OrderedListType::UnDefined;};
  virtual void GetNext(){};
  virtual void Insert(){};
  virtual double DeltaCost(){return 0.0;};
};

class FRIterator:OrderedList {

 public:
  OrderedListType Type() override {return OrderedListType::FR;};
  void GetNext() override;
  void Insert() override;
  double DeltaCost()override;
  bool NextEPoolElement(TYPE_ENTITY_LITERAL_ID node,unsigned int index);
};

class OWIterator:OrderedList{
 public:
  OrderedListType Type() override {return OrderedListType::OW;};
  void GetNext()override;
  void Insert()override;
  double DeltaCost()override;
};

class FQIterator:OrderedList{

};

#endif //TOPK_DPB_ORDEREDLIST_H_
