/*=============================================================================
# Filename: GeneralEvaluation.cpp
# Author: Jiaqi, Chen
# Mail: chenjiaqi93@163.com
# Last Modified: 2017-05-05
# Description: implement functions in GeneralEvaluation.h
=============================================================================*/

#include "GeneralEvaluation.h"

using namespace std;

#define TEST_BGPQUERY

GeneralEvaluation::EvaluationStackStruct::EvaluationStackStruct()
{
	// result = new TempResultSet();
	result = nullptr;
}

GeneralEvaluation::EvaluationStackStruct::EvaluationStackStruct(const EvaluationStackStruct& that)
{
	result = new TempResultSet();
	group_pattern = that.group_pattern;
	if (that.result)
		*result = *(that.result);
}

GeneralEvaluation::EvaluationStackStruct& GeneralEvaluation::EvaluationStackStruct::operator=(const EvaluationStackStruct& that)
{
	TempResultSet *local_result = new TempResultSet();
	if (that.result)
		*local_result = *(that.result);
	if (result)
		delete result;
	result = local_result;
	group_pattern = that.group_pattern;

	return *this;
}

GeneralEvaluation::EvaluationStackStruct::~EvaluationStackStruct()
{
	// if (result)
	// 	delete result;
}

// void *preread_from_index(void *argv)
// {
// 	vector<StringIndexFile*> * indexfile = (vector<StringIndexFile*> *)*(long*)argv;
// 	ResultSet *ret_result = (ResultSet*)*((long*)argv + 1);
// 	vector<int>* proj2temp = (vector<int>*)*((long*)argv + 2);
// 	int id_cols = *((long*)argv + 3);
// 	TempResult* result0 = (TempResult*)*((long*)argv + 4);
// 	vector<bool>* isel = (vector<bool>*)*((long*)argv + 5);
// 	StringIndexFile* 	entity = (*indexfile)[0];
// 	StringIndexFile* 	literal = (*indexfile)[1];
// 	StringIndexFile* 	predicate = (*indexfile)[2];
// 	unsigned total = ret_result->ansNum;
// 	int var_num = ret_result->select_var_num;
// 	// unsigned thelta = total / 4096;
// 	char tmp;
// 	char *entityc = entity->Mmap;
// 	char *literalc = literal->Mmap;
// 	char *prec = predicate->Mmap;
// 	for (unsigned i = 0; i < total; i ++)
// 	{
// 		for (int j = 0; j < var_num; j++)
// 		{
// 			int k = (*proj2temp)[j];
// 			long offset = -1;
// 			if (k != -1)
// 			{
// 				if (k < id_cols)
// 				{
// 					unsigned ans_id = result0->result[i].id[k];
// 					if (ans_id != INVALID)
// 					{
// 						if ((*isel)[k])
// 						{
// 							if (ans_id < Util::LITERAL_FIRST_ID)
// 							{
// 								offset = entity->GetOffsetbyID(ans_id);
// 								if (offset != -1)
// 								{
// 									tmp = entityc[offset];
// 								}
// 							}
// 							else
// 							{
// 								offset = literal->GetOffsetbyID(ans_id - Util::LITERAL_FIRST_ID);
// 								if (offset != -1)
// 								{
// 									tmp = literalc[offset];
// 								}
// 							}
// 						}
// 						else
// 						{
// 							offset = predicate->GetOffsetbyID(ans_id);
// 							if (offset != -1)
// 							{
// 								tmp = prec[offset];
// 							}
// 						}
// 					}
// 				}
// 			}
// 		}
// 	}
//
// 	return NULL;
// }


GeneralEvaluation::GeneralEvaluation(KVstore *_kvstore, StringIndex *_stringindex,  QueryCache *_query_cache, CSR *_csr,
									 TYPE_TRIPLE_NUM *_pre2num,TYPE_TRIPLE_NUM *_pre2sub,
									 TYPE_TRIPLE_NUM *_pre2obj, TYPE_TRIPLE_NUM _triples_num, TYPE_PREDICATE_ID _limitID_predicate,
									 TYPE_ENTITY_LITERAL_ID _limitID_literal, TYPE_ENTITY_LITERAL_ID _limitID_entity,
									 shared_ptr<Transaction> _txn, const BlockInfo *const freelist_entity, TYPE_ENTITY_LITERAL_ID entity_num):
	temp_result(NULL), query_parser(make_shared<QueryParser>()), kvstore(_kvstore), stringindex(_stringindex), query_cache(_query_cache), csr(_csr), ranked(false),
	pre2num(_pre2num), pre2sub(_pre2sub), pre2obj(_pre2obj), triples_num(_triples_num), limitID_predicate(_limitID_predicate),
	limitID_literal(_limitID_literal), limitID_entity(_limitID_entity), txn(_txn), all_entity_id(freelist_entity,entity_num), fp(NULL), export_flag(false)
{
	if (csr)
		pqHandler = make_shared<PathQueryHandler>(csr);
	else
		pqHandler = nullptr;
	well_designed = -1;	// Not checked yet
	this->optimizer_ = make_shared<Optimizer>(kvstore,pre2num,pre2sub,pre2obj,triples_num,limitID_predicate,
                                      limitID_literal,limitID_entity,txn);
	this->bgp_query_total = make_shared<BGPQuery>();
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
		kvstore->getsubIDlistBypreID(i, sublist, sublist_len, true);
		//cout<<"    sub_num: "<<sublist_len<<endl;
		unsigned offset = 0;
		unsigned index = 0;
		for(unsigned j=0;j<sublist_len;j++)
		{
			string sub = kvstore->getEntityByID(sublist[j]);
			//cout<<"    sid: "<<sublist[j]<<"    sub: "<<sub<<endl;
			unsigned* objlist = NULL;
			unsigned objlist_len = 0;
			kvstore->getobjIDlistBysubIDpreID(sublist[j], i, objlist, objlist_len); 
			unsigned len = objlist_len;	// the real object list length
			for(unsigned k=0;k<objlist_len;k++)
			{
				if(objlist[k]>=Util::LITERAL_FIRST_ID)
				{
					--len;
					continue;
				}
				csr[0].adjacency_list[i].push_back(objlist[k]);
			}
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
		kvstore->getobjIDlistBypreID(i, objlist, objlist_len, true);
		//cout<<"    obj_num: "<<objlist_len<<endl;
		unsigned offset = 0;
		unsigned index = 0;
		for(unsigned j=0;j<objlist_len;j++)
		{
			if(objlist[j]>=Util::LITERAL_FIRST_ID)
				continue;
			string obj = kvstore->getEntityByID(objlist[j]);
			unsigned* sublist = NULL;
			unsigned sublist_len = 0;
			kvstore->getsubIDlistByobjIDpreID(objlist[j], i, sublist, sublist_len); 
			unsigned len = sublist_len;
			for(unsigned k=0;k<sublist_len;k++)
			{
				string sub = kvstore->getEntityByID(sublist[k]);
				csr[1].adjacency_list[i].push_back(sublist[k]);
			}
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
	// init vertices and edges num
	set<int> vertices;
	for (int j = 0; j < 2; j++)
	{
		for (unsigned i = 0; i < csr[j].pre_num; i++)
			vertices.insert(csr[j].adjacency_list[i].begin(), csr[j].adjacency_list[i].end());
	}
	csr[1].n = vertices.size();

	unsigned ret = 0;
	for (auto i = 0; i < csr[1].pre_num; i++)	// Same as summing that of csr[0]
		ret += csr[1].adjacency_list[i].size();
	csr[1].m = ret;

	long end_time = Util::get_cur_time();
	cout << "Loading CSR in GeneralEvaluation takes " << (end_time - begin_time) << "ms" << endl;
}

void
GeneralEvaluation::prepPathQuery()
{
	if (!csr)
		loadCSR();
	if (!pqHandler)
		pqHandler = make_shared<PathQueryHandler>(csr);
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
		this->query_parser->setQueryTree(&(this->query_tree));
		this->query_parser->SPARQLParse(_query);
	}
	catch (const char *e)
	{
		printf("%s\n", e);
		return false;
	}
	catch(const runtime_error& e2)
	{
		//cout<<"catch the runtime error"<<endl;
		throw runtime_error(e2.what());
		return false;
	}
	catch (...)
	{
		cout<<"GeneralEvaluation::parseQuery "<<" catch some error."<<endl;
		throw  runtime_error("Some syntax errors in sparql");
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

	// Gather and encode all triples
	if (!this->bgp_query_total)
		this->bgp_query_total = make_shared<BGPQuery>();
	addAllTriples(this->query_tree.getGroupPattern());
	bgp_query_total->EncodeBGPQuery(kvstore, vector<string>(), \
			this->query_tree.getProjectionModifier() == QueryTree::ProjectionModifier::Modifier_Distinct);

	this->rewriting_evaluation_stack.clear();
	this->rewriting_evaluation_stack.emplace_back();
	this->rewriting_evaluation_stack.back().group_pattern = this->query_tree.getGroupPattern();
	this->rewriting_evaluation_stack.back().result = NULL;

	this->temp_result = this->queryEvaluation(0);

	return true;
}

void GeneralEvaluation::getAllPattern(const GroupPattern &group_pattern, vector<GroupPattern::Pattern> &vp)
{
	for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
	{
		if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Group_type)
			getAllPattern(group_pattern.sub_group_pattern[i].group_pattern, vp);
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Pattern_type)
			vp.push_back(group_pattern.sub_group_pattern[i].pattern);
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Union_type)
		{
			for (int j = 0; j < (int)group_pattern.sub_group_pattern[i].unions.size(); j++)
				getAllPattern(group_pattern.sub_group_pattern[i].unions[j], vp);
		}
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Optional_type \
			|| group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Minus_type)
			getAllPattern(group_pattern.sub_group_pattern[i].optional, vp);
	}
}

