#ifndef _KVSTORE_IVTREE_LRU_IVLRU_H
#define _KVSTORE_IVTREE_LRU_IVLRU_H

#include "../../../Util/Util.h"
#include "../node/IVNode.h"

typedef IVNode* IVLnode;
typedef std::list<IVLnode> IVcontainer;

class IVLRU
{
	
private:
	unsigned size;
	unsigned mxsize;
	IVcontainer data_list;
	
public:
	IVLRU();
	IVLRU(unsigned _mxsize);
	~IVLRU();
	IVNode* getTop() const;
	unsigned getLen() const;
	bool isEmpty() const;
	bool insert(IVNode* _np);
	bool removeTop();
	IVcontainer::iterator beginIter();
	IVcontainer::iterator nxtIter(IVcontainer::iterator it);
	bool isEndIter(IVcontainer::iterator it);
	bool removeIt(IVcontainer::iterator it);
	
	
	// void print();
};



#endif