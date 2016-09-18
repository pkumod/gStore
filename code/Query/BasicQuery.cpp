/*=============================================================================
# Filename: BasicQuery.cpp
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-10-31 19:21
# Description: 
=============================================================================*/

#include "BasicQuery.h"
#include "../Database/Database.h"

using namespace std;

/* _query is a SPARQL query string */
BasicQuery::BasicQuery(const string _query)
{
	this->initial();
}

BasicQuery::~BasicQuery()
{
    this->clear();
}

void BasicQuery::clear()
{
    /* clear */
    delete[] this->var_degree;
    delete[] this->var_sig;
    delete[] this->var_name;

    for(int i = 0; i < BasicQuery::MAX_VAR_NUM; i ++)
    {
        delete[] this->edge_sig[i];
        delete[] this->edge_id[i];
        delete[] this->edge_nei_id[i];
        delete[] this->edge_pre_id[i];
        delete[] this->edge_type[i];
    }

    delete[] this->edge_sig;
    delete[] this->edge_id;
    delete[] this->edge_nei_id;
    delete[] this->edge_pre_id;
    delete[] this->edge_type;

    this->var_degree = NULL;
    this->var_sig = NULL;
    this->var_name = NULL;

    this->edge_sig = NULL;
    this->edge_id = NULL;
    this->edge_nei_id = NULL;
    this->edge_pre_id = NULL;
    this->edge_type = NULL;

    delete[] this->candidate_list;
    this->candidate_list = NULL;
    delete[] this->is_literal_candidate_added;
    this->is_literal_candidate_added = NULL;
    for (unsigned i=0;i<this->result_list.size();++i)
    {
        delete[] this->result_list[i];
        this->result_list[i] = NULL;
    }    
}

/* get the number of variables */
int BasicQuery::getVarNum()
{
    return this->graph_var_num;
}

/* get the name of _var in the query graph */
string BasicQuery::getVarName(int _var)
{
    return this->var_name[_var];
}

/* get triples number, also sentences number */
int BasicQuery::getTripleNum()
{
    return this->triple_vt.size();
}

/* get the ID of the i-th edge of _var */
int BasicQuery::getEdgeID(int _var, int _i_th_edge)
{
    return this->edge_id[_var][_i_th_edge];
}

/* get the ID of the i-th edge neighbor of _var */
int BasicQuery::getEdgeNeighborID(int _var, int _i_th_edge)
{
    return this->edge_nei_id[_var][_i_th_edge];
}

/* get the ID of the i-th edge of _var */
int BasicQuery::getEdgePreID(int _var, int _i_th_edge)
{
    return this->edge_pre_id[_var][_i_th_edge];
}

/* get the ID of the i-th edge of _var */
char BasicQuery::getEdgeType(int _var, int _i_th_edge)
{
    return this->edge_type[_var][_i_th_edge];
}

/* get the degree of _var in the query graph */
int BasicQuery::getVarDegree(int _var)
{
    return this->var_degree[_var];
}


/* get the candidate list of _var in the query graph */
IDList& BasicQuery::getCandidateList(int _var){
    return candidate_list[_var];
}

int BasicQuery::getCandidateSize(int _var)
{
    return this->candidate_list[_var].size();
}


/* get the result list of _var in the query graph */
vector<int*>& BasicQuery::getResultList(){
    return result_list;
}


const EntityBitSet& BasicQuery::getVarBitSet(int _i)const
{
    /* check whether _i exceeds will be better */
    return (this->var_sig[_i]);
}

/* check whether the i-th edge of _var is IN edge */
bool BasicQuery::isInEdge(int _var, int _i_th_edge)const
{
    return this->edge_type[_var][_i_th_edge] == BasicQuery::EDGE_IN;
}

/* check whether the i-th edge of _var is OUT edge */
bool BasicQuery::isOutEdge(int _var, int _i_th_edge)const
{
    return this->edge_type[_var][_i_th_edge] == BasicQuery::EDGE_OUT;
}

bool BasicQuery::isOneDegreeNotSelectVar(string& _no_sense_var)
{
    /* begin with ? */
    if(_no_sense_var.at(0) != '?')
    {
        return false;
    }

    /* freq equals 1 */
    if(this->tuple2freq[_no_sense_var] != 1)
    {
        return false;
    }

    return true;
}