TempResultSet* GeneralEvaluation::queryEvaluation(int dep)
{
	// If ASK query, and only one BGP, check if every triple consists of all constants
	// If so, set a special result so that getFinalResult will know
	if (this->query_tree.getQueryForm() == QueryTree::Ask_Query)
	{
		bool singleBGP = true, allConstants = true;
		vector<Triple> triple_vt;
		for (unsigned i = 0; i < this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size(); i++)
		{
			// TODO: this is problematic. Even if all are Pattern_type, could still have multiple BGPs (not coalescable)
			if (this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[i].type \
				!= GroupPattern::SubGroupPattern::Pattern_type)
			{
				singleBGP = false;
				break;
			}
			// Check if the triple consists of all constants
			if (this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[i].pattern.subject.value[0] == '?' \
				|| this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[i].pattern.predicate.value[0] == '?' \
				|| this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[i].pattern.object.value[0] == '?')
			{
				allConstants = false;
				break;
			}
			triple_vt.push_back(Triple(this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[i].pattern.subject.value, \
				this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[i].pattern.predicate.value, \
				this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[i].pattern.object.value));
		}
		if (singleBGP && allConstants)
		{
			// Check if these constant triples exist in database
			bool exist = BGPQuery::CheckConstBGPExist(triple_vt, kvstore);
			// Set a special result (indicates true/false) 
			TempResultSet *result = new TempResultSet();
			(*result).results.push_back(TempResult());
			(*result).results[0].result.emplace_back();
			if (exist)
				(*result).results[0].result[0].str.push_back("true");
			else
				(*result).results[0].result[0].str.push_back("false");
			return result;
		}
	}

	GroupPattern group_pattern;

	// Check well-designed (TODO: check at every depth, now only check once) //
	// If well-designed, split and refill group_pattern according to rewriting //
	if (well_designed == -1)
		well_designed = (int)query_tree.checkWellDesigned();
	// well_designed = 0;

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
		deque<GroupPattern> queue;
		queue.push_back(this->rewriting_evaluation_stack[dep].group_pattern);
		group_pattern.addOneGroupUnion();

		while (!queue.empty())
		{
			if (!this->expanseFirstOuterUnionGroupPattern(queue.front(), queue))
			{
				group_pattern.addOneUnion();
				group_pattern.getLastUnion() = queue.front();
			}
			queue.pop_front();
		}
	}
	group_pattern.initPatternBlockid();

	TempResultSet *result = new TempResultSet();
	// Iterate across all sub-group-patterns, process according to type
	for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
	{
		if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Group_type)
		{
			group_pattern.sub_group_pattern[i].group_pattern.print(0);
			this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
			this->rewriting_evaluation_stack.back().group_pattern = group_pattern.sub_group_pattern[i].group_pattern;
			this->rewriting_evaluation_stack.back().result = NULL;
			this->rewriting_evaluation_stack[dep].result = result;
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
				result->initial = false;
			}
		}
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Pattern_type)
		{
			if (!group_pattern.sub_group_pattern[i].pattern.kleene)
			{
				TempResultSet *sub_result = new TempResultSet();
				
				QueryInfo query_info;
				query_info.limit_ = false;
				query_info.limit_num_ = 0;
				if(this->query_tree.getLimit()!=-1 && this->query_tree.getSingleBGP()) {
					query_info.limit_ = true;
					query_info.limit_num_ = this->query_tree.getLimit();
				}
				query_info.is_distinct_ = this->query_tree.getProjectionModifier() == QueryTree::ProjectionModifier::Modifier_Distinct;
				query_info.ordered_by_expressions_ = make_shared<vector<Order>>();
				for(auto order_item:this->query_tree.getOrderVarVector())
					query_info.ordered_by_expressions_->push_back(order_item);
				
				int st = i;
				while (st > 0 && ((group_pattern.sub_group_pattern[st - 1].type == GroupPattern::SubGroupPattern::Pattern_type
					&& !group_pattern.sub_group_pattern[st - 1].pattern.kleene) 
					|| group_pattern.sub_group_pattern[st - 1].type == GroupPattern::SubGroupPattern::Union_type))
					st--;

				for (int j = st; j < i; j++)
				{
					if (group_pattern.sub_group_pattern[j].type == GroupPattern::SubGroupPattern::Pattern_type)
					{
						// Does not merge triples with predicate as iri*
						if (group_pattern.sub_group_pattern[i].pattern.subject_object_varset.hasCommonVar(group_pattern.sub_group_pattern[j].pattern.subject_object_varset) \
							&& !group_pattern.sub_group_pattern[j].pattern.kleene)
							group_pattern.mergePatternBlockID(i, j);
					}
				}

				if (i + 1 == (int)group_pattern.sub_group_pattern.size() ||
					(group_pattern.sub_group_pattern[i + 1].type != GroupPattern::SubGroupPattern::Pattern_type \
					|| group_pattern.sub_group_pattern[i + 1].pattern.kleene))
				{
					// Start merging (may be multiple BGPs here)
					SPARQLquery sparql_query;
					#ifdef TEST_BGPQUERY
					vector<shared_ptr<BGPQuery>> bgp_query_vec;
					#endif
					vector<vector<string> > encode_varset;
					vector<vector<GroupPattern::Pattern> > basic_query_handle;

					// auto limit_num = query_tree.getLimit();
					auto order_var_vec = query_tree.getOrderVarVector();

					for (int j = st; j <= i; j++)
						if (group_pattern.sub_group_pattern[j].type == GroupPattern::SubGroupPattern::Pattern_type \
							&& !group_pattern.sub_group_pattern[j].pattern.kleene)
						{
							if (!group_pattern.sub_group_pattern[j].pattern.varset.empty())
							{
								if (group_pattern.getRootPatternBlockID(j) == j)		//root node, coming from mergePatternBlockID
								{
									Varset occur;
									vector<GroupPattern::Pattern> basic_query;

									for (int k = st; k <= i; k++)
										if (group_pattern.sub_group_pattern[k].type == GroupPattern::SubGroupPattern::Pattern_type)
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
									// if (this->query_cache != NULL)
									// 	success = checkBasicQueryCache(basic_query, result, occur);
									//// If query cache not hit, save the current BGP to sparql_query for later processing ////
									//// QUESTION: Is basic_query_handle a redundant variable? ////
									if (!success)
									{
										#ifndef TEST_BGPQUERY
										sparql_query.addBasicQuery();
										for (int k = 0; k < (int)basic_query.size(); k++)
											sparql_query.addTriple(Triple(basic_query[k].subject.value,
												basic_query[k].predicate.value,
												basic_query[k].object.value));
										#else
										bgp_query_vec.push_back(make_shared<BGPQuery>());
										for (int k = 0; k < (int)basic_query.size(); k++)
										{
											bgp_query_vec[bgp_query_vec.size() - 1]->AddTriple(Triple(basic_query[k].subject.value,
													basic_query[k].predicate.value,
													basic_query[k].object.value));
										}
										#endif

										encode_varset.push_back(occur.vars);
										basic_query_handle.push_back(basic_query);
									}
								}
							}
						}

					long tv_begin = Util::get_cur_time();
					#ifndef TEST_BGPQUERY
					sparql_query.encodeQuery(this->kvstore, encode_varset);
					#else
					vector<bool> encode_constant_exist(bgp_query_vec.size(), true);
					bool curr_exist;
					for (size_t k = 0; k < bgp_query_vec.size(); k++)
					{
						curr_exist = bgp_query_vec[k]->EncodeSmallBGPQuery(bgp_query_total.get(), kvstore, encode_varset[k], this->query_tree.getProjectionModifier() == QueryTree::ProjectionModifier::Modifier_Distinct);
						encode_constant_exist[k] = curr_exist;
					}
					#endif
					long tv_encode = Util::get_cur_time();
					printf("during Encode, used %ld ms.\n", tv_encode - tv_begin);

					// TODO: refine the fillcand strategy regarding the same layer
					// Now only consider dep == 0
					// The following is problematic for deeper layers, disable for now
					// if (dep == 0 && !(result->results.empty()))
					// 	this->rewriting_evaluation_stack[dep].result = result;
					// if (dep > 0)
					// {
						#ifndef TEST_BGPQUERY
						fillCandList(sparql_query, dep, encode_varset);
						#else
						if (dep == 0)
							fillCandList(bgp_query_vec, dep, encode_varset, result);
						else
							fillCandList(bgp_query_vec, dep, encode_varset);
						#endif
					// }
					long tv_fillcand = Util::get_cur_time();
					printf("after FillCand, used %ld ms.\n", tv_fillcand - tv_encode);

					#ifndef TEST_BGPQUERY
					this->optimizer_->DoQuery(sparql_query,query_info);
					#else
					for (size_t j = 0; j < bgp_query_vec.size(); j++)
					{
						unique_ptr<unsigned[]>& p2id = bgp_query_vec[j]->resultPositionToId();
						p2id = unique_ptr<unsigned[]>(new unsigned [encode_varset[j].size()]);
						for (size_t k = 0; k < encode_varset[j].size(); k++)
							p2id[k] = bgp_query_vec[j]->get_var_id_by_name(Varset(encode_varset[j]).vars[k]);
						// bgp_query_vec[j]->print(kvstore);
						if (!encode_constant_exist[j])
							continue;	// If any constant in this BGP does not exist, do not need to DoQuery
						ranked = get<1>(this->optimizer_->DoQuery(bgp_query_vec[j],query_info));
					}
					#endif

					long tv_handle = Util::get_cur_time();
					printf("during Handle, used %ld ms.\n", tv_handle - tv_encode);

					//collect and join the result of each BGP
					// for (int j = 0; j < sparql_query.getBasicQueryNum(); j++)
					for (size_t j = 0; j < bgp_query_vec.size(); j++)
					{
						TempResultSet *temp = new TempResultSet();
						
						temp->results.push_back(TempResult());

						temp->results[0].id_varset = Varset(encode_varset[j]);
						int varnum = (int)encode_varset[j].size();

						#ifndef TEST_BGPQUERY
						vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(j).getResultList();
						#else
						copyBgpResult2TempResult(bgp_query_vec[j], varnum, temp->results[0]);
						#endif

						if (this->query_cache != NULL)
						{
							//if unconnected, time is incorrect
							// int time = tv_handle - tv_begin;

							long tv_bftry = Util::get_cur_time();
							// bool success = this->query_cache->tryCaching(basic_query_handle[0], temp->results[0], time);
							bool success = false;
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
					
					// printf("Pattern_type result: \n");
					// result->print();

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
			}
			else
			{
				// PathQueryHandler function arguments are vertex IDs
				// Use `tr` to store BFS starting vertices
				TempResultSet *sub_result = new TempResultSet();
				TempResult *tr = NULL;
				if (result->results.size() > 0)
				{
					size_t j;
					for (j = 0; j < result->results.size(); j++)
					{
						if (result->results[j].result.empty())
							j++;
						break;
					}
					if (j < result->results.size() && !result->results[j].result.empty())
						tr = &(result->results[j]);
				}

				// BFS from the subjects / objects to find results
				kleeneClosure(sub_result, tr, group_pattern.sub_group_pattern[i].pattern.subject.value, \
					group_pattern.sub_group_pattern[i].pattern.predicate.value, group_pattern.sub_group_pattern[i].pattern.object.value, dep);
				
				// TODO: cache result
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
		}
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Union_type)
		{
			TempResultSet *sub_result_outer = new TempResultSet();

			for (int j = 0; j < (int)group_pattern.sub_group_pattern[i].unions.size(); j++)
			{
				TempResultSet *sub_result = nullptr;
				if (well_designed == 0)
				{
					this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
					this->rewriting_evaluation_stack.back().group_pattern = group_pattern.sub_group_pattern[i].unions[j];
					this->rewriting_evaluation_stack.back().result = NULL;
					this->rewriting_evaluation_stack[dep].result = result;
					sub_result = queryEvaluation(dep + 1);
				}
				else if (well_designed == 1)
				{			
					// Prepare and print //
					this->rewriting_evaluation_stack[dep].group_pattern = group_pattern.sub_group_pattern[i].unions[j];

					for (int k = 0; k < 80; k++)			printf("=");
					printf("\n");
					rewriting_evaluation_stack[dep].group_pattern.print(dep);
					for (int k = 0; k < 80; k++)			printf("=");
					printf("\n");

					sub_result = new TempResultSet();

					// Construct triple_pattern //
					GroupPattern triple_pattern;
					int group_pattern_triple_num = constructTriplePattern(triple_pattern, dep);

					// Get useful varset //
					Varset useful;
					getUsefulVarset(useful, dep);

					SPARQLquery sparql_query;
					#ifdef TEST_BGPQUERY
					vector<shared_ptr<BGPQuery>> bgp_query_vec;
					#endif
					vector<vector<string> > encode_varset;
					vector<vector<GroupPattern::Pattern> > basic_query_handle;

					// Get connected block (BGP): merge the block IDs of triples with common vars //
					triple_pattern.initPatternBlockid();
					for (int l = 0; l < (int)triple_pattern.sub_group_pattern.size(); l++)
						if (triple_pattern.sub_group_pattern[l].type == GroupPattern::SubGroupPattern::Pattern_type \
							&& !triple_pattern.sub_group_pattern[l].pattern.kleene)
						{
							// cout << "l = " << l << ", not kleene" << endl;
							for (int k = 0; k < l; k++)
								if (triple_pattern.sub_group_pattern[k].type == GroupPattern::SubGroupPattern::Pattern_type \
									&& !triple_pattern.sub_group_pattern[k].pattern.kleene)
									if (triple_pattern.sub_group_pattern[l].pattern.subject_object_varset.hasCommonVar(triple_pattern.sub_group_pattern[k].pattern.subject_object_varset))
										triple_pattern.mergePatternBlockID(l, k);
						}

					// Retrieve current BGP's result if query cache hit, or else save into sparql_query //
					for (int l = 0; l < (int)triple_pattern.sub_group_pattern.size(); l++)
						if (triple_pattern.sub_group_pattern[l].type == GroupPattern::SubGroupPattern::Pattern_type \
							&& !triple_pattern.sub_group_pattern[l].pattern.kleene \
							&& triple_pattern.getRootPatternBlockID(l) == l)
						{
							/// Process only the root triples of each BGP ///
							//// Construct occur: all vars that occur in this BGP ////
							//// Construct basic_query: vector of triples in this block ////
							Varset occur, local_useful = useful;
							vector<GroupPattern::Pattern> basic_query;
							for (int k = 0; k < (int)triple_pattern.sub_group_pattern.size(); k++)
								if (triple_pattern.sub_group_pattern[k].type == GroupPattern::SubGroupPattern::Pattern_type \
									&& triple_pattern.getRootPatternBlockID(k) == l)
								{
									if (k < group_pattern_triple_num)
										occur += triple_pattern.sub_group_pattern[k].pattern.varset;

									basic_query.push_back(triple_pattern.sub_group_pattern[k].pattern);
								}

							//// Reduce useful: return result vars ////
							if (!this->query_tree.checkProjectionAsterisk() && local_useful.hasCommonVar(occur))
								local_useful = local_useful * occur;	// Only the common vars remain
							else
								local_useful = occur;

							//// Print useful vars and triple patterns ////
							printf("useful vars (SELECT + GROUPBY + ORDERBY): ");
							local_useful.print();
							printf("triple patterns: \n");
							for (int k = 0; k < (int)basic_query.size(); k++)
								printf("%s\t%s\t%s\n", basic_query[k].subject.value.c_str(),
									basic_query[k].predicate.value.c_str(),
									basic_query[k].object.value.c_str()
								);
							bool success = false;
							// if (this->query_cache != NULL && dep == 0)
							// 	success = checkBasicQueryCache(basic_query, sub_result, useful);
							//// If query cache not hit, save the current BGP to sparql_query for later processing ////
							//// QUESTION: Is basic_query_handle a redundant variable? ////
							if (!success)
							{
								#ifndef TEST_BGPQUERY
								sparql_query.addBasicQuery();
								for (int k = 0; k < (int)basic_query.size(); k++)
									sparql_query.addTriple(Triple(basic_query[k].subject.value,
										basic_query[k].predicate.value,
										basic_query[k].object.value));
								#else
								bgp_query_vec.push_back(make_shared<BGPQuery>());
								for (int k = 0; k < (int)basic_query.size(); k++)
								{
									bgp_query_vec[bgp_query_vec.size() - 1]->AddTriple(Triple(basic_query[k].subject.value,
											basic_query[k].predicate.value,
											basic_query[k].object.value));
								}
								#endif

								encode_varset.push_back(local_useful.vars);
								basic_query_handle.push_back(basic_query);
							}
						}

					// Encode BGPs //
					long tv_begin = Util::get_cur_time();
					#ifndef TEST_BGPQUERY
					sparql_query.encodeQuery(this->kvstore, encode_varset);
					#else
					vector<string> total_encode_varset;
					vector<bool> encode_constant_exist(bgp_query_vec.size(), true);
					for (size_t k = 0; k < encode_varset.size(); k++)
						total_encode_varset.insert(total_encode_varset.end(), encode_varset[k].begin(), encode_varset[k].end());
					for (size_t k = 0; k < bgp_query_vec.size(); k++)
					{
						bool curr_exist = bgp_query_vec[k]->EncodeSmallBGPQuery(bgp_query_total.get(), kvstore, encode_varset[k], this->query_tree.getProjectionModifier() == QueryTree::ProjectionModifier::Modifier_Distinct);
						encode_constant_exist[k] = curr_exist;
					}
					#endif
					long tv_encode = Util::get_cur_time();
					printf("after Encode, used %ld ms.\n", tv_encode - tv_begin);

					// Set candidate lists of common vars with the parent layer in rewriting_evaluation_stack //
					// TODO: check if this affects BGPQuery execution
					if (dep > 0)
					{
						#ifndef TEST_BGPQUERY
						fillCandList(sparql_query, dep, encode_varset);
						#else
						fillCandList(bgp_query_vec, dep, encode_varset);
						#endif
					}
					long tv_fillcand = Util::get_cur_time();
					printf("after FillCand, used %ld ms.\n", tv_fillcand - tv_encode);

					/* PLEASE REPLACE WITH OPTIMIZER */
					QueryInfo query_info;

					query_info.limit_ = false;
					query_info.limit_num_ = -1;
					if(this->query_tree.getLimit()!=-1 && this->query_tree.getSingleBGP()) {
						query_info.limit_ = true;
						query_info.limit_num_ = this->query_tree.getLimit();
					}

					query_info.is_distinct_ = this->query_tree.getProjectionModifier() == QueryTree::ProjectionModifier::Modifier_Distinct;

					query_info.ordered_by_expressions_ = make_shared<vector<Order>>();
					for(auto order_item:this->query_tree.getOrderVarVector())
						query_info.ordered_by_expressions_->push_back(order_item);

					#ifndef TEST_BGPQUERY
					this->optimizer_->DoQuery(sparql_query,query_info);
					#else
					for (size_t l = 0; l < bgp_query_vec.size(); l++)
					{
						unique_ptr<unsigned[]>& p2id = bgp_query_vec[l]->resultPositionToId();
						p2id = unique_ptr<unsigned[]>(new unsigned [encode_varset[l].size()]);
						for (size_t k = 0; k < encode_varset[l].size(); k++)
							p2id[k] = bgp_query_vec[l]->get_var_id_by_name(Varset(encode_varset[l]).vars[k]);
						// bgp_query_vec[l]->print(kvstore);
						if (!encode_constant_exist[l])
							continue;
						ranked = get<1>(this->optimizer_->DoQuery(bgp_query_vec[l],query_info));
					}
					#endif

					long tv_handle = Util::get_cur_time();
					printf("after Handle, used %ld ms.\n", tv_handle - tv_fillcand);

					// Collect and join the result of each BasicQuery //
					// Each BGP's results are copied out to temp, and then joined with sub_result //
					// NOTE: only one BGP so get rid of the loop
					for (size_t l = 0; l < bgp_query_vec.size(); l++)
					{
						TempResultSet *temp = new TempResultSet();
						temp->results.push_back(TempResult());

						temp->results[0].id_varset = Varset(encode_varset[l]);
						int varnum = (int)encode_varset[l].size();

						#ifndef TEST_BGPQUERY
						vector<unsigned*> &basicquery_result = sparql_query.getBasicQuery(l).getResultList();
						#else
						copyBgpResult2TempResult(bgp_query_vec[l], varnum, temp->results[0]);
						#endif
						
						// cout << "Use count = " << bgp_query_vec[l].use_count() << endl;
						bgp_query_vec[l] = nullptr;

						if (this->query_cache != NULL && dep == 0)
						{
							//if unconnected, time is incorrect
							// int time = tv_handle - tv_begin;

							long tv_bftry = Util::get_cur_time();
							// bool success = this->query_cache->tryCaching(basic_query_handle[0], temp->results[0], time);
							bool success = false;
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

					for (int l = 0; l < (int)triple_pattern.sub_group_pattern.size(); l++)
					{
						if (triple_pattern.sub_group_pattern[l].pattern.kleene)
						{
							TempResultSet *temp = new TempResultSet();

							TempResult *tr = NULL;
							if (sub_result->results.size() > 0)
							{
								size_t k;
								for (k = 0; k < sub_result->results.size(); k++)
								{
									if (sub_result->results[k].result.empty())
										k++;
									break;
								}
								if (k < sub_result->results.size() && !sub_result->results[k].result.empty())
									tr = &(sub_result->results[k]);
							}

							// BFS from the subjects / objects to find results
							kleeneClosure(temp, tr, triple_pattern.sub_group_pattern[l].pattern.subject.value, \
								triple_pattern.sub_group_pattern[l].pattern.predicate.value, triple_pattern.sub_group_pattern[l].pattern.object.value, dep);
							
							// TODO: cache result
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
					}

					// Prepare varset (without this step the following will fail) //
					for (int l = 0; l < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); l++)
						rewriting_evaluation_stack[dep].group_pattern.getVarset();

					// Process BIND //
					for (int l = 0; l < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); l++)
						if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].type == GroupPattern::SubGroupPattern::Bind_type)
						{
							sub_result->doBind(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].bind, kvstore, \
								rewriting_evaluation_stack[dep].group_pattern.group_pattern_subject_object_maximal_varset);
						}

					// Process FILTER (with var in minimal_varset constraint) //
					for (int l = 0; l < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); l++)
						if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].type == GroupPattern::SubGroupPattern::Filter_type)
							if (!rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].filter.done && \
								rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].filter.varset. \
								belongTo(rewriting_evaluation_stack[dep].group_pattern.group_pattern_resultset_minimal_varset))
							// if (!rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].filter.done)
							{
								rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].filter.done = true;
								// If FILTER scope is global && no ORDER BY && is the last operation, doFilter with limit
								if (dep == 0 && query_tree.getOrderByVarset().empty() && l == rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size() - 1) {
									sub_result->doFilter(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].filter, kvstore,\
										rewriting_evaluation_stack[dep].group_pattern.group_pattern_subject_object_maximal_varset, query_tree.getLimit());
								} else
									sub_result->doFilter(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].filter, kvstore,\
										rewriting_evaluation_stack[dep].group_pattern.group_pattern_subject_object_maximal_varset);
							}

					// Process OPTIONAL //
					if (sub_result->results.empty() || !sub_result->results[0].result.empty())
					{
						for (int l = 0; l < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); l++)
						{
							if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].type == GroupPattern::SubGroupPattern::Optional_type)
							{
								if ((int)this->rewriting_evaluation_stack.size() == dep + 1)
								{
									this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
									this->rewriting_evaluation_stack.back().result = NULL;
									// group_pattern = &this->rewriting_evaluation_stack[dep].group_pattern;
								}

								this->rewriting_evaluation_stack[dep].result = sub_result;
								this->rewriting_evaluation_stack[dep + 1].group_pattern = \
									this->rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].optional;
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
					// NOTE: Why separate into two stages? //
					for (int l = 0; l < (int)rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size(); l++)
					{
						if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].type == GroupPattern::SubGroupPattern::Filter_type)
						{
							if (!rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].filter.done)
							{
								rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].filter.done = true;
								// If FILTER scope is global && no ORDER BY && is the last operation, doFilter with limit
								if (dep == 0 && query_tree.getOrderByVarset().empty() && l == rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size() - 1) {
									sub_result->doFilter(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].filter, kvstore,\
										rewriting_evaluation_stack[dep].group_pattern.group_pattern_subject_object_maximal_varset, query_tree.getLimit());
								} else
									sub_result->doFilter(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[l].filter, kvstore,\
										rewriting_evaluation_stack[dep].group_pattern.group_pattern_subject_object_maximal_varset);
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
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Optional_type || group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Minus_type)
		{
			this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
			this->rewriting_evaluation_stack.back().group_pattern = group_pattern.sub_group_pattern[i].optional;
			this->rewriting_evaluation_stack.back().result = NULL;
			this->rewriting_evaluation_stack[dep].result = result;
			TempResultSet *temp = queryEvaluation(dep + 1);
			{
				TempResultSet *new_result = new TempResultSet();

				if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Optional_type)
					result->doOptional(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);
				else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Minus_type)
					result->doMinus(*temp, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

				temp->release();
				result->release();
				delete temp;
				delete result;

				result = new_result;
				result->initial = false;
			}
		}
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Filter_type)
		{
            // If FILTER scope is global && no ORDER BY && is the last operation, doFilter with limit
			if (dep == 0 && query_tree.getOrderByVarset().empty() && i == group_pattern.sub_group_pattern.size() - 1)
				result->doFilter(group_pattern.sub_group_pattern[i].filter, kvstore, group_pattern.group_pattern_subject_object_maximal_varset, query_tree.getLimit());
			else
				result->doFilter(group_pattern.sub_group_pattern[i].filter, kvstore, group_pattern.group_pattern_subject_object_maximal_varset);
			result->initial = false;
		}
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Bind_type)
		{
			result->doBind(group_pattern.sub_group_pattern[i].bind, kvstore, \
				group_pattern.group_pattern_subject_object_maximal_varset);
		}
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Subquery_type)
		{ 
            QueryTree& sub_query=group_pattern.sub_group_pattern[i].subquery;
            if(sub_query.getOffset()==0&&sub_query.getLimit()==-1&&sub_query.getGroupByVarset().empty()&&sub_query.getOrderVarVector().empty() \
				&& sub_query.getProjectionModifier() == QueryTree::Modifier_None)
			{
                cout<<"### Subtree (simp.) print start###\n";
		    	sub_query.getGroupPattern().print(0);
                cout<<"### Subtree (simp.) print end###\n";
		    	// group_pattern.sub_group_pattern[i].group_pattern.getVarset();
                this->rewriting_evaluation_stack.push_back(EvaluationStackStruct());
				this->rewriting_evaluation_stack.back().group_pattern = sub_query.getGroupPattern();
				this->rewriting_evaluation_stack.back().result = NULL;
				this->rewriting_evaluation_stack[dep].result = result;
				TempResultSet *temp = queryEvaluation(dep + 1); //TODO: IF parent IS NOT NULL, IT SHOULD BE JOINED
                cout<<"<SIMP> number of results: ";
                for(std::vector<TempResult>::iterator it=temp->results.begin();it!=temp->results.end(); it++) {
                    Varset rvs=it->getAllVarset();
                    for(std::vector<std::string>::iterator it2=rvs.vars.begin();it2!=rvs.vars.end(); it2++){
                        cout<<*it2<<",";
                    }
                    cout<<it->result.size()<<"; ";
                }
                cout<<endl;
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
            } else {
                GeneralEvaluation tmp_GeneralEvaluation=*this;
                
		    	tmp_GeneralEvaluation.getQueryTree()=sub_query;
                cout<<"### Subtree print start###\n";
                tmp_GeneralEvaluation.getQueryTree().print();
                cout<<"### Subtree print end###\n";
		    	// group_pattern.sub_group_pattern[i].group_pattern.getVarset();
                tmp_GeneralEvaluation.doQuery();
		    	ResultSet* temp_rs = new ResultSet();
                tmp_GeneralEvaluation.getFinalResult(*temp_rs);
                cout<<"<NONSIMP> number of results: "<<temp_rs->ansNum<<endl;
                TempResultSet *temp_trs = new TempResultSet();
                temp_trs->results.push_back(temp_rs->to_tempresult());
				if (result->results.empty())
		    	{
		    		delete result;
		    		result = temp_trs;
		    	}
				else
				{
					TempResultSet *new_result = new TempResultSet();
					result->doJoin(*temp_trs, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);
					temp_trs->release();
					result->release();
					delete temp_trs;
					delete result;

					result = new_result;
				}
            }
            cout<<"### Subquery ###"<<endl;
		}
	}

	// for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
	// {
	// 	if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Pattern_type
	// 		&& group_pattern.sub_group_pattern[i].pattern.kleene)
	// 	{
	// 		// PathQueryHandler function arguments are vertex IDs
	// 		// Use `tr` to store BFS starting vertices
	// 		TempResultSet *sub_result = new TempResultSet();
	// 		TempResult *tr = NULL;
	// 		if (result->results.size() > 0 && result->results[0].result.size() > 0)
	// 			tr = &(result->results[0]);

	// 		// BFS from the subjects / objects to find results
	// 		kleeneClosure(sub_result, tr, group_pattern.sub_group_pattern[i].pattern.subject.value,
	// 			group_pattern.sub_group_pattern[i].pattern.predicate.value, group_pattern.sub_group_pattern[i].pattern.object.value, dep);
			
	// 		// TODO: cache result
	// 		if (result->results.empty())
	// 		{
	// 			delete result;
	// 			result = sub_result;
	// 		}
	// 		else
	// 		{
	// 			TempResultSet *new_result = new TempResultSet();
	// 			result->doJoin(*sub_result, *new_result, this->stringindex, this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset);

	// 			sub_result->release();
	// 			result->release();
	// 			delete sub_result;
	// 			delete result;

	// 			result = new_result;
	// 		}
	// 	}
	// }

	// result->print();

	this->rewriting_evaluation_stack.pop_back();

	return result;
}

