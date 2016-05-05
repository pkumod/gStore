/*=============================================================================
# Filename: Join.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-12-13 16:44
# Description: implement functions in Join.h
=============================================================================*/

#include "Join.h"

using namespace std;

Join::Join()
{
	this->kvstore = NULL;
}

Join::Join(KVstore* _kvstore)
{
	this->kvstore = _kvstore;
}

Join::~Join()
{	
	//noting to do necessarily
}

void
Join::init(BasicQuery* _basic_query)
{
	this->basic_query = _basic_query;
	this->var_num = this->basic_query->getVarNum();
	this->id2pos = (int*)malloc(sizeof(int) * this->var_num);
    memset(id2pos, -1, sizeof(int) * this->var_num);
	this->pos2id = (int*)malloc(sizeof(int) * this->var_num);
    memset(pos2id, -1, sizeof(int) * this->var_num);
	this->id_pos = 0;
	this->start_id = -1;
    int triple_num = this->basic_query->getTripleNum();
	this->dealed_triple = (bool*)calloc(triple_num, sizeof(bool));
}

void
Join::clear()
{
	free(this->id2pos);
	free(this->pos2id);
	//NOTICE:maybe many BasicQuery
	this->current_table.clear();
	while(this->mystack.empty() == false) this->mystack.pop();
	free(this->dealed_triple);
}

double 
Join::score_node(unsigned _degree, unsigned _size)
{
	//PARAM_DEGREE * _degree - PARAM_SIZE * _size
	//BETTER?:use other cost model
	return Join::PARAM_DEGREE * (double)_degree + Join::PARAM_SIZE / (double)_size;
}

int 
Join::judge(int _smallest, int _biggest)
{
	return 0; //DEBUG 
	//BETTER?:use appropiate method according to size and structure
	int edge_num = this->basic_query->getTripleNum();
	double dense = (double)edge_num / this->var_num;
	//BETTER:how to guess the size of can_lists
	double size = (_smallest + _biggest) / 2.0;
	double ans = Join::PARAM_DENSE * dense - size / Join::PARAM_SIZE;
	if(ans > Join::JUDGE_LIMIT)
		return 0;	//multi_join method
	else
		return 1;	//index_join method
}

//select the start point, and maybe search order for index_join
//just for multi_join here, maybe diffrent for index_join later
void 
Join::select()
{
	//NOTICE: only consider vars in select here
	double max = 0;
	int maxi = -1;
	//int border = this->basic_query->getVarNum();
	for(int i = 0; i < this->var_num; ++i)
	{
		//multi-join not works better in star graph
		//QUERY:worse than original in q4.sql for dbpedia2014.nt, analyse
		//from which the join process start?
        if(this->basic_query->isFreeLiteralVariable(i))
        {
            continue;
        }
		double tmp = this->score_node(this->basic_query->getVarDegree(i), this->basic_query->getCandidateSize(i));
		if(tmp > max)
		{
			max = tmp;
			maxi = i;
		}
	}
	if(maxi == -1)  //all vars are literal, so start from anywhere
	{
		this->start_id = 0;
	}
	else
	{
		this->start_id = maxi;
	}	
#ifdef DEBUG_JOIN
	printf("the start id is: %d\n", this->start_id);
#endif
}

//join on the vector of CandidateList, available after 
//retrieved from the VSTREE and store the resut in _result_set
bool
Join::join_sparql(SPARQLquery& _sparql_query)
{
    int basic_query_num = _sparql_query.getBasicQueryNum();
    //join each basic query
    for(int i=0; i < basic_query_num; i++)
    {
        //printf("Basic query %d\n", i);
        this->init(&(_sparql_query.getBasicQuery(i)));
        long begin = Util::get_cur_time();
        this->filter_before_join();
        long after_filter = Util::get_cur_time();
        //printf("after filter_before_join: used %ld ms\n", after_filter-begin);
        this->add_literal_candidate();
        long after_add_literal = Util::get_cur_time();
        //printf("after add_literal_candidate: used %ld ms\n", after_add_literal-after_filter);
        this->join();
        long after_joinbasic = Util::get_cur_time();
        //printf("after join_basic : used %ld ms\n", after_joinbasic-after_add_literal);
        //this->only_pre_filter_after_join(this->basic_query);
        //long after_pre_filter_after_join = Util::get_cur_time();
        //printf("after only_pre_filter_after_join : used %ld ms\n", after_pre_filter_after_join-after_joinbasic);

        //printf("Final result size: %lu\n", this->basic_query->getResultList().size());
		this->clear();
    }

    return true;
}

