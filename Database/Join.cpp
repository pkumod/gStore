/*=============================================================================
# Filename: Join.cpp
# Author: Bookug Lobert
# Mail: 1181955272@qq.com
# Last Modified: 2015-12-13 16:44
# Description: implement functions in Join.h
=============================================================================*/

#include "Join.h"
#define DEBUG_JOIN
using namespace std;

Join::Join()
{
	this->kvstore = NULL;
	this->result_list = NULL;
	this->txn = nullptr;
}

Join::Join(KVstore* _kvstore, TYPE_TRIPLE_NUM* _pre2num, TYPE_PREDICATE_ID _limitID_predicate, TYPE_ENTITY_LITERAL_ID _limitID_literal,
	TYPE_ENTITY_LITERAL_ID _limitID_entity, shared_ptr<Transaction> _txn)
{
	this->kvstore = _kvstore;
	this->result_list = NULL;
	this->pre2num = _pre2num;
	this->limitID_predicate = _limitID_predicate;
	this->limitID_literal = _limitID_literal;
	this->limitID_entity = _limitID_entity;
	this->txn = _txn;
}

Join::~Join()
{
	//noting to do necessarily
}

void
Join::init(BasicQuery* _basic_query, bool * d_triple)
{
	//BETTER:only common are placed here!
	this->basic_query = _basic_query;
	this->var_num = this->basic_query->getVarNum();

	int mapping_len = this->basic_query->getPreVarNum() + this->var_num;
	this->id2pos = (int*)malloc(sizeof(int) * mapping_len);
	memset(id2pos, -1, sizeof(int) * mapping_len);
	//this->id2pos = (int*)malloc(sizeof(int) * this->var_num);
	//memset(id2pos, -1, sizeof(int) * this->var_num);
	this->pos2id = (int*)malloc(sizeof(int) * mapping_len);
	memset(pos2id, -1, sizeof(int) * mapping_len);
	//this->pos2id = (int*)malloc(sizeof(int) * this->var_num);
	//memset(pos2id, -1, sizeof(int) * this->var_num);
	this->id_pos = 0;

	this->start_id = -1;
	int triple_num = this->basic_query->getTripleNum();
	//calloc set all to false intially
	this->dealed_triple = d_triple;
	this->result_list = _basic_query->getResultListPointer();
}

void
Join::clear()
{
	//BETTER:only common are released here!
	free(this->id2pos);
	free(this->pos2id);
	//NOTICE:maybe many BasicQuery
	this->current_table.clear();
	while (this->mystack.empty() == false) this->mystack.pop();
	free(this->dealed_triple);
	//NULL if using multi-join method
	this->result_list = NULL;
	this->satellites.clear();
}

//the return value should be non-negative
/*
node的权重定义为周围边的过滤性和自己的过滤性：
常数/候选集大小；常数权重自己为1000000，周围边为10000（考虑一下分母非0）。
*/
double
Join::score_node(int var)
{
	unsigned degree = this->basic_query->getVarDegree(var);
	unsigned size = this->basic_query->getCandidateSize(var);
	//NOTICE:needn't add degree here, consider in join edge efficience
	double wt = 1;
	for(unsigned i = 0; i < degree; ++i)
	{
		//NOTICE:here we only consider edges which have power to prune
		//each triple/edge need to be processed only once.
		int edge_id = this->basic_query->getEdgeID(var, i);
		//NOTICE: check edge pre id is a better way! (var - -1)
		//Triple triple = this->basic_query->getTriple(edge_id);
		//bool is_pre_var = false;
		//if(triple.predicate[0] == '?')
		//{
			//is_pre_var = true;
		//}

		if(this->dealed_triple[edge_id])
		{
			continue;
		}

		//only consider undealed core join edges
		int id2 = this->basic_query->getEdgeNeighborID(var, i);
		if(id2 < 0 || id2 >= this->var_num || this->basic_query->isSatelliteInJoin(id2))
		{
			continue;
		}
		//CHECK:if the pre id is valid (0<=p<limit_predicateID)
		TYPE_PREDICATE_ID pid = this->basic_query->getEdgePreID(var, i);
		//DEBUG: if TYPE_PREDICATE_ID is changed to unsigned
		if(pid < 0 || pid >= this->limitID_predicate)
		{
			continue;
		}
		wt += Join::PARAM_PRE / (double)(this->pre2num[pid]+1);
	}
	//double wt = Join::PARAM_DEGREE * (double)degree + Join::PARAM_SIZE / (double)size + Join::PARAM_PRE / (double)num;
	//we should deal with literal variable as lately as possible
	//if(!this->is_literal_var(var))
	//{
		//no need to consider size for literal variable, because it may arise a lot
		wt += Join::PARAM_SIZE / ((double)size+1);
	//}

	//the smallest wt returned is 0
	return wt;
}

/*
函数已弃用，一定会走multi-join的分支；
*/
int
Join::judge(unsigned _smallest, unsigned _biggest)
{
	return 0; //DEBUG:remove when index_join is ok
			  //BETTER?:use appropiate method according to size and structure
	int edge_num = this->basic_query->getTripleNum();
	double dense = (double)edge_num / this->var_num;
	//BETTER:how to guess the size of can_lists
	double size = (_smallest + _biggest) / 2.0;
	double ans = Join::PARAM_DENSE * dense - size / Join::PARAM_SIZE;
	double limit = 1.0 / (double)Join::JUDGE_LIMIT;
	if (ans > limit)
		return 0;	//multi_join method
	else
		return 1;	//index_join method
}

/*
选择权重最大的点作为join的初始点
*/
//select the start point for multi-join
void
Join::select()
{
	//NOTICE: only consider vars in select here
	double max = -1;
	int maxi = -1;
	//int border = this->basic_query->getVarNum();
	for(int i = 0; i < this->var_num; ++i)
	{
		//satellites which are not retrieved
		if(!this->basic_query->isReady(i))
		{
			continue;
		}

		double tmp = this->score_node(i);
		if (tmp > max)
		{
			max = tmp;
			maxi = i;
		}
	}
	if (maxi == -1)
	{
		cout << "error to select the first one to join" << endl;
	}
	else
	{
		this->start_id = maxi;
	}
#ifdef DEBUG_JOIN
	//printf("the start id is: %d\n", this->start_id);
	cout << "the start id is: " << this->start_id << endl;
#endif
}

//join on the vector of CandidateList, available after
//retrieved from the VSTREE and store the resut in _result_set
bool
Join::join_sparql(SPARQLquery& _sparql_query)
{
	int basic_query_num = _sparql_query.getBasicQueryNum();
	//join each basic query
	for (int i = 0; i < basic_query_num; i++)
	{
		//fprintf(stderr, "Basic query %d\n", i);
		cout << "Basic query " << i << endl;
		bool * d_triple = (bool*)calloc(this->basic_query->getTripleNum(), sizeof(bool));
		bool ret = this->join_basic(&(_sparql_query.getBasicQuery(i)), d_triple);
		if (!ret)
			cout << "end directly for this basic query: " << i << endl;
	}

	return true;
}

//TODO: consider a node with multiple same predicates(not pre var), use p2s(...false) to do this
//BETTER?: ?s-p-?o, use p2so instead of p2s and p2o to get candidates for ?s and ?o will be better??
//TODO: deal with predicate variables, maybe not ready like literals
/*
原join策略的核心流程，BFS的部分在this->join();
*/
bool
Join::join_basic(BasicQuery* _basic_query, bool* d_triple)
{
	this->init(_basic_query,d_triple);
	long begin = Util::get_cur_time();
	//bool ret1 = this->filter_before_join();
	//long after_constant_filter = Util::get_cur_time();
	////fprintf(stderr, "after filter_before_join: used %ld ms\n", after_filter - begin);
	//cout << "after filter_before_join: used " << (after_constant_filter - begin) << " ms" << endl;
	//if (!ret1)
	//{
		//this->clear();
		//return false;
	//}

	//this->add_literal_candidate();
	//long after_add_literal = Util::get_cur_time();
	//cout << "after add_literal_candidate: used " << (after_add_literal - after_constant_filter) << " ms" << endl;

	//bool ret2 = this->allFilterByPres();
	////bool ret2 = true;
	//long after_pre_filter = Util::get_cur_time();
	//cout << "after allFilterByPres: used " << (after_pre_filter - after_add_literal) << " ms" << endl;
	/*
	bool ret2 = pre_handler();
	long after_prehandler = Util::get_cur_time();
	cout << "after prehandler: used " << (after_prehandler - begin) << " ms" << endl;
	
	if (!ret2)
	{
		this->clear();
		return false;
	}
	*/
	bool ret3 = this->join();
	long after_joinbasic = Util::get_cur_time();
	cout << "during join_basic: used " << (after_joinbasic - begin) << " ms" << endl;
	if (!ret3)
	{
		this->clear();
		return false;
	}
/*
原方案的后续步骤，需要将卫星结点进行join，这部分基本可以删除；
所有结点都在上面join里面做完，因此不需要判断是否为卫星结点。
*/
	bool ret4 = this->only_pre_filter_after_join();
	long after_only_pre_filter = Util::get_cur_time();
	cout << "during only pre filter: used " << (after_only_pre_filter - after_joinbasic) << " ms" << endl;
	if (!ret4)
	{
		this->clear();
		return false;
	}

	//BETTER+QUERY: consider satellite with pre var, which first?
	//I think s2p first is better but s2o is also ok
	//1. filter by predicate vars first, so num decreases, need to specify the case that a var is not retrieved
	//generate later use sp2o or op2s (for each pre var, sevearl candidates)
	//2. generate candidates for satellites first using sp2o or s2o(op2s or o2s), later filtered by pre vars
	//the generating process had better been placed at the final, just before copying result
	this->pre_var_handler();
	//BETTER:maybe also reduce to empty, return false
	long after_pre_var = Util::get_cur_time();
	cout << "during pre var: used " << (after_pre_var - after_only_pre_filter) << " ms" << endl;

	this->copyToResult();
	long after_copy = Util::get_cur_time();
	cout << "during copy to result list: used " << (after_copy - after_pre_var) << " ms" << endl;

	cout << "Final result size: " << this->basic_query->getResultList().size() << endl;
	this->clear();
	return true;
}

