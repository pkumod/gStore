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

class Strategy
{
public:
	Strategy();
	Strategy(KVstore*, VSTree*);
	~Strategy();
	//select efficient strategy to do the sparql query
	bool handle(SPARQLquery&);
	bool handle(SPARQLquery& _query, int myRank, string &internal_tag_str);

private:
	int method;
	KVstore* kvstore;
	VSTree* vstree;
	void handler0(BasicQuery*, vector<int*>&);
	void handler0_0(BasicQuery*, vector<int*>&, string &);
	void handler1(BasicQuery*, vector<int*>&);
	void handler2(BasicQuery*, vector<int*>&);
	void handler3(BasicQuery*, vector<int*>&);
	//QueryHandler *dispatch;
	//void prepare_handler();
};

static const unsigned QUERY_HANDLER_NUM = 4;
typedef void (Strategy::*QueryHandler[QUERY_HANDLER_NUM]) (BasicQuery*, vector<int*>&);
//QueryHandler dispatch;

#endif //_DATABASE_STRATEGY_H

