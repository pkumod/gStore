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
int
IDList::getID(int _i)const
{
	if (this->size() > _i)
	{
		return this->id_list[_i];
	}
	return -1;
}

bool
IDList::addID(int _id)
{
	//a check for duplicate case will be more reliable 
	this->id_list.push_back(_id);
	return true;
}

int
IDList::size()const
{
	return this->id_list.size();
}

bool
IDList::empty()const
{
	return this->id_list.size() == 0;
}

bool
IDList::isExistID(int _id)const
{
	// naive implementation of searching(linear search).
	// you can use binary search when the id list is sorted, if necessary.
	for (unsigned i = 0; i < this->id_list.size(); i++)
	{
		if (this->id_list[i] == _id)
		{
			return true;
		}
	}

	return false;
}

const vector<int>*
IDList::getList()const
{
	return &(this->id_list);
}


int&
IDList::operator[](const int& _i)
{
	if (this->size() > _i)
	{
		return this->id_list[_i];
	}
	return id_list[0];
}

string
IDList::to_str()
{
	std::stringstream _ss;
	_ss << "size=" << this->id_list.size() << "";
	for (unsigned i = 0; i < this->id_list.size(); i++)
	{
		_ss << "\t[" << this->id_list[i] << "]";
	}
	return _ss.str();
}

int
IDList::sort()
{
	std::sort(id_list.begin(), id_list.end());
	return 0;
}

void
IDList::clear()
{
	this->id_list.clear();
}

void
IDList::copy(const vector<int>& _new_idlist)
{
	this->id_list = _new_idlist;
}

void
IDList::copy(const IDList* _new_idlist)
{
	this->id_list = *(_new_idlist->getList());
}