//BETTER:now we compute the answer of nodes first, and later we consider the pre vars
//However, sometimes this may be costly, i.e.  ?s ?p1 ?o. ?s ?p2 ?o.
//notice that pre num are small while object candidates num can be large, so maybe better to compute pre var's candidates first
//
//NOTICE: the method to deal with pre vars here is all right, but maybe cause the duplicates
//num not right because here not all triples are produced and count
//However, generally, we donot care about duplicates num, and if you do care, please rewrite 
//your sparql query to select all vars in your query(later you can get what you need by yourself)
//BETTER:here duplicates num are not all. and also not 0
//maybe we should provide a key word like distinct to control, if remove duplicates or keep all(select all first and project)
bool
Join::pre_var_handler()
{
	//int core_var_num = this->basic_query->getRetrievedVarNum();
	unsigned pre_var_num = this->basic_query->getPreVarNum();
#ifdef DEBUG_JOIN
	cout << "pre var num: " << pre_var_num << endl;
#endif
	//QUERY+BETTER:filter by pre vars one by one or each record together?
	for (unsigned i = 0; i < pre_var_num; ++i)
	{
#ifdef DEBUG_JOIN
		cout << "current pre var id: " << i << endl;
#endif
		const PreVar& pre_var = this->basic_query->getPreVarByID(i);
		bool is_selected = pre_var.selected;

#ifdef DEBUG_JOIN
		cout << "current table size: " << this->current_table.size() << endl;
#endif

		//WARN:do not conflict with original var id
		//1 core var, id can be 1, then pos can be 1 + 0 = 1 for pre var!!! conflict!
		//int pos = core_var_num + i;
		//NOTICE:add all pre var's candidates to be used in copyToResult
		int pos = this->var_num + i;
		//if(is_selected)
		//{
		this->add_id_pos_mapping(pos);
		//}
		//cout<<"id 1 pos "<<this->id2pos[1]<<endl;

		bool if_new_start = false;
		//for each record, use s/o2p for each triple containing this pre var to filter
		for (TableIterator it = this->current_table.begin(); it != this->new_start;)
		{
			IDList valid_ans;
			//bool ok = true;
			unsigned triple_num = pre_var.triples.size();
#ifdef DEBUG_JOIN
			//cout<<"triple num for this var: "<<triple_num<<endl;
#endif
			for (unsigned j = 0; j < triple_num; ++j)
			{
				const Triple& triple = this->basic_query->getTriple(pre_var.triples[j]);
				string sub_name = triple.subject;
				string obj_name = triple.object;
#ifdef DEBUG_JOIN
				//cout << sub_name << endl << triple.predicate << endl << obj_name << endl;
#endif
				TYPE_ENTITY_LITERAL_ID sub_id, obj_id;
				sub_id = obj_id = INVALID_ENTITY_LITERAL_ID;
				int var1 = -1, var2 = -1;

				if (sub_name[0] != '?')
				{
					sub_id = this->kvstore->getIDByEntity(sub_name);
				}
				else
				{
					if (!(this->basic_query->isOneDegreeNotJoinVar(sub_name)))
						var1 = this->basic_query->getIDByVarName(sub_name);
					//satellite in join not retrieved
					if (var1 != -1 && this->basic_query->isSatelliteInJoin(var1))
						var1 = -1;
				}

				if (obj_name[0] != '?')
				{
					obj_id = this->kvstore->getIDByEntity(obj_name);
					//if (obj_id == -1)
					if (obj_id == INVALID_ENTITY_LITERAL_ID)
						obj_id = this->kvstore->getIDByLiteral(obj_name);
				}
				else
				{
					if (!(this->basic_query->isOneDegreeNotJoinVar(obj_name)))
						var2 = this->basic_query->getIDByVarName(obj_name);
					//satellite in join not retrieved
					if (var2 != -1 && this->basic_query->isSatelliteInJoin(var2))
						var2 = -1;
				}
#ifdef DEBUG_JOIN
				cout<<"var1: "<<var1<<"   var2: "<<var2<<endl;
				cout<<"subid: "<<sub_id<<"   objid: "<<obj_id<<endl;
#endif

				unsigned* id_list = NULL;
				unsigned id_list_len = 0;
				//two vars in query
				if (sub_id == INVALID_ENTITY_LITERAL_ID && obj_id == INVALID_ENTITY_LITERAL_ID)
				//if (sub_id == -1 && obj_id == -1)
				{
					if (var1 == -1 && var2 == -1)
					{
						//NOTICE:then this triple is unlinked with others, and it will be viewed as a single query
						//we already deal with this case in the Strategy module in time
					}
					else if (var1 == -1 && var2 != -1)
					{
						//NOTICE: special case here(remember that this one triple case has already been discussed in Strategy.cpp)
						//?s1 ?p1 ?o
						//?s2 ?p2 ?o
						//...
						//?sn ?pn ?o
						//
						//If either subject or object is a constant, then the neighbor's candidates can be added through s2o or o2s
						//in filter_before_join()  and add_literal_candidate() -- so possible literals are solved here
						//
						//Otherwise if predicate is a constant:
						//if join can not be started, then in toStartJoin() ?o's literals can be added by p2o
						//else, ?o's literals can be added in join_two by sp2o
						//
						//Otherwise, predicates are all variables, and this is a star graph:
						//if it connected with other parts(not included in this star), either ?si or ?o(as subject), then join can be started
						//else, this can be pasred as a single BGP, and we shall deal with this special star graph in toStartJoin()
						this->kvstore->getpreIDlistByobjID((*it)[this->id2pos[var2]], id_list, id_list_len, true, txn);
					}
					else if (var1 != -1 && var2 == -1)
					{
						this->kvstore->getpreIDlistBysubID((*it)[this->id2pos[var1]], id_list, id_list_len, true, txn);
					}
					else if (var1 != -1 && var2 != -1)
					{
						//if(this->is_literal_var(var2))
						//{
						//int* oid_list = NULL;
						//int oid_list_len = 0;
						//this->kvstore->getobjIDlistBysubID((*it)[this->id2pos[var1]], oid_list, oid_list_len);
						//this->kvstore->getpreIDlistBysubID((*it)[this->id2pos[var1]], id_list, id_list_len);
						//}
						//cout<<"sub str: "<<this->kvstore->getEntityByID((*it)[this->id2pos[var1]])<<endl;
						//cout<<"obj str: "<<this->kvstore->getEntityByID((*it)[this->id2pos[var2]])<<endl;
						//this->kvstore->getpreIDlistBysubIDobjID((*it)[this->id2pos[var1]], (*it)[this->id2pos[var2]], id_list, id_list_len);
						int sid = (*it)[this->id2pos[var1]], oid = (*it)[this->id2pos[var2]];
						this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len, true, txn);
						//NOTICE:no need to add literals here because they are added when join using s2o
					}
				}
				//two constants in query
				else if (sub_id != INVALID_ENTITY_LITERAL_ID && obj_id != INVALID_ENTITY_LITERAL_ID)
				{
					//just use so2p in query graph to find predicates
					//this->kvstore->getpreIDlistBysubIDobjID(sub_id, obj_id, id_list, id_list_len);
					TYPE_ENTITY_LITERAL_ID sid = sub_id, oid = obj_id;
					this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len, true, txn);
				}
				//sub is var while obj is constant
				else if (sub_id == INVALID_ENTITY_LITERAL_ID && obj_id != INVALID_ENTITY_LITERAL_ID)
				{
					if (var1 == -1)
					{
						this->kvstore->getpreIDlistByobjID(obj_id, id_list, id_list_len, true, txn);
					}
					else
					{
						this->kvstore->getpreIDlistBysubIDobjID((*it)[this->id2pos[var1]], obj_id, id_list, id_list_len, true, txn);
						TYPE_ENTITY_LITERAL_ID sid = (*it)[this->id2pos[var1]], oid = obj_id;
						this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len, true, txn);
					}
				}
				//sub is constant while obj is var
				else if (sub_id != INVALID_ENTITY_LITERAL_ID && obj_id == INVALID_ENTITY_LITERAL_ID)
				{
					if (var2 == -1)
					{
						this->kvstore->getpreIDlistBysubID(sub_id, id_list, id_list_len, true, txn);
					}
					else
					{
						//NOTICE:no need to add literals here because they are added in add_literal_candidate using s2o
						//this->kvstore->getpreIDlistBysubIDobjID(sub_id, (*it)[this->id2pos[var2]], id_list, id_list_len);
						TYPE_ENTITY_LITERAL_ID sid = sub_id, oid = (*it)[this->id2pos[var2]];
						this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len, true, txn);
					}
				}

				//cout<<"the idlist len "<<id_list_len<<endl;
				if (j == 0)
				{
					valid_ans.unionList(id_list, id_list_len);
				}
				else
				{
					valid_ans.intersectList(id_list, id_list_len);
				}
				delete[] id_list;
				if (valid_ans.size() == 0)
				{
#ifdef DEBUG_JOIN
					cout << "already empty!" << endl;
#endif
					//ok = false;
					break;
				}
				else
				{
#ifdef DEBUG_JOIN
					for(unsigned k = 0; k < valid_ans.size(); ++k)
						cout << this->kvstore->getPredicateByID(valid_ans[k])<<" ";
					cout<<endl;
#endif
				}
			}

			//add the candidates of this pre var if selected,
			//beyond graph_var_num if satellites are generated first;
			//beyond core_var_num if not
			//
			//NOTICE: we add all here(select/not) because they maybe needed by generating satellites
			//we need to copy only the selected ones in copyToResult
			unsigned size = valid_ans.size();

			//BETTER:only add pre vars which are selected or linked with satellite
			if (size > 0)
			{
				//if(!is_selected)
				//{
					//it++;
					//continue;
				//}
				it->push_back(valid_ans[0]);
				unsigned begin = 1;
				if (!if_new_start && size > 1)
				{
					this->add_new_to_results(it, valid_ans[1]);
					if_new_start = true;
					//iterator it scan loop, to set the border to avoid going to the newly added ones
					this->new_start = this->current_table.end();
					this->new_start--;
					begin = 2;
				}
				for (unsigned j = begin; j < size; ++j)
				{
					this->add_new_to_results(it, valid_ans[j]);
				}
				it++;
			}
			else
			{
				it = this->current_table.erase(it);
			}
		}

		this->new_start = this->current_table.end();
	}

	cout << "table size after pre_var " << this->current_table.size() << endl;
	return true;
}