bool GeneralEvaluation::expanseFirstOuterUnionGroupPattern(GroupPattern &group_pattern, deque<GroupPattern> &queue)
{
	// group_pattern is the top element of queue
	int first_union_pos = -1;

	// Iterate across sub-group-patterns, find the first UNION
	for (int i = 0; i < (int)group_pattern.sub_group_pattern.size(); i++)
		if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Union_type)
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
		GroupPattern new_group_pattern;

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

void GeneralEvaluation::getFinalResult(ResultSet &ret_result)
{

	if (this->temp_result == NULL)
	{
		return;
	}
	
	char* string_index_buffer = NULL;
	unsigned string_index_buffer_size = 0;
	if (this->query_tree.getQueryForm() == QueryTree::Select_Query)
	{
		// long t0 = Util::get_cur_time();

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

		if (this->query_tree.checkAtLeastOneAggregateFunction() \
		|| this->query_tree.checkAtLeastOneCompTree()\
		|| !this->query_tree.getGroupByVarset().empty())
		{
			// vector<ProjectionVar> &proj = this->query_tree.getProjection();
			vector<ProjectionVar> proj = this->query_tree.getProjection();
			vector<string> order_vars = query_tree.getOrderByVarset().vars;
			for (string var : order_vars)
			{
				bool already_exist = false;
				for (auto p : proj)
				{
					if (p.var == var)
					{
						already_exist = true;
						break;
					}
				}
				if (already_exist)
					continue;
				ProjectionVar proj_var;
				proj_var.aggregate_type = ProjectionVar::None_type;
				proj_var.var = var;
				proj.push_back(proj_var);
			}

			TempResultSet *new_temp_result = new TempResultSet();
			new_temp_result->results.push_back(TempResult());

			TempResult &result0 = this->temp_result->results[0];
			Varset result0Varset = result0.id_varset + result0.str_varset;
			TempResult &new_result0 = new_temp_result->results[0];

			for (int i = 0; i < (int)proj.size(); i++)
				if (proj[i].aggregate_type == ProjectionVar::None_type)
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
				if (proj[i].aggregate_type == ProjectionVar::None_type)
					proj2temp[i] = Varset(proj[i].var).mapTo(result0.getAllVarset())[0];
				else if (proj[i].aggregate_var != "*")
					proj2temp[i] = Varset(proj[i].aggregate_var).mapTo(result0.getAllVarset())[0];

			// vector<int> proj2new = this->query_tree.getProjectionVarset().mapTo(new_result0.getAllVarset());
			vector<int> proj2new((int)proj.size(), -1);
			for (int i = 0; i < (int)proj.size(); i++)
				proj2new[i] = Varset(proj[i].var).mapTo(new_result0.getAllVarset())[0];

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
				if (proj[i].aggregate_type == ProjectionVar::Count_type && proj[i].distinct && proj[i].aggregate_var == "*")
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

			// TODO: (Refactor)
			// 1. Make a fine distinction between aggregates and non-aggregate functions
			// 2. For non-aggregate functions, split argument gathering and executing
			// (possibly get rid of PathArgs and stick with a string vector for args?)
			if (result0_size == 0 && proj.size() == 1 && \
				proj[0].aggregate_type != ProjectionVar::None_type && \
				proj[0].aggregate_type != ProjectionVar::CompTree_type && \
				proj[0].aggregate_type != ProjectionVar::Count_type && \
				proj[0].aggregate_type != ProjectionVar::Groupconcat_type && \
				proj[0].aggregate_type != ProjectionVar::Sum_type && \
				proj[0].aggregate_type != ProjectionVar::Avg_type && \
				proj[0].aggregate_type != ProjectionVar::Min_type && \
				proj[0].aggregate_type != ProjectionVar::Max_type && \
				proj[0].aggregate_type != ProjectionVar::Contains_type && \
				proj[0].aggregate_type != ProjectionVar::Sample_type )
			{
				// Non-aggregate functions, no var (all constant) in arg list

				new_result0.result.emplace_back();
				new_result0.result.back().id = new unsigned[new_result0_id_cols];
				new_result0.result.back().sz = new_result0_id_cols;
				new_result0.result.back().str.resize(new_result0_str_cols);

				for (int i = 0; i < new_result0_id_cols; i++)
					new_result0.result.back().id[i] = INVALID;

				// Path functions
				if (proj[0].aggregate_type != ProjectionVar::PFN_type)
				{
					prepPathQuery();
					vector<int> uid_ls, vid_ls;
					vector<vector<int>> uid_ls_ls;
					vector<int> pred_id_set;
					uid_ls_ls.push_back(vector<int>());
					for (auto vert : proj[0].path_args.vert_set)
						uid_ls_ls.back().push_back(kvstore->getIDByString(vert));
					if (!proj[0].path_args.src.empty())
						uid_ls.push_back(kvstore->getIDByString(proj[0].path_args.src));
					else
						uid_ls.push_back(-1);	// Dummy for loop
					// if (proj[0].aggregate_type != ProjectionVar::ppr_type)
					if (!proj[0].path_args.dst.empty())
						vid_ls.push_back(kvstore->getIDByString(proj[0].path_args.dst));
					else
						vid_ls.push_back(-1);	// Dummy for loop
					if (!proj[0].path_args.pred_set.empty())
					{
						for (auto pred : proj[0].path_args.pred_set)
						{
							TYPE_PREDICATE_ID pred_id = kvstore->getIDByPredicate(pred);
							// cout << "pred_set id:" << pred_id << endl;
							if (pred_id != static_cast<int>(INVALID))
								pred_id_set.push_back(pred_id);
						}
					}
					else
					{
						// Allow all predicates except those in neg_pred_set
						unordered_set<int> neg_pred_id_set;
						for (string pred : proj[0].path_args.neg_pred_set)
							neg_pred_id_set.emplace(kvstore->getIDByPredicate(pred));
						unsigned pre_num = stringindex->getNum(StringIndexFile::Predicate);
						for (unsigned j = 0; j < pre_num; j++)
							if (neg_pred_id_set.find(j) == neg_pred_id_set.end())
								pred_id_set.push_back(j);
					}

					// For each u-v pair, query
					unordered_set<pair<int, int>, hashFunction> unique_pairs;
					bool exist = 0, earlyBreak = 0;	// Boolean queries can break early with true
					stringstream ss;
					bool notFirstOutput = 0;	// For outputting commas
					bool doneOnceOp = 0;	// functions that only need to do once (triangleCounting, pr, labelProp, wcc, clusteringCoeff without source)
					ss << "\"{\"paths\":[";
					cout<<"proj[0].aggregate_type :"<<proj[0].aggregate_type <<endl;
					if (proj[0].aggregate_type == ProjectionVar::maximumKplex_type)
					{
						auto kplex = pqHandler->maximumKplex(pred_id_set, proj[0].path_args.k);
						bool localFirstOutput = true;
						if (notFirstOutput)
							ss << ',';
						else
							notFirstOutput = true;
						ss << "{\"kplex\":[";
						for (int vid : kplex)
						{
							if (localFirstOutput)
								localFirstOutput = false;
							else
								ss << ',';
							ss << kvstore->getStringByID(vid);
						}
						ss << "]}";
					} else if (proj[0].aggregate_type == ProjectionVar::coreTruss_type) {
						auto clique = pqHandler->coreTruss(pred_id_set, proj[0].path_args.misc[0], proj[0].path_args.misc[1]);
						ss << "{\"coreTruss\":[";
						bool localFirstOutput = true;
						for (int vid : clique) {
							if (localFirstOutput)
								localFirstOutput = false;
							else
								ss << ',';
							ss << kvstore->getStringByID(vid);
						}
						ss << "]}";
					}
					for (int uid : uid_ls)
					{
						for (int vid : vid_ls)
						{
							auto uvpair = make_pair(uid, vid);
							if (unique_pairs.find(uvpair) != unique_pairs.end())
								continue;
							unique_pairs.insert(uvpair);
							if (proj[0].aggregate_type == ProjectionVar::cyclePath_type)
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
							else if (proj[0].aggregate_type == ProjectionVar::cycleBoolean_type)
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
							else if (proj[0].aggregate_type == ProjectionVar::shortestPath_type)
							{
								if (uid == vid)
								{
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									vector<int> path; // Empty path
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
							else if (proj[0].aggregate_type == ProjectionVar::kHopReachablePath_type)
							{
								if (uid == vid)
								{
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									vector<int> path; // Empty path
									pathVec2JSON(uid, vid, path, ss);
									continue;
								}
								int hopConstraint = proj[0].path_args.k;
								if (hopConstraint < 0)
									hopConstraint = 999;
								vector<int> path = pqHandler->kHopReachablePath(uid, vid, proj[0].path_args.directed, hopConstraint, pred_id_set);
								if (path.size() != 0)
								{
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									pathVec2JSON(uid, vid, path, ss);
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::kHopEnumerate_type)
							{
								if (uid == vid)
								{
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									vector<int> path; // Empty path
									pathVec2JSON(uid, vid, path, ss);
									continue;
								}
								int hopConstraint = proj[0].path_args.k;
								if (hopConstraint < 0)
									hopConstraint = 999;
								vector<vector<int>> paths = pqHandler->kHopEnumeratePath(uid, vid, proj[0].path_args.retNum, \
									proj[0].path_args.directed, hopConstraint, pred_id_set);
								if (!paths.empty())
								{
									for (auto path : paths)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										pathVec2JSON(uid, vid, path, ss);
									}
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::shortestPathLen_type)
							{
								if (uid == vid)
								{
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									ss << "{\"src\":\"" << kvstore->getStringByID(uid)
									   << "\",\"dst\":\"" << kvstore->getStringByID(vid)
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
									ss << "{\"src\":\"" << kvstore->getStringByID(uid)
									   << "\",\"dst\":\"" << kvstore->getStringByID(vid) << "\",\"length\":";
									ss << (path.size() - 1) / 2;
									ss << "}";
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::kHopReachable_type)
							{
								if (uid == vid)
								{
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									ss << "{\"src\":\"" << kvstore->getStringByID(uid)
									   << "\",\"dst\":\"" << kvstore->getStringByID(vid)
									   << "\",\"value\":\"true\"}";
									continue;
								}
								int hopConstraint = proj[0].path_args.k;
								if (hopConstraint < 0)
									hopConstraint = 999;
								bool reachRes = pqHandler->kHopReachable(uid, vid, proj[0].path_args.directed, hopConstraint, pred_id_set);
								ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"dst\":\""
								   << kvstore->getStringByID(vid) << "\",\"value\":";
								if (reachRes)
									ss << "\"true\"}";
								else
									ss << "\"false\"}";
								// cout << "src = " << kvstore->getStringByID(uid) << ", dst = " << kvstore->getStringByID(vid) << endl;
							}
							else if (proj[0].aggregate_type == ProjectionVar::ppr_type)
							{
								vector<pair<int, double>> v2ppr;
								pqHandler->SSPPR(uid, proj[0].path_args.retNum, proj[0].path_args.k, pred_id_set, v2ppr);
								ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"results\":[";
								for (auto it = v2ppr.begin(); it != v2ppr.end(); ++it)
								{
									if (it != v2ppr.begin())
										ss << ",";
									ss << "{\"dst\":\"" << kvstore->getStringByID(it->first) << "\",\"PPR\":"
									   << it->second << "}";
								}
								ss << "]}";
							}
							else if (proj[0].aggregate_type == ProjectionVar::triangleCounting_type)
							{
								if (!doneOnceOp)
								{
									auto ret = pqHandler->triangleCounting(proj[0].path_args.directed, pred_id_set);
									ss << to_string(ret);
									doneOnceOp = true;
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::closenessCentrality_type)
							{
								auto ret = pqHandler->closenessCentrality(uid, proj[0].path_args.directed, pred_id_set);
								ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"result\":";
								ss << to_string(ret) << "}";
							}
							else if (proj[0].aggregate_type == ProjectionVar::bfsCount_type)
							{
								auto ret = pqHandler->bfsCount(uid, proj[0].path_args.directed, pred_id_set);
								size_t retSz = ret.size();
								ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"results\":[";
								for (size_t j = 0; j < retSz; j++)
								{
									ss << "{\"depth\":" << j << ", \"count\":" << ret[j] << "}";
									if (j != retSz - 1)
										ss << ", ";
								}
								ss << "]}";
							}
							else if (proj[0].aggregate_type == ProjectionVar::pr_type)
							{
								if (!doneOnceOp)
								{
									vector<pair<int, double>> idx2val;
									pqHandler->PR(proj[0].path_args.directed, pred_id_set, proj[0].path_args.retNum, proj[0].path_args.misc[0], \
										proj[0].path_args.misc[1], proj[0].path_args.misc[2], idx2val);
									bool localFirstOutput = true;
									// for (auto mp : ret)
									for (size_t i = 0; i < idx2val.size(); i++)
									{
										if (localFirstOutput)
											localFirstOutput = false;
										else
											ss << ",";
										ss << "{\"src\":\"" << kvstore->getStringByID(idx2val[i].first) << "\",\"result\":" << idx2val[i].second << "}";
									}
									doneOnceOp = true;
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::sssp_type)
							{
								if (notFirstOutput)
									ss << ",";
								else
									notFirstOutput = 1;
								auto paths = pqHandler->SSSP(uid, proj[0].path_args.directed, pred_id_set);
								bool localFirstOutput = true;
								for (auto mp : paths)
								{
									if (mp.second.size() != 0)
									{
										if (localFirstOutput)
											localFirstOutput = false;
										else
											ss << ",";
										pathVec2JSON(uid, mp.second.back(), mp.second, ss);
									}
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::sssplen_type)
							{
								if (notFirstOutput)
									ss << ",";
								else
									notFirstOutput = 1;
								auto paths = pqHandler->SSSPLen(uid, proj[0].path_args.directed, pred_id_set);
								bool localFirstOutput = true;
								for (auto mp : paths)
								{
									if (localFirstOutput)
										localFirstOutput = false;
									else
										ss << ",";
									
									ss << "{\"src\":\"" << kvstore->getStringByID(uid)
									   << "\",\"dst\":\"" << kvstore->getStringByID(mp.first) \
									   << "\",\"length\":" << mp.second << "}";
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::labelProp_type)
							{
								if (!doneOnceOp)
								{
									auto ret = pqHandler->labelProp(proj[0].path_args.directed, pred_id_set, proj[0].path_args.misc[0]);
									bool localFirstOutput = true;
									for (auto mp : ret)
									{
										if (localFirstOutput)
											localFirstOutput = false;
										else
											ss << ",";
										vertVec2JSON(mp, ss);
									}
									doneOnceOp = true;
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::wcc_type)
							{
								if (!doneOnceOp)
								{
									auto ret = pqHandler->WCC(pred_id_set);
									bool localFirstOutput = true;
									for (auto mp : ret)
									{
										if (localFirstOutput)
											localFirstOutput = false;
										else
											ss << ",";
										vertVec2JSON(mp, ss);
									}
									doneOnceOp = true;
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::clusterCoeff_type)
							{
								if (uid == -1)	// Source not given, compute once globally
								{
									if (!doneOnceOp)
									{
										auto ret = pqHandler->clusteringCoeff(proj[0].path_args.directed, pred_id_set);
										ss << to_string(ret);
										doneOnceOp = true;
									}
								}
								else
								{
									auto ret = pqHandler->clusteringCoeff(uid, proj[0].path_args.directed, pred_id_set);
									ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"result\":";
									ss << to_string(ret) << "}";
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::kHopCount_type)
							{
								int hopConstraint = proj[0].path_args.k;
								auto ret = pqHandler->kHopCount(uid, proj[0].path_args.directed, hopConstraint, pred_id_set);
								ss << "{\"src\":\"" << kvstore->getStringByID(uid) 
									<< "\",\"depth\":" << hopConstraint 
									<< ", \"count\":" << ret << "}";
							}
							else if (proj[0].aggregate_type == ProjectionVar::kHopNeighbor_type)
							{
								int hopConstraint = proj[0].path_args.k;
								int retNum = proj[0].path_args.retNum;
								if (retNum < 0) {
									retNum = 100;
								}
								auto ret = pqHandler->kHopNeighbor(uid, proj[0].path_args.directed, hopConstraint, pred_id_set, retNum);
								ss << "{\"src\":\"" << kvstore->getStringByID(uid) 
									<< "\",\"depth\":" << hopConstraint 
									<< ", \"dst\":["; 
								bool hasMore = false;
								for (int nid : ret)
								{
									if (hasMore)
										ss << ","; 
									else 
										hasMore = true;
									ss << "\"" + kvstore->getStringByID(nid) + "\"";
								}
								ss << "]}";
							}
							else if (proj[0].aggregate_type == ProjectionVar::shortestPathCount_type)
							{
								auto ret = pqHandler->shortestPathCount(uid, vid, proj[0].path_args.directed, pred_id_set);
								ss << "{\"src\":\"" << kvstore->getStringByID(uid)
									<< "\", \"dst\":\"" << kvstore->getStringByID(vid)
									<< "\", \"count\":" << ret << "}";
							} else if (proj[0].aggregate_type == ProjectionVar::IC14_type) {
								int knowsPred = kvstore->getIDByPredicate("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/directKnows>");
								int hasCreatorPred = kvstore->getIDByPredicate("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/hasCreator>");
								int typePred = kvstore->getIDByPredicate("<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>");
								int replyPred = kvstore->getIDByPredicate("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/replyOf>");
								int idPred = kvstore->getIDByPredicate("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/id>");
								int postId = kvstore->getIDByString("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/Post>");
								int commentId = kvstore->getIDByString("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/Comment>");
								auto ret = pqHandler->IC14(uid, vid, knowsPred, hasCreatorPred, typePred, replyPred, postId, commentId);
								if (!ret.empty())
								{
									size_t pathSz = ret[0].first.size();
									unsigned* objidlist = nullptr;
									unsigned list_len = 0;
									bool invalid = false;
									vector<int> pathVert;
									for (const auto &path2W : ret)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										invalid = false;
										pathVert.clear();
										for (size_t i = 0; i < pathSz; i += 2) {
											kvstore->getobjIDlistBysubIDpreID(path2W.first[i], idPred, objidlist, list_len);
											if (list_len != 1) {
												invalid = true;
												break;
											}
											pathVert.emplace_back(objidlist[0]);
											delete []objidlist;
										}
										if (invalid)
											continue;
										ss << "{\"path\":[";
										string tmp;
										size_t lPos = 0, rPos = 0;
										for (size_t i = 0; i < pathVert.size(); i++) {
											tmp = kvstore->getStringByID(pathVert[i]);
											lPos = tmp.find('\"');
											rPos = tmp.find('\"', lPos + 1);
											if (i > 0)
												ss << ",";
											ss << tmp.substr(lPos + 1, rPos - 1 - lPos);
										}
										ss << "],\"weight\":" << path2W.second << "}";
									}
								}
							}
							else if (proj[0].aggregate_type == ProjectionVar::louvain_type)
							{
								if (!doneOnceOp)
								{
									pair<size_t, map<int,set<int> > > result;
									float min_modularity_increase = proj[0].path_args.misc[0];
									int phase1_loop_num = proj[0].path_args.misc[1];
									pqHandler->louvain(phase1_loop_num, min_modularity_increase, pred_id_set, proj[0].path_args.directed, result);
									ss << "{\"count\":" << result.first << ", \"details\":[";
									for (auto it=result.second.begin(); it != result.second.end(); it++ )
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										ss << "{\"communityId\":\""<<it->first <<"\", \"menberNum\":" << it->second.size() << "}";
										// ss << "{\"communityId\":\""<<it->first <<"\", \"menbers\": [";
										// bool hasMore = false;
										// for (int mid : it->second)
										// {
										// 	if (hasMore) {
										// 		ss << ",";
										// 	} 
										// 	else 
										// 	{
										// 		hasMore = true;
										// 	}
										// 	ss << "\"" + kvstore->getStringByID(mid) + "\"";
										// }
										// ss << "]";
									}
									ss << "]}";
									doneOnceOp = true;
								}
							}
						}
						if (earlyBreak)
							break;
					}
					ss << "]}\"";
					// cout << "max #paths:" << uid_ls.size() * vid_ls.size() << endl;
					if (proj[0].aggregate_type == ProjectionVar::cycleBoolean_type)
					{
						if (exist)
							new_result0.result.back().str[proj2new[0] - new_result0_id_cols] = "\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
						else
							new_result0.result.back().str[proj2new[0] - new_result0_id_cols] = "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
					}
					else
					{
						new_result0.result.back().str[proj2new[0] - new_result0_id_cols] = ss.str();
					}
				}
				else if (proj[0].aggregate_type == ProjectionVar::PFN_type)
				{
					prepPathQuery();
					vector<int> iri_id_set;
					if (!proj[0].path_args.iri_set.empty())
					{
						for (auto iri : proj[0].path_args.iri_set)
							iri_id_set.push_back(kvstore->getIDByString(iri));
					}
					else
					{
						throw runtime_error("iri_set can not be empty!");
						return;
					}

					int hopConstraint = proj[0].path_args.k;
					bool directed = proj[0].path_args.directed;
					vector<int> pred_id_set;
					if (!proj[0].path_args.pred_set.empty())
					{
						for (auto pred : proj[0].path_args.pred_set)
						{
							TYPE_PREDICATE_ID pred_id = kvstore->getIDByPredicate(pred);
							// cout << "pred_set id:" << pred_id << endl;
							if (pred_id != static_cast<int>(INVALID))
								pred_id_set.push_back(pred_id);
						};
					}
					else
					{
						// Allow all predicates except those in neg_pred_set
						unordered_set<int> neg_pred_id_set;
						for (string pred : proj[0].path_args.neg_pred_set)
							neg_pred_id_set.emplace(kvstore->getIDByPredicate(pred));
						unsigned pre_num = stringindex->getNum(StringIndexFile::Predicate);
						for (unsigned j = 0; j < pre_num; j++)
							if (neg_pred_id_set.find(j) == neg_pred_id_set.end())
								pred_id_set.push_back(j);
					}
					string fun_name = proj[0].path_args.fun_name;
					fun_name = Util::replace_all(fun_name, "\"", "");
					#if defined(DEBUG)
					cout<<">>>>>>>>>>>>> PFN-1 args <<<<<<<<<<<<<<<\n"
						<<"iri_id_set=";
					for (size_t i = 0; i < iri_id_set.size(); i++)
					{
						if (i > 0)
							cout << ", ";
						cout << iri_id_set[i];
					}	
					cout<< "\ndirected=" << directed
						<< "\nk=" << hopConstraint
						<< "\npred_id_set=";
					for (size_t i = 0; i < pred_id_set.size(); i++)
					{
						if (i > 0)
							cout << ", ";
						cout << pred_id_set[i];
					}
					cout << "\nfun_name=" << fun_name 
						<<"\n>>>>>>>>>>>>> PFN-1 args <<<<<<<<<<<<<<<\n"
						<< endl;
					#endif
					stringstream ss;
					std::map<std::string, std::string> rt = dynamicFunction(iri_id_set, directed, hopConstraint, pred_id_set, fun_name, ret_result.getUsername());
					std::map<std::string, std::string>::iterator iter = rt.find("return_type");
					string rt_type = rt.find("return_type")->second;
					string str = rt.find("return_value")->second;
					ss << "\"";
					if (rt_type == "path" && iri_id_set.size() == 2)
					{
						vector<string> path_str;
						vector<int> path_int;
						Util::split(str, ",", path_str);
						size_t len = path_str.size();
						for (size_t i = 0; i < len; i++)
						{
							path_int.push_back(Util::string2int(path_str[i]));
						}
						pathVec2JSON(iri_id_set[0], iri_id_set[1], path_int, ss);
					}
					else if (rt_type == "kvalue") 
					{
						// str is JSONArray sting: [{vid1:value1},{vid2:value2}...]
						rapidjson::Document doc;
						doc.IsArray();
						doc.Parse(str.c_str());
						string iri_src = kvstore->getStringByID(iri_id_set[0]);
						ss << "[";
						for (size_t i = 0; i < doc.Size(); i++)
						{
							if (i > 0) 
							{
								ss << ",";
							}
							rapidjson::Value obj = doc[i].GetObject();
							string dst_str = obj.HasMember("dst") ? obj["dst"].GetString(): "unkown";
							string value_str = obj.HasMember("value") ? obj["value"].GetString() : "";
							string iri_dst = "";
							if (Util::is_number(dst_str))
							{
								iri_dst = kvstore->getStringByID(Util::string2int(dst_str));
							} 
							else
							{
								iri_dst = dst_str;
							}
							ss << "{\"src\":\"" <<iri_src << "\",\"dst\":\""<< iri_dst << "\",\"value\":\""
								<< value_str << "\"}";
						}
						ss << "]";
					}
					else
					{
						ss << str;
					}
					ss << "\"";
					ss >> new_result0.result.back().str[proj2new[0] - new_result0_id_cols];
				}
			}
			// Exclusive with the if branch above
			for (int begin = 0; begin < result0_size;)
			{
				// At the end of an iteration, begin will be set to end + 1
				// The value of end will depend on GROUP BY conditions
				int end;
				if (group2temp.empty()) {
					end = result0_size - 1;
					int resultSz = result0_size;
					if (proj.size() == 1 && proj[0].aggregate_type != ProjectionVar::None_type \
					&& proj[0].aggregate_type != ProjectionVar::CompTree_type && proj[0].aggregate_type != ProjectionVar::Contains_type \
					&& proj[0].aggregate_type != ProjectionVar::PFN_type)
						resultSz = 1;
					// Reserve all lines of results
					new_result0.result.resize(resultSz);
					for (size_t i = 0; i < resultSz; i++) {
						new_result0.result[i].id = new unsigned[new_result0_id_cols];
						for (size_t j = 0; j < new_result0_id_cols; j++)
							new_result0.result[i].id[j] = INVALID;
						new_result0.result[i].sz = new_result0_id_cols;
						new_result0.result[i].str.resize(new_result0_str_cols);
					}
				}
				else {
					end = result0.findRightBounder(group2temp, result0.result[begin], result0_id_cols, group2temp);
					new_result0.result.emplace_back();
					new_result0.result.back().id = new unsigned[new_result0_id_cols];
					new_result0.result.back().sz = new_result0_id_cols;
					new_result0.result.back().str.resize(new_result0_str_cols);
					for (int i = 0; i < new_result0_id_cols; i++)
						new_result0.result.back().id[i] = INVALID;
				}


				for (int i = 0; i < (int)proj.size(); i++)
				{
					if (proj[i].aggregate_type == ProjectionVar::None_type)
					{
						if (group2temp.empty()) {
							for (int j = begin; j <= end; j++) {
								if (proj2temp[i] < result0_id_cols)
									new_result0.result[j].id[proj2new[i]] = result0.result[j].id[proj2temp[i]];
								else
									new_result0.result[j].str[proj2new[i] - new_result0_id_cols] = result0.result[j].str[proj2temp[i] - result0_id_cols];
							}
						} else {
							if (proj2temp[i] < result0_id_cols)
								new_result0.result.back().id[proj2new[i]] = result0.result[begin].id[proj2temp[i]];
							else
								new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = result0.result[begin].str[proj2temp[i] - result0_id_cols];
						}
					}
					else if (proj[i].aggregate_type == ProjectionVar::Count_type
						|| proj[i].aggregate_type == ProjectionVar::Sum_type
						|| proj[i].aggregate_type == ProjectionVar::Avg_type
						|| proj[i].aggregate_type == ProjectionVar::Min_type
						|| proj[i].aggregate_type == ProjectionVar::Max_type
						|| proj[i].aggregate_type == ProjectionVar::Groupconcat_type
						|| proj[i].aggregate_type == ProjectionVar::Sample_type)
					{
						int count = 0;
						bool numeric = false, datetime = false, succ = true;
						EvalMultitypeValue numeric_sum, numeric_min, numeric_max, datetime_min, datetime_max, tmp, res;
						string groupconcat;
						string sample_result;
						numeric_sum.datatype = EvalMultitypeValue::xsd_integer;
						numeric_sum.int_value = 0;
						numeric_min.datatype = EvalMultitypeValue::xsd_long;
						numeric_min.long_value = LONG_LONG_MAX;
						numeric_max.datatype = EvalMultitypeValue::xsd_long;
						numeric_max.long_value = LONG_LONG_MIN;
						datetime_min.datatype = EvalMultitypeValue::xsd_datetime;
						datetime_max.datatype = EvalMultitypeValue::xsd_datetime;

						if (proj[i].aggregate_var != "*")
						{
							if (proj[i].distinct)
							{
								set<string> count_set;
								Varset result0Varset = result0.getAllVarset();
								for (int j = begin; j <= end; j++)
								{
									tmp = result0.doComp(proj[i].comp_tree_root, result0.result[j], result0_id_cols, kvstore, \
										result0Varset);
									if (tmp.datatype == EvalMultitypeValue::xsd_boolean && \
										tmp.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::error_value)
										continue;
									numeric = false;
									datetime = false;
									if (tmp.datatype != EvalMultitypeValue::xsd_integer && tmp.datatype != EvalMultitypeValue::xsd_decimal
										&& tmp.datatype != EvalMultitypeValue::xsd_float && tmp.datatype != EvalMultitypeValue::xsd_double
										&& tmp.datatype != EvalMultitypeValue::xsd_long)
									{
										if (proj[i].aggregate_type == ProjectionVar::Sum_type
											|| proj[i].aggregate_type == ProjectionVar::Avg_type)
										{
											cout << "[ERROR] Invalid type for SUM or AVG." << endl;
											continue;
										}
										else if ((proj[i].aggregate_type == ProjectionVar::Min_type
											|| proj[i].aggregate_type == ProjectionVar::Max_type)
											&& tmp.datatype != EvalMultitypeValue::xsd_datetime)
										{
											cout << "[ERROR] Invalid type for MIN or MAX." << endl;
											continue; 
										}
									}
									else
									{
										if (!numeric)
											numeric = true;
									}
									if (tmp.datatype == EvalMultitypeValue::xsd_datetime)
									{
										if (!datetime)
											datetime = true;
									}
									if (!datetime && !numeric && proj[i].aggregate_type != ProjectionVar::Count_type \
										&& proj[i].aggregate_type != ProjectionVar::Groupconcat_type \
										&& proj[i].aggregate_type != ProjectionVar::Sample_type)
									{
										succ = false;
										break;
									}

									if (proj[i].aggregate_type == ProjectionVar::Count_type
										|| proj[i].aggregate_type == ProjectionVar::Sum_type
										|| proj[i].aggregate_type == ProjectionVar::Avg_type)
										count_set.insert(tmp.term_value);
									else if (proj[i].aggregate_type == ProjectionVar::Groupconcat_type)
										count_set.insert(tmp.str_value);
									else if (proj[i].aggregate_type == ProjectionVar::Sample_type)
										sample_result = tmp.str_value;
									else if (proj[i].aggregate_type == ProjectionVar::Min_type)
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
									else if (proj[i].aggregate_type == ProjectionVar::Max_type)
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
								// for (auto elem : count_set)
								// 	cout << elem << ' ';
								cout << endl;
								count = (int)count_set.size();
								if (proj[i].aggregate_type == ProjectionVar::Sum_type
									|| proj[i].aggregate_type == ProjectionVar::Avg_type)
								{
									for (string elem : count_set)
									{
										tmp.term_value = elem;
										tmp.deduceTypeValue();

										numeric_sum = numeric_sum + tmp;
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::Groupconcat_type)
								{
									for (string elem : count_set)
									{
										size_t bIdx = elem.find('\"'), eIdx = elem.rfind('\"');
										if (bIdx == string::npos || eIdx == string::npos || bIdx == eIdx)
											continue;
										if (groupconcat.empty())
											groupconcat += "\"";
										else
											groupconcat += proj[i].separator;
										groupconcat += elem.substr(bIdx + 1, eIdx - bIdx - 1);
									}
									if (!groupconcat.empty())
										groupconcat += "\"";
								}
								if (proj[i].aggregate_type == ProjectionVar::Avg_type)
								{
									tmp.term_value = "\"" + to_string(count) + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
									tmp.deduceTypeValue();

									numeric_sum = numeric_sum / tmp;
								}
							}
							else 	// No DISTINCT
							{
								Varset result0Varset = result0.getAllVarset();
								for (int j = begin; j <= end; j++)
								{
									tmp = result0.doComp(proj[i].comp_tree_root, result0.result[j], result0_id_cols, kvstore, \
										result0Varset);
									if (tmp.datatype == EvalMultitypeValue::xsd_boolean && \
										tmp.bool_value.value == EvalMultitypeValue::EffectiveBooleanValue::error_value)
										continue;
									if (tmp.datatype != EvalMultitypeValue::xsd_integer
										&& tmp.datatype != EvalMultitypeValue::xsd_decimal
										&& tmp.datatype != EvalMultitypeValue::xsd_float
										&& tmp.datatype != EvalMultitypeValue::xsd_double
										&& tmp.datatype != EvalMultitypeValue::xsd_long)
									{
										if (proj[i].aggregate_type == ProjectionVar::Sum_type
											|| proj[i].aggregate_type == ProjectionVar::Avg_type)
										{
											cout << "[ERROR] Invalid type for SUM or AVG." << endl;
											continue;
										}
										else if ((proj[i].aggregate_type == ProjectionVar::Min_type
											|| proj[i].aggregate_type == ProjectionVar::Max_type)
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
									if (datetime && numeric && proj[i].aggregate_type != ProjectionVar::Count_type)
									{
										succ = false;
										break;
									}

									if (proj[i].aggregate_type == ProjectionVar::Sum_type
										|| proj[i].aggregate_type == ProjectionVar::Avg_type)
									{
										numeric_sum = numeric_sum + tmp;
										cout << "numeric_sum.term_value = " << numeric_sum.term_value << endl;
									}
									else if (proj[i].aggregate_type == ProjectionVar::Groupconcat_type)
									{
										size_t bIdx = tmp.str_value.find('\"'), eIdx = tmp.str_value.rfind('\"');
										if (groupconcat.empty())
											groupconcat += "\"";
										else
											groupconcat += proj[i].separator;
										if (bIdx == string::npos || eIdx == string::npos || bIdx == eIdx)
											groupconcat += tmp.str_value;
										else
											groupconcat += tmp.str_value.substr(bIdx + 1, eIdx - bIdx - 1);
									}
									else if (proj[i].aggregate_type == ProjectionVar::Min_type)
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
									else if (proj[i].aggregate_type == ProjectionVar::Max_type)
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
									else if (proj[i].aggregate_type == ProjectionVar::Sample_type)
									{
										if (sample_result.empty())
											sample_result = tmp.str_value;
									}

									count++;
								}
								if (proj[i].aggregate_type == ProjectionVar::Groupconcat_type)
									groupconcat += "\"";
								else if (proj[i].aggregate_type == ProjectionVar::Avg_type)
								{
									tmp.term_value = "\"" + to_string(count) + "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
									tmp.deduceTypeValue();

									numeric_sum = numeric_sum / tmp;
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

						if (proj[i].aggregate_type == ProjectionVar::Count_type)
						{
							ss << "\"";
							ss << count;
							ss << "\"^^<http://www.w3.org/2001/XMLSchema#integer>";
						}
						else if (succ)
						{
							if (proj[i].aggregate_type == ProjectionVar::Sum_type
								|| proj[i].aggregate_type == ProjectionVar::Avg_type)
							{
								cout << "numeric_sum.term_value = " << numeric_sum.term_value << endl;
								ss << numeric_sum.term_value;
							}
							else if (proj[i].aggregate_type == ProjectionVar::Min_type)
							{
								if (numeric)
									ss << numeric_min.term_value;
								else if (datetime)
									ss << datetime_min.term_value;
							}
							else if (proj[i].aggregate_type == ProjectionVar::Max_type)
							{
								if (numeric)
									ss << numeric_max.term_value;
								else if (datetime)
									ss << datetime_max.term_value;
							}
							else if (proj[i].aggregate_type == ProjectionVar::Sample_type)
							{
								ss << sample_result;
							}
						}
						else 	// Failed
							ss << "";

						if (groupconcat.empty())
							ss >> new_result0.result.back().str[proj2new[i] - new_result0_id_cols];
						else
							new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = groupconcat;
					}
					else if (proj[i].aggregate_type == ProjectionVar::CompTree_type)
					{
						if (group2temp.empty()) {
							for (int j = begin; j <= end; j++) {
								new_result0.result[j].str[proj2new[i] - new_result0_id_cols] = \
									result0.doComp(proj[i].comp_tree_root, result0.result[j], result0_id_cols, kvstore, \
									result0Varset).term_value;
							}
						} else {
							new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = \
								result0.doComp(proj[i].comp_tree_root, result0.result[begin], result0_id_cols, kvstore, \
								result0Varset).term_value;
						}
					}
					else if (proj[i].aggregate_type == ProjectionVar::Contains_type)
					{
						if (proj[i].func_args.size() == 2)
						{
							string arg1 = proj[i].func_args[0], arg2 = proj[i].func_args[1];
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
										stringindex->randomAccess(result0.result[j].id[pos1], &big_str, string_index_buffer, string_index_buffer_size, isel1);
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
										stringindex->randomAccess(result0.result[j].id[pos2], &small_str, string_index_buffer, string_index_buffer_size, isel2);
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

								if (group2temp.empty()) {
									if (big_str.find(small_str) != string::npos)
										new_result0.result[j].str[proj2new[i] - new_result0_id_cols] = \
											"\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
									else
										new_result0.result[j].str[proj2new[i] - new_result0_id_cols] = \
											"\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
								} else {
									if (big_str.find(small_str) != string::npos)
										new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = \
											"\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
									else
										new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = \
											"\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
								}
							}
						}
					}
					else if (proj[i].aggregate_type == ProjectionVar::PFN_type)
					{
						prepPathQuery();
						vector<int> uid_ls, vid_ls;
						vector<int> pred_id_set;
						if (proj[i].path_args.iri_set.size() > 2)
						{
							throw runtime_error("iri_set conatins variable params but total params more than two!");
							return;
						}
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
						{
							uid_ls.push_back(kvstore->getIDByString(proj[i].path_args.src));
						}	

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
						{
							vid_ls.push_back(kvstore->getIDByString(proj[i].path_args.dst));
						}

						// pred_id_set: convert from IRI to integer ID
						if (!proj[i].path_args.pred_set.empty())
						{
							for (auto pred : proj[i].path_args.pred_set)
							{
								TYPE_PREDICATE_ID pred_id = kvstore->getIDByPredicate(pred);
								// cout << "pred_set id:" << pred_id << endl;
								if (pred_id != static_cast<int>(INVALID))
									pred_id_set.push_back(pred_id);
							}
						}
						else
						{
							// Allow all predicates except those in neg_pred_set
							unordered_set<int> neg_pred_id_set;
							for (string pred : proj[i].path_args.neg_pred_set)
								neg_pred_id_set.emplace(kvstore->getIDByPredicate(pred));
							unsigned pre_num = stringindex->getNum(StringIndexFile::Predicate);
							for (unsigned j = 0; j < pre_num; j++)
								if (neg_pred_id_set.find(j) == neg_pred_id_set.end())
									pred_id_set.push_back(j);
						}

						// For each u-v pair, query
						unordered_set<pair<int, int>, hashFunction> unique_pairs;
						bool exist = 0, earlyBreak = 0;	// Boolean queries can break early with true
						stringstream ss;
						bool notFirstOutput = 0;	// For outputting commas
						ss << "\"[";
						cout<<"proj["<<i<<"].aggregate_type :"<<proj[i].aggregate_type <<endl;
						for (int uid : uid_ls)
						{
							for (int vid : vid_ls)
							{
								auto uvpair = make_pair(uid, vid);
								if (unique_pairs.find(uvpair) != unique_pairs.end())
									continue;
								unique_pairs.insert(uvpair);
								prepPathQuery();
								int hopConstraint = proj[i].path_args.k;
								bool directed = proj[i].path_args.directed;
								string fun_name = proj[i].path_args.fun_name;
								fun_name = Util::replace_all(fun_name, "\"", "");
								#if defined(DEBUG)
								cout<<">>>>>>>>>>>>> PFN-2 args <<<<<<<<<<<<<<<\n" 
									<< "uid=" << uid 
									<< "\nvid=" << vid 
									<< "\ndirected=" << directed
									<< "\nk=" << hopConstraint
									<< "\npred_id_set=";
								for (size_t i = 0; i < pred_id_set.size(); i++)
								{
									if (i > 0)
										cout << ", ";
									cout << pred_id_set[i];
								}
								cout << "\nfun_name=" << fun_name 
									<<">>>>>>>>>>>>> PFN-2 args <<<<<<<<<<<<<<<\n"
									<< endl;
								#endif
								vector<int> iri_id_set;
								iri_id_set.push_back(uid);
								iri_id_set.push_back(vid);
								std::map<std::string, std::string> rt = dynamicFunction(iri_id_set, directed, hopConstraint, pred_id_set, fun_name, ret_result.getUsername());
								string rt_type = rt.find("return_type")->second;
								string str = rt.find("return_value")->second;
								if (rt_type == "path")
								{
									vector<string> path_str;
									vector<int> path_int;
									Util::split(str, ",", path_str);
									size_t len = path_str.size();
									for (size_t i = 0; i < len; i++)
									{
										path_int.push_back(Util::string2int(path_str[i]));
									}
									if (path_int.size() != 0)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										pathVec2JSON(uid, vid, path_int, ss);
									}
								}
								else if (rt_type == "kvalue") 
								{
									// str is JSONArray sting: [{vid1:value1},{vid2:value2}...]
									rapidjson::Document doc;
									doc.IsArray();
									doc.Parse(str.c_str());
									string iri_src = kvstore->getStringByID(iri_id_set[0]);
									ss << "[";
									for (size_t i = 0; i < doc.Size(); i++)
									{
										if (i > 0) 
										{
											ss << ",";
										}
										rapidjson::Value obj = doc[i].GetObject();
										string dst_str = obj.HasMember("dst") ? obj["dst"].GetString(): "unkown";
										string value_str = obj.HasMember("value") ? obj["value"].GetString() : "";
										string iri_dst = "";
										if (Util::is_number(dst_str))
										{
											iri_dst = kvstore->getStringByID(Util::string2int(dst_str));
										} 
										else
										{
											iri_dst = dst_str;
										}
										ss << "{\"src\":\"" <<iri_src << "\",\"dst\":\""<< iri_dst << "\",\"value\":\""
											<< value_str << "\"}";
									}
									ss << "]";
								}
								else
								{
									if (!str.empty())
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										ss << "\"" << str << "\"";
									}
								}
							}
							if (earlyBreak)
								break;
						}
						ss << "]\"";
						ss >> new_result0.result.back().str[proj2new[0] - new_result0_id_cols];
					}
					else	// Path query
					{
						prepPathQuery();

						vector<int> uid_ls, vid_ls;
						vector<vector<int>> uid_ls_ls;
						vector<int> pred_id_set;

						// uid_ls_ls (multiple input vertices)
						auto vars2temp = Varset(proj[i].path_args.vert_set).mapTo(result0.getAllVarset());
						for (int j = begin; j <= end; j++)
						{
							bool row_valid = true;
							for (size_t k = 0; k < vars2temp.size(); k++)
								if ((proj[i].path_args.vert_set[k][0] != '?' && vars2temp[k] == -1) \
									|| vars2temp[k] >= result0_id_cols || result0.result[j].id[vars2temp[k]] == INVALID)
								{
									row_valid = false;
									break;
								}
							if (!row_valid) break;

							uid_ls_ls.push_back(vector<int>());

							for (size_t k = 0; k < vars2temp.size(); k++)
							{
								if (vars2temp[k] == -1)
									uid_ls_ls.back().push_back(kvstore->getIDByString(proj[i].path_args.vert_set[k]));
								else
									uid_ls_ls.back().push_back(result0.result[j].id[vars2temp[k]]);
							}
						}
						
						// uid
						if (!proj[i].path_args.src.empty())
						{
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
						}
						else
							uid_ls.push_back(-1);	// Dummy for loop

						// vid
						if (!proj[i].path_args.dst.empty())
						{
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
						}
						else
							vid_ls.push_back(-1);	// Dummy for loop

						// pred_id_set: convert from IRI to integer ID
						if (!proj[i].path_args.pred_set.empty())
						{
							for (auto pred : proj[i].path_args.pred_set)
							{
								TYPE_PREDICATE_ID pred_id = kvstore->getIDByPredicate(pred);
								if (pred_id != static_cast<int>(INVALID))
									pred_id_set.push_back(pred_id);
							}
						}
						else
						{
							// Allow all predicates except those in neg_pred_set
							unordered_set<int> neg_pred_id_set;
							for (string pred : proj[i].path_args.neg_pred_set)
								neg_pred_id_set.emplace(kvstore->getIDByPredicate(pred));
							unsigned pre_num = stringindex->getNum(StringIndexFile::Predicate);
							for (unsigned j = 0; j < pre_num; j++)
								if (neg_pred_id_set.find(j) == neg_pred_id_set.end())
									pred_id_set.push_back(j);
						}

						// For each u-v pair, query
						unordered_set<pair<int, int>, hashFunction> unique_pairs;
						bool exist = 0, earlyBreak = 0;	// Boolean queries can break early with true
						stringstream ss;
						bool notFirstOutput = 0;	// For outputting commas
						bool doneOnceOp = 0;	// functions that only need to do once (triangleCounting, pr, labelProp, wcc, clusteringCoeff without source)
						ss << "\"{\"paths\":[";
						if (proj[i].aggregate_type == ProjectionVar::maximumKplex_type)
						{
							auto kplex = pqHandler->maximumKplex(pred_id_set, proj[0].path_args.k);
							bool localFirstOutput = true;
							if (notFirstOutput)
								ss << ',';
							else
								notFirstOutput = true;
							ss << "{\"kplex\":[";
							for (int vid : kplex)
							{
								if (localFirstOutput)
									localFirstOutput = false;
								else
									ss << ',';
								ss << kvstore->getStringByID(vid);
							}
							ss << "]}";
						} else if (proj[i].aggregate_type == ProjectionVar::coreTruss_type) {
							auto clique = pqHandler->coreTruss(pred_id_set, proj[i].path_args.misc[i], proj[i].path_args.misc[1]);
							ss << "{\"coreTruss\":[";
							bool localFirstOutput = true;
							for (int vid : clique) {
								if (localFirstOutput)
									localFirstOutput = false;
								else
									ss << ',';
								ss << kvstore->getStringByID(vid);
							}
							ss << "]}";
						}
						for (int uid : uid_ls)
						{
							for (int vid : vid_ls)
							{
								auto uvpair = make_pair(uid, vid);
								if (unique_pairs.find(uvpair) != unique_pairs.end())
									continue;
								unique_pairs.insert(uvpair);
								if (proj[i].aggregate_type == ProjectionVar::cyclePath_type)
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
								else if (proj[i].aggregate_type == ProjectionVar::cycleBoolean_type)
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
								else if (proj[i].aggregate_type == ProjectionVar::shortestPath_type)
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
									vector<int> path = pqHandler->shortestPath(uid, vid, proj[i].path_args.directed, pred_id_set);
									if (path.size() != 0)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										pathVec2JSON(uid, vid, path, ss);
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::shortestPathLen_type)
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
									vector<int> path = pqHandler->shortestPath(uid, vid, proj[i].path_args.directed, pred_id_set);
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
								else if (proj[i].aggregate_type == ProjectionVar::kHopReachable_type)
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
									int hopConstraint = proj[i].path_args.k;
									if (hopConstraint < 0)
										hopConstraint = 999;
									bool reachRes = pqHandler->kHopReachable(uid, vid, proj[i].path_args.directed, hopConstraint, pred_id_set);
									ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"dst\":\"" \
										<< kvstore->getStringByID(vid) << "\",\"value\":";
									cout << "src = " << kvstore->getStringByID(uid) << ", dst = " << kvstore->getStringByID(vid) << endl;
									if (reachRes)
										ss << "\"true\"}";
									else
										ss << "\"false\"}";
								}
								else if (proj[i].aggregate_type == ProjectionVar::kHopReachablePath_type)
								{
									if (uid == vid)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										vector<int> path; // Empty path
										pathVec2JSON(uid, vid, path, ss);
										continue;
									}
									int hopConstraint = proj[i].path_args.k;
									if (hopConstraint < 0)
										hopConstraint = 999;
									vector<int> path = pqHandler->kHopReachablePath(uid, vid, proj[i].path_args.directed, hopConstraint, pred_id_set);
									if (path.size() != 0)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										pathVec2JSON(uid, vid, path, ss);
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::kHopEnumerate_type)
								{
									if (uid == vid)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										vector<int> path; // Empty path
										pathVec2JSON(uid, vid, path, ss);
										continue;
									}
									int hopConstraint = proj[i].path_args.k;
									if (hopConstraint < 0)
										hopConstraint = 999;
									vector<vector<int>> paths = pqHandler->kHopEnumeratePath(uid, vid, proj[i].path_args.retNum, proj[i].path_args.directed, hopConstraint, pred_id_set);
									if (!paths.empty())
									{
										for (auto path : paths)
										{
											if (notFirstOutput)
												ss << ",";
											else
												notFirstOutput = 1;
											pathVec2JSON(uid, vid, path, ss);
										}
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::ppr_type)
								{
									vector< pair<int ,double> > v2ppr;
									pqHandler->SSPPR(uid, proj[i].path_args.retNum, proj[i].path_args.k, pred_id_set, v2ppr);
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"results\":[";
									for (auto it = v2ppr.begin(); it != v2ppr.end(); ++it)
									{
										if (it != v2ppr.begin())
											ss << ",";
										ss << "{\"dst\":\"" << kvstore->getStringByID(it->first) << "\",\"PPR\":" \
											<< it->second << "}";
									}
									ss << "]}";
								}
								else if (proj[i].aggregate_type == ProjectionVar::triangleCounting_type)
								{
									if (!doneOnceOp)
									{
										auto ret = pqHandler->triangleCounting(proj[i].path_args.directed, pred_id_set);
										ss << to_string(ret);
										doneOnceOp = true;
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::closenessCentrality_type)
								{
									auto ret = pqHandler->closenessCentrality(uid, proj[i].path_args.directed, pred_id_set);
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"result\":";
									ss << to_string(ret) << "}";
								}
								else if (proj[i].aggregate_type == ProjectionVar::bfsCount_type)
								{
									auto ret = pqHandler->bfsCount(uid, proj[i].path_args.directed, pred_id_set);
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									size_t retSz = ret.size();
									ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"results\":[";
									for (size_t j = 0; j < retSz; j++)
									{
										ss << "{\"depth\":" << j << ", \"count\":" << ret[j] << "}";
										if (j != retSz - 1)
											ss << ", ";
									}
									ss << "]}";
								}
								else if (proj[i].aggregate_type == ProjectionVar::pr_type)
								{
									if (!doneOnceOp)
									{
										vector<pair<int, double>> idx2val;
										pqHandler->PR(proj[i].path_args.directed, pred_id_set, proj[i].path_args.retNum, proj[i].path_args.misc[0], \
											proj[i].path_args.misc[1], proj[i].path_args.misc[2], idx2val);
										bool localFirstOutput = true;
										// for (auto mp : ret)
										for (size_t i = 0; i < idx2val.size(); i++)
										{
											if (localFirstOutput)
												localFirstOutput = false;
											else
												ss << ",";
											ss << "{\"src\":\"" << kvstore->getStringByID(idx2val[i].first) << "\",\"result\":" << idx2val[i].second << "}";
										}
										doneOnceOp = true;
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::sssp_type)
								{
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									auto paths = pqHandler->SSSP(uid, proj[i].path_args.directed, pred_id_set);
									bool localFirstOutput = true;
									for (auto mp : paths)
									{
										if (mp.second.size() != 0)
										{
											if (localFirstOutput)
												localFirstOutput = false;
											else
												ss << ",";
											pathVec2JSON(uid, mp.second.back(), mp.second, ss);
										}
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::sssplen_type)
								{
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									auto paths = pqHandler->SSSPLen(uid, proj[i].path_args.directed, pred_id_set);
									bool localFirstOutput = true;
									for (auto mp : paths)
									{
										if (localFirstOutput)
											localFirstOutput = false;
										else
											ss << ",";
										
										ss << "{\"src\":\"" << kvstore->getStringByID(uid)
										<< "\",\"dst\":\"" << kvstore->getStringByID(mp.first) \
										<< "\",\"length\":" << mp.second << "}";
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::labelProp_type)
								{
									if (!doneOnceOp)
									{
										auto ret = pqHandler->labelProp(proj[i].path_args.directed, pred_id_set, proj[i].path_args.misc[0]);
										bool localFirstOutput = true;
										for (auto mp : ret)
										{
											if (localFirstOutput)
												localFirstOutput = false;
											else
												ss << ",";
											vertVec2JSON(mp, ss);
										}
										doneOnceOp = true;
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::wcc_type)
								{
									if (!doneOnceOp)
									{
										auto ret = pqHandler->WCC(pred_id_set);
										bool localFirstOutput = true;
										for (auto mp : ret)
										{
											if (localFirstOutput)
												localFirstOutput = false;
											else
												ss << ",";
											vertVec2JSON(mp, ss);
										}
										doneOnceOp = true;
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::clusterCoeff_type)
								{
									if (uid == -1)	// Source not given, compute once globally
									{
										if (!doneOnceOp)
										{
											auto ret = pqHandler->clusteringCoeff(proj[i].path_args.directed, pred_id_set);
											ss << to_string(ret);
											doneOnceOp = true;
										}
									}
									else
									{
										auto ret = pqHandler->clusteringCoeff(uid, proj[i].path_args.directed, pred_id_set);
										ss << "{\"src\":\"" << kvstore->getStringByID(uid) << "\",\"result\":";
										ss << to_string(ret) << "}";
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::kHopCount_type)
								{
									int hopConstraint = proj[i].path_args.k;
									auto ret = pqHandler->kHopCount(uid, proj[i].path_args.directed, hopConstraint, pred_id_set);
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									ss << "{\"src\":\"" << kvstore->getStringByID(uid) 
										<< "\",\"depth\":" << hopConstraint 
										<< ", \"count\":" << ret << "}";
								}
								else if (proj[i].aggregate_type == ProjectionVar::kHopNeighbor_type)
								{
									int hopConstraint = proj[i].path_args.k;
									int retNum = proj[i].path_args.retNum;
									if (retNum < 0) {
										retNum = 100;
									}
									auto ret = pqHandler->kHopNeighbor(uid, proj[i].path_args.directed, hopConstraint, pred_id_set, retNum);
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									ss << "{\"src\":\"" << kvstore->getStringByID(uid) 
										<< "\",\"depth\":" << hopConstraint 
										<< ", \"neighbor\":["; 
									bool hasMore = false;
									for (int nid : ret)
									{
										if (hasMore)
											ss << ","; 
										else 
											hasMore = true;
										ss << "\"" + kvstore->getStringByID(nid) + "\"";
									}
									ss << "]}";
								}
								else if (proj[i].aggregate_type == ProjectionVar::shortestPathCount_type)
								{
									auto ret = pqHandler->shortestPathCount(uid, vid, proj[i].path_args.directed, pred_id_set);
									if (notFirstOutput)
										ss << ",";
									else
										notFirstOutput = 1;
									ss << "{\"src\":\"" << kvstore->getStringByID(uid) 
										<<"\", \"dst\":\"" << kvstore->getStringByID(vid)
										<<"\", \"count\":" << ret << "}";
								} else if (proj[0].aggregate_type == ProjectionVar::IC14_type) {
									int knowsPred = kvstore->getIDByPredicate("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/directKnows>");
									int hasCreatorPred = kvstore->getIDByPredicate("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/hasCreator>");
									int typePred = kvstore->getIDByPredicate("<http://www.w3.org/1999/02/22-rdf-syntax-ns#type>");
									int replyPred = kvstore->getIDByPredicate("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/replyOf>");
									int idPred = kvstore->getIDByPredicate("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/id>");
									int postId = kvstore->getIDByString("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/Post>");
									int commentId = kvstore->getIDByString("<http://www.ldbc.eu/ldbc_socialnet/1.0/vocabulary/Comment>");
									auto ret = pqHandler->IC14(uid, vid, knowsPred, hasCreatorPred, typePred, replyPred, postId, commentId);
									size_t pathSz = ret[0].first.size();
									unsigned* objidlist = nullptr;
									unsigned list_len = 0;
									bool invalid = false;
									vector<int> pathVert;
									for (const auto &path2W : ret)
									{
										if (notFirstOutput)
											ss << ",";
										else
											notFirstOutput = 1;
										invalid = false;
										pathVert.clear();
										for (size_t i = 0; i < pathSz; i += 2) {
											kvstore->getobjIDlistBysubIDpreID(path2W.first[i], idPred, objidlist, list_len);
											if (list_len != 1) {
												invalid = true;
												break;
											}
											pathVert.emplace_back(objidlist[0]);
											delete []objidlist;
										}
										if (invalid)
											continue;
										ss << "{\"path\":[";
										string tmp;
										size_t lPos = 0, rPos = 0;
										for (size_t i = 0; i < pathVert.size(); i++) {
											tmp = kvstore->getStringByID(pathVert[i]);
											lPos = tmp.find('\"');
											rPos = tmp.find('\"', lPos + 1);
											if (i > 0)
												ss << ",";
											ss << tmp.substr(lPos + 1, rPos - 1 - lPos);
										}
										ss << "],\"weight\":" << path2W.second << "}";
									}
								}
								else if (proj[i].aggregate_type == ProjectionVar::louvain_type)
								{
									if (!doneOnceOp)
									{
										pair<size_t, map<int,set<int> > > result;
										float min_modularity_increase = proj[i].path_args.misc[0];
										int phase1_loop_num = proj[i].path_args.misc[1];
										pqHandler->louvain(phase1_loop_num, min_modularity_increase, pred_id_set, proj[i].path_args.directed, result);
										ss << "{\"count\":" << result.first << ", \"details\":[";
										for (auto it=result.second.begin(); it != result.second.end(); it++ )
										{
											if (notFirstOutput)
												ss << ",";
											else
												notFirstOutput = 1;
											ss << "{\"communityId\":\""<<it->first <<"\", \"menberNum\":" << it->second.size() << "}";
										}
										ss << "]}";
										doneOnceOp = true;
									}
								}
							}
							if (earlyBreak)
								break;
						}
						ss << "]}\"";
						// cout << "max #paths:" << uid_ls.size() * vid_ls.size() << endl;
						if (proj[i].aggregate_type == ProjectionVar::cycleBoolean_type)
						{
							if (exist)
								new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = "\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
							else
								new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = "\"false\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
						}
						else 
						{
							new_result0.result.back().str[proj2new[i] - new_result0_id_cols] = ss.str();
						}
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

#ifdef STREAM_ON
		long long ret_result_size = (long long)ret_result.ansNum * (long long)ret_result.select_var_num * 100 / Util::GB;
		if (Util::memoryLeft() < ret_result_size || !this->query_tree.getOrderVarVector().empty())
		{
			ret_result.setUseStream();
			printf("set use Stream\n");
		}
#endif

		if (!(query_tree.getSingleBGP() && query_tree.getLimit() != -1 && ranked))
			ret_result.setOutputOffsetLimit(this->query_tree.getOffset(), this->query_tree.getLimit());
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
			if (!(query_tree.getSingleBGP() && query_tree.getLimit() != -1 && ranked))
			{
				// Else, ORDER BY will already have been processed at the BGP level
				for (int i = 0; i < (int)this->query_tree.getOrderVarVector().size(); i++)
				{
					// int var_id = Varset(this->query_tree.getOrderVarVector()[i].var).mapTo(ret_result_varset)[0];
					// Temporary, to be changed to allow for more than one var in one ORDER BY condition
					int var_id = this->query_tree.getOrderVarVector()[i].comp_tree_root.getVarset().mapTo(ret_result_varset)[0];
					if (var_id != -1)
					{
						keys.push_back(var_id);
						desc.push_back(this->query_tree.getOrderVarVector()[i].descending);
					}
				}
			}
			ret_result.openStream(keys, desc);
		}

		vector<int> proj2temp = ret_result_varset.mapTo(result0.getAllVarset());
		int id_cols = result0.id_varset.getVarsetSize();

		vector<bool> isel;	// Indicates whether the var is a subject/object (true) or predicate (false)
		for (int i = 0; i < result0.id_varset.getVarsetSize(); i++)
			isel.push_back(this->query_tree.getGroupPattern().group_pattern_subject_object_maximal_varset.findVar(result0.id_varset.vars[i]));

      if (!ret_result.checkUseStream())
      {
		std::vector<StringIndexFile::AccessRequest> string_index_request[3];
        for (unsigned i = 0; i < ret_result.ansNum; i++)
        {
          ret_result.answer[i] = new string [ret_result.select_var_num];

          for (int j = 0; j < ret_result.select_var_num; j++)
          {
            int k = proj2temp[j];
            if (k != -1)
            {
              if (k < id_cols)
              {
                unsigned ans_id = result0.result[i].id[k];
                if (ans_id != INVALID)
                {
                  this->stringindex->addRequest(string_index_request, ans_id, &ret_result.answer[i][j], isel[k]);
                }
              }
              else
              {
                ret_result.answer[i][j] = result0.result[i].str[k - id_cols];
              }
            }
          }
        }
        this->stringindex->trySequenceAccess(string_index_request, string_index_buffer, string_index_buffer_size);
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
								this->stringindex->randomAccess(ans_id, &ans_str, string_index_buffer, string_index_buffer_size, isel[k]);
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
				{
					if (this->temp_result->results[i].result[0].str.size() == 1 \
						&& this->temp_result->results[i].result[0].str[0] == "false")
						continue;
					ret_result.answer[0][0] = "\"true\"^^<http://www.w3.org/2001/XMLSchema#boolean>";
					break;
				}
		}
	}

	if (string_index_buffer != NULL) delete[] string_index_buffer;
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

void GeneralEvaluation::prepareUpdateTriple(GroupPattern &update_pattern, TripleWithObjType *&update_triple, TYPE_TRIPLE_NUM &update_triple_num)
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
		if (update_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Pattern_type)
		{
			for (int j = 0; j < (int)this->temp_result->results.size(); j++)
				if (update_pattern.sub_group_pattern[i].pattern.varset.belongTo(this->temp_result->results[j].getAllVarset()))
					update_triple_num += this->temp_result->results[j].result.size();
		}

	update_triple = new TripleWithObjType[update_triple_num];

	int update_triple_count = 0;
	
	char* string_index_buffer = NULL;
	unsigned string_index_buffer_size = 0;
	for (int i = 0; i < (int)update_pattern.sub_group_pattern.size(); i++)
		if (update_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Pattern_type)
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
								this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[subject_id], &subject, string_index_buffer, string_index_buffer_size, true);
							else
								subject = this->temp_result->results[j].result[k].str[subject_id - id_cols];
						}

						if (predicate_id != -1)
						{
							if (predicate_id < id_cols)
								this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[predicate_id], &predicate, string_index_buffer, string_index_buffer_size, false);
							else
								predicate = this->temp_result->results[j].result[k].str[predicate_id - id_cols];
						}

						if (object_id != -1)
						{
							if (object_id < id_cols)
								this->stringindex->randomAccess(this->temp_result->results[j].result[k].id[object_id], &object, string_index_buffer, string_index_buffer_size, true);
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
	if (string_index_buffer != NULL) delete[] string_index_buffer;
}

void
GeneralEvaluation::pathVec2JSON(int src, int dst, const vector<int> &v, stringstream &ss, double weight)
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
			if (v[i] >= 0)
				ss << "{\"fromNode\":" << v[i - 1] << ",\"toNode\":" << v[i + 1] \
					<< ",\"predIRI\":\"" << kvstore->getPredicateByID(v[i]) << "\"}";
			else	// Reverse edge
				ss << "{\"fromNode\":" << v[i + 1] << ",\"toNode\":" << v[i - 1] \
					<< ",\"predIRI\":\"" << kvstore->getPredicateByID(-v[i] - 1) << "\"}";
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
	ss << "]";
	if (weight != -1)
		ss << "\"weight\":" << weight;
	ss << "}";
}

void
GeneralEvaluation::vertVec2JSON(const std::vector<int> &v, std::stringstream &ss)
{
	ss << "[";
	int vLen = v.size();
	for (int i = 0; i < vLen; i++)
	{
		ss << '\"' << kvstore->getStringByID(v[i]) << '\"';
		if (i != vLen - 1)
			ss << ",";
	}
	ss << "]";
}

int GeneralEvaluation::constructTriplePattern(GroupPattern& triple_pattern, int dep)
{
	int group_pattern_triple_num = 0;
	// Extract all sub-group-patterns that are themselves triples, add into triple_pattern //
	for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
		if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == GroupPattern::SubGroupPattern::Pattern_type)
		{
			triple_pattern.addOnePattern(GroupPattern::Pattern(
				GroupPattern::Pattern::Element(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].pattern.subject.value),
				GroupPattern::Pattern::Element(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].pattern.predicate.value),
				GroupPattern::Pattern::Element(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].pattern.object.value),
				rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].pattern.kleene
			));
			group_pattern_triple_num++;
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
			GroupPattern &parrent_group_pattern = this->rewriting_evaluation_stack[j].group_pattern;

			for (int k = 0; k < (int)parrent_group_pattern.sub_group_pattern.size(); k++)
			{
				if (parrent_group_pattern.sub_group_pattern[k].type == GroupPattern::SubGroupPattern::Pattern_type)
					useful += parrent_group_pattern.sub_group_pattern[k].pattern.varset;
				else if (parrent_group_pattern.sub_group_pattern[k].type == GroupPattern::SubGroupPattern::Filter_type)
					useful += parrent_group_pattern.sub_group_pattern[k].filter.varset;
			}
		}
		// All vars from current levels' optionals and filters //
		for (int j = 0; j < (int)(rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern.size()); j++)
		{
			if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == GroupPattern::SubGroupPattern::Optional_type)
				useful += rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].optional.group_pattern_resultset_maximal_varset;
			else if (rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].type == GroupPattern::SubGroupPattern::Filter_type)
				useful += rewriting_evaluation_stack[dep].group_pattern.sub_group_pattern[j].filter.varset;
		}
	}
}

bool GeneralEvaluation::checkBasicQueryCache(vector<GroupPattern::Pattern>& basic_query, TempResultSet *sub_result, Varset& useful)
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

void GeneralEvaluation::fillCandList(vector<shared_ptr<BGPQuery>>& bgp_query_vec, int dep, vector<vector<string> >& encode_varset, TempResultSet *fill_result)
{
	TempResultSet *last_result = this->rewriting_evaluation_stack[(dep > 0 ? dep - 1 : 0)].result;
	if (dep == 0 && fill_result)
		last_result = fill_result;
	if (!last_result || last_result->results.empty() || (dep == 0 && last_result->results.size() > 1000000)) return;

	for (unsigned j = 0; j < bgp_query_vec.size(); j++)
	{
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
				} else {
					pos = Varset(basic_query_encode_varset[k]).mapTo(last_result->results[t].str_varset)[0];
					if (pos != -1) {
						for (int l = 0; l < (int)result.size(); l++)
							result_set.insert(kvstore->getIDByString(result[l].str[pos]));
					}
				}
			}

			/// If result_set is non-empty, set it as the current var's candidate list ///
			if (!result_set.empty())
			{
				// vector<unsigned> result_vector;
				IDList result_vector;
				result_vector.reserve(result_set.size());

				for (set<unsigned>::iterator iter = result_set.begin(); iter != result_set.end(); iter++)
					// result_vector.push_back(*iter);
					result_vector.addID(*iter);

				
				bgp_query_vec[j]->set_var_candidate_cache(bgp_query_vec[j]->get_var_id_by_name(basic_query_encode_varset[k]), make_shared<IDList>(result_vector));
				// bgp_query_vec[j]->get_candidate_list_by_id(bgp_query_vec[j]->get_var_id_by_name(basic_query_encode_varset[k]))->copy(result_vector);
				// basic_query.getCandidateList(basic_query.getIDByVarName(basic_query_encode_varset[k])).copy(result_vector);
				// basic_query.setReady(basic_query.getIDByVarName(basic_query_encode_varset[k]));

				printf("fill var %s CandidateList size %d\n", basic_query_encode_varset[k].c_str(), (int)result_vector.size());
			}
		}
	}
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


				basic_query.getCandidateList(basic_query.getIDByVarName(basic_query_encode_varset[k])).copy(result_vector);
				basic_query.setReady(basic_query.getIDByVarName(basic_query_encode_varset[k]));

				printf("fill var %s CandidateList size %d\n", basic_query_encode_varset[k].c_str(), (int)result_vector.size());
			}
		}
	}
}

void GeneralEvaluation::joinBasicQueryResult(SPARQLquery& sparql_query, TempResultSet *new_result, TempResultSet *sub_result, vector<vector<string> >& encode_varset, \
	vector<vector<GroupPattern::Pattern> >& basic_query_handle, long tv_begin, long tv_handle, int dep)
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
			temp->results[0].result.emplace_back();
			temp->results[0].result.back().id = v;
			temp->results[0].result.back().sz = varnum;
		}

		if (this->query_cache != NULL && dep == 0)
		{
			//if unconnected, time is incorrect
			// int time = tv_handle - tv_begin;

			long tv_bftry = Util::get_cur_time();
			// bool success = this->query_cache->tryCaching(basic_query_handle[j], temp->results[0], time);
			bool success = false;
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


std::map<std::string, std::string> GeneralEvaluation::dynamicFunction(const std::vector<int> &iri_set, bool directed, int k, const std::vector<int> &pred_set, const std::string& fun_name, const std::string& username)
{
	try
	{
		std::map<std::string, std::string> returnMap;
		// check funInfo from json file.
		string pfn_file_path = Util::getConfigureValue("pfn_file_path");
        pfn_file_path = Util::replace_all(pfn_file_path, "\"", "");
		if (pfn_file_path[pfn_file_path.length()-1] != '/')
		{
			pfn_file_path.append("/");
		}
        string pfn_lib_path = Util::getConfigureValue("pfn_lib_path");
        pfn_lib_path = Util::replace_all(pfn_lib_path, "\"", "");
		if (pfn_lib_path[pfn_lib_path.length()-1] != '/')
		{
			pfn_lib_path.append("/");
		}
		
		string json_file_path = pfn_file_path + username + "/data.json";
		#if defined(DEBUG)
		cout << "pfn lib path: " << pfn_lib_path << endl;
		cout << "open json file: " << json_file_path << endl;
		#endif // 
		ifstream in;
		in.open(json_file_path, ios::in);
		if (!in.is_open())
		{
			throw runtime_error("open function json file error.");
		}
		string line;
		bool isMatch;
		string temp_name, md5Str;
		string fun_args, fun_status, fun_return;
		isMatch = false;
		fun_return = "";
		while (getline(in, line))
		{
			rapidjson::Document doc;
			doc.SetObject();
			if (!doc.Parse(line.c_str()).HasParseError())
			{
				if (doc.HasMember("funName"))
				{
					temp_name = doc["funName"].GetString();
					if (temp_name == fun_name)
					{
						if (doc.HasMember("funStatus"))
							fun_status = doc["funStatus"].GetString();
						if (fun_status != "2")
							throw runtime_error("function " + fun_name + " not compile yet");
						if (doc.HasMember("funArgs"))
							fun_args = doc["funArgs"].GetString();
						if (doc.HasMember("funReturn"))
							fun_return = doc["funReturn"].GetString();
						if (doc.HasMember("lastTime"))
						{
							md5Str = doc["lastTime"].GetString();
							md5Str = Util::md5(md5Str);
						}
						isMatch = true;
						break;
					}
				}
			}
		}
		in.close();
		if (!isMatch)
		{
			throw runtime_error("function '" + fun_name + "' not exists");
		}
		// check funInfo end
		string error_msg;
		string fileName, soFile;
		fileName = fun_name;
		std::transform(fileName.begin(), fileName.end(), fileName.begin(), ::tolower);
		soFile = pfn_lib_path + username + "/lib" + fileName + md5Str + ".so";
		void *handle;
		string result;
		std::cout << "================================================\nopening " << soFile << endl;
		handle = dlopen(soFile.c_str(), RTLD_LAZY);
		if (!handle)
		{
			error_msg = "load so file error:" + string(dlerror());
			throw runtime_error(error_msg);
		}
		char *error;
		if (fun_args == "1")
		{
			#if defined(DEBUG)
			std::cout << "begin 1 for " << fun_name << endl;
			#endif 
			// int uid, int vid, bool directed, vector<int> pred_set, CSR * _csr
			typedef string (*personalized_fun)(vector<int>, bool, vector<int>, PathQueryHandler *);
			personalized_fun p_fun;
			p_fun = (personalized_fun)dlsym(handle, fun_name.c_str());
			if ((error = dlerror()) != NULL)
			{
				error_msg = "cannot load symbol '" + fun_name + "': " + string(error);
				dlclose(handle);
				throw runtime_error(error_msg);
			}
			// call function
			result = p_fun(iri_set, directed, pred_set, pqHandler.get());
			std::cout << "end with: " << result << endl;
			std::cout << "return type: " << fun_return << endl;
			std::cout<< "================================================\n";
			dlclose(handle);
			returnMap.insert(pair<std::string, std::string>("return_type", fun_return));
			returnMap.insert(pair<std::string, std::string>("return_value", result));
			return returnMap;
		}
		else if (fun_args == "2")
		{
			#if defined(DEBUG)
			std::cout << "begin 2 for " << fun_name << endl;
			#endif
			// int uid, int vid, bool directed, int k, vector<int> pred_set
			typedef string (*personalized_fun)(vector<int>, bool, int, vector<int>, PathQueryHandler *);
			personalized_fun p_fun;
			p_fun = (personalized_fun)dlsym(handle, fun_name.c_str());
			if ((error = dlerror()) != NULL)
			{
				error_msg = "cannot load symbol '" + fun_name + "': " + string(error);
				dlclose(handle);
				throw runtime_error(error_msg);
			}
			// call function
			result = p_fun(iri_set, directed, k, pred_set, pqHandler.get());
			std::cout << "end with: " << result << endl;
			std::cout << "return type: " << fun_return << endl;
			cout<< "================================================\n";
			dlclose(handle);
			returnMap.insert(pair<std::string, std::string>("return_type", fun_return));
			returnMap.insert(pair<std::string, std::string>("return_value", result));
			return returnMap;
		}
		else
		{
			throw runtime_error("unkown function argment type '" + fun_args + "'");
		}
	}
	catch (const std::exception &e)
    {
		string content = "run personalized function fail: " + string(e.what());
        std::cout << content << endl;
		throw runtime_error(content);
    }
	catch (...)
	{
		string content = "run personalized function fail: unknown error";
		std::cout << content << endl;
		throw runtime_error(content);
	}
}
void GeneralEvaluation::addAllTriples(const GroupPattern &group_pattern)
{
	for (size_t i = 0; i < group_pattern.sub_group_pattern.size(); i++)
	{
		if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Group_type)
			addAllTriples(group_pattern.sub_group_pattern[i].group_pattern);
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Union_type)
		{
			for (size_t j = 0; j < group_pattern.sub_group_pattern[i].unions.size(); j++)
				addAllTriples(group_pattern.sub_group_pattern[i].unions[j]);
		}
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Optional_type \
			|| group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Minus_type)
			addAllTriples(group_pattern.sub_group_pattern[i].optional);
		else if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Subquery_type) {
			const QueryTree &subQtr = group_pattern.sub_group_pattern[i].subquery;
			addAllTriples(subQtr.getGroupPattern());
		}
	}

	for (size_t i = 0; i < group_pattern.sub_group_pattern.size(); i++)
	{
		// Here not yet transformed into BGP_type
		if (group_pattern.sub_group_pattern[i].type == GroupPattern::SubGroupPattern::Pattern_type)
		{
			bgp_query_total->AddTriple(Triple(group_pattern.sub_group_pattern[i].pattern.subject.value, \
					group_pattern.sub_group_pattern[i].pattern.predicate.value, \
					group_pattern.sub_group_pattern[i].pattern.object.value));
		}
	}
}

