/*=============================================================================
# Filename: BasicQuery.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-31 19:21
# Description: 
=============================================================================*/

#include "BasicQuery.h"

using namespace std;

// _query is a SPARQL query string
BasicQuery::BasicQuery(const string _query)
{
	this->initial();
}

BasicQuery::~BasicQuery()
{
    this->clear();
}

void 
BasicQuery::clear()
{
    delete[] this->var_degree;
    delete[] this->var_sig;
    delete[] this->var_name;
    delete[] this->link_with_constant;

    for(int i = 0; i < BasicQuery::MAX_VAR_NUM; i ++)
    {
        //delete[] this->edge_sig[i];
        delete[] this->edge_id[i];
        delete[] this->edge_nei_id[i];
        delete[] this->edge_pre_id[i];
        delete[] this->edge_type[i];
    }

    //delete[] this->edge_sig;
    delete[] this->edge_id;
    delete[] this->edge_nei_id;
    delete[] this->edge_pre_id;
    delete[] this->edge_type;

    this->var_degree = NULL;
    this->var_sig = NULL;
    this->var_name = NULL;

    //this->edge_sig = NULL;
    this->edge_id = NULL;
    this->edge_nei_id = NULL;
    this->edge_pre_id = NULL;
    this->edge_type = NULL;

    delete[] this->candidate_list;
    this->candidate_list = NULL;
    //delete[] this->is_literal_candidate_added;
    //this->is_literal_candidate_added = NULL;
	delete[] this->ready;
	this->ready = NULL;
	delete[] this->need_retrieve;
	this->need_retrieve = NULL;
    for (unsigned i=0;i<this->result_list.size();++i)
    {
        delete[] this->result_list[i];
        this->result_list[i] = NULL;
    }    
}

int
BasicQuery::getRetrievedVarNum()
{
	return this->retrieve_var_num;
}

int
BasicQuery::getTotalVarNum()
{
	return this->total_var_num;
}

// get the num of vars need to be joined(>= select_var_num)
int 
BasicQuery::getVarNum()
{
    return this->graph_var_num;
}

// only the num of vars which are in select, means to be in result
int 
BasicQuery::getSelectVarNum()
{
    return this->select_var_num;
}

// all vars begin with '?'
string 
BasicQuery::getVarName(int _var)
{
    return this->var_name[_var];
}

// get triples number, also sentences number
unsigned
BasicQuery::getTripleNum()
{
    return this->triple_vt.size();
}

// get the ID of the i-th edge of _var
int BasicQuery::getEdgeID(int _var, int _i_th_edge)
{
    return this->edge_id[_var][_i_th_edge];
}

// get the ID of the i-th edge neighbor of _var
int BasicQuery::getEdgeNeighborID(int _var, int _i_th_edge)
{
    return this->edge_nei_id[_var][_i_th_edge];
}

// get the ID of the i-th edge of _var
TYPE_PREDICATE_ID
BasicQuery::getEdgePreID(int _var, int _i_th_edge)
{
    return this->edge_pre_id[_var][_i_th_edge];
}

// get the ID of the i-th edge of _var
char BasicQuery::getEdgeType(int _var, int _i_th_edge)
{
    return this->edge_type[_var][_i_th_edge];
}

// get the degree of _var in the query graph 
int
BasicQuery::getVarDegree(int _var)
{
    return this->var_degree[_var];
}

//get the index of edge between two var ids
//-1 if no edge, the index is based on _id0
vector<int>
BasicQuery::getEdgeIndex(int _id0, int _id)
{
	//BETTER:this a brute-force method, using mapping if needed
	//(though the query graph is small generally)
	//is there any way to pre-compute all EdgeIndex relationship
	
	vector<int> result;
	for(int i = 0; i < this->var_degree[_id0]; ++i)
	{
		if(this->edge_nei_id[_id0][i] == _id)
		{
			//NOTICE:maybe exist parallel edges i.e. 
			//?s ?p1 ?o.  ?s ?p2 ?o.
			//?s p1 ?o. ?s p2 ?o.
			//return i;

			//if(this->getEdgePreID(_id0, i) < 0)
			//{
				//continue;
			//}
			result.push_back(i);
		}
	}
	
	//return -1;
	return result;
}

//get the candidate list of _var in the query graph 
IDList& 
BasicQuery::getCandidateList(int _var)
{
    return candidate_list[_var];
}