//NOTICE:we must save the results of pre var, because we may need to use it to generate the results of satellites
//s2o and o2s to generate satellites maybe cause error, because some pre var may exist in several triples!!!
//NOTICE:in this way, all triple counts maybe right(except satellites which may also cause duplicates num not right)
// Detail: record_len = select_var_num + selected_pre_var_num;
// New Table columns:
//    select_var_num first

void
Join::copyToResult()
{
	//copy to result list, adjust the vars order
	this->result_list->clear();
	int select_var_num = this->basic_query->getSelectVarNum();
	int core_var_num = this->basic_query->getRetrievedVarNum();
	int pre_var_num = this->basic_query->getPreVarNum();

	int selected_pre_var_num = this->basic_query->getSelectedPreVarNum();
	//if (this->id_pos != core_var_num + selected_pre_var_num)
	if (this->id_pos != core_var_num + pre_var_num)
	{
		cout << "terrible error in copyToResult!" << endl;
		return;
	}

#ifdef DEBUG_JOIN
	cout << "core var num: " << core_var_num << " select var num: " << select_var_num << endl;
#endif
	this->record_len = select_var_num + selected_pre_var_num;
	this->record = new unsigned[this->record_len];

	for (TableIterator it = this->current_table.begin(); it != this->current_table.end(); ++it)
	{
		int i = 0;
		for (; i < core_var_num; ++i)
		{
			//This is because select var id is always smaller
			if (this->pos2id[i] < select_var_num)
			{
				int vpos = this->basic_query->getSelectedVarPosition(this->pos2id[i]);
				this->record[vpos] = (*it)[i];
			}
		}

#ifdef DEBUG_JOIN
		//cout<<"current id_pos: "<<this->id_pos<<endl;
#endif
		//below are for selected pre vars
		while (i < this->id_pos)
		{
			//only add selected ones
			int pre_var_id = this->pos2id[i] - this->var_num;
			int pvpos = this->basic_query->getSelectedPreVarPosition(pre_var_id);
			if(pvpos >= 0)
			{
				this->record[pvpos] = (*it)[i];
			}
			++i;
		}

		bool valid = true;
		//generate satellites when constructing records
		//NOTICE: satellites in join must be selected
		//core vertex maybe not in select
		for (i = 0; i < core_var_num; ++i)
		{
			int id = this->pos2id[i];
			unsigned ele = (*it)[i];
			int degree = this->basic_query->getVarDegree(id);
			for (int j = 0; j < degree; ++j)
			{
				int id2 = this->basic_query->getEdgeNeighborID(id, j);
				if (this->basic_query->isSatelliteInJoin(id2) == false)
					continue;
#ifdef DEBUG_JOIN
				//cout << "to generate "<<id2<<endl;
#endif
				unsigned* idlist = NULL;
				unsigned idlist_len = 0;
				int triple_id = this->basic_query->getEdgeID(id, j);
				Triple triple = this->basic_query->getTriple(triple_id);

				TYPE_PREDICATE_ID preid = this->basic_query->getEdgePreID(id, j);
				if (preid == -2)  //?p
				{
					string predicate = triple.predicate;
					int pre_var_id = this->basic_query->getPreVarID(predicate);
					//if(this->basic_query->isPreVarSelected(pre_var_id))
					//{
					preid = (*it)[this->id2pos[pre_var_id+this->var_num]];
					//}
				}
				else if (preid == -1)  //INVALID_PREDICATE_ID
				{
					//ERROR
				}

				char edge_type = this->basic_query->getEdgeType(id, j);
				if (edge_type == Util::EDGE_OUT)
				{
					//if(preid < 0)
					//{
						//this->kvstore->getobjIDlistBysubID(ele, idlist, idlist_len, true);
					//}
					//else
					//{
					this->kvstore->getobjIDlistBysubIDpreID(ele, preid, idlist, idlist_len, true, txn);
					//}
				}
				else
				{
					//if(preid < 0)
					//{
						//this->kvstore->getsubIDlistByobjID(ele, idlist, idlist_len, true);
					//}
					//else
					//{
					this->kvstore->getsubIDlistByobjIDpreID(ele, preid, idlist, idlist_len, true, txn);
					//}
				}

				if(idlist_len == 0)
				{
					valid = false;
					break;
				}
				this->satellites.push_back(Satellite(id2, idlist, idlist_len));
#ifdef DEBUG_JOIN
				//cout<<"push a new satellite in"<<endl;
#endif
			}
			if(!valid)
			{
				break;
			}
		}
#ifdef DEBUG_JOIN
		//cout<<"satellites all prepared!"<<endl;
#endif
		int size = satellites.size();
		if(valid)
		{
			this->cartesian(0, size);
		}
#ifdef DEBUG_JOIN
		//cout<<"after cartesian"<<endl;
#endif
		for (int k = 0; k < size; ++k)
		{
			delete[] this->satellites[k].idlist;
			//this->satellites[k].idlist = NULL;
		}
		//WARN:use this to avoid influence on the next loop
		this->satellites.clear();
#ifdef DEBUG_JOIN
		//cout<<"after clear the satellites"<<endl;
#endif
	}

	delete[] this->record;
#ifdef DEBUG_JOIN
	//cout<<"after delete the record"<<endl;
#endif
	this->record = NULL;
	this->record_len = 0;
}

void
Join::cartesian(int pos, int end)
{
	if (pos == end)
	{
		unsigned* new_record = new unsigned[this->record_len];
		memcpy(new_record, this->record, sizeof(unsigned) * this->record_len);
		this->result_list->push_back(new_record);
		return;
	}

	unsigned size = this->satellites[pos].idlist_len;
	int id = this->satellites[pos].id;
	int vpos = this->basic_query->getSelectedVarPosition(id);
	unsigned* list = this->satellites[pos].idlist;
	for (unsigned i = 0; i < size; ++i)
	{
		this->record[vpos] = list[i];
		this->cartesian(pos + 1, end);
	}
}