bool BasicQuery::isLiteralVariable(int _var)
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

bool BasicQuery::isFreeLiteralVariable(int _var)
{
    if (!this->isLiteralVariable(_var))
    {
        return false;
    }

    int neighbor_num = this->var_degree[_var];
    for (int i = 0; i < neighbor_num; i ++)
    {
        if (this->edge_nei_id[_var][i] == -1)
        {
            return false;
        }
    }

    return true;
}

bool BasicQuery::isAddedLiteralCandidate(int _var)
{
    return this->is_literal_candidate_added[_var];
}

void BasicQuery::setAddedLiteralCandidate(int _var)
{
    this->is_literal_candidate_added[_var] = true;
}

void BasicQuery::updateSubSig(int _sub_id, int _pre_id, int _obj_id, string _obj,int _line_id)
{
    /* update var(sub)_signature according this triple */
    bool obj_is_str = (_obj_id == -1) && (_obj.at(0) != '?');
    if(obj_is_str)
    {
        Signature::encodeStr2Entity(_obj.c_str(), this->var_sig[_sub_id]);
    }

    if(_pre_id != -1)
    {
        Signature::encodePredicate2Entity(_pre_id, this->var_sig[_sub_id], BasicQuery::EDGE_OUT);
    }

    /* update var(sub)_degree & edge_id according to this triple */
    int sub_degree = this->var_degree[_sub_id];
    /* edge_id[var_id][i] : the ID of the i-th edge of the var */
    this->edge_id[_sub_id][sub_degree] = _line_id;
    this->edge_nei_id[_sub_id][sub_degree] = _obj_id;
    this->edge_type[_sub_id][sub_degree] = BasicQuery::EDGE_OUT;
    this->edge_pre_id[_sub_id][sub_degree] = _pre_id;
    this->var_degree[_sub_id] ++;
}

void BasicQuery::updateObjSig(int _obj_id, int _pre_id, int _sub_id, string _sub,int _line_id)
{
    /* update var(obj)_signature */
    bool sub_is_str = (_sub_id == -1) && (_sub.at(0) != '?');
    if(sub_is_str)
    {
        //cout << "str2entity" << endl;
        Signature::encodeStr2Entity(_sub.c_str(), this->var_sig[_obj_id]);
    }

    if(_pre_id != -1)
    {
        //cout << "pre2entity" << endl;
        Signature::encodePredicate2Entity(_pre_id, this->var_sig[_obj_id], BasicQuery::EDGE_IN);
    }

    /* update var(sub)_degree & edge_id according to this triple */
    int obj_degree = this->var_degree[_obj_id];
    /* edge_id[var_id][i] : the ID of the i-th edge of the var */
    this->edge_id[_obj_id][obj_degree] = _line_id;
    this->edge_nei_id[_obj_id][obj_degree] = _sub_id;
    this->edge_type[_obj_id][obj_degree] = BasicQuery::EDGE_IN;
    this->edge_pre_id[_obj_id][obj_degree] = _pre_id;
    this->var_degree[_obj_id] ++;
}

