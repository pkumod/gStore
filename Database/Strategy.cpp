/*=============================================================================
# Filename: Strategy.cpp
# Author: Bookug Lobert
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-05-07 16:31
# Description: implement functions in Strategy.h
=============================================================================*/

#include "Strategy.h"

using namespace std;

Strategy::Strategy()
{
	this->method = 0;
	this->kvstore = NULL;
	// this->vstree = NULL;
	this->fp = NULL;
	this->export_flag = false;
	this->txn = nullptr;
	//this->prepare_handler();
}

Strategy::Strategy(KVstore* _kvstore, TYPE_TRIPLE_NUM* _pre2num, TYPE_TRIPLE_NUM* _pre2sub,
 	TYPE_TRIPLE_NUM* _pre2obj, TYPE_PREDICATE_ID _limitID_predicate, TYPE_ENTITY_LITERAL_ID _limitID_literal,
	TYPE_ENTITY_LITERAL_ID _limitID_entity,bool _is_distinct, shared_ptr<Transaction> _txn)
{
	this->method = 0;
	this->kvstore = _kvstore;
	// this->vstree = _vstree;
	this->pre2num = _pre2num;
	this->pre2sub = _pre2sub;
	this->pre2obj = _pre2obj;
	this->limitID_predicate = _limitID_predicate;
	this->limitID_literal = _limitID_literal;
	this->limitID_entity = _limitID_entity;

	this->isDistinct = _is_distinct;
	this->fp = NULL;
	this->export_flag = false;
	this->txn = _txn;
	//this->prepare_handler();
}

Strategy::~Strategy()
{
	//delete[] this->dispatch;
}

//void
//Strategy::prepare_handler()
//{
//this->dispatch = new QueryHandler[Strategy::QUERY_HANDLER_NUM];
//this->dispatch[0] = Strategy::handler0;
//}

//NOTICE: 2-triple case ?s1 p1 c0 ?s2 p2 c0 is viewed as an unconnected graph
//however, this can be dealed due to several basic queries and linking

bool
Strategy::handle(SPARQLquery& _query)
{
#ifdef MULTI_INDEX
	Util::logging("IN GeneralEvaluation::handle");

	vector<BasicQuery*>& queryList = _query.getBasicQueryVec();
	// enumerate each BasicQuery and retrieve their variables' mapping entity in the VSTree.
	vector<BasicQuery*>::iterator iter = queryList.begin();
	for (; iter != queryList.end(); iter++)
	if ((**iter).getEncodeBasicQueryResult())
	{
		this->method = -1;

		vector<unsigned*>& result_list = (*iter)->getResultList();
		//int select_var_num = (*iter)->getSelectVarNum();
		//the num of vars needing to be joined, i.e. selectVarNum if only one triple
		int varNum = (*iter)->getVarNum();  
		//all variables(not including pre vars)
		int total_num = (*iter)->getTotalVarNum();
		int pre_varNum = (*iter)->getPreVarNum();
		int selected_pre_var_num = (*iter)->getSelectedPreVarNum();
		int selected_var_num = (*iter)->getSelectVarNum();

		//NOTICE: special case - query vertices only connected via same variables
		//all constant triples will be viewed as unconnected, if a triple has no variable, 
		//then this triple is a BGP(no other triples in this BGP)
		if(total_num == 0 && pre_varNum == 0)
		{
			this->method = 5;
		}
		else if ((*iter)->getTripleNum() == 1 && pre_varNum == 1)
		{
			if(this->export_flag)
				this->method = 6;
			else
				this->method = 4;
		}

		if (this->method < 0 && pre_varNum == 0 && (*iter)->getTripleNum() == 1)    //only one triple and no predicates
		{
			//only one variable and one triple: ?s pre obj or sub pre ?o
			if (total_num == 1)
			{
				this->method = 1;
			}
			//only two vars: ?s pre ?o
			else if (total_num == 2)
			{
				if (varNum == 1)   //the selected id should be 0
				{
					this->method = 2;
				}
				else   //==2
				{
					this->method = 3;
				}
			}
			//cout << "this BasicQuery use query strategy 2" << endl;
			//cout<<"Final result size: "<<(*iter)->getResultList().size()<<endl;
			//continue;
		}
		if(this->method< 0)
		{
			this->method = 0;
		}

		//QueryHandler dispatch;
		//dispatch[0] = handler0;
		switch (this->method)
		{
		//BETTER: use function pointer array in C++ class
		case 0:
			//default:filter by vstree and then verified by join
			this->handler0(*iter, result_list);
			break;
		case 1:
			this->handler1(*iter, result_list);
			break;
		case 2:
			this->handler2(*iter, result_list);
			break;
		case 3:
			this->handler3(*iter, result_list);
			break;
		case 4:
			this->handler4(*iter, result_list);
			break;
		case 5:
			this->handler5(*iter, result_list);
			break;
		case 6:
			this->handler6(*iter, result_list);
			break;			
		default:
			cout << "not support this method" << endl;

		}
		if(this->method == 6)
		{
			cout << "BasicQuery -- Final result size: " << (*iter)->getResultList()[0][0] << endl;
			(*iter)->getResultList().clear();
		}
		else
		{
			cout << "BasicQuery -- Final result size: " << (*iter)->getResultList().size() << endl;
		}
	}
#else
	cout << "this BasicQuery use original query strategy" << endl;
	//VSTREE:
	//long tv_handle = Util::get_cur_time();
	//(this->vstree)->retrieve(_query);
	//cout << "after Retrieve, used " << (tv_retrieve - tv_handle) << "ms." << endl;
	long tv_retrieve = Util::get_cur_time();

	this->join = new Join(kvstore, pre2num, this->limitID_predicate, this->limitID_literal);
	this->join->join_sparql(_query);
	delete this->join;

	long tv_join = Util::get_cur_time();
	cout << "after Join, used " << (tv_join - tv_retrieve) << "ms." << endl;
#endif
	Util::logging("OUT Strategy::handle");
	return true;
}


