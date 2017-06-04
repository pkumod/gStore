

#include "ISLRU.h"

ISLRU::ISLRU()
{
	size = 0;
	// mxsize = 10;
}

ISLRU::ISLRU(unsigned _mxsize)
{
	size = 0;
	mxsize = _mxsize;
}

ISLRU::~ISLRU()
{
	
}


ISNode* ISLRU::getTop() const
{
	return data_list.front();
}

unsigned ISLRU::getLen() const 
{
	return size;
}

bool ISLRU::isEmpty() const 
{
	return size == 0;
}

bool ISLRU::insert(ISNode* _np) 
{
	// if(size == mxsize)
	// {
		// return false;
	// }
	size++;
	data_list.emplace(data_list.end(), _np);
	return true;
}

bool ISLRU::removeTop()
{
	if(size == 0)
	{
		return false;
	}
	size--;
	data_list.pop_front();
	return true;
}

IScontainer::iterator ISLRU::beginIter()
{
	return data_list.begin();
}

IScontainer::iterator ISLRU::nxtIter(IScontainer::iterator it)
{
	return ++it;
}

bool ISLRU::isEndIter(IScontainer::iterator it)
{
	return it == data_list.end();
}

bool ISLRU::removeIt(IScontainer::iterator it)
{
	data_list.erase(it);
	return true;
}