/* encode relative signature data of the query graph */
void BasicQuery::encodeBasicQuery(KVstore* _p_kvstore, const vector<string>& _query_var)
{
    //cout << "IN buildBasicSignature" << endl;
    /* initial */
    this->initial();


    //cout << "after init" << endl;

    this->buildTuple2Freq();

    /*
     * map id 2 var_name : this->var_name[]
     * map var_name 2 id : this->var_str2id
     *
     */
    this->select_var_num = _query_var.size();

    //debug
    stringstream _ss;
    _ss << "select_var_num=" << this->select_var_num << endl;
    Database::log(_ss.str());

    for(int i = 0; i < (this->select_var_num); i ++)
    {
        string _var = _query_var[i];
        this->var_str2id[_var] = i;
        this->var_name[i] = _var;
    }
/*
    cout << "select variables: ";
    for(unsigned i = 0; i < this->var_str2id.size(); i ++)
    {
        cout << "[" << this->var_name[i] << ", " << i << " " <<  this->var_str2id[this->var_name[i]] << "]\t";
    }
    cout << endl;
*/
    if(this->encode_method == BasicQuery::SELECT_VAR)
    {
        this->findVarNotInSelect();
    }
    else
    {
        this->addInVarNotInSelect();
    }
    /* assign the this->var_num */
    this->graph_var_num = this->var_str2id.size();
/*
    cout<< "graph variables: ";
    for(unsigned i = 0; i < this->var_str2id.size(); i ++)
    {
        cout << "[" << this->var_name[i] << ", " << i << " " <<  this->var_str2id[this->var_name[i]] << "]\t";
    }
    cout << endl;
	cout << "before new IDList!" << endl;	//just for debug
*/
    this->candidate_list = new IDList[this->graph_var_num];

    for(unsigned i = 0; i < this->triple_vt.size(); i ++)
    {
        string& sub = this->triple_vt[i].subject;
        string& pre = this->triple_vt[i].predicate;
        string& obj = this->triple_vt[i].object;
        int pre_id = _p_kvstore->getIDByPredicate(pre);
        {
            stringstream _ss;
            _ss << "pre2id: " << pre << "=>" << pre_id << endl;
            Database::log(_ss.str());
        }
        int sub_id = -1;
        int obj_id = -1;

        /* find var(sub) id */
        map<string, int>::iterator _find_sub_itr = (this->var_str2id).find(sub);
        if(_find_sub_itr != this->var_str2id.end())
        {
            sub_id = _find_sub_itr->second;
        }

        /* find var(obj) id */
        map<string, int>::iterator _find_obj_itr = (this->var_str2id).find(obj);
        if(_find_obj_itr != this->var_str2id.end())
        {
            obj_id = _find_obj_itr->second;
        }

        /* sub is either a var or a string */
        bool sub_is_var = (sub_id != -1);
        if(sub_is_var)
        {
            this->updateSubSig(sub_id, pre_id, obj_id, obj,i);

            //debug
            {
                stringstream _ss;
                _ss << "updateSubSig:\tsub:" << sub_id << "; pre:" << pre_id << "; obj:" << obj_id;
                _ss << "; [" << obj << "]";
                Database::log(_ss.str());
            }
        }

        /* obj is either a var or a string */
        bool obj_is_var = (obj_id != -1);
        if(obj_is_var)
        {
            this->updateObjSig(obj_id, pre_id, sub_id, sub,i);

            //debug
            {
                stringstream _ss;
                _ss << "updateObjSig:\tobj:" << obj_id << "; pre:" << pre_id << "; sub:" << sub_id;
                _ss << "; [" << sub << "]";
                Database::log(_ss.str());
            }
        }

        /* if both end points are variables */
        bool two_var_edge = (sub_is_var && obj_is_var);
        if(two_var_edge)
        {
            if(pre_id != -1)
            {
                //cout << "pre2edge" << endl;
                Signature::encodePredicate2Edge(pre_id, this->edge_sig[sub_id][obj_id]);
//              this->edge_pre_id[sub_id][obj_id] = pre_id;
            }
        }

    }
    //cout << "OUT encodeBasicQuery" << endl;
}


/* add triple */
void BasicQuery::addTriple(const Triple& _triple)
{
    triple_vt.push_back(_triple);
}


const Triple& BasicQuery::getTriple(int _i_th_triple)
{
    return triple_vt.at(_i_th_triple);
}

/* private methods */
void BasicQuery::null_initial()
{
    this->option_vs.clear();
    this->triple_vt.clear();
    this->var_str2id.clear();
    this->var_degree = NULL;
    this->is_literal_candidate_added = NULL;
    this->edge_id = NULL;
    this->edge_nei_id = NULL;
    this->edge_pre_id = NULL;
    this->edge_type = NULL;
    this->var_sig = NULL;
    this->edge_sig = NULL;
    this->encode_method = BasicQuery::NOT_JUST_SELECT;
    this->candidate_list = NULL;
    this->graph_var_num = 0;
    this->select_var_num = 0;
    this->var_name = 0;
}

