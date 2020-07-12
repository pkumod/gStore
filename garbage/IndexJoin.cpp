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
  this->result_list = NULL;
}

Join::Join(KVstore* _kvstore)
{
  this->kvstore = _kvstore;
  this->result_list = NULL;
}

Join::~Join()
{
  //noting to do necessarily
}

void
Join::init(BasicQuery* _basic_query)
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
  this->dealed_triple = (bool*)calloc(triple_num, sizeof(bool));
  this->index_lists = NULL;
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
  while (this->mystack.empty() == false)
    this->mystack.pop();
  free(this->dealed_triple);
  //NULL if using multi-join method
  delete[] this->index_lists;
  this->result_list = NULL;
  this->satellites.clear();
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
  return 0; //DEBUG:remove when index_join is ok
            //BETTER?:use appropiate method according to size and structure
  int edge_num = this->basic_query->getTripleNum();
  double dense = (double)edge_num / this->var_num;
  //BETTER:how to guess the size of can_lists
  double size = (_smallest + _biggest) / 2.0;
  double ans = Join::PARAM_DENSE * dense - size / Join::PARAM_SIZE;
  if (ans > Join::JUDGE_LIMIT)
    return 0; //multi_join method
  else
    return 1; //index_join method
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
  for (int i = 0; i < this->var_num; ++i) {
    //satellites which are not retrieved
    if (!this->basic_query->isReady(i)) {
      continue;
    }

    double tmp = this->score_node(this->basic_query->getVarDegree(i), this->basic_query->getCandidateSize(i));
    if (tmp > max) {
      max = tmp;
      maxi = i;
    }
  }
  if (maxi == -1) {
    cout << "error to select the first one to join" << endl;
  } else {
    this->start_id = maxi;
  }
#ifdef DEBUG_JOIN
  //printf("the start id is: %d\n", this->start_id);
  cerr << "the start id is: " << this->start_id << endl;
#endif
}

//join on the vector of CandidateList, available after
//retrieved from the VSTREE and store the resut in _result_set
bool
Join::join_sparql(SPARQLquery& _sparql_query)
{
  int basic_query_num = _sparql_query.getBasicQueryNum();
  //join each basic query
  for (int i = 0; i < basic_query_num; i++) {
    //fprintf(stderr, "Basic query %d\n", i);
    cerr << "Basic query " << i << endl;
    bool ret = this->join_basic(&(_sparql_query.getBasicQuery(i)));
    if (!ret)
      cerr << "end directly for this basic query: " << i << endl;
  }

  return true;
}

bool
Join::join_basic(BasicQuery* _basic_query)
{
  this->init(_basic_query);
  long begin = Util::get_cur_time();
  bool ret1 = this->filter_before_join();
  long after_constant_filter = Util::get_cur_time();
  //fprintf(stderr, "after filter_before_join: used %ld ms\n", after_filter - begin);
  cerr << "after filter_before_join: used " << (after_constant_filter - begin) << " ms" << endl;
  if (!ret1) {
    this->clear();
    return false;
  }

  this->add_literal_candidate();
  long after_add_literal = Util::get_cur_time();
  cerr << "after add_literal_candidate: used " << (after_add_literal - after_constant_filter) << " ms" << endl;

  bool ret2 = this->allFilterByPres();
  //bool ret2 = true;
  long after_pre_filter = Util::get_cur_time();
  cerr << "after allFilterByPres: used " << (after_pre_filter - after_add_literal) << " ms" << endl;
  if (!ret2) {
    this->clear();
    return false;
  }

  bool ret3 = this->join();
  long after_joinbasic = Util::get_cur_time();
  cerr << "after join_basic: used " << (after_joinbasic - after_pre_filter) << " ms" << endl;
  if (!ret3) {
    this->clear();
    return false;
  }

  //TODO+DEBUG:only_pre_filter needed to deal with undealed isolated edge
  //only when allFilterByPres not used!!!
  //BETTER:we need to choose a way to decide a set of isolated edges to be dealed before join(one by one in join or just before?)
  //the other should be dealed later(filter efficience is low!!!)

  //NOTICE:we do pre_var_handler first, and generate all satellites when coping to result list
  //
  //this->generateAllSatellites();
  //long after_generate_satellite = Util::get_cur_time();
  //cerr<<"after generate satellite: used "<<(after_generate_satellite - after_joinbasic)<<" ms"<<endl;

  //BETTER+QUERY: consider satellite with pre var, which first?
  //I think s2p first is better but s2o is also ok
  //1. filter by predicate vars first, so num decreases, need to specify the case that a var is not retrieved
  //generate later use sp2o or op2s (for each pre var, sevearl candidates)
  //2. generate candidates for satellites first using sp2o or s2o(op2s or o2s), later filtered by pre vars
  //the generating process had better been placed at the final, just before copying result
  this->pre_var_handler();
  //TODO+BETTER:maybe also reduce to empty, return false
  long after_pre_var = Util::get_cur_time();
  cerr << "after pre var: used " << (after_pre_var - after_joinbasic) << " ms" << endl;

  this->copyToResult();
  long after_copy = Util::get_cur_time();
  cerr << "after copy to result list: used " << (after_copy - after_pre_var) << " ms" << endl;

  cerr << "Final result size: " << this->basic_query->getResultList().size() << endl;
  this->clear();
  return true;
}

void
Join::generateAllSatellites()
{
  //BETTER: directly generate to result list, avoiding copying cost?
  int core_var_num = this->basic_query->getRetrievedVarNum();

  for (int i = 0; i < core_var_num; ++i) {
    int id = this->pos2id[i];
    int degree = this->basic_query->getVarDegree(id);
    for (int j = 0; j < degree; ++j) {
      int id2 = this->basic_query->getEdgeNeighborID(id, j);
      if (this->basic_query->isSatelliteInJoin(id2) == false)
        continue;
      char edge_type = this->basic_query->getEdgeType(id, j);
      int preid = this->basic_query->getEdgePreID(id, j);

      for (TableIterator it = this->current_table.begin(); it != this->new_start; ++it) {
        //TODO:generate and add just like join
      }
      this->new_start = this->current_table.end();
    }
  }
}

