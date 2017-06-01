#ifndef _KVSTORE_ISTREE_LRU_ISLRU_H
#define _KVSTORE_ISTREE_LRU_ISLRU_H

#include "../../../Util/Util.h"
#include "../node/ISNode.h"

typedef ISNode* ISLnode;
typedef std::list<ISLnode> IScontainer;

class ISLRU
{
	
private:
	unsigned size;
	unsigned mxsize;
	IScontainer data_list;
	
public:
	ISLRU();
	ISLRU(unsigned _mxsize);
	~ISLRU();
	ISNode* getTop() const;
	unsigned getLen() const;
	bool isEmpty() const;
	bool insert(ISNode* _np);
	bool removeTop();
	IScontainer::iterator beginIter();
	IScontainer::iterator nxtIter(IScontainer::iterator it);
	bool isEndIter(IScontainer::iterator it);
	bool removeIt(IScontainer::iterator it);
	
	
	// void print();
};



#endif