// Input: tr; Output: temp
// Assume space has been alloc'ed for temp
// NOTE: ?s <p>* ?o has all the entities as result; <s> <p>* ?o has <s> as result.
// Therefore, ?s <p>* ?o should never be the first to execute (unconstrained materialization)
// Assume it is not the first in the following (does not handle the case where ?s <p>* ?o 
// is the only triple pattern in the WHERE clause)
void GeneralEvaluation::kleeneClosure(TempResultSet *temp, TempResult * const tr, \
	const string &subject, const string &predicate, const string &object, int dep)
{
	cout << "kleeneClosure, subject = " << subject << ", predicate = " << predicate << ", object = " << object << endl;
	// TempResult *tr = NULL;	// Use `tr` to store BFS starting vertices
	// <s> <p>* ?o
	TempResult *cand = NULL;
	if (subject[0] != '?')
	{
		// if (tr) delete tr;
		cand = new TempResult();
		cand->id_varset = Varset(vector<string>(1, subject));
		cand->result.emplace_back();
		cand->result.back().id = new unsigned[1];
		cand->result.back().id[0] = kvstore->getIDByString(subject);
		cand->result.back().sz = 1;
	}
	else if (object[0] != '?')
	{
		// if (tr) delete tr;
		cand = new TempResult();
		cand->id_varset = Varset(vector<string>(1, object));
		cand->result.emplace_back();
		cand->result.back().id = new unsigned[1];
		cand->result.back().id[0] = kvstore->getIDByString(object);
		cand->result.back().sz = 1;
	}
	else
	{
		if (tr) cand = tr;
		else if (dep > 0)
		{
			int curDep = dep - 1;
			while (curDep >= 0 && (!this->rewriting_evaluation_stack[curDep].result || \
				this->rewriting_evaluation_stack[curDep].result->results.empty() ||
				this->rewriting_evaluation_stack[curDep].result->results[0].result.size() == 0))
				curDep--;
			if (curDep >= 0)
				cand = &(this->rewriting_evaluation_stack[curDep].result->results[0]);
		}
	}
	
	temp->results.push_back(TempResult());
	if (subject[0] != '?')
	{
		temp->results[0].id_varset = Varset(vector<string>(1, object));
		for (size_t i = 0; i < cand->result.size(); i++)
		{
			unsigned tid = cand->result[i].id[0];
			BFS(temp, tid, kvstore->getIDByPredicate(predicate), true, 1);
			// vector<int> reach = pqHandler->BFS(tid, true,
			// 	vector<int>(1, kvstore->getIDByPredicate(predicate)), false);
			// for (int sid : reach)
			// {
			// 	temp->results[0].result.emplace_back();
			// 	temp->results[0].result.back().id = new unsigned[1];
			// 	temp->results[0].result.back().id[0] = sid;
			// 	temp->results[0].result.back().sz = 1;
			// }
		}
	}
	else if (object[0] != '?')
	{
		temp->results[0].id_varset = Varset(vector<string>(1, subject));
		for (size_t i = 0; i < cand->result.size(); i++)
		{
			unsigned sid = cand->result[i].id[0];
			BFS(temp, sid, kvstore->getIDByPredicate(predicate), false, 1);
			// vector<int> reach = pqHandler->BFS(sid, true,
			// 	vector<int>(1, kvstore->getIDByPredicate(predicate)), true);
			// for (int tid : reach)
			// {
			// 	temp->results[0].result.emplace_back();
			// 	temp->results[0].result.back().id = new unsigned[1];
			// 	temp->results[0].result.back().id[0] = tid;
			// 	temp->results[0].result.back().sz = 1;
			// }
		}
	}
	else
	{
		vector<string> vars;
		vars.push_back(subject);
		vars.push_back(object);
		temp->results[0].id_varset = Varset(vars);
		
		set<unsigned> sid_set, tid_set;
		if (!cand || cand->result.size() == 0)
		{
			// // cout << "[ERROR]	Cannot process ?s <p>* ?o as the only graph pattern in WHERE clause. (1)" << endl;
			// all entity -> sid_set/tid_set 
			for(TYPE_ENTITY_LITERAL_ID id = all_entity_id.init(); id!=INVALID_ENTITY_LITERAL_ID; id=all_entity_id.next())
				sid_set.insert(id);
			// tid_set = sid_set;
		}
		else{
			unsigned subjectIdx = 0, objectIdx = 0;
			bool subjectInId = true, objectInId = true, not_case2 = true;
			while (subjectIdx < cand->id_varset.vars.size() && cand->id_varset.vars[subjectIdx] != subject)
				subjectIdx++;
			while (objectIdx < cand->id_varset.vars.size() && cand->id_varset.vars[objectIdx] != object)
				objectIdx++;
			if (subjectIdx == cand->id_varset.vars.size() && objectIdx == cand->id_varset.vars.size())
			{
				subjectIdx = 0;
				objectIdx = 0;
				while (subjectIdx < cand->str_varset.vars.size() && cand->str_varset.vars[subjectIdx] != subject)
					subjectIdx++;
				while (objectIdx < cand->str_varset.vars.size() && cand->str_varset.vars[objectIdx] != object)
					objectIdx++;
				if (subjectIdx < cand->str_varset.vars.size())
					subjectInId = false;
				else if (objectIdx < cand->str_varset.vars.size())
					objectInId = false;
				else{
					not_case2 = false;
					// // cout << "[ERROR]	Cannot process ?s <p>* ?o as the only graph pattern in WHERE clause. (2)" << endl;
					// all entity -> sid_set/tid_set 
					for(TYPE_ENTITY_LITERAL_ID id = all_entity_id.init(); id!=INVALID_ENTITY_LITERAL_ID; id=all_entity_id.next())
						sid_set.insert(id);
					// tid_set = sid_set; 
				}
			}
			if(not_case2){
				if (subjectInId && subjectIdx < cand->id_varset.vars.size())
				{
					for (size_t i = 0; i < cand->result.size(); i++)
						sid_set.insert(cand->result[i].id[subjectIdx]);
				}
				else if (subjectIdx < cand->str_varset.vars.size())
				{
					for (size_t i = 0; i < cand->result.size(); i++)
						sid_set.insert(kvstore->getIDByString(cand->result[i].str[subjectIdx]));
				}
				if (objectInId && objectIdx < cand->id_varset.vars.size())
				{
					for (size_t i = 0; i < cand->result.size(); i++)
						tid_set.insert(cand->result[i].id[objectIdx]);
				}
				else if (objectIdx < cand->str_varset.vars.size())
				{
					for (size_t i = 0; i < cand->result.size(); i++)
						tid_set.insert(kvstore->getIDByString(cand->result[i].str[objectIdx]));
				}
			}
		}
		if (sid_set.size() > 0 && (sid_set.size() <= tid_set.size() || tid_set.size() == 0))
		{
			for (auto sid : sid_set)
				BFS(temp, sid, kvstore->getIDByPredicate(predicate), true, 2);
			// for (size_t i = 0; i < tr->result.size(); i++)
			// {
			// 	unsigned sid = tr->result[i].id[subjectIdx];
			// 	// cout << "sid = " << sid << endl;
			// 	vector<int> reach = pqHandler->BFS(sid, true,
			// 		vector<int>(1, kvstore->getIDByPredicate(predicate)), true);
			// 	for (int tid : reach)
			// 	{
			// 		temp->results[0].result.emplace_back();
			// 		temp->results[0].result.back().id = new unsigned[2];
			// 		temp->results[0].result.back().id[0] = sid;
			// 		temp->results[0].result.back().id[1] = tid;
			// 		temp->results[0].result.back().sz = 2;
			// 	}
			// }
		}
		else if (tid_set.size() > 0 && (tid_set.size() <= sid_set.size() || sid_set.size() == 0))
		{
			for (auto tid : tid_set)
				BFS(temp, tid, kvstore->getIDByPredicate(predicate), false, 2);
			// {
			// 	vector<int> reach = pqHandler->BFS(tid, true,
			// 		vector<int>(1, kvstore->getIDByPredicate(predicate)), false);
			// 	for (int sid : reach)
			// 	{
			// 		temp->results[0].result.emplace_back();
			// 		temp->results[0].result.back().id = new unsigned[2];
			// 		temp->results[0].result.back().id[0] = sid;
			// 		temp->results[0].result.back().id[1] = tid;
			// 		temp->results[0].result.back().sz = 2;
			// 	}
			// }
		}
		else
			cout << "[ERROR]	Cannot process ?s <p>* ?o as the only graph pattern in WHERE clause. (3)" << endl;
	}
}