unsigned
BasicQuery::getCandidateSize(int _var)
{
    return this->candidate_list[_var].size();
}

// get the result list of _var in the query graph
vector<unsigned*>& 
BasicQuery::getResultList()
{
    return result_list;
}

vector<unsigned*>*
BasicQuery::getResultListPointer()
{
    return &result_list;
}

const EntityBitSet& 
BasicQuery::getVarBitSet(int _i)const
{
    // check whether _i exceeds will be better
    return (this->var_sig[_i]);
}

// check whether the i-th edge of _var is IN edge
bool 
BasicQuery::isInEdge(int _var, int _i_th_edge)const
{
    return this->edge_type[_var][_i_th_edge] == Util::EDGE_IN;
}

// check whether the i-th edge of _var is OUT edge
bool 
BasicQuery::isOutEdge(int _var, int _i_th_edge)const
{
    return this->edge_type[_var][_i_th_edge] == Util::EDGE_OUT;
}

bool 
BasicQuery::isOneDegreeVar(string& _no_sense_var)
{
    // vars begin with ?
    if(_no_sense_var.at(0) != '?')
    {
        return false;
    }
    // freq equals 1
    if(this->tuple2freq[_no_sense_var] != 1)
    {
        return false;
    }
    return true;
}

bool 
BasicQuery::isOneDegreeNotJoinVar(string& _no_sense_var)
{
    // vars begin with ?
    if(_no_sense_var.at(0) != '?')
    {
        return false;
    }
    // freq equals 1
    if(this->tuple2freq[_no_sense_var] != 1)
    {
        return false;
    }
	//ERROR:the value returned is 0
	//if(this->var_str2id[_no_sense_var] != -1)
	if(this->var_str2id.find(_no_sense_var) != this->var_str2id.end())
	{
		return false;
	}

    return true;
}

// this var only exist in objects in the query graph, then literal
// candidates are allowed(of course entities are also ok!)
// However, vstree only produce the entity candidates.
// Then we have to add literals before join or in join
bool 
BasicQuery::isLiteralVariable(int _var)
{
    int var_degree = this->getVarDegree(_var);

    for (int i = 0; i < var_degree; i++)
    {
        if (this->isOutEdge(_var, i))
        {
            return false;
        }
    }

    return true;
}

bool 
BasicQuery::isFreeLiteralVariable(int _var)
{
    if (!this->isLiteralVariable(_var))
    {
        return false;
    }

    int neighbor_num = this->var_degree[_var];
    for(int i = 0; i < neighbor_num; i ++)
    {
		int triple_id = this->getEdgeID(_var, i);
		Triple triple = this->getTriple(triple_id);
		//NOTICE: the neighbor can also be a var whose id is -1
        if(triple.subject[0] != '?')
        {
            return false;
        }
    }

    return true;
}

//bool 
//BasicQuery::isAddedLiteralCandidate(int _var)
//{
    //return this->is_literal_candidate_added[_var];
//}

//void 
//BasicQuery::setAddedLiteralCandidate(int _var)
//{
    //this->is_literal_candidate_added[_var] = true;
//}

bool
BasicQuery::if_need_retrieve(int _var)
{
	return _var >= 0 && this->need_retrieve[_var];
}

bool
BasicQuery::isSatelliteInJoin(int _var)
{
	return _var >= 0 && _var < this->graph_var_num && !(this->need_retrieve[_var]);
}

bool
BasicQuery::isReady(int _var) const
{
	return _var >= 0 && _var < this->graph_var_num && this->ready[_var];
}

void
BasicQuery::setReady(int _var)
{
	if(_var >= 0 && _var < this->graph_var_num)
	{
		this->ready[_var] = true;
	}
}

void 
BasicQuery::updateSubSig(int _sub_var_id, TYPE_PREDICATE_ID _pre_id, int _line_id, int _obj_var_id)
{
	//cout<<"sub var id: "<<_sub_var_id<<endl;
    // update var(sub)_degree & edge_id according to this triple
    int sub_degree = this->var_degree[_sub_var_id];
    // edge_id[var_id][i] : the ID of the i-th edge of the var
    this->edge_id[_sub_var_id][sub_degree] = _line_id;
    this->edge_nei_id[_sub_var_id][sub_degree] = _obj_var_id;
    this->edge_type[_sub_var_id][sub_degree] = Util::EDGE_OUT;
    this->edge_pre_id[_sub_var_id][sub_degree] = _pre_id;
    this->var_degree[_sub_var_id] ++;
}