int
IDList::intersectList(const int* _id_list, int _list_len)
{
	if (_id_list == NULL || _list_len == 0)
	{
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
	if (n < _list_len)
	{
		k = (double)n / (double)_list_len;
		n = _list_len;
		method = 2;
	}
	else
	{
		k = (double)_list_len / (double)n;
		method = 1;
	}
	if (n <= 2)
		method = 0;
	else
	{
		double limit = Util::logarithm(n / 2, 2);
		if (k > limit)
			method = 0;
	}

	int remove_number = 0;
	switch (method)
	{
	case 0:
	{   //this bracket is needed if vars are defined in case
		int id_i = 0;
		int index_move_forward = 0;
		vector<int>::iterator it = this->id_list.begin();
		while (it != (this->id_list).end())
		{
			int can_id = *it;
			while ((id_i < _list_len) && (_id_list[id_i] < can_id))
			{
				id_i++;
			}

			if (id_i == _list_len)
			{
				break;
			}

			if (can_id == _id_list[id_i])
			{
				(this->id_list)[index_move_forward] = can_id;
				index_move_forward++;
				id_i++;
			}

			it++;
		}
		remove_number = this->id_list.size() - index_move_forward;
		vector<int>::iterator new_end = this->id_list.begin() + index_move_forward;
		(this->id_list).erase(new_end, this->id_list.end());
		break;
	}
	case 1:
	{
		vector<int> new_id_list;
		for (int i = 0; i < _list_len; ++i)
		{
			if (Util::bsearch_vec_uporder(_id_list[i], this->getList()) != -1)
				new_id_list.push_back(_id_list[i]);
		}
		this->id_list = new_id_list;
		remove_number = n - this->id_list.size();
		break;
	}
	case 2:
	{
		vector<int> new_id_list;
		int m = this->id_list.size(), i;
		for (i = 0; i < m; ++i)
		{
			if (Util::bsearch_int_uporder(this->id_list[i], _id_list, _list_len) != -1)
				new_id_list.push_back(this->id_list[i]);
		}
		this->id_list = new_id_list;
		remove_number = m - this->id_list.size();
		break;
	}
	default:
		cerr << "no such method in IDList::intersectList()" << endl;
		break;
	}

	return remove_number;
}

int
IDList::intersectList(const IDList& _id_list)
{
	// copy _id_list to the temp array first.
	int temp_list_len = _id_list.size();
	int* temp_list = new int[temp_list_len];
	//BETTER:not to copy, just achieve here
	for (int i = 0; i < temp_list_len; i++)
	{
		temp_list[i] = _id_list.getID(i);
	}

	int remove_number = this->intersectList(temp_list, temp_list_len);
	delete[]temp_list;
	return remove_number;
}

int
IDList::unionList(const int* _id_list, int _list_len, bool only_literal)
{
	if (_id_list == NULL || _list_len == 0)
		return 0;

	if (only_literal)
	{
		//NOTICE:this means that the original is no literals and we need to add from a list(containing entities/literals)
		int k = 0;
		//NOTICE:literal id > entity id; the list is ordered
		for (; k < _list_len; ++k)
			if (Util::is_literal_ele(_id_list[k]))
				break;
		for (; k < _list_len; ++k)
			this->addID(_id_list[k]);
		return _list_len - k;
	}
	// O(n)
	int origin_size = (this->id_list).size();
	int* temp_list = new int[origin_size + _list_len];
	int temp_list_len = 0;

	// union
	{
		int i = 0, j = 0;
		while (i < origin_size && j < _list_len)
		{
			if (this->id_list[i] == _id_list[j])
			{
				temp_list[temp_list_len++] = this->id_list[i];
				i++;
				j++;
			}
			else if (this->id_list[i] < _id_list[j])
			{
				temp_list[temp_list_len++] = this->id_list[i];
				i++;
			}
			else
			{
				temp_list[temp_list_len++] = _id_list[j];
				j++;
			}
		}

		while (i < origin_size)
		{
			temp_list[temp_list_len++] = this->id_list[i];
			i++;
		}
		while (j < _list_len)
		{
			temp_list[temp_list_len++] = _id_list[j];
			j++;
		}
	}

	int add_number = temp_list_len - origin_size;

	// update this IDList
	this->clear();
	for (int i = 0; i < temp_list_len; i++)
		this->addID(temp_list[i]);
	delete[]temp_list;

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

int
IDList::unionList(const IDList& _id_list, bool only_literal)
{
	// copy _id_list to the temp array first.
	int temp_list_len = _id_list.size();
	int* temp_list = new int[temp_list_len];
	//BETTER:not to copy, just achieve here
	for (int i = 0; i < temp_list_len; i++)
	{
		temp_list[i] = _id_list.getID(i);
	}
	int ret = this->unionList(temp_list, temp_list_len, only_literal);
	delete[] temp_list;
	return ret;
}

IDList*
IDList::intersect(const IDList& _id_list, const int* _list, int _len)
{
	IDList* p = new IDList;
	if (_list == NULL || _len == 0)  //just copy _id_list
	{
		int size = _id_list.size();
		for (int i = 0; i < size; ++i)
			p->addID(_id_list.getID(i));
		return p;
	}

	//when size is almost the same, intersect O(n)
	//when one size is small ratio, search in the larger one O(mlogn)
	//
	//n>0 m=nk(0<k<1) 
	//compare n(k+1) and nklogn: k0 = log(n/2)2 requiring that n>2
	//k<=k0 binary search; k>k0 intersect
	int method = -1; //0: intersect 1: search in vector 2: search in int*
	int n = _id_list.size();
	double k = 0;
	if (n < _len)
	{
		k = (double)n / (double)_len;
		n = _len;
		method = 2;
	}
	else
	{
		k = (double)_len / (double)n;
		method = 1;
	}
	if (n <= 2)
		method = 0;
	else
	{
		double limit = Util::logarithm(n / 2, 2);
		if (k > limit)
			method = 0;
	}

	//int remove_number = 0;
	switch (method)
	{
	case 0:
	{   //this bracket is needed if vars are defined in case
		int id_i = 0;
		int num = _id_list.size();
		for (int i = 0; i < num; ++i)
		{
			int can_id = _id_list.getID(i);
			while ((id_i < _len) && (_list[id_i] < can_id))
			{
				id_i++;
			}

			if (id_i == _len)
			{
				break;
			}

			if (can_id == _list[id_i])
			{
				p->addID(can_id);
				id_i++;
			}
		}
		break;
	}
	case 1:
	{
		for (int i = 0; i < _len; ++i)
		{
			if (Util::bsearch_vec_uporder(_list[i], _id_list.getList()) != -1)
				p->addID(_list[i]);
		}
		break;
	}
	case 2:
	{
		int m = _id_list.size(), i;
		for (i = 0; i < m; ++i)
		{
			int t = _id_list.getID(i);
			if (Util::bsearch_int_uporder(t, _list, _len) != -1)
				p->addID(t);
		}
		break;
	}
	default:
		cerr << "no such method in IDList::intersectList()" << endl;
		break;
	}

	return p;
}

int
IDList::erase(int i)
{
	id_list.erase(id_list.begin() + i, id_list.end());
	return 0;
}

int
IDList::bsearch_uporder(int _key)
{
	return Util::bsearch_vec_uporder(_key, this->getList());
}