bool 
Strategy::pre_handler(BasicQuery * basic_query, KVstore * kvstore, TYPE_TRIPLE_NUM* pre2num, 
	TYPE_TRIPLE_NUM* pre2sub, TYPE_TRIPLE_NUM* pre2obj, bool * dealed_triple)
{
	int triple_num = basic_query->getTripleNum();

	int var_num = basic_query->getVarNum();
	// use constant filter to estimate now many ffits exist
        vector<int> estimate_num(var_num,10000000);

	cout << "start constant filter here " << endl << endl;
	for (int _var_i = 0; _var_i < var_num; _var_i++)
	{
	    
	    //filter before join here
	    
	    int var_degree = basic_query->getVarDegree(_var_i);
	    IDList &_list = basic_query->getCandidateList(_var_i);
	    cout << "\tVar" << _var_i << " " << basic_query->getVarName(_var_i) << endl;
	   // this->basic_query->setReady(_var_i);
	    for (int j = 0; j < var_degree; j++)
	    {
	        int neighbor_id = basic_query->getEdgeNeighborID(_var_i, j);
			//-1: constant or variable not in join; otherwise, variable in join
	        if (neighbor_id != -1)   
	        {
	            continue;
	        }

	        char edge_type = basic_query->getEdgeType(_var_i, j);
	        int triple_id = basic_query->getEdgeID(_var_i, j);
	        Triple triple = basic_query->getTriple(triple_id);
	        string neighbor_name;
	        
	        if (edge_type == Util::EDGE_OUT)
	        {
	            neighbor_name = triple.object;
	        }
	        else
	        {
	            neighbor_name = triple.subject;
	        }
	        
	        bool only_preid_filter = (basic_query->isOneDegreeNotJoinVar(neighbor_name));
			//NOTICE: we only need to consider constants here
	        if(only_preid_filter)
	        {
	            continue;
	        }
	        else
	        {
	            dealed_triple[triple_id] = true;
				basic_query->setReady(_var_i);
	        }
	        
	        TYPE_PREDICATE_ID pre_id = basic_query->getEdgePreID(_var_i, j);
	        
	        TYPE_ENTITY_LITERAL_ID lit_id = (kvstore)->getIDByEntity(neighbor_name);
	        if (lit_id == INVALID_ENTITY_LITERAL_ID)
	        {
	            lit_id = (kvstore)->getIDByLiteral(neighbor_name);
	        }
	        
	        
	        unsigned id_list_len = 0;
	        unsigned* id_list = NULL;
	        if (pre_id >= 0)
	        {
	            if (edge_type == Util::EDGE_OUT)
	            {
	                kvstore->getsubIDlistByobjIDpreID(lit_id, pre_id, id_list, id_list_len, true, txn);
	            }
	            else
	            {
	                kvstore->getobjIDlistBysubIDpreID(lit_id, pre_id, id_list, id_list_len, true, txn);
	            }
	        }
	        else if (pre_id == -2)
	        {
	            if (edge_type == Util::EDGE_OUT)
	            {
	                kvstore->getsubIDlistByobjID(lit_id, id_list, id_list_len, true, txn);
	            }
	            else
	            {
	                kvstore->getobjIDlistBysubID(lit_id, id_list, id_list_len, true, txn);
	            }
	        }
	        else
	        {
	            id_list_len = 0;
	        }
	        
			//WARN: this may need to check, end directly
	        if (id_list_len == 0)
	        {
	            _list.clear();
	            delete[] id_list;
	            return false;
	        }
	        //updateList(_list, id_list, id_list_len);
	        if (_list.size() == 0)
	            _list.unionList(id_list,id_list_len);
	        else
	            _list.intersectList(id_list, id_list_len);
	        delete[] id_list;

	        if (_list.size() == 0)
	        {
	            return false;
	        }
	    }
            // skip pre_filter when the candidate of a variable is small
            // enough after constant_filter
            if (_list.size() > 0)
	    {
	    	 for (int j = 0; j < var_degree; j++)
	    	{
	        	int neighbor_id = basic_query->getEdgeNeighborID(_var_i, j);//1 1
			//-1: constant or variable not in join; otherwise, variable in join
	 	        if (neighbor_id == -1)   
	        	{
	            	    continue;
	        	}
	        	TYPE_PREDICATE_ID pre_id = basic_query->getEdgePreID(_var_i, j);
				if (pre_id < 0) continue;
	        	char edge_type = basic_query->getEdgeType(_var_i, j);
			int estimate_val;
			if (edge_type == Util::EDGE_OUT)				
			    estimate_val= _list.size()*pre2num[pre_id]/pre2sub[pre_id];
			else
			    estimate_val = _list.size()*pre2num[pre_id]/pre2obj[pre_id];
			if(estimate_val < estimate_num[neighbor_id])
			    estimate_num[neighbor_id] = estimate_val;
			
		}
    
	    }
	
            cout << "\t\t[" << _var_i << "] after constant filter, candidate size = " << _list.size() << endl << endl << endl;
	}

	cout << "pre filter start here" << endl;

	//TODO:use vector instead of set
	for(int _var = 0; _var < var_num; _var++)
	{
	    if(basic_query->isSatelliteInJoin(_var))
	        continue;
            
	    cout << "\tVar" << _var << " " << basic_query->getVarName(_var) << endl;
	    IDList& cans = basic_query->getCandidateList(_var);
	    unsigned size = basic_query->getCandidateSize(_var);
	    
	    //result if already empty for non-literal variable
	    /*
	    if (size == 0)
	    {
	        if(!is_literal_var(_var))
	            return false;
	        else
	            return true;
	    }
	    */
	    int var_degree = basic_query->getVarDegree(_var);
	    //NOTICE:maybe several same predicates
	    set<TYPE_PREDICATE_ID> in_edge_pre_id;
	    set<TYPE_PREDICATE_ID> out_edge_pre_id;
	    for (int i = 0; i < var_degree; i++)
	    {
	        char edge_type = basic_query->getEdgeType(_var, i);
	        int triple_id = basic_query->getEdgeID(_var, i);
	        Triple triple = basic_query->getTriple(triple_id);
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
	        
	        TYPE_PREDICATE_ID pre_id = basic_query->getEdgePreID(_var, i);
	        //WARN+BETTER:invalid(should be discarded in Query) or ?p(should not be considered here)
	        if (pre_id < 0)
	        {
	            continue;
	        }
	        
	        //size:m<n; time:mlgn < n-m
	        //The former time is computed because the m should be small if we select this p, tending to use binary-search
	        //when doing intersectList operation(mlgn < m+n).
	        //The latter time is computed due to the unnecessary copy cost if not using this p
	        //TYPE_TRIPLE_NUM border = size / (Util::logarithm(2, size) + 1);
	        //not use inefficient pre to filter
	        if(dealed_triple[triple_id])
	        {
	            continue;
	        }

	        double border = size / (Util::logarithm(2, size) + 1);
	        if(pre2num[pre_id] > border)
	        {
	        	cout << "skip the prefilter because the pre var is not efficent enough" << endl;
	        	continue;
	        }

	        int prefilter_num;
		if((edge_type == Util::EDGE_OUT && (prefilter_num = pre2obj[pre_id]) > estimate_num[_var]) ||
			(edge_type == Util::EDGE_IN && (prefilter_num = pre2sub[pre_id]) > estimate_num[_var]))
		{
			cout << "skip the prefilter because the constant filter is strong enough" << endl;
			// cout << "estimate_num:" << estimate_num[_var] << endl;
			// cout << "prefilter_num:" << prefilter_num << endl;
			continue;
		}

	        if(basic_query->isOneDegreeVar(neighbor))
	        {
	            dealed_triple[triple_id] = true;
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
	        continue;
	    }
	    basic_query->setReady(_var);
	    //NOTICE:use p2s here, use s2p in only_pre_filter_after_join because pres there are not efficient
	    set<TYPE_PREDICATE_ID>::iterator it;
	    unsigned* list = NULL;
	    unsigned len = 0;
	    for(it = in_edge_pre_id.begin(); it != in_edge_pre_id.end(); ++it)
	    {
	    	// if there exists a variable with limited matches in the query, then skip the filter of other
	    	// variables as soon as possible

		if(pre2num[*it] < 1000000 || cans.size() > 1000000 || cans.size() == 0)
        	{
		        kvstore->getobjIDlistBypreID(*it, list, len, true, txn);
		        if(cans.size() == 0)
		            cans.unionList(list,len);
		        else
		            cans.intersectList(list, len);
		        delete[] list;
		        if(cans.size() == 0)
				{	
					return false;
				}
			}
			else{
				int can_size = cans.size();
				for(std::vector<unsigned>::iterator i = cans.begin(); i != cans.end();)
				{
					kvstore->getpreIDlistByobjID(*i, list, len, true, txn);
					bool can_matched = false;
					int s = 0, e = len - 1;
					int mid = (s + e)/2;
					while (s <= e)
					{
						if(list[mid] == *it)
						{
							can_matched = true;
							break;
						}
						else if(list[mid] < *it)
						{
							s = mid + 1;
						}
						else {
							e = mid - 1;
						}
						mid = (s + e)/2;
					}
					if(can_matched == false)
					{
						i = cans.eraseAt(i);
					}
					else{
						i++;
					}
				}
			}
	    }
    	
    	if(in_edge_pre_id.size() != 0 && cans.size() == 0)
        {
  //          cout << "after in_edge_filter, the cans size = 0" << endl;
            return false;
        }
        for(it = out_edge_pre_id.begin(); it != out_edge_pre_id.end(); ++it)
	{
                
	    	// if there exists a variable with limited matches in the query, then skip the filter of other
	    	// variables as soon as possible
        	
  		if(pre2num[*it] < 1000000 || cans.size() > 1000000  || cans.size() == 0)
        	{
	            kvstore->getsubIDlistBypreID(*it, list, len, true, txn);
	            if(cans.size() == 0)
	                cans.unionList(list,len);
	            else
	                cans.intersectList(list, len);
	            delete[] list;
		        if(cans.size() == 0)
				{
					return false;
				}
			}
			else{
				int can_size = cans.size();
				for(std::vector<unsigned>::iterator i = cans.begin(); i != cans.end();)
				{
					kvstore->getpreIDlistBysubID(*i, list, len, true, txn);
					bool can_matched = false;
					int s = 0, e = len - 1;
					int mid = (s + e)/2;
					while (s <= e)
					{
						if(list[mid] == *it)
						{
							can_matched = true;
							break;
						}
						else if(list[mid] < *it)
						{
							s = mid + 1;
						}
						else {
							e = mid - 1;
						}
						mid = (s + e)/2;
					}
					if(can_matched == false)
					{
						i = cans.eraseAt(i);
					}else{
						i++;
					}
				}
			}
        }
	    
	    //this is a core vertex, so if not literal var, exit when empty
	    if(cans.empty())
	    {
	        return false;
	    }
	    cout << "\t\t[" << _var << "] after pre var filter, candidate size = " << cans.size() << endl << endl << endl;
	}

	return true;

}


void
Strategy::handler0(BasicQuery* _bq, vector<unsigned*>& _result_list)
{
	//long before_filter = Util::get_cur_time();
	cout << "this BasicQuery use query strategy 0 database" << endl;

	//BETTER:not all vars in join filtered by vstree
	//(A)-B-c: B should by vstree, then by c, but A should be generated in join(first set A as not)
	//if A not in join, just filter B by pre
	//divided into star graphs, join core vertices, generate satellites
	//join should also start from a core vertex(neighbor can be constants or vars) if available
	//
	//QUERY: is there any case that a node should be retrieved by other index?(instead of vstree or generate whne join)
	//
	//we had better treat 1-triple case(no ?p) as special, and then in other cases, core vertex exist(if connected)
	//However, if containing ?p and 1-triple, we should treat it also as a special case, or select a variable as core vertex
	//and retrieved (for example, ?s ?p o   or    s ?p ?o, generally no core vertex in these cases)

	long tv_handle = Util::get_cur_time();
	int varNum = _bq->getVarNum();  //the num of vars needing to be joined
	//TODO:parallel by pthread, requiring that index is parallelable
	//for (int i = 0; i < varNum; ++i)
	//{
		//if (_bq->if_need_retrieve(i) == false)
			//continue;
		//bool flag = _bq->isLiteralVariable(i);
		//const EntityBitSet& entityBitSet = _bq->getVarBitSet(i);
		//IDList* idListPtr = &(_bq->getCandidateList(i));
		//this->vstree->retrieveEntity(entityBitSet, idListPtr);
		//if (!flag)
		//{
			//cout<<"set ready: "<<i<<endl;
			//_bq->setReady(i);
		//}
		//the basic query should end if one non-literal var has no candidates
		//if (idListPtr->size() == 0 && !flag)
		//{
			//break;
		//}
	//}

	//BETTER:end directly if one is empty!

	long tv_retrieve = Util::get_cur_time();
	cout << "after Retrieve, used " << (tv_retrieve - tv_handle) << "ms." << endl;

	/*
		pre_handler()主要是对谓词过滤做一些预处理，对量级有一定要求，要求使用二分搜索；
		量级判断的条件和初始化部分有一定的问题；
		这部分代码可以考虑直接删除;
		对于谓词来说，我们尽可能直接调用pre2num来采用lazy initialization的方法获取，或者使用statistics进行获取。
	*/

	bool * d_triple = (bool*)calloc(_bq->getTripleNum(), sizeof(bool));

	bool ret2 = pre_handler(_bq, kvstore, pre2num,pre2sub,pre2obj, d_triple);
	long after_prehandler = Util::get_cur_time();
	cout << "after prehandler: used " << (after_prehandler - tv_retrieve) << " ms" << endl;
	if(!ret2){
		cout << "after the prehandler, the canlist size is 0." << endl;
	}

	Join *join = new Join(kvstore, pre2num, this->limitID_predicate, this->limitID_literal,this->limitID_entity, txn);
	join->join_basic(_bq,d_triple);
	delete join;

	long tv_join = Util::get_cur_time();
	cout << "during Join, used " << (tv_join - tv_retrieve) << "ms." << endl;
}

void
Strategy::handler1(BasicQuery* _bq, vector<unsigned*>& _result_list)
{
	long before_filter = Util::get_cur_time();
	cout << "this BasicQuery use query strategy 1" << endl;
	//int neighbor_id = (*_bq->getEdgeNeighborID(0, 0);  //constant, -1
	char edge_type = _bq->getEdgeType(0, 0);
	int triple_id = _bq->getEdgeID(0, 0);
	Triple triple = _bq->getTriple(triple_id);
	TYPE_PREDICATE_ID pre_id = _bq->getEdgePreID(0, 0);
	unsigned* id_list = NULL;
	unsigned id_list_len = 0;
	if (edge_type == Util::EDGE_OUT)
	{
		//cout<<"edge out!!!"<<endl;
		TYPE_ENTITY_LITERAL_ID nid = (this->kvstore)->getIDByEntity(triple.object);
		//if (nid == -1)
		if (nid == INVALID_ENTITY_LITERAL_ID)
		{
			nid = (this->kvstore)->getIDByLiteral(triple.object);
		}
		this->kvstore->getsubIDlistByobjIDpreID(nid, pre_id, id_list, id_list_len, true, txn);
	}
	else
	{
		//cout<<"edge in!!!"<<endl;
		this->kvstore->getobjIDlistBysubIDpreID(this->kvstore->getIDByEntity(triple.subject), pre_id, id_list, id_list_len, true, txn);
	}

	long after_filter = Util::get_cur_time();
	cout << "after filter, used " << (after_filter - before_filter) << "ms" << endl;
	_result_list.clear();
	//cout<<"now to copy result to list"<<endl;
	for (unsigned i = 0; i < id_list_len; ++i)
	{
		unsigned* record = new unsigned[1];    //only this var is selected
		record[0] = id_list[i];
		//cout<<this->kvstore->getEntityByID(record[0])<<endl;
		_result_list.push_back(record);
	}
	long after_copy = Util::get_cur_time();
	cout << "after copy to result list: used " << (after_copy - after_filter) << " ms" << endl;
	delete[] id_list;
	cout << "Final result size: " << _result_list.size() << endl;
}

void
Strategy::handler2(BasicQuery* _bq, vector<unsigned*>& _result_list)
{
	long before_filter = Util::get_cur_time();
	cout << "this BasicQuery use query strategy 2" << endl;
	int triple_id = _bq->getEdgeID(0, 0);
	Triple triple = _bq->getTriple(triple_id);
	TYPE_PREDICATE_ID pre_id = _bq->getEdgePreID(0, 0);

	//NOTICE:it is ok for var1 or var2 to be -1, i.e. not encoded
	int var1_id = _bq->getIDByVarName(triple.subject);
	int var2_id = _bq->getIDByVarName(triple.object);

	unsigned* id_list = NULL;
	unsigned id_list_len = 0;
	if (var1_id == 0)   //subject var selected
	{
		//use p2s directly
		this->kvstore->getsubIDlistBypreID(pre_id, id_list, id_list_len, false, txn);
	}
	else if (var2_id == 0) //object var selcted
	{
		//use p2o directly
		this->kvstore->getobjIDlistBypreID(pre_id, id_list, id_list_len, false, txn);
	}
	else
	{
		cout << "ERROR in Database::handle(): no selected var!" << endl;
	}
	long after_filter = Util::get_cur_time();
	cout << "after filter, used " << (after_filter - before_filter) << "ms" << endl;
	_result_list.clear();
	for (unsigned i = 0; i < id_list_len; ++i)
	{
		unsigned* record = new unsigned[1];    //only one var
		record[0] = id_list[i];
		_result_list.push_back(record);
	}
	long after_copy = Util::get_cur_time();
	cout << "after copy to result list: used " << (after_copy - after_filter) << " ms" << endl;
	delete[] id_list;
	cout << "Final result size: " << _result_list.size() << endl;
}

void
Strategy::handler3(BasicQuery* _bq, vector<unsigned*>& _result_list)
{
	long before_filter = Util::get_cur_time();
	cout << "this BasicQuery use query strategy 3" << endl;
	int triple_id = _bq->getEdgeID(0, 0);
	Triple triple = _bq->getTriple(triple_id);
	TYPE_PREDICATE_ID pre_id = _bq->getEdgePreID(0, 0);
	unsigned* id_list = NULL;
	unsigned id_list_len = 0;

	_result_list.clear();
	this->kvstore->getsubIDobjIDlistBypreID(pre_id, id_list, id_list_len, true, txn);
	int var1_id = _bq->getSelectedVarPosition(triple.subject);
	int var2_id = _bq->getSelectedVarPosition(triple.object);

	if(var1_id < 0 || var2_id < 0)
	{
		delete[] id_list;
		return;
	}

	long after_filter = Util::get_cur_time();
	cout << "after filter, used " << (after_filter - before_filter) << "ms" << endl;

	for (unsigned i = 0; i < id_list_len; i += 2)
	{
		unsigned* record = new unsigned[2];    //2 vars and selected
		record[var1_id] = id_list[i];
		record[var2_id] = id_list[i + 1];
		_result_list.push_back(record);
	}

	long after_copy = Util::get_cur_time();
	cout << "after copy to result list: used " << (after_copy - after_filter) << " ms" << endl;
	delete[] id_list;
	cout << "Final result size: " << _result_list.size() << endl;
}

void
Strategy::handler4(BasicQuery* _bq, vector<unsigned*>& _result_list)
{
	cout<<"Special Case: consider pre var in this triple"<<endl;
	int varNum = _bq->getVarNum();  
	//all variables(not including pre vars)
	int total_num = _bq->getTotalVarNum();
	int pre_varNum = _bq->getPreVarNum();
	int selected_pre_var_num = _bq->getSelectedPreVarNum();
	int selected_var_num = _bq->getSelectVarNum();
	Triple triple = _bq->getTriple(0);
	int pvpos = _bq->getSelectedPreVarPosition(triple.predicate);

	unsigned* id_list = NULL;
	unsigned id_list_len = 0;
	_result_list.clear();

	//cout<<"total num: "<<total_num <<endl;
	if (total_num == 2)
	{
		cout<<"Special Case 1"<<endl;
		int svpos = _bq->getSelectedVarPosition(triple.subject);
		int ovpos = _bq->getSelectedVarPosition(triple.object);
		cout<<"subject: "<<triple.subject<<" "<<svpos<<endl;
		cout<<"object: "<<triple.object<<" "<<ovpos<<endl;
		cout<<"predicate: "<<triple.predicate<<" "<<pvpos<<endl;
		
		bool only_get_distinct_pre = (ovpos < 0 && svpos < 0 && pvpos >= 0 && this->isDistinct);
		if(only_get_distinct_pre)
			for (TYPE_PREDICATE_ID i = 0; i < this->limitID_predicate; ++i)
			{
				TYPE_PREDICATE_ID pid = i;
				unsigned* record = new unsigned;
				*record = pid;
				_result_list.push_back(record);
			}
		else
		for(TYPE_PREDICATE_ID i = 0; i < this->limitID_predicate; ++i)
		{
		//very special case, to find all triples, select ?s (?p) ?o where { ?s ?p ?o . }
		//filter and join is too costly, should enum all predicates and use p2so
			TYPE_PREDICATE_ID pid = i;
			this->kvstore->getsubIDobjIDlistBypreID(pid, id_list, id_list_len, true, txn);
			int rsize = selected_var_num;
			if(selected_pre_var_num == 1)
			{
				rsize++;
			}

			//always place s/o before p in result list
			for (unsigned j = 0; j < id_list_len; j += 2)
			{
				unsigned* record = new unsigned[rsize];
				//check the s/o var if selected, need to ensure the placement order
				if(ovpos >= 0)
				{
					record[ovpos] = id_list[j+1];
				}
				if(svpos >= 0)
				{
					record[svpos] = id_list[j];
				}

				if(pvpos >= 0)
				{
					record[pvpos] = pid;      //for the pre var
				}
				_result_list.push_back(record);
			}
			delete[] id_list;
		}
		

		id_list = NULL;
	}
	else if (total_num == 1)
	{
		cout<<"Special Case 2"<<endl;
		int vpos = -1;
		if(triple.subject[0] != '?')  //constant
		{
			TYPE_ENTITY_LITERAL_ID sid = (this->kvstore)->getIDByEntity(triple.subject);
			this->kvstore->getpreIDobjIDlistBysubID(sid, id_list, id_list_len, true, txn);
			vpos = _bq->getSelectedVarPosition(triple.object);
		}
		else if(triple.object[0] != '?')  //constant
		{
			TYPE_ENTITY_LITERAL_ID oid = (this->kvstore)->getIDByEntity(triple.object);
			//if (oid == -1)
			if (oid == INVALID_ENTITY_LITERAL_ID)
			{
				oid = (this->kvstore)->getIDByLiteral(triple.object);
			}
			this->kvstore->getpreIDsubIDlistByobjID(oid, id_list, id_list_len, true, txn);
			vpos = _bq->getSelectedVarPosition(triple.subject);
		}

		int rsize = varNum;
		if(selected_pre_var_num == 1)
		{
			rsize++;
		}
		//always place s/o before p in result list
		for (unsigned i = 0; i < id_list_len; i += 2)
		{
			unsigned* record = new unsigned[rsize];
			if(vpos >= 0)
			{
				record[vpos] = id_list[i + 1]; //for the s/o var
			}
			if(pvpos >= 0)
			{
				record[pvpos] = id_list[i];      //for the pre var
			}
			_result_list.push_back(record);
		}
	}
	else if (total_num == 0)  //only ?p and it must be selected
	{
		cout<<"Special Case 3"<<endl;
		//just use so2p
		unsigned sid = (this->kvstore)->getIDByEntity(triple.subject);
		unsigned oid = (this->kvstore)->getIDByEntity(triple.object);
		if (oid == -1)
		{
			oid = (this->kvstore)->getIDByLiteral(triple.object);
		}

		this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len, true, txn);
		//copy to result list
		for (unsigned i = 0; i < id_list_len; ++i)
		{
			unsigned* record = new unsigned[1];
			record[0] = id_list[i];
			_result_list.push_back(record);
		}
	}

	delete[] id_list;
}

