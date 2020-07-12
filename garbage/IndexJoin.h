/*=============================================================================
# Filename: Join.h
# Author: Bookug Lobert 
# Mail: 1181955272@qq.com
# Last Modified: 2015-12-13 16:05
# Description: design join strategies and select/cost modules
=============================================================================*/

#ifndef _JOIN_JOIN_H
#define _JOIN_JOIN_H

#include "../Query/IDList.h"
#include "../Query/BasicQuery.h"
#include "../Query/SPARQLquery.h"
#include "../KVstore/KVstore.h"
#include "../Util/Util.h"

//BETTER?:place multi_join and index_join in separated files

typedef vector<int> RecordType;
typedef vector<int>::iterator RecordIterator;
typedef list<RecordType> TableType;
typedef list<RecordType>::iterator TableIterator;
typedef list<RecordType>::reverse_iterator TableReverseIterator;
//typedef list< vector<int> > TableType;
//typedef list< vector<int> >::iterator TableIterator;
//typedef list< vector<int> >::reverse_iterator TableReverseIterator;
typedef vector<vector<int*> > IdLists;
typedef vector<vector<int> > IdListsLen;

typedef struct IndexItem {
  int value;
  bool isValid; //needed for final travelling
  //NOTICE: the size of vector is expected to be small
  //the order in vector must be same as in IndexList vector
  vector<list<list<struct IndexItem>::iterator> > travel;
  vector<set<int> > check;
  //map< int, list < list<struct IndexItem>::iterator > > links;  //direct next index list id and linking
  //map< int, set <int> > check;     //indirect previous index list id and verifying
  IndexItem()
  {
    this->value = -1;
    this->isValid = false;
  }
  IndexItem(int _val)
  {
    this->value = _val;
    this->isValid = true;
  }
} IndexItem;

typedef struct IndexList {
  //int next;
  //NOTICE:the list should be ordered at the beginning
  list<IndexItem> candidates;
  list<IndexItem>::iterator border; //used to divide valid and invalid area
  bool prepared;                    //find and set all invalid eles restricted by subtree in travelling
  int position;                     //current neighbor to travel
  vector<int> travel_map;           //the mapping between links position and IndexList id
  vector<int> check_map;            //the mapping between check position and IndexList id
  IndexList()
  {
    //this->next = -1;
    this->prepared = false;
    this->position = 0;
  }
  bool end()
  {
    return this->position == (int)this->travel_map.size();
  }
  int next()
  {
    return this->travel_map[this->position++];
  }
  //NOTICE:we can not use binary-search in list, but this search method maybe slow
  //BETTER?:adjust the list to binary-tree or other struture?
  list<IndexItem>::iterator search(int _val)
  {
    for (list<IndexItem>::iterator it = this->candidates.begin(); it != this->border; ++it) {
      if (it->value == _val)
        return it;
    }
    return this->border;
  }
} IndexList;

typedef struct Satellite {
  int id;
  int* idlist;
  int idlist_len;
  Satellite(int _id, int* _idlist, int _idlist_len)
  {
    this->id = _id;
    this->idlist = _idlist;
    this->idlist_len = _idlist_len;
  }
} Satellite;

typedef list<IndexItem> ItemList;
typedef list<IndexItem>::iterator ItemListIterator;
typedef list<list<struct IndexItem>::iterator> IteratorList;

//Database new Join and pass something like kvstore
class Join {
  private:
  int start_id;
  int var_num;
  //bool* dealed_triple;
  BasicQuery* basic_query;
  KVstore* kvstore;
  //used by score_node for parameters
  static const unsigned PARAM_DEGREE = 1;
  static const unsigned PARAM_SIZE = 100000;
  static const unsigned PARAM_DENSE = 1;
  static const double JUDGE_LIMIT = 0.5;
  static const int LIMIT_CANDIDATE_LIST_SIZE = 1000;
  //BETTER?:predefine size to avoid copy cost
  TableType current_table;
  TableIterator new_start; //keep to end() as default
  //list<bool> table_row_new;

