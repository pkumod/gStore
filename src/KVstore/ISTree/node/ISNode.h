/*=============================================================================
# Filename: ISNode.h
# Author: syzz
# Mail: 1181955272@qq.com
# Last Modified: 2015-04-26 16:38
# Description: basic Node class, father of ISIntlNode and ISLeafNode
=============================================================================*/

#ifndef _KVSTORE_ISTREE_NODE_ISNODE_H
#define _KVSTORE_ISTREE_NODE_ISNODE_H

#include "../../../Util/Util.h"
#include "../../../Util/Bstr.h"

class ISNode       				//abstract basic class 
{
public:
	static const unsigned DEGREE = 2 * 63;	//the degree of B+ tree
	static const unsigned MAX_CHILD_NUM = DEGREE;
	static const unsigned MIN_CHILD_NUM = DEGREE >> 1;
	static const unsigned MAX_KEY_NUM = MAX_CHILD_NUM - 1;	//max key-num
	static const unsigned MIN_KEY_NUM = MIN_CHILD_NUM - 1;	//min key-num
	/* diffrent flags for tree-nodes, 32-bit put rank in low-bits means no need to move*/
	static const unsigned NF_IL = 0x80000000;	//is leaf
	static const unsigned NF_ID = 0x00080000;	//is dirty, in rank-area
	static const unsigned NF_IM = 0x20000000;	//in memory, not virtual
	//static const unsigned NF_IV = 0x10000000;	//is virtual
	static const unsigned NF_RK = 0x00ffffff;	//select-rank, in Storage
	static const unsigned NF_HT = 0xf00000;		//height area in rank
	static const unsigned NF_KN = 0x07f000;		//NOTICE: decided by DEGREE
	static const unsigned INTL_SIZE = sizeof(int) * MAX_KEY_NUM;
	static const unsigned LEAF_SIZE = INTL_SIZE + sizeof(Bstr) * MAX_KEY_NUM;
	
	int heapId;
protected:
	unsigned store;			//store address, the BLock index
	unsigned flag;			//NF_RK, NF_IL,NF_ID, NF_IV, propety
							//int num; 			//totle keys num
							//Node* father;		//point to father-node, which must be IntlNode
	int* keys;
	void AllocKeys();
	//void FreeKeys();
public:
	ISNode();
	ISNode(bool isVirtual);
	
	bool isLeaf() const;
	bool isDirty() const;
	void setDirty();
	void delDirty();
	bool inMem() const;
	void setMem();
	void delMem();
	//bool isVirtual() const;
	//void setVirtual();
	//void delVirtual();
	unsigned getRank() const;
	void setRank(unsigned _rank);
	unsigned getHeight() const;
	void setHeight(unsigned _h);
	unsigned getNum() const;
	bool setNum(int _num);
	bool addNum();
	bool subNum();
	unsigned getStore() const;
	void setStore(unsigned _store);
	unsigned getFlag() const;
	void setFlag(unsigned _flag);
	unsigned getKey(int _index) const;	//need to check the index
	bool setKey(unsigned _key, int _index);
	bool addKey(unsigned _key, int _index);
	bool subKey(int _index);

	//several binary key search utilities
	int searchKey_less(unsigned _key) const;
	int searchKey_equal(unsigned _key) const;
	int searchKey_lessEqual(unsigned _key) const;

	//virtual functions: polymorphic
	virtual ISNode* getChild(int _index) const { return NULL; };
	virtual bool setChild(ISNode* _child, int _index) { return true; };
	virtual bool addChild(ISNode* _child, int _index) { return true; };
	virtual bool subChild(int _index) { return true; };
	virtual ISNode* getPrev() const { return NULL; };
	virtual ISNode* getNext() const { return NULL; };

	virtual const Bstr* getValue(int _index) const { return NULL; };
	virtual bool setValue(const Bstr* _value, int _index, bool ifcopy = false) { return true; };
	virtual bool addValue(const Bstr* _value, int _index, bool ifcopy = false) { return true; };
	virtual bool subValue(int _index, bool ifdel = false) { return true; };
	virtual void setPrev(ISNode* _prev) {};
	virtual void setNext(ISNode* _next) {};

	virtual bool setValue(char* _str, unsigned _len, int _index, bool ifcopy = false) { return true; };
	virtual bool addValue(char* _str, unsigned _len, int _index, bool ifcopy = false) { return true; };

	//pure virtual function
	virtual void Virtual() = 0;
	virtual void Normal() = 0;
	virtual unsigned getSize() const = 0;		//return all memory owned
	virtual ISNode* split(ISNode* _father, int _index) = 0;
	virtual ISNode* coalesce(ISNode* _father, int _index) = 0;
	virtual void release() = 0;		//release the node, only remain necessary information
	virtual ~ISNode() {};
	virtual void print(std::string s) = 0;		//DEBUG(print the Node)
};

/*NOTICE(operations in release())
*To save memory, we can only remain store and flag(childs added for Leaf).
*However, in this way childs'pointers is ok to change, use Node** or Node*& is also nonsense
*because the pointer variable may die.
*Another way is only to release dynamic memory, and store thw whole, read the Bstr only to
*build. In this way nodes'pointer doesn't change, and operation is simplified, while memory
*is consumed a bit more. Because Bstrs consume the most memory, and memory-disk swapping is
*the most time-consuming thing, it seems to be a better way.
*WARN:when a node is in memory and not deleted, its basic content is always being! If nodes are
*really too many, this will cause disaster because we can't swap them out until tree is closed!
*To solve this problem, there should be two types of release-function: one to release Bstr, one
*to release the whole(pointer is invalid and rebuild problem)
*/

#endif

