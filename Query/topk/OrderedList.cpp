//
// Created by Yuqi Zhou on 2021/7/4.
//

#include "OrderedList.h"

// getFirst() also implemented here
void FRIterator::TryGetNext(int k) {
  // get first
  if(this->pool_.size()==0)
  {
    this->pool_.push_back(this->queue_.findMin());
    return;
  }

  //
  auto m = this->pool_.size();
  auto em = this->pool_.back();
  this->queue_.popMin();
  if(NextEPoolElement(k,em.identity.pointer,em.index))
  {
    decltype(em) e;
    e.identity.pointer = em.identity.pointer;
    e.index = em.index + 1;
    e.cost = em.cost + this->DeltaCost(e.identity.pointer,em.index);
    this->queue_.push(e);
  }
  if(!queue_.empty()) {
    if (this->queue_.size() > k - m)
      this->queue_.popMax();
    this->pool_.push_back(this->queue_.findMin());
  }
}


void FRIterator::Insert(int k,std::vector<OrderedList *> FQ_iterators) {
  for(auto fq_pointer : FQ_iterators)
  {
    auto cost = fq_pointer->pool_[0].cost;
    if(queue_.size()>=k && cost>queue_.findMax().cost)
      continue;
    DPB::element e{};
    e.cost = cost;
    e.index = 0;
    e.identity.pointer = fq_pointer;
    queue_.push(e);
    if(queue_.size()>=k)
      queue_.popMax();
  }
}

double FRIterator::DeltaCost(OrderedList* node_pointer, int index) {
  return node_pointer->pool_[index+1].cost - node_pointer->pool_[index].cost;
}

bool FRIterator::NextEPoolElement(int k,OrderedList* node_pointer, unsigned int index) {
  if(index == node_pointer->pool_.size())
    node_pointer->TryGetNext(k);
  if(index < node_pointer->pool_.size())
    return true;
  else
    return false;
}

void OWIterator::TryGetNext(int k) {
  return;
}

void OWIterator::Insert(int k,const std::vector<TYPE_ENTITY_LITERAL_ID>& ids, const std::vector<double>& scores) {
  struct ScorePair{
    TYPE_ENTITY_LITERAL_ID id;
    double cost;
    bool operator<(const ScorePair& other){return this->cost<other.cost;};
  };
  std::vector<ScorePair> ranks(ids.size());
  for(int i=0;i<ids.size();i++)
    ranks.push_back(ScorePair{ids[i],scores[i]});
  std::sort(ranks.begin(),ranks.end());
  for(int i=0;i<k && i < ranks.size();i++)
  {
    DPB::element e{};
    e.index = 0;
    e.cost = ranks[i].cost;
    e.identity.node = ranks[i].id;
    this->pool_.push_back(e);
  }
}

void FQIterator::TryGetNext(int k) {
  // get first
  if(this->pool_.size()==0)
  {
    double cost = 0;
    for(int j =0;j<this->FR_OW_iterators.size();j++)
    {
      if(!this->NextEPoolElement(k,this->FR_OW_iterators[j],0))
        return;
      cost += this->FR_OW_iterators[j]->pool_[0].cost;
    }
    DPB::FqElement e;
    e.seq = DPB::sequence(FR_OW_iterators.size(),1);
    e.cost = cost;
    this->dynamic_trie_.insert(e.seq);
    this->queue_.push(e);

    this->e_pool_.push_back(e);
    // transfer e pool to i pool element
    DPB::element ipool_e;
    ipool_e.cost = e.cost;
    ipool_e.index = this->pool_.size();
    ipool_e.identity.pointer = this;
    this->pool_.push_back(ipool_e);
    return;
  }
  auto m = this->e_pool_.size();
  auto em = this->e_pool_.back();
  queue_.popMin();
  auto seq = em.seq;
  for( int j=0;j<this->FR_OW_iterators.size();j++)
  {
    seq[j] += 1;
    if(this->dynamic_trie_.detect(seq))
    {
      if(this->NextEPoolElement(k,this->FR_OW_iterators[j],seq[j]))
      {
        decltype(em) ec;
        ec.cost = em.cost + this->DeltaCost(this->FR_OW_iterators[j],seq[j]);
        ec.seq = seq;
        this->queue_.push(ec);
      }
    }
    seq[j] -= 1;
  }
  if(!queue_.empty()) {
    if (this->queue_.size() > k - m)
      this->queue_.popMax();
    auto inserted = this->queue_.findMin();
    inserted.cost += this->node_score_;
    this->e_pool_.push_back(inserted);
    // transfer e pool to i pool element
    DPB::element e;
    e.cost = inserted.cost;
    e.index = this->pool_.size();
    e.identity.pointer = this;
    this->pool_.push_back(e);
  }
}

bool FQIterator::NextEPoolElement(int k, OrderedList *node_pointer, unsigned int index) {
  if(index == node_pointer->pool_.size())
    node_pointer->TryGetNext(k);
  if(index < node_pointer->pool_.size())
    return true;
  else
    return false;
}

void FQIterator::Insert(std::vector<OrderedList *> FR_OW_iterators) {
  this->FR_OW_iterators = std::move(FR_OW_iterators);
}


double FQIterator::DeltaCost(OrderedList* FR_OW_iterator,int index) {
  return FR_OW_iterator->pool_[index+1].cost - FR_OW_iterator->pool_[index].cost;
}


