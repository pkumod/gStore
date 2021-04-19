//
// Created by 37584 on 2021/3/28.
//

#include "twigPatternTopK.h"
using namespace std;

void FQiterator::GetFirst(){
  // initial an ePoll element 1-1-...-1-1 , cost = 0
  auto ori_seq = std::make_shared<NodeSequence>();
  for(int i =0;i<this->descendant_size_;i++)
    ori_seq->push_back(1);

  auto e1 = make_shared<LatticeSequenceWithScore>(ori_seq, 0);

  for(int j=0;j<this->descendant_size_;j++)
  {
    if(nextIPoolElement(j,0)== false)
      return ;
    else
      e1->score_ += (*(this->i_pools_[j]))[0]->score_;
  }
  trie_.insert(e1);
  queue_.push(e1);
};

bool FQiterator::nextIPoolElement(int descendant_i,int element_i)
{
  // this element has been cached
  if( element_i< this->i_pools_[descendant_i]->size())
    return true;
    // try to cache this element
  else if(element_i == this->i_pools_[descendant_i]->size())
  {
    if(element_i==0)
      this->descendants_[descendant_i]->GetFirst();
    else
      this->descendants_[descendant_i]->GetNext();
    // this element has been filled
    if( element_i< this->i_pools_[descendant_i]->size())
      return true;
    else
      return false;
  }
}

void FQiterator::GetNext()
{
  auto m = this->e_pool_->size()-1;
  auto last_ele = (*this->e_pool_)[m];

  this->queue_.popMin(); // queue.remove_min()
  for(int j=0;j<this->descendant_size_;j++)
  {
    auto new_lattice = std::make_shared<LatticeSequence>(*(last_ele->ori_lattice_));
    // Em.childJ
    (*new_lattice)[j] += 1;

    auto new_ele = make_shared<LatticeSequenceWithScore>(new_lattice, 0);

    if(this->trie_.detect(new_ele))
     if(nextIPoolElement(j,(*last_ele->ori_sequence_)[j]))
     {
       new_ele->score_ = last_ele->score_ + delta_cost(j
           );
       queue_.push(std::move(new_ele));
     }
  }

  if(queue_.size()>0)
  {
    while(queue_.size()> k_-m) queue_.popMax();
    /* Here We need a convert, from lattice node to actual node */
    this->e_pool_->push_back(Convert(queue_.popMin()));
  }
}

/**
 * Search the Lattice Sequence ,i.e. 1-2-4-1-1 , in Trie
 * @param seq : the sequence wanted
 * @return if success, reference to the occurence time
 *         if not found, returns -1, defined at DynamicTrie::invalid_occurence_
 */
int& DynamicTrie::search(std::shared_ptr<LatticeSequenceWithScore> seq) {
  auto ori_seq = seq->ori_sequence_;
  decltype(this->trie_entry_ptr_) layer_entry_ptr_;
  // first layer
  bool found = false;
  auto first_id = (*ori_seq)[0];
  for(const auto& first_lay_ptr:this->trie_entry_ptr_->pointers_)
  {
    if(first_lay_ptr->body.c_ == first_id)
    {
      found = true;
      layer_entry_ptr_ = first_lay_ptr;
      break;
    }
  }

  if(!found)
    return this->invalid_occurence_;

  // second layer till the last
  for(int i=1;i<ori_seq->size();i++)
  {
    found = false;
    auto layer_i = (*ori_seq)[i];
    for(const auto& next_layer_ptr:layer_entry_ptr_->pointers_)
    {
      if(next_layer_ptr->body.c_ == layer_i)
      {
        found = true;
        layer_entry_ptr_ = layer_entry_ptr_;
        break;
      }
    }
    if(!found)
      return this->invalid_occurence_;
    if( i+1 == ori_seq->size() )
      return layer_entry_ptr_->body.counter_;
  }
  return this->invalid_occurence_;
}

/**
 * Insert the Lattice Sequence ,i.e. 1-2-4-1-1 , into Trie
 * @param seq i.e. 1-2-4-1-1
 * @return reference to the occurence time
 */
