

#include "SILRU.h"

SILRU::SILRU()
{
	size = 0;
	// mxsize = 10;
}

SILRU::SILRU(unsigned _mxsize)
{
	size = 0;
	mxsize = _mxsize;
}

SILRU::~SILRU()
{
	
}


SINode* SILRU::getTop() const
{
	return data_list.front();
}

unsigned SILRU::getLen() const 
{
	return size;
}

bool SILRU::isEmpty() const 
{
	return size == 0;
}

bool SILRU::insert(SINode* _np) 
{
	// if(size == mxsize)
	// {
		// return false;
	// }
	size++;
	data_list.emplace(data_list.end(), _np);
	return true;
}

bool SILRU::removeTop()
{
	if(size == 0)
	{
		return false;
	}
	size--;
	data_list.pop_front();
	return true;
}

SIcontainer::iterator SILRU::beginIter()
{
	return data_list.begin();
}

SIcontainer::iterator SILRU::nxtIter(SIcontainer::iterator it)
{
	return ++it;
}

bool SILRU::isEndIter(SIcontainer::iterator it)
{
	return it == data_list.end();
}

bool SILRU::removeIt(SIcontainer::iterator it)
{
	data_list.erase(it);
	return true;
}