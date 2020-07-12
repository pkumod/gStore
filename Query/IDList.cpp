/*=============================================================================
# Filename: IDList.cpp
# Author: Bookug Lobert
# Mail: zengli-bookug@pku.edu.cn
# Last Modified: 2016-05-08 12:44
# Description: originally written by liyouhuan, modified by zengli
=============================================================================*/

#include "IDList.h"

using namespace std;

IDList::IDList()
{
  this->id_list.clear();
}

//return the _i-th id of the list if _i exceeds, return -1
unsigned
IDList::getID(unsigned _i) const
{
  if (this->size() > _i) {
    return this->id_list[_i];
  }

  //return -1;
  return INVALID;
}

bool
IDList::addID(unsigned _id)
{
  //a check for duplicate case will be more reliable
  this->id_list.push_back(_id);
  return true;
}

unsigned
IDList::size() const
{
  return this->id_list.size();
}

bool
IDList::empty() const
{
  return this->id_list.size() == 0;
}

bool
IDList::isExistID(unsigned _id) const
{
  // naive implementation of searching(linear search).
  // you can use binary search when the id list is sorted, if necessary.
  for (unsigned i = 0; i < this->id_list.size(); i++) {
    if (this->id_list[i] == _id) {
      return true;
    }
  }

  return false;
}

const vector<unsigned>*
IDList::getList() const
{
  return &(this->id_list);
}

unsigned&
IDList::
operator[](const unsigned& _i)
{
  if (this->size() > _i) {
    return this->id_list[_i];
  }
  return id_list[0];
}

string
IDList::to_str()
{
  std::stringstream _ss;
  _ss << "size=" << this->id_list.size() << "";
  for (unsigned i = 0; i < this->id_list.size(); i++) {
    _ss << "\t[" << this->id_list[i] << "]";
  }
  return _ss.str();
}

int
IDList::sort()
{
#ifndef PARALLEL_SORT
  std::sort(id_list.begin(), id_list.end());
#else
  //long t1=Util::get_cur_time();
  omp_set_num_threads(thread_num);
  __gnu_parallel::sort(id_list.begin(), id_list.end());
#endif
  //long t2=Util::get_cur_time();
  //long t=t2-t1;
  //cout<<"\ntime:"<<t<<endl;
  return 0;
}

void
IDList::clear()
{
  this->id_list.clear();
}

void
IDList::copy(const vector<unsigned>& _new_idlist)
{
  this->id_list = _new_idlist;
}

void
IDList::copy(const IDList* _new_idlist)
{
  this->id_list = *(_new_idlist->getList());
}

unsigned
IDList::intersectList(const unsigned* _id_list, unsigned _list_len)
{
  if (_id_list == NULL || _list_len == 0) {
    int remove_number = this->id_list.size();
    this->id_list.clear();
    return remove_number;
  }

  //when size is almost the same, intersect O(n)
  //when one size is small ratio, search in the larger one O(mlogn)
  //
  //n>0 m=nk(0<k<1)
  //compare n(k+1) and nklogn: k0 = log(n/2)2 requiring that n>2
  //k<=k0 binary search; k>k0 intersect
  int method = -1; //0: intersect 1: search in vector 2: search in int*
  int n = this->id_list.size();
  double k = 0;
  if (n < _list_len) {
    k = (double)n / (double)_list_len;
    n = _list_len;
    method = 2;
  } else {
    k = (double)_list_len / (double)n;
    method = 1;
  }
  if (n <= 2)
    method = 0;
  else {
    double limit = Util::logarithm(n / 2, 2);
    if (k > limit)
      method = 0;
  }

  int remove_number = 0;
  switch (method) {
  case 0: { //this bracket is needed if vars are defined in case
    unsigned id_i = 0;
    unsigned index_move_forward = 0;
    vector<unsigned>::iterator it = this->id_list.begin();
    while (it != (this->id_list).end()) {
      int can_id = *it;
      while ((id_i < _list_len) && (_id_list[id_i] < can_id)) {
        id_i++;
      }

      if (id_i == _list_len) {
        break;
      }

      if (can_id == _id_list[id_i]) {
        (this->id_list)[index_move_forward] = can_id;
        index_move_forward++;
        id_i++;
      }

      it++;
    }
    remove_number = this->id_list.size() - index_move_forward;
    vector<unsigned>::iterator new_end = this->id_list.begin() + index_move_forward;
    (this->id_list).erase(new_end, this->id_list.end());
    break;
  }
  case 1: {
    vector<unsigned> new_id_list;
    for (unsigned i = 0; i < _list_len; ++i) {
      if (Util::bsearch_vec_uporder(_id_list[i], this->getList()) != INVALID)
        new_id_list.push_back(_id_list[i]);
    }
    this->id_list = new_id_list;
    remove_number = n - this->id_list.size();
    break;
  }
  case 2: {
    vector<unsigned> new_id_list;
    unsigned m = this->id_list.size(), i;
    for (i = 0; i < m; ++i) {
      if (Util::bsearch_int_uporder(this->id_list[i], _id_list, _list_len) != INVALID)
        new_id_list.push_back(this->id_list[i]);
    }
    this->id_list = new_id_list;
    remove_number = m - this->id_list.size();
    break;
  }
  default:
    cout << "no such method in IDList::intersectList()" << endl;
    break;
  }

  return remove_number;
}

unsigned
IDList::intersectList(const IDList& _id_list)
{
  // copy _id_list to the temp array first.
  unsigned temp_list_len = _id_list.size();
  unsigned* temp_list = new unsigned[temp_list_len];
  //BETTER:not to copy, just achieve here
  for (unsigned i = 0; i < temp_list_len; i++) {
    temp_list[i] = _id_list.getID(i);
  }

  unsigned remove_number = this->intersectList(temp_list, temp_list_len);
  delete[] temp_list;
  return remove_number;
}

