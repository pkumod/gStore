/*=============================================================================
# Filename: GeneralEvaluation.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-05-05
# Description: implement functions in GeneralEvaluation.h
=============================================================================*/

#include "GeneralEvaluation.h"
#include<set>
using namespace std;

void *preread_from_index(void *argv)
{
	vector<StringIndexFile*> * indexfile = (vector<StringIndexFile*> *)*(long*)argv;
	ResultSet *ret_result = (ResultSet*)*((long*)argv + 1);
	vector<int>* proj2temp = (vector<int>*)*((long*)argv + 2);
	int id_cols = *((long*)argv + 3);
	TempResult* result0 = (TempResult*)*((long*)argv + 4);
	vector<bool>* isel = (vector<bool>*)*((long*)argv + 5);
	StringIndexFile* 	entity = (*indexfile)[0];
	StringIndexFile* 	literal = (*indexfile)[1];
	StringIndexFile* 	predicate = (*indexfile)[2];
	unsigned total = ret_result->ansNum;
	int var_num = ret_result->select_var_num;
	unsigned thelta = total / 4096;
	char tmp;
	char *entityc = entity->Mmap;
	char *literalc = literal->Mmap;
	char *prec = predicate->Mmap;
	for (unsigned i = 0; i < total; i ++)
	{
		for (int j = 0; j < var_num; j++)
		{
			int k = (*proj2temp)[j];
			long offset = -1;
			if (k != -1)
			{
				if (k < id_cols)
				{
					unsigned ans_id = result0->result[i].id[k];
					if (ans_id != INVALID)
					{
						if ((*isel)[k])
						{
							if (ans_id < Util::LITERAL_FIRST_ID)
							{
								offset = entity->GetOffsetbyID(ans_id);
								if (offset != -1)
								{
									tmp = entityc[offset];
								}
							}
							else
							{
								offset = literal->GetOffsetbyID(ans_id - Util::LITERAL_FIRST_ID);
								if (offset != -1)
								{
									tmp = literalc[offset];
								}
							}
						}
						else
						{
							offset = predicate->GetOffsetbyID(ans_id);	
							if (offset != -1)
							{
								tmp = prec[offset];
							}
						}
					}
				}
			}
		}
	}
	
	return NULL;
}


GeneralEvaluation::GeneralEvaluation(VSTree *_vstree, KVstore *_kvstore, Statistics *_statistics, StringIndex *_stringindex, QueryCache *_query_cache, \
	TYPE_TRIPLE_NUM *_pre2num,TYPE_TRIPLE_NUM *_pre2sub, TYPE_TRIPLE_NUM *_pre2obj, \
	TYPE_PREDICATE_ID _limitID_predicate, TYPE_ENTITY_LITERAL_ID _limitID_literal, \
	TYPE_ENTITY_LITERAL_ID _limitID_entity, CSR *_csr, shared_ptr<Transaction> _txn):
	vstree(_vstree), kvstore(_kvstore), statistics(_statistics), stringindex(_stringindex), query_cache(_query_cache), pre2num(_pre2num), \
	pre2sub(_pre2sub), pre2obj(_pre2obj), limitID_predicate(_limitID_predicate), limitID_literal(_limitID_literal), \
	limitID_entity(_limitID_entity), temp_result(NULL), fp(NULL), export_flag(false), csr(_csr), txn(_txn)
{
	if (csr)
		pqHandler = new PathQueryHandler(csr);
	else
		pqHandler = NULL;
	well_designed = -1;	// Not checked yet
}

GeneralEvaluation::~GeneralEvaluation()
{
	if (pqHandler)
		delete pqHandler;
}

void
GeneralEvaluation::loadCSR()
{
	cout << "GeneralEvaluation::loadCSR" << endl;

	if (csr)
		delete csr;
	csr = new CSR[2];

	unsigned pre_num = stringindex->getNum(StringIndexFile::Predicate);
	csr[0].init(pre_num);
	csr[1].init(pre_num);	
	cout << "pre_num: " << pre_num << endl;
	long begin_time = Util::get_cur_time();

	// Process out-edges (csr[0])
	// i: predicate; j: subject; k: object
	for(unsigned i = 0; i < pre_num; i++)
	{
		string pre = kvstore->getPredicateByID(i);
		cout<<"pid: "<<i<<"    pre: "<<pre<<endl;
		unsigned* sublist = NULL;
		unsigned sublist_len = 0;
		bool ret = kvstore->getsubIDlistBypreID(i, sublist, sublist_len, true);
		//cout<<"    sub_num: "<<sublist_len<<endl;
		unsigned offset = 0;
		unsigned index = 0;
		for(unsigned j=0;j<sublist_len;j++)
		{
			string sub = kvstore->getEntityByID(sublist[j]);
			//cout<<"    sid: "<<sublist[j]<<"    sub: "<<sub<<endl;
			unsigned* objlist = NULL;
			unsigned objlist_len = 0;
			bool ret = kvstore->getobjIDlistBysubIDpreID(sublist[j], i, objlist, objlist_len); 
			unsigned len = objlist_len;	// the real object list length
			for(unsigned k=0;k<objlist_len;k++)
			{
				if(objlist[k]>=2000000000)
				{
					--len;
					continue;
				}
				string obj = kvstore->getEntityByID(objlist[k]);
				//cout<<"        oid: "<<objlist[k]<<"    obj: "<<obj<<endl;
				csr[0].adjacency_list[i].push_back(objlist[k]);
			}
			//cout<<"        obj_num: "<<len<<endl;
			if(len > 0)
			{
				csr[0].id2vid[i].push_back(sublist[j]);
				csr[0].vid2id[i].insert(pair<unsigned, unsigned>(sublist[j], index));
				csr[0].offset_list[i].push_back(offset);
				index++;
				offset += len;
			}
		}
		cout<<csr[0].offset_list[i].size()<<endl;	// # of this predicate's subjects
		cout<<csr[0].adjacency_list[i].size()<<endl;	// # of this predicate's objects
	}

	// Process out-edges (csr[1])
	// i: predicate; j: object; k: subject
	for(unsigned i = 0;i<pre_num;i++)
	{
		string pre = kvstore->getPredicateByID(i);
		cout<<"pid: "<<i<<"    pre: "<<pre<<endl;
		unsigned* objlist = NULL;
		unsigned objlist_len = 0;
		bool ret = kvstore->getobjIDlistBypreID(i, objlist, objlist_len, true);
		//cout<<"    obj_num: "<<objlist_len<<endl;
		unsigned offset = 0;
		unsigned index = 0;
		for(unsigned j=0;j<objlist_len;j++)
		{
			if(objlist[j]>=2000000000)
				continue;
			string obj = kvstore->getEntityByID(objlist[j]);
			//cout<<"    oid: "<<objlist[j]<<"    obj: "<<obj<<endl;
			unsigned* sublist = NULL;
			unsigned sublist_len = 0;
			bool ret = kvstore->getsubIDlistByobjIDpreID(objlist[j], i, sublist, sublist_len); 
			unsigned len = sublist_len;
			for(unsigned k=0;k<sublist_len;k++)
			{
				string sub = kvstore->getEntityByID(sublist[k]);
				//cout<<"        sid: "<<sublist[k]<<"    sub: "<<sub<<endl;
				csr[1].adjacency_list[i].push_back(sublist[k]);
			}
			//cout<<"        sub_num: "<<len<<endl;
			if(len > 0)
			{
				csr[1].id2vid[i].push_back(objlist[j]);
				csr[1].vid2id[i].insert(pair<unsigned, unsigned>(objlist[j], index));
				csr[1].offset_list[i].push_back(offset);
				index++;
				offset += len;
			}
		}
		cout<<csr[1].offset_list[i].size()<<endl;
		cout<<csr[1].adjacency_list[i].size()<<endl;
	}
	long end_time = Util::get_cur_time();
	cout << "Loading CSR in GeneralEvaluation takes " << (end_time - begin_time) << "ms" << endl;
}

void
GeneralEvaluation::prepPathQuery()
{
	if (!csr)
		loadCSR();
	if (!pqHandler)
		pqHandler = new PathQueryHandler(csr);
}

void
GeneralEvaluation::setStringIndexPointer(StringIndex* _tmpsi)
{
	this->stringindex = _tmpsi;
}

bool GeneralEvaluation::parseQuery(const string &_query)
{
	try
	{
		this->query_parser.setQueryTree(&(this->query_tree));
		this->query_parser.SPARQLParse(_query);
	}
	catch (const char *e)
	{
		printf("%s\n", e);
		return false;
	}

	return true;
}

QueryTree& GeneralEvaluation::getQueryTree()
{
	return this->query_tree;
}

bool GeneralEvaluation::doQuery()
{
	query_tree.print();

	if (!this->query_tree.checkProjectionAsterisk() && this->query_tree.getProjection().empty())
		return false;

	this->query_tree.getGroupPattern().getVarset();

	this->query_tree.getGroupByVarset() = this->query_tree.getGroupByVarset() * this->query_tree.getGroupPattern().group_pattern_resultset_maximal_varset;

	if (this->query_tree.checkProjectionAsterisk() && this->query_tree.getProjection().empty() && !this->query_tree.getGroupByVarset().empty())
	{
		printf("[ERROR]	Select * and Group By can't appear at the same time.\n");
		return false;
	}

	if (!this->query_tree.checkSelectAggregateFunctionGroupByValid())
	{
		printf("[ERROR]	The vars/aggregate functions in the Select Clause are invalid.\n");
		return false;
	}

	if (this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset.hasCommonVar(this->query_tree.getGroupPattern().group_pattern_predicate_maximal_varset))
	{
		printf("[ERROR]	There are some vars occur both in subject/object and predicate.\n");
		return false;
	}

	this->strategy = Strategy(this->kvstore, this->vstree, this->pre2num,this->pre2sub, this->pre2obj, 
		this->limitID_predicate, this->limitID_literal, this->limitID_entity,
		this->query_tree.Modifier_Distinct== QueryTree::Modifier_Distinct, txn);

    this->optimizer_ = make_shared<Optimizer>(kvstore,vstree,statistics,pre2num,pre2sub,pre2obj,limitID_predicate,
                                      limitID_literal,limitID_entity,txn);

	// if (this->query_tree.checkWellDesigned())
	// if (false)
	// {
	// 	printf("=================\n");
	// 	printf("||well-designed||\n");
	// 	printf("=================\n");

	// 	this->rewriting_evaluation_stack.clear();
	// 	this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
	// 	this->rewriting_evaluation_stack.back().group_pattern = this->query_tree.getGroupPattern();
	// 	this->rewriting_evaluation_stack.back().result = NULL;

	// 	this->temp_result = this->rewritingBasedQueryEvaluation(0);
	// }
	// else
	// {
	// 	printf("=====================\n");
	// 	printf("||not well-designed||\n");
	// 	printf("=====================\n");

	// 	this->temp_result = this->semanticBasedQueryEvaluation(this->query_tree.getGroupPattern());
	// }

	this->rewriting_evaluation_stack.clear();
	this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
	this->rewriting_evaluation_stack.back().group_pattern = this->query_tree.getGroupPattern();
	this->rewriting_evaluation_stack.back().result = NULL;

	this->temp_result = this->queryEvaluation(0);

	return true;
}

