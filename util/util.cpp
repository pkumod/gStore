/*
 * util.cpp
 *
 *  Created on: 2014-6-20
 *      Author: liyouhuan
 */
#include "util.h"

int util::cmp_int(const void* _i1, const void* _i2)
{
	return *(int*)_i1  -  *(int*)_i2;
}

void util::sort(int*& _id_list, int _list_len)
{
	qsort(_id_list, _list_len, sizeof(int), util::cmp_int);
}

int util::bsearch_int_uporder(int _key,int* _array,int _array_num)
 {
	if (_array_num == 0) {
		return -1;
	}
	if (_array == NULL) {
		return -1;
	}
	int _first = _array[0];
	int _last = _array[_array_num - 1];

	if (_last == _key) {
		return _array_num - 1;
	}

	if (_last < _key || _first > _key) {
		return -1;
	}

	int low = 0;
	int high = _array_num - 1;

	int mid;
	while (low <= high) {
		mid = (high - low) / 2 + low;
		if (_array[mid] == _key) {
			return mid;
		}
		if (_array[mid] > _key) {
			high = mid - 1;
		} else {
			low = mid + 1;
		}
	}
	return -1;
}

bool util::bsearch_preid_uporder(int _preid, int* _pair_idlist, int _list_len)
{
	if(_list_len == 0)
	{
		return false;
	}
	int pair_num = _list_len / 2;
	int _first = _pair_idlist[2*0 + 0];
	int _last = _pair_idlist[2*(pair_num-1) + 0];

	if(_preid == _last){
		return true;
	}

	bool not_find = (_last < _preid || _first > _preid);
	if(not_find){
		return false;
	}

	int low = 0;
	int high = pair_num - 1;
	int mid;

	while (low <= high) {
		mid = (high - low) / 2 + low;
		if (_pair_idlist[2*mid + 0] == _preid) {
			return true;
		}

		if (_pair_idlist[2*mid + 0] > _preid) {
			high = mid - 1;
		} else {
			low = mid + 1;
		}
	}

	return false;
}

int util::bsearch_vec_uporder(int _key, const std::vector<int>& _vec)
{
	int tmp_size = _vec.size();
	if (tmp_size == 0) {
		return -1;
	}

	int _first = _vec[0];
	int _last = _vec[tmp_size - 1];

	if (_key == _last) {
		return tmp_size - 1;
	}

	bool not_find = (_last < _key || _first > _key);
	if (not_find) {
		return -1;
	}

	int low = 0;
	int high = tmp_size - 1;
	int mid;

	while (low <= high) {
		mid = (high - low) / 2 + low;
		if (_vec[mid] == _key) {
			return mid;
		}

		if (_vec[mid] > _key) {
			high = mid - 1;
		} else {
			low = mid + 1;
		}
	}
	return -1;
}

std::string util::result_id_str(std::vector<int*>& _v, int _var_num)
{
	std::stringstream _ss;

	for(int i = 0; i < _v.size(); i ++)
	{
		int* _p_int = _v[i];
		_ss << "[";
		for(int j = 0; j < _var_num-1; j ++)
		{
			_ss << _p_int[j] << ",";
		}
		_ss << _p_int[_var_num-1] << "]\t";
	}

	return _ss.str();
}

bool util::dir_exist(const std::string _dir)
{
	return (opendir(_dir.c_str()) != NULL);
}
bool util::create_dir(const std:: string _dir)
{
	if(! util::dir_exist(_dir))
	{
		mkdir(_dir.c_str(), 0755);
		return true;
	}

	return false;
}

long util::get_cur_time()
{
	timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

bool util::save_to_file(const char* _dir, const std::string _content)
{
    ofstream fout(_dir);

    if (fout.is_open())
    {
        fout << _content;
        fout.close();
    }

    return false;
}
