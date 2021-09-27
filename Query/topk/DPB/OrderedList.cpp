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

  auto em = this->pool_.back();
  auto em_pointer = this->fqs_map_[em.node];
  this->queue_.popMin();
  if(NextEPoolElement(k,em_pointer,em.index + 1))
  {
    decltype(em) e;
    e.node = em.node;
    e.index = em.index + 1;
    e.cost = em.cost + this->DeltaCost(em_pointer,e.index);
    this->queue_.push(e);
  }
  if(!queue_.empty()) {
    this->pool_.push_back(this->queue_.findMin());
  }
}


/**
 * Insert One FQ iterator into the FR
 * @param k
 * @param fq_pointer
 */
void FRIterator::Insert(TYPE_ENTITY_LITERAL_ID fq_id,
                        std::shared_ptr<OrderedList> fq_pointer,
                        OnePointPredicatePtr predicates_vec) {
  (*this->type_predicates_)[fq_id] = predicates_vec;
  auto cost = fq_pointer->pool_[0].cost;
  DPB::element e{};
  e.cost = cost;
  e.index = 0;
  e.node = fq_id;
  this->fqs_map_[fq_id] = fq_pointer;
  queue_.push(e);
}

double FRIterator::DeltaCost(std::shared_ptr<OrderedList> node_pointer, int index) {
  auto delta =  node_pointer->pool_[index].cost - node_pointer->pool_[index-1].cost;
  return delta;
}

bool FRIterator::NextEPoolElement(unsigned int k, std::shared_ptr<OrderedList> node_pointer, unsigned int index) {
  if(index == node_pointer->pool_.size())
    node_pointer->TryGetNext(k);
  if(index < node_pointer->pool_.size())
    return true;
  else
    return false;
}

/**
 *
 * @param i_th
 * @param record
 * @param predicate_information not used, because FR itself saves the information
 */
void FRIterator::GetResult(int i_th, std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record,
                           NodeOneChildVarPredicatesPtr predicate_information) {
  auto fq = this->pool_[i_th];
  auto fq_i_th = fq.index;
  auto fq_id = fq.node;
  auto fq_pointer = this->fqs_map_[fq_id];
  if(this->type_predicates_->find(fq_id)!=this->type_predicates_->end()) {
    auto predicates = (*this->type_predicates_)[fq_id];
    for (auto pre_id:*predicates)
      record->push_back(pre_id);
  }
  fq_pointer->GetResult(fq_i_th,record);
}
FRIterator::FRIterator() {
  this->type_predicates_=std::make_shared<NodeOneChildVarPredicates>();
}

void OWIterator::TryGetNext(unsigned int k) {
}

void OWIterator::Insert(const std::vector<TYPE_ENTITY_LITERAL_ID>& ids,
                        const std::vector<double>& scores)
{
  struct ScorePair{
    TYPE_ENTITY_LITERAL_ID id;
    double cost;
    bool operator<(const ScorePair& other) const{return this->cost<other.cost;};
  };
  std::vector<ScorePair> ranks;
  this->pool_.reserve(ids.size());
  ranks.reserve(ids.size());
  for(unsigned int  i=0;i<ids.size();i++)
    ranks.push_back(ScorePair{ids[i],scores[i]});
  std::sort(ranks.begin(),ranks.end());
  for(unsigned int i=0;i < ranks.size();i++)
  {
    DPB::element e{};
    e.index = i;
    e.cost = ranks[i].cost;
    e.node = ranks[i].id;
    this->pool_.push_back(e);
  }
}

/**
 * Default score : 0.0
 * @param ids
 */
void OWIterator::Insert(const std::vector<TYPE_ENTITY_LITERAL_ID> &ids) {
  pool_.reserve(ids.size());
  for(unsigned int i=0;i < ids.size();i++)
  {
    DPB::element e{};
    e.index = i;
    e.cost = 0.0;
    e.node = ids[i];
    this->pool_.push_back(e);
  }
}