// use the appropriate method to join candidates
bool
Join::join()
{
	//TODO:compute the time cost of different parts
	//the smallest candidate list size of the not-literal vars
	int smallest = this->basic_query->getCandidateSize(this->basic_query->getVarID_FirstProcessWhenJoin());
	if(smallest == 0)
		return false;  //empty result
	int biggest = this->basic_query->getVarID_MaxCandidateList();

	int method = this->judge(smallest, biggest);
	switch(method)
	{
	case 0:
		this->multi_join();
		break;
	case 1:
		this->index_join();
		break;
	default:
		break;
	}

	return true;
}

void
Join::add_new_to_results(TableIterator it, int id)
{
	//NTC:already have one more in *it if need to push back
	RecordType tmp(*it);	
	*(tmp.rbegin()) = id;
	this->current_table.push_back(tmp);
}

int
Join::choose_next_node(bool* _dealed_triple, int id)
{ 
	//choose a child to search deeply
	int degree = this->basic_query->getVarDegree(id);
	int maxi = -1;
	double max = 0;
	for(int i = 0; i < degree; ++i)
	{
		// each triple/edge need to be processed only once.
		int edge_id = this->basic_query->getEdgeID(id, i);
		if(_dealed_triple[edge_id])
		{
			continue;
		}
		int var_id2 = this->basic_query->getEdgeNeighborID(id, i);
		if(var_id2 == -1)  //not in join, including constant
		{
			continue;
		}
		//NTC:not using updated degrees, other not the whole loop
		double tmp = this->score_node(this->basic_query->getVarDegree(var_id2), this->basic_query->getCandidateSize(var_id2));
		if(max < tmp)
		{
			max = tmp;
			maxi = i;
		}
	}		
	return maxi;
}

void
Join::acquire_all_id_lists(IdLists& _id_lists, IdListsLen& _id_lists_len, IDList& _can_list, vector<int>& _edges, bool* _dealed_triple, int _id, int can_list_size)
{
	int* tmp_id_list;
	int tmp_id_list_len;
	for(int i = 0; i < this->id_pos; ++i)
	{
		// keep empty if not valid/used
		_id_lists.push_back(vector<int*>());
		_id_lists_len.push_back(vector<int>());
		int edge_index = _edges[i];
		if(edge_index != -1)
		{
			int pre_id = this->basic_query->getEdgePreID(_id, edge_index);
			//int edge_id = this->basic_query->getEdgeID(_id, edge_index);
			int edge_type = this->basic_query->getEdgeType(_id, edge_index);
			if(pre_id >= 0)  // valid
			{
				for(int j = 0; j < can_list_size; ++j)
				{
					if(edge_type == BasicQuery::EDGE_IN)
					{
						this->kvstore->getsubIDlistByobjIDpreID(_can_list[i], \
					  pre_id, tmp_id_list, tmp_id_list_len);
					}
					else      //EDGE_OUT
					{
						this->kvstore->getobjIDlistBysubIDpreID(_can_list[i],\
							pre_id, tmp_id_list, tmp_id_list_len);
					}
					_id_lists.rbegin()->push_back(tmp_id_list);
					_id_lists_len.rbegin()->push_back(tmp_id_list_len);
				}	
			}
		}	
	}
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
	if(this->basic_query->isAddedLiteralCandidate(_id))
		return false;
	else
		return true;
}

bool
Join::is_literal_ele(int _id)
{
	return _id >= Util::LITERAL_FIRST_ID;	
}