void
Join::toStartJoin()
{
	for (int i = 0; i < this->var_num; ++i)
	{
		if (this->basic_query->isReady(i))
		{
			return;
		}
	}

	cout << "toStartJoin(): need to prepare a ready node"<<endl;

	int maxi = -1;
	double max = 0;
	for (int i = 0; i < this->var_num; ++i)
	{
		if (!this->basic_query->isSatelliteInJoin(i))
		{
			double tmp = this->score_node(i);
			if (tmp >= max)
			{
				max = tmp;
				maxi = i;
			}
		}
	}

	//TODO: delete the code that generate the literal_candidate_list;
	//NOTICE:not add literal, so no constant neighbor, this must be free literal variable
	int var_id = maxi;
	int var_degree = this->basic_query->getVarDegree(var_id);
	//cout<<"var id: "<<var_id<<"   "<<"var degree: "<<var_degree<<endl;
	
	IDList &literal_candidate_list = this->basic_query->getCandidateList(var_id);
	bool flag = false;
	for (int j = 0; j < var_degree; ++j)
	{
		//int neighbor_id = this->basic_query->getEdgeNeighborID(var_id, j);
		TYPE_PREDICATE_ID predicate_id = this->basic_query->getEdgePreID(var_id, j);
		int triple_id = this->basic_query->getEdgeID(var_id, j);
		Triple triple = this->basic_query->getTriple(triple_id);
		string neighbor_name = triple.subject;
		IDList this_edge_literal_list;
		unsigned* object_list = NULL;
		unsigned object_list_len = 0;

		if (predicate_id >= 0)
		{
			flag = true;
			(this->kvstore)->getobjIDlistBypreID(predicate_id, object_list, object_list_len, true, txn);
		}
		else
		{
			continue;
		}
		//cout<<"predicate id: "<<predicate_id<<endl<<this->kvstore->getPredicateByID(predicate_id)<<endl;
		//for(int sb = 0; sb < object_list_len; ++sb)
		//{
			//cout<<object_list[sb]<<" ";
		//}
		//cout<<endl;
		this_edge_literal_list.unionList(object_list, object_list_len, true);
		delete[] object_list;
		cout<<"preid: "<<predicate_id<<" length: "<<object_list_len<<" literals: "<<this_edge_literal_list.size()<<endl;

		if (j == 0)
		{
			literal_candidate_list.unionList(this_edge_literal_list);
		}
		else
		{
			literal_candidate_list.intersectList(this_edge_literal_list);
		}
	}
	
	/*
	star graph query的特殊处理
	*/
	//a special case is a star graph, where all pres are vars
	if(!flag) 
	{
		cout<<"Special Case: star graph whose pres are all var"<<endl;
		//get all literals in this db
		for(TYPE_ENTITY_LITERAL_ID i = 0; i < this->limitID_entity; ++i)
		{
			TYPE_ENTITY_LITERAL_ID id = i;
			string literal = this->kvstore->getEntityByID(id);
			if(literal == "")
			{
				continue;
			}
			//BETTER:cache the whole literal id list to improve the query throughput
			literal_candidate_list.addID(id);
		}
		for(TYPE_ENTITY_LITERAL_ID i = 0; i < this->limitID_literal; ++i)
		{
			TYPE_ENTITY_LITERAL_ID id = i + Util::LITERAL_FIRST_ID;
			string literal = this->kvstore->getLiteralByID(id);
			if(literal == "")
			{
				continue;
			}
			//BETTER:cache the whole literal id list to improve the query throughput
			literal_candidate_list.addID(id);
		}
		IDList& origin_candidate_list = this->basic_query->getCandidateList(var_id);
		//int origin_candidate_list_len = origin_candidate_list.size();
		origin_candidate_list.unionList(literal_candidate_list, false);
	}

	//int after_add_literal_candidate_list_len = origin_candidate_list.size();
	this->basic_query->setReady(var_id);

	cout<<"the prepared var id: "<<var_id<<endl;
	//cout<<"add literals num: "<<literal_candidate_list.size()<<endl;
	//cout<<"current can size: "<<origin_candidate_list.size()<<endl;
}

/*
根据之前的预处理结果进行最大候选集和最小候选集的判断；
由于我们没有index join，因此这部分的逻辑就是走完this->toStartJoin()后直接到this->multi_join()；
*/
// use the appropriate method to join candidates
bool
Join::join()
{
	//in case of no start point, if all core vertices are literal vars
	this->toStartJoin();

	//the smallest candidate list size of the not-satellite vars
	int id = this->basic_query->getVarID_FirstProcessWhenJoin();
	unsigned smallest = 0;
	if(id >= 0)
	{
		smallest = this->basic_query->getCandidateSize(id);
	}
	else
	{
		cout<<"error in join() - id < 0"<<endl;
		return false;
	}
	//if(!this->is_literal_var(id) && smallest == 0)
	if( smallest == 0)
	{
		cout<<"join() - already empty"<<endl;
		return false;  //empty result
	}

	int id_max = this->basic_query->getVarID_MaxCandidateList();
	unsigned biggest = 0;
	if(id_max >= 0)
	{
		biggest = this->basic_query->getCandidateSize(id_max);
	}
	else
	{
		cout<<"error in join() - id < 0"<<endl;
		return false;
	}
	//if(!this->is_literal_var(id_max) && biggest == 0)
	if(biggest == 0)
	{
		cout<<"join() - already empty"<<endl;
		return false;  //empty result
	}

	int method = this->judge(smallest, biggest);
	bool ret = true;
	switch (method)
	{
	case 0:
		//printf("use multi-join here!\n");
		cout << "use multi-join here!" << endl;
		ret = this->multi_join();
		break;
	case 1:
		//printf("use index-join here!\n");
		cout << "use index-join here!" << endl;
		//ret = this->index_join();
		break;
	default:
		//printf("ERROR: no method found!\n");
		cout << "ERROR: no method found!" << endl;
		break;
	}

	return ret;
}

/*
BFS的过程中会进一步寻找权重更高的点加入到join；
*/
int
Join::choose_next_node(int id)
{
	//choose a child to search deeply
	int degree = this->basic_query->getVarDegree(id);
	int maxi = -1;
	double max = 0;
	for (int i = 0; i < degree; ++i)
	{
		int var_id2 = this->basic_query->getEdgeNeighborID(id, i);
		if(var_id2 == -1)  //not in join, including constant
		{
			continue;
		}

		//satellites which are not retrieved
		if (this->basic_query->if_need_retrieve(var_id2) == false)
		{
			continue;
		}

		// each triple/edge need to be processed only once.
		int edge_id = this->basic_query->getEdgeID(id, i);
		if (this->dealed_triple[edge_id])
		{
#ifdef DEBUG_JOIN
			cout<<"this triple already dealed: "<<edge_id<<endl;
#endif
			continue;
		}

		//NTC:not using updated degrees, other not the whole loop
		double tmp = this->score_node(var_id2);
		//cout<<"score: "<<tmp<<endl;
		if (max < tmp)
		{
			max = tmp;
			maxi = i;
		}
	}
	return maxi;
}

bool
Join::is_literal_var(int _id)
{
	//if(!this->basic_query->isFreeLiteralVariable(_id) || this->basic_query->isAddedLiteralCandidate(_id))
	//if(!this->basic_query->isFreeLiteralVariable(_id))
	//{
	//return false;
	//}
	//BETTER?:this is not needed because we ensure that
	//all dealed nodes's literals are added!
	//this->basic_query->setAddedLiteralCandidate(_id);
	//if(this->basic_query->isAddedLiteralCandidate(_id))
	if (this->basic_query->isReady(_id))
		return false;
	else
		return true;
	//NOTICE:satellites are not considered in join, so only free literal variable checked here
	//(some free literal var maybe also added)
}

//===================================================================================================
//Below are functions to do multi-join method
//===================================================================================================

void
Join::add_new_to_results(TableIterator it, unsigned id)
{
	//NTC:already have one more in *it if need to push back
	RecordType tmp(*it);
	*(tmp.rbegin()) = id;
	this->current_table.push_back(tmp);
}

//after remove VSTREE, modify here
void
Join::update_answer_list(IDList*& valid_ans_list, IDList& _can_list, unsigned* id_list, unsigned id_list_len, bool _is_ready)
{
	if (valid_ans_list == NULL)
	{
		if(_is_ready)
			valid_ans_list = IDList::intersect(_can_list, id_list, id_list_len);
		else
		{
			valid_ans_list = new IDList();
			for(int i = 0; i < id_list_len; i++)
				valid_ans_list->addID(id_list[i]);
		}

	}
	else
	{
		valid_ans_list->intersectList(id_list, id_list_len);
	}
}

//TODO: multiple lists intersect, how about sort and intersect from small to big?
//but this need to generate all first, I think sort by pre2num if better!
//
//TODO: set the entity_literal border in kvstore, and intersect entity part and literal part respectively