void 
BasicQuery::updateObjSig(int _obj_var_id, TYPE_PREDICATE_ID _pre_id, int _line_id, int _sub_var_id)
{
	//cout<<"obj var id: "<<_obj_var_id<<endl;
    // update var(sub)_degree & edge_id according to this triple
    int obj_degree = this->var_degree[_obj_var_id];
    // edge_id[var_id][i] : the ID of the i-th edge of the var 
    this->edge_id[_obj_var_id][obj_degree] = _line_id;
    this->edge_nei_id[_obj_var_id][obj_degree] = _sub_var_id;
    this->edge_type[_obj_var_id][obj_degree] = Util::EDGE_IN;
    this->edge_pre_id[_obj_var_id][obj_degree] = _pre_id;
    this->var_degree[_obj_var_id] ++;
}

/**
 * encode relative signature data of the query graph
 * two parts: predicate var id & entity id , both start from 0
 * @param _p_kvstore the KV Store instance to query the node id
 * @param _query_var
 * @return
 */
bool
BasicQuery::encodeBasicQuery(KVstore* _p_kvstore, const vector<string>& _query_var)
{
	//WARN:?p is ok to exist in both s/o or p position
	//TODO:return only the entity ID, not the predicate ID?
	//so the _query_var mix all the variables, not considering the order
    cout << "IN buildBasicSignature" << endl;
    //this->initial();
    //cout << "after init" << endl;

    this->buildTuple2Freq();

    // map id 2 var_name : this->var_name[]
    // map var_name 2 id : this->var_str2id

	//find all predicate vars
    for(unsigned i = 0; i < this->triple_vt.size(); i ++)
    {
        string& pre = this->triple_vt[i].predicate;
		if(pre[0] != '?')
		{
			continue;
		}

        string& sub = this->triple_vt[i].subject;
        string& obj = this->triple_vt[i].object;
		int pid = this->getPreVarID(pre);
		if(pid == -1)
		{
			this->pre_var.push_back(PreVar(pre));
			pid = this->pre_var.size() - 1;
		}
		//map<int, PreVar>::iterator it = this->pre_var.find(pid);
		//it->second.triples.push_back(i);
		this->pre_var[pid].triples.push_back(i);
	}

	//NOTICE: we append the candidates for selected pre_var to original select_var_num columns
    this->select_var_num = this->selected_pre_var_num = 0;
	cout<<"now to check the query var list order:"<<endl;



//	I want to see what is _query_var
	for(int i = 0; i < _query_var.size(); ++i){
		cout << "_query_var[" << i << "] = " << _query_var[i] << endl;
	}


	for(unsigned i = 0; i < _query_var.size(); ++i)
	{
		//NOTICE:not place pre var in join
		string var = _query_var[i];
//		cout<<i<<" "<<var<<endl;
		int pid = this->getPreVarID(var);
		if(pid == -1) // not pre var
		{
			this->selected_var_set.insert(var);
			//assign id for this var
			int id = this->select_var_num;
			this->selected_var_position[id] = i;
			this->var_str2id[var] = id;
			this->var_name[id] = var;
			this->select_var_num++;
			continue;
		}
        // is pre var
		this->selected_var_position[-1-pid] = i;
		this->pre_var[pid].selected = true;
		this->selected_pre_var_num++;
	}

#ifdef DEBUG
    stringstream _ss;
    _ss << "select_var_num=" << this->select_var_num << endl;
    Util::logging(_ss.str());
#endif

    cout << "select variables: ";
    for(unsigned i = 0; i < this->var_str2id.size(); ++i)
    {
        cout << "[" << this->var_name[i] << ", " << i << " " << this->var_str2id[this->var_name[i]] << "]\t";
    }
    cout << endl;
	//BETTER:ouput the selected pre vars

	this->total_var_num = this->select_var_num;
	this->addInVarNotInSelect();

    // assign the this->var_num, all need to join
    // (var_str2id is modified)
    this->graph_var_num = this->var_str2id.size();

    cout<< "graph variables: ";
    for(unsigned i = 0; i < this->var_str2id.size(); i ++)
    {
        cout << "[" << this->var_name[i] << ", " << i << " " <<  this->var_str2id[this->var_name[i]] << "]\t";
    }
    cout << endl;

    this->candidate_list = new IDList[this->graph_var_num];

    for(unsigned i = 0; i < this->triple_vt.size(); i ++)
    {
        string& sub = this->triple_vt[i].subject;
        string& pre = this->triple_vt[i].predicate;
        string& obj = this->triple_vt[i].object;

//		cout << endl << sub << "  " << pre << "  " << obj << endl;


		//int pre_id = -1;    //not found
		TYPE_PREDICATE_ID pre_id = INVALID_PREDICATE_ID;    //not found
		if(pre[0] == '?')   //pre var
		{
			pre_id = -2;   //mark that this is a pre var
		}
		else
		{
			// -1 if not found, this means this query is invalid
			pre_id = _p_kvstore->getIDByPredicate(pre);
			{
				//stringstream _ss;
				//_ss << "pre2id: " << pre << "=>" << pre_id << endl;
				//Util::logging(_ss.str());
			}
		}
		if(pre_id == -1)
		{
			cout << "invalid query because the pre is not found: " << pre << endl;
			return false;
		}

        int sub_var_id = -1;
        int obj_var_id = -1;
        // -1 if not found, this means this subject is a constant
        map<string, int>::iterator _find_sub_itr = (this->var_str2id).find(sub);
        if(_find_sub_itr != this->var_str2id.end())
        {
            sub_var_id = _find_sub_itr->second;
        }

        // -1 if not found, this means this object is a constant(string)
        map<string, int>::iterator _find_obj_itr = (this->var_str2id).find(obj);
        if(_find_obj_itr != this->var_str2id.end())
        {
            obj_var_id = _find_obj_itr->second;
        }

        // sub is either a var or a string
        bool sub_is_var = (sub_var_id != -1);
        if(sub_is_var)
        {
			//int obj_id = -1;
			TYPE_ENTITY_LITERAL_ID obj_id = INVALID_ENTITY_LITERAL_ID;
			if(obj.at(0) != '?')
			{
				this->link_with_constant[sub_var_id] = true;
			}
			this->updateSubSig(sub_var_id, pre_id, i, obj_var_id);
			//cout<<"to update sub: "<<sub<<endl<<sub_var_id<<" "<<pre_id<<" "<<obj_id<<" "<<obj<<endl<<obj_var_id<<endl;
        }

        // obj is either a var or a string
        bool obj_is_var = (obj_var_id != -1);
        if(obj_is_var)
        {
			//int sub_id = -1;
			TYPE_ENTITY_LITERAL_ID sub_id = INVALID_ENTITY_LITERAL_ID;
			if(sub.at(0) != '?')
			{
				this->link_with_constant[obj_var_id] = true;
			}
            this->updateObjSig(obj_var_id, pre_id, i, sub_var_id);
        }
    }

#ifdef DEBUG
	//cout<<"yy: "<<Signature::BitSet2str(this->var_sig[2])<<endl;
	//cout<<this->var_name[2]<<endl;
#endif

	//set need_retrieve for vars in join whose total degree > 1
	this->retrieve_var_num = 0;
	for(int i = 0; i < this->graph_var_num; ++i)
	{
		if(this->var_degree[i] > 1)
		{
			this->need_retrieve[i] = true;
			this->retrieve_var_num++;
		}
	}

    cout << "OUT encodeBasicQuery" << endl;
    this->encode_result = true;

    for(int i = 0; i < var_str2id.size(); ++i) {
    	if(this->link_with_constant[i]){
    		cout << "var[" << i <<"]: " << var_name[i] <<" is linked with constant: " << "true" << endl;
    	} else{
			cout << "var[" << i <<"]: " << var_name[i] <<" is linked with constant: " << "false" << endl;
		}
    }
	return true;
}

