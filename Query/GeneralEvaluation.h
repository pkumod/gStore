/*=============================================================================
# Filename: GeneralEvaluation.h
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-05-05
# Description: 
=============================================================================*/

#ifndef _QUERY_GENERALEVALUATION_H
#define _QUERY_GENERALEVALUATION_H

#include "../StringIndex/StringIndex.h"
#include "../Database/CSR.h"
#include "../Database/Optimizer.h"
#include "../Parser/QueryParser.h"
#include "../Util/EvalMultitypeValue.h"
#include "SPARQLquery.h"
#include "RegexExpression.h"
#include "TempResult.h"
#include "QueryCache.h"
#include "ResultSet.h"
#include <dlfcn.h>
#include "PathQueryHandler.h"

struct hashFunction
{
  size_t operator()(const pair<int , 
                    int> &x) const
  {
    return x.first ^ x.second;
  }
};

class GeneralEvaluation
{
	private:
		TempResultSet *temp_result;
		shared_ptr<QueryParser> query_parser;
		QueryTree query_tree;
		int well_designed;
		KVstore *kvstore;
		StringIndex *stringindex;
		shared_ptr<Optimizer> optimizer_;
		QueryCache *query_cache;
		PathQueryHandler *pqHandler;
		CSR *csr;
		bool ranked;

		TYPE_TRIPLE_NUM *pre2num;
		TYPE_TRIPLE_NUM *pre2sub;
		TYPE_TRIPLE_NUM *pre2obj;
		TYPE_TRIPLE_NUM triples_num;
		TYPE_PREDICATE_ID limitID_predicate;
		TYPE_ENTITY_LITERAL_ID limitID_literal;
		TYPE_ENTITY_LITERAL_ID limitID_entity;
		shared_ptr<Transaction> txn;

		shared_ptr<BGPQuery> bgp_query_total;

		char* string_index_buffer;
		unsigned string_index_buffer_size;

		// freelist_entity+entity_num: used for getting all entity id as sources in kleeneClosure evaluation
		class AllEntityId
		{
			const BlockInfo *const freelist_entity,*head; // deleted entity id
			const TYPE_ENTITY_LITERAL_ID entity_num;
			TYPE_ENTITY_LITERAL_ID next_id, iter_cnt; 
		public:
			AllEntityId(const BlockInfo *const freelist_entity, TYPE_ENTITY_LITERAL_ID entity_num):freelist_entity(freelist_entity),head(freelist_entity),entity_num(entity_num),next_id(0),iter_cnt(0){}
			// return next entity_id, or INVALID_ENTITY_LITERAL_ID if there's no such 
			TYPE_ENTITY_LITERAL_ID next(){
				if(iter_cnt>=entity_num)
					return INVALID_ENTITY_LITERAL_ID;
				++iter_cnt;
				while(head && head->num==next_id){
					++next_id;
					head=head->next;
				}
				++next_id;
				return next_id-1;
			}
			TYPE_ENTITY_LITERAL_ID init(){
				next_id=0;
				iter_cnt=0;
				head=freelist_entity;
				return next();
			}
		} all_entity_id;
		
    public:
    	FILE* fp;
    	bool export_flag;