TempResultSet* GeneralEvaluation::queryEvaluation(int dep)
{
	QueryTree::GroupPattern group_pattern;

	// Check well-designed (TODO: check at every depth, now only check once) //
	// If well-designed, split and refill group_pattern according to rewriting //
	if (well_designed == -1)
		well_designed = (int)query_tree.checkWellDesigned();

	if (well_designed == 0)	// Not well-designed, semantic-based evaluation
	{
		printf("=================================\n");
		printf("||not well-designed at dep = %d||\n", dep);
		printf("=================================\n");
		group_pattern = rewriting_evaluation_stack[dep].group_pattern;
	}
	else if (well_designed == 1)	// Well-designed, rewriting-based evaluation
	{
		printf("=============================\n");
		printf("||well-designed at dep = %d||\n", dep);
		printf("=============================\n");

		/// Construct group_pattern_union, which will consist of all expansion results ///
		/// (group-patterns without UNIONs, whose results will have to be UNION'ed to get ///
		/// the original result) ///
		deque<QueryTree::GroupPattern> queue;
		queue.push_back(this->rewriting_evaluation_stack[dep].group_pattern);
		// vector<QueryTree::GroupPattern> group_pattern_union;
		group_pattern.addOneGroupUnion();
		int numUnions = 0;

		while (!queue.empty())
		{
			if (!this->expanseFirstOuterUnionGroupPattern(queue.front(), queue))
			{
				// group_pattern_union.push_back(queue.front());	// Expansion complete
				group_pattern.addOneUnion();
				group_pattern.getLastUnion() = queue.front();
				// group_pattern.sub_group_pattern[0].unions[numUnions] = queue.front();
			}
			queue.pop_front();
		}
	}
	group_pattern.initPatternBlockid();

	TempResultSet *result = new TempResultSet();
	// Iterate across all sub-group-patterns, process according to type
	for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
		if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Group_type)
		{
			group_pattern.sub_group_pattern[i].group_pattern.print(0);
			this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
			this->rewriting_evaluation_stack.back().group_pattern = group_pattern.sub_group_pattern[i].group_pattern;
			this->rewriting_evaluation_stack.back().result = NULL;
			TempResultSet *temp = queryEvaluation(dep + 1);

			if (result->results.empty())
			{
				delete result;
				result = temp;
			}
			else
			{
				TempResultSet *new_result = new TempResultSet();
				result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				temp->release();
				result->release();
				delete temp;
				delete result;

				result = new_result;
			}
		}
		else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
		{
			int st = i;
			while (st > 0 && (group_pattern.sub_group_pattern[st - 1].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type || group_pattern.sub_group_pattern[st - 1].type == QueryTree::GroupPattern::SubGroupPattern::Union_type))
				st--;

			for (int j = st; j < i; j++)
				if (group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
				{
					if (group_pattern.sub_group_pattern[i].pattern.subject_object_varset.hasCommonVar(group_pattern.sub_group_pattern[j].pattern.subject_object_varset))
						group_pattern.mergePatternBlockID(i, j);
				}

			if (i + 1 == (int)group_pattern.sub_group_pattern.size() ||
				(group_pattern.sub_group_pattern[i + 1].type != QueryTree::GroupPattern::SubGroupPattern::Pattern_type && group_pattern.sub_group_pattern[i + 1].type != QueryTree::GroupPattern::SubGroupPattern::Union_type))
			{
				// Reach the end of this BGP, start merging
				SPARQLquery sparql_query;
				vector<vector<string> > encode_varset;
				vector<vector<QueryTree::GroupPattern::Pattern> > basic_query_handle;

				for (int j = st; j <= i; j++)
					if (group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
					{
						if (!group_pattern.sub_group_pattern[j].pattern.varset.empty())
						{
							if (group_pattern.getRootPatternBlockID(j) == j)		//root node, coming from mergePatternBlockID
							{
								Varset occur;
								vector<QueryTree::GroupPattern::Pattern> basic_query;

								for (int k = st; k <= i; k++)
									if (group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
										if (group_pattern.getRootPatternBlockID(k) == j)
										{
											occur += group_pattern.sub_group_pattern[k].pattern.varset;
											basic_query.push_back(group_pattern.sub_group_pattern[k].pattern);
										}

								printf("select vars: ");
								occur.print();

								printf("triple patterns: \n");
								for (int k = 0; k < (int)basic_query.size(); k++)
									printf("%s\t%s\t%s\n", basic_query[k].subject.value.c_str(),
										basic_query[k].predicate.value.c_str(),
										basic_query[k].object.value.c_str()
									);

								bool success = false;
								if (this->query_cache != NULL)
									success = checkBasicQueryCache(basic_query, result, occur);
								//// If query cache not hit, save the current BGP to sparql_query for later processing ////
								//// QUESTION: Is basic_query_handle a redundant variable? ////
								if (!success)
								{
									sparql_query.addBasicQuery();
									for (int k = 0; k < (int)basic_query.size(); k++)
										sparql_query.addTriple(Triple(basic_query[k].subject.value,
											basic_query[k].predicate.value,
											basic_query[k].object.value));

									encode_varset.push_back(occur.vars);
									basic_query_handle.push_back(basic_query);
								}
							}
						}
					}

				long tv_begin = Util::get_cur_time();
				sparql_query.encodeQuery(this->kvstore, encode_varset);
				long tv_encode = Util::get_cur_time();
				printf("during Encode, used %ld ms.\n", tv_encode - tv_begin);

				/* PLEASE REPLACE WITH OPTIMIZER */
				QueryInfo query_info;

				query_info.limit_ = false;
				if(this->query_tree.getLimit()!=-1) {
					query_info.limit_ = true;
					query_info.limit_num_ = this->query_tree.getLimit();
				}

				query_info.is_distinct_ = this->query_tree.getProjectionModifier() == QueryTree::ProjectionModifier::Modifier_Distinct;

				query_info.ordered_by_vars_ = make_shared<vector<QueryTree::Order>>();
				for(auto order_item:this->query_tree.getOrderVarVector())
					query_info.ordered_by_vars_->push_back(order_item);

				this->optimizer_->DoQuery(sparql_query,query_info);

				long tv_handle = Util::get_cur_time();
				printf("during Handle, used %ld ms.\n", tv_handle - tv_encode);

				//collect and join the result of each BasicQuery
				TempResultSet *new_result;
				for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
				{
					TempResultSet *temp = new TempResultSet();
					temp->results.push_back(TempResult());

					temp->results[0].id_varset = Varset(encode_varset[j]);
					int varnum = (int)encode_varset[j].size();

					vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(j).getResultList();
					int basicquery_result_num = (int)basicquery_result.size();

					temp->results[0].result.reserve(basicquery_result_num);
					for (int k = 0; k < basicquery_result_num; k++)
					{
						unsigned *v = new unsigned[varnum];
						memcpy(v, basicquery_result[k], sizeof(int) * varnum);
						temp->results[0].result.push_back(TempResult::ResultPair());
						temp->results[0].result.back().id = v;
					}

					if (this->query_cache != NULL)
					{
						//if unconnected, time is incorrect
						int time = tv_handle - tv_begin;

						long tv_bftry = Util::get_cur_time();
						bool success = this->query_cache->tryCaching(basic_query_handle[j], temp->results[0], time);
						if (success)	printf("QueryCache cached\n");
						else			printf("QueryCache didn't cache\n");
						long tv_aftry = Util::get_cur_time();
						printf("during tryCache, used %ld ms.\n", tv_aftry - tv_bftry);
					}

					if (result->results.empty())
					{
						delete result;
						result = temp;
					}
					else
					{
						TempResultSet *new_result = new TempResultSet();
						result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

						temp->release();
						result->release();
						delete temp;
						delete result;

						result = new_result;
					}
				}
				
				// printf("Pattern_type result: \n");
				// result->print();
			}
		}
		else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Union_type)
		{
			TempResultSet *sub_result_outer = new TempResultSet();

			for (int j = 0; j < (int)group_pattern.sub_group_pattern[i].unions.size(); j++)
			{
				TempResultSet *sub_result;
				if (well_designed == 0)
				{
					this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
					this->rewriting_evaluation_stack.back().group_pattern = group_pattern.sub_group_pattern[i].unions[j];
					this->rewriting_evaluation_stack.back().result = NULL;
					TempResultSet *sub_result = queryEvaluation(dep + 1);
				}
				else if (well_designed == 1)
				{			
					// Prepare and print //
					this->rewriting_evaluation_stack[dep].group_pattern = group_pattern.sub_group_pattern[i].unions[j];

					for (int j = 0; j < 80; j++)			printf("=");	printf("\n");
					rewriting_evaluation_stack[dep].group_pattern.print(dep);
					for (int j = 0; j < 80; j++)			printf("=");	printf("\n");

					sub_result = new TempResultSet();

					// Construct triple_pattern //
					QueryTree::GroupPattern triple_pattern;
					int group_pattern_triple_num = constructTriplePattern(triple_pattern, dep);

					// Get useful varset //
					Varset useful;
					getUsefulVarset(useful, dep);

					SPARQLquery sparql_query;
					vector<vector<string> > encode_varset;
					vector<vector<QueryTree::GroupPattern::Pattern> > basic_query_handle;

					// Get connected block (BGP): merge the block IDs of triples with common vars //
					triple_pattern.initPatternBlockid();
					for (int j = 0; j < (int)triple_pattern.sub_group_pattern.size(); j++)
						if (triple_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
						{
							for (int k = 0; k < j; k++)
								if (triple_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
									if (triple_pattern.sub_group_pattern[j].pattern.subject_object_varset.hasCommonVar(triple_pattern.sub_group_pattern[k].pattern.subject_object_varset))
										triple_pattern.mergePatternBlockID(j, k);
						}

					// Retrieve current BGP's result if query cache hit, or else save into sparql_query //
					for (int j = 0; j < (int)triple_pattern.sub_group_pattern.size(); j++)
						if (triple_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type) {
							/// Process only the root triples of each BGP ///
							if (triple_pattern.getRootPatternBlockID(j) == j)
							{
								//// Construct occur: all vars that occur in this BGP ////
								//// Construct basic_query: vector of triples in this block ////
								Varset occur;
								vector<QueryTree::GroupPattern::Pattern> basic_query;
								for (int k = 0; k < (int)triple_pattern.sub_group_pattern.size(); k++)
									if (triple_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
										if (triple_pattern.getRootPatternBlockID(k) == j)
										{
											if (k < group_pattern_triple_num)
												occur += triple_pattern.sub_group_pattern[k].pattern.varset;

											basic_query.push_back(triple_pattern.sub_group_pattern[k].pattern);
										}

								//// Reduce useful: return result vars ////
								if (!this->query_tree.checkProjectionAsterisk() && useful.hasCommonVar(occur))
									useful = useful * occur;	// Only the common vars remain
								else
									useful = occur;

								//// Print useful vars and triple patterns ////
								printf("useful vars (SELECT + GROUPBY + ORDERBY): ");
								useful.print();
								printf("triple patterns: \n");
								for (int k = 0; k < (int)basic_query.size(); k++)
									printf("%s\t%s\t%s\n", basic_query[k].subject.value.c_str(),
										basic_query[k].predicate.value.c_str(),
										basic_query[k].object.value.c_str()
									);

								//// Check query cache for saved results of this BGP ////
			        			if(this->export_flag)
			        			{
			        				this->strategy.fp = this->fp;
			        				this->strategy.export_flag = this->export_flag;
			        			}
								bool success = false;
								if (this->query_cache != NULL && dep == 0)
									success = checkBasicQueryCache(basic_query, sub_result, useful);
								//// If query cache not hit, save the current BGP to sparql_query for later processing ////
								//// QUESTION: Is basic_query_handle a redundant variable? ////
								if (!success)
								{
									sparql_query.addBasicQuery();
									for (int k = 0; k < (int)basic_query.size(); k++)
										sparql_query.addTriple(Triple(basic_query[k].subject.value,
											basic_query[k].predicate.value,
											basic_query[k].object.value));

									encode_varset.push_back(useful.vars);
									basic_query_handle.push_back(basic_query);
								}
							}
						}

					// Encode BGPs //
					long tv_begin = Util::get_cur_time();
					sparql_query.encodeQuery(this->kvstore, encode_varset);
					long tv_encode = Util::get_cur_time();
					printf("after Encode, used %ld ms.\n", tv_encode - tv_begin);

					// Set candidate lists of common vars with the parent layer in rewriting_evaluation_stack //
					if (dep > 0)
						fillCandList(sparql_query, dep, encode_varset);
					long tv_fillcand = Util::get_cur_time();
					printf("after FillCand, used %ld ms.\n", tv_fillcand - tv_encode);

					/* PLEASE REPLACE WITH OPTIMIZER */
					QueryInfo query_info;

					query_info.limit_ = false;
					if(this->query_tree.getLimit()!=-1) {
						query_info.limit_ = true;
						query_info.limit_num_ = this->query_tree.getLimit();
					}

					query_info.is_distinct_ = this->query_tree.getProjectionModifier() == QueryTree::ProjectionModifier::Modifier_Distinct;

					query_info.ordered_by_vars_ = make_shared<vector<QueryTree::Order>>();
					for(auto order_item:this->query_tree.getOrderVarVector())
						query_info.ordered_by_vars_->push_back(order_item);

					this->optimizer_->DoQuery(sparql_query,query_info);

					long tv_handle = Util::get_cur_time();
					printf("after Handle, used %ld ms.\n", tv_handle - tv_fillcand);

					// Collect and join the result of each BasicQuery //
					// Each BGP's results are copied out to temp, and then joined with sub_result //
					for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
					{
						TempResultSet *temp = new TempResultSet();
						temp->results.push_back(TempResult());

						temp->results[0].id_varset = Varset(encode_varset[j]);
						int varnum = (int)encode_varset[j].size();

						vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(j).getResultList();
						int basicquery_result_num = (int)basicquery_result.size();

						temp->results[0].result.reserve(basicquery_result_num);
						for (int k = 0; k < basicquery_result_num; k++)
						{
							unsigned *v = new unsigned[varnum];
							memcpy(v, basicquery_result[k], sizeof(int) * varnum);
							temp->results[0].result.push_back(TempResult::ResultPair());
							temp->results[0].result.back().id = v;
						}

						if (this->query_cache != NULL && dep == 0)
						{
							//if unconnected, time is incorrect
							int time = tv_handle - tv_begin;

							long tv_bftry = Util::get_cur_time();
							bool success = this->query_cache->tryCaching(basic_query_handle[j], temp->results[0], time);
							if (success)	printf("QueryCache cached\n");
							else			printf("QueryCache didn't cache\n");
							long tv_aftry = Util::get_cur_time();
							printf("during tryCache, used %ld ms.\n", tv_aftry - tv_bftry);
						}

						if (sub_result->results.empty())
						{
							delete sub_result;
							sub_result = temp;
						}
						else
						{
							TempResultSet *new_result = new TempResultSet();
							sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

							temp->release();
							sub_result->release();
							delete temp;
							delete sub_result;

							sub_result = new_result;
						}
					}

					// Prepare varset (without this step the following will fail) //
					for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
						rewriting_evaluation_stack[dep].group_pattern.getVarset();

					// Process BIND //
					for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
						if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Bind_type)
						{
							TempResultSet *temp = new TempResultSet();
							temp->results.push_back(TempResult());

							temp->results[0].str_varset = rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].bind.varset;

							temp->results[0].result.push_back(TempResult::ResultPair());
							temp->results[0].result[0].str.push_back(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].bind.str);

							TempResultSet *new_result = new TempResultSet();
							sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

							temp->release();
							sub_result->release();
							delete temp;
							delete sub_result;

							sub_result = new_result;
						}

					// Process FILTER (with var in minimal_varset constraint) //
					for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
						if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
							if (!rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.done && \
								rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.varset. \
								belongTo(rewriting_evaluation_stack[dep].group_pattern.group_pattern_resultset_minimal_varset))
							// if (!rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.done)
							{
								rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.done = true;

								TempResultSet *new_result = new TempResultSet();
								sub_result->doFilter(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.root, *new_result, \
									this->stringindex, rewriting_evaluation_stack[dep].group_pattern.group_pattern_subject_object_maximal_varset);

								sub_result->release();
								delete sub_result;

								sub_result = new_result;
							}

					// Process OPTIONAL //
					if (sub_result->results.empty() || !sub_result->results[0].result.empty())
					{
						for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
						{
							if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
							{
								if ((int)this->rewriting_evaluation_stack.size() == dep + 1)
								{
									this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
									this->rewriting_evaluation_stack.back().result = NULL;
									// group_pattern = &this->rewriting_evaluation_stack[dep].group_pattern;
								}

								this->rewriting_evaluation_stack[dep].result = sub_result;
								this->rewriting_evaluation_stack[dep + 1].group_pattern = \
									this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].optional;
								// this->rewriting_evaluation_stack[dep + 1].group_pattern.print(0);

								// TempResultSet *temp = rewritingBasedQueryEvaluation(dep + 1);
								TempResultSet *temp = queryEvaluation(dep + 1);

								TempResultSet *new_result = new TempResultSet();
								sub_result->doOptional(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

								temp->release();
								sub_result->release();
								delete temp;
								delete sub_result;

								sub_result = new_result;
							}
						}
					}

					// Process the remaining FILTER (without constraint) //
					// QUESTION: Why separate into two stages? //
					for (int j = 0; j < (int)rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size(); j++)
					{
						if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
						{
							if (!rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.done)
							{
								rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.done = true;

								TempResultSet *new_result = new TempResultSet();
								sub_result->doFilter(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.root, *new_result, \
									this->stringindex, rewriting_evaluation_stack[dep].group_pattern.group_pattern_subject_object_maximal_varset);

								sub_result->release();
								delete sub_result;

								sub_result = new_result;
								printf("IN SECOND doFilter\n");
							}
						}
					}
				}

				if (sub_result_outer->results.empty())
				{
					delete sub_result_outer;
					sub_result_outer = sub_result;
				}
				else
				{
					TempResultSet *new_result = new TempResultSet();
					sub_result_outer->doUnion(*sub_result, *new_result);

					sub_result->release();
					sub_result_outer->release();
					delete sub_result;
					delete sub_result_outer;

					sub_result_outer = new_result;
				}
			}

			if (result->results.empty())
			{
				delete result;
				result = sub_result_outer;
			}
			else
			{
				TempResultSet *new_result = new TempResultSet();
				result->doJoin(*sub_result_outer, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				sub_result_outer->release();
				result->release();
				delete sub_result_outer;
				delete result;

				result = new_result;
			}
		}
		else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type || group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Minus_type)
		{
			this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
			this->rewriting_evaluation_stack.back().group_pattern = group_pattern.sub_group_pattern[i].optional;
			this->rewriting_evaluation_stack.back().result = NULL;
			TempResultSet *temp = queryEvaluation(dep + 1);
			{
				TempResultSet *new_result = new TempResultSet();

				if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
					result->doOptional(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);
				else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Minus_type)
					result->doMinus(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				temp->release();
				result->release();
				delete temp;
				delete result;

				result = new_result;
			}
		}
		else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
		{
			TempResultSet *new_result = new TempResultSet();
			result->doFilter(group_pattern.sub_group_pattern[i].filter.root, *new_result, this->stringindex, group_pattern.group_pattern_subject_object_maximal_varset);

			result->release();
			delete result;

			result = new_result;
		}
		else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Bind_type)
		{
			TempResultSet *temp = new TempResultSet();
			temp->results.push_back(TempResult());

			temp->results[0].str_varset = group_pattern.sub_group_pattern[i].bind.varset;

			temp->results[0].result.push_back(TempResult::ResultPair());
			temp->results[0].result[0].str.push_back(group_pattern.sub_group_pattern[i].bind.str);

			{
				TempResultSet *new_result = new TempResultSet();
				result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				temp->release();
				result->release();
				delete temp;
				delete result;

				result = new_result;
			}
		}

	// result->print();

	this->rewriting_evaluation_stack.pop_back();

	return result;
}