int 
BasicQuery::getSelectedVarPosition(int _var)
{
	map<int, int>::iterator it = this->selected_var_position.find(_var);
	if(it == this->selected_var_position.end())
	{
		return -1;
	}
	else
	{
		return it->second;
	}
}

int 
BasicQuery::getSelectedPreVarPosition(int _var)
{
	//change var to -1-var to avoid conflicting with normal var
	map<int, int>::iterator it = this->selected_var_position.find(-1-_var);
	if(it == this->selected_var_position.end())
	{
		return -1;
	}
	else
	{
		return it->second;
	}
}

int 
BasicQuery::getSelectedVarPosition(string _var)
{
	int id = this->getIDByVarName(_var);
	if(id < 0)
	{
		return -1;
	}
	map<int, int>::iterator it = this->selected_var_position.find(id);
	if(it == this->selected_var_position.end())
	{
		return -1;
	}
	else
	{
		return it->second;
	}
}

int 
BasicQuery::getSelectedPreVarPosition(string _var)
{
	int id = this->getPreVarID(_var);
	if(id < 0)
	{
		return -1;
	}
	//change var to -1-var to avoid conflicting with normal var
	map<int, int>::iterator it = this->selected_var_position.find(-1-id);
	if(it == this->selected_var_position.end())
	{
		return -1;
	}
	else
	{
		return it->second;
	}
}

