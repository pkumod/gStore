/*=============================================================================
# Filename: SPARQLquery.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-31 19:13
# Description: originally written by liyouhuan, modified by chenjiaqi and zengli
=============================================================================*/

#ifndef _QUERY_SPARQLQUERY_H
#define _QUERY_SPARQLQUERY_H

#include "BasicQuery.h"

class SPARQLquery {
  private:
  vector<BasicQuery*> query_union;
  vector<string> query_var;

  public:
  SPARQLquery(const string& _query);

  SPARQLquery();
  ~SPARQLquery();

  void addQueryVar(const string& _var);

  void addTriple(const Triple& _triple);

  void addBasicQuery(BasicQuery* _basic_q);

  void addBasicQuery();

  const int getBasicQueryNum();

  BasicQuery& getBasicQuery(int _basic_query_id);

  const int getQueryVarNum();

  const vector<string>& getQueryVar() const;

  const string& getQueryVar(int _id);

  void encodeQuery(KVstore* _p_kv_store);
  void encodeQuery(KVstore* _p_kv_store, vector<vector<string> > sparql_query_varset);

  vector<BasicQuery*>& getBasicQueryVec();

  void print(ostream& _out_stream);
  std::string triple_str();
  std::string candidate_str();
  std::string result_str();
  std::string to_str();
};

#endif //_QUERY_SPARQLQUERY_H