bool
Join::pre_var_handler()
{
  //int core_var_num = this->basic_query->getRetrievedVarNum();
  unsigned pre_var_num = this->basic_query->getPreVarNum();
#ifdef DEBUG_JOIN
  cerr << "pre var num: " << pre_var_num << endl;
#endif
  //QUERY+BETTER:filter by pre vars one by one or each record together?
  for (unsigned i = 0; i < pre_var_num; ++i) {
#ifdef DEBUG_JOIN
    cerr << "current pre var id: " << i << endl;
#endif
    const PreVar& pre_var = this->basic_query->getPreVarByID(i);

#ifdef DEBUG_JOIN
    cerr << "current table size: " << this->current_table.size() << endl;
#endif

    //WARN:do not conflict with original var id
    //1 core var, id can be 1, then pos can be 1 + 0 = 1 for pre var!!! conflict!
    //int pos = core_var_num + i;
    int pos = this->var_num + i;
    this->add_id_pos_mapping(pos);
    //cout<<"id 1 pos "<<this->id2pos[1]<<endl;

    bool if_new_start = false;
    //for each record, use s/o2p for each triple containing this pre var to filter
    for (TableIterator it = this->current_table.begin(); it != this->new_start;) {
      IDList valid_ans;
      //bool ok = true;
      unsigned triple_num = pre_var.triples.size();
#ifdef DEBUG_JOIN
//cerr<<"triple num for this var: "<<triple_num<<endl;
#endif
      for (unsigned j = 0; j < triple_num; ++j) {
        const Triple& triple = this->basic_query->getTriple(pre_var.triples[j]);
        string sub_name = triple.subject;
        string obj_name = triple.object;
#ifdef DEBUG_JOIN
//cerr << sub_name << endl << triple.predicate << endl << obj_name << endl;
#endif
        int sub_id = -1, obj_id = -1, var1 = -1, var2 = -1;

        if (sub_name[0] != '?') {
          sub_id = this->kvstore->getIDByEntity(sub_name);
        } else {
          if (!(this->basic_query->isOneDegreeNotJoinVar(sub_name)))
            var1 = this->basic_query->getIDByVarName(sub_name);
          //satellite in join not retrieved
          if (var1 != -1 && this->basic_query->isSatelliteInJoin(var1))
            var1 = -1;
        }

        if (obj_name[0] != '?') {
          obj_id = this->kvstore->getIDByEntity(obj_name);
          if (obj_id == -1)
            obj_id = this->kvstore->getIDByLiteral(obj_name);
        } else {
          if (!(this->basic_query->isOneDegreeNotJoinVar(obj_name)))
            var2 = this->basic_query->getIDByVarName(obj_name);
          //satellite in join not retrieved
          if (var2 != -1 && this->basic_query->isSatelliteInJoin(var2))
            var2 = -1;
        }
        //cout<<"var1: "<<var1<<"   var2: "<<var2<<endl;

        int* id_list = NULL;
        int id_list_len = 0;
        //two vars in query
        if (sub_id == -1 && obj_id == -1) {
          if (var1 == -1 && var2 == -1) {
            //NOTICE: this is a special case: select ?p where { ?s ?p ?o . }
            //must be only one triple, otherwise exist a node > 1
            //(?s2 ?p ?o2 is not ok, not connected query graph)
            //WARN+QUERY: if only this triple, no answer for ?p
            //we need to output all predicates in data graph, so store one file containing
            //entity/literal/predicate num when building, and reoutput all when changing
            //(binary file i snot visible, so use character file)
            //
            //we shall deal with this case in the Strategy module in time
          } else if (var1 == -1 && var2 != -1) {
            //TODO+NOTICE: we must add literals here, also enum all predicates and using p2o
            //but literals should only be added once for each predicate
            //QUERY:maybe many in edges all with unbound predicates
            //we think this case is very rare, so not consider now
            //
            //how about bound predicates? ?s1 p1 ?o    ?s2 p2 ?o (?o retrieved, p2o to add literals)
            //all unbound predicates: ?s1 ?p1 ?o    ?s2 ?p2 ?o
            //if exist constant neighbor, just use s2o to add literals(already discussed)
            //NOTICE+WARN+TODO: in these cases, all subject degree is 1, but we can not start from ?o because
            //it is a literal var!!!
            //(so join can not be processed, however, do not need to join here)
            //must add literal for ?o before or treat it as special case, considered in Strategy!!

            //cout<<"pos: "<<this->id2pos[var2]<<"   ele: "<<(*it)[0]<<endl;
            this->kvstore->getpreIDlistByobjID((*it)[this->id2pos[var2]], id_list, id_list_len, true);
          } else if (var1 != -1 && var2 == -1) {
            this->kvstore->getpreIDlistBysubID((*it)[this->id2pos[var1]], id_list, id_list_len, true);
          } else if (var1 != -1 && var2 != -1) {
            //if(this->is_literal_var(var2))
            //{
            //int* oid_list = NULL;
            //int oid_list_len = 0;
            //this->kvstore->getobjIDlistBysubID((*it)[this->id2pos[var1]], oid_list, oid_list_len);
            //this->kvstore->getpreIDlistBysubID((*it)[this->id2pos[var1]], id_list, id_list_len);
            //}
            //cerr<<"sub str: "<<this->kvstore->getEntityByID((*it)[this->id2pos[var1]])<<endl;
            //cerr<<"obj str: "<<this->kvstore->getEntityByID((*it)[this->id2pos[var2]])<<endl;
            //this->kvstore->getpreIDlistBysubIDobjID((*it)[this->id2pos[var1]], (*it)[this->id2pos[var2]], id_list, id_list_len);
            int sid = (*it)[this->id2pos[var1]], oid = (*it)[this->id2pos[var2]];
            this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len, true);
            //NOTICE:no need to add literals here because they are added when join using s2o
          }
        }
        //two constants in query
        else if (sub_id != -1 && obj_id != -1) {
          //just use so2p in query graph to find predicates
          //this->kvstore->getpreIDlistBysubIDobjID(sub_id, obj_id, id_list, id_list_len);
          int sid = sub_id, oid = obj_id;
          this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len, true);
        }
        //sub is var while obj is constant
        else if (sub_id == -1 && obj_id != -1) {
          if (var1 == -1) {
            this->kvstore->getpreIDlistByobjID(obj_id, id_list, id_list_len, true);
          } else {
            this->kvstore->getpreIDlistBysubIDobjID((*it)[this->id2pos[var1]], obj_id, id_list, id_list_len, true);
            int sid = (*it)[this->id2pos[var1]], oid = obj_id;
            this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len, true);
          }
        }
        //sub is constant while obj is var
        else if (sub_id != -1 && obj_id == -1) {
          if (var2 == -1) {
            this->kvstore->getpreIDlistBysubID(sub_id, id_list, id_list_len, true);
          } else {
            //NOTICE:no need to add literals here because they are added in add_literal_candidate using s2o
            //this->kvstore->getpreIDlistBysubIDobjID(sub_id, (*it)[this->id2pos[var2]], id_list, id_list_len);
            int sid = sub_id, oid = (*it)[this->id2pos[var2]];
            this->kvstore->getpreIDlistBysubIDobjID(sid, oid, id_list, id_list_len, true);
          }
        }

        //cout<<"the idlist len "<<id_list_len<<endl;
        if (j == 0) {
          valid_ans.unionList(id_list, id_list_len);
        } else {
          valid_ans.intersectList(id_list, id_list_len);
        }
        delete[] id_list;
        if (valid_ans.size() == 0) {
#ifdef DEBUG_JOIN
          cerr << "already empty!" << endl;
#endif
          //ok = false;
          break;
        } else {
#ifdef DEBUG_JOIN
//for(int k = 0; k < valid_ans.size(); ++k)
//cerr << this->kvstore->getPredicateByID(valid_ans[k])<<endl;
#endif
        }
      }

      //add the candidates of this pre var if selected,
      //beyond graph_var_num if satellites are generated first;
      //beyond core_var_num if not
      //
      //NOTICE: we add all here(select/not) because they maybe needed by generating satellites
      //we need to copy only the selected ones in copyToResult
      int size = valid_ans.size();

      if (size > 0) {
        it->push_back(valid_ans[0]);
        int begin = 1;
        if (!if_new_start && size > 1) {
          this->add_new_to_results(it, valid_ans[1]);
          if_new_start = true;
          this->new_start = this->current_table.end();
          this->new_start--;
          begin = 2;
        }
        for (int j = begin; j < size; ++j) {
          this->add_new_to_results(it, valid_ans[j]);
        }
        it++;
      } else {
        it = this->current_table.erase(it);
      }
    }

    this->new_start = this->current_table.end();
  }

  cout << "table size after pre_var " << this->current_table.size() << endl;
  return true;
}

void
Join::copyToResult()
{
  //copy to result list, adjust the vars order
  this->result_list->clear();
  int select_var_num = this->basic_query->getSelectVarNum();
  int core_var_num = this->basic_query->getRetrievedVarNum();
  int pre_var_num = this->basic_query->getPreVarNum();

  //TODO:set right selected_pre_var_num here
  int selected_pre_var_num = pre_var_num;
  if (this->id_pos != core_var_num + selected_pre_var_num) {
    cerr << "terrible error in copyToResult!" << endl;
    return;
  }

#ifdef DEBUG_JOIN
  cerr << "core var num: " << core_var_num << " select var num: " << select_var_num << endl;
#endif
  this->record_len = select_var_num + pre_var_num;
  this->record = new int[this->record_len];

  for (TableIterator it = this->current_table.begin(); it != this->current_table.end(); ++it) {
    int i = 0;
    for (; i < core_var_num; ++i) {
      //This is because sleect var id is always smaller
      if (this->pos2id[i] < select_var_num)
        this->record[this->pos2id[i]] = (*it)[i];
    }

#ifdef DEBUG_JOIN
//cerr<<"current id_pos: "<<this->id_pos<<endl;
#endif
    //below are for selected pre vars
    while (i < this->id_pos) {
      //TODO:only add selected ones
      //int pre_var_id = this->pos2id[i] - core_var_num;
      int pre_var_id = this->pos2id[i] - this->var_num;
      this->record[select_var_num + pre_var_id] = (*it)[i];
      ++i;
    }

    //generate satellites when constructing records
    //NOTICE: satellites in join must be selected
    //core vertex maybe not in select
    //
    //vector<Satellite> satellites;
    for (i = 0; i < core_var_num; ++i) {
      int id = this->pos2id[i];
      int ele = (*it)[i];
      int degree = this->basic_query->getVarDegree(id);
      for (int j = 0; j < degree; ++j) {
        int id2 = this->basic_query->getEdgeNeighborID(id, j);
        if (this->basic_query->isSatelliteInJoin(id2) == false)
          continue;
#ifdef DEBUG_JOIN
//cerr << "to generate "<<id2<<endl;
#endif
        int* idlist = NULL;
        int idlist_len = 0;
        int triple_id = this->basic_query->getEdgeID(id, j);
        Triple triple = this->basic_query->getTriple(triple_id);

        int preid = this->basic_query->getEdgePreID(id, j);
        if (preid == -2) //?p
        {
          string predicate = triple.predicate;
          int pre_var_id = this->basic_query->getPreVarID(predicate);
          preid = (*it)[core_var_num + pre_var_id];
        } else if (preid == -1) {
          //ERROR
        }

        char edge_type = this->basic_query->getEdgeType(id, j);
        if (edge_type == Util::EDGE_OUT) {
          this->kvstore->getobjIDlistBysubIDpreID(ele, preid, idlist, idlist_len, true);
        } else {
          this->kvstore->getsubIDlistByobjIDpreID(ele, preid, idlist, idlist_len, true);
        }
        this->satellites.push_back(Satellite(id2, idlist, idlist_len));
#ifdef DEBUG_JOIN
//cerr<<"push a new satellite in"<<endl;
#endif
      }
    }
#ifdef DEBUG_JOIN
//cerr<<"satellites all prepared!"<<endl;
#endif
    int size = satellites.size();
    this->cartesian(0, size);
#ifdef DEBUG_JOIN
//cerr<<"after cartesian"<<endl;
#endif
    for (int k = 0; k < size; ++k) {
      delete[] this->satellites[k].idlist;
      //this->satellites[k].idlist = NULL;
    }
    //WARN:use this to avoid influence on the next loop
    this->satellites.clear();
#ifdef DEBUG_JOIN
//cerr<<"after clear the satellites"<<endl;
#endif
  }
  delete[] this->record;
#ifdef DEBUG_JOIN
//cerr<<"after delete the record"<<endl;
#endif
  this->record = NULL;
  this->record_len = 0;
}