//NOTICE: consider two directions according to table1 size and table2 size
//1. ->  add ID mapping record for the first linking column, whole(offset, size) zengli
//2. <-  join using inverted index for each column, offset and size for each column, hulin
//However, the result is that this case is rare, and not really better
//
//NOTICE: you may think that when joining to enlarge the current table, there maybe exist many duplicates in a column,
//which causes too many redunt linking operations.
//However, the case is really rare in our test(the reason may be that the web graph is always very sparse)
//If we add a buffer for this case, will cause worse performance
/*
join_two的代码应该对应我们的JoinANode部分;
上面的注释不太明白是什么意思。
*/
bool
Join::join_two(vector< vector<int> >& _edges, IDList& _can_list, unsigned _can_list_size, int _id, bool _is_ready)
{
	//if(_can_list_size == 0 && !_is_literal)
	if(_can_list_size == 0 && _is_ready)
	{
		return false;   //empty result
	}
	bool found = false;
	bool if_new_start = false; //the first to add to end in while
	for(TableIterator it0 = this->current_table.begin(); it0 != this->new_start;)
	{
#ifdef DEBUG_JOIN
		if (this->new_start != this->current_table.end())
		{
			//printf("now the new_start is:");
			cout << "now the new_start is:";
			for (RecordIterator it1 = this->new_start->begin(); it1 != this->new_start->end(); ++it1)
			{
				//printf(" %d", *it1);
				cout << " " << *it1;
			}
			//printf("\n");
			cout << endl;
		}
		else
			//printf("new_start still in end?!\n");
			cout << "new_start still in end?!" << endl;
		//printf("now the record is:");
		cout << "now the record is:";
		for (RecordIterator it1 = it0->begin(); it1 != it0->end(); ++it1)
		{
			//printf(" %d", *it1);
			cout << " " << *it1;
		}
		//printf("\n");
		cout << endl;
#endif

		int cnt = 0;
		//update the valid id num according to restrictions by multi vars
		//also ordered while id_list and can_list are ordered
		//IDList valid_ans_list;
		IDList* valid_ans_list = NULL;
		//list<int> valid_ans_list;
		bool matched = true;
		//NOTICE:we can generate cans from either direction, but this way is convenient and better
		for (RecordIterator it1 = it0->begin(); it1 != it0->end(); ++it1, ++cnt)
		{
#ifdef DEBUG_JOIN
			//printf("cnt is: %d\n", cnt);
			cout << "cnt is: " << cnt << endl;
#endif
			vector<int> edge_index = _edges[cnt];
			if (edge_index.size() == 0)
			{
				continue;
			}
#ifdef DEBUG_JOIN
			cout << "edge exists!" << endl;
#endif
			unsigned ele = *it1;
			bool exist_constant_pre = false;
			bool s2o_pre_var = false;
			bool o2s_pre_var = false;
			for(vector<int>::iterator it2 = edge_index.begin(); it2 != edge_index.end(); ++it2)
			{
				int edge_type = this->basic_query->getEdgeType(_id, *it2);
				TYPE_PREDICATE_ID pre_id = this->basic_query->getEdgePreID(_id, *it2);

				if (pre_id == -2)    //predicate var
				{
					if (edge_type == Util::EDGE_IN)
					{
						s2o_pre_var = true;
					}
					else
					{
						o2s_pre_var = true;
					}
#ifdef DEBUG_JOIN
					cout << "this is a predicate var!" << endl;
#endif
					continue;
				}
				else if(pre_id >= 0)
				{
					if(!exist_constant_pre)
					{
						exist_constant_pre = true;
					}
				}
				else   //-1
				{
					cout << "invalid pre found in join_two!!!" << endl;
					matched = false;
					break;
				}

				unsigned* id_list;
				unsigned id_list_len;
				if (edge_type == Util::EDGE_IN)
				{
#ifdef DEBUG_JOIN
					cout << "this is an edge to our id to join!" << endl;
#endif
					this->kvstore->getobjIDlistBysubIDpreID(ele, pre_id, id_list, id_list_len, true, txn);
				}
				else
				{
#ifdef DEBUG_JOIN
					cout << "this is an edge from our id to join!" << endl;
#endif
					this->kvstore->getsubIDlistByobjIDpreID(ele, pre_id, id_list, id_list_len, true, txn);
				}
				if (id_list_len == 0)
				{
					//id_list == NULL in this case, no need to free
					matched = false;
#ifdef DEBUG_JOIN
					cout << "this id_list is empty!" << endl;
#endif
					break;
				}

				//NOTICE:using so2p to filter is not good
				//The cost to join two ordered lists is the basic operation
				//of the whole join process!(O(klogn) < O(k+n) gennerally, for k < n)
				//Notice that n is the candidate list size just retrieved from vstree

				//only can occur the first time, means cnt == 0
				//if(valid_ans_list.size() == 0)
				update_answer_list(valid_ans_list, _can_list, id_list, id_list_len, _is_ready);
				delete[] id_list;
				if (valid_ans_list->size() == 0)
				{
					matched = false;
					break;
				}
			}

			if(!matched)
			{
				break;
			}
			if(exist_constant_pre)
			{
				//NOTICE: this means there exists constant pre in parallel edges, so update_answer_list has already been used
				//in this case, later we needn't do s2o_pre_var or o2s_pre_var because sp2o and op2s is more precise
				continue;
			}
			//all pres are variable, so use s2o or o2s to add
			if(s2o_pre_var)
			{
				unsigned* id_list2;
				unsigned id_list2_len;
				this->kvstore->getobjIDlistBysubID(ele, id_list2, id_list2_len, true, txn);
				update_answer_list(valid_ans_list, _can_list, id_list2, id_list2_len, _is_ready);
				delete[] id_list2;
				if (valid_ans_list->size() == 0)
				{
					matched = false;
					break;
				}
			}
			if(o2s_pre_var)
			{
				unsigned* id_list2;
				unsigned id_list2_len;
				this->kvstore->getsubIDlistByobjID(ele, id_list2, id_list2_len, true, txn);
				update_answer_list(valid_ans_list, _can_list, id_list2, id_list2_len, _is_ready);
				delete[] id_list2;
				if (valid_ans_list->size() == 0)
				{
					matched = false;
					break;
				}
			}
		}

		if (matched)
		{
#ifdef DEBUG_JOIN
			cout << "this record is matched!!" << endl;
#endif
			found = true;
			unsigned size = valid_ans_list->size();

			it0->push_back((*valid_ans_list)[0]);
			unsigned begin = 1;
			if (!if_new_start && size > 1)
			{
				this->add_new_to_results(it0, (*valid_ans_list)[1]);
				if_new_start = true;
				//this->new_start = this->current_table.rbegin().base();
				this->new_start = this->current_table.end();
				this->new_start--; //-1 is not allowed
				begin = 2;
			}

			for (unsigned i = begin; i < size; ++i)
			{
				//WARN+NOTICE:this strategy may cause that duplicates are not together!
				this->add_new_to_results(it0, (*valid_ans_list)[i]);
			}
			it0++;
		}
		else
		{
			it0 = this->current_table.erase(it0);
#ifdef DEBUG_JOIN
			cout << "this record is not matched!" << endl;
#endif
		}
		delete valid_ans_list;
		valid_ans_list = NULL;
	}
	return found;
}

void
Join::add_id_pos_mapping(int _id)
{
	this->pos2id[this->id_pos] = _id;
	this->id2pos[_id] = this->id_pos;
	this->id_pos++;
}

void
Join::reset_id_pos_mapping()
{
	memset(this->id2pos, -1, sizeof(int) * this->var_num);
	memset(this->pos2id, -1, sizeof(int) * this->var_num);
	this->id_pos = 0;
}

//BETTER+QUERY:why this more costly in some query containing literal vars?
//should not filter for literal var and just generate when join?
//QUERY:is the allFilterBySatellites sometimes costly if candidate list is too large?
//in this case we can join first and filter by edge later