void BasicQuery::initial()
{
    /* initial */
    this->graph_var_num = 0;
    this->var_degree = new int[BasicQuery::MAX_VAR_NUM];
    this->var_sig = new EntityBitSet[BasicQuery::MAX_VAR_NUM];
    this->var_name = new string[BasicQuery::MAX_VAR_NUM];

    this->edge_sig = new EdgeBitSet*[BasicQuery::MAX_VAR_NUM];
    this->edge_id = new int*[BasicQuery::MAX_VAR_NUM];
    this->edge_nei_id = new int*[BasicQuery::MAX_VAR_NUM];
    this->edge_pre_id = new int*[BasicQuery::MAX_VAR_NUM];
    this->edge_type = new char*[BasicQuery::MAX_VAR_NUM];

    this->is_literal_candidate_added = new bool[BasicQuery::MAX_VAR_NUM];

    for(int i = 0; i < BasicQuery::MAX_VAR_NUM; i ++)
    {
        this->var_degree[i] = 0;
        this->var_sig[i].reset();
        this->var_name[i] = "";
        this->is_literal_candidate_added[i] = false;

        this->edge_sig[i] = new EdgeBitSet[BasicQuery::MAX_VAR_NUM];
        this->edge_id[i] = new int[BasicQuery::MAX_VAR_NUM];
        this->edge_nei_id[i] = new int[BasicQuery::MAX_VAR_NUM];
        this->edge_pre_id[i] = new int[BasicQuery::MAX_VAR_NUM];
        this->edge_type[i] = new char[BasicQuery::MAX_VAR_NUM];

        for(int j = 0; j < BasicQuery::MAX_VAR_NUM; j ++)
        {
            this->edge_sig[i][j].reset();
            this->edge_id[i][j] = -1;
            this->edge_nei_id[i][j] = -1;
            this->edge_pre_id[i][j] = -1;
            this->edge_type[i][j] = '\0';
        }
    }
}

/* only add those who act as bridge in query graph
 * so, by now we think only those not in select but occurring more than once
 * need to be added in var set;
 * */