void
Join::cartesian(int pos, int end)
{
  if (pos == end) {
    int* new_record = new int[this->record_len];
    memcpy(new_record, this->record, sizeof(int) * this->record_len);
    this->result_list->push_back(new_record);
    return;
  }

  int size = this->satellites[pos].idlist_len;
  int id = this->satellites[pos].id;
  int* list = this->satellites[pos].idlist;
  for (int i = 0; i < size; ++i) {
    this->record[id] = list[i];
    this->cartesian(pos + 1, end);
  }
}

void
Join::toStartJoin()
{
  for (int i = 0; i < this->var_num; ++i) {
    if (this->basic_query->isReady(i)) {
      return;
    }
  }

  cout << "toStartJoin(): need to prepare a ready node" << endl;

  int maxi = -1;
  double max = 0;
  for (int i = 0; i < this->var_num; ++i) {
    if (!this->basic_query->isSatelliteInJoin(i)) {
      double tmp = this->score_node(this->basic_query->getVarDegree(i), this->basic_query->getCandidateSize(i));
      if (tmp > max) {
        max = tmp;
        maxi = i;
      }
    }
  }

  //TODO+DEBUG:add literals error?

  //NOTICE:not add literal, so no constant neighbor, this must be free literal variable
  int var_id = maxi;
  int var_degree = this->basic_query->getVarDegree(var_id);
  //cout<<"var id: "<<var_id<<"   "<<"var degree: "<<var_degree<<endl;
  IDList literal_candidate_list;
  for (int j = 0; j < var_degree; ++j) {
    //int neighbor_id = this->basic_query->getEdgeNeighborID(var_id, j);
    int predicate_id = this->basic_query->getEdgePreID(var_id, j);
    int triple_id = this->basic_query->getEdgeID(var_id, j);
    Triple triple = this->basic_query->getTriple(triple_id);
    string neighbor_name = triple.subject;
    IDList this_edge_literal_list;
    int* object_list = NULL;
    int object_list_len = 0;

    if (predicate_id >= 0)
      (this->kvstore)->getobjIDlistBypreID(predicate_id, object_list, object_list_len, true);
    //cout<<"predicate id: "<<predicate_id<<endl<<this->kvstore->getPredicateByID(predicate_id)<<endl;
    //for(int sb = 0; sb < object_list_len; ++sb)
    //{
    //cout<<object_list[sb]<<" ";
    //}
    //cout<<endl;
    this_edge_literal_list.unionList(object_list, object_list_len, true);
    delete[] object_list;
    cout << "preid: " << predicate_id << " length: " << object_list_len << " literals: " << this_edge_literal_list.size() << endl;

    if (j == 0) {
      literal_candidate_list.unionList(this_edge_literal_list);
    } else {
      literal_candidate_list.intersectList(this_edge_literal_list);
    }
  }

  IDList& origin_candidate_list = this->basic_query->getCandidateList(var_id);
  //int origin_candidate_list_len = origin_candidate_list.size();
  origin_candidate_list.unionList(literal_candidate_list, true);
  //int after_add_literal_candidate_list_len = origin_candidate_list.size();
  this->basic_query->setReady(var_id);

  cout << "add literals num: " << literal_candidate_list.size() << endl;
  cout << "current can size: " << origin_candidate_list.size() << endl;
}

// use the appropriate method to join candidates
bool
Join::join()
{
  //in case of no start point, if all core vertices are literal vars
  this->toStartJoin();

  //the smallest candidate list size of the not-satellite vars
  int id = this->basic_query->getVarID_FirstProcessWhenJoin();
  int smallest = this->basic_query->getCandidateSize(id);
  if (!this->is_literal_var(id) && smallest == 0)
    return false; //empty result
  int biggest = this->basic_query->getVarID_MaxCandidateList();

  int method = this->judge(smallest, biggest);
  bool ret = true;
  switch (method) {
  case 0:
    //printf("use multi-join here!\n");
    cerr << "use multi-join here!" << endl;
    ret = this->multi_join();
    break;
  case 1:
    //printf("use index-join here!\n");
    cerr << "use index-join here!" << endl;
    ret = this->index_join();
    break;
  default:
    //printf("ERROR: no method found!\n");
    cerr << "ERROR: no method found!" << endl;
    break;
  }

  return ret;
}