void GeneralEvaluation::BFS(TempResultSet *temp, int sid, int pred, bool forward, int numCol)
{
	queue<unsigned> ret;
	unordered_set<int> ret_set;
	ret.push(sid);
	ret_set.insert(sid);
	unsigned curr;
	temp->results[0].result.emplace_back();
	if (numCol == 1)
	{
		temp->results[0].result.back().id = new unsigned[1];
		temp->results[0].result.back().id[0] = sid;
		temp->results[0].result.back().sz = 1;
	}
	else if (numCol == 2)
	{
		temp->results[0].result.back().id = new unsigned[2];
		temp->results[0].result.back().id[0] = sid;
		temp->results[0].result.back().id[1] = sid;
		temp->results[0].result.back().sz = 2;
	}
	while (!ret.empty())
	{
		curr = ret.front();
		ret.pop();
		if (forward)
		{
			unsigned* outList = NULL;
			unsigned outSz = 0, outNei;
			kvstore->getobjIDlistBysubIDpreID(curr, pred, outList, outSz); 
			for(unsigned i = 0; i < outSz; i++)
			{
				if(outList[i] >= Util::LITERAL_FIRST_ID)
					continue;
				outNei = outList[i];
				// cout << "outNei = " << outNei << endl;
				if (ret_set.find(outNei) == ret_set.end())
				{
					ret.push(outNei);
					ret_set.insert(outNei);
					temp->results[0].result.emplace_back();
					if (numCol == 1)
					{
						temp->results[0].result.back().id = new unsigned[1];
						temp->results[0].result.back().id[0] = outNei;
						temp->results[0].result.back().sz = 1;
					}
					else if (numCol == 2)
					{
						temp->results[0].result.back().id = new unsigned[2];
						temp->results[0].result.back().id[0] = sid;
						temp->results[0].result.back().id[1] = outNei;
						temp->results[0].result.back().sz = 2;
					}
				}
			}
		}
		else
		{
			unsigned *inList = NULL;
			unsigned inSz = 0, inNei;
			kvstore->getsubIDlistByobjIDpreID(curr, pred, inList, inSz);
			for(unsigned i = 0; i < inSz; i++)
			{
				if (inList[i] >= Util::LITERAL_FIRST_ID)
					continue;
				inNei = inList[i];
				if (ret_set.find(inNei) == ret_set.end())
				{
					ret.push(inNei);
					ret_set.insert(inNei);
					temp->results[0].result.emplace_back();
					temp->results[0].result.back().sz = 2;
					if (numCol == 1)
					{
						temp->results[0].result.back().id = new unsigned[1];
						temp->results[0].result.back().id[0] = inNei;
						temp->results[0].result.back().sz = 1;
					}
					else if (numCol == 2)
					{
						temp->results[0].result.back().id = new unsigned[2];
						temp->results[0].result.back().id[0] = inNei;
						temp->results[0].result.back().id[1] = sid;
						temp->results[0].result.back().sz = 2;
					}
				}
			}
		}
	}
}