bool
BasicQuery::getEncodeBasicQueryResult() const
{
	return this->encode_result;
}

int
BasicQuery::getIDByVarName(const string& _name)
{
	map<string,int>::iterator it = this->var_str2id.find(_name);
	if(it != this->var_str2id.end())
	{
		return it->second;
	}
	else
	{
		//WARN:if return var_str2id[_name] directly, this will be 0
		//maybe conflict with valid ID 0
		return -1;
	}
}

bool 
BasicQuery::isVarSelected(const string& _name) const
{
	set<string>::iterator it = this->selected_var_set.find(_name);
	if(it == this->selected_var_set.end())
	{
		return false;
	}
	else
	{
		return true;
	}
}

int
BasicQuery::getPreVarID(const string& _name) const
{
	//map<int, PreVar>::const_iterator it;   //this is a const function
	//for(it = this->pre_var.begin(); it != this->pre_var.end(); ++it)
		//if(it->second.name == _name)
			//return it->first;
	for(unsigned i = 0; i < this->pre_var.size(); ++i)
		if(this->pre_var[i].name == _name)
			return i;
	return -1;
	//BETTER:use string2int map to speed up if using in too-many-loops
}

unsigned
BasicQuery::getPreVarNum() const
{
	return this->pre_var.size();
}

const PreVar&
BasicQuery::getPreVarByID(unsigned _id) const
{
	if(_id >= 0 && _id < this->pre_var.size())
		return this->pre_var[_id];
	else
		return this->pre_var[0];
}
unsigned 
BasicQuery::getSelectedPreVarNum() const
{
	return this->selected_pre_var_num;
}

bool 
BasicQuery::isPreVarSelected(unsigned _pid) const
{
	//BETTER:check if the pid is valid
	return this->pre_var[_pid].selected;
}

void 
BasicQuery::addTriple(const Triple& _triple)
{
    triple_vt.push_back(_triple);
}


const Triple& 
BasicQuery::getTriple(int _i_th_triple)
{
    return triple_vt.at(_i_th_triple);
}


void 
BasicQuery::initial()
{
	//initial
    this->encode_result = false;
    this->graph_var_num = 0;
    this->var_degree = new int[BasicQuery::MAX_VAR_NUM];
    this->var_sig = new EntityBitSet[BasicQuery::MAX_VAR_NUM];
    this->var_name = new string[BasicQuery::MAX_VAR_NUM];
    this->link_with_constant = new bool[BasicQuery::MAX_VAR_NUM];

    //this->edge_sig = new EdgeBitSet*[BasicQuery::MAX_VAR_NUM];
    this->edge_id = new int*[BasicQuery::MAX_VAR_NUM];
    this->edge_nei_id = new int*[BasicQuery::MAX_VAR_NUM];
    this->edge_pre_id = new int*[BasicQuery::MAX_VAR_NUM];
    this->edge_type = new char*[BasicQuery::MAX_VAR_NUM];

    //this->is_literal_candidate_added = new bool[BasicQuery::MAX_VAR_NUM];
    this->ready = new bool[BasicQuery::MAX_VAR_NUM];
    this->need_retrieve = new bool[BasicQuery::MAX_VAR_NUM];

    for(int i = 0; i < BasicQuery::MAX_VAR_NUM; ++i)
    {
        this->var_degree[i] = 0;
        this->var_sig[i].reset();
        this->var_name[i] = "";
        this->link_with_constant[i] = false;
        //this->is_literal_candidate_added[i] = false;
		this->ready[i] = false;
        this->need_retrieve[i] = false;

        //this->edge_sig[i] = new EdgeBitSet[BasicQuery::MAX_VAR_NUM];
        this->edge_id[i] = new int[BasicQuery::MAX_VAR_NUM];
        this->edge_nei_id[i] = new int[BasicQuery::MAX_VAR_NUM];
        this->edge_pre_id[i] = new int[BasicQuery::MAX_VAR_NUM];
        this->edge_type[i] = new char[BasicQuery::MAX_VAR_NUM];

        for(int j = 0; j < BasicQuery::MAX_VAR_NUM; ++j)
        {
            //this->edge_sig[i][j].reset();
            this->edge_id[i][j] = -1;
            this->edge_nei_id[i][j] = -1;
            this->edge_pre_id[i][j] = -1;
            this->edge_type[i][j] = '\0';
        }
    }
}