void OWIterator::GetResult(int i_th, std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record,
                           NodeOneChildVarPredicatesPtr predicate_information) {
  auto &i_th_element = this->pool_[i_th];
  auto node_id = i_th_element.node;
  auto predicates = (*predicate_information)[node_id];
  for(auto predicate_id:*predicates)
    record->push_back(predicate_id);
  record->push_back(node_id);
}


void FQIterator::TryGetNext(unsigned int k) {
  // get first
  if(this->pool_.size()==0)
  {
    double cost = 0;
    for(unsigned int j =0; j<this->fr_ow_iterators_.size(); j++)
    {
      if(!FQIterator::NextEPoolElement(k, this->fr_ow_iterators_[j], 0))
        return;
      cost += this->fr_ow_iterators_[j]->pool_[0].cost;
    }
    DPB::FqElement e;
    e.seq = DPB::sequence(fr_ow_iterators_.size(), 0);
    e.cost = cost;
    this->dynamic_trie_.insert(e.seq);
    this->queue_.push(e);

    this->e_pool_.push_back(e);
    // transfer e pool to i pool element
    DPB::element ipool_element{};
    ipool_element.cost = e.cost;
    ipool_element.index = this->pool_.size();
    //this->seq_list_.push_back(e.seq);
    this->pool_.push_back(ipool_element);
    return;
  }
  if(this->queue_.empty())
    return;

  // Get Next-- insert the children of the top
  //auto m = this->e_pool_.size();
  auto em = this->e_pool_.back();
  queue_.popMin();
  auto seq = em.seq;
  for(unsigned int j=0; j<this->fr_ow_iterators_.size(); j++)
  {
    seq[j] += 1;
    if(this->dynamic_trie_.detect(seq))
    {
      if(this->NextEPoolElement(k, this->fr_ow_iterators_[j], seq[j]))
      {
        decltype(em) ec;
        ec.cost = em.cost + this->DeltaCost(this->fr_ow_iterators_[j], seq[j]);
        ec.seq = seq;
        this->queue_.push(ec);
      }
    }
    seq[j] -= 1;
  }
  if(!queue_.empty()) {
    auto inserted = this->queue_.findMin();
    inserted.cost += this->node_score_;
    this->e_pool_.push_back(inserted);
    // transfer e pool to i pool element
    DPB::element e{};
    e.cost = inserted.cost;
    e.index = this->pool_.size()-1;
    //this->seq_list_.push_back(std::move(inserted.seq));
    this->pool_.push_back(e);
  }
}

bool FQIterator::NextEPoolElement(unsigned int k, std::shared_ptr<OrderedList> node_pointer, unsigned int index) {
  if(index == node_pointer->pool_.size())
    node_pointer->TryGetNext(k);
  if(index < node_pointer->pool_.size())
    return true;
  else
    return false;
}

void FQIterator::Insert(std::shared_ptr<OrderedList> FR_OW_iterator) {
  this->fr_ow_iterators_.push_back(FR_OW_iterator);
}

/**
 * Insert a bulk of FR or OW iterators.
 * inserting one certain type each time each time
 * certain type [i] specified by it's the i-th child of its father
 * @param FR_OW_iterators
 */
void FQIterator::Insert(std::vector<std::shared_ptr<OrderedList>> FR_OW_iterators) {
  this->fr_ow_iterators_ = std::move(FR_OW_iterators);
}


double FQIterator::DeltaCost(std::shared_ptr<OrderedList> FR_OW_iterator, int index) {
  auto delta =  FR_OW_iterator->pool_[index].cost - FR_OW_iterator->pool_[index-1].cost;
  return delta;
}

void FQIterator::GetResult(int i_th, std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> record,
                           NodeOneChildVarPredicatesPtr predicate_information) {
  record->push_back(this->node_id_);
  //auto &seq = this->seq_list_[i_th];
  auto &seq = this->e_pool_[i_th].seq;
  for(unsigned int i =0; i<this->fr_ow_iterators_.size(); i++) {
    if(fr_ow_iterators_[i]->Type() ==OrderedListType::OW) {
      auto ow_predicates = this->types_predicates_[i];
      fr_ow_iterators_[i]->GetResult(seq[i], record,ow_predicates);
    }
    else
      fr_ow_iterators_[i]->GetResult(seq[i], record);
  }
}
