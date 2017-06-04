#ifndef _KVSTORE_SITREE_LRU_SILRU_H
#define _KVSTORE_SITREE_LRU_SILRU_H

#include "../../../Util/Util.h"
#include "../node/SINode.h"

typedef SINode* SILnode;
typedef std::list<SILnode> SIcontainer;

class SILRU
{
	
private:
	unsigned size;
	unsigned mxsize;
	SIcontainer data_list;
	
public:
	SILRU();
	SILRU(unsigned _mxsize);
	~SILRU();
	SINode* getTop() const;
	unsigned getLen() const;
	bool isEmpty() const;
	bool insert(SINode* _np);
	bool removeTop();
	SIcontainer::iterator beginIter();
	SIcontainer::iterator nxtIter(SIcontainer::iterator it);
	bool isEndIter(SIcontainer::iterator it);
	bool removeIt(SIcontainer::iterator it);
	
	
	// void print();
};



#endif