TempResultSet* GeneralEvaluation::semanticBasedQueryEvaluation(QueryTree::GroupPattern &group_pattern)
{
	// group_pattern.print(0);

	TempResultSet *result = new TempResultSet();

	group_pattern.initPatternBlockid();

	// Iterate across all sub-group-patterns, process according to type
	for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
		if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Group_type)
		{
			group_pattern.sub_group_pattern[i].group_pattern.print(0);
			// group_pattern.sub_group_pattern[i].group_pattern.getVarset();
			TempResultSet *temp = semanticBasedQueryEvaluation(group_pattern.sub_group_pattern[i].group_pattern);

			if (result->results.empty())
			{
				delete result;
				result = temp;
			}
			else
			{
				TempResultSet *new_result = new TempResultSet();
				result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				temp->release();
				result->release();
				delete temp;
				delete result;

				result = new_result;
			}
		}
		else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
		{
			// printf("Pattern: %s %s %s\n", group_pattern.sub_group_pattern[i].pattern.subject.value.c_str(), \
			// 	group_pattern.sub_group_pattern[i].pattern.predicate.value.c_str(), \
			// 	group_pattern.sub_group_pattern[i].pattern.object.value.c_str());
			int st = i;
			while (st > 0 && (group_pattern.sub_group_pattern[st - 1].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type || group_pattern.sub_group_pattern[st - 1].type == QueryTree::GroupPattern::SubGroupPattern::Union_type))
				st--;

			for (int j = st; j < i; j++)
				if (group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
				{
					if (group_pattern.sub_group_pattern[i].pattern.subject_object_varset.hasCommonVar(group_pattern.sub_group_pattern[j].pattern.subject_object_varset))
						group_pattern.mergePatternBlockID(i, j);
				}

			if (i + 1 == (int)group_pattern.sub_group_pattern.size() ||
				(group_pattern.sub_group_pattern[i + 1].type != QueryTree::GroupPattern::SubGroupPattern::Pattern_type && group_pattern.sub_group_pattern[i + 1].type != QueryTree::GroupPattern::SubGroupPattern::Union_type))
			{
				// Reach the end of this BGP, start merging
				SPARQLquery sparql_query;
				vector<vector<string> > encode_varset;
				vector<vector<QueryTree::GroupPattern::Pattern> > basic_query_handle;

				auto limit_num = query_tree.getLimit();
				auto order_var_vec = query_tree.getOrderVarVector();

				for (int j = st; j <= i; j++)
					if (group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
					{
						if (!group_pattern.sub_group_pattern[j].pattern.varset.empty())
						{
							if (group_pattern.getRootPatternBlockID(j) == j)		//root node, coming from mergePatternBlockID
							{
								Varset occur;
								vector<QueryTree::GroupPattern::Pattern> basic_query;

								for (int k = st; k <= i; k++)
									if (group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
										if (group_pattern.getRootPatternBlockID(k) == j)
										{
											occur += group_pattern.sub_group_pattern[k].pattern.varset;
											basic_query.push_back(group_pattern.sub_group_pattern[k].pattern);
										}

								printf("select vars: ");
								occur.print();

								printf("triple patterns: \n");
								for (int k = 0; k < (int)basic_query.size(); k++)
									printf("%s\t%s\t%s\n", basic_query[k].subject.value.c_str(),
										basic_query[k].predicate.value.c_str(),
										basic_query[k].object.value.c_str()
									);

								TempResultSet *temp = new TempResultSet();
								temp->results.push_back(TempResult());

								// bool success = false;
								// if (this->query_cache != NULL)
								// 	success = checkBasicQueryCache(basic_query, sub_result, useful);
								// //// If query cache not hit, save the current BGP to sparql_query for later processing ////
								// //// QUESTION: Is basic_query_handle a redundant variable? ////
								// if (!success)
								// {
								// 	sparql_query.addBasicQuery();
								// 	for (int k = 0; k < (int)basic_query.size(); k++)
								// 		sparql_query.addTriple(Triple(basic_query[k].subject.value,
								// 			basic_query[k].predicate.value,
								// 			basic_query[k].object.value));

								// 	encode_varset.push_back(useful.vars);
								// 	basic_query_handle.push_back(basic_query);
								// }

								bool success = false;
								if (this->query_cache != NULL)
								{
									long tv_bfcheck = Util::get_cur_time();
									success = this->query_cache->checkCached(basic_query, occur, temp->results[0]);
									long tv_afcheck = Util::get_cur_time();
									printf("after checkCache, used %ld ms.\n", tv_afcheck - tv_bfcheck);
								}

								if (success)
								{
									printf("QueryCache hit\n");
									printf("Final result size: %d\n", (int)temp->results[0].result.size());

									TempResultSet *new_result = new TempResultSet();
									result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

									result->release();
									delete result;

									result = new_result;
								}
								else
								{
									printf("QueryCache miss\n");

									sparql_query.addBasicQuery();
									for (int k = 0; k < (int)basic_query.size(); k++)
										sparql_query.addTriple(Triple(basic_query[k].subject.value,
											basic_query[k].predicate.value,
											basic_query[k].object.value));

									encode_varset.push_back(occur.vars);
									basic_query_handle.push_back(basic_query);
								}

								temp->release();
								delete temp;
							}
						}
					}

				long tv_begin = Util::get_cur_time();
				sparql_query.encodeQuery(this->kvstore, encode_varset);
				long tv_encode = Util::get_cur_time();
				printf("during Encode, used %ld ms.\n", tv_encode - tv_begin);

				// *
				// I will change this segment
				//
				// * //
				// this->strategy.handle(sparql_query);
				long tv_handle = Util::get_cur_time();
				printf("during Handle, used %ld ms.\n", tv_handle - tv_encode);

				//collect and join the result of each BasicQuery
				for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
				{
					TempResultSet *temp = new TempResultSet();
					temp->results.push_back(TempResult());

					temp->results[0].id_varset = Varset(encode_varset[j]);
					int varnum = (int)encode_varset[j].size();

					vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(j).getResultList();
					int basicquery_result_num = (int)basicquery_result.size();

					temp->results[0].result.reserve(basicquery_result_num);
					for (int k = 0; k < basicquery_result_num; k++)
					{
						unsigned *v = new unsigned[varnum];
						memcpy(v, basicquery_result[k], sizeof(int) * varnum);
						temp->results[0].result.push_back(TempResult::ResultPair());
						temp->results[0].result.back().id = v;
					}

					if (this->query_cache != NULL)
					{
						//if unconnected, time is incorrect
						int time = tv_handle - tv_begin;

						long tv_bftry = Util::get_cur_time();
						bool success = this->query_cache->tryCaching(basic_query_handle[j], temp->results[0], time);
						if (success)	printf("QueryCache cached\n");
						else			printf("QueryCache didn't cache\n");
						long tv_aftry = Util::get_cur_time();
						printf("during tryCache, used %ld ms.\n", tv_aftry - tv_bftry);
					}

					if (result->results.empty())
					{
						delete result;
						result = temp;
					}
					else
					{
						TempResultSet *new_result = new TempResultSet();
						result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

						temp->release();
						result->release();
						delete temp;
						delete result;

						result = new_result;
					}
				}
				// printf("Pattern_type result: \n");
				// result->print();
			}
		}
		else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Union_type)
		{
			TempResultSet *sub_result = new TempResultSet();

			for (int j = 0; j < (int)group_pattern.sub_group_pattern[i].unions.size(); j++)
			{
				TempResultSet *temp = semanticBasedQueryEvaluation(group_pattern.sub_group_pattern[i].unions[j]);

				if (sub_result->results.empty())
				{
					delete sub_result;
					sub_result = temp;
				}
				else
				{
					TempResultSet *new_result = new TempResultSet();
					sub_result->doUnion(*temp, *new_result);

					temp->release();
					sub_result->release();
					delete temp;
					delete sub_result;

					sub_result = new_result;
				}
			}

			if (result->results.empty())
			{
				delete result;
				result = sub_result;
			}
			else
			{
				TempResultSet *new_result = new TempResultSet();
				result->doJoin(*sub_result, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				sub_result->release();
				result->release();
				delete sub_result;
				delete result;

				result = new_result;
			}
		}
		else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type || group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Minus_type)
		{
			// printf("Optional_type\n");

			TempResultSet *temp = semanticBasedQueryEvaluation(group_pattern.sub_group_pattern[i].optional);
			// printf("Optional_type result: \n");
			// temp->print();

			{
				TempResultSet *new_result = new TempResultSet();

				if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
					result->doOptional(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);
				else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Minus_type)
					result->doMinus(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				temp->release();
				result->release();
				delete temp;
				delete result;

				result = new_result;
			}
		}
		else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
		{
			TempResultSet *new_result = new TempResultSet();
			result->doFilter(group_pattern.sub_group_pattern[i].filter.root, *new_result, this->stringindex, group_pattern.group_pattern_subject_object_maximal_varset);

			result->release();
			delete result;

			result = new_result;
		}
		else if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Bind_type)
		{
			TempResultSet *temp = new TempResultSet();
			temp->results.push_back(TempResult());

			temp->results[0].str_varset = group_pattern.sub_group_pattern[i].bind.varset;

			temp->results[0].result.push_back(TempResult::ResultPair());
			temp->results[0].result[0].str.push_back(group_pattern.sub_group_pattern[i].bind.str);

			{
				TempResultSet *new_result = new TempResultSet();
				result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				temp->release();
				result->release();
				delete temp;
				delete result;

				result = new_result;
			}
		}

	// result->print();
	return result;
}