//TODO:add debug info and check when the var is not free
bool 
Join::new_join_with_multi_vars_prepared(IdLists& _id_lists, IdListsLen& _id_lists_len, vector<int>& _edges, IDList& _can_list, int _can_list_size)
{
	if(_can_list_size == 0)
	{
		return false;   //empty result
	}
	bool found = false;    //no record matched
	bool if_new_start = false; //the first to add to end in while
	//list< list<int> > temp_table;
	for(TableIterator it0 = this->current_table.begin(); it0 != this->new_start;)
	{
		bool matched = false;   //this record matched
		bool added = false;   //if one ele added already
		for(int i = 0; i < _can_list_size; ++i)
		{
			int cnt = 0;
			bool linked = true;
			for(RecordIterator it1 = it0->begin(); it1 != it0->end(); ++it1, ++cnt)
			{
				int edge_index = _edges[cnt];
				if(edge_index == -1)
				{
					continue;
				}
				int ele = *it1;
				if(_id_lists_len[cnt][i] == 0)
				{
					linked = false;
					break;
				}
				if(Util::bsearch_int_uporder(ele, _id_lists[cnt][i], _id_lists_len[cnt][i]) == -1)
				{
					linked = false;
					break;
				}
			}
			if(linked)
			{
				if(added)
				{
					this->add_new_to_results(it0, _can_list[i]);
					if(!if_new_start)
					{
						if_new_start = true;
						this->new_start = this->current_table.end();
						this->new_start--;
					}
				}
				else
				{
					added = true;
					it0->push_back(_can_list[i]);
				}
				matched = true;
			}
		}
		if(matched)
		{
			found = true;
			it0++;
			//it3++;
		}
		else
		{
			it0 = this->current_table.erase(it0);
			//it3 = this->table_row_new.erase(it3);
		}		
	}
	return found;
}

bool
Join::new_join_with_multi_vars_not_prepared(vector<int>& _edges, IDList& _can_list, int _can_list_size, int _id, bool _is_literal)
{
	if(_can_list_size == 0 && !_is_literal)
	{
		return false;   //empty result
	}
	bool found = false;
	bool if_new_start = false; //the first to add to end in while
	for(TableIterator it0 = this->current_table.begin(); it0 != this->new_start;)
	{
#ifdef DEBUG_JOIN
		if(this->new_start != this->current_table.end())
		{
			printf("now the new_start is:");
			for(RecordIterator it1 = this->new_start->begin(); it1 != this->new_start->end(); ++it1)
			{
				printf(" %d", *it1);
			}
			printf("\n");
		}
		else
			printf("new_start still in end?!\n");
		printf("now the record is:");
		for(RecordIterator it1 = it0->begin(); it1 != it0->end(); ++it1)
		{
			printf(" %d", *it1);
		}
		printf("\n");
#endif

		int cnt = 0;
		//update the valid id num according to restrictions by multi vars
		//also ordered while id_list and can_list are ordered
		list<int> valid_ans_list; 
		bool matched = true;
		for(RecordIterator it1 = it0->begin(); it1 != it0->end(); ++it1, ++cnt)
		{
#ifdef DEBUG_JOIN
			printf("cnt is: %d\n", cnt);
#endif
			int edge_index = _edges[cnt];
			if(edge_index == -1)
			{
				continue;
			}
#ifdef DEBUG_JOIN
			printf("edge exists!\n");
#endif
			int ele = *it1;
			int edge_type = this->basic_query->getEdgeType(_id, edge_index);
			int pre_id = this->basic_query->getEdgePreID(_id, edge_index);
			int* id_list;
			int id_list_len;
            if (edge_type == BasicQuery::EDGE_IN)
            {
#ifdef DEBUG_JOIN
				printf("this is an edge to our id to join!\n");
#endif
                this->kvstore->getobjIDlistBysubIDpreID(ele,\
						pre_id, id_list, id_list_len);
            }
            else
            {
#ifdef DEBUG_JOIN
				printf("this is an edge from our id to join!\n");
#endif
                this->kvstore->getsubIDlistByobjIDpreID(ele,\
						pre_id, id_list, id_list_len);
            }
			if(id_list_len == 0)
			{
				//id_list == NULL in this case, no need to free
				matched = false;
#ifdef DEBUG_JOIN
				printf("this id_list is empty!\n");
#endif
				break;
			}

			//BETTER?:to union/intersect two ordered lists directly?
			//The cost to join two ordered lists are the basic operation 
			//of the whole join process!
			//only can occur the first time, means cnt == 0
			if(valid_ans_list.empty())
			{
				for(int i = 0; i < id_list_len; ++i)
				{
					//if we found this element(entity/literal) in 
					//var1's candidate list, or this is a literal 
					//element and var2 is a free literal variable,
					//we should add this one to result.
					bool flag = false;
//NOTICE:this var is free, but it can also contain 
//entities. Candidates after retrieved from vstree will 
//contain all possible entities, but no literals.
					if(this->is_literal_ele(id_list[i]))
					{
						if(_is_literal)
						{
							flag = true;
#ifdef DEBUG_JOIN
							printf("to add literal for free variable!\n");
#endif
						}
					}
					else
					{
						flag = _can_list.bsearch_uporder(id_list[i]) >= 0;
					}		
					if(!flag) continue;
					//printf("add the ele to list!\n");
					valid_ans_list.push_back(id_list[i]);
				}
			}
			else
			{
				for(list<int>::iterator it2 = valid_ans_list.begin(); it2 != valid_ans_list.end();)
				{
					int tmp = *it2;
					if(Util::bsearch_int_uporder(tmp, id_list, id_list_len) == -1)	
					{
						it2 = valid_ans_list.erase(it2);
					}
					else
					{
						it2++;
					}
				}
			}
			delete[] id_list;
			if(valid_ans_list.empty())
			{
				matched = false;
				break;
			}
		}
		if(matched)
		{
#ifdef DEBUG_JOIN
			printf("this record is matched!!\n");
#endif
			found = true;			
			bool added = false;
			//add new var results to table from valid_ans_list
			for(list<int>::iterator it2 = valid_ans_list.begin(); it2 != valid_ans_list.end(); ++it2)
			{
				if(added)
				{
					this->add_new_to_results(it0, *it2);
					if(!if_new_start)
					{
						if_new_start = true;
						//DEBUG:we can acquire ele by *it in this way,
						//but it still equals end()!
						//this->new_start = this->current_table.rbegin().base();
						this->new_start = this->current_table.end();
						this->new_start--; //-1 is not allowed 
#ifdef DEBUG_JOIN
						if(this->new_start == this->current_table.end())
							printf("error to set new_start!\n");
						else
							printf("now the new_start is set again!\n");
#endif
					}
				}
				else
				{
					added = true;
					it0->push_back(*it2);
				}
			}	
			it0++;
		}
		else
		{
			it0 = this->current_table.erase(it0);
#ifdef DEBUG_JOIN
			printf("this record is not matched!\n");
#endif
		}		
	}
	return found;
}