int& DynamicTrie::insert(std::shared_ptr<LatticeSequenceWithScore> seq) {
  auto ori_seq = seq->ori_sequence_;
  decltype(this->trie_entry_ptr_) layer_entry_ptr_;
  // first layer
  bool found = false;
  auto first_id = (*ori_seq)[0];
  for(const auto& first_lay_ptr:this->trie_entry_ptr_->pointers_)
  {
    if(first_lay_ptr->body.c_ == first_id)
    {
      found = true;
      layer_entry_ptr_ = first_lay_ptr;
      break;
    }
  }

  if(!found)// insert first node
  {
    layer_entry_ptr_= make_shared<TrieEntry>(first_id);
    this->trie_entry_ptr_->pointers_.push_back(layer_entry_ptr_);
  }

  bool inserted = false;
  // second layer till the last
  for(int i=1;i<ori_seq->size();i++)
  {
    found = false;
    inserted = false;
    auto layer_i = (*ori_seq)[i];
    for(const auto& next_layer_ptr:layer_entry_ptr_->pointers_)
    {
      if(next_layer_ptr->body.c_ == layer_i)
      {
        found = true;
        layer_entry_ptr_ = layer_entry_ptr_;
        break;
      }
    }
    if(!found)
    {
      inserted = true;
      layer_entry_ptr_= make_shared<TrieEntry>(layer_i);
      this->trie_entry_ptr_->pointers_.push_back(layer_entry_ptr_);
    }
  }
  if(inserted)
    layer_entry_ptr_->body.counter_ = 0;
  return layer_entry_ptr_->body.counter_;
}

bool DynamicTrie::detect(std::shared_ptr<LatticeSequenceWithScore> seq) {
  int &occur_time = this->search(seq);
  if(occur_time==this->invalid_occurence_)
  {
    this->insert(seq)++;
    // First inserted, occur time = 1
    if(1 < ParentNum(seq->ori_sequence_))
      return false;
    else
      return true;
  }
  else
    occur_time++;
  if(occur_time < ParentNum(seq->ori_sequence_))
    return false;
  return true;
}

void FRiterator::GetFirst() {
  if(queue_.size()>0) {
    // queue.get_min()
    auto queue_min = queue_.findMin();
    this->i_pool_.push_back(queue_.findMin());
  }
}

void FRiterator::GetNext() {
  auto m = this->i_pool_.size();
  auto e_m = this->i_pool_[m-1];
  queue_.popMin(); // queue.remove_min()
  if(nextEPoolElement(e_m->node_i,e_m->index_)){
    auto new_ele = make_shared<FRIPoolElement>();
    new_ele->node_i = e_m->node_i;
    new_ele->index_ = e_m->index_+1;
    new_ele->score_ = e_m->score_ + this->delta_cost( e_m->node_i , e_m->index_);
    queue_.push(new_ele);
  }

  if(queue_.size()>0)
  {
    if(queue_.size() > this->k_-m)
      queue_.popMax();
    this->i_pool_.push_back(queue_.findMin());
  }
}

bool FRiterator::nextEPoolElement(int descendant_i,int element_i)
{
  auto node_FQ = this->FQ_descendants_[descendant_i];
  auto node_e_pool = node_FQ->e_pool_;
  if (element_i< node_e_pool->size())
    return true;
  else if(element_i == node_e_pool->size())
  {
    node_FQ->GetNext();
    if(element_i<node_e_pool->size())
      return true;
    return false;
  }

}

void FRiterator::Insert(shared_ptr<FQiterator> FQ_node) {
  auto e = make_shared<FRIPoolElement>();
  e->node_i = FQ_node->node_v_;
  e->index_ = 0;
  e->score_ = FQ_node->node_cost_ + (*FQ_node->e_pool_)[0]->score_;
  queue_.push(e);
  if(queue_.size()>this->k_)
    queue_.popMax();
}

double FRiterator::delta_cost(int descendant_i, int element_i) {
  auto i_e_pool = this->FQ_descendants_[descendant_i]->e_pool_;
  return (*i_e_pool)[element_i]->score_-(*i_e_pool)[element_i-1]->score_;
};

void OneWayIterator::GetFirst() {
  return;
}

void OneWayIterator::GetNext() {
  return;
}
double OneWayIterator::delta_cost(int element_i)
{
  auto i_pool = this->i_pool_;
  return i_pool[element_i]->score_ -  i_pool[element_i-1]->score_;
}