/*
BFS逻辑
*/
bool
Join::multi_join()
{
	this->select();

	//keep an increasing vector for temp results, not in id order
	//vals num generally < 10, so just enum them and check if conncted
	//finally, copy in order to result_list in BasicQuery
	TableIterator it0;
	list<int>::iterator it1;
	vector<int>::iterator it2;
	//list<bool>::iterator it3;

	//The best strategy is to ensure that for each record, all satellite edges exist
	//then after join all core vertices, generate candidates for each satellite
	//and these are just the final accurate answer
	//It's out of question better than generating candidates for satellites now
	//
	//NOTICE:this should be done just once, so use it before pushing candidates
	//pruning the original candidates first(satellites only concerned with itself)
	//this->filterBySatellites(this->start_id);

	IDList& start_table = this->basic_query->getCandidateList(this->start_id);
	unsigned start_size = this->basic_query->getCandidateSize(this->start_id);
#ifdef DEBUG_JOIN
	cout << "the start size " << start_size << endl;
#endif
	for (unsigned i = 0; i < start_size; ++i)
	{
		unsigned ele = start_table.getID(i);
		//NOTICE: we can denote the total size here in vector, but no need because the variables' num is small
		//(won't double to require more space)
		RecordType record(1, ele);
		this->current_table.push_back(record);
		//this->table_row_new.push_back(false);
	}
	this->add_id_pos_mapping(this->start_id);
	//cout<<"the mapping is id "<<this->start_id<<"   and pos "<<this->id2pos[this->start_id]<<endl;
	this->new_start = this->current_table.end();

	this->mystack.push(this->start_id);
#ifdef DEBUG_JOIN
	//fprintf(stderr, "now to start the stack loop\n");
	cout << "now to start the stack loop" << endl;
#endif
	while (!this->mystack.empty())
	{
		int id = this->mystack.top();

#ifdef DEBUG_JOIN
		//fprintf(stderr, "the current id: %d\n", id);
		cout << "the current id: " << id << endl;
#endif
		//int id = mystack[top];
		int maxi = this->choose_next_node(id);
		if (maxi == -1) //all edges of this node are dealed
		{
#ifdef DEBUG_JOIN
			cout << "the node is totally dealed: " << id << endl;
#endif
			//top--;
			this->mystack.pop();
			continue;
		}
		int id2 = this->basic_query->getEdgeNeighborID(id, maxi);
#ifdef DEBUG_JOIN
		cout << "the next node id to join: " << id2 << endl;
#endif
		//this->filterBySatellites(id2);
#ifdef DEBUG_JOIN
		cout << "the start size " << this->basic_query->getCandidateSize(id2) << endl;
#endif

		vector< vector<int> > edges; //the edge index for table column in id2
						   // the outer is node-loop, inner is canlist-loop
		vector< vector<int*> > id_lists;
		vector< vector<int> > id_lists_len;
		//int* tmp_id_list;
		//int tmp_id_list_len;
		IDList& can_list = this->basic_query->getCandidateList(id2);
		unsigned can_list_size = can_list.size();

		for (int i = 0; i < this->id_pos; ++i)
		{
			vector<int> edge_index = this->basic_query->getEdgeIndex(id2, this->pos2id[i]);
			edges.push_back(edge_index);
		}
		//NOTICE: there are several ways to join two tables
		//h is the cost to search kvstore, m is the returned list size
		//n is the normal can_list_size, k is the vars num to
		//consider now, r is the record num
		//0. expand and intersect with another table: not ok!
		//1. given two node to find if exist right pre:
		//O(1) space, O(rhknlogn) time,
		//2. bsearch in can_list: O(mk+n) space, O(rmkhlogn) time
		//3. bsearch in id_list: O(nkm) space, O(rnklogm+knh)
		//
		//most queries will contain many constants(entity/literal)
		//var's can_list with one constant neighbor will be small,
		//otherwise will be big compared with id_list
		//the can_list of var representing literals is not valid,
		//must use kvstore->get...() to join

		//NOTICE: not cancle the followings, to be used for later
		//TODO: if queries contain predicate variables, it may be hard to prepare candidates for a node
		//(so it is not ready, can also be represented by is_literal_var())
		/*
		bool is_literal = this->is_literal_var(id2);
		if(is_literal)
		{
#ifdef DEBUG_PRECISE
			cout << "this var may contain literals: " << id2 << endl;
#endif
			this->basic_query->setReady(id2);
		}
		else
		{
#ifdef DEBUG_PRECISE
			cout << "this var not contain literals: " << id2 << endl;
#endif
		}
		*/
		bool flag = false;
#ifdef DEBUG_PRECISE
			cout << "this edge uses not-prepared-join way" << endl;
#endif
			flag = this->join_two(edges, can_list, can_list_size, id2, this->basic_query->isReady(id2));

		//if current_table is empty, ends directly
		if (!flag)
		{
#ifdef DEBUG_JOIN
			cout << "the result is already empty!!" << endl;
#endif
			//break;
			return false; //to avoid later invalid copy
		}

		for (int i = 0; i < this->id_pos; ++i)
		{
			vector<int> edge_index = edges[i];
			for(vector<int>::iterator it = edge_index.begin(); it != edge_index.end(); ++it)
			{
				int edge_id = this->basic_query->getEdgeID(id2, *it);
				dealed_triple[edge_id] = true;
			}
		}

		this->new_start = this->current_table.end();
		this->add_id_pos_mapping(id2);
		this->mystack.push(id2);
	}
#ifdef DEBUG_JOIN
	cout << "now end the stack loop" << endl;
#endif

	//BETTER?:though the whole current_table is ordered here, the
	//selected columns are not definitely ordered, needing to be
	//sorted at the end. We can join based on the selected var's
	//candidate to ensure the order, but this may be complicated.
	//If we want to ensure the order here, new table is a must!
	//and the duplicates cannot be checked unless the last step!
	//The result list will not be too large generally, and the sort
	//is not in any loop.(but if the size is too large?)
	return true;
}



//===================================================================================================
//Below are functions before or after Join
//===================================================================================================

//sort the candidate lists and deal with all constant neigbors
/*
index join的函数，可以直接忽略；
*/
bool
Join::filter_before_join()
{
	//fprintf(stderr, "*****IIIIIIN filter_before_join\n");
	cout << "*****IN filter_before_join" << endl;

	for (int i = 0; i < this->var_num; i++)
	{
		bool flag = this->basic_query->isLiteralVariable(i);
		//fprintf(stderr, "\tVar%d %s\n", i, this->basic_query->getVarName(i).c_str());
		cout << "\tVar" << i << " " << this->basic_query->getVarName(i) << endl;
		IDList &can_list = this->basic_query->getCandidateList(i);
		//fprintf(stderr, "\t\tsize of canlist before filter: %d\n", can_list.size());
		cout << "\t\tsize of canlist before filter: " << can_list.size() << endl;

		//NOTICE:must sort before using binary search.
		//However, the sort-merge maybe not always better because the sort() will take too much time if
		//the can_list size is large, i.e. > 1000000
		can_list.sort();
		//vstree ? place on ID?
		//TODO: use BoolArray isntead of bitset
		//n is candidate num, m is sp2o num, then when n<m/(lg2(m)_lg2(n)), sort m and binary search in m
		//otherwise, use BoolArray for n, only construct a time
		//NOTICE: for parallelism, use a BoolArray for each BGP(either on join or in Strategy)

		long begin = Util::get_cur_time();
		bool ret = this->constant_edge_filter(i);
		long after_constant_edge_filter = Util::get_cur_time();
		//fprintf(stderr, "\t\tconstant_edge_filter: used %ld ms\n", after_constant_edge_filter - begin);
		cout << "\t\tconstant_edge_filter: used " << (after_constant_edge_filter - begin) << " ms" << endl;
		//		this->preid_filter(this->basic_query, i);
		//		long after_preid_filter = Util::get_cur_time();
		//cout << "\t\tafter_preid_filter: used " << (after_preid_filter-after_literal_edge_filter) << " ms" << endl;
		//fprintf(stderr, "\t\t[%d] after filter, candidate size = %d\n\n\n", i, can_list.size());
		cout << "\t\t[" << i << "] after filter, candidate size= " << can_list.size() << endl << endl << endl;

		//debug
		//		{
		//			stringstream _ss;
		//			for(int i = 0; i < can_list.size(); i ++)
		//			{
		//				string _can = this->kvstore->getEntityByID(can_list[i]);
		//				_ss << "[" << _can << ", " << can_list[i] << "]\t";
		//			}
		//			_ss << endl;
		//			Util::logging(_ss.str());
		//			cout << can_list.to_str() << endl;
		//		}
		if (!flag && !ret)   //already empty
		{
			return false;
		}
	}
	//fprintf(stderr, "OOOOOOUT filter_before_join\n");
	cout << "OUT filter_before_join" << endl;
	return true;
}

