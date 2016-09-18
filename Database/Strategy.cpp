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
	this->vstree = NULL;
	//this->prepare_handler();
}

Strategy::Strategy(KVstore* _kvstore, VSTree* _vstree)
{
	this->method = 0;
	this->kvstore = _kvstore;
	this->vstree = _vstree;
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
//however, this can be dealed due to several basicquery and linking

bool
Strategy::handle(SPARQLquery& _query)
{
#ifdef MULTI_INDEX
	Util::logging("IN GeneralEvaluation::handle");

	vector<BasicQuery*>& queryList = _query.getBasicQueryVec();
	// enumerate each BasicQuery and retrieve their variables' mapping entity in the VSTree.
	vector<BasicQuery*>::iterator iter = queryList.begin();
	for (; iter != queryList.end(); iter++)
	{
		this->method = 0;

		vector<int*>& result_list = (*iter)->getResultList();
		//int select_var_num = (*iter)->getSelectVarNum();
		int varNum = (*iter)->getVarNum();  //the num of vars needing to be joined
		int total_num = (*iter)->getTotalVarNum();
		int pre_varNum = (*iter)->getPreVarNum();

		if ((*iter)->getTripleNum() == 1 && pre_varNum == 1)
		{
			Triple triple = (*iter)->getTriple(0);
			int* id_list = NULL;
			int id_list_len = 0;
			result_list.clear();

			if (total_num == 2)
			{
				//TODO:consider special case, select ?s (?p) ?o where { ?s ?p ?o . }
				//filter and join is too costly, should enum all predicates and use p2so
				//maybe the selected vars are ?s (?p) or ?o (?p)
				cerr << "not supported now!" << endl;
			}
			else if (total_num == 1)
			{
				//TODO:if just select s/o, use o2s/s2o
				//if only p is selected, use s2p or o2p
				//only if both s/o and p are selected, use s2po or o2ps

				if (triple.subject[0] != '?')  //constant
				{
					int sid = (this->kvstore)->getIDByEntity(triple.subject);
					this->kvstore->getpreIDobjIDlistBysubID(sid, id_list, id_list_len);
				}
				else if (triple.object[0] != '?')  //constant
				{
					int oid = (this->kvstore)->getIDByEntity(triple.object);
					if (oid == -1)
					{
						oid = (this->kvstore)->getIDByLiteral(triple.object);
					}
					this->kvstore->getpreIDsubIDlistByobjID(oid, id_list, id_list_len);
				}

				//always place s/o before p in result list
				for (int i = 0; i < id_list_len; i += 2)
				{
					int* record = new int[2];    //2 vars selected
					record[1] = id_list[i];      //for the pre var
					record[0] = id_list[i + 1]; //for the s/o var
					result_list.push_back(record);
				}
			}
			else if (total_num == 0)  //only ?p
			{
				//just use so2p
				int sid = (this->kvstore)->getIDByEntity(triple.subject);
				int oid = (this->kvstore)->getIDByEntity(triple.object);
				if (oid == -1)
				{
					oid = (this->kvstore)->getIDByLiteral(triple.object);
				}

				this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len);
				//copy to result list
				for (int i = 0; i < id_list_len; ++i)
				{
					int* record = new int[1];
					record[0] = id_list[i];
					result_list.push_back(record);
				}
			}

			delete[] id_list;
			continue;
		}

		if (pre_varNum == 0 && (*iter)->getTripleNum() == 1)    //only one triple and no predicates
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
			//cerr << "this BasicQuery use query strategy 2" << endl;
			//cerr<<"Final result size: "<<(*iter)->getResultList().size()<<endl;
			//continue;
		}

		//QueryHandler dispatch;
		//dispatch[0] = handler0;
		switch (this->method)
		{
		case 0:
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
		default:
			cerr << "not support this method" << endl;

		}
		cerr << "Final result size: " << (*iter)->getResultList().size() << endl;
		//BETTER: use function pointer array in C++ class
	}
