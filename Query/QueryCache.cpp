/*=============================================================================
# Filename: QueryCache.cpp
# Author: Hao, Cui
# Mail: prospace@bupt.edu.cn
# Last Modified: 2017-06-01
# Description: implement functions in QueryCache.h
=============================================================================*/

#include "QueryCache.h"

using namespace std;

bool QueryCache::getMinimalRepresentation(const Patterns& triple_pattern, Patterns& minimal_repre, map<string, string>& minimal_mapping)
{
  if ((int)triple_pattern.size() > TRIPLE_NUM_LIMIT)
    return false;

  set<string> varset;
  for (int i = 0; i < (int)triple_pattern.size(); i++) {
    if (triple_pattern[i].subject.value[0] == '?')
      varset.insert(triple_pattern[i].subject.value);
    if (triple_pattern[i].predicate.value[0] == '?')
      varset.insert(triple_pattern[i].predicate.value);
    if (triple_pattern[i].object.value[0] == '?')
      varset.insert(triple_pattern[i].object.value);
  }

  int var_count = varset.size();
  if (var_count > VAR_NUM_LIMIT)
    return false;

  string permutation;
  int n_permutation = 1;
  for (int i = 0; i < var_count; i++) {
    permutation.push_back(char('a' + i));
    n_permutation *= (i + 1);
  }

  vector<QueryTree::GroupPattern::Pattern> temp_repre;
  map<string, string> temp_mapping;

  for (int i = 0; i < n_permutation; i++) {
    temp_repre = triple_pattern;
    temp_mapping.clear();

    for (int j = 0; j < (int)temp_repre.size(); j++) {
      string& subject = temp_repre[j].subject.value;
      string& predicate = temp_repre[j].predicate.value;
      string& object = temp_repre[j].object.value;

      if (subject[0] == '?') {
        if (temp_mapping.count(subject) == 0)
          temp_mapping[subject] = "?" + permutation.substr((int)temp_mapping.size(), 1);

        subject = temp_mapping[subject];
      }
      if (predicate[0] == '?') {
        if (temp_mapping.count(predicate) == 0)
          temp_mapping[predicate] = "?" + permutation.substr((int)temp_mapping.size(), 1);

        predicate = temp_mapping[predicate];
      }
      if (object[0] == '?') {
        if (temp_mapping.count(object) == 0)
          temp_mapping[object] = "?" + permutation.substr((int)temp_mapping.size(), 1);

        object = temp_mapping[object];
      }
    }

#ifndef PARALLEL_SORT
    sort(temp_repre.begin(), temp_repre.end());
#else
    omp_set_num_threads(thread_num);
    __gnu_parallel::sort(temp_repre.begin(), temp_repre.end());
#endif
    if (i == 0) {
      minimal_repre = temp_repre;
      minimal_mapping = temp_mapping;
    } else if (temp_repre < minimal_repre) {
      minimal_repre = temp_repre;
      minimal_mapping = temp_mapping;
    }

    next_permutation(permutation.begin(), permutation.end());
  }

  return true;
}