	public:
		GeneralEvaluation(KVstore *_kvstore, StringIndex *_stringindex,  QueryCache *_query_cache, CSR *_csr,
						  TYPE_TRIPLE_NUM *_pre2num,TYPE_TRIPLE_NUM *_pre2sub,
						  TYPE_TRIPLE_NUM *_pre2obj, TYPE_TRIPLE_NUM _triples_num, TYPE_PREDICATE_ID _limitID_predicate,
						  TYPE_ENTITY_LITERAL_ID _limitID_literal, TYPE_ENTITY_LITERAL_ID _limitID_entity,
						  shared_ptr<Transaction> txn = nullptr, const BlockInfo *const freelist_entity = nullptr, TYPE_ENTITY_LITERAL_ID entity_num = 0);
		// Note that query_tree, well_designed, ranked, bgp_query_total not copied
		GeneralEvaluation(const GeneralEvaluation& _ge): query_parser(_ge.query_parser), well_designed(-1), \
			kvstore(_ge.kvstore), stringindex(_ge.stringindex), optimizer_(_ge.optimizer_), \
			query_cache(_ge.query_cache), pqHandler(_ge.pqHandler), csr(_ge.csr), ranked(false), pre2num(_ge.pre2num), \
			pre2sub(_ge.pre2sub), pre2obj(_ge.pre2obj), limitID_predicate(_ge.limitID_predicate), \
			limitID_literal(_ge.limitID_literal), limitID_entity(_ge.limitID_entity), txn(_ge.txn), \
			all_entity_id(_ge.all_entity_id), \
			fp(_ge.fp), export_flag(_ge.export_flag){}

		~GeneralEvaluation();

		bool parseQuery(const std::string &_query);
		QueryTree& getQueryTree();

		bool doQuery();

		void addAllTriples(const GroupPattern &group_pattern);

		void setStringIndexPointer(StringIndex* _tmpsi);
		
	private:
		struct EvaluationStackStruct
		{
			GroupPattern group_pattern;
			TempResultSet *result;
			EvaluationStackStruct();
			EvaluationStackStruct(const EvaluationStackStruct& that);
			EvaluationStackStruct& operator=(const EvaluationStackStruct& that);
			~EvaluationStackStruct();
		};
		std::vector<EvaluationStackStruct> rewriting_evaluation_stack;

	public:
		bool expanseFirstOuterUnionGroupPattern(GroupPattern &group_pattern, std::deque<GroupPattern> &queue);

		TempResultSet* queryEvaluation(int dep);

		void getFinalResult(ResultSet &ret_result);
		void releaseResult();

		void prepareUpdateTriple(GroupPattern &update_pattern, TripleWithObjType *&update_triple, TYPE_TRIPLE_NUM &update_triple_num);

	private:
		void loadCSR();
		void prepPathQuery();
		void pathVec2JSON(int src, int dst, const std::vector<int> &v, std::stringstream &ss);
		void vertVec2JSON(const std::vector<int> &v, std::stringstream &ss);

		int constructTriplePattern(GroupPattern& triple_pattern, int dep);
		void getUsefulVarset(Varset& useful, int dep);
		bool checkBasicQueryCache(vector<GroupPattern::Pattern>& basic_query, TempResultSet *sub_result, Varset& useful);
		void fillCandList(SPARQLquery& sparql_query, int dep, vector<vector<string> >& encode_varset);
		void fillCandList(vector<shared_ptr<BGPQuery>>& bgp_query_vec, int dep, vector<vector<string> >& encode_varset, TempResultSet *fill_result=nullptr);
		void joinBasicQueryResult(SPARQLquery& sparql_query, TempResultSet *new_result, TempResultSet *sub_result, vector<vector<string> >& encode_varset, \
			vector<vector<GroupPattern::Pattern> >& basic_query_handle, long tv_begin, long tv_handle, int dep=0);
		void getAllPattern(const GroupPattern &group_pattern, vector<GroupPattern::Pattern> &vp);
		void copyBgpResult2TempResult(std::shared_ptr<BGPQuery> bgp_query, int varnum, TempResult &tr);
		std::map<std::string, std::string> dynamicFunction(const std::vector<int> &iri_set, bool directed, int k, const std::vector<int> &pred_set, const std::string& fun_name, const std::string& username);

		void kleeneClosure(TempResultSet *temp, TempResult * const tr, const string &subject, const string &predicate, const string &object, int dep);
		void BFS(TempResultSet *temp, int sid, int pred, bool forward, int numCol=2);
};

#endif // _QUERY_GENERALEVALUATION_H