void GeneralEvaluation::copyBgpResult2TempResult(std::shared_ptr<BGPQuery> bgp_query, int varnum, TempResult &tr)
{
	auto &bgpquery_result = *(bgp_query->get_result_list_pointer1());
	int bgpquery_result_num = (int)bgpquery_result.size();

    Varset new_id_varset;
    vector<bool> global_EntityPredicate(varnum, false);
    vector<bool> local_Entity(varnum, true);
    auto &qVar2Type = this->query_tree.getVar2Type();
    for (int i = 0; i < varnum; i++){
        string &tr_id_var = tr.id_varset.vars[i];
        if(qVar2Type.find(tr_id_var) != qVar2Type.end() && qVar2Type[tr_id_var] == QueryTree::EntityPredicate){
            global_EntityPredicate[i]=true;
            tr.str_varset.addVar(tr_id_var);

            int var_id = bgp_query->get_var_id_by_name(tr_id_var);
            const shared_ptr<VarDescriptor> &var_descrip = bgp_query->get_vardescrip_by_id(var_id);
            if(var_descrip->var_type_ == VarDescriptor::VarType::Predicate)
                local_Entity[i]= false;
        }
        else
            new_id_varset.addVar(tr_id_var);
    }
    tr.id_varset=new_id_varset;
    int new_id_cols=tr.id_varset.getVarsetSize();

	tr.result.reserve(bgpquery_result_num);
    if(new_id_cols == varnum){
        for (int k = 0; k < bgpquery_result_num; k++){
            tr.result.emplace_back();
            tr.result.back().id = new unsigned[new_id_cols];
            tr.result.back().sz = new_id_cols;
            std::move(bgpquery_result[k].begin(), bgpquery_result[k].end(), tr.result.back().id);
        }
    }
    else{
		std::vector<StringIndexFile::AccessRequest> string_index_request[3];
        for (int k = 0; k < bgpquery_result_num; k++)
        {
            tr.result.emplace_back();
            tr.result.back().id = new unsigned[new_id_cols];
            tr.result.back().sz = new_id_cols;
            tr.result.back().str.reserve(varnum-new_id_cols);

            int new_id_pos=0;
            for(int i=0; i < varnum; i++){
                if(global_EntityPredicate[i]){
                    tr.result.back().str.emplace_back();
                    this->stringindex->addRequest(string_index_request, bgpquery_result[k][i], &tr.result.back().str.back(), local_Entity[i]);
                }
                else
                    tr.result.back().id[new_id_pos++]=bgpquery_result[k][i];
            }
        }
		char* string_index_buffer = NULL;
		unsigned string_index_buffer_size = 0;
        this->stringindex->trySequenceAccess(string_index_request, string_index_buffer, string_index_buffer_size);
		if (string_index_buffer != NULL) delete[] string_index_buffer;
    }
}