//TODO:if any constants in a query are not found in kvstore, then this BGP should end to speed up the processing

void
Strategy::handler5(BasicQuery* _bq, vector<unsigned*>& _result_list)
{
	cout<<"Special Case: consider constant triple"<<endl;
	Triple triple = _bq->getTriple(0);
	_result_list.clear();

	TYPE_ENTITY_LITERAL_ID subid = this->kvstore->getIDByEntity(triple.subject);
	//if(subid == -1) //not found
	if(subid == INVALID_ENTITY_LITERAL_ID) //not found
	{
		return;
	}
	TYPE_PREDICATE_ID preid = this->kvstore->getIDByPredicate(triple.predicate);
	//if(preid == -1) //not found
	if(preid == INVALID_PREDICATE_ID) //not found
	{
		return;
	}
	TYPE_ENTITY_LITERAL_ID objid = this->kvstore->getIDByEntity(triple.object);
	//if(objid == -1)
	if(objid == INVALID_ENTITY_LITERAL_ID)
	{
		objid = this->kvstore->getIDByLiteral(triple.object);
	}
	//if(objid == -1)
	if(objid == INVALID_ENTITY_LITERAL_ID)
	{
		return;
	}

	unsigned* id_list = NULL;
	unsigned id_list_len = 0;
	(this->kvstore)->getobjIDlistBysubIDpreID(subid, preid, id_list, id_list_len, true, txn);
	if (Util::bsearch_int_uporder(objid, id_list, id_list_len) != INVALID)
	{
		unsigned* record = new unsigned[3];
		record[0] = subid;
		record[1] = preid;
		record[2] = objid;
		_result_list.push_back(record);
	}
	delete[] id_list;
}

