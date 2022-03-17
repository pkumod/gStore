//
// Created by Yuqi Zhou on 2022/3/17.
//

#ifndef INC_010MERGE_QUERY_TOPK_DPB_COMPRESSEDVECTOR_H_
#define INC_010MERGE_QUERY_TOPK_DPB_COMPRESSEDVECTOR_H_
#include "../../../Util/Util.h"
extern size_t Combinatorial[2048][12];

class CompressedVector {
  unsigned int k_,m_;
  std::vector<std::vector<bool>> content_;
  inline size_t E(unsigned int balls,unsigned int box)
  {
    return Combinatorial[box + balls - 1][box - 1];
  }
  size_t GetNumber(std::vector<unsigned int>& seq)
  {
    int num = 0,ac = 0;
    int n = 0;
    for (auto seq_e : seq)
      n += seq_e;
    int m = seq.size();
    for (int i = 0; i < seq.size()-1; i++)
    {
      if (seq[i] > 0)
      {
        int remain = n - ac;
        for (int j = 0; j < seq[i]; j++)
          num += E(remain-j, m - i - 1);
        ac += seq[i];
      }
    }
    return num;
  }
 public:
  CompressedVector(unsigned int k,unsigned int m):k_(k),m_(m){
    content_.resize(k+1);
  };
  void Insert(std::vector<unsigned int>& seq);
  bool AllParentsInserted(std::vector<unsigned int> &seq);
  static void InitialCombinatorial(int balls, int boxes);
};


#endif //INC_010MERGE_QUERY_TOPK_DPB_COMPRESSEDVECTOR_H_