unsigned
IDList::unionList(const unsigned* _id_list, unsigned _list_len, bool only_literal)
{
  if (_id_list == NULL || _list_len == 0)
    return 0;

  if (only_literal) {
    //NOTICE:this means that the original is no literals and we need to add from a list(containing entities/literals)
    unsigned k = 0;
    //NOTICE:literal id > entity id; the list is ordered
    for (; k < _list_len; ++k)
      if (Util::is_literal_ele(_id_list[k]))
        break;

    //TODO+BETTER: speed up the process to find the first literal
    for (; k < _list_len; ++k)
      this->addID(_id_list[k]);
    return _list_len - k;
  }
  // O(n)
  unsigned origin_size = (this->id_list).size();
  unsigned* temp_list = new unsigned[origin_size + _list_len];
  unsigned temp_list_len = 0;

  // union
  {
    unsigned i = 0, j = 0;
    while (i < origin_size && j < _list_len) {
      if (this->id_list[i] == _id_list[j]) {
        temp_list[temp_list_len++] = this->id_list[i];
        i++;
        j++;
      } else if (this->id_list[i] < _id_list[j]) {
        temp_list[temp_list_len++] = this->id_list[i];
        i++;
      } else {
        temp_list[temp_list_len++] = _id_list[j];
        j++;
      }
    }

    while (i < origin_size) {
      temp_list[temp_list_len++] = this->id_list[i];
      i++;
    }
    while (j < _list_len) {
      temp_list[temp_list_len++] = _id_list[j];
      j++;
    }
  }

  unsigned add_number = temp_list_len - origin_size;

  // update this IDList
  this->clear();
  for (int i = 0; i < temp_list_len; i++)
    this->addID(temp_list[i]);
  delete[] temp_list;

  return add_number;

  // O(n*logn)
  /*
	int origin_size = (this->id_list).size();

	//union
	{
	int i = 0, j = 0;
	for (i = 0; i < origin_size; i ++)
	{
	while (j < _list_len && _id_list[j] < this->id_list[i])
	{
	this->addID(_id_list[j]);
	j ++;
	}

	if (j == _list_len)
	{
	break;
	}
	}

	for(; j < _list_len; j ++)
	{
	this->addID(_id_list[j]);
	}
	}

	//sort
	this->sort();

	int add_number = this->size() - origin_size;

	return add_number;
	*/
}

unsigned
IDList::unionList(const IDList& _id_list, bool only_literal)
{
  // copy _id_list to the temp array first.
  unsigned temp_list_len = _id_list.size();
  unsigned* temp_list = new unsigned[temp_list_len];
  //BETTER:not to copy, just achieve here
  for (unsigned i = 0; i < temp_list_len; i++) {
    temp_list[i] = _id_list.getID(i);
  }
  unsigned ret = this->unionList(temp_list, temp_list_len, only_literal);
  delete[] temp_list;
  return ret;
}

IDList*
IDList::intersect(const IDList& _id_list, const unsigned* _list, unsigned _len)
{
  IDList* p = new IDList;
  //if (_list == NULL || _len == 0)  //just copy _id_list
  //{
  //int size = _id_list.size();
  //for (int i = 0; i < size; ++i)
  //p->addID(_id_list.getID(i));
  //return p;
  //}

  //when size is almost the same, intersect O(n)
  //when one size is small ratio, search in the larger one O(mlogn)
  //
  //n>0 m=nk(0<k<1)
  //compare n(k+1) and nklogn: k0 = log(n/2)2 requiring that n>2
  //k<=k0 binary search; k>k0 intersect
  int method = -1; //0: intersect 1: search in vector 2: search in int*
  unsigned n = _id_list.size();
  double k = 0;
  if (n < _len) {
    k = (double)n / (double)_len;
    n = _len;
    method = 2;
  } else {
    k = (double)_len / (double)n;
    method = 1;
  }
  if (n <= 2)
    method = 0;
  else {
    double limit = Util::logarithm(n / 2, 2);
    if (k > limit)
      method = 0;
  }

  //int remove_number = 0;
  switch (method) {
  case 0: { //this bracket is needed if vars are defined in case
    int id_i = 0;
    unsigned num = _id_list.size();
    for (unsigned i = 0; i < num; ++i) {
      unsigned can_id = _id_list.getID(i);
      while ((id_i < _len) && (_list[id_i] < can_id)) {
        id_i++;
      }

      if (id_i == _len) {
        break;
      }

      if (can_id == _list[id_i]) {
        p->addID(can_id);
        id_i++;
      }
    }
    break;
  }
  case 1: {
    for (unsigned i = 0; i < _len; ++i) {
      if (Util::bsearch_vec_uporder(_list[i], _id_list.getList()) != INVALID)
        p->addID(_list[i]);
    }
    break;
  }
  case 2: {
    unsigned m = _id_list.size(), i;
    for (i = 0; i < m; ++i) {
      unsigned t = _id_list.getID(i);
      if (Util::bsearch_int_uporder(t, _list, _len) != INVALID)
        p->addID(t);
    }
    break;
  }
  default:
    cout << "no such method in IDList::intersectList()" << endl;
    break;
  }

  return p;
}

bool
IDList::erase(unsigned i)
{
  id_list.erase(id_list.begin() + i, id_list.end());
  return true;
}

unsigned
IDList::bsearch_uporder(unsigned _key)
{
  return Util::bsearch_vec_uporder(_key, this->getList());
}

std::vector<unsigned>::iterator
IDList::eraseAt(std::vector<unsigned>::iterator it)
{
  return id_list.erase(it);
}