#else
	cerr << "this BasicQuery use original query strategy" << endl;
	long tv_handle = Util::get_cur_time();
	(this->vstree)->retrieve(_query);
	long tv_retrieve = Util::get_cur_time();
	cout << "after Retrieve, used " << (tv_retrieve - tv_handle) << "ms." << endl;

	this->join = new Join(kvstore);
	this->join->join_sparql(_query);
	delete this->join;

	long tv_join = Util::get_cur_time();
	cout << "after Join, used " << (tv_join - tv_retrieve) << "ms." << endl;
#endif
	Util::logging("OUT Strategy::handle");
	return true;
}

bool
Strategy::handle(SPARQLquery& _query, int myRank, string &internal_tag_str)
{
#ifdef MULTI_INDEX
    Util::logging("IN GeneralEvaluation::handle");

    vector<BasicQuery*>& queryList = _query.getBasicQueryVec();
    // enumerate each BasicQuery and retrieve their variables' mapping entity in the VSTree.
    vector<BasicQuery*>::iterator iter=queryList.begin();
    for(; iter != queryList.end(); iter++)
    {
        this->method = 0;

        vector<int*>& result_list = (*iter)->getResultList();
        int select_var_num = (*iter)->getSelectVarNum();
        int varNum = (*iter)->getVarNum();  //the num of vars needing to be joined
        int total_num = (*iter)->getTotalVarNum();
        int pre_varNum = (*iter)->getPreVarNum();

		if((*iter)->getTripleNum() == 1 && pre_varNum == 1)
		{
			Triple triple = (*iter)->getTriple(0);
			int* id_list = NULL;
			int id_list_len = 0;
			result_list.clear();

			if(total_num == 2)
			{
				//TODO:consider special case, select ?s (?p) ?o where { ?s ?p ?o . }
				//filter and join is too costly, should enum all predicates and use p2so
				//maybe the selected vars are ?s (?p) or ?o (?p)
				cerr << "not supported now!" << endl;
			}	
			else if(total_num == 1)
			{
				//TODO:if just select s/o, use o2s/s2o
				//if only p is selected, use s2p or o2p
				//only if both s/o and p are selected, use s2po or o2ps

				if(triple.subject[0] != '?')  //constant
				{
					int sid = (this->kvstore)->getIDByEntity(triple.subject);
					this->kvstore->getpreIDobjIDlistBysubID(sid, id_list, id_list_len);
				}
				else if(triple.object[0] != '?')  //constant
				{
					int oid = (this->kvstore)->getIDByEntity(triple.object);
					if(oid == -1)
					{
						oid = (this->kvstore)->getIDByLiteral(triple.object);
					}
					this->kvstore->getpreIDsubIDlistByobjID(oid, id_list, id_list_len);
				}

				//always place s/o before p in result list
				for(int i = 0; i < id_list_len; i += 2)
				{
					int* record = new int[2];    //2 vars selected
					record[1] = id_list[i];      //for the pre var
					record[0] = id_list[i+1]; //for the s/o var
					result_list.push_back(record);
				}
			}
			else if(total_num == 0)  //only ?p
			{
				//just use so2p
				int sid = (this->kvstore)->getIDByEntity(triple.subject);
				int oid = (this->kvstore)->getIDByEntity(triple.object);
				if(oid == -1)
				{
					oid = (this->kvstore)->getIDByLiteral(triple.object);
				}

				this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len);
				//copy to result list
				for(int i = 0; i < id_list_len; ++i)
				{
					int* record = new int[1];
					record[0] = id_list[i];
					result_list.push_back(record);
				}
			}

			delete[] id_list;
			continue;
		}

        if(pre_varNum == 0 && (*iter)->getTripleNum() == 1)    //only one triple and no predicates
        {
            //only one variable and one triple: ?s pre obj or sub pre ?o
            if(total_num == 1)
            {
                this->method = 1;
            }
            //only two vars: ?s pre ?o
            else if(total_num == 2)
            {
                if(varNum == 1)   //the selected id should be 0
                {
                    this->method = 2;
                }
                else   //==2
                {
                    this->method = 3;
                }
            }
            //cerr << "this BasicQuery use query strategy 2" << endl;
            //cerr<<"Final result size: "<<(*iter)->getResultList().size()<<endl;
            //continue;
        }

        //QueryHandler dispatch;
        //dispatch[0] = handler0;
        switch(this->method)
        {
        case 0:
            this->handler0_0(*iter, result_list, internal_tag_str);
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
        default:
            cerr << "not support this method" << endl;

        }
        //cerr<<"Final result size: "<<(*iter)->getResultList().size()<<endl;
        //BETTER: use function pointer array in C++ class
    }