/*
index join的函数，可以直接忽略；
*/
//decrease the candidates of _var_i using its constant neighbors
bool
Join::constant_edge_filter(int _var_i)
{
	//Util::logging("IN literal_edge_filter"); //debug

	int var_degree = this->basic_query->getVarDegree(_var_i);
	IDList &_list = this->basic_query->getCandidateList(_var_i);
	for (int j = 0; j < var_degree; j++)
	{
		int neighbor_id = this->basic_query->getEdgeNeighborID(_var_i, j);
		//fprintf(stderr, "\t\t\tneighbor_id=%d\n", neighbor_id);
		cout << "\t\t\tneighbor_id=" << neighbor_id << endl;
		if (neighbor_id != -1)   //variables in join not considered here
		{
			continue;
		}

		char edge_type = this->basic_query->getEdgeType(_var_i, j);
		int triple_id = this->basic_query->getEdgeID(_var_i, j);
		Triple triple = this->basic_query->getTriple(triple_id);
		string neighbor_name;

		if (edge_type == Util::EDGE_OUT)
		{
			neighbor_name = triple.object;
		}
		else
		{
			neighbor_name = triple.subject;
		}

		//NOTICE: this is another case, vars not in join, we only need constants
		bool only_preid_filter = (this->basic_query->isOneDegreeNotJoinVar(neighbor_name));
		if(only_preid_filter)
		{
			continue;
		}
		else
		{
			this->dealed_triple[triple_id] = true;
		}

		TYPE_PREDICATE_ID pre_id = this->basic_query->getEdgePreID(_var_i, j);
		TYPE_ENTITY_LITERAL_ID lit_id = (this->kvstore)->getIDByEntity(neighbor_name);
		//if (lit_id == -1)
		if (lit_id == INVALID_ENTITY_LITERAL_ID)
		{
			lit_id = (this->kvstore)->getIDByLiteral(neighbor_name);
		}

		//			cout << "\t\tedge[" << j << "] "<< lit_string << " has id " << lit_id << "";
		//			cout << " preid:" << pre_id << " type:" << edge_type
		//					<< endl;
		//		{
		//					stringstream _ss;
		//					_ss << "\t\tedge[" << j << "] "<< lit_string << " has id " << lit_id << "";
		//					_ss << " preid:" << pre_id << " type:" << edge_type
		//							<< endl;
		//					Util::logging(_ss.str());
		//		}

		unsigned id_list_len = 0;
		unsigned* id_list = NULL;
		if (pre_id >= 0)
		{
			if (edge_type == Util::EDGE_OUT)
			{
				(this->kvstore)->getsubIDlistByobjIDpreID(lit_id, pre_id, id_list, id_list_len, true, txn);
			}
			else
			{
				(this->kvstore)->getobjIDlistBysubIDpreID(lit_id, pre_id, id_list, id_list_len, true, txn);
			}
		}
		else if (pre_id == -2)
		{
			if (edge_type == Util::EDGE_OUT)
			{
				(this->kvstore)->getsubIDlistByobjID(lit_id, id_list, id_list_len, true, txn);
			}
			else
			{
				(this->kvstore)->getobjIDlistBysubID(lit_id, id_list, id_list_len, true, txn);
			}
		}
		else
			// pre_id == -1 means we cannot find such predicate in rdf file, so the result set of this sparql should be empty.
			// note that we cannot support to query sparqls with predicate variables ?p.
		{
			id_list_len = 0;
			//			if (edge_type == Util::EDGE_OUT)
			//			{
			//			    (this->kvstore)->getsubIDlistByobjID(lit_id, id_list, id_list_len);
			//			}
			//			else
			//			{
			//			    (this->kvstore)->getobjIDlistBysubID(lit_id, id_list, id_list_len);
			//			}
		}

		//debug
		//      {
		//          stringstream _ss;
		//          _ss << "id_list: ";
		//          for (int i=0;i<id_list_len;i++)
		//          {
		//              _ss << "[" << id_list[i] << "]\t";
		//          }
		//          _ss<<endl;
		//          Util::logging(_ss.str());
		//      }

		if (id_list_len == 0)
		{
			_list.clear();
			delete[] id_list;
			return false;
		}
		//			cout << "\t\t can:" << can_list.to_str() << endl;
		//			cout << "\t\t idlist has :";
		//			for(int i_ = 0; i_ < id_list_len; i_ ++)
		//			{
		//				cout << "[" << id_list[i_] << "]\t";
		//			}
		//			cout << endl;

		_list.intersectList(id_list, id_list_len);
		delete[]id_list;
		if (_list.size() == 0)
		{
			return false;
		}
	}

	//Util::logging("OUT constant_edge_filter");
	return true;
}

/*
index join的函数，可以直接忽略；
*/
//BETTER?:merge with constant_edge_filter?
//this only consider subject constant neighbors, while the latter also
//consider constant object neighbors(literal), as well as entities
//neighbors.
//(only in objects, no constant neighbors are called free, dealed in join)
//
//add literal candidates to these variables' candidate list
//which may include literal results.
void
Join::add_literal_candidate()
{
	//Util::logging("IN add_literal_candidate");
	//
	// deal with literal variable candidate list.
	// because we do not insert any literal elements into VSTree, we can not retrieve them from VSTree.
	// for these variable which may include some literal results, we should add all possible literal candidates to the candidate list.
	for (int i = 0; i < this->var_num; i++)
	{
		if (this->basic_query->isReady(i))
		{
			continue;
		}

		if(this->basic_query->isSatelliteInJoin(i))
		{
			continue;
		}

		// for these literal variable without any linking entities(we call free literal variable),
		// we will add their literal candidates when join-step.
		if(this->basic_query->isFreeLiteralVariable(i))
		{
			continue;
		}

		int var_id = i;
		int var_degree = this->basic_query->getVarDegree(var_id);
		IDList literal_candidate_list;

		bool flag = false;
		// intersect each edge's literal candidate.
		for (int j = 0; j < var_degree; j++)
		{
			int neighbor_id = this->basic_query->getEdgeNeighborID(var_id, j);
			TYPE_PREDICATE_ID predicate_id = this->basic_query->getEdgePreID(var_id, j);
			int triple_id = this->basic_query->getEdgeID(var_id, j);
			Triple triple = this->basic_query->getTriple(triple_id);
			string neighbor_name = triple.subject;
			IDList this_edge_literal_list;

			// if the neighbor of this edge is an entity, we can add all literals which has an exact predicate edge linking to this entity.
			if (neighbor_id == -1)
			{
				TYPE_ENTITY_LITERAL_ID subject_id = (this->kvstore)->getIDByEntity(neighbor_name);
				unsigned* object_list = NULL;
				unsigned object_list_len = 0;

				if (predicate_id >= 0)
				{
					(this->kvstore)->getobjIDlistBysubIDpreID(subject_id, predicate_id, object_list, object_list_len, true, txn);
				}
				else if (predicate_id == -2)
				{
					this->kvstore->getobjIDlistBysubID(subject_id, object_list, object_list_len, true, txn);
				}
				//NOTICE:only literals should be unioned
				this_edge_literal_list.unionList(object_list, object_list_len, true);
				delete[] object_list;
			}
			// if the neighbor of this edge is variable, then the neighbor variable can not have any literal results,
			// we should add literals when join these two variables, see the Database::join function for details.

			// deprecated...
			// if the neighbor of this edge is variable, we should add all this neighbor variable's candidate entities' neighbor literal,
			// which has one corresponding predicate edge linking to this variable.
			else
			{
				continue;
				/*
				IDList& neighbor_candidate_list = this->basic_query->getCandidateList(neighbor_id);
				int neighbor_candidate_list_size = neighbor_candidate_list.size();
				for (int k = 0;k < neighbor_candidate_list_size; k ++)
				{
				int subject_id = neighbor_candidate_list.getID(k);
				int* object_list = NULL;
				int object_list_len = 0;

				(this->kvstore)->getobjIDlistBysubIDpreID(subject_id, predicate_id, object_list, object_list_len);
				this_edge_literal_list.unionList(object_list, object_list_len);
				delete []object_list;
				}
				*/
			}


			if (!flag)
			{
				flag = true;
				literal_candidate_list.unionList(this_edge_literal_list);
			}
			else
			{
				literal_candidate_list.intersectList(this_edge_literal_list);
			}
		}

		// this variable's literal candidates have been added.
		//this->basic_query->setAddedLiteralCandidate(var_id);
		if(flag)
		{
			this->basic_query->setReady(var_id);
		}
		else
		{
			continue;
		}

		// add the literal_candidate_list to the original candidate list.
		IDList& origin_candidate_list = this->basic_query->getCandidateList(var_id);
		//int origin_candidate_list_len = origin_candidate_list.size();
		origin_candidate_list.unionList(literal_candidate_list, true);
		//int after_add_literal_candidate_list_len = origin_candidate_list.size();

		//{
		//stringstream _ss;
		//_ss << "var[" << var_id << "] candidate list after add literal:\t"
		//<< origin_candidate_list_len << "-->" << after_add_literal_candidate_list_len << endl;
		/*
		for (int i = 0; i < after_add_literal_candidate_list_len; i ++)
		{
		int candidate_id = origin_candidate_list.getID(i);
		string candidate_name;
		if (i < origin_candidate_list_len)
		{
		candidate_name = (this->kvstore)->getEntityByID(origin_candidate_list.getID(i));
		}
		else
		{
		candidate_name = (this->kvstore)->getLiteralByID(origin_candidate_list.getID(i));
		}
		_ss << candidate_name << "(" << candidate_id << ")\t";
		}
		*/
		//Util::logging(_ss.str());
		//}
	}
	//Util::logging("OUT add_literal_candidate");
}

/*
这个函数和only_pre_filter_after_join的比较，处理join逻辑后续的结点；
待比较他们俩的逻辑之后，决定把哪一个补到愿策略的后面。
*/
//NOTICE:I think we should use this instead of only_pre_filter_after_join
//this function not only consider satellite predicates, but also one degree not selected var and other vars in join
//(constants are not necessary considered here)
//this check is fast because predicate num is small, but the performance can be very good
//(instead of filter when joining, we do a precheck first!)
bool
Join::allFilterByPres()
{
	//NOTICE:this check is a must to ensure that we can get all right answers
	//for core vertices after join, then we can generate satellites directly
	for(int i = 0; i < this->var_num; ++i)
	{
		if(this->basic_query->isSatelliteInJoin(i))
			continue;
		if(this->preFilter(i) == false)
			return false;
	}
	return true;
}

