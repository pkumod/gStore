//
// Created by Yuqi Zhou on 2021/7/4.
//

#include "OrderedList.h"

// getFirst() also implemented here
void FRIterator::TryGetNext(unsigned int k) {
  // get first
  if(this->pool_.size()==0)
  {
    this->pool_.push_back(this->queue_.findMin());
    return;
  }
  if(this->queue_.empty())
    return;
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


/**
 * Insert One FQ iterator into the FR
 * @param k
 * @param fq_pointer
 */
void FRIterator::Insert(unsigned int k, OrderedList* fq_pointer,
                        OnePointPredicatePtr predicates_vec) {

  auto already_FQ_num = this->type_predicates_->size();
  this->type_predicates_->push_back(predicates_vec);

  auto cost = fq_pointer->pool_[0].cost;
  if(queue_.size()>=k && cost>queue_.findMax().cost)
    return;
  DPB::element e{};
  e.cost = cost;
  e.index = already_FQ_num;

  e.identity.pointer = fq_pointer;
  queue_.push(e);
  if(queue_.size()>k)
    queue_.popMax();
}

double FRIterator::DeltaCost(OrderedList* node_pointer, int index) {
  auto delta =  node_pointer->pool_[index+1].cost - node_pointer->pool_[index].cost;
  return delta;
}

bool FRIterator::NextEPoolElement(unsigned int k, OrderedList* node_pointer, unsigned int index) {
  if(index + 1 == node_pointer->pool_.size())
    node_pointer->TryGetNext(k);
  if(index + 1< node_pointer->pool_.size())
    return true;
  else
    return false;
}

void FRIterator::GetResult(int i_th, std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record) {
  auto fq = this->pool_[i_th];
  auto fq_i_th = fq.index;
  fq.identity.pointer->GetResult(fq_i_th,record);
}

void OWIterator::TryGetNext(unsigned int k) {
  return;
}

void OWIterator::Insert(unsigned int k,const std::vector<TYPE_ENTITY_LITERAL_ID>& ids, const std::vector<double>& scores) {
  struct ScorePair{
    TYPE_ENTITY_LITERAL_ID id;
    double cost;
    bool operator<(const ScorePair& other){return this->cost<other.cost;};
  };
  std::vector<ScorePair> ranks;
  ranks.reserve(ids.size());
  for(unsigned int  i=0;i<ids.size();i++)
    ranks.push_back(ScorePair{ids[i],scores[i]});
  std::sort(ranks.begin(),ranks.end());
  for(unsigned int i=0;i<k && i < ranks.size();i++)
  {
    DPB::element e{};
    e.index = 0;
    e.cost = ranks[i].cost;
    e.identity.node = ranks[i].id;
    this->pool_.push_back(e);
  }
}

void OWIterator::GetResult(int i_th, std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record) {
  auto &i_th_element = this->pool_[i_th];
  auto node_id = i_th_element.identity.node;
  record->push_back(node_id);
}

void FQIterator::TryGetNext(unsigned int k) {
  // get first
  if(this->pool_.size()==0)
  {
    double cost = 0;
    for(unsigned int j =0;j<this->FR_OW_iterators.size();j++)
    {
      if(!this->NextEPoolElement(k,this->FR_OW_iterators[j],0))
        return;
      cost += this->FR_OW_iterators[j]->pool_[0].cost;
    }
    DPB::FqElement e;
    e.seq = DPB::sequence(FR_OW_iterators.size(),0);
    e.cost = cost;
    this->dynamic_trie_.insert(e.seq);
    this->queue_.push(e);

    this->e_pool_.push_back(e);
    // transfer e pool to i pool element
    DPB::element ipool_e;
    ipool_e.cost = e.cost;
    ipool_e.index = this->pool_.size();
    ipool_e.identity.pointer = this;
    this->seq_list_.push_back(e.seq);
    this->pool_.push_back(ipool_e);
    return;
  }
  if(this->queue_.empty())
    return;
  auto m = this->e_pool_.size();
  auto em = this->e_pool_.back();
  queue_.popMin();
  auto seq = em.seq;
  for( unsigned int j=0;j<this->FR_OW_iterators.size();j++)
  {
    // it means this iterator cannot output more
    if(seq[j]>=k)
      continue;
    seq[j] += 1;
    if(this->dynamic_trie_.detect(seq))
    {
      if(this->NextEPoolElement(k,this->FR_OW_iterators[j],seq[j]))
      {
        decltype(em) ec;
        ec.cost = em.cost + this->DeltaCost(this->FR_OW_iterators[j],seq[j] - 1);
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
    this->seq_list_.push_back(std::move(inserted.seq));
    this->pool_.push_back(e);
  }
}

bool FQIterator::NextEPoolElement(unsigned int k, OrderedList *node_pointer, unsigned int index) {
  auto required_size = index + 1;
  if(required_size == node_pointer->pool_.size())
    node_pointer->TryGetNext(k);
  if(required_size <= node_pointer->pool_.size())
    return true;
  else
    return false;
}

void FQIterator::Insert(OrderedList *FR_OW_iterator) {
  this->FR_OW_iterators.push_back(FR_OW_iterator);
}

void FQIterator::Insert(std::vector<OrderedList *> FR_OW_iterators) {
  this->FR_OW_iterators = std::move(FR_OW_iterators);
}


double FQIterator::DeltaCost(OrderedList* FR_OW_iterator, int index) {
  auto delta =  FR_OW_iterator->pool_[index+1].cost - FR_OW_iterator->pool_[index].cost;
  return delta;
}

void FQIterator::GetResult(int i_th, std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record) {
  record->push_back(this->node_id_);
  auto &seq = this->seq_list_[i_th];
  for(unsigned int i =0;i<this->FR_OW_iterators.size();i++)
    FR_OW_iterators[i]->GetResult(seq[i],record);
}