#else
    cerr << "this BasicQuery use original query strategy" << endl;
    long tv_handle = Util::get_cur_time();
    (this->vstree)->retrieve(_query);
    long tv_retrieve = Util::get_cur_time();
    cout << "after Retrieve, used " << (tv_retrieve - tv_handle) << "ms." << endl;

    this->join = new Join(kvstore);
    this->join->join_sparql(_query);
    delete this->join;

    long tv_join = Util::get_cur_time();
    cout << "after Join, used " << (tv_join - tv_retrieve) << "ms." << endl;
#endif
    Util::logging("OUT Strategy::handle");
    return true;
}

void
Strategy::handler0_0(BasicQuery* _bq, vector<int*>& _result_list, string &internal_tag_str)
{
	int star_flag = 0;
	if(star_flag == 0){
		_bq->setRetrievalTag();
	}
    long before_filter = Util::get_cur_time();
	
    long tv_handle = Util::get_cur_time();
    int varNum = _bq->getVarNum();  //the num of vars needing to be joined
    for(int i = 0; i < varNum; ++i)
    {
		if(_bq->if_need_retrieve(i) == false)
			continue;
        bool flag = _bq->isLiteralVariable(i);
        const EntityBitSet& entityBitSet = _bq->getVarBitSet(i);
        IDList* idListPtr = &( _bq->getCandidateList(i) );
        this->vstree->retrieveEntity(entityBitSet, idListPtr);
		if(!flag)
		{
			_bq->setReady(i);
		}
        //the basic query should end if one non-literal var has no candidates
        if(idListPtr->size() == 0 && !flag)
        {
            //break;
        }
    }
	
    long tv_retrieve = Util::get_cur_time();
    //printf("join_pe !!!! \n");
    Join *join = new Join(kvstore);
	if(star_flag == 1){
		join->join_basic(_bq);
	}else{
		join->join_pe(_bq, internal_tag_str);
	}
    delete join;

    long tv_join = Util::get_cur_time();
    //cout << "after Join, used " << (tv_join - tv_retrieve) << "ms." << endl;
}

void
Strategy::handler0(BasicQuery* _bq, vector<int*>& _result_list)
{
	//long before_filter = Util::get_cur_time();
	cerr << "this BasicQuery use query strategy 0" << endl;

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
	for (int i = 0; i < varNum; ++i)
	{
		if (_bq->if_need_retrieve(i) == false)
			continue;
		bool flag = _bq->isLiteralVariable(i);
		const EntityBitSet& entityBitSet = _bq->getVarBitSet(i);
		IDList* idListPtr = &(_bq->getCandidateList(i));
		this->vstree->retrieveEntity(entityBitSet, idListPtr);
		if (!flag)
		{
			_bq->setReady(i);
		}
		//the basic query should end if one non-literal var has no candidates
		if (idListPtr->size() == 0 && !flag)
		{
			break;
		}
	}

	//if(_bq->isReady(0))
	//cout<<"error: var 0 is ready?"<<endl;
	//TODO:end directly if one is empty!

	long tv_retrieve = Util::get_cur_time();
	cout << "after Retrieve, used " << (tv_retrieve - tv_handle) << "ms." << endl;
	Join *join = new Join(kvstore);
	join->join_basic(_bq);
	delete join;

	long tv_join = Util::get_cur_time();
	cout << "after Join, used " << (tv_join - tv_retrieve) << "ms." << endl;
}

