/*=============================================================================
# Filename: Strategy.h
# Author: Bookug Lobert 
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-05-07 16:28
# Description: 
=============================================================================*/

#ifndef _DATABASE_STRATEGY_H
#define _DATABASE_STRATEGY_H

#include "../Util/Util.h"
#include "../Util/Triple.h"
#include "Join.h"
#include "../Query/IDList.h"
#include "../Query/SPARQLquery.h"
#include "../Query/BasicQuery.h"
#include "../KVstore/KVstore.h"
#include "../VSTree/VSTree.h"

class Strategy {
  public:
  FILE* fp;
  bool export_flag;

  Strategy();
  // if there exists a variable with limited matches in the query, then skip the filter of other
  // variables as soon as possible
  Strategy(KVstore*, VSTree*, TYPE_TRIPLE_NUM*, TYPE_TRIPLE_NUM*, TYPE_TRIPLE_NUM*, TYPE_PREDICATE_ID,
           TYPE_ENTITY_LITERAL_ID, TYPE_ENTITY_LITERAL_ID);
  ~Strategy();
  //select efficient strategy to do the sparql query
  bool handle(SPARQLquery&);
  bool pre_handler(BasicQuery* basic_query, KVstore* kvstore, TYPE_TRIPLE_NUM* pre2num,
                   TYPE_TRIPLE_NUM* pre2sub, TYPE_TRIPLE_NUM* pre2obj, bool* dealed_triple);

  private:
  int method;
  KVstore* kvstore;
  VSTree* vstree;
  TYPE_TRIPLE_NUM* pre2num;
  TYPE_TRIPLE_NUM* pre2sub;
  TYPE_TRIPLE_NUM* pre2obj;
  TYPE_PREDICATE_ID limitID_predicate;
  TYPE_ENTITY_LITERAL_ID limitID_literal;
  TYPE_ENTITY_LITERAL_ID limitID_entity;

  //NOTICE: even the ID type is int, it is no problem and no waste that we use unsigned in answer
  //(because -1, -2 or other invalid IDs can not be in answer)
  void handler0(BasicQuery*, vector<unsigned*>&);
  void handler1(BasicQuery*, vector<unsigned*>&);
  void handler2(BasicQuery*, vector<unsigned*>&);
  void handler3(BasicQuery*, vector<unsigned*>&);
  void handler4(BasicQuery*, vector<unsigned*>&);
  void handler5(BasicQuery*, vector<unsigned*>&);
  void handler6(BasicQuery*, vector<unsigned*>&);
  //QueryHandler *dispatch;
  //void prepare_handler();
};

//function pointer array
static const unsigned QUERY_HANDLER_NUM = 4;
typedef void (Strategy::*QueryHandler[QUERY_HANDLER_NUM])(BasicQuery*, vector<unsigned*>&);
//QueryHandler dispatch;

#endif //_DATABASE_STRATEGY_H