bool
Join::if_prepare_idlist(int _can_list_size, bool _is_literal)
{
		if(!_is_literal && _can_list_size < Join::LIMIT_CANDIDATE_LIST_SIZE)
			return true;
		else
			return false;
}

void
Join::add_id_pos_mapping(int _id)
{
	this->pos2id[this->id_pos] = _id;
	this->id2pos[_id] = this->id_pos;
	this->id_pos++;
}

void
Join::multi_join()
{
	this->select();
	//bool* dealed_id_list = (bool*)malloc(sizeof(bool) * var_num);
    //memset(dealed_id_list, 0, sizeof(bool) * var_num);
	//bool* dealed_triple = (bool*)malloc(sizeof(bool) * triple_num);
    //memset(dealed_triple, 0, sizeof(bool) * triple_num);
	
	//keep an increasing vector for temp results, not in id order
	//vals num generally < 10, so just enum them and check if conncted
	//finally, copy in order to result_list in BasicQuery
	TableIterator it0;
	list<int>::iterator it1;
	vector<int>::iterator it2;
	//list<bool>::iterator it3;
    IDList& start_table = this->basic_query->getCandidateList(this->start_id);
	int start_size = this->basic_query->getCandidateSize(this->start_id);
    for(int i = 0; i < start_size; ++i)
    {
		RecordType record(1, start_table.getID(i));
		this->current_table.push_back(record);
		//this->table_row_new.push_back(false);
    }
	this->add_id_pos_mapping(this->start_id);
	this->new_start = this->current_table.end();
	
	//BETTER?:we can use nodes in stack to consider links instead of
	//nodes in current_table, but this needs the stack to be visited 
	//below top, requiring us to implement on our own(array/vector)
	//DEBUG: var_num > 100, maybe using vector, increasing dynamicly
	//int mystack[100];   
	//int top = -1;
	//mystack[++top] = this->start_id;
	//
	//if using nodes in current_table to consider links, no [] 
	//can be used(except changing to vector, but wasteful)
	//and then visit eles below top in stack is not ok, 
	//so choose STL stack
	this->mystack.push(this->start_id);
	while(!this->mystack.empty())
	{
		int id = this->mystack.top();
		//int id = mystack[top];
		int maxi = this->choose_next_node(dealed_triple, id);
		if(maxi == -1) //all edges of this node are dealed
		{
			//top--;
			this->mystack.pop();
			continue;	
		}
		int id2 = this->basic_query->getEdgeNeighborID(id, maxi);

		//pre_id == -1 means we cannot find such predicate in rdf file, so the result set of this sparql should be empty.
		//note that we cannot support to query sparqls with predicate variables ?p.
		//TODO: if all missed?!
		//preid < 0 !
		//if(id_list[cnt].empty())
		//{
		//	ifEmpty = true;
		//	break;
		//}

		vector<int> edges; //the edge index for table column in id2
		// the outer is node-loop, inner is canlist-loop
		vector< vector<int*> > id_lists;
		vector< vector<int> > id_lists_len;
		//int* tmp_id_list;
		//int tmp_id_list_len;
		IDList& can_list = this->basic_query->getCandidateList(id2);
		int can_list_size = can_list.size();

		for(int i = 0; i < this->id_pos; ++i)
		{
			int edge_index = this->basic_query->getEdgeIndex(id2, this->pos2id[i]);
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
		bool is_literal = this->is_literal_var(id2);

		bool flag = false;
		bool if_prepare = this->if_prepare_idlist(can_list_size, is_literal);
//#ifdef DEBUG_JOIN
		if_prepare = false;
//#endif
		//needed if place can_list in the outer loop to join
		if(if_prepare)
		{
			this->acquire_all_id_lists(id_lists, id_lists_len, can_list, edges, dealed_triple, id2, can_list_size);
			flag = this->new_join_with_multi_vars_prepared(id_lists, id_lists_len, edges, can_list, can_list_size);
			//need to release id_lists if using acquire_all_id_lists() firstly
			for(vector< vector<int*> >::iterator p1 = id_lists.begin(); p1 != id_lists.end(); ++p1)
			{
				for(vector<int*>::iterator p2 = p1->begin(); p2 != p1->end(); ++p2)
				{
					delete[] *p2;
				}
			}
		}
		else
		{
			flag = this->new_join_with_multi_vars_not_prepared(edges, can_list, can_list_size, id2, is_literal);
		}

		//if current_table is empty, ends directly
		if(!flag)
		{
			//break;
			return; //to avoid later invalid copy 
		}

		for(int i = 0; i < this->id_pos; ++i)
		{
			int edge_index = edges[i];
			if(edge_index != -1)
			{
				int edge_id = this->basic_query->getEdgeID(id2, edge_index);
				dealed_triple[edge_id] = true;
			}	
		}

		this->new_start = this->current_table.end();
		this->add_id_pos_mapping(id2);
		this->mystack.push(id2);
	}	

	this->only_pre_filter_after_join();

	//copy to result list, adjust the vars order
    vector<int*>& result_list = this->basic_query->getResultList();
    result_list.clear();
	int select_var_num = this->basic_query->getSelectVarNum();
	for(it0 = this->current_table.begin(); it0 != this->current_table.end(); ++it0)
	{
        //int* record = (int*)malloc(sizeof(int) * select_var_num);
        int* record = new int[select_var_num];
		for(int i = 0; i < this->id_pos; ++i)
		{
			if(this->pos2id[i] < select_var_num)
				record[this->pos2id[i]] = (*it0)[i];		
		}
        result_list.push_back(record);
	}

	//QUERY:the var order in output seems to be different
	//on screen compared with in file(the right one)

	//BETTER?:though the whole current_table is ordered here, the
	//selected columns are not definitely ordered, needing to be
	//sorted at the end. We can join based on the selected var's 
	//candidate to ensure the order, but this may be complicated.
	//If we want to ensure the order here, new table is a must!
	//and the duplicates cannot be checked unless the last step!
	//The result list will not be too large generally, and the sort
	//is not in any loop.(but if the size is too large?)
}

//BETTER?:place multi_join and index_join in separated files

//TODO:add debug info and check
void
Join::index_join()
{
	//NOTICE:can combine multi-join if do index_join for each edge 
	//firstly(two-sided), then search deeply(check return edge), 
	//each time a path,if all is linked, then it is a result
	//However, the performance is hard to guarantee, because the
	//pre index-join is costly(always all candidates)
	//index-join saves the cost to compute links every time, while
	//other temporal-table based method may get a smaller table size 
	//to continue(no need to compute all candidates).
	//
	//BETTER?:there is two ways to do better, notice that the question to
	//add literals or not can be solved by isAdded...
	//The index join method saves the memory cost because 2m+2mn < 3mn,
	//and time may be reduced if the pre-process is not too costly
	//because we can reuse the links other than recompute in temporal table
	//New struct is needed for node, i.e. list<int, list<iterator> >, 
	//because we may have to delete, but how can we know if an iterator 
	//is valid if the one it points to is removed?
	//1. based on edges: process each time only in valid area(already 
	//macthed with others, invalid is removed), and finally it must be 
	//all ok, just copy to result_list. We should select the edge order
	//to better the efficiency, but how can we keep only a neighbor links
	//set if we want to save memory?(ensure all can be linked later)
	//2. based on points: search deeply while multi-index-join, only a 
	//neighbor links set is kept for a node(not every edge), so memory 
	//cost is low. Finally, travel around along valid iterator, copy...
}

//sort the candidate lists and deal with all constant neigbors
void
Join::filter_before_join()
{
    //printf("*****IIIIIIN filter_before_join\n");

    for(int i = 0; i < this->var_num; i++)
    {
        //printf("\tVar%d %s\n", i, this->basic_query->getVarName(i).c_str());
        IDList &can_list = this->basic_query->getCandidateList(i);
        //printf("\t\tsize of canlist before filter: %d\n", can_list.size());
        //NOTICE:must sort before using binary search.
        can_list.sort();

        long begin = Util::get_cur_time();
        this->literal_edge_filter(i);
        long after_literal_edge_filter = Util::get_cur_time();
        //printf("\t\tliteral_edge_filter: used %ld ms\n", after_literal_edge_filter-begin);
//		this->preid_filter(this->basic_query, i);
//		long after_preid_filter = Util::get_cur_time();
//		cout << "\t\tafter_preid_filter: used " << (after_preid_filter-after_literal_edge_filter) << " ms" << endl;
        //printf("\t\t[%d] after filter, candidate size = %d\n\n\n", i, can_list.size());

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
    }
    //printf("OOOOOOUT filter_before_join\n");
}

//decrease the candidates of _var_i using its constant neighbors
void
Join::literal_edge_filter(int _var_i)
{
    //Util::logging("IN literal_edge_filter"); //debug

    int var_degree = this->basic_query->getVarDegree(_var_i);
    for(int j = 0; j < var_degree; j ++)
    {
        int neighbor_id = this->basic_query->getEdgeNeighborID(_var_i, j);
        if(neighbor_id != -1)   //variables in join not considered here
        {
            continue;
        }
		
		//QUERY:not only constants considered?
        char edge_type = this->basic_query->getEdgeType(_var_i, j);
        int triple_id = this->basic_query->getEdgeID(_var_i, j);
        Triple triple = this->basic_query->getTriple(triple_id);
        string neighbor_name;

        if (edge_type == BasicQuery::EDGE_OUT)
        {
            neighbor_name = triple.object;
        }
        else
        {
            neighbor_name = triple.subject;
        }

        bool only_preid_filter = (this->basic_query->isOneDegreeNotSelectVar(neighbor_name));
        if(only_preid_filter)
        {
            continue;
        }
        int pre_id = this->basic_query->getEdgePreID(_var_i, j);
        IDList &_list = this->basic_query->getCandidateList(_var_i);

        int lit_id = (this->kvstore)->getIDByEntity(neighbor_name);
        if(lit_id == -1)
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

        int id_list_len = 0;
        int* id_list = NULL;
        if (pre_id >= 0)
        {
            if (edge_type == BasicQuery::EDGE_OUT)
            {
                (this->kvstore)->getsubIDlistByobjIDpreID(lit_id, pre_id, id_list, id_list_len);
            }
            else
            {
                (this->kvstore)->getobjIDlistBysubIDpreID(lit_id, pre_id, id_list, id_list_len);
            }
        }
        else
            // pre_id == -1 means we cannot find such predicate in rdf file, so the result set of this sparql should be empty.
            // note that we cannot support to query sparqls with predicate variables ?p.
        {
            id_list_len = 0;
//			if (edge_type == BasicQuery::EDGE_OUT)
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

        if(id_list_len == 0)
        {
            _list.clear();
            delete []id_list;
            return;
        }
        //			cout << "\t\t can:" << can_list.to_str() << endl;
        //			cout << "\t\t idlist has :";
        //			for(int i_ = 0; i_ < id_list_len; i_ ++)
        //			{
        //				cout << "[" << id_list[i_] << "]\t";
        //			}
        //			cout << endl;

        _list.intersectList(id_list, id_list_len);
        delete []id_list;
    }

    //Util::logging("OUT literal_edge_filter"); //debug
}

// this part can be omited or improved if the encode way of predicate
// is good enough
//also, we can decide whether we need run this part (if there are predicates encode overlap) by var_i's edge in queryGraph,
//for each edge e of var_i,
//if neightbor on e is an var, but not in select
//then, if the var's degree is 1, it has none contribution to filter
//only its sole edge property(predicate) makes sense
//we should make sure that var_i has an edge matching the predicate
//so this function will do the filtering
//TBD:
//if pre_id = -1,
//it means the entity id must has at least one  edge
//NOTICE:not used now!
void
Join::preid_filter(int _var_i)
{
    //IDList & _list, int _pre_id, char _edge_type
    for (int j = 0; j < this->basic_query->getVarDegree(_var_i); j++)
    {
        int neighbor_id = this->basic_query->getEdgeNeighborID(_var_i, j);
        //	continue;
        if (neighbor_id != -1)
        {
            continue;
        }

        char edge_type = this->basic_query->getEdgeType(_var_i, j);
        int triple_id = this->basic_query->getEdgeID(_var_i, j);
        Triple triple = this->basic_query->getTriple(triple_id);
        string neighbor_name;

        if (edge_type == BasicQuery::EDGE_OUT)
        {
            neighbor_name = triple.object;
        }
        else
        {
            neighbor_name = triple.subject;
        }

        // if neightbor is an var, but not in select
        // then, if its degree is 1, it has none contribution to filter
        // only its sole edge property(predicate) makes sense
        // we should make sure that current candidateVar has an edge matching the predicate
        bool only_preid_filter = (this->basic_query->isOneDegreeNotSelectVar(neighbor_name));
        if (!only_preid_filter)
        {
            continue;
        }

        int pre_id = this->basic_query->getEdgePreID(_var_i, j);
        IDList& _list = this->basic_query->getCandidateList(_var_i);
        int* remain_list = new int[_list.size()];
        int remain_len = 0;
        int _entity_id = -1;
        int* pair_list = NULL;
        int pair_len = 0;

        for (int i = 0; i < _list.size(); i++)
        {
            _entity_id = _list[i];
            if (edge_type == BasicQuery::EDGE_IN)
            {
                (this->kvstore)->getpreIDsubIDlistByobjID
                (_entity_id, pair_list,	pair_len);
            }
            else
            {
                (this->kvstore)->getpreIDobjIDlistBysubID
                (_entity_id, pair_list,	pair_len);
            }

            bool exist_preid = Util::bsearch_preid_uporder
                               (pre_id, pair_list,	pair_len);

            if (exist_preid)
            {
                remain_list[remain_len] = _entity_id;
                remain_len++;
            }

            delete[] pair_list;
            pair_len = 0;
        }// end for i 0 to _list.size 

        _list.intersectList(remain_list, remain_len);

		 //can be imported 
        delete[] remain_list;
	} //end for j : varDegree 
}

//if neighbor is an var, but not in select
//then, if its degree is 1, it has none contribution to filter
//only its sole edge property(predicate) makes sense
//we should make sure that current candidateVar has an edge matching the predicate
void
Join::only_pre_filter_after_join()
{
    for(int var_id = 0; var_id < this->var_num; var_id++)
    {
        int var_degree = this->basic_query->getVarDegree(var_id);

        //get all the only predicate filter edges for this variable.
        vector<int> in_edge_pre_id;
        vector<int> out_edge_pre_id;
        for(int i = 0; i < var_degree; i++)
        {
			//WARN:one degree not in select var's id is also -1 !!
			//constant neighbors already be dealed in literal_edge_filter
			//if(this->basic_query->getEdgeNeighborID(var_id, i) == -1)
				//continue;
            char edge_type = this->basic_query->getEdgeType(var_id, i);
            int triple_id = this->basic_query->getEdgeID(var_id, i);
            Triple triple = this->basic_query->getTriple(triple_id);
            string neighbor_name;

            if (edge_type == BasicQuery::EDGE_OUT)
            {
                neighbor_name = triple.object;
            }
            else
            {
                neighbor_name = triple.subject;
            }

            bool only_preid_filter = (this->basic_query->isOneDegreeNotSelectVar(neighbor_name));
            if (!only_preid_filter)
            {
                continue;
            }

            int pre_id = this->basic_query->getEdgePreID(var_id, i);

            if (edge_type == BasicQuery::EDGE_OUT)
            {
                out_edge_pre_id.push_back(pre_id);
            }
            else
            {
                in_edge_pre_id.push_back(pre_id);
            }
        }

        if (in_edge_pre_id.empty() && out_edge_pre_id.empty())
        {
            continue;
        }

        for(TableIterator it = this->current_table.begin(); it != this->current_table.end();)
        {
            int entity_id = (*it)[this->id2pos[var_id]];
            int* pair_list = NULL;
            int pair_len = 0;
            bool exist_preid = true;

			//BETTER?:use getsubIDlistByobjID or getobjIDlistBysubID to 
			//replace getpreIDlist...
            if(exist_preid && !in_edge_pre_id.empty())
            {
                (this->kvstore)->getpreIDsubIDlistByobjID(entity_id, pair_list, pair_len);

                for(vector<int>::iterator itr_pre = in_edge_pre_id.begin(); itr_pre != in_edge_pre_id.end(); itr_pre++)
                {
                    int pre_id = (*itr_pre);
                    exist_preid = Util::bsearch_preid_uporder(pre_id, pair_list, pair_len);
                    if(!exist_preid)
                    {
                        break;
                    }
                }
                delete[] pair_list;
            }
            if(exist_preid && !out_edge_pre_id.empty())
            {
                (this->kvstore)->getpreIDobjIDlistBysubID(entity_id, pair_list, pair_len);

                for(vector<int>::iterator itr_pre = out_edge_pre_id.begin(); itr_pre != out_edge_pre_id.end(); itr_pre++)
                {
                    int pre_id = (*itr_pre);
                    exist_preid = Util::bsearch_preid_uporder(pre_id, pair_list, pair_len);
                    if(!exist_preid)
                    {
                        break;
                    }
                }
                delete[] pair_list;
            }

            //result sequence is illegal when there exists any missing filter predicate id.
            if(!exist_preid)
            {
				it = this->current_table.erase(it);
            }
			else
			{
				it++;
			}
        }
    }
}

//BETTER?:merge with literal_edge_filter? 
//this only consider subject constant neighbors, while the latter also 
//consider constant object neighbors(literal), as well as entities 
//neighbors. 
//(only in objects, no constant neighbors are called free, dealed in join)
//
//NOTICE:not only literals, but also entities may be added here!!!
//(candidates already contain all possible entities, and entities 
//produced here may not be ok!)
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
    for(int i = 0; i < this->var_num; i++)
    {
        //debug
        //{
        //    stringstream _ss;
        //    _ss << "var[" << i << "]\t";
        //    if (this->basic_query->isLiteralVariable(i))
        //    {
        //        _ss << "may have literal result.";
        //    }
        //    else
        //    {
        //        _ss << "do not have literal result.";
        //    }
        //    _ss << endl;
        //    //Util::logging(_ss.str());
        //}

        if(!this->basic_query->isLiteralVariable(i))
        {
            // if this variable is not literal variable, we can assume that its literal candidates have been added.
            this->basic_query->setAddedLiteralCandidate(i);
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
        for(int j = 0; j < var_degree; j ++)
        {
            int neighbor_id = this->basic_query->getEdgeNeighborID(var_id, j);
            int predicate_id = this->basic_query->getEdgePreID(var_id, j);
            int triple_id = this->basic_query->getEdgeID(var_id, j);
            Triple triple = this->basic_query->getTriple(triple_id);
            string neighbor_name = triple.subject;
            IDList this_edge_literal_list;

            // if the neighbor of this edge is an entity, we can add all literals which has an exact predicate edge linking to this entity.
            if(neighbor_id == -1)
            {
                int subject_id = (this->kvstore)->getIDByEntity(neighbor_name);
                int* object_list = NULL;
                int object_list_len = 0;

                (this->kvstore)->getobjIDlistBysubIDpreID(subject_id, predicate_id, object_list, object_list_len);
                this_edge_literal_list.unionList(object_list, object_list_len);
                delete []object_list;
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


            if(!flag)
            {
				flag = true;
                literal_candidate_list.unionList(this_edge_literal_list);
            }
            else
            {
                literal_candidate_list.intersectList(this_edge_literal_list);
            }
        }

        // add the literal_candidate_list to the original candidate list.
        IDList& origin_candidate_list = this->basic_query->getCandidateList(var_id);
        int origin_candidate_list_len = origin_candidate_list.size();
        origin_candidate_list.unionList(literal_candidate_list);
        int after_add_literal_candidate_list_len = origin_candidate_list.size();

        // this variable's literal candidates have been added.
        this->basic_query->setAddedLiteralCandidate(var_id);

        //debug
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