bool GeneralEvaluation::expanseFirstOuterUnionGroupPattern(QueryTree::GroupPattern &group_pattern, deque<QueryTree::GroupPattern> &queue)
{
	int first_union_pos = -1;

	// Iterate across sub-group-patterns, find the first UNION
	for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
		if (group_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Union_type)
		{
			first_union_pos = i;
			break;
		}

	// None of the sub-group-patterns is UNION (expansion complete), exit with false
	if (first_union_pos == -1)
		return false;

	// Extract the UNION'ed sub-group-patterns, raise them one-by-one to the level of the original UNION;
	// push each newly constructed group-pattern into queue.
	// For example, if the input group-pattern is A (B UNION C) (D UNION E), then the queue has two items:
	// A B (D UNION E), A C (D UNION E)
	// Each item can still be expanded once by consecutive calls to this function (expanseFirstOuterUnionGroupPattern)
	for (int i = 0; i < (int)group_pattern.sub_group_pattern[first_union_pos].unions.size(); i++)
	{
		QueryTree::GroupPattern new_group_pattern;

		for (int j = 0; j < first_union_pos; j++)
			new_group_pattern.sub_group_pattern.push_back(group_pattern.sub_group_pattern[j]);

		for (int j = 0; j < (int)group_pattern.sub_group_pattern[first_union_pos].unions[i].sub_group_pattern.size(); j++)
			new_group_pattern.sub_group_pattern.push_back(group_pattern.sub_group_pattern[first_union_pos].unions[i].sub_group_pattern[j]);

		for (int j = first_union_pos + 1; j < (int)group_pattern.sub_group_pattern.size(); j++)
			new_group_pattern.sub_group_pattern.push_back(group_pattern.sub_group_pattern[j]);

		queue.push_back(new_group_pattern);
	}

	return true;
}

TempResultSet* GeneralEvaluation::rewritingBasedQueryEvaluation(int dep)
{
	// Construct group_pattern_union, which will consist of all expansion results //
	// (group-patterns without UNIONs, whose results will have to be UNION'ed to get //
	// the original result) //
	deque<QueryTree::GroupPattern> queue;
	queue.push_back(this->rewriting_evaluation_stack[dep].group_pattern);
	vector<QueryTree::GroupPattern> group_pattern_union;
	while (!queue.empty())
	{
		if (!this->expanseFirstOuterUnionGroupPattern(queue.front(), queue)) {
			group_pattern_union.push_back(queue.front());	// Expansion complete
		}
		queue.pop_front();
	}
	TempResultSet *result = new TempResultSet();

	// Treat each expansion result as a separate query //
	// rewriting_evaluation_stack is for OPTIONAL, each nested OPTIONAL dep + 1 //
	for (int i = 0; i < (int)group_pattern_union.size(); i++)
	{
		// Prepare and print //
		this->rewriting_evaluation_stack[dep].group_pattern = group_pattern_union[i];
		for (int j = 0; j < 80; j++)			printf("=");	printf("\n");
		rewriting_evaluation_stack[dep].group_pattern.print(dep);
		for (int j = 0; j < 80; j++)			printf("=");	printf("\n");
		TempResultSet *sub_result = new TempResultSet();

		// Construct triple_pattern //
		QueryTree::GroupPattern triple_pattern;
		int group_pattern_triple_num = constructTriplePattern(triple_pattern, dep);

		// Get useful varset //
		Varset useful;
		getUsefulVarset(useful, dep);

		SPARQLquery sparql_query;
		vector<vector<string> > encode_varset;
		vector<vector<QueryTree::GroupPattern::Pattern> > basic_query_handle;

		// Get connected block (BGP): merge the block IDs of triples with common vars //
		triple_pattern.initPatternBlockid();
		for (int j = 0; j < (int)triple_pattern.sub_group_pattern.size(); j++)
			if (triple_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
			{
				for (int k = 0; k < j; k++)
					if (triple_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
						if (triple_pattern.sub_group_pattern[j].pattern.subject_object_varset.hasCommonVar(triple_pattern.sub_group_pattern[k].pattern.subject_object_varset))
							triple_pattern.mergePatternBlockID(j, k);
			}

		// Retrieve current BGP's result if query cache hit, or else save into sparql_query //
		for (int j = 0; j < (int)triple_pattern.sub_group_pattern.size(); j++)
			if (triple_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type) {
				/// Process only the root triples of each BGP ///
				if (triple_pattern.getRootPatternBlockID(j) == j)
				{
					//// Construct occur: all vars that occur in this BGP ////
					//// Construct basic_query: vector of triples in this block ////
					Varset occur;
					vector<QueryTree::GroupPattern::Pattern> basic_query;
					for (int k = 0; k < (int)triple_pattern.sub_group_pattern.size(); k++)
						if (triple_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
							if (triple_pattern.getRootPatternBlockID(k) == j)
							{
								if (k < group_pattern_triple_num)
									occur += triple_pattern.sub_group_pattern[k].pattern.varset;

								basic_query.push_back(triple_pattern.sub_group_pattern[k].pattern);
							}

					//// Reduce useful: return result vars ////
					if (!this->query_tree.checkProjectionAsterisk() && useful.hasCommonVar(occur))
						useful = useful * occur;	// Only the common vars remain
					else
						useful = occur;

					//// Print useful vars and triple patterns ////
					printf("useful vars (SELECT + GROUPBY + ORDERBY): ");
					useful.print();
					printf("triple patterns: \n");
					for (int k = 0; k < (int)basic_query.size(); k++)
						printf("%s\t%s\t%s\n", basic_query[k].subject.value.c_str(),
							basic_query[k].predicate.value.c_str(),
							basic_query[k].object.value.c_str()
						);

					//// Check query cache for saved results of this BGP ////
        			if(this->export_flag)
        			{
        				this->strategy.fp = this->fp;
        				this->strategy.export_flag = this->export_flag;
        			}
					bool success = false;
					if (this->query_cache != NULL && dep == 0)
						success = checkBasicQueryCache(basic_query, sub_result, useful);
					//// If query cache not hit, save the current BGP to sparql_query for later processing ////
					//// QUESTION: Is basic_query_handle a redundant variable? ////
					if (!success)
					{
						sparql_query.addBasicQuery();
						for (int k = 0; k < (int)basic_query.size(); k++)
							sparql_query.addTriple(Triple(basic_query[k].subject.value,
								basic_query[k].predicate.value,
								basic_query[k].object.value));

						encode_varset.push_back(useful.vars);
						basic_query_handle.push_back(basic_query);
					}
				}
			}

		// Encode BGPs //
		long tv_begin = Util::get_cur_time();
		sparql_query.encodeQuery(this->kvstore, encode_varset);
		long tv_encode = Util::get_cur_time();
		printf("after Encode, used %ld ms.\n", tv_encode - tv_begin);

		// Set candidate lists of common vars with the parent layer in rewriting_evaluation_stack //
		if (dep > 0)
			fillCandList(sparql_query, dep, encode_varset);
		long tv_fillcand = Util::get_cur_time();
		printf("after FillCand, used %ld ms.\n", tv_fillcand - tv_encode);

		// replace this class to the new 'optimizer'
		//  this->strategy.handle(sparql_query);

      // Execute sparql_query //
      QueryInfo query_info;

      query_info.limit_ = false;
      if(this->query_tree.getLimit()!=-1) {
        query_info.limit_ = true;
        query_info.limit_num_ = this->query_tree.getLimit();
      }

      query_info.is_distinct_ = this->query_tree.getProjectionModifier() == QueryTree::ProjectionModifier::Modifier_Distinct;

      query_info.ordered_by_vars_ = make_shared<vector<QueryTree::Order>>();
      for(auto order_item:this->query_tree.getOrderVarVector())
        query_info.ordered_by_vars_->push_back(order_item);

      this->optimizer_->DoQuery(sparql_query,query_info);

		long tv_handle = Util::get_cur_time();
		printf("after Handle, used %ld ms.\n", tv_handle - tv_fillcand);

		// Collect and join the result of each BasicQuery //
		// Each BGP's results are copied out to temp, and then joined with sub_result //
		for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
		{
			TempResultSet *temp = new TempResultSet();
			temp->results.push_back(TempResult());

			temp->results[0].id_varset = Varset(encode_varset[j]);
			int varnum = (int)encode_varset[j].size();

			vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(j).getResultList();
			int basicquery_result_num = (int)basicquery_result.size();

			temp->results[0].result.reserve(basicquery_result_num);
			for (int k = 0; k < basicquery_result_num; k++)
			{
				unsigned *v = new unsigned[varnum];
				memcpy(v, basicquery_result[k], sizeof(int) * varnum);
				temp->results[0].result.push_back(TempResult::ResultPair());
				temp->results[0].result.back().id = v;
			}

			if (this->query_cache != NULL && dep == 0)
			{
				//if unconnected, time is incorrect
				int time = tv_handle - tv_begin;

				long tv_bftry = Util::get_cur_time();
				bool success = this->query_cache->tryCaching(basic_query_handle[j], temp->results[0], time);
				if (success)	printf("QueryCache cached\n");
				else			printf("QueryCache didn't cache\n");
				long tv_aftry = Util::get_cur_time();
				printf("during tryCache, used %ld ms.\n", tv_aftry - tv_bftry);
			}

			if (sub_result->results.empty())
			{
				delete sub_result;
				sub_result = temp;
			}
			else
			{
				TempResultSet *new_result = new TempResultSet();
				sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				temp->release();
				sub_result->release();
				delete temp;
				delete sub_result;

				sub_result = new_result;
			}
		}

		// Prepare varset (without this step the following will fail) //
		for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
			rewriting_evaluation_stack[dep].group_pattern.getVarset();

		// Process BIND //
		for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
			if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Bind_type)
			{
				TempResultSet *temp = new TempResultSet();
				temp->results.push_back(TempResult());

				temp->results[0].str_varset = rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].bind.varset;

				temp->results[0].result.push_back(TempResult::ResultPair());
				temp->results[0].result[0].str.push_back(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].bind.str);

				TempResultSet *new_result = new TempResultSet();
				sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				temp->release();
				sub_result->release();
				delete temp;
				delete sub_result;

				sub_result = new_result;
			}

		// Process FILTER (with var in minimal_varset constraint) //
		for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
			if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
				if (!rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.done && \
					rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.varset. \
					belongTo(rewriting_evaluation_stack[dep].group_pattern.group_pattern_resultset_minimal_varset))
				// if (!rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.done)
				{
					rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.done = true;

					TempResultSet *new_result = new TempResultSet();
					sub_result->doFilter(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.root, *new_result, \
						this->stringindex, rewriting_evaluation_stack[dep].group_pattern.group_pattern_subject_object_maximal_varset);

					sub_result->release();
					delete sub_result;

					sub_result = new_result;
				}

		// Process OPTIONAL //
		if (sub_result->results.empty() || !sub_result->results[0].result.empty())
		{
			for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
			{
				if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
				{
					if ((int)this->rewriting_evaluation_stack.size() == dep + 1)
					{
						this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
						this->rewriting_evaluation_stack.back().result = NULL;
						// group_pattern = &this->rewriting_evaluation_stack[dep].group_pattern;
					}

					this->rewriting_evaluation_stack[dep].result = sub_result;
					this->rewriting_evaluation_stack[dep + 1].group_pattern = \
						this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].optional;
					// this->rewriting_evaluation_stack[dep + 1].group_pattern.print(0);

					TempResultSet *temp = rewritingBasedQueryEvaluation(dep + 1);

					TempResultSet *new_result = new TempResultSet();
					sub_result->doOptional(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

					temp->release();
					sub_result->release();
					delete temp;
					delete sub_result;

					sub_result = new_result;
				}
			}
		}

		// Process the remaining FILTER (without constraint) //
		// QUESTION: Why separate into two stages? //
		for (int j = 0; j < (int)rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size(); j++)
		{
			if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
			{
				if (!rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.done)
				{
					rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.done = true;

					TempResultSet *new_result = new TempResultSet();
					sub_result->doFilter(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.root, *new_result, \
						this->stringindex, rewriting_evaluation_stack[dep].group_pattern.group_pattern_subject_object_maximal_varset);

					sub_result->release();
					delete sub_result;

					sub_result = new_result;
					printf("IN SECOND doFilter\n");
				}
			}
		}

		// Do UNION with the previous expansion results //
		if (result->results.empty())
		{
			delete result;
			result = sub_result;
		}
		else
		{
			TempResultSet *new_result = new TempResultSet();
			result->doUnion(*sub_result, *new_result);

			sub_result->release();
			result->release();
			delete sub_result;
			delete result;

			result = new_result;
		}
	}

	return result;
}

