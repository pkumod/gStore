/*=============================================================================
# Filename: ResultFilter.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2016-05-03 15:36
# Description: implement functions in ResultFilter.h
=============================================================================*/

#include "ResultFilter.h"

using namespace std;

void ResultFilter::addVar(string var)
{
  if (this->result_filter.count(var) == 0)
    this->result_filter.insert(make_pair(var, make_pair(0, vector<int>(this->MAX_SIZE, 0))));
}

void ResultFilter::changeResultHashTable(SPARQLquery& query, int value)
{
  long tv_begin = Util::get_cur_time();

  for (int i = 0; i < query.getBasicQueryNum(); i++) {
    BasicQuery& basicquery = query.getBasicQuery(i);
    vector<unsigned*>& basicquery_result = basicquery.getResultList();
    unsigned result_num = basicquery_result.size();
    int var_num = basicquery.getVarNum();

    for (int j = 0; j < var_num; j++)
      this->addVar(basicquery.getVarName(j));

    vector<vector<int>*> refer;
    for (int j = 0; j < var_num; j++) {
      this->result_filter[basicquery.getVarName(j)].first += value;
      refer.push_back(&this->result_filter[basicquery.getVarName(j)].second);
    }

    for (unsigned j = 0; j < result_num; j++)
      for (int k = 0; k < var_num; k++) {
        (*refer[k])[this->hash(basicquery_result[j][k])] += value;
      }
  }

  long tv_end = Util::get_cur_time();
  printf("after ResultFilter::change, used %ld ms.\n", tv_end - tv_begin);
}

void ResultFilter::candFilterWithResultHashTable(BasicQuery& basicquery)
{
  for (int j = 0; j < basicquery.getVarNum(); j++) {
    map<string, pair<int, vector<int> > >::iterator iter = this->result_filter.find(basicquery.getVarName(j));
    if (iter != this->result_filter.end() && iter->second.first != 0) {
      vector<int>& col = iter->second.second;

      IDList& idlist = basicquery.getCandidateList(j);
      IDList new_idlist;

      printf("candFilter on %s\n", basicquery.getVarName(j).c_str());
      printf("before candFilter, size = %d\n", idlist.size());
      long tv_begin = Util::get_cur_time();

      for (unsigned k = 0; k < idlist.size(); k++) {
        unsigned id = idlist.getID(k);
        if (col[hash(id)] > 0) {
          new_idlist.addID(id);
        }
      }
      idlist = new_idlist;

      long tv_end = Util::get_cur_time();
      printf("after candFilter, size = %d, used %ld ms.\n", idlist.size(), tv_end - tv_begin);
    }
  }
}
