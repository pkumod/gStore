#include "../../Util/Util.h"
#include "./MinMaxHeap.hpp"
#ifndef GSTORELIMITK_QUERY_TOPKSTRUCTURE_TOPKSTRUCTURE_H_
#define GSTORELIMITK_QUERY_TOPKSTRUCTURE_TOPKSTRUCTURE_H_

using NodeSequence = std::vector<TYPE_ENTITY_LITERAL_ID>;
using NodeSequencePtr = std::shared_ptr<NodeSequence>;

using LatticeSequence = std::vector<TYPE_ENTITY_LITERAL_ID>;
using LatticeSequencePtr = std::shared_ptr<LatticeSequence>;
/*
 * FR-iPool element is a (node, index, cost)
 * FQ.ePool element is a (seq, cost)
 * Each OW-iPool, say v.iPoolJ , element is a (node, cost)
 *
 * For convience, we merge the two type FR-iPool element and OW-iPool into one struct 'FRIPoolElement'
 * */

struct FRIPoolElement{
  NodeSequencePtr ori_sequence_;
  double score_;
  int index_;
  int node_i;
};

struct FQEPoolElement{
  NodeSequencePtr ori_sequence_;
  LatticeSequencePtr ori_lattice_;
  double score_;
  FQEPoolElement(std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> ori_sequence, double score):
      ori_sequence_(ori_sequence),score_(score){};
};

using FQEPoolElementPtr = std::shared_ptr<FQEPoolElement>;

struct LatticeSequenceWithScore{
  NodeSequencePtr ori_sequence_;
  double score_;
  int counter_;
  LatticeSequenceWithScore(std::shared_ptr<std::vector<TYPE_ENTITY_LITERAL_ID>> ori_sequence, double score):
      ori_sequence_(ori_sequence),score_(score),counter_(0){};
};

using LatticeSequenceWithScorePtr = std::shared_ptr<LatticeSequenceWithScore>;
using SequencePoolPtr = std::shared_ptr<std::deque<FQEPoolElementPtr>>;

class LatticeSequenceWithScoreCompare{
 public:
  bool operator()(const LatticeSequenceWithScorePtr& a,const LatticeSequenceWithScorePtr& b)
  {
    return a->score_ < b->score_;
  }
};

using FRIPoolElementPtr = std::shared_ptr<FRIPoolElement>;
class FRIPoolElementPtrCompare{
 public:
  bool operator()(const FRIPoolElementPtr& a,const FRIPoolElementPtr& b)
  {
    return a->score_ < b->score_;
  }
};

using MinMaxHeapLatticeSeq =  minmax::MinMaxHeap<LatticeSequenceWithScorePtr, // element type
                                          std::vector<LatticeSequenceWithScorePtr>,  // container
                                          LatticeSequenceWithScoreCompare>;  // compare function

using FRIPoolQueue =  minmax::MinMaxHeap<FRIPoolElementPtr, // element type
                                                 std::vector<FRIPoolElementPtr>,  // container
                                         FRIPoolElementPtrCompare>;  // compare function

#endif //GSTORELIMITK_QUERY_TOPKSTRUCTURE_TOPKSTRUCTURE_H_