  //keep the mapping for disordered ids in vector<int> table
  int* id2pos;
  int id_pos; //the num of id put into id2pos currently
  int* pos2id;
  bool* dealed_triple;
  stack<int> mystack;

  vector<int*>* result_list;
  vector<Satellite> satellites;
  int* record;
  int record_len;

  void init(BasicQuery* _basic_query);
  void clear();
  void add_id_pos_mapping(int _id);
  void reset_id_pos_mapping();

  //judge which method should be used according to
  //the size of candidates and structure of quering graph
  int judge(int _smallest, int _biggest);

  //select the start point and search order
  void select();

  //score the cost to link two tables and the efficience
  //of filtering
  //int score(List1, List2);

  //score the node according to degree and size
  double score_node(unsigned _degree, unsigned _size);

  void toStartJoin();

  bool filter_before_join();
  bool constant_edge_filter(int _var_i);
  void preid_filter(int _var_i);
  bool only_pre_filter_after_join();
  void add_literal_candidate();
  bool pre_var_handler();
  //bool filterBySatellites(int _var, int _ele);
  bool filterBySatellites(int _var);
  bool allFilterByPres();
  void generateAllSatellites();
  void cartesian(int pos, int end);

  //functions for help
  //copy/add to the end of current_table and set true
  void add_new_to_results(TableIterator it, int id);

  //void set_results_old(list<bool>::iterator it);
  int choose_next_node(int id);

  bool is_literal_var(int id);
  bool is_literal_ele(int _id);

  void copyToResult();

  //BETTER?:change these params to members in class
  void acquire_all_id_lists(IdLists& _id_lists, IdListsLen& _id_lists_len, IDList& _can_list, vector<int>& _edges, int _id, int _can_list_size);
  bool if_prepare_idlist(int _can_list_size, bool _is_literal);
  bool new_join_with_multi_vars_prepared(IdLists& _id_lists, IdListsLen& _id_lists_len, vector<int>& _edges, IDList& _can_list, int _can_list_size);
  bool new_join_with_multi_vars_not_prepared(vector<int>& _edges, IDList& _can_list, int _can_list_size, int _id, bool _is_literal);

  bool multi_join();

  //================================================================================================
  //The index join method saves the memory cost because 2m+2mn < 3mn,
  //and time may be reduced if the pre-process is not too costly
  //because we can reuse the links other than recompute in temporal table
  //New struct is needed for node, i.e. list<bool, int, list<iterator> >,
  //because we may have to delete, but how can we know if an iterator
  //is valid if the one it points to is removed?(remove if the other is removed; using end())
  //1. based on edges: process each time only in valid area(already
  //macthed with others, invalid is removed), and finally it must be
  //all ok, just copy to result_list. We should select the edge order
  //to better the efficiency, but how can we keep only a neighbor links
  //set if we want to save memory?(ensure all can be linked later)
  //2. based on points: search deeply like multi-index-join, only a
  //neighbor links set is kept for a node(not every edge), so memory
  //cost is low. Finally, travel around along valid iterator, copy...

  IndexList* index_lists;

  void buildIndexLists();
  bool travel_init(int _lid);

  bool index_link(int _nid, int _idx);
  bool index_filter(int _nid, int _idx);
  bool table_travel(int _id1, int _id2);
  bool table_check(int _id1, int _id2);

  bool index_travel_one();
  bool index_travel_two();
  bool index_travel();
  bool index_join();

  //NOTICE:this is only used to join a BasicQuery
  bool join();

  public:
  Join();
  Join(KVstore* _kvstore);
  //these functions can be called by Database
  bool join_sparql(SPARQLquery& _sparql_query);
  bool join_basic(BasicQuery* _basic_query);
  ~Join();
};

#endif //_JOIN_JOIN_H