void
Strategy::handler6(BasicQuery* _bq, vector<unsigned*>& _result_list)
{
  cout << "Special Case:select * and write to stream" << endl;
  int varNum = _bq->getVarNum();
  //all variables(not including pre vars)
  int total_num = _bq->getTotalVarNum();
  int pre_varNum = _bq->getPreVarNum();
  int selected_pre_var_num = _bq->getSelectedPreVarNum();
  int selected_var_num = _bq->getSelectVarNum();
  Triple triple = _bq->getTriple(0);
  int pvpos = _bq->getSelectedPreVarPosition(triple.predicate);  

  unsigned* id_list = NULL;
  unsigned id_list_len = 0;
  _result_list.clear();

  int svpos = _bq->getSelectedVarPosition(triple.subject);
  int ovpos = _bq->getSelectedVarPosition(triple.object);
  cout<<"subject: "<<triple.subject<<" "<<svpos<<endl;
  cout<<"object: "<<triple.object<<" "<<ovpos<<endl;
  cout<<"predicate: "<<triple.predicate<<" "<<pvpos<<endl;
  //very special case, to find all triples, select ?s (?p) ?o where { ?s ?p ?o . }
  //filter and join is too costly, should enum all predicates and use p2so
  unsigned* rsize = new unsigned[1];
  rsize[0] = 0;
  for (TYPE_PREDICATE_ID i = 0; i < this->limitID_predicate; ++i)
  {
    TYPE_PREDICATE_ID pid = i;
    string p = this->kvstore->getPredicateByID(pid);
    string pre = Util::node2string(p.c_str());
    this->kvstore->getsubIDobjIDlistBypreID(pid, id_list, id_list_len, true, txn);
    for (unsigned j = 0; j < id_list_len; j += 2)
    {
   		string s = this->kvstore->getEntityByID(id_list[j]);
   	    string sub = Util::node2string(s.c_str());
   		string o;
   		if(id_list[j + 1] >= Util::LITERAL_FIRST_ID)
   			o = this->kvstore->getLiteralByID(id_list[j + 1]);
   		else
   			o = this->kvstore->getEntityByID(id_list[j + 1]);
   	    string obj = Util::node2string(o.c_str());
   		string record = sub + "\t" + pre + "\t" + obj + ".\n";
    	fprintf(this->fp, "%s", record.c_str());
	rsize[0] += 1;
    }
    delete[] id_list;
  }
  id_list = NULL;
  _result_list.push_back(rsize);
}