// only add those who act as bridge in query graph
// so, by now we think only those not in select but occurring more 
// than once need to be added in var set
void BasicQuery::addInVarNotInSelect()
{
    // all vars in this set is met before at least once
    int _v_n_i_s_next_id = this->var_str2id.size() + 0;
    for(unsigned i = 0; i < this->triple_vt.size(); i++)
    {
        string& sub = this->triple_vt[i].subject;
        if(sub.at(0) == '?')
        {
            map<string, int>::iterator find_sub_itr = this->var_str2id.find(sub);
            bool not_var_yet = (find_sub_itr == this->var_str2id.end());
            if(not_var_yet)
            {
				this->total_var_num++;
                int _freq = this->tuple2freq[sub];
                // so the var str must occur more than once
                if(_freq > 1)
                {
                    this->var_str2id[sub] = _v_n_i_s_next_id;
                    this->var_name[_v_n_i_s_next_id] = sub;
                    _v_n_i_s_next_id++;
                }
            }
        }

        string& obj = this->triple_vt[i].object;
        if(obj.at(0) == '?')
        {
            map<string, int>::iterator find_obj_itr = this->var_str2id.find(obj);
            bool not_var_yet = (find_obj_itr == this->var_str2id.end());
            if(not_var_yet)
            {
				this->total_var_num++;
                int _freq = this->tuple2freq[obj];
                // so the var str must occur more than once
                if(_freq > 1)
                {
                    this->var_str2id[obj] = _v_n_i_s_next_id;
                    this->var_name[_v_n_i_s_next_id] = obj;
                    _v_n_i_s_next_id++;
                }
            }
        }
    }
}


void 
BasicQuery::buildTuple2Freq()
{
    vector<Triple>::iterator itr = this->triple_vt.begin();
    bool not_found = false;
    int _freq = 0;
    while(itr != this->triple_vt.end())
    {
        Triple& _t = *itr;
        // sub tuple
        not_found = (this->tuple2freq.find(_t.subject) == this->tuple2freq.end());
        if(not_found)
        {
            this->tuple2freq[_t.subject] = 1;
        }
        else
        {
            _freq = this->tuple2freq[_t.subject];
            this->tuple2freq[_t.subject] = _freq + 1;
        }
        // pre tuple
        not_found = (this->tuple2freq.find(_t.predicate) == this->tuple2freq.end());
        if(not_found)
        {
            this->tuple2freq[_t.predicate] = 1;
        }
        else
        {
            _freq = this->tuple2freq[_t.predicate];
            this->tuple2freq[_t.predicate] = _freq + 1;
        }

        // obj tuple
        not_found = (this->tuple2freq.find(_t.object) == this->tuple2freq.end());
        if(not_found)
        {
            this->tuple2freq[_t.object] = 1;
        }
        else
        {
            _freq = this->tuple2freq[_t.object];
            this->tuple2freq[_t.object] = _freq + 1;
        }
        itr++;
    }
}

void 
BasicQuery::print(ostream& _out_stream)
{
    _out_stream << this->triple_str() << endl;
    return;
}


int
BasicQuery::getVarID_MaxCandidateList()
{
    int max_var = -1;
    unsigned max_size = 0;
    for(int i = 0; i < this->graph_var_num; i ++)
    {
        unsigned tmp_size = (this->candidate_list[i]).size();
        if(tmp_size > max_size)
        {
            max_var = i;
            max_size = tmp_size;
        }
    }

    return max_var;
}

