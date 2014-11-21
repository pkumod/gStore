/*
 * IDList.cpp
 *
 *  Created on: 2014-7-2
 *      Author: liyouhuan
 */

#include "IDList.h"
#include<sstream>
#include "../util/util.h"
#include <algorithm>
using namespace std;

IDList::IDList()
{
	this->id_list.clear();
}

/* return the _i-th id of the list
 * if _i exceeds, return -1;
 *  */
int IDList::getID(int _i)const
{
	if(this->size() > _i)
	{
		return this->id_list[_i];
	}
	return -1;
}

bool IDList::addID(int _id)
{
	/* a check for duplicate case will be more reliable */
	this->id_list.push_back(_id);
	return true;
}

int IDList::size()const
{
	return this->id_list.size();
}

bool IDList::isExistID(int _id)const
{
    // naive implementation of searching(linear search).
    // you can use binary search when the id list is sorted, if necessary.
    for (int i=0;i<this->id_list.size();i++)
    {
        if (this->id_list[i] == _id)
        {
            return true;
        }
    }

    return false;
}

const std::vector<int>* IDList::getList()const
{
	return &(this->id_list);
}


int& IDList::operator[](const int& _i)
{
	if(this->size() > _i)
	{
		return this->id_list[_i];
	}
	return id_list[0];
}

std::string IDList::to_str()
{
	std::stringstream _ss;
	_ss << "size=" << this->id_list.size() << "";
	for(int i = 0; i < this->id_list.size(); i ++)
	{
		_ss << "\t[" << this->id_list[i] << "]";
	}
	return _ss.str();
}

int IDList::sort()
{
	std::sort(id_list.begin(),id_list.end());
	return 0;
}

void IDList::clear()
{
    this->id_list.clear();
}

int IDList::intersectList(const int* _id_list, int _list_len)
{
	int id_i = 0;
	int index_move_forward = 0;
	std::vector<int>::iterator it = this->id_list.begin();
	while(it != (this->id_list).end())
	{
		int can_id = *it;
		while( (id_i < _list_len) && (_id_list[id_i] < can_id) )
		{
			id_i ++;
		}

		if(id_i == _list_len){
			break;
		}

		if(can_id == _id_list[id_i])
		{
			(this->id_list)[index_move_forward] = can_id;
			index_move_forward ++;
			id_i ++;
		}

		it ++;
	}

	int remove_number = this->id_list.size() - index_move_forward;
	std::vector<int>::iterator new_end =
			this->id_list.begin() + index_move_forward;

	(this->id_list).erase(new_end, this->id_list.end());

	return remove_number;
}

int IDList::intersectList(const IDList& _id_list)
{
    // copy _id_list to the temp array first.
    int temp_list_len = _id_list.size();
    int* temp_list = new int[temp_list_len];
    for (int i = 0; i < temp_list_len; i ++)
    {
        temp_list[i] = _id_list.getID(i);
    }

    int remove_number = this->intersectList(temp_list, temp_list_len);
    delete []temp_list;

    return remove_number;
}

int IDList::unionList(const int* _id_list, int _list_len)
{
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
                temp_list[temp_list_len ++] = this->id_list[i];
                i ++;
                j ++;
            }
            else if (this->id_list[i] < _id_list[j])
            {
                temp_list[temp_list_len ++] = this->id_list[i];
                i ++;
            }
            else
            {
                temp_list[temp_list_len ++] = _id_list[j];
                j ++;
            }
        }

        while (i < origin_size)
        {
            temp_list[temp_list_len ++] = this->id_list[i];
            i ++;
        }
        while (j < _list_len)
        {
            temp_list[temp_list_len ++] = _id_list[j];
            j ++;
        }
    }

    int add_number = temp_list_len - origin_size;

    // update this IDList
    this->clear();
    for (int i = 0; i < temp_list_len; i ++)
        this->addID(temp_list[i]);
    delete []temp_list;

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

int IDList::unionList(const IDList& _id_list)
{
    // copy _id_list to the temp array first.
    int temp_list_len = _id_list.size();
    int* temp_list = new int[temp_list_len];
    for (int i = 0; i < temp_list_len; i ++)
    {
        temp_list[i] = _id_list.getID(i);
    }

    return this->unionList(temp_list, temp_list_len);
}

int IDList::erase(int i)
{
	id_list.erase(id_list.begin()+i,id_list.end());
	return 0;
}


int IDList::bsearch_uporder(int _key)
{
	return util::bsearch_vec_uporder(_key, this->id_list);
}
