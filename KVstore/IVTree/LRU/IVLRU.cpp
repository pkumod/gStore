

#include "IVLRU.h"

IVLRU::IVLRU()
{
	size = 0;
	// mxsize = 10;
}

IVLRU::IVLRU(unsigned _mxsize)
{
	size = 0;
	mxsize = _mxsize;
}

IVLRU::~IVLRU()
{
	
}


IVNode* IVLRU::getTop() const
{
	return data_list.front();
}

unsigned IVLRU::getLen() const 
{
	return size;
}

bool IVLRU::isEmpty() const 
{
	return size == 0;
}

bool IVLRU::insert(IVNode* _np) 
{
	// if(size == mxsize)
	// {
		// return false;
	// }
	size++;
	data_list.emplace(data_list.end(), _np);
	return true;
}

bool IVLRU::removeTop()
{
	if(size == 0)
	{
		return false;
	}
	size--;
	data_list.pop_front();
	return true;
}

IVcontainer::iterator IVLRU::beginIter()
{
	return data_list.begin();
}

IVcontainer::iterator IVLRU::nxtIter(IVcontainer::iterator it)
{
	return ++it;
}

bool IVLRU::isEndIter(IVcontainer::iterator it)
{
	return it == data_list.end();
}

bool IVLRU::removeIt(IVcontainer::iterator it)
{
	data_list.erase(it);
	return true;
}