bool QueryCache::tryCaching(const Patterns& triple_pattern, const TempResult& temp_result, int eva_time)
{
  lock_guard<mutex>(this->query_cache_lock); //when quit this scope the lock will be released

  Patterns minimal_repre;
  map<string, string> minimal_mapping;

  if (!this->getMinimalRepresentation(triple_pattern, minimal_repre, minimal_mapping))
    return false;

  if (eva_time < MINIMAL_EVA_TIME_LIMIT)
    return false;

  long long temp_result_memory_used = (long long)temp_result.id_varset.getVarsetSize() * (long long)temp_result.result.size();
  if (temp_result_memory_used > ITEM_MEMORY_LIMIT)
    return false;

  while (temp_result_memory_used + total_memory_used > TOTAL_MEMORY_LIMIT) {
    LRUStruct x = lru.top();
    lru.pop();

    pair<vector<unsigned>, int>& cached_element = cache[x.second.first][x.second.second];
    if (x.first != cached_element.second) {
      x.first = cached_element.second;
      lru.push(x);
    } else {
      total_memory_used -= (long long)cached_element.first.size();

      cache[x.second.first].erase(x.second.second);
      if (cache[x.second.first].empty())
        cache.erase(x.second.first);
    }
  }

  int varnum = temp_result.id_varset.getVarsetSize();

  Varset unordered_varset;
  for (int i = 0; i < varnum; i++)
    unordered_varset.addVar(minimal_mapping[temp_result.id_varset.vars[i]]);

  Varset ordered_varset = unordered_varset;
#ifndef PARALLEL_SORT
  sort(ordered_varset.vars.begin(), ordered_varset.vars.end());
#else
  omp_set_num_threads(thread_num);
  __gnu_parallel::sort(ordered_varset.vars.begin(), ordered_varset.vars.end());
#endif
  vector<int> unordered2ordered = unordered_varset.mapTo(ordered_varset);

  if (cache.count(minimal_repre) == 0) {
    cache[minimal_repre] = map<vector<string>, pair<vector<unsigned>, int> >();
  }

  if (cache[minimal_repre].count(ordered_varset.vars) == 0) {
    cache[minimal_repre][ordered_varset.vars] = pair<vector<unsigned>, int>();
  }

  vector<unsigned>& cache_result = cache[minimal_repre][ordered_varset.vars].first;

  cache_result.resize(varnum * (int)temp_result.result.size());

  for (int i = 0; i < (int)temp_result.result.size(); i++)
    for (int j = 0; j < varnum; j++)
      cache_result[i * varnum + unordered2ordered[j]] = temp_result.result[i].id[j];

  cache[minimal_repre][ordered_varset.vars].second = ++time_now;
  lru.push(make_pair(time_now, make_pair(minimal_repre, ordered_varset.vars)));
  total_memory_used += temp_result_memory_used;

  if (time_now >= (int)1e9) {
    time_now = 0;

    for (map<Patterns, map<vector<string>, pair<vector<unsigned>, int> > >::iterator i = cache.begin(); i != cache.end(); i++)
      for (map<vector<string>, pair<vector<unsigned>, int> >::iterator j = i->second.begin(); j != i->second.end(); j++)
        j->second.second = time_now;
  }

  return true;
}

//NOTICE: in this function we also modify some contents, so we must use mutex instead of rwlock
bool QueryCache::checkCached(const Patterns& triple_pattern, const Varset& varset, TempResult& temp_result)
{
  //this->query_cache_lock.lock();
  lock_guard<mutex>(this->query_cache_lock); //when quit this scope the lock will be released

  Patterns minimal_repre;
  map<string, string> minimal_mapping;

  if (!this->getMinimalRepresentation(triple_pattern, minimal_repre, minimal_mapping))
    return false;

  int varnum = varset.getVarsetSize();

  Varset unordered_varset;
  for (int i = 0; i < varnum; i++)
    unordered_varset.addVar(minimal_mapping[varset.vars[i]]);

  Varset ordered_varset = unordered_varset;
#ifndef PARALLEL_SORT
  sort(ordered_varset.vars.begin(), ordered_varset.vars.end());
#else
  omp_set_num_threads(thread_num);
  __gnu_parallel::sort(ordered_varset.vars.begin(), ordered_varset.vars.end());
#endif
  vector<int> unordered2ordered = unordered_varset.mapTo(ordered_varset);

  if (cache.count(minimal_repre) != 0) {
    if (cache[minimal_repre].count(ordered_varset.vars) != 0) {
      vector<unsigned>& cache_result = cache[minimal_repre][ordered_varset.vars].first;

      temp_result.id_varset = varset;

      temp_result.result.resize((int)cache_result.size() / varnum);
      for (int i = 0; i < (int)temp_result.result.size(); i++) {
        unsigned* v = new unsigned[varnum];

        for (int j = 0; j < varnum; j++)
          v[j] = cache_result[i * varnum + unordered2ordered[j]];

        temp_result.result[i].id = v;
      }

      time_now = min(time_now + 1, (int)1e9);
      cache[minimal_repre][ordered_varset.vars].second = time_now;

      return true;
    }
  }

  return false;
}

void QueryCache::clear()
{
  time_now = 0;
  total_memory_used = 0;

  cache.clear();
  while (!lru.empty())
    lru.pop();
}