void BasicQuery::addInVarNotInSelect()
{
    /* all vars in this set is met before at least once */
    int _v_n_i_s_next_id = this->var_str2id.size() + 0;
    for(unsigned i = 0; i < this->triple_vt.size(); i ++)
    {
        string& sub = this->triple_vt[i].subject;
        if(sub.at(0) == '?')
        {
            map<string, int>::iterator find_sub_itr = this->var_str2id.find(sub);
            bool not_var_yet = (find_sub_itr == this->var_str2id.end());
            if(not_var_yet)
            {
                int _freq = this->tuple2freq[sub];
                /* so the var str must occur more than once */
                if(_freq > 1)
                {
                    this->var_str2id[sub] = _v_n_i_s_next_id;
                    this->var_name[_v_n_i_s_next_id] = sub;
                    _v_n_i_s_next_id ++;
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
                int _freq = this->tuple2freq[obj];
                /* so the var str must occur more than once */
                if(_freq > 1)
                {
                    this->var_str2id[obj] = _v_n_i_s_next_id;
                    this->var_name[_v_n_i_s_next_id] = obj;
                    _v_n_i_s_next_id ++;
                }
            }
        }
    }
}

/* map id 2 var_name : this->var_name[]
 * map var_name 2 id : this->var_str2id
 *
 * invalid, because var has two type: var_in_select  var_not_in_select
 */
void BasicQuery::findVarNotInSelect()
{
    int _v_n_i_s_next_id = this->var_str2id.size() + 0;
    for(unsigned i = 0; i < this->triple_vt.size(); i ++)
    {
        string& sub = this->triple_vt[i].subject;
        if(sub.at(0) == '?')
        {
            map<string, int>::iterator find_sub_itr = this->var_str2id.find(sub);
            if(find_sub_itr == this->var_str2id.end())
            {
                this->var_not_in_select[sub] = _v_n_i_s_next_id;
                this->var_name[_v_n_i_s_next_id] = sub;
                _v_n_i_s_next_id ++;
            }
        }

        string& obj = this->triple_vt[i].object;
        if(obj.at(0) == '?')
        {
            map<string, int>::iterator find_obj_itr = this->var_str2id.find(obj);
            if(find_obj_itr == this->var_str2id.end())
            {
                this->var_not_in_select[obj] = _v_n_i_s_next_id;
                this->var_name[_v_n_i_s_next_id] = obj;
                _v_n_i_s_next_id ++;
            }
        }
    }
}

void BasicQuery::buildTuple2Freq()
{
    vector<Triple>::iterator itr = this->triple_vt.begin();
    bool not_found = false;
    int _freq = 0;
    while(itr != this->triple_vt.end())
    {
        Triple& _t = *itr;
        /* sub tuple */
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
        /* pre tuple */
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

        /* obj tuple */
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

        itr ++;
    }
}

void BasicQuery::print(ostream& _out_stream){
    _out_stream << this->triple_str() << endl;
    return;
}


int BasicQuery::getVarID_MinCandidateList()
{
    int min_var = -1;
    int min_size = Database::TRIPLE_NUM_MAX;
    for(int i = 0; i < this->graph_var_num; i ++)
    {
        int tmp_size = (this->candidate_list[i]).size();
        if(tmp_size < min_size)
        {
            min_var = i;
            min_size = tmp_size;
        }
    }
    return min_var;
}

int BasicQuery::getVarID_MaxCandidateList()
{
    int max_var = -1;
    int max_size = -1;
    for(int i = 0; i < this->graph_var_num; i ++)
    {
        int tmp_size = (this->candidate_list[i]).size();
        if(tmp_size > max_size)
        {
            max_var = i;
            max_size = tmp_size;
        }
    }
    return max_var;
}

int BasicQuery::getVarID_FirstProcessWhenJoin()
{
    int min_var = -1;
    int min_size = Database::TRIPLE_NUM_MAX;
    for(int i = 0; i < this->graph_var_num; i ++)
    {
        // when join variables' mapping candidate list, we should start with entity variable.
        // since literal variables' candidate list may not include all literals.
        if (this->isLiteralVariable(i))
        {
            continue;
        }

        int tmp_size = (this->candidate_list[i]).size();
        if(tmp_size < min_size)
        {
            min_var = i;
            min_size = tmp_size;
        }
    }

    if (min_var == -1)
    {
        // in this case, all variables may include literal results.
        // then the join-step starting with any variables is ok.
        return 0;
    }
    else
    {
        return min_var;
    }
}

struct BasicQuery::ResultEqual
{
    int result_len;
    ResultEqual(int _l):result_len(_l){}
    bool operator() (int* const& a, int* const& b)
    {
        for (int i=0;i<result_len;++i)
        {
            if (a[i] != b[i]) return false;
        }
        return true;
    }
};

struct BasicQuery::ResultCmp
{
    int result_len;
    ResultCmp(int _l):result_len(_l){}
    bool operator() (int* const& a, int* const& b)
    {
        if (a[result_len] != b[result_len]) return (a[result_len] > b[result_len]);
        for (int i=0;i<result_len;++i)
        {
            if (a[i] != b[i]) return (a[i] < b[i]);
        }
        return true;
    }
};

bool BasicQuery::dupRemoval_invalidRemoval()
{
    cout << "IN dupRemoval_invalidRemoval" << endl;

    ResultCmp resCmp(this->graph_var_num);
    ResultEqual resEqual(this->select_var_num);

    sort(this->result_list.begin(), this->result_list.end(), resCmp);
    
    int dup_num = 0, valid_num = 0;
    int result_size = this->result_list.size();

    for (int i=0;i<result_size;++i)
    {
        if (this->result_list[i][this->graph_var_num] == -1)
        {
            delete[] this->result_list[i];
            this->result_list[i] = NULL;
            continue;
        }

        if (i+1==result_size || !resEqual(this->result_list[i], this->result_list[i+1])) // valid result
        {
            this->result_list[valid_num++] = this->result_list[i];
        }
        else
        {
            delete[] this->result_list[i];
            this->result_list[i] = NULL;
            dup_num++;
        }
    }
    this->result_list.resize(valid_num);
    
    cout << "dup_num: " << dup_num << endl;
    cout << "invalid_num: " << result_size - valid_num << endl;

    cout << "OUT dupRemoval_invalidRemoval" << endl;
    return true;
}

bool BasicQuery::dupRemoval()
{
    cout << "IN dupRemoval" << endl;

    ResultCmp resCmp(this->graph_var_num);
    ResultEqual resEqual(this->select_var_num);

    sort(this->result_list.begin(), this->result_list.end(), resCmp);
    
    int dup_num = 0, valid_num = 0;
    int result_size = this->result_list.size();

    for (int i=0;i<result_size;++i)
    {
        if (i+1==result_size || !resEqual(this->result_list[i], this->result_list[i+1])) // valid result
        {
            this->result_list[valid_num++] = this->result_list[i];
        }
        else
        {
            delete[] this->result_list[i];
            this->result_list[i] = NULL;
            dup_num++;
        }
    }
    this->result_list.resize(valid_num);
    
    cout << "dup_num: " << dup_num << endl;
    //cout << "invalid_num: " << result_size - valid_num << endl;

    cout << "OUT dupRemoval" << endl;
    return true;
}

// deprecated.
// int BasicQuery::cmp_result(const void* _a, const void* _b)
// {
//     int** pa = (int**)_a;
//     int** pb = (int**)_b;
//     for(int i = 0; i < BasicQuery::MAX_VAR_NUM; i ++)
//     {
//         if((*pa)[i] != (*pb)[i])
//         {
//             return (*pa)[i] - (*pb)[i];
//         }
//     }
//     return 0;
// }

/*
 * I think this function is inefficient and inferior, we should re-write it later. by hanshuo
 */
// this function is deprecated.
// bool BasicQuery::dupRemoval_invalidRemoval()
// {
//     int result_size = this->result_list.size();
//     int ** p_tmp = new int*[result_size];
//     for(int i = 0; i < result_size; i ++)
//     {
//         p_tmp[i] = new int[BasicQuery::MAX_VAR_NUM+1];
//         memset(p_tmp[i], 0, sizeof(int)*(BasicQuery::MAX_VAR_NUM+1));
//     }


//     cout << "before copy" << endl;
//     vector<int*>::iterator tmp_itr1 = this->result_list.begin();
//     int p_tmp_count = 0;
//     /* copy valid result into p_tmp */
//     while(tmp_itr1 != this->result_list.end())
//     {
//         /* invalidRemoval */
//         if((*tmp_itr1)[this->graph_var_num] != -1)
//         {
//             /* the invalid tagging bit is at [this->graph_var_num]
//              * while, the last result id is at [this->select_var_num-1]
//              * before where(included) we should delete duplication
//              *  */
//             memcpy(p_tmp[p_tmp_count], (*tmp_itr1), sizeof(int)*(this->select_var_num));
//             p_tmp_count ++;
//         }
//         tmp_itr1 ++;
//     }

//     cout << "before qsort" << endl;
//     /* sort p_tmp for future duplication removal */
//     qsort(p_tmp, p_tmp_count, sizeof(int**), BasicQuery::cmp_result);
//     cout << "before label" << endl;
//     int _cmp = -1;
//     int dup_num = 0;
//     for(int i = 1; i < p_tmp_count; i ++)
//     {
//         _cmp = BasicQuery::cmp_result( (void*)(p_tmp+(i-1)), (void*)(p_tmp+i) );
//         if(_cmp == 0)
//         {
//             /* set invalid */
//             p_tmp[i][BasicQuery::MAX_VAR_NUM] = -1;
//             dup_num ++;
//         }
//     }

//     vector<int*>::iterator tmp_itr2 = this->result_list.begin();

//     cout << "dup_num: " << dup_num << endl;
//     /* dupRemoval when re-assign valid ones back to result_list */
//     for(int i = 0; i < p_tmp_count; i ++)
//     {
//         if(p_tmp[i][BasicQuery::MAX_VAR_NUM] != -1)
//         {
//             memcpy(*tmp_itr2, p_tmp[i], sizeof(int)*this->graph_var_num);
//             tmp_itr2 ++;
//         }
//     }
//     this->result_list.erase(tmp_itr2, this->result_list.end());

//     cout << "before delete" << endl;
//     for(int i = 0; i < result_size; i ++)
//     {
//         delete[] (p_tmp[i]);
//     }
//     cout << "delete[] p_tmp" << endl;
//     delete[] p_tmp;

//     return true;
// }

string BasicQuery::candidate_str()
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
    for (int i=0;i<getTripleNum();i++)
    {
        _ss<<(this->getTriple(i).toString())<<endl;
    }
    return _ss.str();
}


string BasicQuery::to_str()
{
    Database::log("IN BasicQuery::to_str");
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

    for(int i = 0; i < this->graph_var_num; i ++)
    {
        for(int j = 0; j < BasicQuery::MAX_VAR_NUM; j ++)
        {
            if(edge_sig[i][j].count() != 0)
            {
                _ss << "pre_id=" << edge_pre_id[i][j] << "\t";
                _ss << i << ":" << j << "\t" << edge_sig[i][j] << endl;
            }
        }
        _ss << endl;
    }

    Database::log(_ss.str()); //debug

    Database::log("OUT BasicQuery::to_str");

    return _ss.str();
}