void GeneralEvaluation::getFinalResult(ResultSet &ret_result)
{

	if (this->temp_result == NULL)
	{
		return;
	}

	if (this->query_tree.getQueryForm() == QueryTree::Select_Query)
	{
		long t0 = Util::get_cur_time();

		if (this->temp_result->results.empty())
		{
			this->temp_result->results.push_back(TempResult());
			this->temp_result->results[0].id_varset += this->query_tree.getGroupPattern().group_pattern_resultset_maximal_varset;
		}

		Varset useful = this->query_tree.getResultProjectionVarset() + this->query_tree.getGroupByVarset() \
						+ this->query_tree.getOrderByVarset();

		if (this->query_tree.checkProjectionAsterisk())
		{
			for (int i = 0; i < (int)this->temp_result->results.size(); i++)
				useful += this->temp_result->results[i].getAllVarset();
		}

		if ((int)this->temp_result->results.size() > 1 || this->query_tree.getProjectionModifier() == QueryTree::Modifier_Distinct)
		{
			TempResultSet *new_temp_result = new TempResultSet();

			this->temp_result->doProjection1(useful, *new_temp_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

			this->temp_result->release();
			delete this->temp_result;

			this->temp_result = new_temp_result;
		}

		if (this->query_tree.checkAtLeastOneAggregateFunction() || !this->query_tree.getGroupByVarset().empty())
		{
			// vector<QueryTree::ProjectionVar> &proj = this->query_tree.getProjection();
			vector<QueryTree::ProjectionVar> proj = this->query_tree.getProjection();
			vector<string> order_vars = query_tree.getOrderByVarset().vars;
			for (string var : order_vars)
			{
				QueryTree::ProjectionVar proj_var;
				proj_var.aggregate_type = QueryTree::ProjectionVar::None_type;
				proj_var.var = var;
				proj.push_back(proj_var);
			}

			TempResultSet *new_temp_result = new TempResultSet();
			new_temp_result->results.push_back(TempResult());

			TempResult &result0 = this->temp_result->results[0];
			TempResult &new_result0 = new_temp_result->results[0];

			for (int i = 0; i < (int)proj.size(); i++)
				if (proj[i].aggregate_type == QueryTree::ProjectionVar::None_type)
				{
					if (result0.id_varset.findVar(proj[i].var))
						new_result0.id_varset.addVar(proj[i].var);
					else if (result0.str_varset.findVar(proj[i].var))
						new_result0.str_varset.addVar(proj[i].var);
				}
				else
					new_result0.str_varset.addVar(proj[i].var);

			vector<int> proj2temp((int)proj.size(), -1);
			for (int i = 0; i < (int)proj.size(); i++)
				if (proj[i].aggregate_type == QueryTree::ProjectionVar::None_type)
					proj2temp[i] = Varset(proj[i].var).mapTo(result0.getAllVarset())[0];
				else if (proj[i].aggregate_var != "*")
					proj2temp[i] = Varset(proj[i].aggregate_var).mapTo(result0.getAllVarset())[0];

			vector<int> proj2new = this->query_tree.getProjectionVarset().mapTo(new_result0.getAllVarset());

			int result0_size = (int)result0.result.size();
			vector<int> group2temp;

			if (!this->query_tree.getGroupByVarset().empty())
			{
				group2temp = this->query_tree.getGroupByVarset().mapTo(result0.getAllVarset());
				result0.sort(0, result0_size - 1, group2temp);
			}

			TempResultSet *temp_result_distinct = NULL;
			vector<int> group2distinct;

			for (int i = 0; i < (int)proj.size(); i++)
				if (proj[i].aggregate_type == QueryTree::ProjectionVar::Count_type && proj[i].distinct && proj[i].aggregate_var == "*")
				{
					temp_result_distinct = new TempResultSet();

					this->temp_result->doDistinct1(*temp_result_distinct);
					group2distinct = this->query_tree.getGroupByVarset().mapTo(temp_result_distinct->results[0].getAllVarset());
					temp_result_distinct->results[0].sort(0, (int)temp_result_distinct->results[0].result.size() - 1, group2distinct);

					break;
				}

			int result0_id_cols = result0.id_varset.getVarsetSize();
			int new_result0_id_cols = new_result0.id_varset.getVarsetSize();
			int new_result0_str_cols = new_result0.str_varset.getVarsetSize();

			if (result0_size == 0 && proj.size() == 1 && \
				proj[0].aggregate_type != QueryTree::ProjectionVar::None_type && \
				proj[0].aggregate_type != QueryTree::ProjectionVar::Count_type && \
				proj[0].aggregate_type != QueryTree::ProjectionVar::CompTree_type && \
				proj[0].aggregate_type != QueryTree::ProjectionVar::Sum_type && \
				proj[0].aggregate_type != QueryTree::ProjectionVar::Avg_type && \
				proj[0].aggregate_type != QueryTree::ProjectionVar::Min_type && \
				proj[0].aggregate_type != QueryTree::ProjectionVar::Max_type)
			{
				// Path query, both nodes are IRI

				new_result0.result.push_back(TempResult::ResultPair());
				new_result0.result.back().id = new unsigned[new_result0_id_cols];
				new_result0.result.back().str.resize(new_result0_str_cols);

				for (int i = 0; i < new_result0_id_cols; i++)
					new_result0.result.back().id[i] = INVALID;

				prepPathQuery();
				vector<int> uid_ls, vid_ls;
				vector<int> pred_id_set;
				uid_ls.push_back(kvstore->getIDByString(proj[0].path_args.src));
				vid_ls.push_back(kvstore->getIDByString(proj[0].path_args.dst));
				if (!proj[0].path_args.pred_set.empty())
				{
					for (auto pred : proj[0].path_args.pred_set)
						pred_id_set.push_back(kvstore->getIDByPredicate(pred));
				}
				else
				{
					// Allow all predicates
					unsigned pre_num = stringindex->getNum(StringIndexFile::Predicate);
					for (unsigned j = 0; j < pre_num; j++)
						pred_id_set.push_back(j);
				}

				// For each u-v pair, query
				bool exist = 0, earlyBreak = 0;	// Boolean queries can break early with true
				stringstream ss;
				bool notFirstOutput = 0;	// For outputting commas
				ss << "\"{\"paths\":[";
				for (int uid : uid_ls)
				{
					for (int vid : vid_ls)
					{
						if (proj[0].aggregate_type == QueryTree::ProjectionVar::cyclePath_type)
						{
							if (uid == vid)
								continue;
							vector<int> path = pqHandler->cycle(uid, vid, proj[0].path_args.directed, pred_id_set);
							if (path.size() != 0)
							{
								if (notFirstOutput)
									ss << ",";
								else
									notFirstOutput = 1;
								pathVec2JSON(uid, vid, path, ss);
							}
						}
						else if (proj[0].aggregate_type == QueryTree::ProjectionVar::cycleBoolean_type)
						{
							if (uid == vid)
								continue;
							if (pqHandler->cycle(uid, vid, proj[0].path_args.directed, pred_id_set).size() != 0)
							{
								exist = 1;
								earlyBreak = 1;
								break;
							}
						}
						else if (proj[0].aggregate_type == QueryTree::ProjectionVar::shortestPath_type)
						{
							if (uid == vid)
							{
								if (notFirstOutput)
									ss << ",";
								else
									notFirstOutput = 1;
								vector<int> path;	// Empty path
								pathVec2JSON(uid, vid, path, ss);
								continue;
							}
							vector<int> path = pqHandler->shortestPath(uid, vid, proj[0].path_args.directed, pred_id_set);
							if (path.size() != 0)
							{
								if (notFirstOutput)
									ss << ",";
								else
									notFirstOutput = 1;
								pathVec2JSON(uid, vid, path, ss);
							}
						}
						else if (proj[0].aggregate_type == QueryTree::ProjectionVar::shortestPathLen_type)
						{
							if (uid == vid)
							{
								if (notFirstOutput)
									ss << ",";
								else
									notFirstOutput = 1;
								ss << "{\"src\":\"" << kvstore->getStringByID(uid) \
									<< "\",\"dst\":\"" << kvstore->getStringByID(vid) \
									<< "\",\"length\":0}";
								continue;
							}
							vector<int> path = pqHandler->shortestPath(uid, vid, proj[0].path_args.directed, pred_id_set);
							if (path.size() != 0)
							{
								if (notFirstOutput)
									ss << ",";
								else
									notFirstOutput = 1;
								ss << "{\"src\":\"" << kvstore->getStringByID(uid) \
									<< "\",\"dst\":\"" << kvstore->getStringByID(vid) << "\",\"length\":";
								ss << (path.size() - 1) / 2;
								ss << "}";
							}
						}
						else if (proj[0].aggregate_type == QueryTree::ProjectionVar::kHopReachable_type)
						{
							if (uid == vid)
							{
								if (notFirstOutput)
									ss << ",";
								else
									notFirstOutput = 1;
								ss << "{\"src\":\"" << kvstore->getStringByID(uid) \
									<< "\",\"dst\":\"" << kvstore->getStringByID(vid) \
									<< "\",\"value\":\"true\"}";
								continue;
							}
							int hopConstraint = proj[0].path_args.k;
							if (hopConstraint < 0)
								hopConstraint = 999;
							bool reachRes = pqHandler->kHopReachable(uid, vid, proj[0].path_args.directed, hopConstraint, pred_id_set);
							ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"dst\":\"" \
								<< kvstore->getStringByID(vid) << "\",\"value\":";
							if (reachRes)
								ss << "\"true\"}";
							else
								ss << "\"false\"}";
							cout << "src = " << kvstore->getStringByID(uid) << ", dst = " << kvstore->getStringByID(vid) << endl;
						}
					}
					if (earlyBreak)
						break;
				}
				ss << "]}\"";
				if (proj[0].aggregate_type == QueryTree::ProjectionVar::cycleBoolean_type)
				{
					if (exist)
						new_result0.result.back().str[proj2new[0] - new_result0_id_cols] = "true";
					else
						new_result0.result.back().str[proj2new[0] - new_result0_id_cols] = "false";
				}
				else
					ss >> new_result0.result.back().str[proj2new[0] - new_result0_id_cols];
			}

			// Exclusive with the if branch above
			for (int begin = 0; begin < result0_size;)
			{
				int end;
				if (group2temp.empty())
					end = result0_size - 1;
				else
					end = result0.findRightBounder(group2temp, result0.result[begin], result0_id_cols, group2temp);

				new_result0.result.push_back(TempResult::ResultPair());
				new_result0.result.back().id = new unsigned[new_result0_id_cols];
				new_result0.result.back().str.resize(new_result0_str_cols);

				for (int i = 0; i < new_result0_id_cols; i++)
					new_result0.result.back().id[i] = INVALID;

				for (int i = 0; i < (int)proj.size(); i++)
				{
					if (proj[i].aggregate_type == QueryTree::ProjectionVar::None_type)
					{
						if (proj2temp[i] < result0_id_cols)
							new_result0.result.back().id[proj2new[i]] = result0.result[begin].id[proj2temp[i]];
						else
							new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = result0.result[begin].str[proj2temp[i] - result0_id_cols];
					}
					else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Count_type
						|| proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
						|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type
						|| proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type
						|| proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
					{
						int count = 0;
						bool numeric = false, datetime = false, succ = true;
						EvalMultitypeValue numeric_sum, numeric_min, numeric_max, datetime_min, datetime_max, tmp, res;
						numeric_sum.datatype = EvalMultitypeValue::xsd_integer;
						numeric_sum.int_value = 0;
						numeric_min.datatype = EvalMultitypeValue::xsd_integer;
						numeric_min.int_value = INT_MAX;
						numeric_max.datatype = EvalMultitypeValue::xsd_integer;
						numeric_max.int_value = INT_MIN;
						datetime_min.datatype = EvalMultitypeValue::xsd_datetime;
						datetime_max.datatype = EvalMultitypeValue::xsd_datetime;

						if (proj[i].aggregate_var != "*")
						{
							if (proj[i].distinct)
							{
								if (proj2temp[i] < result0_id_cols)
								{
									// set<int> count_set;
									set<string> count_set;
									for (int j = begin; j <= end; j++)
									{
										if (result0.result[j].id[proj2temp[i]] != INVALID)
										{
											bool isel = query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset.findVar(proj[i].aggregate_var);
											stringindex->randomAccess(result0.result[j].id[proj2temp[i]], &tmp.term_value, isel);
											tmp.deduceTypeValue();
											if (tmp.datatype != EvalMultitypeValue::xsd_integer
												&& tmp.datatype != EvalMultitypeValue::xsd_decimal
												&& tmp.datatype != EvalMultitypeValue::xsd_float
												&& tmp.datatype != EvalMultitypeValue::xsd_double)
											{
												if (proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
													|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
												{
													cout << "[ERROR] Invalid type for SUM or AVG." << endl;
													continue;
												}
												else if ((proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type
													|| proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
													&& tmp.datatype != EvalMultitypeValue::xsd_datetime)
												{
													cout << "[ERROR] Invalid type for MIN or MAX." << endl;
													continue; 
												}
											}
											if (tmp.datatype == EvalMultitypeValue::xsd_datetime)
											{
												if (!datetime)
													datetime = true;
											}
											else
											{
												if (!numeric)
													numeric = true;
											}
											if (datetime && numeric && proj[i].aggregate_type != QueryTree::ProjectionVar::Count_type)
											{
												succ = false;
												break;
											}

											if (proj[i].aggregate_type == QueryTree::ProjectionVar::Count_type
												|| proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
												|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
												count_set.insert(tmp.term_value);
											else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type)
											{
												if (numeric)
												{
													res = numeric_min > tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														numeric_min = tmp;
												}
												else if (datetime)
												{
													res = datetime_min > tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														datetime_min = tmp;
												}
											}
											else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
											{
												if (numeric)
												{
													res = numeric_max < tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														numeric_max = tmp;
												}
												else if (datetime)
												{
													res = datetime_max < tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														datetime_max = tmp;
												}
											}
										}
									}
									
									// Obtained count_set
									count = (int)count_set.size();
									if (proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
										|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
									{
										for (string elem : count_set)
										{
											tmp.term_value = elem;
											tmp.deduceTypeValue();

											numeric_sum = numeric_sum + tmp;
										}
									}
									if (proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
									{
										tmp.term_value = "\"" + to_string(count) + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
										tmp.deduceTypeValue();

										numeric_sum = numeric_sum / tmp;
									}
								}
								else
								{
									set<string> count_set;
									for (int j = begin; j <= end; j++)
									{
										if (result0.result[j].str[proj2temp[i] - result0_id_cols].length() > 0)
										{
											tmp.term_value = result0.result[j].str[proj2temp[i] - result0_id_cols];
											tmp.deduceTypeValue();
											if (tmp.datatype != EvalMultitypeValue::xsd_integer
												&& tmp.datatype != EvalMultitypeValue::xsd_decimal
												&& tmp.datatype != EvalMultitypeValue::xsd_float
												&& tmp.datatype != EvalMultitypeValue::xsd_double)
											{
												if (proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
													|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
												{
													cout << "[ERROR] Invalid type for SUM or AVG." << endl;
													continue;
												}
												else if ((proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type
													|| proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
													&& tmp.datatype != EvalMultitypeValue::xsd_datetime)
												{
													cout << "[ERROR] Invalid type for MIN or MAX." << endl;
													continue; 
												}
											}
											if (tmp.datatype == EvalMultitypeValue::xsd_datetime)
											{
												if (!datetime)
													datetime = true;
											}
											else
											{
												if (!numeric)
													numeric = true;
											}
											if (datetime && numeric && proj[i].aggregate_type != QueryTree::ProjectionVar::Count_type)
											{
												succ = false;
												break;
											}

											if (proj[i].aggregate_type == QueryTree::ProjectionVar::Count_type)
												count_set.insert(result0.result[j].str[proj2temp[i] - result0_id_cols]);
											else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type)
											{
												if (numeric)
												{
													res = numeric_min > tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														numeric_min = tmp;
												}
												else if (datetime)
												{
													res = datetime_min > tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														datetime_min = tmp;
												}
											}
											else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
											{
												if (numeric)
												{
													res = numeric_max < tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														numeric_max = tmp;
												}
												else if (datetime)
												{
													res = datetime_max < tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														datetime_max = tmp;
												}
											}

											count_set.insert(result0.result[j].str[proj2temp[i] - result0_id_cols]);
										}
									}

									// Obtained count_set
									count = (int)count_set.size();
									if (proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
										|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
									{
										for (string elem : count_set)
										{
											tmp.term_value = elem;
											tmp.deduceTypeValue();

											numeric_sum = numeric_sum + tmp;
										}
									}
									if (proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
									{
										tmp.term_value = "\"" + to_string(count) + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
										tmp.deduceTypeValue();

										numeric_sum = numeric_sum / tmp;
									}
								}
							}
							else 	// No DISTINCT
							{
								if (proj2temp[i] < result0_id_cols)
								{
									for (int j = begin; j <= end; j++)
									{
										if (result0.result[j].id[proj2temp[i]] != INVALID)
										{
											bool isel = query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset.findVar(proj[i].aggregate_var);
											stringindex->randomAccess(result0.result[j].id[proj2temp[i]], &tmp.term_value, isel);
											tmp.deduceTypeValue();
											if (tmp.datatype != EvalMultitypeValue::xsd_integer
												&& tmp.datatype != EvalMultitypeValue::xsd_decimal
												&& tmp.datatype != EvalMultitypeValue::xsd_float
												&& tmp.datatype != EvalMultitypeValue::xsd_double)
											{
												if (proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
													|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
												{
													cout << "[ERROR] Invalid type for SUM or AVG." << endl;
													continue;
												}
												else if ((proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type
													|| proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
													&& tmp.datatype != EvalMultitypeValue::xsd_datetime)
												{
													cout << "[ERROR] Invalid type for MIN or MAX." << endl;
													continue; 
												}
											}
											if (tmp.datatype == EvalMultitypeValue::xsd_datetime)
											{
												if (!datetime)
													datetime = true;
											}
											else
											{
												if (!numeric)
													numeric = true;
											}
											if (datetime && numeric && proj[i].aggregate_type != QueryTree::ProjectionVar::Count_type)
											{
												succ = false;
												break;
											}

											if (proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
												|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
											{
												numeric_sum = numeric_sum + tmp;
												cout << "numeric_sum.term_value = " << numeric_sum.term_value << endl;
											}
											else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type)
											{
												if (numeric)
												{
													res = numeric_min > tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														numeric_min = tmp;
												}
												else if (datetime)
												{
													res = datetime_min > tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														datetime_min = tmp;
												}
											}
											else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
											{
												if (numeric)
												{
													res = numeric_max < tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														numeric_max = tmp;
												}
												else if (datetime)
												{
													res = datetime_max < tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														datetime_max = tmp;
												}
											}

											count++;
										}
									}
									if (proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
									{
										tmp.term_value = "\"" + to_string(count) + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
										tmp.deduceTypeValue();

										numeric_sum = numeric_sum / tmp;
									}
								}
								else
								{
									for (int j = begin; j <= end; j++)
									{
										if (result0.result[j].str[proj2temp[i] - result0_id_cols].length() > 0)
										{
											tmp.term_value = result0.result[j].str[proj2temp[i] - result0_id_cols];
											tmp.deduceTypeValue();
											if (tmp.datatype != EvalMultitypeValue::xsd_integer
												&& tmp.datatype != EvalMultitypeValue::xsd_decimal
												&& tmp.datatype != EvalMultitypeValue::xsd_float
												&& tmp.datatype != EvalMultitypeValue::xsd_double)
											{
												if (proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
													|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
												{
													cout << "[ERROR] Invalid type for SUM or AVG." << endl;
													continue;
												}
												else if ((proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type
													|| proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
													&& tmp.datatype != EvalMultitypeValue::xsd_datetime)
												{
													cout << "[ERROR] Invalid type for MIN or MAX." << endl;
													continue; 
												}
											}
											if (tmp.datatype == EvalMultitypeValue::xsd_datetime)
											{
												if (!datetime)
													datetime = true;
											}
											else
											{
												if (!numeric)
													numeric = true;
											}
											if (datetime && numeric && proj[i].aggregate_type != QueryTree::ProjectionVar::Count_type)
											{
												succ = false;
												break;
											}

											if (proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
												|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
											{
												numeric_sum = numeric_sum + tmp;
												cout << "numeric_sum.term_value = " << numeric_sum.term_value << endl;
											}
											else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type)
											{
												if (numeric)
												{
													res = numeric_min > tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														numeric_min = tmp;
												}
												else if (datetime)
												{
													res = datetime_min > tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														datetime_min = tmp;
												}
											}
											else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
											{
												if (numeric)
												{
													res = numeric_max < tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														numeric_max = tmp;
												}
												else if (datetime)
												{
													res = datetime_max < tmp;
													if (res.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::true_value)
														datetime_max = tmp;
												}
											}

											count++;
										}
									}

									if (proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
									{
										tmp.term_value = "\"" + to_string(count) + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
										tmp.deduceTypeValue();

										numeric_sum = numeric_sum / tmp;
									}
								}
							}
						}
						else 	// Only COUNT is possible
						{
							if (proj[i].distinct)
							{
								count = temp_result_distinct->results[0].findRightBounder(group2distinct, result0.result[begin], result0_id_cols, group2temp) -
									temp_result_distinct->results[0].findLeftBounder(group2distinct, result0.result[begin], result0_id_cols, group2temp) + 1;
							}
							else
							{
								count = end - begin + 1;
							}
						}

						// Write aggregate result
						stringstream ss;

						if (proj[i].aggregate_type == QueryTree::ProjectionVar::Count_type)
						{
							ss << "\"";
							ss << count;
							ss << "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
						}
						else if (succ)
						{
							if (proj[i].aggregate_type == QueryTree::ProjectionVar::Sum_type
								|| proj[i].aggregate_type == QueryTree::ProjectionVar::Avg_type)
							{
								cout << "numeric_sum.term_value = " << numeric_sum.term_value << endl;
								ss << numeric_sum.term_value;
							}
							else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Min_type)
							{
								if (numeric)
									ss << numeric_min.term_value;
								else if (datetime)
									ss << datetime_min.term_value;
							}
							else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Max_type)
							{
								if (numeric)
									ss << numeric_max.term_value;
								else if (datetime)
									ss << datetime_max.term_value;
							}
							
						}
						else 	// Failed
							ss << "";

						ss >> new_result0.result.back().str[proj2new[i] - new_result0_id_cols];
					}
					else if (proj[i].aggregate_type == QueryTree::ProjectionVar::CompTree_type)
					{
						// Strictly speaking, not an aggregate; each original line will produce a line of results
						for (int j = begin; j <= end; j++)
						{
							new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = \
								result0.doComp(proj[i].comp_tree_root, result0.result[j], result0_id_cols, stringindex, \
								query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset).term_value;
							if (j < end)
							{
								new_result0.result.push_back(TempResult::ResultPair());
								new_result0.result.back().id = new unsigned[new_result0_id_cols];
								new_result0.result.back().str.resize(new_result0_str_cols);
							}
						}
					}
					else if (proj[i].aggregate_type == QueryTree::ProjectionVar::Contains_type)
					{
						if (proj[i].builtin_args.size() == 2)
						{
							string arg1 = proj[i].builtin_args[0], arg2 = proj[i].builtin_args[1];
							int pos1 = -2, pos2 = -2;	// -2 for string, -1 for error var, others for correct var
														// But < result0_id_cols is impossible, because must be string
							bool isel1 = false, isel2 = false;
							string big_str = "", small_str = "";
							if (arg1[0] == '?')
							{
								pos1 = Varset(arg1).mapTo(query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset)[0];
								if (pos1 == -1)
								{
									cout << "[ERROR] Cannot get arg1 in CONTAINS." << endl;
									break;
								}
								isel1 = query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset.findVar(arg1);
							}
							else if (arg1[0] == '\"')
							{
								big_str = arg1;
								int idx = big_str.length() - 1;
								while (idx >= 0 && big_str[idx] != '"')
									idx--;
								if (idx >= 1)
									big_str = big_str.substr(1, idx - 1);
							}
							
							if (arg2[0] == '?')
							{
								pos2 = Varset(arg2).mapTo(query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset)[0];
								if (pos2 == -1)
								{
									cout << "[ERROR] Cannot get arg2 in CONTAINS." << endl;
									break;
								}
								isel2 = query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset.findVar(arg2);
							}
							else if (arg2[0] == '\"')
							{
								small_str = arg2;
								int idx = small_str.length() - 1;
								while (idx >= 0 && small_str[idx] != '"')
									idx--;
								if (idx >= 1)
									small_str = small_str.substr(1, idx - 1);
							}

							for (int j = begin; j <= end; j++)
							{
								if (pos1 >= 0)
								{
									if (pos1 < result0_id_cols)
										stringindex->randomAccess(result0.result[j].id[pos1], &big_str, isel1);
									else
										big_str = result0.result[j].str[pos1 - result0_id_cols];
									int idx = big_str.length() - 1;
									while (idx >= 0 && big_str[idx] != '"')
										idx--;
									if (idx >= 1)
										big_str = big_str.substr(1, idx - 1);
								}
								if (pos2 >= 0)
								{
									if (pos2 < result0_id_cols)
										stringindex->randomAccess(result0.result[j].id[pos2], &small_str, isel2);
									else
										small_str = result0.result[j].str[pos2 - result0_id_cols];
									int idx = small_str.length() - 1;
									while (idx >= 0 && small_str[idx] != '"')
										idx--;
									if (idx >= 1)
										small_str = small_str.substr(1, idx - 1);
								}

								cout << "big_str = " << big_str << endl;
								cout << "small_str = " << small_str << endl;

								if (big_str.find(small_str) != string::npos)
									new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = \
										"\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
								else
									new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = \
										"\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>";

								if (j < end)
								{
									new_result0.result.push_back(TempResult::ResultPair());
									new_result0.result.back().id = new unsigned[new_result0_id_cols];
									new_result0.result.back().str.resize(new_result0_str_cols);
								}
							}
						}
					}
					else	// Path query
					{
						prepPathQuery();

						vector<int> uid_ls, vid_ls;
						vector<int> pred_id_set;

						// uid
						if (proj[i].path_args.src[0] == '?')	// src is a variable
						{
							int var2temp = Varset(proj[i].path_args.src).mapTo(result0.getAllVarset())[0];
							if (var2temp >= result0_id_cols)
								cout << "[ERROR] src must be an entity!" << endl;	// TODO: throw exception
							else
							{
								for (int j = begin; j <= end; j++)
								{
									if (result0.result[j].id[var2temp] != INVALID)
										uid_ls.push_back(result0.result[j].id[var2temp]);
								}
							}
						}
						else	// src is an IRI
							uid_ls.push_back(kvstore->getIDByString(proj[i].path_args.src));

						cout << "uid: ";
						for (int uid : uid_ls)
							cout << uid << ' ';
						cout << endl;

						// vid
						if (proj[i].path_args.dst[0] == '?')	// dst is a variable
						{
							int var2temp = Varset(proj[i].path_args.dst).mapTo(result0.getAllVarset())[0];
							cout << "vid var2temp = " << var2temp << endl;
							if (var2temp >= result0_id_cols)
								cout << "[ERROR] dst must be an entity!" << endl;	// TODO: throw exception
							else
							{
								for (int j = begin; j <= end; j++)
								{
									if (result0.result[j].id[var2temp] != INVALID)
										vid_ls.push_back(result0.result[j].id[var2temp]);
								}
							}
						}
						else	// dst is an IRI
							vid_ls.push_back(kvstore->getIDByString(proj[i].path_args.dst));

						cout << "vid: ";
						for (int vid : vid_ls)
							cout << vid << ' ';
						cout << endl;

						// pred_id_set: convert from IRI to integer ID
						if (!proj[i].path_args.pred_set.empty())
						{
							for (auto pred : proj[i].path_args.pred_set)
								pred_id_set.push_back(kvstore->getIDByPredicate(pred));
						}
						else
						{
							// Allow all predicates
							unsigned pre_num = stringindex->getNum(StringIndexFile::Predicate);
							for (unsigned j = 0; j < pre_num; j++)
								pred_id_set.push_back(j);
						}

						// For each u-v pair, query
						bool exist = 0, earlyBreak = 0;	// Boolean queries can break early with true
						stringstream ss;
						bool notFirstOutput = 0;	// For outputting commas
						ss << "\"{\"paths\":[";
						for (int uid : uid_ls)
						{
							for (int vid : vid_ls)
							{
								if (proj[i].aggregate_type == QueryTree::ProjectionVar::cyclePath_type)
								{
									if (uid == vid)
										continue;
									vector<int> path = pqHandler->cycle(uid, vid, proj[i].path_args.directed, pred_id_set);
									if (path.size() != 0)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										pathVec2JSON(uid, vid, path, ss);
									}
								}
								else if (proj[i].aggregate_type == QueryTree::ProjectionVar::cycleBoolean_type)
								{
									if (uid == vid)
										continue;
									if (pqHandler->cycle(uid, vid, proj[i].path_args.directed, pred_id_set).size() != 0)
									{
										exist = 1;
										earlyBreak = 1;
										break;
									}
								}
								else if (proj[i].aggregate_type == QueryTree::ProjectionVar::shortestPath_type)
								{
									if (uid == vid)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										vector<int> path;	// Empty path
										pathVec2JSON(uid, vid, path, ss);
										continue;
									}
									vector<int> path = pqHandler->shortestPath(uid, vid, proj[0].path_args.directed, pred_id_set);
									if (path.size() != 0)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										pathVec2JSON(uid, vid, path, ss);
									}
								}
								else if (proj[i].aggregate_type == QueryTree::ProjectionVar::shortestPathLen_type)
								{
									if (uid == vid)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										ss << "{\"src\":\"" << kvstore->getStringByID(uid) \
											<< "\",\"dst\":\"" << kvstore->getStringByID(vid) \
											<< "\",\"length\":0}";
										continue;
									}
									vector<int> path = pqHandler->shortestPath(uid, vid, proj[0].path_args.directed, pred_id_set);
									if (path.size() != 0)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										ss << "{\"src\":\"" << kvstore->getStringByID(uid) \
											<< "\",\"dst\":\"" << kvstore->getStringByID(vid) << "\",\"length\":";
										ss << (path.size() - 1) / 2;
										ss << "}";
									}
								}
								else if (proj[i].aggregate_type == QueryTree::ProjectionVar::kHopReachable_type)
								{
									if (uid == vid)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										ss << "{\"src\":\"" << kvstore->getStringByID(uid) \
											<< "\",\"dst\":\"" << kvstore->getStringByID(vid) \
											<< "\",\"value\":\"true\"}";
										continue;
									}
									int hopConstraint = proj[0].path_args.k;
									if (hopConstraint < 0)
										hopConstraint = 999;
									bool reachRes = pqHandler->kHopReachable(uid, vid, proj[0].path_args.directed, hopConstraint, pred_id_set);
									ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"dst\":\"" \
										<< kvstore->getStringByID(vid) << "\",\"value\":";
									cout << "src = " << kvstore->getStringByID(uid) << ", dst = " << kvstore->getStringByID(vid) << endl;
									if (reachRes)
										ss << "\"true\"}";
									else
										ss << "\"false\"}";
								}
									}
									if (earlyBreak)
										break;
								}
								ss << "]}\"";
								if (proj[i].aggregate_type == QueryTree::ProjectionVar::cycleBoolean_type)
								{
									if (exist)
										new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = "true";
									else
										new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = "false";
								}
								else
									ss >> new_result0.result.back().str[proj2new[i] - new_result0_id_cols];
							}
				}
				begin = end + 1;
			}

			if (temp_result_distinct != NULL)
			{
				temp_result_distinct->release();
				delete temp_result_distinct;
			}

			this->temp_result->release();
			delete this->temp_result;

			this->temp_result = new_temp_result;
		}

		//temp_result --> ret_result

		if (this->query_tree.getProjectionModifier() == QueryTree::Modifier_Distinct)
		{
			// cout << "Modifier_Distinct!!!" << endl;

			TempResultSet *new_temp_result = new TempResultSet();

			this->temp_result->doDistinct1(*new_temp_result);

			this->temp_result->release();
			delete this->temp_result;

			this->temp_result = new_temp_result;
		}

		TempResult &result0 = this->temp_result->results[0];
		Varset ret_result_varset;

		if (this->query_tree.checkProjectionAsterisk() && this->query_tree.getProjectionVarset().empty())
		{
			ret_result.setVar(result0.getAllVarset().vars);
			ret_result_varset = result0.getAllVarset();
			ret_result.select_var_num = result0.getAllVarset().getVarsetSize();
			ret_result.true_select_var_num = ret_result.select_var_num;
		}
		else
		{
			vector<string> proj_vars = query_tree.getProjectionVarset().vars;
			vector<string> order_vars = query_tree.getOrderByVarset().vars;
			proj_vars.insert(proj_vars.end(), order_vars.begin(), order_vars.end());
			ret_result.setVar(proj_vars);
			ret_result_varset = query_tree.getProjectionVarset() + query_tree.getOrderByVarset();
			ret_result.select_var_num = ret_result_varset.getVarsetSize();
			ret_result.true_select_var_num = query_tree.getProjectionVarset().getVarsetSize();
		}

		ret_result.ansNum = (int)result0.result.size();
		ret_result.setOutputOffsetLimit(this->query_tree.getOffset(), this->query_tree.getLimit());

#ifdef STREAM_ON
		long long ret_result_size = (long long)ret_result.ansNum * (long long)ret_result.select_var_num * 100 / Util::GB;
		if (Util::memoryLeft() < ret_result_size || !this->query_tree.getOrderVarVector().empty())
		{
			ret_result.setUseStream();
			printf("set use Stream\n");
		}
#endif

		if (!ret_result.checkUseStream())
		{
			ret_result.answer = new string*[ret_result.ansNum];
			for (unsigned i = 0; i < ret_result.ansNum; i++)
				ret_result.answer[i] = NULL;
		}
		else
		{
			vector <unsigned> keys;
			vector <bool> desc;
			for (int i = 0; i < (int)this->query_tree.getOrderVarVector().size(); i++)
			{
				// int var_id = Varset(this->query_tree.getOrderVarVector()[i].var).mapTo(ret_result_varset)[0];
				// Temporary, to be changed to allow for more than one var in one ORDER BY condition
				int var_id = this->query_tree.getOrderVarVector()[i].comp_tree_root->getCompTreeVarset().mapTo(ret_result_varset)[0];
				if (var_id != -1)
				{
					keys.push_back(var_id);
					desc.push_back(this->query_tree.getOrderVarVector()[i].descending);
				}
			}
			ret_result.openStream(keys, desc);
		}

		vector<int> proj2temp = ret_result_varset.mapTo(result0.getAllVarset());
		int id_cols = result0.id_varset.getVarsetSize();

		vector<bool> isel;
		for (int i = 0; i < result0.id_varset.getVarsetSize(); i++)
			isel.push_back(this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset.findVar(result0.id_varset.vars[i]));

		if (!ret_result.checkUseStream())
		{

			//pthread_t tidp;
			//long arg[6];
			vector<StringIndexFile* > a = this->stringindex->get_three_StringIndexFile();
			/*arg[0] = (long)&a;
			arg[1] = (long)&ret_result;
			arg[2] = (long)&proj2temp;
			arg[3] = (long)id_cols;
			arg[4] = (long)&result0;
			arg[5] = (long)&isel;
			pthread_create(&tidp, NULL, &preread_from_index, arg);*/

			unsigned retAnsNum = ret_result.ansNum;
			unsigned selectVar = ret_result.select_var_num;
			/*
			int counterEntity = 0;
			int counterLiteral = 0;
			int counterPredicate = 0;

			for (int j = 0; j < selectVar; j++)
			{
				int k = proj2temp[j];
				if (k != -1)
				{
					if (k < id_cols)
					{
						if (isel[k])
						{
							for (unsigned i = 0; i < retAnsNum; i++)
							{
								unsigned ans_id = result0.result[i].id[k];
								if (ans_id == INVALID)
									continue;
								if (ans_id < Util::LITERAL_FIRST_ID)
									counterEntity++;
								else
									counterLiteral++;
							}
						}
						else
							for (unsigned i = 0; i < retAnsNum; i++)
							{
								unsigned ans_id = result0.result[i].id[k];
								if (ans_id == INVALID)
									continue;
								counterPredicate++;
							}
					}
				}
			}
			a[0]->request_reserve(counterEntity);
			a[1]->request_reserve(counterLiteral);
			a[2]->request_reserve(counterPredicate);*/
			
			ret_result.delete_another_way = 1;
			string *t = new string[retAnsNum*selectVar];
			for (unsigned int i = 0,off =0 ; i < retAnsNum; i++, off += selectVar)
				ret_result.answer[i] = t + off;

			a[0]->set_string_base(t);
			a[1]->set_string_base(t);
			a[2]->set_string_base(t);


			for (int j = 0; j < selectVar; j++)
			{
				int k = proj2temp[j];
				if (k != -1)
				{
					if (k < id_cols)
					{
						if (isel[k])
						{
							for (unsigned i = 0; i < retAnsNum; i++)
							{
								unsigned ans_id = result0.result[i].id[k];
								if (ans_id != INVALID)
								{
									if (ans_id < Util::LITERAL_FIRST_ID)
										a[0]->addRequest(ans_id, i*selectVar + j);
									else
										a[1]->addRequest(ans_id - Util::LITERAL_FIRST_ID , i*selectVar + j);
								}
							}
						}
						else
						{
							for (unsigned i = 0; i < retAnsNum; i++)
							{
								unsigned ans_id = result0.result[i].id[k];
								if (ans_id != INVALID)
									a[2]->addRequest(ans_id, i*selectVar + j);
							}
						}
					}
					else
					{
						for (unsigned i = 0; i < retAnsNum; i++)
						{
							ret_result.answer[i][j] = result0.result[i].str[k - id_cols];
							// Up to this point the backslashes are hidden
						}
					}
				}
			}
			//cout << "in getFinal Result the first half use " << Util::get_cur_time() - t0 << "  ms" << endl;
			//pthread_join(tidp, NULL);
			this->stringindex->trySequenceAccess(true, -1);
		}
		else
		{
			for (unsigned i = 0; i < ret_result.ansNum; i++)
				for (int j = 0; j < ret_result.select_var_num; j++)
				{
					int k = proj2temp[j];
					if (k != -1)
					{
						if (k < id_cols)
						{
							string ans_str;

							unsigned ans_id = result0.result[i].id[k];
							if (ans_id != INVALID)
							{
								this->stringindex->randomAccess(ans_id, &ans_str, isel[k]);
							}
							ret_result.writeToStream(ans_str);
						}
						else
						{
							string ans_str = result0.result[i].str[k - id_cols];
							ret_result.writeToStream(ans_str);
						}
					}
				}

			ret_result.resetStream();
		}
	}

	else if (this->query_tree.getQueryForm() == QueryTree::Ask_Query)
	{
		ret_result.true_select_var_num = ret_result.select_var_num = 1;
		ret_result.setVar(vector<string>(1, "?_askResult"));
		ret_result.ansNum = 1;

		if (!ret_result.checkUseStream())
		{
			ret_result.answer = new string*[ret_result.ansNum];
			ret_result.answer[0] = new string[ret_result.select_var_num];

			ret_result.answer[0][0] = "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
			for (int i = 0; i < (int)this->temp_result->results.size(); i++)
				if (!this->temp_result->results[i].result.empty())
					ret_result.answer[0][0] = "\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
		}
	}

	this->releaseResult();
}

void GeneralEvaluation::releaseResult()
{
	if (this->temp_result == NULL)
		return;

	this->temp_result->release();
	delete this->temp_result;
	this->temp_result = NULL;
}

void GeneralEvaluation::prepareUpdateTriple(QueryTree::GroupPattern &update_pattern, TripleWithObjType *&update_triple, TYPE_TRIPLE_NUM &update_triple_num)
{
	update_pattern.getVarset();

	if (update_triple != NULL)
	{
		delete[] update_triple;
		update_triple = NULL;
	}

	if (this->temp_result == NULL)
		return;

	update_triple_num = 0;

	for (int i = 0; i < (int)update_pattern.sub_group_pattern.size(); i++)
		if (update_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
		{
			for (int j = 0; j < (int)this->temp_result->results.size(); j++)
				if (update_pattern.sub_group_pattern[i].pattern.varset.belongTo(this->temp_result->results[j].getAllVarset()))
					update_triple_num += this->temp_result->results[j].result.size();
		}

	update_triple = new TripleWithObjType[update_triple_num];

	int update_triple_count = 0;
	for (int i = 0; i < (int)update_pattern.sub_group_pattern.size(); i++)
		if (update_pattern.sub_group_pattern[i].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
		{
			for (int j = 0; j < (int)this->temp_result->results.size(); j++)
			{
				int id_cols = this->temp_result->results[j].id_varset.getVarsetSize();

				if (update_pattern.sub_group_pattern[i].pattern.varset.belongTo(this->temp_result->results[j].getAllVarset()))
				{
					int subject_id = -1, predicate_id = -1, object_id = -1;
					if (update_pattern.sub_group_pattern[i].pattern.subject.value[0] == '?')
						subject_id = Varset(update_pattern.sub_group_pattern[i].pattern.subject.value).mapTo(this->temp_result->results[j].getAllVarset())[0];
					if (update_pattern.sub_group_pattern[i].pattern.predicate.value[0] == '?')
						predicate_id = Varset(update_pattern.sub_group_pattern[i].pattern.predicate.value).mapTo(this->temp_result->results[j].getAllVarset())[0];
					if (update_pattern.sub_group_pattern[i].pattern.object.value[0] == '?')
						object_id = Varset(update_pattern.sub_group_pattern[i].pattern.object.value).mapTo(this->temp_result->results[j].getAllVarset())[0];

					string subject, predicate, object;
					TripleWithObjType::ObjectType object_type;

					if (subject_id == -1)
						subject = update_pattern.sub_group_pattern[i].pattern.subject.value;
					if (predicate_id == -1)
						predicate = update_pattern.sub_group_pattern[i].pattern.predicate.value;
					if (object_id == -1)
						object = update_pattern.sub_group_pattern[i].pattern.object.value;

					for (int k = 0; k < (int)this->temp_result->results[j].result.size(); k++)
					{
						if (subject_id != -1)
						{
							if (subject_id < id_cols)
								this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[subject_id], &subject, true);
							else
								subject = this->temp_result->results[j].result[k].str[subject_id - id_cols];
						}

						if (predicate_id != -1)
						{
							if (predicate_id < id_cols)
								this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[predicate_id], &predicate, false);
							else
								predicate = this->temp_result->results[j].result[k].str[predicate_id - id_cols];
						}

						if (object_id != -1)
						{
							if (object_id < id_cols)
								this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[object_id], &object, true);
							else
								object = this->temp_result->results[j].result[k].str[object_id - id_cols];
						}

						if (object[0] == '<')
							object_type = TripleWithObjType::Entity;
						else
							object_type = TripleWithObjType::Literal;

						update_triple[update_triple_count++] = TripleWithObjType(subject, predicate, object, object_type);
					}
				}
			}
		}
}

void
GeneralEvaluation::pathVec2JSON(int src, int dst, const vector<int> &v, stringstream &ss)
{
	unordered_map<int, string> nodeIRI;
	ss << "{\"src\":\"" << kvstore->getStringByID(src) \
		<< "\",\"dst\":\"" << kvstore->getStringByID(dst) << "\",";
	ss << "\"edges\":[";
	int vLen = v.size();
	for (int i = 0; i < vLen; i++)
	{
		if (i % 2 == 0)	// Node
			nodeIRI[v[i]] = kvstore->getStringByID(v[i]);
		else	// Edge
		{
			ss << "{\"fromNode\":" << v[i - 1] << ",\"toNode\":" << v[i + 1] \
				<< ",\"predIRI\":\"" << kvstore->getPredicateByID(v[i]) << "\"}";
			if (i != vLen - 2)	// Not the last edge
				ss << ",";
		}
	}
	ss << "],\"nodes\":[";	// End of edges, start of nodes
	for (auto it = nodeIRI.begin(); it != nodeIRI.end(); ++it)
	{
		ss << "{\"nodeIndex\":" << it->first << ",\"nodeIRI\":\"" << it->second << "\"}";
		// if (it != nodeIRI.end() - 1)
		// 	ss << ",";
		if (next(it) != nodeIRI.end())
			ss << ",";
	}
	ss << "]}";
}

int GeneralEvaluation::constructTriplePattern(QueryTree::GroupPattern& triple_pattern, int dep)
{
	int group_pattern_triple_num = 0;
	// Extract all sub-group-patterns that are themselves triples, add into triple_pattern //
	for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
		if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
		{
			triple_pattern.addOnePattern(QueryTree::GroupPattern::Pattern(
				QueryTree::GroupPattern::Pattern::Element(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].pattern.subject.value),
				QueryTree::GroupPattern::Pattern::Element(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].pattern.predicate.value),
				QueryTree::GroupPattern::Pattern::Element(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].pattern.object.value)
			));
			group_pattern_triple_num++;
		}
	triple_pattern.getVarset();
	// Add extra triple patterns from shallower levels of rewriting_evaluation_stack //
	if (dep > 0)
	{
		Varset need_add;

		/// Construct var_count: map from variable name to #occurrence in triple_pattern ///
		map<string, int> var_count;
		for (int j = 0; j < (int)triple_pattern.sub_group_pattern.size(); j++)
			if (triple_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
			{
				string subject = triple_pattern.sub_group_pattern[j].pattern.subject.value;
				string object = triple_pattern.sub_group_pattern[j].pattern.object.value;

				if (subject[0] == '?')
				{
					if (var_count.count(subject) == 0)
						var_count[subject] = 0;
					var_count[subject]++;
				}
				if (object[0] == '?')
				{
					if (var_count.count(object) == 0)
						var_count[object] = 0;
					var_count[object]++;
				}
			}

		/// Construct need_add: all variable that occur exactly once ///
		for (map<string, int>::iterator iter = var_count.begin(); iter != var_count.end(); iter++)
			if (iter->second == 1)
				need_add.addVar(iter->first);

		/// Add triples from shallower levels of rewriting_evaluation_stack that ///
		/// - have common variables with need_add ///
		/// - have varset size == 1 OR 2 ///
		/// (After adding, subtract the covered var from need_add) ///
		/// QUESTION: Why first 1 then 2? ///
		for (int j = 0; j < dep; j++)
		{
			QueryTree::GroupPattern &parrent_group_pattern = this->rewriting_evaluation_stack[j].group_pattern;

			for (int k = 0; k < (int)parrent_group_pattern.sub_group_pattern.size(); k++)
				if (parrent_group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
					if (need_add.hasCommonVar(parrent_group_pattern.sub_group_pattern[k].pattern.subject_object_varset) &&
						parrent_group_pattern.sub_group_pattern[k].pattern.varset.getVarsetSize() == 1)	// Only difference here: 1
					{
						triple_pattern.addOnePattern(QueryTree::GroupPattern::Pattern(
							QueryTree::GroupPattern::Pattern::Element(parrent_group_pattern.sub_group_pattern[k].pattern.subject.value),
							QueryTree::GroupPattern::Pattern::Element(parrent_group_pattern.sub_group_pattern[k].pattern.predicate.value),
							QueryTree::GroupPattern::Pattern::Element(parrent_group_pattern.sub_group_pattern[k].pattern.object.value)
						));
						need_add = need_add - parrent_group_pattern.sub_group_pattern[k].pattern.subject_object_varset;
					}
		}
		for (int j = 0; j < dep; j++)
		{
			QueryTree::GroupPattern &parrent_group_pattern = this->rewriting_evaluation_stack[j].group_pattern;

			for (int k = 0; k < (int)parrent_group_pattern.sub_group_pattern.size(); k++)
				if (parrent_group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
					if (need_add.hasCommonVar(parrent_group_pattern.sub_group_pattern[k].pattern.subject_object_varset) &&
						parrent_group_pattern.sub_group_pattern[k].pattern.varset.getVarsetSize() == 2)	// Only difference here: 2
					{
						triple_pattern.addOnePattern(QueryTree::GroupPattern::Pattern(
							QueryTree::GroupPattern::Pattern::Element(parrent_group_pattern.sub_group_pattern[k].pattern.subject.value),
							QueryTree::GroupPattern::Pattern::Element(parrent_group_pattern.sub_group_pattern[k].pattern.predicate.value),
							QueryTree::GroupPattern::Pattern::Element(parrent_group_pattern.sub_group_pattern[k].pattern.object.value)
						));
						need_add = need_add - parrent_group_pattern.sub_group_pattern[k].pattern.subject_object_varset;
					}
		}
	}
	triple_pattern.getVarset();

	return group_pattern_triple_num;
}

void GeneralEvaluation::getUsefulVarset(Varset& useful, int dep)
{
	// Vars that are SELECT, GROUP BY, or ORDER BY //
	useful = this->query_tree.getResultProjectionVarset() + this->query_tree.getGroupByVarset() \
				+ this->query_tree.getOrderByVarset();
	if (!this->query_tree.checkProjectionAsterisk())
	{
		// All vars from ancestor levels' triples and filters //
		for (int j = 0; j < dep; j++)
		{
			QueryTree::GroupPattern &parrent_group_pattern = this->rewriting_evaluation_stack[j].group_pattern;

			for (int k = 0; k < (int)parrent_group_pattern.sub_group_pattern.size(); k++)
			{
				if (parrent_group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Pattern_type)
					useful += parrent_group_pattern.sub_group_pattern[k].pattern.varset;
				else if (parrent_group_pattern.sub_group_pattern[k].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
					useful += parrent_group_pattern.sub_group_pattern[k].filter.varset;
			}
		}
		// All vars from current levels' triples and filters //
		for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
		{
			if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Optional_type)
				useful += rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].optional.group_pattern_resultset_maximal_varset;
			else if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == QueryTree::GroupPattern::SubGroupPattern::Filter_type)
				useful += rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.varset;
		}
	}
}

bool GeneralEvaluation::checkBasicQueryCache(vector<QueryTree::GroupPattern::Pattern>& basic_query, TempResultSet *sub_result, Varset& useful)
{
	bool success = false;
	if (this->query_cache != NULL)
	{
		TempResultSet *temp = new TempResultSet();
		temp->results.push_back(TempResult());
		long tv_bfcheck = Util::get_cur_time();
		success = this->query_cache->checkCached(basic_query, useful, temp->results[0]);
		long tv_afcheck = Util::get_cur_time();
		printf("after checkCache, used %ld ms.\n", tv_afcheck - tv_bfcheck);

		// If query cache hit, save partial result //
		// Note the semantics of doJoin: sub_result is joined with temp, and saved into new_result //
		if (success)
		{
			printf("QueryCache hit\n");
			printf("Final result size: %d\n", (int)temp->results[0].result.size());

			TempResultSet *new_result = new TempResultSet();
			sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

			sub_result->release();
			delete sub_result;

			sub_result = new_result;
		}
		else
			printf("QueryCache miss\n");

		temp->release();
		delete temp;
	}
	return success;
}

void GeneralEvaluation::fillCandList(SPARQLquery& sparql_query, int dep, vector<vector<string> >& encode_varset)
{
	TempResultSet *&last_result = this->rewriting_evaluation_stack[dep - 1].result;

	for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
	{
		BasicQuery &basic_query = sparql_query.getBasicQuery(j);
		vector<string> &basic_query_encode_varset = encode_varset[j];

		for (int k = 0; k < (int)basic_query_encode_varset.size(); k++)
		{
			/// For each var in the current BGP ///
			/// construct result_set: the candidate values of this var from the parent result in rewriting_evaluation_stack ///
			set<unsigned> result_set;

			for (int t = 0; t < (int)last_result->results.size(); t++)
			{
				//// For each var in each BGP, check each result in the parent result in rewriting_evaluation_stack ////
				//// If this var exists in the parent result, retrieve its value in the parent result into result_set ////
				vector<TempResult::ResultPair> &result = last_result->results[t].result;

				int pos = Varset(basic_query_encode_varset[k]).mapTo(last_result->results[t].id_varset)[0];
				if (pos != -1)
				{
					for (int l = 0; l < (int)result.size(); l++)
						result_set.insert(result[l].id[pos]);
				}
			}

			/// If result_set is non-empty, set it as the current var's candidate list ///
			if (!result_set.empty())
			{
				vector<unsigned> result_vector;
				result_vector.reserve(result_set.size());

				for (set<unsigned>::iterator iter = result_set.begin(); iter != result_set.end(); iter++)
					result_vector.push_back(*iter);

				basic_query.getCandidateList(k).copy(result_vector);
				basic_query.setReady(k);

				printf("fill var %s CandidateList size %d\n", basic_query_encode_varset[k].c_str(), (int)result_vector.size());
			}
		}
	}
}

void GeneralEvaluation::joinBasicQueryResult(SPARQLquery& sparql_query, TempResultSet *new_result, TempResultSet *sub_result, vector<vector<string> >& encode_varset, \
	vector<vector<QueryTree::GroupPattern::Pattern> >& basic_query_handle, long tv_begin, long tv_handle, int dep)
{
	// Each BGP's results are copied out to temp, and then joined with sub_result //
	for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
	{
		TempResultSet *temp = new TempResultSet();
		temp->results.push_back(TempResult());

		temp->results[0].id_varset = Varset(encode_varset[j]);
		int varnum = (int)encode_varset[j].size();

		vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(j).getResultList();
		int basicquery_result_num = (int)basicquery_result.size();

		temp->results[0].result.reserve(basicquery_result_num);
		for (int k = 0; k < basicquery_result_num; k++)
		{
			unsigned *v = new unsigned[varnum];
			memcpy(v, basicquery_result[k], sizeof(int) * varnum);
			temp->results[0].result.push_back(TempResult::ResultPair());
			temp->results[0].result.back().id = v;
		}

		if (this->query_cache != NULL && dep == 0)
		{
			//if unconnected, time is incorrect
			int time = tv_handle - tv_begin;

			long tv_bftry = Util::get_cur_time();
			bool success = this->query_cache->tryCaching(basic_query_handle[j], temp->results[0], time);
			if (success)	printf("QueryCache cached\n");
			else			printf("QueryCache didn't cache\n");
			long tv_aftry = Util::get_cur_time();
			printf("during tryCache, used %ld ms.\n", tv_aftry - tv_bftry);
		}

		if (sub_result->results.empty())
		{
			delete sub_result;
			sub_result = temp;
		}
		else
		{
			// TempResultSet *new_result = new TempResultSet();
			new_result = new TempResultSet();
			sub_result->doJoin(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

			temp->release();
			sub_result->release();
			delete temp;
			delete sub_result;

			sub_result = new_result;
		}
	}

	printf("In joinBasicQueryResult, print varset\n");
	sub_result->results[0].getAllVarset();
	printf("11111\n");
}