//NOTICE:we should only consider satellites, because constant neighbor edges are already dealed
//and edge in join can be dealed quicked if not satisfy by sp2o or op2s
bool    //false when no result for this basicquery
Join::preFilter(int _var)
{
	//if cans size is small, then can precise to avoid burst-increment when deep-join
	//if size is very large, the cost is high and not many can be filtered!
	//(keep state for each one-degree node, if considered)
	IDList& cans = this->basic_query->getCandidateList(_var);
	unsigned size = this->basic_query->getCandidateSize(_var);

	//result if already empty for non-literal variable
	if (size == 0)
	{
		if(!is_literal_var(_var))
			return false;
		else
			return true;
	}

	int var_degree = this->basic_query->getVarDegree(_var);
	//NOTICE:maybe several same predicates
	set<TYPE_PREDICATE_ID> in_edge_pre_id;
	set<TYPE_PREDICATE_ID> out_edge_pre_id;

	for (int i = 0; i < var_degree; i++)
	{
		char edge_type = this->basic_query->getEdgeType(_var, i);
		int triple_id = this->basic_query->getEdgeID(_var, i);
		Triple triple = this->basic_query->getTriple(triple_id);
		string neighbor;
		if (edge_type == Util::EDGE_OUT)
		{
			neighbor = triple.object;
		}
		else
		{
			neighbor = triple.subject;
		}

		//not consider edge with constant neighbors here
		if(neighbor[0] != '?')
		{
			//cout << "not to filter: " << neighbor_name << endl;
			continue;
		}
		//else
		//cout << "need to filter: " << neighbor_name << endl;

		TYPE_PREDICATE_ID pre_id = this->basic_query->getEdgePreID(_var, i);
		//WARN+BETTER:invalid(should be discarded in Query) or ?p(should not be considered here)
		if (pre_id < 0)
		{
			continue;
		}

		//TODO+BETTER: is any pre really used? do we need to losen the restrictions?

		//size:m<n; time:mlgn < n-m
		//The former time is computed because the m should be small if we select this p, tending to use binary-search 
		//when doing intersectList operation(mlgn < m+n).
		//The latter time is computed due to the unnecessary copy cost if not using this p
		TYPE_TRIPLE_NUM border = size / (Util::logarithm(2, size) + 1);
		//not use inefficient pre to filter
		if(this->dealed_triple[triple_id] || this->pre2num[pre_id] > border)
		{
			continue;
		}
		if(this->basic_query->isOneDegreeVar(neighbor))
		{
			this->dealed_triple[triple_id] = true;
		}

		if (edge_type == Util::EDGE_OUT)
		{
			out_edge_pre_id.insert(pre_id);
		}
		else
		{
			in_edge_pre_id.insert(pre_id);
		}
	}

	if (in_edge_pre_id.empty() && out_edge_pre_id.empty())
	{
		return true;
	}

	//NOTICE:use p2s here, use s2p in only_pre_filter_after_join because pres there are not efficient
	set<TYPE_PREDICATE_ID>::iterator it;
	unsigned* list = NULL;
	unsigned len = 0;
	for(it = in_edge_pre_id.begin(); it != in_edge_pre_id.end(); ++it)
	{
		this->kvstore->getobjIDlistBypreID(*it, list, len, true, txn);
		cans.intersectList(list, len);
		delete[] list;
	}
	if(!cans.empty())
	{
		for(it = out_edge_pre_id.begin(); it != out_edge_pre_id.end(); ++it)
		{
			this->kvstore->getsubIDlistBypreID(*it, list, len, true, txn);
			cans.intersectList(list, len);
			delete[] list;
		}
	}

	//this is a core vertex, so if not literal var, exit when empty
	if(!is_literal_var(_var) && cans.empty())
	{
	    return false;
	}

	cout << "var " << _var << "size after pre_filter " << cans.size() << endl;
	return true;
}

//if neighbor is an var, but not in select
//then, if its degree is 1, it has none contribution to filter
//only its edge property(predicate) makes sense
//we should make sure that current candidateVar has an edge matching the predicate
bool
Join::only_pre_filter_after_join()
{
	for (int var_id = 0; var_id < this->var_num; var_id++)
	{
		int var_degree = this->basic_query->getVarDegree(var_id);
		//cout<<"var: "<<this->basic_query->getVarName(var_id)<<endl;

		//get all the only predicate filter edges for this variable.
		vector<TYPE_PREDICATE_ID> in_edge_pre_id;
		vector<TYPE_PREDICATE_ID> out_edge_pre_id;

		for (int i = 0; i < var_degree; i++)
		{
			//cout<<"var linking edge: "<<i<<endl;

			//WARN:one degree not in select var's id is also -1 !!
			//constant neighbors already be dealed in literal_edge_filter
			//if(this->basic_query->getEdgeNeighborID(var_id, i) == -1)
			//continue;
			char edge_type = this->basic_query->getEdgeType(var_id, i);
			int triple_id = this->basic_query->getEdgeID(var_id, i);
			Triple triple = this->basic_query->getTriple(triple_id);
			//this is a pre var
			if(triple.predicate[0] == '?')
			{
				continue;
			}
			string neighbor_name;
			if (edge_type == Util::EDGE_OUT)
			{
				neighbor_name = triple.object;
			}
			else
			{
				neighbor_name = triple.subject;
			}


			//BETTER?:also check for satellite edges?change to isOneDegreeVar() and remove size-checking in copyToResult()
			bool only_preid_filter = (this->basic_query->isOneDegreeNotJoinVar(neighbor_name));
			if (!only_preid_filter || this->dealed_triple[triple_id])
			{
				//cout << "not to filter: " << neighbor_name << endl;
				continue;
			}
			this->dealed_triple[triple_id] = true;
			//else
			//cout << "need to filter: " << neighbor_name << endl;

			TYPE_PREDICATE_ID pre_id = this->basic_query->getEdgePreID(var_id, i);
			if (pre_id < 0)
			{
				continue;
			}

			if (edge_type == Util::EDGE_OUT)
			{
				out_edge_pre_id.push_back(pre_id);
			}
			else
			{
				in_edge_pre_id.push_back(pre_id);
			}
		}
		//cout<<"checkpoint1"<<endl;

		if(in_edge_pre_id.empty() && out_edge_pre_id.empty())
		{
			continue;
		}

		for (TableIterator it = this->current_table.begin(); it != this->current_table.end();)
		{
			TYPE_ENTITY_LITERAL_ID entity_id = (*it)[this->id2pos[var_id]];
			unsigned* pair_list = NULL;
			unsigned pair_len = 0;
			bool exist_preid = true;

			//NOTICE: four ways to judge if the predicates exist
			//getpreIDsubIDlistByobjID getpreIDobjIDlistBysubID
			//getsubIDlistBypreIDobjID getobjIDlistBysubIDpreID
			//I think the best one is: getpreIDlistBysubID getpreIDlistByobjID
			//how about getsubIDlistBypreID getobjIDlistBypreID
			//
			//the predicates in query can not be too large, so just loop
			//you can also use an intersect one if the two ordered list are both large
			if (exist_preid && !in_edge_pre_id.empty())
			{
				//(this->kvstore)->getpreIDsubIDlistByobjID(entity_id, pair_list, pair_len);
				(this->kvstore)->getpreIDlistByobjID(entity_id, pair_list, pair_len, true, txn);
				for (vector<TYPE_PREDICATE_ID>::iterator itr_pre = in_edge_pre_id.begin(); itr_pre != in_edge_pre_id.end(); itr_pre++)
				{
					TYPE_PREDICATE_ID pre_id = (*itr_pre);
					//exist_preid = Util::bsearch_preid_uporder(pre_id, pair_list, pair_len);
					if (Util::bsearch_int_uporder(pre_id, pair_list, pair_len) == INVALID)
					{
						exist_preid = false;
					}
					if (!exist_preid)
					{
						break;
					}
				}
				delete[] pair_list;
			}
			if (exist_preid && !out_edge_pre_id.empty())
			{
				//(this->kvstore)->getpreIDobjIDlistBysubID(entity_id, pair_list, pair_len);
				(this->kvstore)->getpreIDlistBysubID(entity_id, pair_list, pair_len, true, txn);

				for (vector<TYPE_PREDICATE_ID>::iterator itr_pre = out_edge_pre_id.begin(); itr_pre != out_edge_pre_id.end(); itr_pre++)
				{
					TYPE_PREDICATE_ID pre_id = (*itr_pre);
					//exist_preid = Util::bsearch_preid_uporder(pre_id, pair_list, pair_len);
					if (Util::bsearch_int_uporder(pre_id, pair_list, pair_len) == INVALID)
					{
						exist_preid = false;
					}
					if (!exist_preid)
					{
						break;
					}
				}
				delete[] pair_list;
			}

			//result sequence is illegal when there exists any missing filter predicate id.
			if (!exist_preid)
			{
				it = this->current_table.erase(it);
			}
			else
			{
				it++;
			}
		}
		if (this->current_table.empty())
		{
			return false;
		}
	}
	return true;
}