int
BasicQuery::getVarID_FirstProcessWhenJoin()
{
    int min_var = -1;
    unsigned min_size = Util::TRIPLE_NUM_MAX;
    //int min_var2 = -1;
    //int min_size2 = Util::TRIPLE_NUM_MAX;
    for(int i = 0; i < this->graph_var_num; ++i)
    {
        // when join variables' mapping candidate list, we should start with entity variable.
        // since literal variables' candidate list may not include all literals.
        //if(this->isSatelliteInJoin(i))
		//if(this->isLiteralVariable(i) || this->isSatelliteInJoin(i))
		if(!this->isReady(i))
        {
			cout<<"var "<<i<<" is not ready!"<<endl;
            continue;
        }
		else
			cout<<"var "<<i<<" is ready!"<<endl;

		unsigned tmp_size = (this->candidate_list[i]).size();
		//if(this->isLiteralVariable(i))
		//{
			//if(tmp_size < min_size2)
			//{
				//min_var2 = i;
				//min_size2 = tmp_size;
			//}
		//}
		//else
		//{
		if(tmp_size < min_size)
		{
			min_var = i;
			min_size = tmp_size;
		}
		//}
    }

    if(min_var == -1)
    {
        // in this case, all core vertices are literal variables
		// we must generate candidates for satellites or use p2o to add literals for a core vertex
		// For example:
		// A->B<-C select A B C
        //return min_var2;
		return -1;
    }
    else
    {
        return min_var;
    }
}

string 
BasicQuery::candidate_str()
{
    stringstream _ss;

    _ss << "varNum: " << this->getVarNum() << endl;
    for(int i = 0; i < this->getVarNum(); i ++)
    {
        _ss << "[[" << i << ":" << this->getVarName(i) << "]]" << endl;
        _ss << "\t" << this->candidate_list[i].to_str() << endl;
    }
    return _ss.str();
}

string BasicQuery::result_str()
{
    stringstream _ss;

    _ss << "resultNum: " << this->result_list.size() << endl;
    _ss << Util::result_id_str(this->result_list, this->graph_var_num) << endl;

    return _ss.str();
}

string BasicQuery::triple_str()
{
    stringstream _ss;

    _ss<<"Triple num:"<<this->getTripleNum()<<endl;

    for (int i = 0; i < getTripleNum(); i++)
    {
        _ss<<(this->getTriple(i).toString())<<endl;
    }

    return _ss.str();
}


string BasicQuery::to_str()
{
    Util::logging("IN BasicQuery::to_str");
    stringstream _ss;

    _ss << "Triples: " << endl;
    for(unsigned i = 0; i < this->triple_vt.size(); i ++)
    {
        _ss << "\t" << this->triple_vt[i].toString() << endl;
    }
    _ss << "Vars: " << endl;
    for(int i = 0; i < this->graph_var_num; i ++)
    {
        _ss << "\t" << i << " : [name=" << this->var_name[i] << "]";
        _ss << " [degree=" << this->var_degree[i] << "]";
        _ss << " [sig=" << Signature::BitSet2str(this->var_sig[i]) << "]";
        _ss << endl << endl;
    }
    _ss << "Edge: " << endl;
    for(int i = 0; i < this->graph_var_num; i ++)
    {
        for(int j = 0; j < BasicQuery::MAX_VAR_NUM; j ++)
        {
            if(edge_type[i][j] == '\0') continue;

            if(edge_type[i][j] != '\0')
            {
                _ss << "[" << i << "][" << j << "]: type=" << edge_type[i][j] << "\t";
            }
            if(edge_nei_id[i][j] != -1)
            {
                _ss << "edge_nei_id=" << edge_nei_id[i][j] << "\t";
            }
            _ss << endl;
        }
        _ss << endl;
    }

    //for(int i = 0; i < this->graph_var_num; i ++)
    //{
        //for(int j = 0; j < BasicQuery::MAX_VAR_NUM; j ++)
        //{
            //if(edge_sig[i][j].count() != 0)
            //{
                //_ss << "pre_id=" << edge_pre_id[i][j] << "\t";
                //_ss << i << ":" << j << "\t" << edge_sig[i][j] << endl;
            //}
        //}
        //_ss << endl;
    //}

    Util::logging(_ss.str()); //debug

    Util::logging("OUT BasicQuery::to_str");

    return _ss.str();
}