int
Join::choose_next_node(int id)
{
  //choose a child to search deeply
  int degree = this->basic_query->getVarDegree(id);
  int maxi = -1;
  double max = 0;
  for (int i = 0; i < degree; ++i) {
    int var_id2 = this->basic_query->getEdgeNeighborID(id, i);
    if (var_id2 == -1) //not in join, including constant
    {
      continue;
    }

    //satellites which are not retrieved
    if (this->basic_query->if_need_retrieve(var_id2) == false) {
      continue;
    }

    // each triple/edge need to be processed only once.
    int edge_id = this->basic_query->getEdgeID(id, i);
    if (this->dealed_triple[edge_id]) {
      continue;
    }

    //NTC:not using updated degrees, other not the whole loop
    double tmp = this->score_node(this->basic_query->getVarDegree(var_id2), this->basic_query->getCandidateSize(var_id2));
    if (max < tmp) {
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
Join::add_new_to_results(TableIterator it, int id)
{
  //NTC:already have one more in *it if need to push back
  RecordType tmp(*it);
  *(tmp.rbegin()) = id;
  this->current_table.push_back(tmp);
}

void
Join::acquire_all_id_lists(IdLists& _id_lists, IdListsLen& _id_lists_len, IDList& _can_list, vector<int>& _edges, int _id, int can_list_size)
{
  int* tmp_id_list;
  int tmp_id_list_len;
  for (int i = 0; i < this->id_pos; ++i) {
    // keep empty if not valid/used
    _id_lists.push_back(vector<int*>());
    _id_lists_len.push_back(vector<int>());
    int edge_index = _edges[i];
    if (edge_index != -1) {
      int pre_id = this->basic_query->getEdgePreID(_id, edge_index);
      //int edge_id = this->basic_query->getEdgeID(_id, edge_index);
      int edge_type = this->basic_query->getEdgeType(_id, edge_index);
      if (pre_id >= 0) // valid
      {
        for (int j = 0; j < can_list_size; ++j) {
          if (edge_type == Util::EDGE_IN) {
            this->kvstore->getsubIDlistByobjIDpreID(_can_list[i],
                                                    pre_id, tmp_id_list, tmp_id_list_len, true);
          } else //EDGE_OUT
          {
            this->kvstore->getobjIDlistBysubIDpreID(_can_list[i],
                                                    pre_id, tmp_id_list, tmp_id_list_len, true);
          }
          _id_lists.rbegin()->push_back(tmp_id_list);
          _id_lists_len.rbegin()->push_back(tmp_id_list_len);
        }
      }
    }
  }
}

//DEBUG:add debug info and check when the var is not free
bool
Join::new_join_with_multi_vars_prepared(IdLists& _id_lists, IdListsLen& _id_lists_len, vector<int>& _edges, IDList& _can_list, int _can_list_size)
{
  if (_can_list_size == 0) {
    return false; //empty result
  }
  bool found = false;        //no record matched
  bool if_new_start = false; //the first to add to end in while
                             //list< list<int> > temp_table;
  for (TableIterator it0 = this->current_table.begin(); it0 != this->new_start;) {
    bool matched = false; //this record matched
    bool added = false;   //if one ele added already
    for (int i = 0; i < _can_list_size; ++i) {
      int cnt = 0;
      bool linked = true;
      for (RecordIterator it1 = it0->begin(); it1 != it0->end(); ++it1, ++cnt) {
        int edge_index = _edges[cnt];
        if (edge_index == -1) {
          continue;
        }
        int ele = *it1;
        if (_id_lists_len[cnt][i] == 0) {
          linked = false;
          break;
        }
        if (Util::bsearch_int_uporder(ele, _id_lists[cnt][i], _id_lists_len[cnt][i]) == -1) {
          linked = false;
          break;
        }
      }
      if (linked) {
        if (added) {
          this->add_new_to_results(it0, _can_list[i]);
          if (!if_new_start) {
            if_new_start = true;
            this->new_start = this->current_table.end();
            this->new_start--;
          }
        } else {
          added = true;
          it0->push_back(_can_list[i]);
        }
        matched = true;
      }
    }
    if (matched) {
      found = true;
      it0++;
      //it3++;
    } else {
      it0 = this->current_table.erase(it0);
      //it3 = this->table_row_new.erase(it3);
    }
  }
  return found;
}

bool
Join::new_join_with_multi_vars_not_prepared(vector<int>& _edges, IDList& _can_list, int _can_list_size, int _id, bool _is_literal)
{
  if (_can_list_size == 0 && !_is_literal) {
    return false; //empty result
  }
  bool found = false;
  bool if_new_start = false; //the first to add to end in while
  for (TableIterator it0 = this->current_table.begin(); it0 != this->new_start;) {
#ifdef DEBUG_JOIN
    if (this->new_start != this->current_table.end()) {
      //printf("now the new_start is:");
      cerr << "now the new_start is:";
      for (RecordIterator it1 = this->new_start->begin(); it1 != this->new_start->end(); ++it1) {
        //printf(" %d", *it1);
        cerr << " " << *it1;
      }
      //printf("\n");
      cerr << endl;
    } else
      //printf("new_start still in end?!\n");
      cerr << "new_start still in end?!" << endl;
    //printf("now the record is:");
    cerr << "now the record is:";
    for (RecordIterator it1 = it0->begin(); it1 != it0->end(); ++it1) {
      //printf(" %d", *it1);
      cerr << " " << *it1;
    }
    //printf("\n");
    cerr << endl;
#endif

    int cnt = 0;
    //update the valid id num according to restrictions by multi vars
    //also ordered while id_list and can_list are ordered
    //IDList valid_ans_list;
    IDList* valid_ans_list = NULL;
    //list<int> valid_ans_list;
    bool matched = true;
    //NOTICE:we can generate cans from either direction, but this way is convenient and better
    for (RecordIterator it1 = it0->begin(); it1 != it0->end(); ++it1, ++cnt) {
#ifdef DEBUG_JOIN
      //printf("cnt is: %d\n", cnt);
      cerr << "cnt is: " << cnt << endl;
#endif
      int edge_index = _edges[cnt];
      if (edge_index == -1) {
        continue;
      }
#ifdef DEBUG_JOIN
      cerr << "edge exists!" << endl;
#endif
      int ele = *it1;
      int edge_type = this->basic_query->getEdgeType(_id, edge_index);
      int pre_id = this->basic_query->getEdgePreID(_id, edge_index);

      if (pre_id == -2) //predicate var
      {
#ifdef DEBUG_JOIN
        cerr << "this is a predicate var!" << endl;
#endif
        //if(valid_ans_list == NULL)
        //{
        //valid_ans_list = IDList::intersect(_can_list, NULL, 0);
        //}
        //else
        //{
        //}
        //continue;
      }

      int* id_list;
      int id_list_len;
      if (edge_type == Util::EDGE_IN) {
#ifdef DEBUG_JOIN
        //printf("this is an edge to our id to join!\n");
        cerr << "this is an edge to our id to join!" << endl;
#endif
        if (pre_id == -2)
          this->kvstore->getobjIDlistBysubID(ele, id_list, id_list_len, true);
        else if (pre_id >= 0)
          this->kvstore->getobjIDlistBysubIDpreID(ele,
                                                  pre_id, id_list, id_list_len, true);
      } else {
#ifdef DEBUG_JOIN
        //printf("this is an edge from our id to join!\n");
        cerr << "this is an edge from our id to join!" << endl;
#endif
        if (pre_id == -2)
          this->kvstore->getsubIDlistByobjID(ele, id_list, id_list_len, true);
        else
          this->kvstore->getsubIDlistByobjIDpreID(ele, pre_id, id_list, id_list_len, true);
      }
      if (id_list_len == 0) {
        //id_list == NULL in this case, no need to free
        matched = false;
#ifdef DEBUG_JOIN
        //printf("this id_list is empty!\n");
        cerr << "this id_list is empty!" << endl;
#endif
        break;
      }

      //NOTICE:using so2p to filter is not good
      //The cost to join two ordered lists is the basic operation
      //of the whole join process!(O(klogn) < O(k+n) gennerally, for k < n)
      //Notice that n is the candidate list size just retrieved from vstree

      //TODO+BETTER:compute all sets by multiple restrictions, and choose
      //the minimal to start intersect operations

      //only can occur the first time, means cnt == 0
      //if(valid_ans_list.size() == 0)
      if (valid_ans_list == NULL) {
        //WARN:this is too costly due to coping elements!
        //valid_ans_list.unionList(_can_list);
        if (_is_literal) {
          int entity_len = 0;
          while (true) {
            if (entity_len == id_list_len || Util::is_literal_ele(id_list[entity_len]))
              break;
            entity_len++;
          }
          //valid_ans_list.intersectList(id_list, entity_len);
          valid_ans_list = IDList::intersect(_can_list, id_list, entity_len);
          valid_ans_list->unionList(id_list + entity_len, id_list_len - entity_len, true);
          //this->basic_query->setAddedLiteralCandidate(_id);
        } else {
          valid_ans_list = IDList::intersect(_can_list, id_list, id_list_len);
        }
        //for(int i = 0; i < id_list_len; ++i)
        //{
        //if we found this element(entity/literal) in
        //var1's candidate list, or this is a literal
        //element and var2 is a free literal variable,
        //we should add this one to result.
        //bool flag = false;
        //NOTICE:this var is free, but it can also contain
        //entities. Candidates after retrieved from vstree will
        //contain all possible entities, but no literals.
        //if(Util::is_literal_ele(id_list[i]))
        //{
        //if(_is_literal)
        //{
        //flag = true;
        //#ifdef DEBUG_JOIN
        //printf("to add literal for free variable!\n");
        //#endif
        //}
        //}
        //else
        //{
        //flag = _can_list.bsearch_uporder(id_list[i]) >= 0;
        //}
        //if(!flag) continue;
        //printf("add the ele to list!\n");
        //valid_ans_list.addID(id_list[i]);
        //}
      } else {
        valid_ans_list->intersectList(id_list, id_list_len);
        //for(list<int>::iterator it2 = valid_ans_list.begin(); it2 != valid_ans_list.end();)
        //{
        //int tmp = *it2;
        //if(Util::bsearch_int_uporder(tmp, id_list, id_list_len) == -1)
        //{
        //it2 = valid_ans_list.erase(it2);
        //}
        //else
        //{
        //it2++;
        //}
        //}
      }
      delete[] id_list;
      if (valid_ans_list->size() == 0) {
        matched = false;
        break;
      }
    }
    if (matched) {
#ifdef DEBUG_JOIN
      //printf("this record is matched!!\n");
      cerr << "this record is matched!!" << endl;
#endif
      found = true;
      //bool added = false;
      //add new var results to table from valid_ans_list
      //for(list<int>::iterator it2 = valid_ans_list.begin(); it2 != valid_ans_list.end(); ++it2)
      int size = valid_ans_list->size();

      it0->push_back((*valid_ans_list)[0]);
      int begin = 1;
      if (!if_new_start && size > 1) {
        this->add_new_to_results(it0, (*valid_ans_list)[1]);
        if_new_start = true;
        //this->new_start = this->current_table.rbegin().base();
        this->new_start = this->current_table.end();
        this->new_start--; //-1 is not allowed
        begin = 2;
      }

      for (int i = begin; i < size; ++i) {
        //WARN+NOTICE:this strategy may cause that duplicates are not together!
        this->add_new_to_results(it0, (*valid_ans_list)[i]);
      }
      it0++;
    } else {
      it0 = this->current_table.erase(it0);
#ifdef DEBUG_JOIN
      //printf("this record is not matched!\n");
      cerr << "this record is not matched!" << endl;
#endif
    }
    delete valid_ans_list;
    valid_ans_list = NULL;
  }
  return found;
}

bool
Join::if_prepare_idlist(int _can_list_size, bool _is_literal)
{
  if (!_is_literal && _can_list_size < Join::LIMIT_CANDIDATE_LIST_SIZE)
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
//TODO:check the time of each part in bsbm_100000, self5.sql, self6.sql
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

  //BETTER:filter all vertices first by allFilterByPres first and then select the minium?
  //QUERY+TODO:literal var not suitable for joining first!

  //The best strategy is to ensure that for each record, all satellite edges exist
  //then after join all core vertices, generate candidates for each satellite
  //and these are just the final accurate answer
  //It's out of question better than generating candidates for satellites now
  //
  //NOTICE:this should be done just once, so use it before pushing candidates
  //pruning the original candidates first(satellites only concerned with itself)
  //this->filterBySatellites(this->start_id);

  IDList& start_table = this->basic_query->getCandidateList(this->start_id);
  int start_size = this->basic_query->getCandidateSize(this->start_id);
#ifdef DEBUG_JOIN
  cerr << "the start size " << start_size << endl;
#endif
  for (int i = 0; i < start_size; ++i) {
    int ele = start_table.getID(i);
    RecordType record(1, ele);
    this->current_table.push_back(record);
    //this->table_row_new.push_back(false);
  }
  this->add_id_pos_mapping(this->start_id);
  //cout<<"the mapping is id "<<this->start_id<<"   and pos "<<this->id2pos[this->start_id]<<endl;
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
#ifdef DEBUG_JOIN
  //fprintf(stderr, "now to start the stack loop\n");
  cerr << "now to start the stack loop" << endl;
#endif
  while (!this->mystack.empty()) {
    int id = this->mystack.top();

#ifdef DEBUG_JOIN
    //fprintf(stderr, "the current id: %d\n", id);
    cerr << "the current id: " << id << endl;
#endif
    //int id = mystack[top];
    int maxi = this->choose_next_node(id);
    if (maxi == -1) //all edges of this node are dealed
    {
#ifdef DEBUG_JOIN
      //fprintf(stderr, "the node is totally dealed: %d\n", id);
      cerr << "the node is totally dealed: " << id << endl;
#endif
      //top--;
      this->mystack.pop();
      continue;
    }
    int id2 = this->basic_query->getEdgeNeighborID(id, maxi);
#ifdef DEBUG_JOIN
    //fprintf(stderr, "the next node id to join: %d\n", id2);
    cerr << "the next node id to join: " << id2 << endl;
#endif
//this->filterBySatellites(id2);
#ifdef DEBUG_JOIN
    cerr << "the start size " << this->basic_query->getCandidateSize(id2) << endl;
#endif

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
    vector<vector<int*> > id_lists;
    vector<vector<int> > id_lists_len;
    //int* tmp_id_list;
    //int tmp_id_list_len;
    IDList& can_list = this->basic_query->getCandidateList(id2);
    int can_list_size = can_list.size();

    for (int i = 0; i < this->id_pos; ++i) {
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
    if (is_literal) {
#ifdef DEBUG_PRECISE
      //fprintf(stderr, "this var may contain literals: %d\n", id2);
      cerr << "this var may contain literals: " << id2 << endl;
#endif
      this->basic_query->setReady(id2);
    } else {
#ifdef DEBUG_PRECISE
      //fprintf(stderr, "this var not contain literals: %d\n", id2);
      cerr << "this var not contain literals: " << id2 << endl;
#endif
    }

    bool flag = false;
    bool if_prepare = this->if_prepare_idlist(can_list_size, is_literal);
    //#ifdef DEBUG_JOIN
    if_prepare = false;
    //#endif
    //needed if place can_list in the outer loop to join
    if (if_prepare) {
#ifdef DEBUG_PRECISE
      //fprintf(stderr, "this edge uses prepared-join way\n");
      cerr << "this edge uses prepared-join way" << endl;
#endif
      this->acquire_all_id_lists(id_lists, id_lists_len, can_list, edges, id2, can_list_size);
      flag = this->new_join_with_multi_vars_prepared(id_lists, id_lists_len, edges, can_list, can_list_size);
      //need to release id_lists if using acquire_all_id_lists() firstly
      for (vector<vector<int*> >::iterator p1 = id_lists.begin(); p1 != id_lists.end(); ++p1) {
        for (vector<int*>::iterator p2 = p1->begin(); p2 != p1->end(); ++p2) {
          delete[] * p2;
        }
      }
    } else {
#ifdef DEBUG_PRECISE
      //fprintf(stderr, "this edge uses not-prepared-join way\n");
      cerr << "this edge uses not-prepared-join way" << endl;
#endif
      flag = this->new_join_with_multi_vars_not_prepared(edges, can_list, can_list_size, id2, is_literal);
    }

    //if current_table is empty, ends directly
    if (!flag) {
#ifdef DEBUG_JOIN
      //fprintf(stderr, "the result is already empty!!\n");
      cerr << "the result is already empty!!" << endl;
#endif
      //break;
      return false; //to avoid later invalid copy
    }

    for (int i = 0; i < this->id_pos; ++i) {
      int edge_index = edges[i];
      if (edge_index != -1) {
        int edge_id = this->basic_query->getEdgeID(id2, edge_index);
        dealed_triple[edge_id] = true;
      }
    }

    this->new_start = this->current_table.end();
    this->add_id_pos_mapping(id2);
    this->mystack.push(id2);
  }
#ifdef DEBUG_JOIN
  //fprintf(stderr, "now end the stack loop\n");
  cerr << "now end the stack loop" << endl;
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
//Below are functions to do index-join method
//===================================================================================================

void
Join::buildIndexLists()
{
  this->index_lists = new IndexList[this->var_num];
  for (int i = 0; i < this->var_num; ++i) {
    IDList& can_list = this->basic_query->getCandidateList(i);
    int can_list_size = can_list.size();
    for (int j = 0; j < can_list_size; ++j) {
      this->index_lists[i].candidates.push_back(IndexItem(can_list[j]));
    }
    this->index_lists[i].candidates.push_back(IndexItem());
    this->index_lists[i].border = this->index_lists[i].candidates.end();
    this->index_lists[i].border--;
  }
}

//NOTICE: list of _id1 is all ok, but maybe add literals for list of _id2
bool
Join::index_link(int _nid, int _idx)
{
  int _id1 = _nid, _id2 = this->basic_query->getEdgeNeighborID(_nid, _idx);
  bool is_literal = this->is_literal_var(_id2);
  list<IndexItem>& can1 = this->index_lists[_id1].candidates;
  list<IndexItem>& can2 = this->index_lists[_id2].candidates;
  this->index_lists[_id1].travel_map.push_back(_id2);

  //all set to false first, later change to valid if ok
  for (ItemListIterator it = can1.begin(); it != this->index_lists[_id1].border; ++it) {
    it->isValid = false;
  }
  for (ItemListIterator it = can2.begin(); it != this->index_lists[_id2].border; ++it) {
    it->isValid = false;
  }
  for (ItemListIterator it = can1.begin(); it != this->index_lists[_id1].border; ++it) {
    int edge_type = this->basic_query->getEdgeType(_id1, _idx);
    int pre_id = this->basic_query->getEdgePreID(_id1, _idx);
    int* id_list;
    int id_list_len;
    if (edge_type == Util::EDGE_IN) {
#ifdef DEBUG_JOIN
      //fprintf(stderr, "this is an edge to our id to join!\n");
      cerr << "this is an edge to our id to join!" << endl;
#endif
      this->kvstore->getobjIDlistBysubIDpreID(it->value, pre_id, id_list, id_list_len, true);
    } else {
#ifdef DEBUG_JOIN
      //fprintf(stderr, "this is an edge from our id to join!\n");
      cerr << "this is an edge from our id to join!" << endl;
#endif
      this->kvstore->getsubIDlistByobjIDpreID(it->value, pre_id, id_list, id_list_len, true);
    }
    if (id_list_len == 0) {
//id_list is NULL in this case
#ifdef DEBUG_JOIN
      //fprintf(stderr, "this id_list is empty!\n");
      cerr << "this id_list is empty!" << endl;
#endif
      continue;
    }
    it->travel.push_back(IteratorList());
    for (int i = 0; i < id_list_len; ++i) {
      //if we found this element(entity/literal) in var1's candidate list, or this is a literal
      //element and var2 is a free literal variable, we should add this one to result.
      bool flag = false;
      ItemListIterator ret;
      if (Util::is_literal_ele(id_list[i])) {
        //NOTICE:literals cannot exist in the result from VStree, so no need to search
        //if added already, then the expression in if() returns false
        if (is_literal) {
          //QUERY:maybe same one between different records, and should be dealed to be ordered!
          flag = true;
          //BETTER?:the adding way is due to the not-sort and not-binary search method
          can2.push_back(IndexItem(id_list[i]));
          ret = --can2.end();
#ifdef DEBUG_JOIN
          //fprintf(stderr, "to add literal for free variable!\n");
          cerr << "to add literal for free variable!" << endl;
#endif
        }
      } else {
        //BETTER:currently we can search in the candidate list, but the iterator?
        ret = this->index_lists[_id2].search(id_list[i]);
        if (ret != this->index_lists[_id2].border)
          flag = true;
      }
      if (!flag)
        continue;
      //printf("add the ele to list!\n");
      it->isValid = true;
      it->travel[it->travel.size() - 1].push_back(ret);
      ret->isValid = true;
    }

    delete[] id_list;
  }

  //deal with invalid eles in can1 and can2
  for (ItemListIterator it = can1.begin(); it != this->index_lists[_id1].border;) {
    if (it->isValid)
      it++;
    else {
      //BETTER:no need to add to end if the start list
      can1.push_back(*it);
      it = can1.erase(it);
    }
  }
  for (ItemListIterator it = can2.begin(); it != this->index_lists[_id2].border;) {
    if (it->isValid)
      it++;
    else {
      //NOTICE:here we can removve directly
      it = can2.erase(it);
    }
  }
  //NOTICE:we deal with possible literals in the final to avoid meaningless search before
  //The list returned by get... is sorted, and all literal id > vertex id,, so if needing
  //sort, nonsense to compare the latter part with the former part
  //However, there maybe different ordered literals lists to be added, so the order won't be
  //naturally kept
  if (this->index_lists[_id2].border != --can2.end()) {
    //adjust the border in can2, also due to the structure and unsort,, not-binary search method
    can2.erase(this->index_lists[_id2].border);
    this->index_lists[_id2].border = --can2.end();
  }

  return true;
}

//_nid is the newer
bool
Join::index_filter(int _nid, int _idx)
{
  //TODO:the two lists are allok, only remove no add
  int _id1 = _nid, _id2 = this->basic_query->getEdgeNeighborID(_nid, _idx);
  bool is_literal = this->is_literal_var(_id2);
  list<IndexItem>& can1 = this->index_lists[_id1].candidates;
  list<IndexItem>& can2 = this->index_lists[_id2].candidates;
  this->index_lists[_id1].travel_map.push_back(_id2);
  //QUERY:how about the search? nonsense to search directly in the candidate list!
  //or search reversely in the list returned by get...?
  //Notice that the newer one maybe not ordered!
  //
  //TODO:the literals should join the filter process?
  //consider to reset the border of list after filter!!!
  //
  //reverse to filter, searching in objlist, which is generated by the smaller one, and this is ordered
  return true;
}

bool
Join::table_travel(int _id1, int _id2)
{
  //NOTICE: all is ok if in valid area, just travel and link the two
  return true;
}

bool
Join::table_check(int _id1, int _id2)
{
  //NOTICE: need to verify the linking, but exist-question is many and frustrating
  return true;
}

bool
Join::travel_init(int _lid)
{
  if (this->index_lists[_lid].prepared)
    return true;
  int size = this->index_lists[_lid].travel_map.size();
  if (size == 0) {
    this->index_lists[_lid].prepared = true;
    return true;
  }
  list<IndexItem>& can = this->index_lists[_lid].candidates;
  for (list<IndexItem>::iterator it = can.begin(); it != this->index_lists[_lid].border;) {
    //deal with invalid eles according to neighbor list
    for (int i = 0; i < size; ++i) {
      int tid = this->index_lists[_lid].travel_map[i];
      if (this->travel_init(tid) == false) {
        return false;
      }
      list<list<IndexItem>::iterator>& next = it->travel[i];
      for (list<list<IndexItem>::iterator>::iterator it2 = next.begin(); it2 != next.end();) {
        if ((*it2)->isValid == false) {
          it2 = next.erase(it2);
        } else {
          it2++;
        }
      }
      if (next.empty()) {
        it->isValid = false;
        break;
      }
    }
    if (it->isValid) {
      it++;
    } else {
      IndexItem tmp = *it;
      it = can.erase(it);
      can.push_back(tmp);
    }
  }
  if (can.begin() == this->index_lists[_lid].border)
    return false;
  //BETTER:remove all invalid eles in the next lists now
  this->index_lists[_lid].prepared = true;
  return true;
}

//WARN:we nned to use IndexItem iterator/object instead of int in current table now,
//NOTICE: this travel strategy is based on the relation between two index lists
//otherwise things not work.. So, this strategy is not used for now.
bool
Join::index_travel_two()
{
  if (this->travel_init(this->start_id) == false)
    return false;
  //reuse mystack because by now the stack is already empty
  this->reset_id_pos_mapping();
  this->mystack.push(this->start_id);
  this->add_id_pos_mapping(this->start_id);
  //init the current table with the start index list
  list<IndexItem>& can = this->index_lists[this->start_id].candidates;
  for (list<IndexItem>::iterator it = can.begin(); it != this->index_lists[this->start_id].border; ++it) {
    RecordType record(1, it->value);
    this->current_table.push_back(record);
  }

  //fprintf(stderr, "now to travel and store in current table\n");
  cerr << "now to travel and store in current table" << endl;
  while (!this->mystack.empty()) {
    int id = this->mystack.top();
    //fprintf(stderr, "the current id: %d\n", id);
    cerr << "the current id: " << id << endl;
    if (this->index_lists[id].end()) //all linking of this index list are travelled
    {
      //fprintf(stderr, "the list is totally dealed: %d\n", id);
      cerr << "the list is totally dealed: " << id << endl;
      this->mystack.pop();
      continue;
    }
    int id2 = this->index_lists[id].next();
    //fprintf(stderr, "the next list id to travel: %d\n", id2);
    cerr << "the next list id to travel: " << id2 << endl;

    bool flag = true;
    //NOTICE: we assume that the former node is all ok, scanning it to border
    flag = this->table_travel(id, id2);
    //if already empty(fail to link the two lists), ends directly
    if (!flag) {
      //fprintf(stderr, "the result is already empty!!\n");
      cerr << "the result is already empty!!" << endl;
      return false; //to avoid later invalid copy
    }
    int size = this->index_lists[id2].check_map.size();
    for (int i = 0; i < size; ++i) {
      flag = this->table_check(id2, this->index_lists[id2].check_map[i]);
      if (!flag) {
        //fprintf(stderr, "the result is already empty!!\n");
        cerr << "the result is already empty!!" << endl;
        return false; //to avoid later invalid copy
      }
    }
    this->add_id_pos_mapping(id2);
    this->mystack.push(id2);
  }
  return true;
}

//NOTICE: this strtegy is based on one-line travesal, so prepare one iterator for each index list
bool
Join::index_travel_one()
{
  //TODO
  return true;
}

bool
Join::index_travel()
{
  return this->index_travel_one();
}

bool
Join::index_join()
{
  this->buildIndexLists();
  //OPTION: remove contents in candidate_list now(which are originally removed in BasicQuery::clear())
  this->select();
  this->mystack.push(this->start_id);
  this->add_id_pos_mapping(this->start_id);

  //fprintf(stderr, "now to start the stack loop\n");
  cerr << "now to start the stack loop" << endl;
  while (!this->mystack.empty()) {
    int id = this->mystack.top();
    //fprintf(stderr, "the current id: %d\n", id);
    cerr << "the current id: " << id << endl;
    int maxi = this->choose_next_node(id);
    if (maxi == -1) //all edges of this node are dealed
    {
      //fprintf(stderr, "the node is totally dealed: %d\n", id);
      cerr << "theh node is totally dealed: " << id << endl;
      this->mystack.pop();
      continue;
    }
    int id2 = this->basic_query->getEdgeNeighborID(id, maxi);
    //fprintf(stderr, "the next node id to join: %d\n", id2);
    cerr << "the next node id to join: " << id2 << endl;

    IDList& can_list = this->basic_query->getCandidateList(id2);
    //int can_list_size = can_list.size();

    bool is_literal = this->is_literal_var(id2);
    if (is_literal)
      //fprintf(stderr, "this var may contain literals: %d\n", id2);
      cerr << "this var may contain literals: " << id2 << endl;
    else
      //fprintf(stderr, "this var not contain literals: %d\n", id2);
      cerr << "this var not contain literals: " << id2 << endl;

    bool flag = true;
    //NOTICE: we assume that the former node is all ok, scanning it to border
    //flag = this->index_link(id, id2);
    flag = this->index_link(id, maxi);
    //if already empty(fail to link the two lists), ends directly
    if (!flag) {
      //fprintf(stderr, "the result is already empty!!\n");
      cerr << "the result is already empty!!" << endl;
      return false; //to avoid later invalid copy
    }
    int edge_index, edge_id = this->basic_query->getEdgeID(id, maxi);
    this->dealed_triple[edge_id] = true;
    for (int i = 0; i < this->id_pos; ++i) {
      if (this->pos2id[i] == id)
        continue;
      edge_index = this->basic_query->getEdgeIndex(id2, this->pos2id[i]);
      if (edge_index == -1)
        continue;

      //flag = this->index_filter(id2, this->pos2id[i]);
      flag = this->index_filter(id2, edge_index);
      //if already empty(fail to link the two lists), ends directly
      if (!flag) {
        //fprintf(stderr, "the result is already empty!!\n");
        cerr << "the result is already empty!!" << endl;
        return false; //to avoid later invalid copy
      }

      edge_id = this->basic_query->getEdgeID(id2, edge_index);
      this->dealed_triple[edge_id] = true;
    }

    this->add_id_pos_mapping(id2);
    this->mystack.push(id2);
  }
  //fprintf(stderr, "now end the stack loop\n");
  cerr << "now end the stack loop" << endl;

  // To travel and store in current_table, then do the last filter
  if (this->index_travel() == false)
    return false;
  //printf("now to filter through only_pre_filter_after_join\n");
  cerr << "now to filter through only_pre_filter_after_join" << endl;
  this->only_pre_filter_after_join();

  //copy to result list, adjust the vars order
  vector<int*>& result_list = this->basic_query->getResultList();
  result_list.clear();
  int select_var_num = this->basic_query->getSelectVarNum();
  for (TableIterator it0 = this->current_table.begin(); it0 != this->current_table.end(); ++it0) {
    int* record = (int*)malloc(sizeof(int) * select_var_num);
    for (int i = 0; i < this->id_pos; ++i) {
      if (this->pos2id[i] < select_var_num)
        record[this->pos2id[i]] = (*it0)[i];
    }
    result_list.push_back(record);
  }

  return true;
}

//===================================================================================================
//Below are functions before or after Join
//===================================================================================================

//sort the candidate lists and deal with all constant neigbors
bool
Join::filter_before_join()
{
  //fprintf(stderr, "*****IIIIIIN filter_before_join\n");
  cerr << "*****IN filter_before_join" << endl;

  for (int i = 0; i < this->var_num; i++) {
    bool flag = this->basic_query->isLiteralVariable(i);
    //fprintf(stderr, "\tVar%d %s\n", i, this->basic_query->getVarName(i).c_str());
    cerr << "\tVar" << i << " " << this->basic_query->getVarName(i) << endl;
    IDList& can_list = this->basic_query->getCandidateList(i);
    //fprintf(stderr, "\t\tsize of canlist before filter: %d\n", can_list.size());
    cerr << "\t\tsize of canlist before filter: " << can_list.size() << endl;
    //NOTICE:must sort before using binary search.
    can_list.sort();

    long begin = Util::get_cur_time();
    bool ret = this->constant_edge_filter(i);
    long after_constant_edge_filter = Util::get_cur_time();
    //fprintf(stderr, "\t\tconstant_edge_filter: used %ld ms\n", after_constant_edge_filter - begin);
    cerr << "\t\tconstant_edge_filter: used " << (after_constant_edge_filter - begin) << " ms" << endl;
    //		this->preid_filter(this->basic_query, i);
    //		long after_preid_filter = Util::get_cur_time();
    //cout << "\t\tafter_preid_filter: used " << (after_preid_filter-after_literal_edge_filter) << " ms" << endl;
    //fprintf(stderr, "\t\t[%d] after filter, candidate size = %d\n\n\n", i, can_list.size());
    cerr << "\t\t[" << i << "] after filter, candidate size= " << can_list.size() << endl << endl << endl;

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
    if (!flag && !ret) //already empty
    {
      return false;
    }
  }
  //fprintf(stderr, "OOOOOOUT filter_before_join\n");
  cerr << "OUT filter_before_join" << endl;
  return true;
}

//decrease the candidates of _var_i using its constant neighbors
bool
Join::constant_edge_filter(int _var_i)
{
  //Util::logging("IN literal_edge_filter"); //debug

  int var_degree = this->basic_query->getVarDegree(_var_i);
  IDList& _list = this->basic_query->getCandidateList(_var_i);
  for (int j = 0; j < var_degree; j++) {
    int neighbor_id = this->basic_query->getEdgeNeighborID(_var_i, j);
    //fprintf(stderr, "\t\t\tneighbor_id=%d\n", neighbor_id);
    cerr << "\t\t\tneighbor_id=" << neighbor_id << endl;
    if (neighbor_id != -1) //variables in join not considered here
    {
      continue;
    }

    char edge_type = this->basic_query->getEdgeType(_var_i, j);
    int triple_id = this->basic_query->getEdgeID(_var_i, j);
    Triple triple = this->basic_query->getTriple(triple_id);
    string neighbor_name;

    if (edge_type == Util::EDGE_OUT) {
      neighbor_name = triple.object;
    } else {
      neighbor_name = triple.subject;
    }

    //NOTICE: this is another case, vars not in join, we only need constants
    bool only_preid_filter = (this->basic_query->isOneDegreeNotJoinVar(neighbor_name));
    if (only_preid_filter) {
      continue;
    }
    int pre_id = this->basic_query->getEdgePreID(_var_i, j);

    int lit_id = (this->kvstore)->getIDByEntity(neighbor_name);
    if (lit_id == -1) {
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
    if (pre_id >= 0) {
      if (edge_type == Util::EDGE_OUT) {
        (this->kvstore)->getsubIDlistByobjIDpreID(lit_id, pre_id, id_list, id_list_len, true);
      } else {
        (this->kvstore)->getobjIDlistBysubIDpreID(lit_id, pre_id, id_list, id_list_len, true);
      }
    } else if (pre_id == -2) {
      if (edge_type == Util::EDGE_OUT) {
        (this->kvstore)->getsubIDlistByobjID(lit_id, id_list, id_list_len, true);
      } else {
        (this->kvstore)->getobjIDlistBysubID(lit_id, id_list, id_list_len, true);
      }
    } else
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

    if (id_list_len == 0) {
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
    delete[] id_list;
    if (_list.size() == 0) {
      return false;
    }
  }

  Util::logging("OUT constant_edge_filter");
  return true;
}

//BETTER?:merge with constant_edge_filter?
//this only consider subject constant neighbors, while the latter also
//consider constant object neighbors(literal), as well as entities
//neighbors.
//(only in objects, no constant neighbors are called free, dealed in join)
//
//BETTER:not only literals, but also entities may be added here!!!
//(candidates already contain all possible entities, and entities
//produced here may not be ok!)
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
  for (int i = 0; i < this->var_num; i++) {
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

    //if(!this->basic_query->isLiteralVariable(i))
    //{
    //// if this variable is not literal variable, we can assume that its literal candidates have been added.
    //this->basic_query->setAddedLiteralCandidate(i);
    //continue;
    //}

    if (this->basic_query->isReady(i)) {
      continue;
    }

    if (this->basic_query->isSatelliteInJoin(i)) {
      continue;
    }

    // for these literal variable without any linking entities(we call free literal variable),
    // we will add their literal candidates when join-step.
    if (this->basic_query->isFreeLiteralVariable(i)) {
      continue;
    }

    int var_id = i;
    int var_degree = this->basic_query->getVarDegree(var_id);
    IDList literal_candidate_list;

    bool flag = false;
    // intersect each edge's literal candidate.
    for (int j = 0; j < var_degree; j++) {
      int neighbor_id = this->basic_query->getEdgeNeighborID(var_id, j);
      int predicate_id = this->basic_query->getEdgePreID(var_id, j);
      int triple_id = this->basic_query->getEdgeID(var_id, j);
      Triple triple = this->basic_query->getTriple(triple_id);
      string neighbor_name = triple.subject;
      IDList this_edge_literal_list;

      // if the neighbor of this edge is an entity, we can add all literals which has an exact predicate edge linking to this entity.
      if (neighbor_id == -1) {
        int subject_id = (this->kvstore)->getIDByEntity(neighbor_name);
        int* object_list = NULL;
        int object_list_len = 0;

        if (predicate_id >= 0)
          (this->kvstore)->getobjIDlistBysubIDpreID(subject_id, predicate_id, object_list, object_list_len, true);
        else if (predicate_id == -2) {
          this->kvstore->getobjIDlistBysubID(subject_id, object_list, object_list_len, true);
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
      else {
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

      if (!flag) {
        flag = true;
        literal_candidate_list.unionList(this_edge_literal_list);
      } else {
        literal_candidate_list.intersectList(this_edge_literal_list);
      }
    }

    // add the literal_candidate_list to the original candidate list.
    IDList& origin_candidate_list = this->basic_query->getCandidateList(var_id);
    //int origin_candidate_list_len = origin_candidate_list.size();
    origin_candidate_list.unionList(literal_candidate_list, true);
    //int after_add_literal_candidate_list_len = origin_candidate_list.size();

    // this variable's literal candidates have been added.
    //this->basic_query->setAddedLiteralCandidate(var_id);
    this->basic_query->setReady(var_id);

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

//NOTICE:I think we should use this instead of only_pre_filter_after_join
//this function not only consider satellite predicates, but also one degree not selected var and other vars in join
//(constants ar enot necessary considered here)
//this check is fast because predicate num is small, but the performance can be very good
//(instead of filter when joining, we do a precheck first!)
bool
Join::allFilterByPres()
{
  //NOTICE:this check is a must to ensure that we can get all right answers
  //for core vertices after join, then we can generate satellites directly
  for (int i = 0; i < this->var_num; ++i) {
    if (this->basic_query->isSatelliteInJoin(i))
      continue;
    if (this->filterBySatellites(i) == false)
      return false;
  }
  return true;
}

//NOTICE:we should only consider satellites, because constant neighbor edges are already dealed
//and edge in join can be dealed quicked if not satisfy by sp2o or op2s
bool //false when no result for this basicquery
    Join::filterBySatellites(int _var)
{
  //TODO:not consider already dealed edge
  //TODO:not always filter by pres first, maybe not efficient
  //if cans size is small, then can precise to avoid burst-increment when deep-join
  //if size is very large, the cost is high and not many can be filtered!
  //(keep state for each one-degree node, if considered)
  IDList& cans = this->basic_query->getCandidateList(_var);
  int size = this->basic_query->getCandidateSize(_var);

  //result if already empty for non-literal variable
  if (size == 0 && !is_literal_var(_var))
    return false;

  int var_degree = this->basic_query->getVarDegree(_var);
  vector<int> in_edge_pre_id;
  vector<int> out_edge_pre_id;

  for (int i = 0; i < var_degree; i++) {
    char edge_type = this->basic_query->getEdgeType(_var, i);
    int triple_id = this->basic_query->getEdgeID(_var, i);
    Triple triple = this->basic_query->getTriple(triple_id);
    string neighbor;
    if (edge_type == Util::EDGE_OUT) {
      neighbor = triple.object;
    } else {
      neighbor = triple.subject;
    }

    //not consider edge with constant neighbors here
    if (neighbor[0] != '?') {
      //cerr << "not to filter: " << neighbor_name << endl;
      continue;
    }
    //else
    //cerr << "need to filter: " << neighbor_name << endl;

    int pre_id = this->basic_query->getEdgePreID(_var, i);
    //WARN+BETTER:invalid(should be discarded in Query) or ?p(should not be considered here)
    if (pre_id < 0) {
      continue;
    }

    if (edge_type == Util::EDGE_OUT) {
      out_edge_pre_id.push_back(pre_id);
    } else {
      in_edge_pre_id.push_back(pre_id);
    }
  }

  //BETTER:maybe several duplicates of predicates
  //use set instead?
  if (in_edge_pre_id.empty() && out_edge_pre_id.empty()) {
    return true;
  }

  //QUERY:maybe we can divide edges into two separate groups according to the size of p2s
  //NOTICE+BETTER: the cost should be due to the cans size, p2s size and s2p size
  //generally, size of p2s is larger than s2p, but smaller than size of cans
  //The best way is to extract the features of dataset and keep
  //but we may use a simple strategy here: use p2s if cans size is too large, i.e. > size of p2s
  //(assuming 5000 here)
  //WARN:different edge may corresponding different size of subjects, like <rdf:type> is too large

  //QUERY: erase is too costly, use an invalid[] array, maybe bitset due to large candidates size
  //only consider valid ones when join loop, but how about intersect and union?
  //
  //we build a new idlist with all valid ones, and update to the original idlist
  //(consider in current_table is not good, too many duplicates)
  //IDList* valid_list = NULL;
  //int *list = NULL;
  //int len = 0;

  ////TODO+BETTER:o2p and s2p may duplicate many times

  //if (!in_edge_pre_id.empty())
  //{
  //	int size2 = in_edge_pre_id.size();
  //	for (int i = 0; i < size2; ++i)
  //	{
  //		int preid = in_edge_pre_id[i];
  //		this->kvstore->getobjIDlistBypreID(preid, list, len);
  //		if (i == 0)
  //		{
  //			if (size > len)
  //			{
  //				valid_list = IDList::intersect(cans, list, len);
  //			}
  //			else
  //			{
  //				valid_list = new IDList;
  //				int* list2 = NULL;
  //				int len2 = 0;
  //				for (int j = 0; j < size; ++j)
  //				{
  //					this->kvstore->getpreIDlistByobjID(cans[j], list2, len2);
  //					if (Util::bsearch_int_uporder(preid, list2, len2) != -1)
  //					{
  //						valid_list->addID(cans[j]);
  //					}
  //					delete[] list2;
  //				}
  //			}
  //		}
  //		else
  //		{
  //			if (valid_list->size() > len)
  //			{
  //				valid_list->intersectList(list, len);
  //			}
  //			else
  //			{
  //				int* list2 = NULL;
  //				int len2 = 0;
  //				IDList* new_list = new IDList;
  //				int size3 = valid_list->size();
  //				for (int j = 0; j < size3; ++j)
  //				{
  //					this->kvstore->getpreIDlistByobjID(valid_list->getID(j), list2, len2);
  //					if (Util::bsearch_int_uporder(preid, list2, len2) != -1)
  //					{
  //						new_list->addID(valid_list->getID(j));
  //					}
  //					delete[] list2;
  //				}
  //				delete valid_list;
  //				valid_list = new_list;
  //			}
  //		}
  //		delete[] list;
  //	}
  //}

  //if (!is_literal_var(_var) && valid_list != NULL && valid_list->empty())
  //{
  //	//cerr << "quit when empty in edge"<<endl;
  //	return false;
  //}

  //if (!out_edge_pre_id.empty())
  //{
  //	int size2 = out_edge_pre_id.size();
  //	for (int i = 0; i < size2; ++i)
  //	{
  //		int preid = out_edge_pre_id[i];
  //		this->kvstore->getsubIDlistBypreID(preid, list, len);
  //		//cerr<<"p2s len "<<len<<endl;
  //		if (valid_list == NULL && i == 0)
  //		{
  //			if (size > len)
  //			{
  //				valid_list = IDList::intersect(cans, list, len);
  //			}
  //			else
  //			{
  //				valid_list = new IDList;
  //				int* list2 = NULL;
  //				int len2 = 0;
  //				for (int j = 0; j < size; ++j)
  //				{
  //					this->kvstore->getpreIDlistBysubID(cans[j], list2, len2);
  //					if (Util::bsearch_int_uporder(preid, list2, len2) != -1)
  //					{
  //						valid_list->addID(cans[j]);
  //					}
  //					delete[] list2;
  //				}
  //			}
  //		}
  //		else
  //		{
  //			if (valid_list->size() > len)
  //			{
  //				valid_list->intersectList(list, len);
  //			}
  //			else
  //			{
  //				int* list2 = NULL;
  //				int len2 = 0;
  //				IDList* new_list = new IDList;
  //				int size3 = valid_list->size();
  //				for (int j = 0; j < size3; ++j)
  //				{
  //					this->kvstore->getpreIDlistBysubID(valid_list->getID(j), list2, len2);
  //					if (Util::bsearch_int_uporder(preid, list2, len2) != -1)
  //					{
  //						new_list->addID(valid_list->getID(j));
  //					}
  //					delete[] list2;
  //				}
  //				delete valid_list;
  //				valid_list = new_list;
  //			}
  //		}
  //		delete[] list;
  //	}
  //}

  //if (!is_literal_var(_var) && valid_list->empty())
  //{
  //	//cerr << "quit when empty out edge"<<endl;
  //	return false;
  //}
  //cans.copy(valid_list);
  //delete valid_list;

  vector<int> valid_idlist;
  for (int i = 0; i < size; ++i) {
    int ele = cans[i];
    int* list = NULL;
    int list_len = 0;
    bool exist_preid = true;

    if (exist_preid && !in_edge_pre_id.empty()) {
      //(this->kvstore)->getpreIDsubIDlistByobjID(entity_id, pair_list, pair_len);
      (this->kvstore)->getpreIDlistByobjID(ele, list, list_len, true);

      for (vector<int>::iterator itr_pre = in_edge_pre_id.begin(); itr_pre != in_edge_pre_id.end(); itr_pre++) {
        int pre_id = (*itr_pre);
        //the return value is pos, -1 if not found
        if (Util::bsearch_int_uporder(pre_id, list, list_len) == -1)
          exist_preid = false;
        if (!exist_preid) {
          break;
        }
      }
      delete[] list;
    }

    //NOTICE:we do not use intersect here because the case is a little different
    //first the pre num is not so much in a query
    //second once a pre in query is not found, break directly

    if (exist_preid && !out_edge_pre_id.empty()) {
      //(this->kvstore)->getpreIDobjIDlistBysubID(entity_id, pair_list, pair_len);
      (this->kvstore)->getpreIDlistBysubID(ele, list, list_len, true);

      for (vector<int>::iterator itr_pre = out_edge_pre_id.begin(); itr_pre != out_edge_pre_id.end(); itr_pre++) {
        int pre_id = (*itr_pre);
        if (Util::bsearch_int_uporder(pre_id, list, list_len) == -1)
          exist_preid = false;
        if (!exist_preid) {
          break;
        }
      }
      delete[] list;
    }

    //result sequence is illegal when there exists any missing filter predicate id.
    if (exist_preid) {
      valid_idlist.push_back(ele);
    }
  }

  //this is a core vertex, so if not literal var, exit when empty
  if (!is_literal_var(_var) && valid_idlist.empty()) {
    return false;
  }
  cans.copy(valid_idlist);

  cerr << "var " << _var << "size after pre_filter " << cans.size() << endl;
  return true;
}

//if neighbor is an var, but not in select
//then, if its degree is 1, it has none contribution to filter
//only its sole edge property(predicate) makes sense
//we should make sure that current candidateVar has an edge matching the predicate
bool
Join::only_pre_filter_after_join()
{
  for (int var_id = 0; var_id < this->var_num; var_id++) {
    int var_degree = this->basic_query->getVarDegree(var_id);

    //get all the only predicate filter edges for this variable.
    vector<int> in_edge_pre_id;
    vector<int> out_edge_pre_id;
    for (int i = 0; i < var_degree; i++) {
      //WARN:one degree not in select var's id is also -1 !!
      //constant neighbors already be dealed in literal_edge_filter
      //if(this->basic_query->getEdgeNeighborID(var_id, i) == -1)
      //continue;
      char edge_type = this->basic_query->getEdgeType(var_id, i);
      int triple_id = this->basic_query->getEdgeID(var_id, i);
      Triple triple = this->basic_query->getTriple(triple_id);
      string neighbor_name;

      if (edge_type == Util::EDGE_OUT) {
        neighbor_name = triple.object;
      } else {
        neighbor_name = triple.subject;
      }

      bool only_preid_filter = (this->basic_query->isOneDegreeNotJoinVar(neighbor_name));
      if (!only_preid_filter) {
        //cerr << "not to filter: " << neighbor_name << endl;
        continue;
      }
      //else
      //cerr << "need to filter: " << neighbor_name << endl;

      int pre_id = this->basic_query->getEdgePreID(var_id, i);
      if (pre_id < 0) {
        continue;
      }

      if (edge_type == Util::EDGE_OUT) {
        out_edge_pre_id.push_back(pre_id);
      } else {
        in_edge_pre_id.push_back(pre_id);
      }
    }

    if (in_edge_pre_id.empty() && out_edge_pre_id.empty()) {
      continue;
    }

    for (TableIterator it = this->current_table.begin(); it != this->current_table.end();) {
      int entity_id = (*it)[this->id2pos[var_id]];
      int* pair_list = NULL;
      int pair_len = 0;
      bool exist_preid = true;

      //NOTICE: four ways to judge if the predicates exist
      //getpreIDsubIDlistByobjID getpreIDobjIDlistBysubID
      //getsubIDlistBypreIDobjID getobjIDlistBysubIDpreID
      //I think the best one is: getpreIDlistBysubID getpreIDlistByobjID
      //how about getsubIDlistBypreID getobjIDlistBypreID
      //
      //the predicates in query can not be too large, so just loop
      //you can also use an intersect one if the two ordered list are both large
      if (exist_preid && !in_edge_pre_id.empty()) {
        //(this->kvstore)->getpreIDsubIDlistByobjID(entity_id, pair_list, pair_len);
        (this->kvstore)->getpreIDlistByobjID(entity_id, pair_list, pair_len, true);

        for (vector<int>::iterator itr_pre = in_edge_pre_id.begin(); itr_pre != in_edge_pre_id.end(); itr_pre++) {
          int pre_id = (*itr_pre);
          //exist_preid = Util::bsearch_preid_uporder(pre_id, pair_list, pair_len);
          if (Util::bsearch_int_uporder(pre_id, pair_list, pair_len) == -1)
            exist_preid = false;
          if (!exist_preid) {
            break;
          }
        }
        delete[] pair_list;
      }
      if (exist_preid && !out_edge_pre_id.empty()) {
        //(this->kvstore)->getpreIDobjIDlistBysubID(entity_id, pair_list, pair_len);
        (this->kvstore)->getpreIDlistBysubID(entity_id, pair_list, pair_len, true);

        for (vector<int>::iterator itr_pre = out_edge_pre_id.begin(); itr_pre != out_edge_pre_id.end(); itr_pre++) {
          int pre_id = (*itr_pre);
          //exist_preid = Util::bsearch_preid_uporder(pre_id, pair_list, pair_len);
          if (Util::bsearch_int_uporder(pre_id, pair_list, pair_len) == -1)
            exist_preid = false;
          if (!exist_preid) {
            break;
          }
        }
        delete[] pair_list;
      }

      //result sequence is illegal when there exists any missing filter predicate id.
      if (!exist_preid) {
        it = this->current_table.erase(it);
      } else {
        it++;
      }
    }
    if (this->current_table.empty()) {
      return false;
    }
  }
  return true;
}