void
Strategy::handler1(BasicQuery* _bq, vector<int*>& _result_list)
{
	long before_filter = Util::get_cur_time();
	cerr << "this BasicQuery use query strategy 1" << endl;
	//int neighbor_id = (*_bq->getEdgeNeighborID(0, 0);  //constant, -1
	char edge_type = _bq->getEdgeType(0, 0);
	int triple_id = _bq->getEdgeID(0, 0);
	Triple triple = _bq->getTriple(triple_id);
	int pre_id = _bq->getEdgePreID(0, 0);
	int* id_list = NULL;
	int id_list_len = 0;
	if (edge_type == Util::EDGE_OUT)
	{
		//cerr<<"edge out!!!"<<endl;
		int nid = (this->kvstore)->getIDByEntity(triple.object);
		if (nid == -1)
		{
			nid = (this->kvstore)->getIDByLiteral(triple.object);
		}
		this->kvstore->getsubIDlistByobjIDpreID(nid, pre_id, id_list, id_list_len);
	}
	else
	{
		//cerr<<"edge in!!!"<<endl;
		this->kvstore->getobjIDlistBysubIDpreID(this->kvstore->getIDByEntity(triple.subject), pre_id, id_list, id_list_len);
	}

	long after_filter = Util::get_cur_time();
	cerr << "after filter, used " << (after_filter - before_filter) << "ms" << endl;
	_result_list.clear();
	//cerr<<"now to copy result to list"<<endl;
	for (int i = 0; i < id_list_len; ++i)
	{
		int* record = new int[1];    //only this var is selected
		record[0] = id_list[i];
		//cerr<<this->kvstore->getEntityByID(record[0])<<endl;
		_result_list.push_back(record);
	}
	long after_copy = Util::get_cur_time();
	cerr << "after copy to result list: used " << (after_copy - after_filter) << " ms" << endl;
	delete[] id_list;
	cerr << "Final result size: " << _result_list.size() << endl;
}

void
Strategy::handler2(BasicQuery* _bq, vector<int*>& _result_list)
{
	long before_filter = Util::get_cur_time();
	cerr << "this BasicQuery use query strategy 2" << endl;
	int triple_id = _bq->getEdgeID(0, 0);
	Triple triple = _bq->getTriple(triple_id);
	int pre_id = _bq->getEdgePreID(0, 0);
	int var1_id = _bq->getIDByVarName(triple.subject);
	int var2_id = _bq->getIDByVarName(triple.object);
	int* id_list = NULL;
	int id_list_len = 0;
	if (var1_id == 0)   //subject var selected
	{
		//use p2s directly
		this->kvstore->getsubIDlistBypreID(pre_id, id_list, id_list_len);
	}
	else if (var2_id == 0) //object var selected
	{
		//use p2o directly
		this->kvstore->getobjIDlistBypreID(pre_id, id_list, id_list_len);
	}
	else
	{
		cerr << "ERROR in Database::handle(): no selected var!" << endl;
	}
	long after_filter = Util::get_cur_time();
	cerr << "after filter, used " << (after_filter - before_filter) << "ms" << endl;
	_result_list.clear();
	for (int i = 0; i < id_list_len; ++i)
	{
		int* record = new int[1];    //only one var
		record[0] = id_list[i];
		_result_list.push_back(record);
	}
	long after_copy = Util::get_cur_time();
	cerr << "after copy to result list: used " << (after_copy - after_filter) << " ms" << endl;
	delete[] id_list;
	cerr << "Final result size: " << _result_list.size() << endl;
}

void
Strategy::handler3(BasicQuery* _bq, vector<int*>& _result_list)
{
	long before_filter = Util::get_cur_time();
	cerr << "this BasicQuery use query strategy 3" << endl;
	int triple_id = _bq->getEdgeID(0, 0);
	Triple triple = _bq->getTriple(triple_id);
	int pre_id = _bq->getEdgePreID(0, 0);
	int* id_list = NULL;
	int id_list_len = 0;
	this->kvstore->getsubIDobjIDlistBypreID(pre_id, id_list, id_list_len);
	int var1_id = _bq->getIDByVarName(triple.subject);
	int var2_id = _bq->getIDByVarName(triple.object);
	long after_filter = Util::get_cur_time();
	cerr << "after filter, used " << (after_filter - before_filter) << "ms" << endl;
	_result_list.clear();
	for (int i = 0; i < id_list_len; i += 2)
	{
		int* record = new int[2];    //2 vars and selected
		record[var1_id] = id_list[i];
		record[var2_id] = id_list[i + 1];
		_result_list.push_back(record);
	}
	long after_copy = Util::get_cur_time();
	cerr << "after copy to result list: used " << (after_copy - after_filter) << " ms" << endl;
	delete[] id_list;
	cerr << "Final result size: " << _result_list.size() << endl;
}