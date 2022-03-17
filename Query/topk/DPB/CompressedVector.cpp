//
// Created by Yuqi Zhou on 2022/3/17.
//

#include "CompressedVector.h"

using namespace std;
size_t Combinatorial[2048][12];
void CompressedVector::Insert(vector<unsigned int> &seq) {
  size_t n = 0;
  for(auto seq_e:seq)
    n += seq_e;
  int num = 0,ac = 0;
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
  size_t cap = E(n,this->m_);
  if(n==0)
    cap = 1;
  if(this->content_[n].size()==0)
    this->content_[n].resize(cap, false);
  this->content_[n][num] = true;
}

bool CompressedVector::AllParentsInserted(vector<unsigned int> &seq) {
  size_t n = 0;
  for(auto seq_e:seq)
    n += seq_e;
  if(n == 0)
    return true;
  if(this->content_[n-1].empty())
    return false;
  for(unsigned int i = 0;i<seq.size();i++)
  {
    bool flag = true;
    if(seq[i]==0)
      continue;
    seq[i] -= 1;
    auto parent_num = GetNumber(seq);
    if(this->content_[n-1][parent_num] == false)
      flag = false;
    seq[i] += 1;
    if(!flag)
      return false;
  }
  return true;
}

void CompressedVector::InitialCombinatorial(int balls, int boxes) {
  size_t tmp_C[balls+1][balls+1];
  for (int i = 0; i <= balls; i++)
    for (int j = 0; j <= boxes; j++)
      tmp_C[i][j] = 0;
  tmp_C[1][0] = tmp_C[1][1] = 1;
  // C_m^n = C_{m-1}^{n-1} +C_{m-1}^{n}
  for (int i = 2; i <= balls; i++)
  {
    int left = 0, right = i;
    while (left <= right)
    {
      size_t acc = 0;
      if(left>0)
        acc = tmp_C[i - 1][left - 1] + tmp_C[i - 1][left];
      else
        acc = tmp_C[i - 1][left];
      tmp_C[i][left] = tmp_C[i][right] = acc;
      left++, right--;
    }
  }
  for (int i = 0; i <= balls; i++)
    for (int j = 0; j <= boxes; j++)
      Combinatorial[i][j] = tmp_C[i][j];
}
